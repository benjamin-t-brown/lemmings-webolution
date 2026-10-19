# A4b — the cell mesh is built at level load from the collision grid; exact scan still open

2026-10-19, branch `decompile`. **A4b checkpoint, not a finished generator.**
The round located the producer chain of the per-cell tube quads at `0x4ECB10` and pinned
its key instructions. It did **not** decode the exact face-generation scan, so no production
rule was implemented and no pixel score changed. The A4a `0/9` result and the A2/A3 retained
failures stand.

Authority: stock 640x480 `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
No live game process was started or driven. REA (Ghidra) was opened read-only and closed.

## Verdict

The runtime per-cell quads are **generated at level load**, not read from the stored
`PolyList 09` records or the stored per-cell topology. The generator's input is the level's
**runtime collision grid** — the same 256x96 `u32` mask the simulation collides against,
whose bytes are already recorded as identical to the stored grid. The stored 897-vertex
`CylinderMesh::surface` pool remains a valid vertex bank; what production still lacks is the
rule that turns the grid into the per-cell face list.

## Pinned producer chain (independently re-read)

All addresses below were re-read with `tools/harness/read_steppers.py`; the raw-byte audit is
`tools/harness/audit_cellmesh_producer.py` (21 pinned windows, 7 decoded call edges, all pass).

| Address | Bytes | READ |
| --- | --- | --- |
| `0x43A41A` | `e8 25 6f 02 00` | `call 0x461344` before the bucket sweep |
| `0x461344` | `b9 10 cb 4e 00` / `e8 1f 1d 00 00` | `ecx=0x4ECB10`, `call 0x463070` |
| `0x463070` | `89 88 00 00 0a 00` | resets `[pool+0xA0000] = pool` |
| `0x43A431` | `83 7d fc 20` | bucket sweep bound `0x20` |
| `0x43A43A` | `c1 e1 03` | `shl ecx,3` (group argument `bucket*8`) |
| `0x43A447` | `8d 8c 10 08 02 00 00` | `lea ecx,[world + bucket*0x10 + 0x208]` |
| `0x43A44E` | `e8 68 64 02 00` | `call 0x4608BB` per bucket |
| `0x460A2D` | `89 b0 c8 00 00 00` | store material at `entry+0xC8` |
| `0x460A41` | `e8 1e 18 00 00` | `call 0x462264` per band |
| `0x46226E` | `e8 76 f3 ff ff` | `call 0x4615E9` (face/side lists) |
| `0x462276` | `e8 85 0e 00 00` | `call 0x463100` |
| `0x46227E` | `e8 8d 11 00 00` | `call 0x463410` (mesh0/mesh4 assembly) |
| `0x463497` / `0x463F07` | `e8 eb ed ff ff` / `e8 7b e3 ff ff` | mesh0 / mesh4 allocation via `0x462287` |
| `0x4622ED` | `b9 10 cb 4e 00` | mesh pool object literal `0x4ECB10` |
| `0x4622F2` | `e8 a5 2c 00 00` | `call 0x464F9C` bump allocator |
| `0x464F9C` | `8b 88 00 00 0a 00` / `c1 e2 07` / `8d 54 11 10` | bumps `[pool+0xA0000]` by `count*0x80 + 0x10` |
| `0x465011` | `6a 02` / `e8 f9 4b 00 00` | `push 2`, `call 0x469C14` (header stamp) |
| `0x40DE60` | `25 ff 00 00 00` / `c1 e2 08` / `03 c2` / `add ecx,0x41c` | collision cell: `x&0xff`, `y<<8`, index `x+y*0x100`, base `world+0x41C` |

The collision accessor also clamps `y` to `[0, [world+0x418]-1]`, matching the 96-row mask.
The allocator's `+0xA0000` bump is the direct writer of the arena object headers; the
mesh object's `kind=2` header stamp is confirmed at `0x465011`.

## What is measured, read, and still open

**Measured (independent):** the pinned bytes and call edges above; the retained arena hash
`f96bcf7…` and its 389 mesh0 quads (389 distinct index sets, max index 896); the stored pool
is byte-identical to the live 897-vertex bank; the 156/156 outer-wall presence rule from A4a.

**READ:** the chain is the level-load mesh builder; `FUN_0043A1C6` gets/creates vertices and
`FUN_0043D2B8` interpolates the cylinder template at `world+0x18`; the mesh0/mesh4 assembly
uses the scratch lists at `0x4ECA80`/`0x4ECA58`/`0x4ECA48`/`0x4ECA68`.

**Inferred, not measured:** that the `.LVL` “bakes the same generator's output” into the
897-vertex pool. What is measured is that the stored pool and the live vertex bank are
byte-identical; *why* they match is an inference. Do not promote the inference as a byte
fact.

**Still open (the critical gap):**

1. The exact face-generation scan inside `FUN_004615E9` (`0x4615E9..0x461E1C`, 2100 bytes):
   which collision-cell predicate emits which face, how the face's short fields and the
   four range pairs are computed, and how a cell's bucket/band window restricts them.
2. The eight `0x10`-byte part descriptors at `0x4ECA90` built by `FUN_00461E1D`
   (`0x461E1D..0x462263`): the per-part UV/colour descriptors.
3. The colour operands `0x4B1820/24/28/30` and the `entry+0x34..0x3B` path.
4. No all-level sweep: the producer chain is verified on `MM_CLIMB_002` only.

The round's “no reader of the stored surface/grid0/PolyList” statement is explicitly
bounded: `FUN_00461E1D` was not enumerated. Keep it bounded.

## Because no rule was finished

- No production code, renderer, or test changed.
- No nine-view re-score was run. The A4a result stands: `0/9` retained views pass, best
  `initial` `0.037812306`; the A2 wider `0.022030839`, cavity/rim, and A3 sensitivity
  failures are unchanged.
- The executable and its 17/17 test suite are unchanged. The facilitator re-ran
  `ctest --preset dev-debug`: **17/17 pass**.

## Next question

Decode `FUN_004615E9` and `FUN_00461E1D` against the stored `CollisionGrid`/`LevelIndex`,
then build an **offline** candidate generator and validate it against the retained arena as
an oracle before touching production. Only after a per-cell match should the nine retained
views be re-rendered under the unchanged gate. The SDL window/event loop and controls remain
a later round.

## Reproduction

```text
python tools/harness/audit_cellmesh_producer.py          # static pins + arena facts
python tools/harness/read_steppers.py --at 0x0043A41A --len 0x50
python tools/harness/read_steppers.py --at 0x004622ED --len 0x18
python tools/harness/read_steppers.py --at 0x00464F9C --len 0x60
python tools/harness/read_steppers.py --at 0x0040DE60 --len 0x50
```

Local ARENA/PINS artifacts are under ignored `build/tmp/a4-cell-mesh-source/`; the static
audit script is tracked under `tools/harness/`.
