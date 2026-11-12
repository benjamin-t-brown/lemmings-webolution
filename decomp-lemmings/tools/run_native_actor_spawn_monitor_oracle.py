"""Run verified spawned monitor bodies with explicit region/state/queue adapters."""
import argparse
import hashlib
from pathlib import Path
import struct
import subprocess
import sys
from verify_reference import EXE_SHA256, FUNCTIONS, pe_file_offset

PINS = (
    (0x004151F0, 86512, 111, "1a4787584349fab59100b1936ba48b906564a7054266a5dd4970649a45fe0b20"),
    (0x0041527B, 86651, 993, "489df9beceb928e0ee709746ba621145715e56c28c1bf33c9874ece884b2c509"),
    (0x00409310, 37648, 13, "8a964d1d7ff7ce1a3c60997027339b1b12723566ff413f0bb34cb92f56441d3a"),
    (0x00401320, 4896, 13, "8a964d1d7ff7ce1a3c60997027339b1b12723566ff413f0bb34cb92f56441d3a"),
    (0x0042F183, 192899, 171, "63d472d73bc51e453f1214a0665846d1895786a14a3735ad25dae540434fb4a0"),
    (0x00415800, 88064, 70, "4daf083b4770f6afc5a4dda2424f901e4cd517c82c04a625c89421add528b24a"),
    (0x004821D0, 532944, 14, "befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c"),
    (0x0041565C, 87644, 119, "a37e04e0eb7faf59c362a1f658b0a3a8f0904da9a4c76de39fdb3da04869b10c"),
    (0x00404D40, 19776, 45, "3ebc9d00a9d2a61f993f1a29129630d9b6b9f86c193fd84945747265e7021535"),
    (0x004066D0, 26320, 36, "a4ad0092f5c119225ea631a845d2868e0ce880df82c92534edcca4a4791c5134"),
    (0x00401B80, 7040, 14, "befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c"),
    (0x00406700, 26368, 68, "699c6b6a605605e3da76d40aeb46eaa9af24f91e75b34166c02d29d7c0c2b306"),
)
ADDRESSES = tuple(row[0] for row in PINS) + (
    0x418390, 0x4066A0, 0x403EB0, 0x4017F0, 0x404D10, 0x40DE60,
    0x40DEC0, 0x40DF30, 0x40DF50, 0x40BAA0, 0x49C5AC, 0x49C8F4, 0x49FC56)
CONSTANTS = ((0x4B0338, '0000000000000000'),
             (0x4B0698, '000000e08e79453e'), (0x4B07A8, '0000000000007040'))


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
        offset = pe_file_offset(data, address, 8)
        body = data[offset:offset + 8]
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
