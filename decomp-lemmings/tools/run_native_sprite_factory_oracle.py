"""Compare original sprite factory, constructors and destructor wrappers with explicit dependencies."""
import argparse
import hashlib
from pathlib import Path
import struct
import subprocess
import sys
from verify_reference import EXE_SHA256, FUNCTIONS, pe_file_offset

PINS = (
    (0x00484EB0, 544432, 311, "272a93728a8b58a1764d91609f7e2ce63973884fc89b77f14d5da2be806875d5"),
    (0x00484FE7, 544743, 55, "0ba8f7c2b2707d831b1dcf3973e2fe4f4577e52ce1f03a6c720300d3cb96ba6f"),
    (0x00485050, 544848, 78, "327b8144c5da1bcebbabe7f21c96f969fa7036ae6a79fe3fa703cf69f8718e7c"),
    (0x00485210, 545296, 53, "1f0030291fa3a68771361e044671fd4c4ed373926b81c41092ccd2071eedc704"),
    (0x00485270, 545392, 53, "1f0030291fa3a68771361e044671fd4c4ed373926b81c41092ccd2071eedc704"),
    (0x004850A0, 544928, 46, "9af8266f53a255c2de4a09e685b10e229484059afa61b741ae5098fe1750b900"),
    (0x004850D0, 544976, 75, "6955cf9d156ac7052b6a2aab2aa525b932ac84a51c5a23c71e8d91cd509c8f17"),
    (0x00485250, 545360, 19, "342c81fd54c806d61db72994c542d667431ef9f0ef6f51d67f3d41f82f9718a1"),
    (0x004852B0, 545456, 19, "2ea60ef448944579c91dba652fa31ba4f6eec613f6fae3546dde439a83dc67ec"),
)
ADDRESSES = tuple(row[0] for row in PINS)
CONSTANTS = (
    (0x4C33C4, b"Sprites\\%s.spr\0"),
    (0x4C33D4, b"D:\\lrpc\\Source\\Sprites\\SpriteSheets.cpp\0"),
)


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
    trials = [()] + [("--fatal", str(i)) for i in
                      (0, 1, 2, 42, 43, 44, 126, 127, 128, 252, 253, 254, 294, 295, 296, 378, 379, 380)]
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
