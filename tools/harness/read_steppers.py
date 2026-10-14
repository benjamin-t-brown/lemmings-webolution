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

The exe path defaults to the retail install and can be overridden with `--exe`
or `WEBOLUTION_GAME_DIR`.
"""

from __future__ import annotations

import argparse
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


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--exe", help="path to the stock Lemmings Revolution.exe")
    ap.add_argument("--states", action="store_true", help="the state map, and stop")
    ap.add_argument("--at", help="disassemble this address instead of the steppers")
    ap.add_argument("--len", default="0x120", help="bytes to read with --at or --consts")
    ap.add_argument("--consts", help="list the .rdata doubles this function loads")
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
