# Basher one-way walls

**Now You're Stalking** (`E22_KF_1`, map node 27). Playtest 18 H2 identified
rendering seams on these arrow blocks. The Basher defect was traced in source.
The terrain importer kept the arrow bits in the gate restoration baseline, but
the Basher only saw both walls as Earth. The seams fix was accepted separately.
[Playtest 18](../playtest/2026-11-18-playtest-18.md) · [Arrow-block census](../../build/stalking-seams-001/tube-probe-before.log)

## Installed route and static evidence

`MapScene::build` maps node 27 to group 1, slot 1. `FrontendSession::clickMap`
selects that ID and `runApplicationLeg` passes it through `loadRetailLevel`
and `loadRetailLevelReport` to `Terrain::fromCollisionGrid`. The read-only
probe used those map and level loaders. The 96-row grid maps file Y to
terrain Y as `95 - fileY`. [Installed route probe](../../build/basher-one-way-001/installed-route-probe.log)

| File cell | Terrain cell | Word | Refuses | Permits |
| --- | --- | --- | --- | --- |
| (160, 29) | (160, 66) | `0x05` | Facing right | Facing left |
| (189, 29) | (189, 66) | `0x09` | Facing left | Facing right |

Ghidra 12.1.4 analyzed the registered `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Instructions at `0x40C0DC..0x40C109` select and test the facing mask.
The refusal loads the Scrape handle at `0x40C139`, calls playback at
`0x40C148` and returns refusal. The permitted arm zeroes the cell at
`0x40C16D`. Ghidra pseudocode agrees with these branches. REA closed without
a snapshot. [Static evidence and source trace](../../build/basher-one-way-001/source-trace.md)

## Production behavior

The panel selection and actor click reach `World::assign`.
`RuntimeSimState::advanceElapsed` advances the world and dispatches the
Basher step on its normal tick boundary. Each scheduled punch checks its own
rows against the active flags. Refusal queues one Scrape and turns the actor
back into a Walker before the punch writes terrain. The existing whole-footprint
steel check uses the same branch. From the permitted side, the lower and upper
punches use `clearSkillBox` as before. `Terrain::set` clears the live flag
and increments the version, so the terrain mesh updates and a replacement
Builder brick has no directional flag.
[Basher step](../../src/sim/World.cpp#L1070) · [Terrain](../../src/sim/Terrain.cpp) · [Mesh sync](../../src/app/TubeScene.cpp#L1726)

The clone provisionally refuses the entire coalesced punch if any cell in its
footprint refuses. The original checks one cell at a time. An earlier
successful lower punch stays cleared if the upper punch later refuses.

## Checks

Registered UCRT64 GCC 16.1.0, CMake and Ninja built `webolution` and
`test_basher_two_hit`. The existing legacy test now covers both masks, lower
and upper refusal with one Scrape, and cleared-cell reuse. Its execution is
pending milestone sign-off. The read-only installed-data probe confirmed
240 exact `0x05` and 256 exact `0x09` cells in buckets 20–25, bands 1–2,
all with the expected active facing rule. The probe opened no save.
[Configure](../../build/basher-one-way-001/configure.log) · [Build](../../build/basher-one-way-001/build.log) · [Final build](../../build/basher-one-way-001/build-final.log) · [Probe runs](../../build/basher-one-way-001/source-trace.md#checks)

- **Handoff** — `needs_review`. Supervisor smoke, including the 102-level sweep and recovery suite, is pending.
- **Next action** — Review the diff and smoke result, then record the human outcome and any remaining route blocker. Playtest 18 had no win for node 27, and the new behavior awaits human confirmation.

**What to check** — Enter node 27 from the map. Above the steel rail at file
feet Y 29, face right from X 158 into the `0x05` wall at X 159, and face left
from X 205 into the `0x09` wall at X 204. Each should stay intact with one
Scrape and a turn. From the central gap, face left from X 175 into X 174 or
right from X 188 into X 189. Each should open a visible passage bottom first,
then top. These positions derive from the installed collision geometry.
