"""Read the lemming state machine and its constants out of the exe.

This is the re-runnable form of the static read in PLAN.md 2.3. It needs
nothing but the shipped executable: no Ghidra, no REA, no running game. REA was
used for exactly one thing, `xrefs` on `0x004137CA`, and the answer it gave is
the reason this script exists rather than a decompiler transcript: the only
reference to the climb stepper is `0x004B0964`, which is in `.rdata`. It is a
vtable slot. There is no call site to decompile, so the way in is through the
vtable, and that is a plain read of the file.

The states are 6-slot classes:

    +0x00  deleting destructor
    +0x04  step this state for one tick
    +0x08  on-enter hook -- sets the animation, writes this state's constants
    +0x0C  on-exit hook, an empty stub in most
    +0x10  common base stub 0x004011D0
    +0x14  common base stub 0x004011E0

Slot 2 is an *enter hook*, not a name getter. Among other work it calls
`0x0042FD25` with a string, and that string is the `.SPR` animation strip name:
`0x0042FD25` indexes a table at `0x004E335C` by the lemming's `+0x4C` and looks
the name up in the sheet. Scanning all of `.text` for

    push <str> ; mov ecx, [ebp+8] ; call 0x0042FD25

finds 24 sites, and their strings are exactly the 15 strips in
`LEMMINGSLO.SPR` -- bash block build cartwheel climb dig drown explode fall
flip float mine shrug umbopen walk -- plus `trampoline`, which has its own
sheet. Nothing else in the binary sets an animation. That closed set, checked
against our own `.SPR` parser, is what makes the naming a read rather than a
guess. Three of the sites are inside steppers rather than enter hooks, which is
why a vtable-only scan misses `build`, `float` and `umbopen`.

What the constants mean. Each vertical stepper asks a chunk function for a time
slice in seconds, takes min(slice, remaining), and steps Y by
`direction * speed * slice`, so a speed constant is world units per *second*.
A whole tick is 0.2 s, measured three independent ways (PLAN.md 2.2). The
walk's 5.0 and the climb's 5.0 reproduce two independently *measured* per-tick
values exactly -- 1 lattice column and 1 world unit -- and that is what fixes
the units for every other state read here.

The ordinary faller is the state at vtable `0x004B0A20`. It is the one that
carries the splat test, the floater check and the umbrella, so it is the state
a lemming's own fall uses; the sibling at `0x004B0C60` has none of those and
its 25.0 is some other descent. See PLAN.md 2.3.

Needs `capstone` (`pip install capstone`). Usage:

    python tools/harness/read_steppers.py                  # the whole read
    python tools/harness/read_steppers.py --states         # the state map
    python tools/harness/read_steppers.py --at 0x416038 --len 0x60
    python tools/harness/read_steppers.py --consts 0x4160bd --len 0x300
    python tools/harness/read_steppers.py --cell-flags

`--cell-flags` is the collision-cell bit scan (the seven `kCellUnreadBits`
values). It does not change the stepper report above.

The exe path defaults to the retail install and can be overridden with `--exe`
or `WEBOLUTION_GAME_DIR`.
"""

from __future__ import annotations

import argparse
import bisect
import os
import re
import struct
import sys
from pathlib import Path

# The stock 640x480 executable this read was taken against. A different build
# would move every address below, so the digest is checked, not assumed.
EXPECTED_SHA256 = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"

DEFAULT_EXE = Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")

# MEASURED, PLAN.md 2.2: the simulation steps five times a second, on a flat
# 200 ms period, and is not frame-locked. Every per-tick number below is a
# units-per-second constant multiplied by this.
TICK_SECONDS = 0.2

# MEASURED, PLAN.md Phase 3: a climber rises one world unit per tick, which is
# two rows of our mask.
ROWS_PER_UNIT = 2

# 0x0042FD25(this = lemming, str) sets the animation by .SPR strip name.
ANIM_SETTER = 0x0042FD25

# The steppers worth printing in full, by vtable. Slot 1 is the stepper.
STEPPERS = [
    ("walk", 0x004B0D28, 0x0041BDD4, 0x120),
    ("climb", 0x004B0960, 0x004137CA, 0x15A),
    ("fall (the ordinary one)", 0x004B0A20, 0x004160BD, 0x1B0),
    ("fall (the 25.0 sibling)", 0x004B0C60, 0x0041A6D5, 0x15B),
]

# The speed constants, each as (label, constant address, the instruction that
# loads it, and the unit one tick of it is measured in).
SPEEDS = [
    ("walk", 0x004B10B0, 0x0041BE70, "mask columns"),
    ("climb", 0x004B10B0, 0x0041386C, "world units"),
    ("fall", 0x004B0A18, 0x0041604A, "world units"),
    ("fall, 25.0 sibling", 0x004B0C40, 0x0041A758, "world units"),
]

# Thresholds the ordinary faller tests, each against the fall accumulator --
# the double at the state's +0x18, incremented by 1.0 per sub-step at
# 0x00416318, so it counts world units fallen.
THRESHOLDS = [
    ("floatAfter (umbrella opens past this)", 0x004B03E8, 0x004162D9),
    ("splatDistance (kills past this)", 0x004B10A8, 0x004165FF),
]

# `bash`, `dig` and `mine` are one class parameterised by its enter hook: all
# three vtables carry the same slot-1 stepper. The hook sets the animation and
# the animation rate; it writes no speed, no interval and no reach.
SKILL_STATES = [
    ("bash", 0x004B06AC, 0x0040BFAA),
    ("dig", 0x004B09F0, 0x004159B4),
    ("mine", 0x004B0B68, 0x0041884A),
]
SHARED_SKILL_STEPPER = 0x0041B077

# A terrain-removal function, `FUN_0041B683`. No direct caller in `.text`: it
# is slot 3 of the `trampoline` vtable `0x004B0D08` and is reached only by
# virtual dispatch. Whose removal it is, is NOT established -- bash, dig and
# mine carry different slot-3 entries (`0x00401320`, `0x00415995`,
# `0x004188C3`), so do not read this as the skills' dig depth or bash reach.
# What is read is the shape: four cells, and which bits get cleared.
REMOVAL = 0x0041B683
# Loop 1 runs `i = 0 .. [0x004B10B8]` inclusive, stepping `y + i * [+0x44]`,
# so the bound is one less than the number of cells cleared.
REMOVAL_DEPTH_BOUND = 0x004B10B8
REMOVAL_CLEARS = [
    ("column run, [+0x44] direction", 0x0041B783, 0x0000F77C),
    ("one further cell", 0x0041B801, 0x0000E77C),
]


