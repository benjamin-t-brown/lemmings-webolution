#!/usr/bin/env python3
"""Re-read the static producer chain of the per-cell tube mesh (A4b).

READ-only static audit. It opens the stock executable, verifies its SHA-256,
reads the pinned instruction bytes at each address, decodes the relative
`E8` calls and asserts the producer edges recorded in
`build/tmp/a4-cell-mesh-source/round-note.md`. Nothing is written and no
process is started.

With `--arena-dir` it additionally re-derives the offline facts about the
retained A1 live-arena capture (read-only, under ignored build/tmp). That
capture is an oracle only; it must never become production input.

Run with normal Windows Python (do NOT prepend UCRT64 bin).
"""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
import sys
from collections import Counter, defaultdict
from pathlib import Path

STOCK = Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")
STOCK_SHA256 = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
IMAGE_BASE = 0x00400000


def offset(va: int) -> int:
    """Virtual address -> file offset (rva == file offset for .text here)."""
    return va - IMAGE_BASE


# (address, nbytes, expected hex, note)
PINS = [
    (0x0043A41A, 5, "e8256f0200", "call FUN_00461344 - reset the mesh pool before the bucket sweep"),
    (0x0043A431, 4, "837dfc20", "outer-wall sweep bound: bucket < 0x20"),
    (0x0043A43A, 3, "c1e103", "shl ecx,3 -> group argument is bucket*8"),
    (0x0043A447, 7, "8d8c1008020000", "lea ecx,[world + bucket*0x10 + 0x208] = group header"),
    (0x0043A44E, 5, "e868640200", "call FUN_004608BB - per-bucket cell builder"),
    (0x004608C5, 21, "8b45089983e20703c2c1f8039983e20303c2c1f802",
     "FUN_004608BB: col = ((bucket*8)/8)/4 = bucket>>2 (A2 texture-addressing pin)"),
    (0x004608DD, 10, "b9106a4c00e8b9b1faff", "FUN_004608BB: height = world+0x0 via FUN_0040BAA0"),
    (0x00460A0B, 5, "e850adfdff", "call FUN_0043B760 - group entry count"),
    (0x00460A1B, 6, "8b4df88d34c1", "material = col + ((bands-1-band)/2)*8"),
    (0x00460A2D, 6, "89b0c8000000", "store material at entry+0xC8"),
    (0x00460A41, 5, "e81e180000", "call FUN_00462264 - build the cell mesh"),
    (0x0046226E, 5, "e876f3ffff", "FUN_00462264 -> FUN_004615E9 (faces + side lists)"),
    (0x00462276, 5, "e8850e0000", "FUN_00462264 -> FUN_00463100"),
    (0x0046227E, 5, "e88d110000", "FUN_00462264 -> FUN_00463410 (mesh0 + mesh4)"),
    (0x00463425, 5, "e876f3ffff", "mesh0 count source A = list 0x4ECA80"),
    (0x00463431, 5, "e85af5ffff", "mesh0 count source B = list 0x4ECA58"),
    (0x0046343D, 5, "e80ef7ffff", "mesh0/mesh4 count source C = list 0x4ECA48"),
    (0x00463497, 5, "e8ebedffff", "mesh0 allocation: FUN_00462287(entry, count, entry+0xB0, 1)"),
    (0x00463F07, 5, "e87be3ffff", "mesh4 allocation: FUN_00462287(entry, count, entry+0xB4, 0)"),
    (0x004622ED, 5, "b910cb4e00", "FUN_00462287: mesh pool object = 0x4ECB10"),
    (0x004622F2, 5, "e8a52c0000", "FUN_00462287 -> FUN_00464F9C (bump allocate)"),
]

# producer edges: (call site, target, label)
EDGES = [
    (0x0043A44E, 0x004608BB, "bucket sweep -> per-bucket cell builder"),
    (0x00460A41, 0x00462264, "per-band cell -> mesh builder"),
    (0x0046226E, 0x004615E9, "cell -> face/side list builder"),
    (0x0046227E, 0x00463410, "cell -> mesh0/mesh4 assembler"),
    (0x00463497, 0x00462287, "mesh0 allocation"),
    (0x00463F07, 0x00462287, "mesh4 allocation"),
    (0x004622F2, 0x00464F9C, "pool bump allocator"),
]

# functions whose bodies sit inside the producer chain
BODIES = [
    (0x004608BB, 0x00460A4F, "FUN_004608BB per-(bucket) cell sweep"),
    (0x00462264, 0x00462286, "FUN_00462264 cell mesh driver"),
    (0x00462287, 0x004622FC, "FUN_00462287 pool reserve wrapper"),
    (0x00463410, 0x00464BF3, "FUN_00463410 mesh0/mesh4 assembler"),
    (0x004615E9, 0x00461E1C, "FUN_004615E9 face/side list builder (reads the grid)"),
    (0x00461E1D, 0x00462263, "FUN_00461E1D part-table builder"),
    (0x0043A1C6, 0x0043A31C, "FUN_0043A1C6 get-or-create vertex (world+0x148 pool)"),
    (0x0043D2B8, 0x0043D540, "FUN_0043D2B8 interpolate the cylinder template"),
    (0x0040DE60, 0x0040DE8F, "FUN_0040DE60 collision-cell accessor (world+0x41C)"),
]


