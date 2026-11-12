"""Compare original cache loading and linked list insertion with factory/allocation adapters."""
import argparse
import hashlib
from pathlib import Path
import struct
import subprocess
import sys
from verify_reference import EXE_SHA256, FUNCTIONS, pe_file_offset

PINS = (
    (0x00430660, 198240, 132, "eddcb88ba3ead72b31dbe328b54a5677681306cff1b39ed15db4f93523f70610"),
    (0x00430920, 198944, 33, "65eb72c5dcc4eef9e79634224883eed9512f5f5d518e120100eaca3f5333f080"),
    (0x00430950, 198992, 25, "d2c20f080c996c65c1ebee4fbb538478e72059e23979d5bde8de44db7ef9a5a3"),
    (0x00430970, 199024, 147, "6822089cdf27432eb828c3371ab64fe5a12ef4e7d456af6aacf3946e5677879d"),
    (0x00430A10, 199184, 64, "8f8f5d5b00e3d8183e16d753fd33145772d5a0441361b1b4f9e64c41cf6ce79e"),
    (0x004306F0, 198384, 106, "c9ac76eb0fdf7ee30c39be22275c04a317e44ba8e990c4aa6d4030d792b31b72"),
    (0x00430760, 198496, 17, "a26cd3064709e57ee483cfbbb2753f8da501718ce4ced1e834a5123dd2fcaa3d"),
    (0x00430780, 198528, 29, "279bb61281a416389bf3313e5358634b5d927b0d349db8573597cecd1ad504d0"),
    (0x0042FF90, 196496, 178, "bb1d636337916f8eb82b34865e31ffd9b3b511a623178daaf0028d0eb8fcdeaa"),
    (0x00430050, 196688, 55, "930ae18523a7fe6bde9b56b59842e51d84eb14a72f7229844573fa34821222f1"),
    (0x00446010, 286736, 47, "4d72c8cd7a7947d6b7170dcc631bb5cf29aba80f71e3d6563cb5a42d14d9e204"),
    (0x004307A0, 198560, 30, "3f6d2d5c7711f92761fa78394636a1ea22836c94e313ce6fc2378ef7e44d1c34"),
    (0x00406500, 25856, 14, "befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c"),
    (0x00485590, 546192, 47, "8685ba5caf6c47a39e9c6ae88251572de751818d8f75991dba48369952a7b79a"),
)
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
    for _ in range(8):
        result = subprocess.run([str(args.program)], input=payload, capture_output=True, timeout=30)
        if result.returncode != 3:
            sys.stdout.buffer.write(result.stdout)
            sys.stderr.buffer.write(result.stderr)
            return result.returncode
    return result.returncode


if __name__ == '__main__': raise SystemExit(main())
