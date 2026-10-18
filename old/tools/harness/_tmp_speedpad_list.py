"""Check whether walkers are on object-overlap list 0x004C88D0."""
from __future__ import annotations

import struct
import sys

sys.path.insert(0, ".")
from lrstate import attach, u32  # noqa: E402
from measure_spawn import LEMMING_VT, OTHER_VT, actor_ptrs  # noqa: E402
from memdump import k32, read_mem  # noqa: E402

LIST = 0x004C88D0


def main() -> int:
    h, _ = attach(write=False)
    meta = read_mem(h, LIST, 32)
    print("meta", [hex(struct.unpack_from("<I", meta, i)[0]) for i in range(0, 32, 4)])
    # try walking as intrusive list from +4 pointer
    node = struct.unpack_from("<I", meta, 4)[0]
    seen = []
    for _ in range(80):
        if not node:
            break
        # common patterns: [lem*, next] or [next, lem*]
        a = u32(h, node) or 0
        b = u32(h, node + 4) or 0
        c = u32(h, node + 8) or 0
        seen.append((node, a, b, c))
        # advance: prefer pointer that looks like another node (heap)
        nxt = b if (b > 0x01000000 and b != node) else (c if c > 0x01000000 else 0)
        if nxt in {n for n, *_ in seen}:
            break
        node = nxt
    print(f"walked {len(seen)} nodes")
    actors = set(actor_ptrs(h))
    for node, a, b, c in seen[:20]:
        flags = []
        for v in (a, b, c):
            if v in actors:
                flags.append(f"LEM:{v:#x}")
            vt = u32(h, v) if v else 0
            if vt in (LEMMING_VT, OTHER_VT):
                flags.append(f"vtlem:{v:#x}")
        print(f"  node {node:#x} [{a:#x},{b:#x},{c:#x}] {flags}")
    # count how many actor_ptrs appear in list payload
    lem_hits = 0
    for node, a, b, c in seen:
        for v in (a, b, c):
            if v in actors or (u32(h, v) in (LEMMING_VT, OTHER_VT) if v else False):
                lem_hits += 1
                break
    print("nodes with lemming", lem_hits, "actors", len(actors))
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
