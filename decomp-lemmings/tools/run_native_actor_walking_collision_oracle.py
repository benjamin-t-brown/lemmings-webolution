"""Run original walking collision and geometry helpers with declared boundary adapters."""

import argparse
import hashlib
from pathlib import Path
import struct
import subprocess
import sys

from verify_reference import EXE_SHA256, FUNCTIONS, pe_file_offset


COLLISION = (0x0041BFFC, 2169, "837e048169e5423ee649ff92a802ab611cc029b1802c81451360e9d3aadcb3fd")
HELPERS = (0x42F22E, 0x4066A0, 0x403EB0, 0x4017F0, 0x404D10, 0x40DE60,
           0x40DEC0, 0x40DF30, 0x40DF50, 0x40BAA0, 0x49C5AC, 0x49C8F4,
           0x49FC56, 0x42F66B)
CONSTANTS = ((0x4B0338, "0000000000000000"), (0x4B0698, "000000e08e79453e"),
             (0x4B07A8, "0000000000007040"))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--program", type=Path, required=True)
    args = parser.parse_args()
    data = args.exe.read_bytes()
    if hashlib.sha256(data).hexdigest() != EXE_SHA256:
        parser.error("reference executable SHA-256 mismatch")
    pins = [COLLISION]
    for address in HELPERS:
        _, _, length, digest = next(row for row in FUNCTIONS if row[0] == address)
        pins.append((address, length, digest))
    blocks = []
    for address, length, digest in pins:
        offset = pe_file_offset(data, address, length)
        body = data[offset:offset + length]
        if hashlib.sha256(body).hexdigest() != digest:
            parser.error(f"0x{address:08X} bytes do not match the reference")
        blocks.append(struct.pack("<II", address, length) + body)
    for address, expected in CONSTANTS:
        offset = pe_file_offset(data, address, 8)
        body = data[offset:offset + 8]
        if body.hex() != expected:
            parser.error(f"0x{address:08X} constant mismatch")
        blocks.append(struct.pack("<II", address, 8) + body)
    payload = struct.pack("<I", len(blocks)) + b"".join(blocks)
    for _ in range(8):
        result = subprocess.run([str(args.program)], input=payload, capture_output=True,
                                check=False, timeout=30)
        if result.returncode != 3:
            sys.stdout.buffer.write(result.stdout)
            sys.stderr.buffer.write(result.stderr)
            return result.returncode
    sys.stderr.buffer.write(result.stderr)
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
