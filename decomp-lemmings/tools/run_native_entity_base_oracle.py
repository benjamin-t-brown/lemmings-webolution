"""Run verified entity-reader bytes with isolated stream and side-effect adapters."""

import argparse
import hashlib
from pathlib import Path
import subprocess
import sys

from verify_reference import EXE_SHA256, FUNCTIONS, pe_file_offset


# Additional unmodified helpers used by this oracle; main ledger pins the
# three entity readers and all other scalar/vector readers.
HELPERS = (
    (0x00423360, 31, "d7bd98a68723f3516226dd2566681c2455b28a0642c9bccc1005665faadfee3a"),
    (0x00423380, 25, "66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be"),
)
ADDRESSES = (0x00402540, 0x00402560, 0x00402580, 0x00423291,
             0x00423360, 0x00423380, 0x004265E8, 0x0042FB5F,
             0x0047DAC8, 0x00480BB0, 0x004821DE)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", required=True, type=Path)
    parser.add_argument("--program", required=True, type=Path)
    args = parser.parse_args()
    data = args.exe.read_bytes()
    if hashlib.sha256(data).hexdigest() != EXE_SHA256:
        parser.error("reference executable SHA-256 mismatch")
    pins = {address: (length, digest) for address, _, length, digest in FUNCTIONS}
    pins.update({address: (length, digest) for address, length, digest in HELPERS})
    bodies = []
    for address in ADDRESSES:
        length, expected = pins[address]
        offset = pe_file_offset(data, address, length)
        body = data[offset:offset + length]
        if len(body) != length or hashlib.sha256(body).hexdigest() != expected:
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
