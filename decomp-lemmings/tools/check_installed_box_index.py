"""Compare the recovered C++ .box index with installed archive entries."""

import argparse
from pathlib import Path
import struct
import subprocess

from inspect_box import read_index


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--probe", required=True, type=Path)
    parser.add_argument("--archive", required=True, type=Path, nargs="+")
    args = parser.parse_args()
    total = 0
    for archive in args.archive:
        entries = read_index(archive)
        expected = bytearray(struct.pack("<I", len(entries)))
        for entry in entries:
            name = entry.name.encode("latin-1")
            expected.extend(struct.pack("<I", len(name)))
            expected.extend(name)
            expected.extend(struct.pack("<II", entry.offset, entry.size))
        result = subprocess.run([str(args.probe), str(archive)],
                                capture_output=True, check=False)
        if result.returncode != 0 or result.stdout != expected:
            parser.error(f"archive index mismatch: {archive}: "
                         f"{result.stderr.decode(errors='replace')}")
        levels = [(i, e) for i, e in enumerate(entries)
                  if e.name.casefold().startswith("compiled\\") and
                  e.name.casefold().endswith(".lvl")]
        if not levels:
            parser.error(f"no compiled levels found in {archive}")
        ordinal, sample = levels[0]
        result = subprocess.run([str(args.probe), str(archive), "--entry", str(ordinal)],
                                capture_output=True, check=False)
        with archive.open("rb") as source:
            source.seek(sample.offset)
            original = source.read(sample.size)
        if result.returncode != 0 or result.stdout != original:
            parser.error(f"archive entry mismatch: {archive}:{sample.name}")
        total += len(entries)
    print(f"matched {total} installed archive entries and sampled level payloads")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
