"""Compare original texture factory, constructor, indexed bridge and BMP conversion with explicit dependencies."""
import argparse
import hashlib
from pathlib import Path
import struct
import subprocess
import sys
from verify_reference import EXE_SHA256, FUNCTIONS, pe_file_offset

PINS = ((4668832, 474528, 14, 'befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c'), (4479024, 284720, 29, '83c33b4a7dc182b6d187c9b58971163a8594359c68bdb305735c9679217fdaae'), (4522272, 327968, 24, '13d64daa3c44070ffedb8010e1892c40ff61a9452a2044daa6046609572677c5'), (4522296, 327992, 34, '4770f516d2f0a776fcded1a5341eafdf45ac0d827e5cd534354b30cee31b6a9e'), (4658976, 464672, 47, 'abb938ae116aa4564a7b04ff6896291505b508071fa9ba72df8dac4f50c888da'), (4661642, 467338, 705, '9e1dcd7709f171b94c58cde6dc40fe495c4f8d93117ce582ec145e0162b290e5'), (4662416, 468112, 267, '7027e58244c8e9dc414d9ab36719c5d51756fd2d63d5f544f22d763496c5355c'), (4670429, 476125, 261, 'b13be28ffb2444ae3a60e4616ce59d940fa98d76915743196ce20301f3f3b583'), (4662347, 468043, 69, 'd243851a10714311d0a9ba3fcea9ffbce85e6eb94735c53b548c85b4248a98da'), (4703664, 509360, 35, 'eb9a915170340852600a26fc3e9885821597850ec73975d7a02a5d4a932d5c0b'), (4704240, 509936, 14, 'befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c'), (4279552, 85248, 30, '3f6d2d5c7711f92761fa78394636a1ea22836c94e313ce6fc2378ef7e44d1c34'), (4220160, 25856, 14, 'befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c'))
ADDRESSES = tuple(row[0] for row in PINS)
CONSTANTS = ((4990184, b'Lo\x00\x00Hi\x00\x00'), (4990192, b'Textures\\%s.bmp\x00'), (4990208, b'Textures\\%s.bmp\x00'), (4990224, b'Textures\\%s.alf\x00'), (4990240, b'D:\\lrpc\\Source\\3DEngine\\TexturePC.cpp\x00'), (4990280, b'INN_\x00'), (4990288, b'OUT_\x00'))


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
    trials = [()] + [("--fatal", str(i)) for i in (0, 1, 2, 270, 540, 810, 1080, 1350, 1620, 1890, 2160, 2430, 2700, 2970, 3240, 3510, 3780, 4050, 4320, 4590)]
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