def read(data: bytes, va: int, n: int) -> bytes:
    return data[offset(va):offset(va) + n]


def decode_call(data: bytes, va: int):
    raw = read(data, va, 5)
    if len(raw) != 5 or raw[0] != 0xE8:
        return None
    rel = struct.unpack_from("<i", raw, 1)[0]
    return va + 5 + rel


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--exe", type=Path, default=STOCK)
    ap.add_argument("--arena-dir", type=Path,
                    default=Path("build/tmp/a1-live-arena"),
                    help="retained A1 live-arena capture directory (oracle only)")
    ap.add_argument("--skip-arena", action="store_true")
    args = ap.parse_args()

    data = args.exe.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    print(f"exe    {args.exe}")
    print(f"sha256 {digest}")
    if digest != STOCK_SHA256:
        print("FAIL: not the recorded stock executable")
        return 2

    failures = 0
    print("\n-- pinned instruction bytes --")
    for va, n, want, note in PINS:
        got = read(data, va, n)
        ok = got.hex() == want
        failures += 0 if ok else 1
        print(f"{'ok ' if ok else 'BAD'} {va:#010x} {got.hex():<34} {note}")
        if not ok:
            print(f"      expected {want}")

    print("\n-- decoded producer edges --")
    for site, target, label in EDGES:
        got = decode_call(data, site)
        ok = got == target
        failures += 0 if ok else 1
        print(f"{'ok ' if ok else 'BAD'} {site:#010x} -> {got and hex(got)} "
              f"(want {target:#x})  {label}")

    print("\n-- producer function bodies --")
    for lo, hi, label in BODIES:
        print(f"    {lo:#010x}..{hi:#010x}  {label}")

    if args.skip_arena:
        return 1 if failures else 0

    arena_path = args.arena_dir / "arena.bin"
    manifest_path = args.arena_dir / "live-arena-manifest.json"
    if not arena_path.exists() or not manifest_path.exists():
        print(f"\n(arena capture not present at {args.arena_dir}; skipping offline facts)")
        return 1 if failures else 0

    print("\n-- offline facts re-derived from the retained A1 live arena (oracle only) --")
    arena = arena_path.read_bytes()
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    print(f"    arena sha256 {hashlib.sha256(arena).hexdigest()}")
    print(f"    manifest arena sha256 {manifest['arena']['sha256']}")
    if hashlib.sha256(arena).hexdigest() != manifest["arena"]["sha256"]:
        print("FAIL: arena.bin does not match the manifest digest")
        failures += 1

    def quads(entry, route):
        r = entry[route]
        if not r.get("count"):
            return []
        out = []
        for q in range(r["count"]):
            raw = arena[r["array_offset"] + 0x80 * q:r["array_offset"] + 0x80 * (q + 1)]
            idx = tuple(struct.unpack_from("<I", raw, 0x20 + 0x18 * c)[0] for c in range(4))
            uvs = tuple(struct.unpack_from("<2d", raw, 0x20 + 0x18 * c + 8) for c in range(4))
            out.append((idx, uvs))
        return out

    entries = manifest["entries"]
    allq = []
    counts = Counter()
    for e in entries:
        qs = quads(e, "mesh0")
        allq += qs
        counts[(e["group"], e["entry"])] = len(qs)
    uniq = {q[0] for q in allq}
    print(f"    mesh0 quads {len(allq)}  unique index sets {len(uniq)}"
          f"  max index {max(max(q[0]) for q in allq)}")
    dist = Counter(counts.values())
    print(f"    mesh0 quads/cell histogram {dict(sorted(dist.items()))}")

    # UV anchor per (bucket%4, band): the small per-quad nudges are the tell
    anchors = defaultdict(set)
    for e in entries:
        for idx, uvs in quads(e, "mesh0"):
            g, b = e["group"], e["entry"]
            for (u, v) in uvs:
                anchors[(g % 4, b)].add((round(u, 6), round(v, 6)))
    print("    UV value sets per (bucket%4, band) - anchor grid + per-quad nudges:")
    for key in sorted(anchors):
        vals = sorted(anchors[key])
        print(f"      {key}: n={len(vals)} sample={vals[:4]}")

    # mesh0/mesh4 relationship
    same = sum(1 for e in entries
               if e["mesh4"].get("count") and e["mesh0"].get("count")
               and e["mesh0"]["array_sha256"] == e["mesh4"]["array_sha256"])
    both = sum(1 for e in entries if e["mesh4"].get("count") and e["mesh0"].get("count"))
    print(f"    cells with both meshes {both}; byte-identical mesh0/mesh4 {same}")

    print("\n{}".format("ALL PINS OK" if not failures else f"{failures} FAILURE(S)"))
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