class Image:
    """The executable as an addressable image."""

    def __init__(self, path: Path):
        self.path = path
        self.data = path.read_bytes()
        b = self.data
        pe = struct.unpack_from("<I", b, 0x3C)[0]
        if b[pe : pe + 4] != b"PE\0\0":
            raise SystemExit(f"{path}: not a PE image")
        coff = pe + 4
        nsec = struct.unpack_from("<H", b, coff + 2)[0]
        optsz = struct.unpack_from("<H", b, coff + 16)[0]
        opt = coff + 20
        self.base = struct.unpack_from("<I", b, opt + 28)[0]
        self.sections = []
        for i in range(nsec):
            o = opt + optsz + i * 40
            name = b[o : o + 8].rstrip(b"\0").decode("latin1")
            vsize, va, rsize, ptr = struct.unpack_from("<IIII", b, o + 8)
            self.sections.append((name, self.base + va, max(vsize, rsize), ptr))

    def section(self, name: str):
        for s in self.sections:
            if s[0] == name:
                return s
        raise SystemExit(f"{self.path}: no {name} section")

    def offset(self, addr: int):
        """File offset for a virtual address, or None if it is not mapped."""
        for _name, va, size, ptr in self.sections:
            if va <= addr < va + size:
                return ptr + (addr - va)
        return None

    def read(self, addr: int, n: int) -> bytes:
        o = self.offset(addr)
        if o is None:
            raise SystemExit(f"{addr:#010x} is not in any section")
        return self.data[o : o + n]

    def u32(self, addr: int) -> int:
        return struct.unpack_from("<I", self.read(addr, 4))[0]

    def f64(self, addr: int) -> float:
        return struct.unpack_from("<d", self.read(addr, 8))[0]

    def cstr(self, addr: int, limit: int = 96):
        o = self.offset(addr)
        if o is None:
            return None
        end = o
        while end < len(self.data) and self.data[end] and end - o < limit:
            end += 1
        s = self.data[o:end]
        if len(s) < 2 or not all(32 <= c < 127 for c in s):
            return None
        return s.decode("latin1")


def animation_sites(img: Image):
    """Every `push <str>; mov ecx,[ebp+8]; call 0x0042FD25` in `.text`.

    One site per state entry. The byte pattern is a fixed 13 bytes, so this is
    a literal scan and not a disassembly: `68 imm32` / `8b 4d 08` / `e8 rel32`.
    """
    _n, text_va, text_size, text_ptr = img.section(".text")
    blob = img.data[text_ptr : text_ptr + text_size]
    out = []
    i = 0
    while True:
        i = blob.find(b"\x68", i)
        if i < 0 or i + 13 > len(blob):
            break
        if blob[i + 5 : i + 8] == b"\x8b\x4d\x08" and blob[i + 8] == 0xE8:
            rel = struct.unpack_from("<i", blob, i + 9)[0]
            if text_va + i + 13 + rel == ANIM_SETTER:
                imm = struct.unpack_from("<I", blob, i + 1)[0]
                name = img.cstr(imm)
                if name:
                    out.append((text_va + i, imm, name))
        i += 1
    return out


# An enter hook is small; this covers every one of them, and a site belonging
# to the next function would be past the hook's own `ret` anyway.
HOOK_SPAN = 0x240


def vtables(img: Image, sites=None):
    """Every 6-slot state vtable, named by the animation its enter hook sets.

    The shape is what identifies a table: slots 4 and 5 are the two common base
    stubs `0x004011D0` and `0x004011E0` in all of them, and the first four
    slots point into `.text`. Scanning for that pair is more reliable than
    scanning for the 0x18 stride, because the stride breaks wherever the
    compiler emitted a double literal between two tables.

    A table is named when an animation site falls inside its enter hook. States
    that change no animation on entry stay unnamed, and so do the three whose
    animation is set from the stepper instead -- `build`, `float`, `umbopen`.
    """
    sites = animation_sites(img) if sites is None else sites
    _n, rdata_va, rdata_size, rdata_ptr = img.section(".rdata")
    _n, text_va, text_size, _p = img.section(".text")
    blob = img.data[rdata_ptr : rdata_ptr + rdata_size]
    out = []
    for i in range(0, len(blob) - 24, 4):
        slots = struct.unpack_from("<6I", blob, i)
        if slots[4] != 0x004011D0 or slots[5] != 0x004011E0:
            continue
        if not all(text_va <= s < text_va + text_size for s in slots[:4]):
            continue
        hook = slots[2]
        name = next((n for a, _s, n in sites if hook <= a < hook + HOOK_SPAN), None)
        out.append((rdata_va + i, slots, name))
    return out


def _capstone():
    try:
        import capstone
    except ImportError:
        raise SystemExit("needs capstone: pip install capstone")
    return capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)


ABSOLUTE = re.compile(r"\[(0x4[0-9a-f]{5})\]")


def constants(img: Image, start: int, limit: int):
    """Distinct `.rdata` doubles a function loads, in address order.

    This is how a state's own constants surface: the enter hook writes them and
    the stepper compares against them, and both reach them through an absolute
    `.rdata` operand.
    """
    md = _capstone()
    _n, rdata_va, rdata_size, _p = img.section(".rdata")
    seen, out = set(), []
    for ins in md.disasm(img.read(start, limit), start):
        m = ABSOLUTE.search(ins.op_str)
        if not m:
            continue
        ea = int(m.group(1), 16)
        if not (rdata_va <= ea < rdata_va + rdata_size):
            continue
        key = (ins.mnemonic, ea)
        if key in seen:
            continue
        seen.add(key)
        out.append((ins.address, ins.mnemonic, ea, img.f64(ea)))
    return out


def disassemble(img: Image, addr: int, length: int, stop_at_ret: bool = True):
    """Disassemble, annotating `.rdata` operands with the double they hold."""
    md = _capstone()
    _n, rdata_va, rdata_size, _p = img.section(".rdata")
    for ins in md.disasm(img.read(addr, length), addr):
        note = ""
        m = ABSOLUTE.search(ins.op_str)
        if m:
            ea = int(m.group(1), 16)
            if rdata_va <= ea < rdata_va + rdata_size:
                raw = img.read(ea, 8)
                note = f"   ; {raw.hex(' ')} = {img.f64(ea)!r}"
        print(f"{ins.address:08x}  {ins.bytes.hex(' '):<20}  {ins.mnemonic:<6} {ins.op_str}{note}")
        if ins.mnemonic == "ret" and stop_at_ret:
            print("  --- ret ---")
            return


def locate_exe(arg: str | None) -> Path:
    if arg:
        return Path(arg)
    game_dir = os.environ.get("WEBOLUTION_GAME_DIR")
    if game_dir:
        p = Path(game_dir) / "Lemmings Revolution.exe"
        if p.exists():
            return p
    return DEFAULT_EXE


