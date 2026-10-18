"""Measure the unread tail of PolyList 09 kind 5 and kind 7 records.

src/assets/PolyList.cpp names the first 52 bytes of each record: a u32 index
and six little-endian f64s (u, v, r, g, b, a). Kind 5 records are 160 bytes
and kind 7 records are 212. This script checks the layout that fits those
strides exactly:

    kind 5: 3 x (u32 + 6 f64) + one trailing u32
    kind 7: 4 x (u32 + 6 f64) + one trailing u32

because 3*52+4 = 160 and 4*52+4 = 212. It reports the value ranges. It does
not decide what the trailing u32 means.

Levels: COMPILED\\*.LVL in lemmings.box, with patch.box overriding the same
name (AssetLibrary's rule).

Usage:
    python tools/harness/read_polylist_tail.py
    python tools/harness/read_polylist_tail.py MM_CLIMB_002
"""
from __future__ import annotations

import math
import struct
import sys
from collections import Counter
from pathlib import Path

GAME = Path(r"C:\progs\Lemmings Revolution")
NAME = struct.pack("<I", 11) + b"PolyList 09"
GROUP = 52  # u32 + 6 f64, the prefix the parser already names
STRIDE = {5: 160, 7: 212}
FOCUS = ("MM_CLIMB_002", "MM_FLOAT", "VH13_KF_1")


def stem_of(name: str) -> str:
    """COMPILED\\MM_FLOAT.LVL -> MM_FLOAT. MM_FLOAT_CLIMB2 stays distinct."""
    base = name.replace("/", "\\").rsplit("\\", 1)[-1]
    if base.upper().endswith(".LVL"):
        base = base[:-4]
    return base.upper()
# Slots inside one 52-byte group, same order as the leading vertex.
FIELDS = ("index", "u", "v", "r", "g", "b", "a")


def read_box(path: Path):
    data = path.read_bytes()
    if data[:6] != b"LEMBOX":
        raise SystemExit(f"not a LEMBOX: {path}")
    count, name_bytes = struct.unpack_from("<II", data, 6)
    p = 14
    blob = data[p : p + name_bytes]
    o = 0
    names = []
    for _ in range(count):
        (n,) = struct.unpack_from("<I", blob, o)
        o += 4
        names.append(blob[o : o + n].decode("latin-1"))
        o += n
    p += name_bytes
    (c1,) = struct.unpack_from("<I", data, p)
    p += 4
    offs = struct.unpack_from(f"<{c1}I", data, p)
    p += 4 * c1
    (c2,) = struct.unpack_from("<I", data, p)
    p += 4
    sizes = struct.unpack_from(f"<{c2}I", data, p)
    return data, names, offs, sizes


def level_map() -> dict[str, bytes]:
    out: dict[str, bytes] = {}
    for box_name in ("lemmings.box", "patch.box"):
        path = GAME / box_name
        if not path.exists():
            continue
        data, names, offs, sizes = read_box(path)
        n = 0
        for i, name in enumerate(names):
            upper = name.upper()
            if upper.startswith("COMPILED\\") and upper.endswith(".LVL"):
                out[name] = data[offs[i] : offs[i] + sizes[i]]
                n += 1
        print(f"{box_name}: {n} COMPILED *.LVL")
    return out


def iter_matched(blob: bytes):
    hits = []
    start = 0
    while True:
        i = blob.find(NAME, start)
        if i < 0:
            break
        hits.append(i)
        start = i + len(NAME)
    for hi in range(len(hits) - 1):
        payload_at = hits[hi] + len(NAME)
        nxt = hits[hi + 1]
        if nxt < payload_at + 16:
            continue
        payload = nxt - payload_at
        kind, count, u2, u3 = struct.unpack_from("<IIII", blob, payload_at)
        stride = STRIDE.get(kind)
        if not stride or count == 0 or payload != 16 + stride * count:
            continue
        yield kind, count, u2, u3, payload_at + 16, stride


def corners_of(kind: int) -> int:
    return (STRIDE[kind] - 4) // GROUP


def decode_group(rec: bytes, at: int):
    index = struct.unpack_from("<I", rec, at)[0]
    doubles = struct.unpack_from("<6d", rec, at + 4)
    return (index, *doubles)


