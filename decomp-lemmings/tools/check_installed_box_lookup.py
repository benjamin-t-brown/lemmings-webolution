"""Check patch-first, case-normalized lookups against installed archive names."""

import argparse
from pathlib import Path
import subprocess

from inspect_box import read_index


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--probe", required=True, type=Path)
    parser.add_argument("--patch", required=True, type=Path)
    parser.add_argument("--base", required=True, type=Path)
    args = parser.parse_args()
    patch = read_index(args.patch)
    base = read_index(args.base)
    patch_names = {entry.name: i for i, entry in enumerate(patch)}
    base_names = {entry.name: i for i, entry in enumerate(base)}
    overlaps = sorted(patch_names.keys() & base_names.keys())
    assert overlaps, "no overlapping archive names"
    patch_only = next(name for name in patch_names if name not in base_names)
    base_only = next(name for name in base_names if name not in patch_names)
    names = overlaps + [patch_only, base_only, base_only.lower(),
                        "C:\\" + base_only.lower(), "DOES\\NOT\\EXIST.DAT"]
    expected = [(0, patch_names[name]) for name in overlaps]
    expected += [(0, patch_names[patch_only]), (1, base_names[base_only]),
                 (1, base_names[base_only]), (1, base_names[base_only]),
                 (-1, -1)]
    result = subprocess.run([str(args.probe), str(args.patch), str(args.base)],
                            input="\n".join(names) + "\n", text=True,
                            capture_output=True, check=False)
    got = [tuple(map(int, line.split())) for line in result.stdout.splitlines()]
    if result.returncode != 0 or got != expected:
        parser.error(f"archive lookup mismatch: {result.stderr}")
    print(f"matched {len(overlaps)} overlapping names with patch precedence")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