def report_states(img: Image):
    """The state map: the animation sites, and the vtables they name."""
    sites = animation_sites(img)
    print(f"# {len(sites)} animation-set sites -- every state entry in the binary")
    print("#   site        string      animation")
    for site, imm, name in sites:
        print(f"    {site:08x}    {imm:08x}    {name}")
    print()
    tables = vtables(img, sites)
    named = [t for t in tables if t[2]]
    print(f"# {len(tables)} six-slot state vtables, {len(named)} named by their enter hook")
    print("#   vtable      dtor      step      enter     exit      animation")
    for va, slots, name in tables:
        mark = "  <--" if any(va == vt for _n, vt, _s, _l in STEPPERS) else ""
        print(
            f"    {va:08x}    {slots[0]:08x}  {slots[1]:08x}  {slots[2]:08x}  "
            f"{slots[3]:08x}  {name or '-'}{mark}"
        )
    print()
    hooks = [s[2] for _va, s, _n in tables]
    print("# Set from a stepper rather than an enter hook, so no vtable claims them:")
    for site, _imm, name in sites:
        if not any(h <= site < h + HOOK_SPAN for h in hooks):
            print(f"    {site:08x}  {name}")
    print()
    return sites, tables


def report_speeds(img: Image):
    """The speed constants and the thresholds, against the measured tick."""
    print(
        "# Speeds. A constant is world units per second; a tick is "
        f"{TICK_SECONDS} s (MEASURED)."
    )
    for label, addr, used_at, unit in SPEEDS:
        v = img.f64(addr)
        print(
            f"#   {label:<20} [{addr:08x}] = {v!r:<6} /s  -> {v * TICK_SECONDS!r} "
            f"{unit} per tick   (loaded at {used_at:#010x})"
        )
    print()
    print("# The ordinary faller, vtable 0x004B0A20. Its speed accessor")
    print("# 0x00416038 branches on the lemming's umbrella flag [+0x4b]:")
    print("#   0041604a  fld   [004b0a18]   ; the fall speed")
    print("#   00416050  fdiv  [004b03f8]   ; halved while the umbrella is open")
    fall = img.f64(0x004B0A18)
    half = img.f64(0x004B03F8)
    for label, speed in (("fall", fall), ("float", fall / half)):
        units = speed * TICK_SECONDS
        print(
            f"#   {label:<6} {speed!r:<5} /s  -> {units!r} world units per tick"
            f"  = {units * ROWS_PER_UNIT!r} mask rows"
        )
    print()
    print("# Thresholds, against the fall accumulator at the state's +0x18")
    print("# (world units, incremented 1.0 per sub-step at 0x00416318):")
    for label, addr, used_at in THRESHOLDS:
        v = img.f64(addr)
        print(
            f"#   {label:<38} [{addr:08x}] = {v!r} world units"
            f"  = {v * ROWS_PER_UNIT!r} mask rows   (tested at {used_at:#010x})"
        )
    print()
    print("# The splat test at 0x004165FF is skipped when the lemming's")
    print("# has-floater byte [+0x49] is set, and the landing cell's own flags")
    print("# can exempt it too: bit 0x80 is not landable at all, and bits")
    print("# 0x200 / 0x4000 exempt lemming types 1 and 2 ([lemming+0x4c]).")


def report_skills(img: Image):
    """The terrain skills: what their states share, and what the removal does.

    The headline is a negative. `bash`, `dig` and `mine` carry no interval, no
    reach and no speed of their own -- the three vtables share one stepper and
    their enter hooks differ only in the animation they select and the rate
    they give it. The stepper's own debug strings call the mechanism a
    "staggered action": it advances the animation and fires the removal on a
    new frame, so what classic Lemmings expresses as a tick interval this
    engine expresses as an animation rate. Nothing here can be promoted into
    `Rules::basherInterval` and friends without reading the animation clock.
    """
    print("# The terrain skills share one stepper, parameterised by the hook:")
    for name, vt, hook in SKILL_STATES:
        slot1 = img.u32(vt + 4)
        shared = "shared" if slot1 == SHARED_SKILL_STEPPER else f"!! {slot1:#010x}"
        print(f"#   {name:<5} vtable {vt:08x}  enter {hook:08x}  stepper {slot1:08x}  {shared}")
    print(f"#   the shared stepper is {SHARED_SKILL_STEPPER:#010x}")
    print()
    print("# Its timing is the animation's, not a tick count:")
    print("#   0041b163  cmp dword ptr [edx+0x50], 3   ; the same state tag the")
    print("#   0041b16e  fdiv qword ptr [0x004b03f8]   ; climb halves on, 0x004137d6")
    print("#   0041b19b  call 0x00468b5d               ; on the lemming's +0x128")
    print("# and the 100.0 at 0x004b0ce0 is a percentage in a log line, not a")
    print("# game constant -- 0x004bf564 is \"Staggered action new frame %d\".")
    print("# +0x128 is the animation controller: 0x004683e9 writes a double to")
    print("# its +0x1d8 (dig's hook passes 0.75, against the 0.5 the .SPR sheet")
    print("# carries for that strip), and 0x00468b04 scales by the 0.06 at")
    print("# 0x004b1940. So the skill rates live there, and are unread.")
    print()
    bound = img.u32(REMOVAL_DEPTH_BOUND)
    print(f"# A terrain removal is {REMOVAL:#010x} -- slot 3 of the trampoline")
    print("# vtable 0x004b0d08, no direct caller, and NOT shown to be the skills'.")
    print(f"#   loop bound [{REMOVAL_DEPTH_BOUND:08x}] = {bound}, inclusive from 0")
    print(f"#   -> {bound + 1} cells stepped by [+0x44] from the lemming's own cell,")
    print(f"#      which is {bound + 1} world units = {(bound + 1) * ROWS_PER_UNIT} of our mask rows")
    for label, site, mask in REMOVAL_CLEARS:
        cleared = (~mask) & 0xFFFF
        bits = ", ".join(f"{1 << b:#x}" for b in range(16) if cleared & (1 << b))
        print(f"#   {site:08x}  and {mask:#06x}  clears {bits}   ({label})")
    print()
    print("# No enter hook writes [+0x44], so which way a given skill removes is")
    print("# not set where the hooks are. Until that is read, the cell count")
    print("# above cannot be assigned to diggerDepth rather than basherReach,")
    print("# and `Rules` keeps its provisional values.")


# The seven bits CollisionGrid.h still lists as kCellUnreadBits. Each known
# bit was an immediate on test/and/or, or a push into the box stamp, so this
# scan looks for those forms and nothing subtler. See --cell-flags.
UNREAD_CELL_BITS = (0x4, 0x8, 0x20, 0x40, 0x100, 0x400, 0x8000)
UNREAD_CELL_MASK = 0x4 | 0x8 | 0x20 | 0x40 | 0x100 | 0x400 | 0x8000
# Bit index -> mask, for bt/bts/btr/btc immediates.
UNREAD_BIT_INDEX = {2: 0x4, 3: 0x8, 5: 0x20, 6: 0x40, 8: 0x100, 10: 0x400, 15: 0x8000}
# 2^n-1. An and/test with one of these keeps or tests a low run, not one flag.
WIDTH_MASKS = {(1 << n) - 1 for n in range(1, 17)}
REG8 = {"al", "ah", "bl", "bh", "cl", "ch", "dl", "dh"}
# AH is bits 8..15 of the dword. `or ah, 8` sets 0x800, the known blocker bit,
# not unread bit 0x8. The low byte (AL) is bits 0..7, so `or al, 0x40` really
# is bit 0x40.
HIGH8 = {"ah", "bh", "ch", "dh"}
REG16 = {"ax", "bx", "cx", "dx", "si", "di", "bp", "sp"}
REG32 = {"eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", "esp"}
MASK_MNEMONICS = {"test", "and", "or", "xor", "push", "bt", "bts", "btr", "btc"}

