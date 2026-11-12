"""Compare recovered containment with original bytes in a 32-bit helper.

The executable is read but never launched or modified. Its function bytes are
sent over stdin to a small owned test process, not stored in the repository.
"""

import argparse
import hashlib
from pathlib import Path
import subprocess

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
    _, offset, length, expected = next(row for row in FUNCTIONS if row[0] == 0x00430BA1)
    body = data[offset:offset + length]
    if len(body) != length or hashlib.sha256(body).hexdigest() != expected:
        parser.error("0x00430BA1 function bytes do not match the reference")

    return subprocess.run([str(args.program)], input=body, check=False).returncode


if __name__ == "__main__":
    raise SystemExit(main())
