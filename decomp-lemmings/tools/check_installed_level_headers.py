"""Compare the C++ header reader with installed .LVL entries in .box files."""

import argparse
from pathlib import Path
import struct
import subprocess

from inspect_box import read_index, read_exact


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--probe", required=True, type=Path)
    parser.add_argument("--archive", required=True, type=Path, nargs="+")
    args = parser.parse_args()
    payload = bytearray()
    expected = []
    for archive in args.archive:
        with archive.open("rb") as source:
            for entry in read_index(archive):
                if not (entry.name.casefold().startswith("compiled\\") and
                        entry.name.casefold().endswith(".lvl")):
                    continue
                source.seek(entry.offset)
                header = read_exact(source, 40)
                if header[:12] != bytes.fromhex("080000004c6576656c203233"):
                    parser.error(f"unexpected level marker: {entry.name}")
                fields = struct.unpack_from("<6I", header, 12)
                flags = tuple(int(x != 0) for x in header[36:40])
                expected.append((archive.name, entry.name,
                                 (1, *fields, *flags, fields[0] * 58, 40)))
                payload.extend(header)
    if not expected:
        parser.error("no compiled levels found in the supplied archives")
    result = subprocess.run([str(args.probe)], input=payload,
                            capture_output=True, check=False)
    if result.returncode != 0:
        parser.error(result.stderr.decode(errors="replace"))
    rows = result.stdout.decode().splitlines()
    if len(rows) != len(expected):
        parser.error(f"probe returned {len(rows)} rows for {len(expected)} headers")
    for line, (archive_name, entry_name, target) in zip(rows, expected):
        observed = tuple(int(value) for value in line.split())
        if observed != target:
            parser.error(f"header mismatch: {archive_name}:{entry_name}: {observed} != {target}")
    print(f"matched {len(rows)} installed compiled level headers")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