# The cell probe, and the two box writers. MEASURED from the image, not guessed:
# 0x40DE60 returns a pointer to one u32 cell; 0x4311D0 does `or [cell], [ebp+8]`;
# 0x43127E does `not` on [ebp+8] and `and`s that into the cell.
CELL_PROBE = 0x0040DE60
CELL_STAMP = 0x004311D0
CELL_UNSTAMP = 0x0043127E

# Sites the known bits are read at. If the sweep does not find one of these,
# it lost coverage and a negative on the unread bits is not evidence.
_KNOWN_CELL_SITES = (
    (0x004248E6, "and", 0x2),
    (0x00424905, "and", 0x1),
    (0x0041A99A, "and", 0x80),
    (0x004101F8, "or", 0x1),
    (0x0040E1D5, "and", 0xF7FF),
    (0x0041B783, "and", 0xF77C),
    (0x0041B801, "and", 0xE77C),
    (0x004311AE, "push", 0x80),
    (0x00428D58, "push", 0x200),
    (0x00428D79, "push", 0x2000),
    (0x00428D9A, "push", 0x4000),
    (0x0043126C, "or", None),
)
_KNOWN_PROLOGUES = (
    0x0040DE60,
    0x00404D10,
    0x004311D0,
    0x0043127E,
    0x0043119B,
    0x0041A849,
    0x0042478F,
    0x0041B683,
    0x0041011C,
    0x0040E0DF,
)
# Combinations named in the brief. They are not extra bits; the scan reports
# whether the image uses the combination as an immediate anyway.
_COMBO_IMMS = {
    0x82: "0x80|0x02",
    0x403: "0x400|0x002|0x001",
    0x21: "0x20|0x01",
}


def _popcount(n: int) -> int:
    return bin(n).count("1")


def _imm(op_str: str):
    """The trailing immediate, or None when the last operand is not one.

    Capstone prints small immediates in decimal (`and edx, 2`) and larger ones
    in hex (`and edx, 0x80`). Both parse here. A memory operand does not.
    """
    tok = op_str.split(",")[-1].strip()
    body = tok[1:] if tok.startswith("-") else tok
    if not body or not body[0].isdigit():
        return None
    try:
        return int(tok, 0) & 0xFFFFFFFF
    except ValueError:
        return None


def _dest(op_str: str) -> str:
    return op_str.split(",")[0].strip()


def _text_bytes(img: Image):
    """File-backed .text, plus the section flags so the boundary is visible.

    Image.section reports max(vsize, raw). The scan uses the raw file bytes
    only: nothing past the file is code.
    """
    b = img.data
    pe = struct.unpack_from("<I", b, 0x3C)[0]
    coff = pe + 4
    nsec = struct.unpack_from("<H", b, coff + 2)[0]
    optsz = struct.unpack_from("<H", b, coff + 16)[0]
    opt = coff + 20
    found = None
    flags = []
    for i in range(nsec):
        o = opt + optsz + i * 40
        name = b[o : o + 8].rstrip(b"\0").decode("latin1")
        vsize, va, rsize, ptr = struct.unpack_from("<IIII", b, o + 8)
        chars = struct.unpack_from("<I", b, o + 36)[0]
        flags.append((name, img.base + va, vsize, rsize, chars))
        if name == ".text":
            found = (img.base + va, vsize, rsize, b[ptr : ptr + rsize], chars)
    if found is None:
        raise SystemExit(f"{img.path}: no .text section")
    return found, flags


def _linear(code: bytes, base: int):
    """Every instruction in .text, resuming one byte past anything undecodable.

    Capstone stops at the first byte it will not decode. A jump table in the
    middle of .text would otherwise hide the rest of the image, so this skips
    that byte and continues. `skipped` is the count of those bytes.
    """
    md = _capstone()
    out = []
    skipped = 0
    i = 0
    n = len(code)
    while i < n:
        start = len(out)
        for ins in md.disasm(code[i:], base + i):
            out.append((ins.address, ins.size, ins.mnemonic, ins.op_str))
        if len(out) == start:
            skipped += 1
            i += 1
            continue
        nxt = out[-1][0] + out[-1][1] - base
        if nxt <= i:
            skipped += 1
            i += 1
            continue
        if nxt < n:
            skipped += 1
            i = nxt + 1
        else:
            i = nxt
    return out, skipped


def _prologues(code: bytes, base: int):
    """MSVC `push ebp; mov ebp, esp` sitting on a real boundary.

    The boundary is int3 padding, a plain ret, a ret imm16, or the hot-patch
    `mov edi, edi`. A bare 0x00 is not enough: that byte is the high half of
    many immediates, and treating it as a boundary splits live functions.
    """
    sig = b"\x55\x8b\xec"
    starts = []
    i = 0
    while True:
        j = code.find(sig, i)
        if j < 0:
            break
        prev = code[j - 1] if j else 0xCC
        ret_imm = j >= 3 and code[j - 3] == 0xC2
        hot = j >= 2 and code[j - 2 : j] == b"\x8b\xff"
        if j == 0 or prev in (0xCC, 0xC3, 0x90) or ret_imm or hot:
            starts.append(base + j)
        i = j + 1
    return starts


def _func_of(starts: list[int], addr: int):
    i = bisect.bisect_right(starts, addr) - 1
    if i < 0:
        return None
    return starts[i]


def _and_touch(imm: int):
    """How an `and` immediate relates to the unread bits.

    Returns (channel, action, bits) or None. `channel` is specific, width or
    alignment. A width mask (2^n-1) is not a one-bit flag test. The exception
    is a width that clears exactly one unread bit and no other bit in the low
    16 -- that is `and x, 0x7fff` clearing 0x8000, and it is specific.
    """
    imm &= 0xFFFFFFFF
    high = imm >> 16
    if high == 0xFFFF:
        cleared = (~imm) & 0xFFFF
        if cleared and cleared <= 0x3F and cleared & (cleared + 1) == 0:
            bits = cleared & UNREAD_CELL_MASK
            return ("alignment", "clear", bits) if bits else None
        bits = cleared & UNREAD_CELL_MASK
        if bits:
            return "specific", "clear", bits
        return None
    if imm > 0xFFFF:
        return None
    if imm in WIDTH_MASKS:
        cleared = (~imm) & 0xFFFF
        if cleared in UNREAD_CELL_BITS:
            return "specific", "clear", cleared
        kept = imm & UNREAD_CELL_MASK
        if kept:
            return "width", "keep", kept
        cleared_unread = cleared & UNREAD_CELL_MASK
        if cleared_unread:
            return "width", "clear", cleared_unread
        return None
    if _popcount(imm) <= 8:
        bits = imm & UNREAD_CELL_MASK
        if bits:
            return "specific", "keep", bits
        return None
    cleared = (~imm) & 0xFFFF
    bits = cleared & UNREAD_CELL_MASK
    if bits and _popcount(cleared) <= 6:
        return "specific", "clear", bits
    return None


