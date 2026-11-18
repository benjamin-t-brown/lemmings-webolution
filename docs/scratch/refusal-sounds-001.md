# refusal-sounds-001

Status: **needs_review; smoke pending supervisor execution**. Playable v1,
asset-census-001 rank 5. Difficulty routine; recommendation Sonnet; assigned
Codex override used. Installed first route: **Just Mine, node 5 / MM_MINE**.
Before: the existing metal-refusal branches returned to walking silently.
This source-traced omission is recorded by the accepted [asset census](asset-census-001.md);
human [playtest 11 A8/A11](../playtest/2026-11-16-playtest-11.md) supplies the
broader missing mine/dig feedback context, not a claimed observation of these
specific refusal sounds. Audible refusal is the next bounded audio dependency.

`World::step -> stepBasher/stepMiner/stepDigger` now calls the existing
`cueSample` immediately before each existing refusal exit: **Scrape** for bash
and dig, **MineMetal** for mine. Each attempt emits once even if several probed
cells refuse. The preflights, clear footprints/order, turns, Walker transitions,
frames, clocks, success sounds and skill assignment feedback are unchanged.

Production playback: `cueSample` snapshots actor X/feet Y/radial slab .75 ->
`SoundEvent -> takeSoundEvents -> main.cpp drainCues -> CuePlayback::drain` ->
the current `TubeScene::soundWorldPosition`/camera `spatialSoundGains` resolver ->
`playName -> Mixer::loadFrom` on first use -> `Mixer::play/fill`. Samples use the
existing patch/base overlay, remembered load failures and additive one-shot
policy; their stereo gains remain fixed for each tail. No new audio lifecycle
or mixer behavior was needed.

