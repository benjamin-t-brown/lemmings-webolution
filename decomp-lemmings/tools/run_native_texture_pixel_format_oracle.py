"""Compare original texture pixel-format selection with explicit dependencies."""
import argparse
import hashlib
from pathlib import Path
import struct
import subprocess
import sys
from verify_reference import EXE_SHA256, FUNCTIONS, pe_file_offset

PINS = ((4668190, 473886, 636, '71f87df4a55a244e1dee5f8d7a9b8316a563bce07a60f0d41c610b37adf0485a'),)
ADDRESSES = tuple(row[0] for row in PINS)
CONSTANTS = ()


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
    trials = [()]
    for trial in trials:
        for _ in range(8):
            result = subprocess.run([str(args.program), *trial], input=payload,
                                    capture_output=True, timeout=30)
            if result.returncode != 3:
                break
        sys.stdout.buffer.write(result.stdout)
        sys.stderr.buffer.write(result.stderr)
        if result.returncode:
            return result.returncode
    return 0



if __name__ == '__main__': raise SystemExit(main())