def _touch(mnemonic: str, imm: int):
    """(channel, action, bits) if this immediate mentions an unread bit."""
    if mnemonic in ("bt", "bts", "btr", "btc"):
        bit = UNREAD_BIT_INDEX.get(imm)
        if bit:
            return "specific", mnemonic, bit
        return None
    if mnemonic == "and":
        return _and_touch(imm)
    if mnemonic not in ("test", "or", "xor", "push"):
        return None
    if imm > 0xFFFF:
        # Addresses and other large constants. A flag push is a 16-bit value;
        # an address that happens to have bit 10 set is not a test of 0x400.
        return None
    bits = imm & UNREAD_CELL_MASK
    if not bits:
        return None
    if imm in WIDTH_MASKS:
        return "width", mnemonic, bits
    action = {"or": "set", "xor": "xor", "test": "test", "push": "push"}[mnemonic]
    return "specific", action, bits


def _raw_e8(code: bytes, base: int, target: int):
    """Every E8 rel32 whose target is `target`, decoded or not.

    An E8 byte inside another instruction is a false hit. The report compares
    this list with the calls the linear sweep actually decoded.
    """
    sites = []
    i = 0
    n = len(code)
    while True:
        j = code.find(b"\xe8", i)
        if j < 0 or j + 5 > n:
            break
        rel = struct.unpack_from("<i", code, j + 1)[0]
        if base + j + 5 + rel == target:
            sites.append(base + j)
        i = j + 1
    return sites


def _push_before(insns, index: int):
    """The immediate pushed for this call, if the window holds one.

    Walks back at most eight instructions and stops at a call, jump or ret.
    Accepts `push imm` or `mov reg, imm` followed by `push reg`.
    """
    for k in range(index - 1, max(-1, index - 8), -1):
        _addr, _sz, mn, op = insns[k]
        if mn in ("call", "jmp", "ret"):
            return None
        if mn == "push":
            imm = _imm(op)
            if imm is None:
                return None
            return _addr, imm
        if mn == "mov":
            imm = _imm(op)
            dest = _dest(op)
            if imm is None or dest not in REG32:
                continue
            for t in range(k + 1, index):
                if insns[t][2] == "push" and insns[t][3].strip() == dest:
                    return _addr, imm
    return None


def _decode_imm32(code: bytes, base: int, pattern: bytes, expected: int):
    """Instructions whose immediate is the 4 bytes at each `pattern` hit.

    Independent of the linear sweep: for each occurrence, try decoding an
    instruction that starts up to 7 bytes earlier and consumes the pattern.
    This is what makes a zero result auditable when the sweep might have
    desynced.
    """
    md = _capstone()
    found = []
    seen = set()
    start = 0
    while True:
        j = code.find(pattern, start)
        if j < 0:
            break
        start = j + 1
        for back in range(0, 8):
            at = j - back
            if at < 0:
                break
            ins = next(md.disasm(code[at : at + 15], base + at), None)
            if ins is None:
                continue
            if not (ins.address <= base + j < ins.address + ins.size):
                continue
            if ins.address + ins.size < base + j + len(pattern):
                continue
            got = _imm(ins.op_str)
            if got != expected:
                continue
            if ins.mnemonic not in MASK_MNEMONICS:
                continue
            if ins.address in seen:
                break
            seen.add(ins.address)
            found.append((ins.address, ins.size, ins.mnemonic, ins.op_str))
            break
    return found


def _ptr_sites(img: Image, addr: int, limit: int = 6):
    """Dwords equal to `addr` in .rdata and .data. A vtable slot, if there is one."""
    pat = struct.pack("<I", addr)
    hits = []
    b = img.data
    pe = struct.unpack_from("<I", b, 0x3C)[0]
    coff = pe + 4
    nsec = struct.unpack_from("<H", b, coff + 2)[0]
    optsz = struct.unpack_from("<H", b, coff + 16)[0]
    opt = coff + 20
    for i in range(nsec):
        o = opt + optsz + i * 40
        name = b[o : o + 8].rstrip(b"\0").decode("latin1")
        if name not in (".rdata", ".data"):
            continue
        vsize, va, rsize, ptr = struct.unpack_from("<IIII", b, o + 8)
        blob = b[ptr : ptr + rsize]
        s = 0
        while len(hits) < limit:
            j = blob.find(pat, s)
            if j < 0:
                break
            hits.append((name, img.base + va + j))
            s = j + 1
    return hits


