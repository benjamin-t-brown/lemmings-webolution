"""Compare original actor scheduler bytes with the typed source scheduler."""
import argparse
import hashlib
from pathlib import Path
import subprocess
import sys

from verify_reference import EXE_SHA256, pe_file_offset


PINS = (
    (0x0042E2BF, 983, "816b9a400888304cddf787041e376fe009cbdbec0d426b38bf8f23be75d40ac8"),
    (0x00406A00, 18, "45713e9071f67175de33bff8071f8358847bcf139043b6cfbb69f2f7744aaa47"),
    (0x0042FEF0, 83, "7bc9223189ff9857ef8bcb775ae6a6aec1dffe2f66e3631cb95f3c7d32d675d5"),
    (0x00430170, 17, "31f46434012fd7ba8bbf8b14308e9a805714de8400146ddb80ab47e40c27d964"),
    (0x00430190, 29, "8e254846ffcac263c367901dff6450f94b61c566b4e29c0a90f464ecdeea2ffc"),
    (0x004B0338, 8, "af5570f5a1810b7af78caf4bc70a660f0df51e42baf91d4de5b2328de0e83dfc"),
    (0x0042E696, 116, "7fd9a9d9b7942dc958956a910a5379f17c4a52236d10be1a8f4334bc918fa9fb"),
    (0x0042ECC9, 132, "0a76be8749b49bf144bcd4af0cac7fff7af37d37cb78dea6a7d663fa20c6a453"),
    (0x00430A50, 17, "6e3d11629cf3a32c1e772d65313b8c31bf1b78d21c58555f949dd329a5326f5d"),
    (0x00430A70, 17, "d3bc3e852e9ca92d62e0d140c341815bf8b7b6e7b043f9f8b0a5c45665694124"),
    (0x00430290, 25, "0782df8156e4ec4a9fad514bc8874375d7a7e5edcbacd2f747e9634fb380e29f"),
    (0x00430590, 147, "92bafe77b038dc356a5bb3e5af77bf2e4cb7aae2eaf08666c61c06803d4550b5"),
    (0x00430630, 44, "5d03e6ed2be1fdea3fb7309a5e517ebec66f39ab63a01cbe660434509585702c"),
)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", required=True, type=Path)
    parser.add_argument("--program", required=True, type=Path)
    args = parser.parse_args()
    data = args.exe.read_bytes()
    if hashlib.sha256(data).hexdigest() != EXE_SHA256:
        parser.error("reference executable SHA-256 mismatch")
    bodies = []
    for address, length, digest in PINS:
        offset = pe_file_offset(data, address, length)
        body = data[offset:offset + length]
        if hashlib.sha256(body).hexdigest() != digest:
            parser.error(f"0x{address:08X} bytes do not match the reference")
        bodies.append(body)
    for _ in range(8):
        result = subprocess.run([str(args.program)], input=b"".join(bodies),
                                capture_output=True, check=False, timeout=30)
        if result.returncode != 3:
            sys.stdout.buffer.write(result.stdout)
            sys.stderr.buffer.write(result.stderr)
            return result.returncode
    sys.stderr.buffer.write(result.stderr)
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
