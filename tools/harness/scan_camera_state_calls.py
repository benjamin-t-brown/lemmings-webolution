"""Conservative raw-call inventory for the stock camera depth investigation.

Every byte in file-backed executable sections is considered as an instruction
start. Results include false positives and other interfaces. A nearby global
pointer literal is a review aid, not proof of receiver identity or arguments.
Register-indirect calls and nonliteral slot selection remain outside this scan.
Requires Capstone. No target execution or modification occurs.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

from capstone import Cs, CS_ARCH_X86, CS_MODE_32, CS_OP_MEM


def scan(path: Path) -> dict:
    data = path.read_bytes()
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    if data[:2] != b"MZ" or data[pe:pe + 4] != b"PE\0\0":
        raise ValueError("not a PE executable")
    opt = pe + 24
    if struct.unpack_from("<H", data, opt)[0] != 0x10B:
        raise ValueError("this audit expects the stock PE32 format")
    base = struct.unpack_from("<I", data, opt + 28)[0]
    table = opt + struct.unpack_from("<H", data, pe + 20)[0]
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    coverage, records = [], []
    for section in range(struct.unpack_from("<H", data, pe + 6)[0]):
        pos = table + 40 * section
        name = data[pos:pos + 8].rstrip(b"\0").decode("ascii")
        rva, size, offset = struct.unpack_from("<III", data, pos + 12)
        flags = struct.unpack_from("<I", data, pos + 36)[0]
        if not flags & 0x20000000:
            continue
        code = data[offset:offset + size]
        if len(code) != size:
            raise ValueError("truncated executable section")
        coverage.append({"section": name, "va": hex(base + rva), "raw_bytes": size})
        for index, byte in enumerate(code):
            if byte != 0xFF:
                continue
            ins = next(decoder.disasm(code[index:index + 15], base + rva + index, count=1), None)
            if not ins or ins.mnemonic != "call" or len(ins.operands) != 1:
                continue
            operand = ins.operands[0]
            if operand.type != CS_OP_MEM:
                continue
            mem = operand.mem
            if mem.disp != 0x58 or not mem.base or mem.index:
                continue
            start = max(0, index - 100)
            before = code[start:index]
            records.append({
                "va": hex(ins.address), "bytes": ins.bytes.hex(), "operands": ins.op_str,
                "context_start": hex(base + rva + start), "context_hex": before.hex(),
                "near_global_device_literal": struct.pack("<I", 0x58CDB8) in before,
            })
    return {"executable_sha256": hashlib.sha256(data).hexdigest(), "coverage": coverage,
            "scope": __doc__, "calls": records}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, default=Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe"))
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    report = scan(args.exe)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(report, indent=2))
        print(json.dumps({"output": str(args.output), "candidates": len(report["calls"])}))
    else:
        print(json.dumps(report, indent=2))
