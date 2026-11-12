"""Run original actor animation selection with explicit bind/select adapters."""
import argparse
import hashlib
from pathlib import Path
import struct
import subprocess
import sys
from verify_reference import EXE_SHA256, FUNCTIONS, pe_file_offset

PINS = (
    (0x0042FD25, 195877, 157, "bd3db9e2d45e8fccecce4bbefe963a322512f9e6c04fb7e314833b46cbaed439"),
    (0x0048408C, 540812, 103, "ff45973dd2bf698273bd545d7785aef0974f4fe3e9c671975263985953e9a32a"),
    (0x004303D0, 197584, 25, "4bc6a396629309176d0ee3fbabf45ff8ac1b1fcecd8a5b90aaa450d0480791d7"),
    (0x004303F0, 197616, 16, "8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936"),
)
ADDRESSES = (0x42FD25, 0x48408C, 0x4303D0, 0x4303F0,
             0x484150, 0x41A4E0, 0x403720, 0x49C870)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe', type=Path, required=True)
    parser.add_argument('--program', type=Path, required=True)
    args = parser.parse_args()
    data = args.exe.read_bytes()
    if hashlib.sha256(data).hexdigest() != EXE_SHA256:
        parser.error('reference executable digest mismatch')
    pins = {row[0]: row for row in (*FUNCTIONS, *PINS)}
    blocks = []
    for address in ADDRESSES:
        _, offset, size, digest = pins[address]
        if offset != pe_file_offset(data, address, size): parser.error('PE mapping mismatch')
        body = data[offset:offset + size]
        if hashlib.sha256(body).hexdigest() != digest: parser.error('code digest mismatch')
        blocks.append(struct.pack('<II', address, size) + body)
    payload = struct.pack('<I', len(blocks)) + b''.join(blocks)
    for _ in range(8):
        result = subprocess.run([str(args.program)], input=payload, capture_output=True, timeout=30)
        if result.returncode != 3:
            sys.stdout.buffer.write(result.stdout)
            sys.stderr.buffer.write(result.stderr)
            return result.returncode
    return result.returncode


if __name__ == '__main__': raise SystemExit(main())