Static authority: repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 **32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f**.
Reused accepted [Scrape research](../../old/docs/scratch/phase3-round25-scrape.md),
[gameplay sound](gameplay-sounds-001.md), [work sound](work-sounds-001.md) and
[positioned audio](positional-audio-001.md) evidence. No new REA session or
original execution. Retained Ghidra **12.1.4** instruction packets were copied to
[static/](../../build/refusal-sounds-001/static/), retaining their Evidence IDs
and profile digest `3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
[Instruction cross-check](../../build/refusal-sounds-001/static/crosscheck.txt):

| Sample | Registration and refusal trigger |
| --- | --- |
| Scrape | `45E5A0` selects string `4C0CA4`; `45E5B2` stores +`1C8C0`, giving global `4C6A08 + 1C8C0 = 4E32C8`. Bash tests bit 2, facing 4/8 and blocker 800 at `40C0F8/40C104/40C110`; `40C139` loads the handle, `40C148` plays via positioned `445274`, then returns refusal. Dig tests bit 2/directional mask at `415B1B/415B2A`; `415B33` loads Scrape, `415B4A` plays via actor-tracked `44535B`, `415B52` returns to walking. |
| MineMetal | `45E678` selects string `4C0CF0`; `45E68A` stores +`1C8E4`, giving global `4E32EC`. Mine tests bit 2/directional mask at `418AAD/418AB9`; `418AC2` loads MineMetal, `418ADA` plays via `44535B`, `418ADF` marks refusal. It is distinct from successful Mine (`4E32E8`). |

Limits: these are static traces and synthetic clone checks, not an original
listening/comparison run. Once per existing hybrid refusal exit is the clone
policy; native per-cell callback multiplicity and tracking are not reproduced.
Position/gain/tail and overlapping-voice behavior inherit the accepted provisional
audio policy. **Bash/mine directional one-way flags are still absent from the
clone importer/checks**, as documented by basher-two-hit-001; this audio-only
task adds no refusal branch or terrain semantics. Digger's existing upright and
inverted directional refusals now sound. Human listening on a natural metal
contact remains pending; replay success does not establish audible output.

Registered UCRT64 GCC/CMake/Ninja built `webolution`, `test_positional_sound`
and `test_gameplay_sounds`, eight jobs: [build.log](../../build/refusal-sounds-001/build.log).
Focused legacy checks: **2/2 PASS, 0.46 s**, [focused.log](../../build/refusal-sounds-001/focused.log),
[JUnit](../../build/refusal-sounds-001/focused.xml). Added assertions to the
existing legacy positional unit guard one request despite multiple refusing
cells, both facings/seam directions, metal for all three skills, both digger
directional masks, preserved terrain/position/turn/state, no repeated request
after stopping/draining, and actual stereo PCM gains through CuePlayback/Mixer.
Existing work checks still pass. Production decoder reads the installed mono
samples: Scrape **12048 Hz / 4770 frames / 0.395916 s**; MineMetal
**22050 Hz / 18624 frames / 0.844626 s**.

All **19 active promoted human replays** ran individually through
`build/agent-native/main/webolution.exe --game-dir "Lemmings Revolution" --replay <file>`;
all exit **0**, **PASS**, **won=1 completed=1**. Complete paths/outputs and
individual logs: [replays.json](../../build/refusal-sounds-001/replays.json).
No divergence or retirement request; no recording changed.

| Replay level | Saved/required | Final step |
| --- | --- | ---: |
| MM_BASH_002 | 10/10 | 1844 |
| MM_BASH_MINE_WLR1 | 20/10 | 2272 |
| MM_BLOCK_001_A | 24/15 | 1619 |
| MM_BLOCK_BOMB_A | 12/10 | 1546 |
| MM_BOMB | 17/10 | 954 |
| MM_BUILD | 20/10 | 1685 |
| MM_CLIMB_002 | 10/1 | 1252 |
| MM_DIG_000 | 19/10 | 1014 |
| MM_FLOAT | 10/1 | 978 |
| MM_MINE | 20/10 | 1137 |
| MM_W_012 | 16/10 | 1742 |
| MM_E1_3_SKILLS | 10/5 | 2095 |
| M06_KF_1 | 91/50 | 1544 |
| MM_E1_4SKILLS | 20/10 | 2506 |
| MM_E2_4SKILLS | 14/10 | 2142 |
| MM_BASH_DIG_001_A | 20/10 | 3241 |
| MM_BUILD_BLOCK_A | 15/10 | 2482 |
| E19_KF_2 | 24/20 | 6034 |
| E20_MG_1 | 7/5 | 12810 |

The first summary incorrectly checked stdout only; replay results are on stderr.
Corrected classification from the same retained successful outputs, without
rerunning recordings. The initial classification is preserved under build/.

Changed: `src/sim/World.cpp` (three event emissions),
`src/__test__/test_positional_sound.cpp` (legacy invariants/installed decoding),
this report. Task-entry backups/diff are under
[entry/](../../build/refusal-sounds-001/entry/); the task-only patch is
[task-only.patch](../../build/refusal-sounds-001/task-only.patch).
Supervisor PID **18648** owns RepositoryLease. Get-Process found no clone/game/
build runner; detailed CIM metadata was denied. Pre-existing edits are preserved;
no protected controls or shared instructions changed. Text is LF; scoped
whitespace check passes. Child checks use private `build/refusal-sounds-001/appdata`;
replay opens no save. Normal/installed saves were unused. No original execution,
hooks, debugger, capture, installations, subagents, detached work or commits.
**Mandatory smoke remains pending supervisor execution after worker exit.**

Handoff: refusal one-shots integrated; focused legacy checks and all 19 promoted wins pass; bash/mine one-way omission retained.
Next: supervisor smoke and independent review, then human listening; task remains needs_review.

**What to check:** **Just Mine, node 5 / MM_MINE**: let a miner reach metal;
hear one MineMetal clang as it stops mining and returns to walking. On **Just
Bash, node 6 / MM_BASH_002**, bash into metal; on **Just Dig, node 7 /
MM_DIG_000**, dig into metal or an existing blocking directional surface;
hear one Scrape per refusal. Rotate before contact: the new sound should pan
and attenuate with the worker like other level effects, with the same refusal
outcome as before.