def report_cell_flags(img: Image) -> int:
    """Scan .text for the seven unread collision-cell bits.

    Measured, and what the scan will not pretend to have done, is printed
    ahead of any verdict. A bit is a candidate only when a specific
    test/and/or/xor/bt/push mentions it. Width masks, stack alignment and
    `test ah` after `fnstsw` are counted and then set aside: they are real
    instructions, and they are not tests of one cell flag.
    """
    (base, vsize, rsize, code, chars), sections = _text_bytes(img)
    execute = 0x20000000
    print("# Collision-cell bit scan")
    print(f"#   .text va {base:#010x}  virtual {vsize:#x}  file-backed {rsize:#x}")
    print("#   sections:")
    for name, va, vs, rs, ch in sections:
        mark = " executable" if ch & execute else ""
        print(f"#     {name:<8} va {va:#010x}  virtual {vs:#x}  raw {rs:#x}{mark}")
    print("#   searched: file-backed .text only")
    print("#   mnemonics: test, and, or, xor, push, bt, bts, btr, btc")
    print("#   immediates: imm8 and imm32, including and-eax (opcode 25) and push")
    print("#   clear-masks: and-immediate with the high half 0xFFFF, or a 16-bit")
    print("#     value that clears 1..6 bits and is not a 2^n-1 width")
    print("#   also: mov reg, imm followed within 8 insns by test/and/or/xor of that")
    print("#     reg; shr reg, <bit index> followed within 5 by test/and of 1")
    print("#   also: every direct call of the stamp 0x4311d0 and the unstamp")
    print("#     0x43127e, and the immediate pushed for it")
    print("#   not searched: test reg, reg and cmp reg, 0 (whole-word nonzero);")
    print("#     bt reg, reg (the index is not an immediate); shifts other than")
    print("#     the shr-then-test-1 pair; indirect calls; any section but .text")
    print("#   width masks 2^n-1 and `and esp/ebp` alignment are counted, not cited")
    print("#   `test ah` after fnstsw is the FPU status word, not a cell")
    print("#   an immediate on AH/BH/CH/DH is shifted left 8 before the bit test,")
    print("#     because that register is bits 8..15 (`or ah, 8` sets 0x800)")
    print()

    insns, skipped = _linear(code, base)
    decoded = sum(size for _a, size, _m, _o in insns)
    print(f"# coverage: {len(insns)} instructions, {decoded:#x} bytes decoded,")
    print(f"#   {skipped} bytes skipped, section raw {rsize:#x}")
    starts = _prologues(code, base)
    print(f"#   {len(starts)} prologues (55 8b ec on ret / int3 / hot-patch)")
    print()

    by_addr = {addr: (size, mn, op) for addr, size, mn, op in insns}
    addrs = [a for a, _s, _m, _o in insns]

    failed = False
    print("# self-check, known sites. A miss means this sweep cannot support a negative.")
    for addr, mn, imm in _KNOWN_CELL_SITES:
        got = by_addr.get(addr)
        if got is None:
            print(f"#   MISSING  {addr:08x}  {mn}")
            failed = True
            continue
        _sz, gmn, gop = got
        gimm = _imm(gop)
        ok = gmn == mn and (imm is None or gimm == imm)
        if imm is None:
            ok = ok and "ebp" in gop
        mark = "ok" if ok else "MISMATCH"
        if not ok:
            failed = True
        print(f"#   {mark:<8} {addr:08x}  {gmn} {gop}")
    print("# self-check, known prologues:")
    start_set = set(starts)
    for addr in _KNOWN_PROLOGUES:
        if addr not in start_set:
            off = addr - base
            prev = code[off - 1] if off else 0
            print(f"#   MISSING  {addr:08x}  prev byte {prev:#04x}")
            failed = True
        else:
            print(f"#   ok       {addr:08x}")
    print()

    # Direct calls, from the sweep and from a raw E8 scan.
    call_index = {}
    callers = {}
    for i, (addr, _sz, mn, op) in enumerate(insns):
        if mn != "call" or not op.startswith("0x"):
            continue
        target = int(op, 16)
        callers.setdefault(target, []).append(addr)
        call_index[addr] = i
    print("# direct calls (sweep):")
    for label, target in (
        ("probe", CELL_PROBE),
        ("stamp", CELL_STAMP),
        ("unstamp", CELL_UNSTAMP),
        ("array accessor", 0x00404D10),
    ):
        sweep_n = len(callers.get(target, ()))
        raw = _raw_e8(code, base, target)
        raw_only = [a for a in raw if a not in call_index]
        print(
            f"#   {label:<16} {target:08x}  sweep {sweep_n:<4}  "
            f"raw E8 {len(raw):<4}  raw-only {len(raw_only)}"
        )
        if raw_only and target in (CELL_PROBE, CELL_STAMP, CELL_UNSTAMP):
            shown = ", ".join(f"{a:08x}" for a in raw_only[:8])
            print(f"#     raw-only sites (E8 not decoded as this call): {shown}")
    print()

    probe_funcs = set()
    for site in callers.get(CELL_PROBE, ()):
        fn = _func_of(starts, site)
        if fn is not None:
            probe_funcs.add(fn)
    print(f"# functions that call the probe: {len(probe_funcs)}")

    # Stamp-like: a probe function that ors a stack argument into something.
    # Clear-like: a probe function that executes `not` (the unstamp does).
    stamp_like = set()
    clear_like = set()
    or_arg_sites = []
    not_sites = []
    for addr, _sz, mn, op in insns:
        fn = _func_of(starts, addr)
        if fn not in probe_funcs:
            continue
        if mn == "or" and "ebp +" in op and _imm(op) is None:
            stamp_like.add(fn)
            or_arg_sites.append((addr, op, fn))
        if mn == "not":
            clear_like.add(fn)
            not_sites.append((addr, op, fn))
    # The known writers, even if the pattern match missed them.
    if CELL_STAMP in start_set:
        stamp_like.add(CELL_STAMP)
    if CELL_UNSTAMP in start_set:
        clear_like.add(CELL_UNSTAMP)
    print(f"#   stamp-like (or from [ebp+]): {', '.join(f'{a:08x}' for a in sorted(stamp_like)) or '-'}")
    print(f"#   clear-like (not):            {', '.join(f'{a:08x}' for a in sorted(clear_like)) or '-'}")
    for addr, op, fn in or_arg_sites:
        print(f"#     {addr:08x}  or {op}   in {fn:08x}")
    for addr, op, fn in not_sites:
        print(f"#     {addr:08x}  not {op}   in {fn:08x}")
    print()

    writers = stamp_like | clear_like
    print("# arguments pushed to stamp-like and clear-like functions:")
    writer_args = []
    for target in sorted(writers):
        for site in callers.get(target, ()):
            got = _push_before(insns, call_index[site])
            if got is None:
                writer_args.append((site, target, None, None))
                print(f"#   call {site:08x}  -> {target:08x}  no immediate in the 8 insns before it")
                continue
            paddr, imm = got
            writer_args.append((site, target, paddr, imm))
            bits = imm & UNREAD_CELL_MASK
            note = f"  unread {_fmt_bits(bits)}" if bits else ""
            print(f"#   call {site:08x}  -> {target:08x}  push {paddr:08x} imm {imm:#x}{note}")
    if CELL_STAMP in start_set and not any(t == CELL_STAMP and imm == 0x200 for _s, t, _p, imm in writer_args):
        print("#   MISSING the known push 0x200 into the stamp")
        failed = True
    print()

    # Walk every mask instruction once.
    specific = []
    width_probe = []
    partial_probe = []
    partial_outside = {b: 0 for b in UNREAD_CELL_BITS}
    fpu = 0
    high8_shifted = 0
    align_n = 0
    large_hits = []
    combo_sites = {c: [] for c in _COMBO_IMMS}
    for i, (addr, size, mn, op) in enumerate(insns):
        if mn not in MASK_MNEMONICS:
            continue
        imm = _imm(op)
        if imm is None:
            continue
        dest = _dest(op)
        # Count the combination on the encoded immediate, before the AH shift.
        if imm in combo_sites and mn in ("test", "and", "or", "xor", "push"):
            combo_sites[imm].append(addr)
        if dest in HIGH8 and imm <= 0xFF:
            imm = imm << 8
            high8_shifted += 1
        prev = insns[i - 1][2] if i else ""
        if dest == "ah" and prev == "fnstsw":
            fpu += 1
            continue
        if mn == "and" and dest in ("esp", "ebp"):
            align_n += 1
            continue
        touched = _touch(mn, imm)
        if touched is None and imm > 0xFFFF and mn in ("test", "or", "xor", "push"):
            low = imm & 0xFFFF
            if low in UNREAD_CELL_BITS and not (0x400000 <= imm <= 0x5E8000):
                large_hits.append((addr, mn, op, imm))
            continue
        if touched is None:
            continue
        channel, action, bits = touched
        fn = _func_of(starts, addr)
        # AL/BL/CL/DL are bits 0..7 of the cell word. AH and the 16-bit
        # forms are not, so those stay out of the cited list.
        partial = dest in ("ah", "bh", "ch", "dh") or dest in REG16
        rec = (addr, size, mn, op, action, bits, imm, fn, partial)
        if channel == "alignment":
            align_n += 1
            continue
        if channel == "width":
            if fn in probe_funcs:
                width_probe.append(rec)
            continue
        if partial and dest not in ("al", "bl", "cl", "dl"):
            # AX/AH and the other high or 16-bit forms are not the cell dword.
            # AL is bits 0..7, so `or al, 0x40` is kept as a real bit set.
            if fn in probe_funcs:
                partial_probe.append(rec)
            else:
                for b in UNREAD_CELL_BITS:
                    if bits & b:
                        partial_outside[b] += 1
            continue
        specific.append(rec)

    # mov reg, imm  /  test-or-and that reg. And shr-then-test-1.
    forwarded = []
    for i, (addr, size, mn, op) in enumerate(insns):
        if mn == "mov":
            imm = _imm(op)
            dest = _dest(op)
            if imm is None or dest not in REG32 or imm > 0xFFFF:
                continue
            if not (imm & UNREAD_CELL_MASK) or _popcount(imm) > 8 or imm in WIDTH_MASKS:
                continue
            for nxt in insns[i + 1 : i + 8]:
                if nxt[2] in ("call", "jmp", "ret"):
                    break
                if nxt[2] not in ("test", "and", "or", "xor"):
                    continue
                parts = [p.strip() for p in nxt[3].split(",")]
                if len(parts) == 2 and parts[1] == dest and _imm(nxt[3]) is None:
                    fn = _func_of(starts, addr)
                    forwarded.append((addr, imm, nxt[0], nxt[2], nxt[3], fn))
                    break
        if mn == "shr":
            imm = _imm(op)
            dest = _dest(op)
            bit = UNREAD_BIT_INDEX.get(imm) if imm is not None else None
            if bit is None or dest not in REG32:
                continue
            for nxt in insns[i + 1 : i + 6]:
                if nxt[2] in ("call", "jmp", "ret"):
                    break
                if nxt[2] in ("test", "and") and _imm(nxt[3]) == 1 and _dest(nxt[3]) == dest:
                    fn = _func_of(starts, addr)
                    forwarded.append((addr, bit, nxt[0], nxt[2], nxt[3], fn))
                    break

    print(f"# set aside: {fpu} fnstsw/test-ah, {align_n} esp/ebp or low-run alignment")
    print(f"#   {high8_shifted} AH/BH/CH/DH immediates shifted left 8 before the bit test")
    print(f"# specific hits (unread bit in the immediate): {len(specific)}")
    print(f"# width-mask hits inside probe functions: {len(width_probe)}")
    print(f"# partial-register hits inside probe functions: {len(partial_probe)}")
    print(f"# reg-forwarded or shr-then-test-1: {len(forwarded)}")
    if large_hits:
        print(f"# large non-address immediates whose low 16 is exactly one unread bit: {len(large_hits)}")
        for addr, mn, op, imm in large_hits[:12]:
            print(f"#   {addr:08x}  {mn} {op}")
    print()

    print("# combination immediates (exact value, any test/and/or/xor/push):")
    for imm, label in _COMBO_IMMS.items():
        sites = combo_sites[imm]
        shown = ", ".join(f"{a:08x}" for a in sites[:8])
        print(f"#   {imm:#06x}  {label:<20}  {len(sites)}  {shown}")
    print()

    # Per-bit verdict. A hit counts as cell-adjacent when its function calls the
    # probe, when a direct caller of that function calls the probe, or when the
    # hit is a push into a stamp-like / clear-like writer.
    def _from_probe(fn):
        if fn is None:
            return False
        if fn in probe_funcs:
            return True
        for site in callers.get(fn, ()):
            if _func_of(starts, site) in probe_funcs:
                return True
        return False

    writer_imms_by_bit = {b: [] for b in UNREAD_CELL_BITS}
    for site, target, paddr, imm in writer_args:
        if imm is None:
            continue
        for b in UNREAD_CELL_BITS:
            if imm & b:
                writer_imms_by_bit[b].append((site, target, paddr, imm))

    print("# verdict")
    for bit in UNREAD_CELL_BITS:
        cellish = []
        other = []
        for rec in specific:
            if not (rec[5] & bit):
                continue
            # A push of 0x18 or 0x28 is a size that happens to contain the bit.
            # Pushes into the stamp are counted separately, above.
            if rec[4] == "push":
                other.append(rec)
                continue
            if _from_probe(rec[7]):
                cellish.append(rec)
            else:
                other.append(rec)
        for addr, imm, naddr, nmn, nop, fn in forwarded:
            if imm & bit and _from_probe(fn):
                cellish.append((addr, 0, "fwd", f"{nmn} {nop}", "fwd", bit, imm, fn, False))
        pushed = writer_imms_by_bit[bit]
        if cellish or pushed:
            print(f"#   {bit:#06x}  CANDIDATE  cell-adjacent {len(cellish)}  writer-push {len(pushed)}  other {len(other)}")
        elif other:
            extra = partial_outside[bit]
            tail = f"  plus {extra} partial-reg outside probe functions" if extra else ""
            print(
                f"#   {bit:#06x}  NOT A CELL TEST  {len(other)} other immediate(s), "
                f"none cell-adjacent{tail}"
            )
        else:
            extra = partial_outside[bit]
            tail = f"  ({extra} partial-reg outside probe functions, not cited)" if extra else ""
            print(f"#   {bit:#06x}  NO SPECIFIC IMMEDIATE in the forms above{tail}")
    print()

    def _show(rec, lead="   "):
        addr, size, mn, op, action, bits, imm, fn, partial = rec
        raw = img.read(addr, size).hex(" ") if size else ""
        who = f"{fn:08x}" if fn else "no-prologue"
        part = "  partial-reg" if partial else ""
        near = "  probe-func" if fn in probe_funcs else ""
        if not near and _from_probe(fn):
            near = "  called-from-probe"
        print(f"{lead}{addr:08x}  {raw:<24}  {mn:<5} {op}")
        print(f"{lead}  {action} imm {imm:#x}  bits {_fmt_bits(bits)}  fn {who}{near}{part}")
        i = bisect.bisect_left(addrs, addr)
        for paddr, _psz, pmn, pop in insns[max(0, i - 4) : i]:
            print(f"{lead}    {paddr:08x}  {pmn:<6} {pop}")
        e8 = callers.get(fn, ()) if fn else ()
        if e8:
            shown = ", ".join(f"{a:08x}" for a in e8[:8])
            more = f" (+{len(e8) - 8})" if len(e8) > 8 else ""
            print(f"{lead}  direct callers: {shown}{more}")
        elif fn and action in ("keep", "test", "set", "clear", "bt", "bts", "btr", "btc"):
            ptrs = _ptr_sites(img, fn)
            if ptrs:
                shown = ", ".join(f"{n}:{a:08x}" for n, a in ptrs)
                print(f"{lead}  no E8 caller; dword sites: {shown}")
            else:
                print(f"{lead}  no E8 caller; no dword in .rdata/.data")

    print("# cell-adjacent specific hits (push immediates omitted; stamp pushes are above):")
    any_cell = False
    for rec in specific:
        if rec[8] or rec[4] == "push":
            continue
        if _from_probe(rec[7]):
            _show(rec)
            any_cell = True
    if not any_cell:
        print("    (none)")
    print()
    if forwarded:
        print("# reg-forwarded masks and shr-then-test-1:")
        for addr, imm, naddr, nmn, nop, fn in forwarded:
            mark = "probe" if _from_probe(fn) else "other"
            print(f"    {addr:08x}  imm {imm:#x}  then {naddr:08x} {nmn} {nop}  ({mark})")
        print()
    if partial_probe:
        print("# partial-register hits inside probe functions (not cited):")
        for rec in partial_probe:
            _show(rec)
        print()
    if width_probe:
        print("# width-mask hits inside probe functions (not cited):")
        for rec in width_probe[:40]:
            addr, _sz, mn, op, action, bits, imm, fn, _p = rec
            print(f"    {addr:08x}  {mn} {op}  {action} {_fmt_bits(bits)}  fn {fn:08x}")
        if len(width_probe) > 40:
            print(f"    ... {len(width_probe) - 40} more")
        print()

    print("# other specific hits, grouped (not cell-adjacent):")
    groups = {}
    for rec in specific:
        if rec[8] or _from_probe(rec[7]):
            continue
        key = (rec[2], rec[6])
        groups.setdefault(key, []).append(rec)
    if not groups:
        print("    (none)")
    for key in sorted(groups, key=lambda k: (k[1], k[0])):
        recs = groups[key]
        mn, imm = key
        examples = ", ".join(f"{r[0]:08x}" for r in recs[:6])
        print(f"    {mn} {imm:#x}   {len(recs)}   e.g. {examples}")
    print()

    # Byte-pattern cross-check. Imm8 forms are covered by the sweep; the known
    # imm8 sites in the self-check are what says that coverage is real.
    print("# imm32 byte-pattern cross-check (independent of the linear sweep):")
    sweep_specific_addrs = {r[0] for r in specific}
    for bit in UNREAD_CELL_BITS:
        patterns = [
            (struct.pack("<I", bit), bit, "imm"),
            (struct.pack("<I", (~bit) & 0xFFFFFFFF), (~bit) & 0xFFFFFFFF, "clear32"),
            (struct.pack("<I", (~bit) & 0xFFFF), (~bit) & 0xFFFF, "clear16"),
        ]
        for pattern, expected, label in patterns:
            decoded_hits = _decode_imm32(code, base, pattern, expected)
            # A decoded instruction the sweep also has is not a miss. One the
            # sweep decoded at that address but did not classify as specific
            # is reported, so a filter bug is visible.
            extra = [h for h in decoded_hits if h[0] not in sweep_specific_addrs]
            if not decoded_hits:
                print(f"#   {bit:#06x}  {label:<8}  pattern {pattern.hex(' ')}  0 instructions")
                continue
            print(
                f"#   {bit:#06x}  {label:<8}  {len(decoded_hits)} instruction(s), "
                f"{len(extra)} not in the specific-hit list"
            )
            for addr, size, mn, op in extra[:8]:
                cover = None
                for saddr, ssize, smn, sop in insns:
                    if saddr <= addr < saddr + ssize:
                        cover = (saddr, smn, sop)
                        break
                if addr in by_addr:
                    why = "sweep has this address"
                elif cover is not None:
                    # The backward decode started mid-instruction. The sweep's
                    # instruction is the one reached from the section entry.
                    why = f"misaligned under {cover[0]:08x} {cover[1]} {cover[2]}"
                else:
                    why = "SWEEP MISSED"
                    failed = True
                print(f"#     {why}  {addr:08x}  {mn} {op}")
    print()
    if failed:
        print("# SELF-CHECK FAILED. Do not cite a negative from this run.")
        return 1
    print("# self-check passed.")
    return 0


