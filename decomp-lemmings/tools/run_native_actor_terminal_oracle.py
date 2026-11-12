"""Execute terminal behavior bodies with explicit effect/model adapters."""
import argparse
import hashlib
from pathlib import Path
import struct
import subprocess
import sys
from verify_reference import EXE_SHA256, FUNCTIONS, pe_file_offset

PINS = (
    (0x0040B3A0, 45984, 48, "4d91ce609376caf9a4d1b52e8178680d6a4317cb979143c19204d8fd30d1fa5f"),
    (0x0040B3EC, 46060, 1542, "bcae2c0c12e932e925309cf5c18207c38cadc831ae92c237711c8d8d8bf7955e"),
    (0x0040B9F2, 47602, 104, "f5737cd3bb0aed25041d2a5462df5d47394decffea281a32467e1fd112c08fdb"),
    (0x0040BA60, 47712, 13, "8a964d1d7ff7ce1a3c60997027339b1b12723566ff413f0bb34cb92f56441d3a"),
    (0x004010D0, 4304, 31, "38fd038f41a342d0889685b8ddb612588565f0a9b1c5bdfe2a2cdb905e7103de"),
    (0x00418390, 99216, 47, "254616efe4eeecb6423131c308a44bd17d051f5651e8a0adebbe4d242ca65772"),
)
ADDRESSES = (0x40B3A0, 0x40B3EC, 0x40B9F2, 0x40BA60, 0x4010D0, 0x418390,
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
    zero = data[pe_file_offset(data, 0x4B0338, 8):][:8]
    if zero != bytes(8): parser.error('zero constant mismatch')
    blocks.append(struct.pack('<II', 0x4B0338, 8) + zero)
    payload = struct.pack('<I', len(blocks)) + b''.join(blocks)
    for _ in range(8):
        result = subprocess.run([str(args.program)], input=payload, capture_output=True, timeout=30)
        if result.returncode != 3:
            sys.stdout.buffer.write(result.stdout)
            sys.stderr.buffer.write(result.stderr)
            return result.returncode
    return result.returncode


if __name__ == '__main__': raise SystemExit(main())
