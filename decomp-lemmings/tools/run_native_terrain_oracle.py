"""Run verified original terrain-loop bytes in an owned 32-bit process."""

import argparse
import hashlib
from pathlib import Path
import subprocess
import sys

from verify_reference import DATA, EXE_SHA256, FUNCTIONS


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", required=True, type=Path)
    parser.add_argument("--program", required=True, type=Path)
    args = parser.parse_args()
    data = args.exe.read_bytes()
    if hashlib.sha256(data).hexdigest() != EXE_SHA256:
        parser.error("reference executable SHA-256 mismatch")
    bodies = []
    for address in (0x00404D10, 0x0040BAA0, 0x0040DE60, 0x0040DEC0,
                    0x0040DF30, 0x0040DF50, 0x0043F510, 0x0049C5AC,
                    0x0049C8F4, 0x0049FC56):
        _, offset, length, expected = next(row for row in FUNCTIONS if row[0] == address)
        body = data[offset:offset + length]
        if len(body) != length or hashlib.sha256(body).hexdigest() != expected:
            parser.error(f"0x{address:08X} bytes do not match the reference")
        bodies.append(body)
    constants = []
    for offset, expected in ((721720, "0000000000000000"),
                             (721912, "0000000000000040"),
                             (722584, "000000e08e79453e"),
                             (722856, "0000000000007040")):
        value = data[offset:offset + 8]
        if value.hex() != expected:
            parser.error(f"constant at file offset {offset} does not match")
        constants.append(value)
    masks = []
    for address in (0x004B1528, 0x004B155C):
        _, offset, length, expected = next(row for row in DATA if row[0] == address)
        mask = data[offset:offset + length]
        if len(mask) != length or hashlib.sha256(mask).hexdigest() != expected:
            parser.error(f"mask at 0x{address:08X} does not match")
        masks.append(mask)
    payload = b"".join(bodies + constants + masks)
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
