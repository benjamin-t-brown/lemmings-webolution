"""Execute region-terminal behavior bodies with explicit effect/model adapters."""
import argparse
import hashlib
from pathlib import Path
import struct
import subprocess
import sys
from verify_reference import EXE_SHA256, FUNCTIONS, pe_file_offset

PINS = (
    (0x00406C20, 27680, 48, "80426f004f3e5f3aad6c62a452cdc08a93e2c7d127d8fd597ee161a5409428c9"),
    (0x00406C6C, 27756, 1418, "86ca371273326093ff33932bb22bc3621912c81a71190cf31002606569fc3fcc"),
    (0x004071F6, 29174, 104, "f5737cd3bb0aed25041d2a5462df5d47394decffea281a32467e1fd112c08fdb"),
    (0x00407260, 29280, 13, "8a964d1d7ff7ce1a3c60997027339b1b12723566ff413f0bb34cb92f56441d3a"),
    (0x004010D0, 4304, 31, "38fd038f41a342d0889685b8ddb612588565f0a9b1c5bdfe2a2cdb905e7103de"),
    (0x00418390, 99216, 47, "254616efe4eeecb6423131c308a44bd17d051f5651e8a0adebbe4d242ca65772"),
)
ADDRESSES = (0x406C20, 0x406C6C, 0x4071F6, 0x407260, 0x4010D0, 0x418390,
             0x40BAA0, 0x406A00, 0x401830, 0x4017F0, 0x403EB0, 0x49C5AC)


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
    for address, expected in ((0x4B02D0, '000000000000f03f'), (0x4B02C8, '0000000000001040')):
        body = data[pe_file_offset(data, address, 8):][:8]
        if body.hex() != expected: parser.error('constant mismatch')
        blocks.append(struct.pack('<II', address, 8) + body)
    payload = struct.pack('<I', len(blocks)) + b''.join(blocks)
    for _ in range(8):
        result = subprocess.run([str(args.program)], input=payload, capture_output=True, timeout=30)
        if result.returncode != 3:
            sys.stdout.buffer.write(result.stdout)
            sys.stderr.buffer.write(result.stderr)
            return result.returncode
    return result.returncode


if __name__ == '__main__': raise SystemExit(main())