def _fmt_bits(mask: int) -> str:
    names = [f"{b:#x}" for b in UNREAD_CELL_BITS if mask & b]
    return ",".join(names) if names else "-"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--exe", help="path to the stock Lemmings Revolution.exe")
    ap.add_argument("--states", action="store_true", help="the state map, and stop")
    ap.add_argument("--at", help="disassemble this address instead of the steppers")
    ap.add_argument("--len", default="0x120", help="bytes to read with --at or --consts")
    ap.add_argument("--consts", help="list the .rdata doubles this function loads")
    ap.add_argument(
        "--cell-flags",
        action="store_true",
        help="scan .text for the seven unread collision-cell bits",
    )
    ap.add_argument("--no-digest-check", action="store_true")
    args = ap.parse_args()

    exe = locate_exe(args.exe)
    if not exe.exists():
        print(f"not found: {exe}\npass --exe or set WEBOLUTION_GAME_DIR", file=sys.stderr)
        return 2
    img = Image(exe)

    if not args.no_digest_check:
        import hashlib

        got = hashlib.sha256(img.data).hexdigest()
        if got != EXPECTED_SHA256:
            print(
                f"{exe}\n  sha256 {got}\n  expected {EXPECTED_SHA256}\n"
                "  Every address in PLAN.md 2.3 belongs to that build. Re-run with\n"
                "  --no-digest-check only to look around; do not quote the result.",
                file=sys.stderr,
            )
            return 2

    print(f"# {exe}")
    print(f"# image base {img.base:#010x}")
    for name, va, size, ptr in img.sections:
        print(f"#   {name:<8} va {va:#010x}  size {size:#x}  file offset {ptr:#x}")
    print()

    if args.cell_flags:
        return report_cell_flags(img)

    if args.at:
        disassemble(img, int(args.at, 16), int(args.len, 16))
        return 0

    if args.consts:
        for addr, mn, ea, v in constants(img, int(args.consts, 16), int(args.len, 16)):
            print(f"    {addr:08x}  {mn:<6} [{ea:08x}] = {v!r}")
        return 0

    report_states(img)
    if args.states:
        return 0

    for name, vt, stepper, length in STEPPERS:
        slot1 = img.u32(vt + 4)
        if slot1 != stepper:
            print(f"!! {name}: vtable {vt:#010x} slot 1 is {slot1:#010x}, expected {stepper:#010x}")
            continue
        print(f"===== {name}: vtable {vt:#010x} slot 1 -> {stepper:#010x} =====")
        disassemble(img, stepper, length, stop_at_ret=False)
        print()

    print("===== the ordinary faller's speed accessor: 0x00416038 =====")
    disassemble(img, 0x00416038, 0x4A)
    print()

    report_speeds(img)
    print()
    report_skills(img)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
