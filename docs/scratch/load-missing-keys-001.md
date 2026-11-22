# Playtest 27 S2 follow-up · Missing terrain vertices on load

The edited **Just Dig** (`MM_DIG_000`, node 7) was the ceiling test for S2 on
**There’s only one way down!** (`E07_JC_1`). The clone loaded the 141 new earth
cells as solid, but their faces referenced missing corner keys and the slabs
were invisible. `runApplicationLeg` loads the level through `TubeScene::build`,
which now asks `buildCellMeshes` to collect missing ordinary corner keys on
every level with a collision grid and key table. The existing second pass
supplies those vertices through `ensureTerrainVertex` before `TubeScene::render`
draws the terrain. The change is confined to mesh construction.
[Playtest entry](../playtest/2026-11-21-playtest-27.md)
· [Loader](../../src/app/main.cpp#L503)
· [Mesh build](../../src/app/TubeScene.cpp#L165)
· [Terrain render](../../src/app/main.cpp#L835)

**Original static evidence.** Ghidra 12.1.4 read the registered x86
`Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
`0x43A31D` calls `0x43C930` to compute `layer + (x + y * 0x101) * 2`.
At `0x43A356`–`0x43A35A`, the resolver enters the allocation path only for
`-1`. It writes the new index at `0x43A384` and calls the template vertex
routine `0x43D2B8` at `0x43A3AE`. Xrefs and instructions show that the
mesh wrapper calls `0x463410` at `0x46227E`. The face builder tests the
face record’s movable flag at `0x46359B`–`0x4635A0`. Ordinary corners call
`0x43A31D` at `0x4635BD`–`0x463624`, and movable corners call the separate
`0x43A1C6` resolver. The observed path has no level-wide counter-gate test.
The clone’s two-pass collection implements that ordinary key lookup. REA
closed without a snapshot. Original frame fidelity remains unverified.
[REA records and instruction excerpts](../../build/load-missing-keys-001/rea-static.json)

**Installed levels.** The census deduplicated requested `(x, y, layer)` keys
against the stored table using the loader’s stamped collision grid. All 50
levels without counter gates requested zero missing keys. On 11 of the 52
gated levels, gate stamps introduce ordinary corners at the boundaries of
nearby static faces. The existing pass already supplied the 79 distinct keys
across those levels, including six on `H02_JC_1`. All 102 levels had zero
unresolved lookups both before and after. All 102 seeded opening frame PNGs
from the production renderer matched byte for byte.
[Per-level census](../../build/load-missing-keys-001/key-census.tsv)
· [Frame sweep comparison](../../build/load-missing-keys-001/sweep-frame-comparison.tsv)

**Edited Just Dig.** The same 141 cells were solid in both builds. Missing
distinct keys fell from 40 to zero, and unresolved lookups fell from 176 to
zero. Both slabs are absent in the incoming frames and visible after the fix.
The lead’s generator produced the local `patch.box` from the registered
installation, with a hard link to the base archive. The observer used a
private save location. [Patch generation](../../build/load-missing-keys-001/generate-patch.log)
· [Incoming census](../../build/load-missing-keys-001/before-edited/census.tsv)
· [Fixed census](../../build/load-missing-keys-001/after-edited/census.tsv)
· [Slab A before](../../build/load-missing-keys-001/before-edited/slab-a.png)
· [Slab A after](../../build/load-missing-keys-001/after-edited/slab-a.png)
· [Slab C before](../../build/load-missing-keys-001/before-edited/slab-c.png)
· [Slab C after](../../build/load-missing-keys-001/after-edited/slab-c.png)

**Checks.** The release build and `smoke_tests` target built successfully.
Five smoke-labelled checks passed (`actorpose`, `cellmesh`, `frontendentry`,
`levelinteraction`, `levelvisibility`). The promoted **Just Dig** attempt 8
passed through the release `--replay` CLI at step 1014, with 19/10 saved and
`won=1 completed=1`. The frame and key census found no affected installed
route, and the stock Just Dig replay covers the edited level’s base ID.
Scratch executables and objects were removed.
[Incoming release build](../../build/load-missing-keys-001/release-before-build.log)
· [Fixed release build](../../build/load-missing-keys-001/release-after-build.log)
· [Smoke target build](../../build/load-missing-keys-001/smoke-target-build.log)
· [Focused smoke run](../../build/load-missing-keys-001/focused-smoke.log)
· [Replay log](../../build/load-missing-keys-001/replays/MM_DIG_000-1794811135430339400-8.log)
· [Cleanup log](../../build/load-missing-keys-001/cleanup.log)

Handoff **needs_review**. The supervisor smoke profile is pending.\
Next action is independent review and the user’s playtest.

**What to check.** Open **Just Dig** using `build/load-missing-keys-001/edited-data`.
Rotate and zoom around the upper arrow-marked walkway. The two earth slabs
above it should be visible as soon as the level loads.
