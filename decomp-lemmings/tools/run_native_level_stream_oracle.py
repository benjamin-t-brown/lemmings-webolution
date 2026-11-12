"""Run verified original level-string bytes with an in-memory read adapter."""

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
    if hashlib.sha256(data).hexdigest() != EXE_SHA256:
        parser.error("reference executable SHA-256 mismatch")
    bodies = []
    for address in (0x00402540, 0x00402560, 0x0047D940, 0x0047DAC8):
        _, offset, length, expected = next(row for row in FUNCTIONS if row[0] == address)
        body = data[offset:offset + length]
        if len(body) != length or hashlib.sha256(body).hexdigest() != expected:
            parser.error(f"0x{address:08X} bytes do not match the reference")
        bodies.append(body)
    payload = b"".join(bodies)
    for _ in range(8):
        result = subprocess.run([str(args.program)], input=payload,
                                capture_output=True, check=False)
        if result.returncode != 3:
            sys.stdout.buffer.write(result.stdout)
            sys.stderr.buffer.write(result.stderr)
            return result.returncode
    sys.stderr.buffer.write(result.stderr)
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
