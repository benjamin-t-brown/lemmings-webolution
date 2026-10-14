"""Read the lemming state steppers and their speed constants out of the exe.

This is the re-runnable form of the static read in PLAN.md 2.3 -- the one that
produced `Rules::fallSpeed`. It needs nothing but the shipped executable: no
Ghidra, no REA, no running game. REA was used for exactly one thing, `xrefs` on
`0x004137CA`, and the answer it gave is the reason this script exists rather
than a decompiler transcript: the only reference to the climb stepper is
`0x004B0964`, which is in `.rdata`. It is a vtable slot. There is no call site
to decompile, so the way to the fall path is through the vtable, and that is a
plain read of the file.

What it does, in order:

  1. Maps the PE sections, so addresses can be turned into file offsets.
  2. Finds the vtable starts by scanning `.text` for immediates that land in
     `.rdata`. That is what separates a vtable from the double literals the
     compiler parks between them -- both are referenced, but a vtable start is
     referenced from exactly two places, its constructor and its destructor.
  3. Names the states. Slot 2 of each vtable is a name getter with a fixed
     prologue, `55 8b ec 51 89 4d fc 68 <str>`, so the string is readable
     straight out of the bytes. `0x004137AB` pushes "climb", which is what
     identified slot 2 and therefore the whole 6-slot layout.
  4. Disassembles slot 1 -- the per-tick stepper -- for walk, climb and fall,
     and annotates every `.rdata` memory operand with the double it holds.

The point of (4) is the comparison. The three steppers are the same
instructions; only the speed constant differs, and the constant is world units
per *second*. The walk's 5.0 and the climb's 5.0 reproduce two independently
measured per-tick values exactly (1 column, 1 world unit, at the measured 200
ms tick), which is what fixes the units -- and then the fall's 25.0 gives 5
world units per tick, 10 mask rows. See PLAN.md 2.3 for the addresses and for
what is still unread.

Needs `capstone` (`pip install capstone`). Usage:

    python tools/harness/read_steppers.py                     # the whole read
    python tools/harness/read_steppers.py --vtables           # just the table
    python tools/harness/read_steppers.py --at 0x41a6d5 --len 0x15a

The exe path defaults to the retail install and can be overridden with
`--exe` or `WEBOLUTION_GAME_DIR`.
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

# The three steppers PLAN.md 2.3 compares, by vtable. Slot 1 is the stepper.
STEPPERS = [
    ("walk", 0x004B0D28, 0x0041BDD4, 0x120),
    ("climb", 0x004B0960, 0x004137CA, 0x15A),
    ("fall", 0x004B0C60, 0x0041A6D5, 0x15B),
]

# Slot 2's prologue: push ebp; mov ebp,esp; push ecx; mov [ebp-4],ecx; push <str>
NAME_PROLOGUE = bytes.fromhex("558bec51894dfc68")


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

    def state_name(self, getter: int):
        """The string a slot-2 name getter pushes, if it has that shape."""
        o = self.offset(getter)
        if o is None or self.data[o : o + len(NAME_PROLOGUE)] != NAME_PROLOGUE:
            return None
        return self.cstr(struct.unpack_from("<I", self.data, o + len(NAME_PROLOGUE))[0])


def vtables(img: Image):
    """Every 6-slot state vtable, with the name its slot 2 getter yields.

    The shape is what identifies them: slots 4 and 5 are the two common base
    stubs `0x004011D0` and `0x004011E0` in all of them, and the first four
    slots point into `.text`. Scanning for that pair is more reliable than
    scanning for the 0x18 stride, because the stride breaks wherever the
    compiler emitted a double literal between two tables.
    """
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
        out.append((rdata_va + i, slots, img.state_name(slots[2])))
    return out


def disassemble(img: Image, addr: int, length: int):
    """Disassemble, annotating `.rdata` operands with the double they hold."""
    try:
        import capstone
    except ImportError:
        raise SystemExit("needs capstone: pip install capstone")
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    _n, rdata_va, rdata_size, _p = img.section(".rdata")
    absolute = re.compile(r"\[(0x4[0-9a-f]{5})\]")
    for ins in md.disasm(img.read(addr, length), addr):
        note = ""
        m = absolute.search(ins.op_str)
        if m:
            ea = int(m.group(1), 16)
            if rdata_va <= ea < rdata_va + rdata_size:
                raw = img.read(ea, 8)
                note = f"   ; {raw.hex(' ')} = {img.f64(ea)!r}"
        print(f"{ins.address:08x}  {ins.bytes.hex(' '):<20}  {ins.mnemonic:<6} {ins.op_str}{note}")
        if ins.mnemonic == "ret":
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


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--exe", help="path to the stock Lemmings Revolution.exe")
    ap.add_argument("--vtables", action="store_true", help="list the state vtables and stop")
    ap.add_argument("--at", help="disassemble this address instead of the three steppers")
    ap.add_argument("--len", default="0x120", help="bytes to disassemble with --at")
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

    tables = vtables(img)
    named = [t for t in tables if t[2]]
    print(f"# {len(tables)} six-slot state vtables, {len(named)} of them named by slot 2")
    print("#   vtable      slot0 (dtor)  slot1 (step)  slot3         name")
    for va, slots, name in tables:
        if name or not args.vtables:
            mark = "  <--" if any(va == vt for _n, vt, _s, _l in STEPPERS) else ""
            print(
                f"    {va:08x}    {slots[0]:08x}      {slots[1]:08x}      "
                f"{slots[3]:08x}      {name or '-'}{mark}"
            )
    print()
    if args.vtables:
        return 0

    for name, vt, stepper, length in STEPPERS:
        slot1 = img.u32(vt + 4)
        if slot1 != stepper:
            print(f"!! {name}: vtable {vt:#010x} slot 1 is {slot1:#010x}, expected {stepper:#010x}")
            continue
        print(f"===== {name}: vtable {vt:#010x} slot 1 -> {stepper:#010x} =====")
        disassemble(img, stepper, length)
        print()

    print("# The speed constants, and the per-tick step at the measured 200 ms tick:")
    for addr, used_at, what in (
        (0x004B10B0, 0x0041BE70, "walk  (mask columns)"),
        (0x004B10B0, 0x0041386C, "climb (world units)"),
        (0x004B0C40, 0x0041A758, "fall  (world units)"),
    ):
        speed = img.f64(addr)
        print(
            f"#   {what}  {img.read(addr, 8).hex(' ')} = {speed!r} per second"
            f"  -> {speed * 0.2!r} per tick   (loaded at {used_at:#010x})"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
