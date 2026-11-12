"""Run original landing and coordinate/grid routines with explicit state adapters."""
import argparse
import hashlib
from pathlib import Path
import struct
import subprocess
import sys

from verify_reference import EXE_SHA256, FUNCTIONS, pe_file_offset

LANDING = (0x00416433, 706, "d6342397236ba119cdff7c865fce1aefeb2f10fd27028ea4760b86d339d9945f")
SELECTOR = (0x0042F8B9, 121, "7cf669715e700dd603d30fa1310673c55a7e98f22a60cd96b0dcfe27a26e8fcd")
CONSTRUCTORS = (
    (0x0040B3A0, 48, "4d91ce609376caf9a4d1b52e8178680d6a4317cb979143c19204d8fd30d1fa5f"),
    (0x0040EC7E, 48, "6eecedeb1303e6e4de0ba7303e979661fb59d1939bb6125cbef3627b5f8f4cc4"),
    (0x0041BD60, 62, "70f14575bf28f09be3fd872cd47207fcb38fd264370a6aec21ec71fd4e56ff18"),
)
HELPERS = (0x42F22E, 0x4066A0, 0x403EB0, 0x4017F0, 0x404D10, 0x40DE60,
           0x40DEC0, 0x40DF30, 0x40DF50, 0x40BAA0, 0x49C5AC, 0x49C8F4, 0x49FC56)
CONSTANTS = ((0x4B0338, "0000000000000000"), (0x4B0698, "000000e08e79453e"),
             (0x4B07A8, "0000000000007040"), (0x4B10A8, "0000000000003e40"))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--program", type=Path, required=True)
    args = parser.parse_args()
    data = args.exe.read_bytes()
    if hashlib.sha256(data).hexdigest() != EXE_SHA256:
        parser.error("reference executable SHA-256 mismatch")
    pins = [LANDING, SELECTOR, *CONSTRUCTORS]
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
