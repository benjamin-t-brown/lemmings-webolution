"""Compare the recovered emitter with verified original bytes and boundary adapters."""

import argparse
import hashlib
from pathlib import Path
import subprocess
import sys

from verify_reference import EXE_SHA256, pe_file_offset


PINS = (
    (0x00425A95, 728, "f0babcbc6518c418763f529e6ff209728216e2c855fd3d57297951f9ebf47bce"),
    (0x004B0338, 8, "af5570f5a1810b7af78caf4bc70a660f0df51e42baf91d4de5b2328de0e83dfc"),
    (0x004B0F48, 8, "3a782dc88acafd2e210287a7ccbcefda1d89207e71b059f4c28ba42c72e220e8"),
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
                                capture_output=True, check=False)
        if result.returncode != 3:
            sys.stdout.buffer.write(result.stdout)
            sys.stderr.buffer.write(result.stderr)
            return result.returncode
    sys.stderr.buffer.write(result.stderr)
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
