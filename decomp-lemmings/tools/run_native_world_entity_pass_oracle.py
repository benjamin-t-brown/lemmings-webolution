"""Compare the preserved world collection block with explicit effect adapters."""
import argparse
import hashlib
from pathlib import Path
import struct
import subprocess
import sys
from verify_reference import EXE_SHA256, FUNCTIONS, pe_file_offset

PINS = (
    (0x004423D2, 271314, 3569, "e1b02be8cfe9fb35345494b637c8a07db697ff288f9e52914372293637628fbf"),
    (0x00446400, 287744, 17, "31f46434012fd7ba8bbf8b14308e9a805714de8400146ddb80ab47e40c27d964"),
    (0x00446420, 287776, 29, "8e254846ffcac263c367901dff6450f94b61c566b4e29c0a90f464ecdeea2ffc"),
    (0x004465A0, 288160, 25, "66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be"),
    (0x004465C0, 288192, 16, "8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936"),
    (0x00446B20, 289568, 25, "66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be"),
    (0x00446B40, 289600, 16, "8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936"),
    (0x00446C40, 289856, 25, "66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be"),
    (0x00446C60, 289888, 16, "8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936"),
    (0x0041CB40, 117568, 25, "66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be"),
    (0x0041CB60, 117600, 16, "8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936"),
    (0x004131C0, 78272, 25, "66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be"),
    (0x004131E0, 78304, 16, "8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936"),
    (0x0041CB70, 117616, 25, "66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be"),
    (0x0041CB90, 117648, 16, "8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936"),
    (0x00447620, 292384, 25, "66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be"),
    (0x00447640, 292416, 16, "8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936"),
    (0x00446F60, 290656, 25, "66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be"),
    (0x00446F80, 290688, 16, "8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936"),
    (0x0041E0B0, 123056, 25, "66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be"),
    (0x0041E0D0, 123088, 16, "8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936"),
    (0x004090F0, 37104, 17, "31f46434012fd7ba8bbf8b14308e9a805714de8400146ddb80ab47e40c27d964"),
    (0x00409110, 37136, 29, "8e254846ffcac263c367901dff6450f94b61c566b4e29c0a90f464ecdeea2ffc"),
    (0x0041CBA0, 117664, 17, "70169696f07dda40d76cec558fc5a4b7f4bcaa4410401b5889072b73e4930537"),
    (0x00406A00, 27136, 18, "45713e9071f67175de33bff8071f8358847bcf139043b6cfbb69f2f7744aaa47"),
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