class Field:
    def __init__(self) -> None:
        self.n = 0
        self.nonfinite = 0
        self.lo = math.inf
        self.hi = -math.inf
        self.in_unit = 0       # [-0.01, 1.01], the band the UV test uses
        self.in_color = 0      # [0, 255]
        self.integer = 0
        self.eq_corner0 = 0    # same slot as this record's first group

    def add(self, val: float, corner0: float) -> None:
        self.n += 1
        if not math.isfinite(val):
            self.nonfinite += 1
            return
        if val < self.lo:
            self.lo = val
        if val > self.hi:
            self.hi = val
        if -0.01 <= val <= 1.01:
            self.in_unit += 1
        if 0.0 <= val <= 255.0:
            self.in_color += 1
        if val == math.trunc(val):
            self.integer += 1
        if val == corner0:
            self.eq_corner0 += 1


class IndexField:
    def __init__(self) -> None:
        self.n = 0
        self.lo = None
        self.hi = None
        self.eq_corner0 = 0
        self.delta_from_prev: Counter[int] = Counter()
        self.delta_overflow = 0

    def add(self, index: int, corner0: int, prev: int | None) -> None:
        self.n += 1
        if self.lo is None or index < self.lo:
            self.lo = index
        if self.hi is None or index > self.hi:
            self.hi = index
        if index == corner0:
            self.eq_corner0 += 1
        if prev is not None:
            d = index - prev
            if d in self.delta_from_prev or len(self.delta_from_prev) < 16:
                self.delta_from_prev[d] += 1
            else:
                self.delta_overflow += 1


class KindStats:
    def __init__(self, kind: int) -> None:
        self.kind = kind
        self.ncorners = corners_of(kind)
        self.levels: set[str] = set()
        self.blocks = 0
        self.records = 0
        self.bad_finite = 0
        self.indices = [IndexField() for _ in range(self.ncorners)]
        self.doubles = [[Field() for _ in range(6)] for _ in range(self.ncorners)]
        self.trailer = Counter()
        self.trailer_overflow = 0
        self.trailer_lo = None
        self.trailer_hi = None
        self.blocks_trailer_varies = 0
        self.trailer_eq_u2 = 0
        self.trailer_eq_u3 = 0
        self.trailer_eq_kind = 0
        self.trailer_eq_count = 0
        self.frac_alpha: Counter[float] = Counter()
        self.samples: list[str] = []

    def add_record(self, rec: bytes, u2: int, u3: int, count: int, level: str, at: int) -> None:
        self.records += 1
        groups = [decode_group(rec, c * GROUP) for c in range(self.ncorners)]
        tail = struct.unpack_from("<I", rec, self.ncorners * GROUP)[0]
        for g in groups:
            for d in g[1:]:
                if not math.isfinite(d):
                    self.bad_finite += 1
            alpha = g[6]
            if math.isfinite(alpha) and alpha != math.trunc(alpha):
                if alpha in self.frac_alpha or len(self.frac_alpha) < 12:
                    self.frac_alpha[alpha] += 1
        c0 = groups[0]
        prev = None
        for ci, g in enumerate(groups):
            self.indices[ci].add(g[0], c0[0], prev)
            prev = g[0]
            for fi, val in enumerate(g[1:]):
                self.doubles[ci][fi].add(val, c0[fi + 1])
        if self.trailer_lo is None or tail < self.trailer_lo:
            self.trailer_lo = tail
        if self.trailer_hi is None or tail > self.trailer_hi:
            self.trailer_hi = tail
        if tail in self.trailer or len(self.trailer) < 24:
            self.trailer[tail] += 1
        else:
            self.trailer_overflow += 1
        if tail == u2:
            self.trailer_eq_u2 += 1
        if tail == u3:
            self.trailer_eq_u3 += 1
        if tail == self.kind:
            self.trailer_eq_kind += 1
        if tail == count:
            self.trailer_eq_count += 1
        if len(self.samples) < 2:
            self.samples.append(self.format_sample(level, at, groups, tail))

    def format_sample(self, level, at, groups, tail) -> str:
        lines = [f"  {level} +{at:#x}  trailing u32={tail}"]
        for ci, g in enumerate(groups):
            nums = ", ".join(f"{FIELDS[i+1]}={g[i+1]:.6g}" for i in range(6))
            lines.append(f"    corner {ci}  index={g[0]}  {nums}")
        return "\n".join(lines)

    def consume_block(self, level, blob, rec0, count, u2, u3) -> None:
        self.blocks += 1
        self.levels.add(level)
        stride = STRIDE[self.kind]
        tails = []
        for n in range(count):
            at = rec0 + n * stride
            rec = blob[at : at + stride]
            self.add_record(rec, u2, u3, count, level, at)
            tails.append(struct.unpack_from("<I", rec, self.ncorners * GROUP)[0])
        if any(t != tails[0] for t in tails):
            self.blocks_trailer_varies += 1


