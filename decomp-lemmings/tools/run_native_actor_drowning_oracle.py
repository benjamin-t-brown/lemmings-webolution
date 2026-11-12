"""Run verified drowning constructor/entry/update with animation/model adapters."""
import argparse
import hashlib
from pathlib import Path
import struct
import subprocess
import sys
from verify_reference import EXE_SHA256, FUNCTIONS, pe_file_offset

PINS = (
    (0x00408700, 34560, 48, "5340d1a8f921a030a37df03ee26ab3f04c218ac1705941d7b97c29e7846fb151"),
    (0x0040874C, 34636, 26, "95343cf9ec4047afc2f53e367b9995503ab13f94fb4f86757755a943cce22f31"),
    (0x00408766, 34662, 99, "982a0a9cfeefe0c8bde6cd56d296f2c7adaa42e54d9ab0ed69dfbc996f7a1c16"),
    (0x00401320, 4896, 13, "8a964d1d7ff7ce1a3c60997027339b1b12723566ff413f0bb34cb92f56441d3a"),
)
ADDRESSES = tuple(row[0] for row in PINS) + (0x418390,)
CONSTANTS = ((0x4B0568, bytes.fromhex('0000000000000040')), (0x4BF130, b'drown\0'))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe', type=Path, required=True)
    parser.add_argument('--program', type=Path, required=True)
    args = parser.parse_args()
    data = args.exe.read_bytes()
    if hashlib.sha256(data).hexdigest() != EXE_SHA256: parser.error('reference digest mismatch')
    pins = {row[0]: row for row in (*FUNCTIONS, *PINS)}
    blocks = []
    for address in ADDRESSES:
        _, offset, size, digest = pins[address]
        if offset != pe_file_offset(data, address, size): parser.error('PE mapping mismatch')
        body = data[offset:offset + size]
        if hashlib.sha256(body).hexdigest() != digest: parser.error('code digest mismatch')
        blocks.append(struct.pack('<II', address, size) + body)
    for address, expected in CONSTANTS:
        offset = pe_file_offset(data, address, len(expected))
        body = data[offset:offset + len(expected)]
        if body != expected: parser.error('constant mismatch')
        blocks.append(struct.pack('<II', address, len(body)) + body)
    payload = struct.pack('<I', len(blocks)) + b''.join(blocks)
    for _ in range(8):
        result = subprocess.run([str(args.program)], input=payload, capture_output=True, timeout=30)
        if result.returncode != 3:
            sys.stdout.buffer.write(result.stdout)
            sys.stderr.buffer.write(result.stderr)
            return result.returncode
    return result.returncode


if __name__ == '__main__': raise SystemExit(main())
