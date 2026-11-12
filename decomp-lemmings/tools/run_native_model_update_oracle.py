"""Execute pinned model/blink readers in an owned x86 comparison process."""
import argparse
import hashlib
from pathlib import Path
import subprocess
import sys
from verify_reference import EXE_SHA256, FUNCTIONS, pe_file_offset


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", required=True, type=Path)
    parser.add_argument("--program", required=True, type=Path)
    args = parser.parse_args()
    data = args.exe.read_bytes()
    if hashlib.sha256(data).hexdigest() != EXE_SHA256:
        parser.error("reference executable digest mismatch")
    payload = bytearray()
    for address in (0x432383,0x423470,0x43416C,0x434261,0x434290,
                    0x41DC00,0x41DC20,0x49C5AC):
        _, offset, length, digest = next(row for row in FUNCTIONS if row[0] == address)
        if offset != pe_file_offset(data,address,length):
            parser.error("function mapping mismatch")
        body = data[offset:offset+length]
        if hashlib.sha256(body).hexdigest() != digest:
            parser.error("function digest mismatch")
        payload.extend(body)
    for address, expected in ((0x4B0338,"0000000000000000"),
                              (0x4B1210,"9a9999999999c93f"),
                              (0x4B03F0,"0000000000001440")):
        offset = pe_file_offset(data,address,8)
        body = data[offset:offset+8]
        if body.hex() != expected:
            parser.error("constant mismatch")
        payload.extend(body)
    for _ in range(8):
        result = subprocess.run([str(args.program)], input=payload, capture_output=True)
        if result.returncode != 3:
            sys.stdout.buffer.write(result.stdout)
            sys.stderr.buffer.write(result.stderr)
            return result.returncode
    sys.stderr.buffer.write(result.stderr)
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