def pct(n, d) -> str:
    if not d:
        return "n/a"
    return f"{100.0 * n / d:.1f}%"


def report(st: KindStats) -> None:
    stride = STRIDE[st.kind]
    print()
    print(
        f"=== kind {st.kind}: {st.ncorners} x 52B + u32 = {st.ncorners * GROUP + 4} "
        f"(stride {stride}) ==="
    )
    print(
        f"levels {len(st.levels)}  blocks {st.blocks}  records {st.records}  "
        f"nonfinite doubles {st.bad_finite}"
    )
    for ci in range(st.ncorners):
        ix = st.indices[ci]
        deltas = ""
        if ci:
            top = ix.delta_from_prev.most_common(6)
            deltas = "  delta-from-prev " + ", ".join(f"{d}:{c}" for d, c in top)
            if ix.delta_overflow:
                deltas += f"  (+{ix.delta_overflow} outside the kept set)"
        print(
            f"  corner {ci} index  n={ix.n}  range [{ix.lo}, {ix.hi}]  "
            f"eq corner0 {pct(ix.eq_corner0, ix.n)}{deltas}"
        )
        for fi, field in enumerate(st.doubles[ci]):
            name = FIELDS[fi + 1]
            if field.nonfinite == field.n:
                print(f"    {name}  all nonfinite")
                continue
            print(
                f"    {name:5}  [{field.lo:.6g}, {field.hi:.6g}]  "
                f"unit {pct(field.in_unit, field.n)}  "
                f"in[0,255] {pct(field.in_color, field.n)}  "
                f"integer {pct(field.integer, field.n)}  "
                f"eq corner0 {pct(field.eq_corner0, field.n)}  "
                f"nonfinite {field.nonfinite}"
            )
    kept = sum(st.trailer.values())
    print(
        f"  trailing u32  range [{st.trailer_lo}, {st.trailer_hi}]  "
        f"kept-distinct {len(st.trailer)} covering {kept}/{st.records}  "
        f"overflow {st.trailer_overflow}  "
        f"blocks where it varies {st.blocks_trailer_varies}/{st.blocks}"
    )
    print(
        f"    eq header u2 {pct(st.trailer_eq_u2, st.records)}  "
        f"eq u3 {pct(st.trailer_eq_u3, st.records)}  "
        f"eq kind {pct(st.trailer_eq_kind, st.records)}  "
        f"eq block count {pct(st.trailer_eq_count, st.records)}"
    )
    top = st.trailer.most_common(12)
    if top:
        print("    top " + ", ".join(f"{v}:{c}" for v, c in top))
    if st.frac_alpha:
        shown = ", ".join(f"{v:.6g}:{c}" for v, c in st.frac_alpha.most_common(12))
        print(f"  non-integer alpha values (capped): {shown}")
    for s in st.samples:
        print(s)


def main() -> None:
    want = [a.upper() for a in sys.argv[1:]]
    levels = level_map()
    print(f"levels visible: {len(levels)}")
    for kind, stride in STRIDE.items():
        n = corners_of(kind)
        ok = n * GROUP + 4 == stride
        print(f"kind {kind}: {n}*52+4 = {n * GROUP + 4} vs stride {stride}  fit={ok}")

    selected = levels
    if want:
        selected = {
            n: b
            for n, b in levels.items()
            if stem_of(n) in want or any(w in stem_of(n) for w in want)
        }
        print(f"filtered to {len(selected)} levels")

    stats = {5: KindStats(5), 7: KindStats(7)}
    per_level = {tag: {5: KindStats(5), 7: KindStats(7)} for tag in FOCUS}
    for name, blob in selected.items():
        stem = stem_of(name)
        for kind, count, u2, u3, rec0, _stride in iter_matched(blob):
            stats[kind].consume_block(name, blob, rec0, count, u2, u3)
            if not want and stem in per_level:
                per_level[stem][kind].consume_block(name, blob, rec0, count, u2, u3)

    print(f"\ncorpus ({len(selected)} levels)")
    for st in stats.values():
        report(st)
    if not want:
        for tag in FOCUS:
            names = [n for n in levels if stem_of(n) == tag]
            print(f"\n----- {tag}  {names} -----")
            for st in per_level[tag].values():
                report(st)


if __name__ == "__main__":
    main()
