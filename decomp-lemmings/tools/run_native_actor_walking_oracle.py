"""Compare original walking routines with the recovered source and adapted effects."""
import argparse
import hashlib
from pathlib import Path
import struct
import subprocess
import sys
from verify_reference import EXE_SHA256, FUNCTIONS, pe_file_offset

PINS = (
    (0x41BDD4, 397, "cba6f8e3d252cf51973ff1b03314e897f6d87bbf1091cf61045e426095364a3f"),
    (0x41BF61, 155, "efe9de72310047c149230e15bfb60288411eb328ccc26c0c95a07fc0a3faf415"),
    (0x41BDBA, 26, "3f6b5071b426e3f9c52b6a6b01178c429de0bf7aa21f512430c91a6649b4e9dd"),
)
HELPERS = (0x40DEC0, 0x40DF30, 0x49C5AC, 0x49C8F4, 0x49FC56)
CONSTANTS = ((0x4B0338, "0000000000000000"), (0x4B0698, "000000e08e79453e"),
             (0x4B07A8, "0000000000007040"), (0x4B10B0, "0000000000001440"),
             (0x4B03F8, "0000000000000040"), (0x4B02D0, "000000000000f03f"),
             (0x4BF65C, "77616c6b00"))

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--program", type=Path, required=True)
    args = parser.parse_args()
    data = args.exe.read_bytes()
    if hashlib.sha256(data).hexdigest() != EXE_SHA256:
        parser.error("reference executable SHA-256 mismatch")
    pins = list(PINS)
    for address in HELPERS:
        _, _, length, digest = next(row for row in FUNCTIONS if row[0] == address)
        pins.append((address, length, digest))
    blocks = []
    for address, length, digest in pins:
        offset = pe_file_offset(data, address, length)
        body = data[offset:offset + length]
        if hashlib.sha256(body).hexdigest() != digest:
            parser.error(f"0x{address:08X} bytes mismatch")
        blocks.append(struct.pack("<II", address, length) + body)
    for address, expected in CONSTANTS:
        length = len(expected) // 2
        offset = pe_file_offset(data, address, length)
        body = data[offset:offset + length]
        if body.hex() != expected:
            parser.error(f"0x{address:08X} constant mismatch")
        blocks.append(struct.pack("<II", address, length) + body)
    payload = struct.pack("<I", len(blocks)) + b"".join(blocks)
    for _ in range(8):
        result = subprocess.run([str(args.program)], input=payload, capture_output=True, timeout=30)
        if result.returncode != 3:
            sys.stdout.buffer.write(result.stdout)
            sys.stderr.buffer.write(result.stderr)
            return result.returncode
    return result.returncode

if __name__ == "__main__":
    raise SystemExit(main())
