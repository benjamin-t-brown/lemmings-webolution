"""Run verified original subbox code in an owned 32-bit test process."""

import argparse
import hashlib
from pathlib import Path
import subprocess
import sys

from verify_reference import EXE_SHA256, FUNCTIONS


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", required=True, type=Path)
    parser.add_argument("--program", required=True, type=Path)
    args = parser.parse_args()

    data = args.exe.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if digest != EXE_SHA256:
        parser.error(f"reference executable SHA-256 mismatch: {digest}")

    bodies = []
    for address in (0x00428670, 0x00431370, 0x00430C9C):
        _, offset, length, expected = next(row for row in FUNCTIONS if row[0] == address)
        body = data[offset:offset + length]
        if len(body) != length or hashlib.sha256(body).hexdigest() != expected:
            parser.error(f"0x{address:08X} function bytes do not match the reference")
        bodies.append(body)
    for _ in range(8):
        result = subprocess.run([str(args.program)], input=b"".join(bodies),
                                capture_output=True, check=False)
        if result.returncode != 3:  # low fixed VA was occupied by ASLR
            sys.stdout.buffer.write(result.stdout)
            sys.stderr.buffer.write(result.stderr)
            return result.returncode
    sys.stderr.buffer.write(result.stderr)
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
