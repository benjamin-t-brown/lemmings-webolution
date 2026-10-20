#!/usr/bin/env python3
"""A4f: scan initialized PE .text for actor/sprite object field accesses."""
from __future__ import annotations
import hashlib
import sys
from pathlib import Path

import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86_const import X86_OP_MEM, X86_OP_IMM

EXPECTED = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
OFFSETS = {0x118, 0x320, 0x328, 0x330, 0x340}
VTABLE = 0x004B1AD8


def main() -> int:
    path = Path(sys.argv[1] if len(sys.argv) > 1 else r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")
    data = path.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if digest != EXPECTED:
        raise SystemExit(f"wrong authority digest: {digest}")
    pe = pefile.PE(data=data, fast_load=True)
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    section = next(s for s in pe.sections if s.Name.rstrip(b"\0") == b".text")
    raw_start, raw_end = section.PointerToRawData, section.PointerToRawData + section.SizeOfRawData
    if raw_end > len(data):
        raise SystemExit(".text raw bytes exceed file")
    base = pe.OPTIONAL_HEADER.ImageBase + section.VirtualAddress
    print(f"sha256={digest} .text raw=[0x{raw_start:x},0x{raw_end:x}) va=0x{base:x} size=0x{section.SizeOfRawData:x}")
    for ins in md.disasm(data[raw_start:raw_end], base):
        interesting = []
        for i, op in enumerate(ins.operands):
            if op.type == X86_OP_MEM and op.mem.disp in OFFSETS:
                mode = "write?" if i == 0 and ins.mnemonic not in ("cmp", "test", "push", "call", "jmp") else "read/ref"
                interesting.append(f"{mode} [base+0x{op.mem.disp:x}]")
            if op.type == X86_OP_IMM and (op.imm & 0xffffffff) == VTABLE:
                interesting.append("vtable-immediate")
        if interesting:
            print(f"0x{ins.address:08x} {ins.bytes.hex()} {ins.mnemonic} {ins.op_str} ; {', '.join(interesting)}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
