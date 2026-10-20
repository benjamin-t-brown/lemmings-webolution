# Phase A6d11: hatch spawn writes actor `+0x08` (READ)

Brief `build/tmp/a6d11-plus8-spawn-init-brief.md`. Packet
`build/tmp/a6d11-plus8-spawn-init/`. Cursor `round-agent` on `composer-2.5-fast`,
dispatch `6e01a261-5e7f-4ba1-a4a9-1d890206eeb9`. Offline.

## Result

- **READ:** first lasting terrain sample is `rep movsd` at **`0x4230F9`**
  (`ECX=6`) in **`0x4230A2`**, dest `actor+8`, source the ctor 3-double.
- **READ:** hatch release `0x425BF7`–`0x425C30`:
  `movsx` **`+0x9C`** then **`+0x9A`**, `fild`, push doubles, call
  **`0x42DE51`**.
- **READ:** `0x42DE6F` `push 0x3FE00000` / `push 0` is Z **0.5**, then
  `0x4017F0` builds `(X, Y, 0.5)` and `0x42DEA5` calls `0x4230A2`.
- **READ:** `0x42E109` zeros **`+0x360`**, not `+8`.
- **Mapper:** `terrainSampleFromHatchSpawn` = `(double)+0x9A`,
  `(double)+0x9C`, `z = 0.5`. Not wired into the A6d3 inject path.

## Facilitator check

2026-10-20, Cursor facilitator.

- Re-read `0x4230A2` (`add edi, 8` / `rep movsd`), `0x425C04`/`0x425C1A`
  (`movsx` `+0x9C` / `+0x9A`), `0x42DE6F` (`0x3FE00000`), `0x42DEA5`,
  `0x42E109` (`[+0x360]=0`).
- **Narrowed:** `0x4230C5` constructs the `+8` slot (`0x4066A0`) before the
  copy. The hatch sample is the following `rep movsd`, not that ctor.
- Isolated mutations of `ActorTerrain.cpp` (fresh build each):
  - `kNativeSpawnZSlab` `0.5 → 0.75` fails `MM_CLIMB_002 z`,
    `negative X z`, and `Z is not motion slab 0.75`.
  - `out.x = hatch.y` fails both `x` checks and `X/Y independence`.
  Restored SHA matches backup (LF).
- `test_actorterrain` **8** checks (worker said 6). **24/24 CTest** after restore.
  Phase-2 hatch words 32/42 on MM_CLIMB_002 are the test fixture, not a new
  live read. No game.

## Parked / next

Spawn sample is closed. Ongoing `+0x08` after the first tick (walk remainder,
fall Y, live Z 0.75) is still open. Resume with the **first post-spawn
`+0x10` write**, not another hatch `+0x9A` pass. Evidence: this note,
[A6d10](phase-a6-sim-plus8-writer.md).
