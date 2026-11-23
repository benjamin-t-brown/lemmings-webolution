# Miner one-way terrain on One Way Ticket

**One Way Ticket** (`MM_ALL_8_001`, map node 22). Playtest 13 C4 locates the
reverse-bash zone. The Miner bypass was traced in clone source. Its steel-only
preflight let both approaches reach `clearSkillBox` on directional earth.
[Playtest 13](../playtest/2026-11-17-playtest-13.md) · [Incoming World.cpp](../../build/miner-one-way-001/World.HEAD.cpp)

The map click selects the installed level, then `loadRetailLevelReport` imports
the collision grid through `Terrain::fromCollisionGrid`. The Miner panel and
actor click reach `World::assign`. `RuntimeSimState::advanceElapsed` advances
the world to `World::stepMiner`, which now uses the live directional flag query
in its existing steel preflight. The refusal branch queues MineMetal once,
notifies the axe presentation, turns the lemming and returns to Walker before
any clear. A permitted stroke still cuts and advances diagonally.
[Map click](../../src/app/FrontendSession.cpp#L443) · [Loader](../../src/sim/RetailLevel.cpp#L336) · [Assignment](../../src/app/main.cpp#L1287) · [Elapsed update](../../src/app/RuntimeHud.cpp#L142) · [Miner](../../src/sim/World.cpp#L1110)

The installed grid starts at byte 255753. From file feet (70,53), clone feet
(70,42), the right-facing footprint reaches file cells (71,52) and (72,52).
The left-facing footprint reaches (69,52) and (68,52). All four are `0x05`
and both footprints are steel-free.
[Installed cell reads](../../build/miner-one-way-001/scout.log)

Ghidra 12.1.4 analyzed the registered `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Instructions at `0x4189B0..0x4189C0` select `0x04` for right and `0x08`
for left. Steel or a matching bit takes the branch at `0x418AAD..0x418ADF`,
which loads MineMetal and bypasses that cell's clear at `0x418B29`. Pseudocode
agrees and five direct xrefs reach helper `0x4188F6`. The clone
provisionally preflights the whole stroke. The original checks individual cells.
REA closed without a snapshot.
[Static evidence](../../build/miner-one-way-001/rea-miner.json)

Both clone observations used the connected SDL map, panel and actor clicks.
The diagnostic placed one Walker before each assignment with a camera aimed
at the contact. At 208 ms the right-facing Miner turned left in place with one
MineMetal request and zero mesh edits. The left-facing Miner advanced to
(69,43), queued Mine, and cleared two cells. Terrain sync rebuilt one mesh
block and the level view shows a notch below the lemming.
[Probe](../../build/miner-one-way-001/contact.cpp) · [Right run](../../build/miner-one-way-001/contact-right-final.log) · [Left run](../../build/miner-one-way-001/contact-left-final.log) · [Left before stroke](../../build/miner-one-way-001/left-before-stroke.png) · [Left after stroke](../../build/miner-one-way-001/left-after-stroke.png)

The release build passed. All nine promoted recordings with accepted Miner
assignments reproduced, including this level at step 13812 with 46/40 saved.
The promoted Now You're Stalking replay (`E22_KF_1`) also reproduced its
Basher assignments at steps 5366 and 17073 and passed at step 24402 with
49/49 saved. The replay reported no divergence.
[Now You're Stalking replay log](../../build/miner-one-way-001/replays/E22_KF_1-1795077450383129100-7.log)

The SDL observations used private 1,504-byte saves and preserved the prepared
flag and A/B fields. Replays used the save-free path. The existing legacy Miner
test builds with a guard for both masks and both facings. Its execution is
pending milestone sign-off.
[Release build](../../build/miner-one-way-001/build.log) · [Replay results](../../build/miner-one-way-001/replays.tsv) · [Legacy build](../../build/miner-one-way-001/legacy-miner-axe-build-final.log) · [Source diff](../../build/miner-one-way-001/task-source.patch)

Handoff **needs_review**. Supervisor smoke, including the 102-level sweep and recovery suite, is pending.\
Next action is independent review after smoke, then human confirmation.

**What to check.** In One Way Ticket, assign Miner on the arrow ledge near
file feet (70,53). Facing right, the lemming should turn with one metallic cue
and leave the ledge intact. Facing left, it should open a visible diagonal cut.
