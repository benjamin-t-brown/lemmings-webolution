"""Run verified actor movement and two-pass extra-state cleanup bytes."""

import argparse
import hashlib
from pathlib import Path
import struct
import subprocess
import sys

from verify_reference import EXE_SHA256, FUNCTIONS, pe_file_offset

PINS = (
    (0x0042E725, 101, "c5cf8bc9b8e06c427119c28fe34ca99f56383d36896ff33105f0297a01841942"),
    (0x0042EE74, 47, "4df50eff7bce9fd5c64817e772f5a5629c8c855b882dcba4557dd4e3a9b1111b"),
    (0x0042E78A, 105, "c9d0af90fd6a61c55724341da8e80c9da60674fa546f78b216b4f03adc4f9ef7"),
    (0x004302B0, 187, "f85bf9bf209b039ea65fc25c49f9c2718b8991e2f99f44446c1a77feb3673dd1"),
)
HELPERS = (0x0040DF50, 0x0040BAA0, 0x00430170, 0x00430190,
           0x0042E7F3, 0x0042ED4D, 0x004301B0, 0x0042FEF0, 0x00430A70)


def main() -> int:
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
            parser.error(f"0x{address:08X} bytes do not match the reference")
        blocks.append(struct.pack("<II", address, length) + body)
    offset = pe_file_offset(data, 0x004B0338, 8)
    zero = data[offset:offset + 8]
    if zero != bytes(8):
        parser.error("zero constant mismatch")
    blocks.append(struct.pack("<II", 0x004B0338, 8) + zero)
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
