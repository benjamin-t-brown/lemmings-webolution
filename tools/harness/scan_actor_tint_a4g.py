#!/usr/bin/env python3
"""A4g: scan PE .text for stores to actor draw-object vertex RGB fields."""
from __future__ import annotations

import hashlib
import json
import struct
import sys
from pathlib import Path

import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86_const import X86_OP_MEM, X86_OP_REG

EXPECTED = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"

# Draw object inline vertex records: base object+0x08, stride 0x68.
# RGB doubles at record+0x38/+0x40/+0x48 (alpha +0x50).
RGB_OBJECT_DISPS = (
    0x40,
    0x48,
    0x50,
    0xA8,
    0xB0,
    0xB8,
    0x110,
    0x118,
    0x120,
    0x178,
    0x180,
    0x188,
)
# Same fields expressed relative to each record base (loop-friendly).
RGB_RECORD_DISPS = (0x38, 0x40, 0x48)
STRIDE_IMM = 0x68

WRITE_MNEMS = frozenset(
    {
        "mov",
        "movsd",
        "movss",
        "fstp",
        "fistp",
        "fstp",
        "rep movsd",
        "rep movsb",
        "rep stosd",
        "stosd",
        "xchg",
    }
)
NOT_WRITE = frozenset({"cmp", "test", "push", "call", "jmp", "je", "jne", "jg", "jl", "ja", "jb", "jbe", "jae", "jz", "jnz"})


def is_write(ins) -> bool:
    if ins.mnemonic in NOT_WRITE:
        return False
    if ins.mnemonic in ("fstp", "fistp", "movsd", "movss", "stosd"):
        return True
    if ins.mnemonic == "mov" and ins.operands and ins.operands[0].type == X86_OP_MEM:
        return True
    if ins.mnemonic == "rep movsd":
        return True
    return False


def mem_disp(op) -> int | None:
    if op.type != X86_OP_MEM:
        return None
    return op.mem.disp & 0xFFFFFFFF


def classify_hit(ins, disp: int) -> str:
    if disp in RGB_OBJECT_DISPS:
        return f"object+0x{disp:x}"
    if disp in RGB_RECORD_DISPS:
        return f"record+0x{disp:x}"
    return f"disp+0x{disp:x}"


def scan_text(data: bytes, pe: pefile.PE) -> list[dict]:
    section = next(s for s in pe.sections if s.Name.rstrip(b"\0") == b".text")
    raw_start = section.PointerToRawData
    raw_end = raw_start + section.SizeOfRawData
    base = pe.OPTIONAL_HEADER.ImageBase + section.VirtualAddress
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    hits: list[dict] = []
    for ins in md.disasm(data[raw_start:raw_end], base):
        if not is_write(ins):
            continue
        for i, op in enumerate(ins.operands):
            if i != 0 and ins.mnemonic not in ("fstp", "fistp", "movsd", "movss"):
                continue
            d = mem_disp(op)
            if d is None:
                continue
            if d in RGB_OBJECT_DISPS or d in RGB_RECORD_DISPS:
                hits.append(
                    {
                        "va": ins.address,
                        "bytes": ins.bytes.hex(),
                        "mnemonic": ins.mnemonic,
                        "op_str": ins.op_str,
                        "field": classify_hit(ins, d),
                        "disp": d,
                    }
                )
    return hits


def scan_255_literal_copies(data: bytes, pe: pefile.PE) -> list[dict]:
    """Find push 0x406fe000 used outside 0x440A10 constructor block."""
    section = next(s for s in pe.sections if s.Name.rstrip(b"\0") == b".text")
    raw = data[section.PointerToRawData : section.PointerToRawData + section.SizeOfRawData]
    base = pe.OPTIONAL_HEADER.ImageBase + section.VirtualAddress
    pattern = bytes([0x68, 0x00, 0xE0, 0x6F, 0x40])
    out = []
    off = 0
    while True:
        idx = raw.find(pattern, off)
        if idx < 0:
            break
        va = base + idx
        if not (0x440ABC <= va <= 0x440B75):
            out.append({"va": va, "bytes": pattern.hex(), "note": "push 255.0 double high dword"})
        off = idx + 1
    return out


def main() -> int:
    exe = Path(
        sys.argv[1] if len(sys.argv) > 1 else r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe"
    )
    out_dir = Path(sys.argv[2] if len(sys.argv) > 2 else "build/tmp/a4g-actor-tint-writer")
    out_dir.mkdir(parents=True, exist_ok=True)

    data = exe.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if digest != EXPECTED:
        raise SystemExit(f"wrong authority digest: {digest}")

    pe = pefile.PE(data=data, fast_load=True)
    hits = scan_text(data, pe)
    pushes = scan_255_literal_copies(data, pe)

    # Dedupe by va+field
    seen = set()
    unique = []
    for h in hits:
        key = (h["va"], h["disp"])
        if key in seen:
            continue
        seen.add(key)
        unique.append(h)
    unique.sort(key=lambda x: (x["va"], x["disp"]))

    # Direct call sites to the bulk RGB setter (e8 rel32 to 0x406260).
    target = 0x406260
    call_sites: list[dict] = []
    section = next(s for s in pe.sections if s.Name.rstrip(b"\0") == b".text")
    raw = data[section.PointerToRawData : section.PointerToRawData + section.SizeOfRawData]
    base = pe.OPTIONAL_HEADER.ImageBase + section.VirtualAddress
    for idx in range(len(raw) - 5):
        if raw[idx] != 0xE8:
            continue
        rel = int.from_bytes(raw[idx + 1 : idx + 5], "little", signed=True)
        callee = base + idx + 5 + rel
        if callee != target:
            continue
        va = base + idx
        call_sites.append({"va": va, "bytes": raw[idx : idx + 5].hex()})
    call_sites.sort(key=lambda x: x["va"])

    report = {
        "sha256": digest,
        "rgb_hits": unique,
        "call_406260": call_sites,
        "push_255_outside_ctor": pushes,
        "ctor_range_excluded": "0x440A10..0x440BE4 initializes RGB to 255",
    }
    out_json = out_dir / "scan-hits.json"
    out_json.write_text(json.dumps(report, indent=2), encoding="utf-8")

    print(f"sha256={digest}")
    print(f"rgb_field_store_hits={len(unique)}")
    for h in unique:
        print(
            f"0x{h['va']:08x} {h['bytes']} {h['mnemonic']} {h['op_str']} ; {h['field']}"
        )
    print(f"push_255_elsewhere={len(pushes)}")
    for p in pushes:
        print(f"0x{p['va']:08x} {p['bytes']} ; {p['note']}")
    print(f"wrote {out_json}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
