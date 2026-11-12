# playable-expansion-001: Blocker, Builder and next-gadget assessment

Status: assessment executed (resumed run); task left `needs_review`. Everything below is a **scripted clone observation**
(SDL events queued by the source-built driver `src/__test__/test_playableexpansion.cpp` through the public
`runApplication`). It is not human play, not original timing and not original-fidelity evidence, and it does not show that any
level is or is not winnable. No production file changed; no original EXE/oracle/debugger/hook/capture ran.
Retained artifacts (ignored by git): `build/playable-expansion-001/` — survey in `survey/`, the verifier run in
`runs/20261103-003501/` (this report cites only that run; a later verifier run writes a new `runs/<stamp>/` and does not
overwrite it, but `survey/` and `current-run.txt` are overwritten by later runs).

## Resume provenance
Previous worker stopped on Claude `usage_limit_reached` (HTTP 429) with the driver unverified and unregistered (entry SHA-256
`24f99d6a…e533b4`). This resume read the whole driver, found it complete, **changed nothing in it** (SHA-256 unchanged), added
only the CMake registration (`playableexpansion_{survey,play_N,restart_N}`, N=1..4, ordered play N -> restart N -> play N+1) and ran
the registered verifier. It compiled with one warning (`dropAhead` unused, driver line 252; left in place so the retained
run matches the driver hash — harmless dead helper).

## Map unlock dependencies (`survey/map-graph.txt`, derived read-only with in-memory flags)
Fresh map: nodes 0, 1, 2 selectable. Completing node 0 unlocks **3 (MM_BLOCK_001_A) and 4 (MM_BUILD)**; so each core level needs exactly
one genuine win (node 0) — sequential order is not required. Gadget candidates by wins required from a fresh map:
**MM_W_012 (node 8, 4 levers): 2 wins, chain 0 -> 3 -> 8** (requires winning node 3); then E19_KF_2 (19), E20_mg_1 (21), MM_ALL_8_001 (22)
at 4; MM_TELE_003 (38, teleports) at 6; M21_KF_1 (51, speed pad + teleports) at 7. Every lever level is behind node 3 or node 4's descendants,
so no gadget level is reachable without a win on a core level or a different chain.

## Route / outcome matrix (run `20261103-003501`, one isolated save created fresh by step 1)
| Step | Level (node) | Entry | Attempts | Outcome | Persistence (fresh process) |
| --- | --- | --- | --- | --- | --- |
| 1 | MM_CLIMB_002 (0), Climber x10 | startup -> Play Current Game -> map click (steps 2/3 clicked map pixels 240,179 and 240,212) | 1 | **WIN** 10/10 need 1, 33.4 s wall / 214.2 s sim, 10 clicks all assign-ok | **VERIFIED**: `restart-result.md`, flag[0]=1, 1504-byte file, fnv `c05382077a330a32` unchanged by loading, flags/states equal play map |
| 2 | MM_BLOCK_001_A (3), Blocker x10, 30 lemmings need 15 | same path, from the earned map (states `2,3,3,3,3,1…`: nodes 3,4 selectable) | 2 | **LOSS, LOSS**: 4/30, then 7/30 saved | **UNVERIFIED** (no win) |
| 3 | MM_BUILD (4), Builder x10, 20 lemmings need 10 | same | 2 | **LOSS, LOSS**: 0/20 and 0/20 saved | **UNVERIFIED** (no win) |
| 4 | gadget | map inspected after step 3 | 0 | **BLOCKED**: no selectable gadget level; nothing entered, level field empty (`play-result.txt` prints node=8 as the preferred candidate) | n/a |

Core levels were both reachable with only genuinely earned progress (flags after step 1 = node 0 only; `flags_at_map_before` in steps 2/3
show a single `1`). No prepared flags, no direct-level substitute, no injected wins, no actor/terrain/stock/clock writes. Normal user save
`%LOCALAPPDATA%/LemmingsWebolution/SavedGame.dat` was **absent before and after** (`normal_save_unchanged=1` in each play-result); the
isolated save fnv stayed `c05382077a330a32` through steps 2-3 (losses wrote nothing; `saveCount=0`). Fast-forward plate (54,348) was clicked
once per attempt via the public path (trace `click-fast-forward`; ~16 ticks per presented frame afterwards); nothing else (no pause).

### Step 2 MM_BLOCK_001_A (policy: Blocker at a real drop edge, hazard below, spacing 24, cap 4; revision 2 = cap 8, spacing 14)
Terrain (survey): 896 water/acid cells, 1 hatch, 1 exit, no gadgets, stock Blocker 10 only, limit 1500 ticks.
* Attempt 1 (wall 46.8 s, 1526 ticks): 4 clicks -> stock 10->6, 4 assign-ok. Blocker state observed on lemming 0 (179/24, intended = picked) and lemming 9 (151/65, intended = picked). Clicks on intended 3 (83/45) and 5 (176/65) spent stock with no Blocker transition found in the 4-frame window by `picked`, but the end dump lists them as `blocker` (3:blocker@83/45, 5:blocker@176/65): so those are **driver observation-window misses (late transition), not lost stock**. Real count 4 Blockers.
* Observed Blocker effects: lemming 9's blocker turned 20 distinct walkers around (`blocker-turn`, `turned_near=20`), lemming 0's 3. `field_cells_seen=16` for each standing blocker.
* Outcome: 4 saved, 2 dead, 24 still walking back and forth at y=65 (x 153-175) when the 1500-tick limit hit.
* Attempt 2 (revMode 2, chosen because attempt 1 placed Blockers but lost): same four sites; 7 saved, 2 dead, 21 active. Loss again.
* Anomaly candidate (not isolated): `blocker-crossed` fired 4 times (attempt 1, blocker 9) and 6 times (attempt 2, blocker 12): a Walker whose x offset from a standing Blocker changed sign within |dx|<=6 across **one presented frame (~17 ticks)**, e.g. `lemming=17 dx=-11->4`. Per-frame sampling cannot tell "walked through a Blocker field" from "turned at the field and was seen on the far side after a second turn", so this is **unverified, not a demonstrated defect**.
* Driver/policy limitation, **not a clone defect**: the first Blocker (179/24, top ledge) stands on the route for the whole level (1432 ticks) and turned only 3 walkers; it plausibly cuts the flow it was supposed to protect. Blocker cannot be removed (no skill for it), so the one-shot policy cannot revise this site once placed. No Blocker/Basher/Miner/Digger combination was tried because only Blocker stock exists.

### Step 3 MM_BUILD (policy: Builder when a wall/gap ahead; revision 2 = cap 9, shorter spacing)
Terrain: 0 hazard cells, 1 hatch, 1 exit, stock Builder 10, limit 1500 ticks.
* Attempt 1 (46.8 s, 1529 ticks): 4 clicks, stock 10->6, 4 assign-ok. Builders: lemming 1 (183/68, face -1): `bricks_first=8`, `bricks_min=1`, `builder_frames=3` (~50 ticks), `new_solid_cells=16` in a 65x40 window; lemming 14 (155/68): left Builder after 1 frame with `bricks_min=8` and 1 new solid cell. Clicks 1 and 4 (intended 8 at 242/46 and 5 at 188/72) show `picked=-` with stock spent; no Builder transition was recorded for them within the window; the end dump shows no Builder, so I cannot say what those two assignments did (possible late application, or a click that landed on a non-walking lemming — `hovered_snapshot` differed from the intended index every time, `hovered=12` vs intended 8, `13` vs 12, `1` vs 3, `1` vs 5). That is **targeting ambiguity in the driver (box-centre projected clicks) until proven otherwise**.
* Outcome: 0 saved, 0 dead, all 20 lemmings alive and walking at y≈67-72, x 154-188, at the time limit. Nobody reached the exit.
* Attempt 2 (revMode 2, because attempt 1 placed Builders but 0 saved): 4 clicks, 10->6; Builders lemming 3 (163/68, face +1, 3 frames, bricks 8->1, 16 new cells) and lemming 12 (166/65, 6 cells). 0 saved, 0 dead, 20 active.
* `builder-follower-past-end` is a loose metric (any alive lemming within 40 cells ahead of the Builder's final position and |dy|<=8). All 19 others satisfied it in each attempt because the whole population mills in one pocket; it is **not evidence of traversal** and must not be read as such. Builder-created terrain (16 new solid cells, 8 bricks consumed to 1) was observed, but the step height/length and whether the bridge connected anywhere were not measured and no lemming left the pocket.
* In all 4 observed Builder transitions the lemming that became a Builder was a different index from the intended one (attempt 1: intended 12 -> 1, intended 3 -> 14; attempt 2: intended 14 -> 3, intended 2 -> 12; `picked_intended=0`), i.e. the intended site was not achieved in any attempt. The population moves ~16 ticks between decision and click resolution at fast-forward, so the intended lemming has walked away — a driver timing/targeting limitation, not shown to be a clone defect.

## Ranked findings
1. **Core Blocker/Builder levels were not won by two bounded policies each; gadget coverage remains unverified.** Both losses are with a working skill path (skill selected by key, stock decremented, Blocker/Builder states observed, terrain bricks added, walkers turned), which is a clone **success** for assignment/Blocker-turn/Builder-placement, but no winning route exists in evidence. Outcome is not a demonstrated clone defect: the policies are heuristic, and the bounded two-attempt rule means this proves nothing about solvability.
2. **Click targeting is unreliable in a crowd** (driver/pick limitation): `picked_intended` 2 of 4 clicks for Blocker (both attempts; the other 2 per attempt were late-transition misses of the 4-frame window, end dump shows them as Blockers) and 0 of 4 for Builder (`picked_other=2`, `picked_none=2` per attempt). Same limitation as the baseline's finding 3. A hit on an unintended lemming spent the one-way skill and made one Builder start at a wrong site.
3. **Candidate defect, unisolated: lemmings crossing a standing Blocker** (`blocker-crossed` 4 and 6 events). Needs per-tick observation (not per-frame at ~16 ticks) to settle.
4. **Builder `bricksLeft` starts at 8** in the traces (`bricks_first=8`); the source comments in `World.h` refer to a twelfth brick. Not investigated — possible rules question, not a demonstrated failure of anything here.
5. Gadget interaction (lever / teleport / speed pad) has **zero** observation; node 8 needs a win on node 3 first. No claim.
6. No crash, hang, save error or anomaly (`anomaly=-`, `runApplication_status=0` in every step).

## Smallest justified next step
Further play investigation (not a production repair): one bounded follow-up that (a) runs MM_BLOCK_001_A with per-tick or paused-frame sampling to settle finding 3, (b) targets Builder/Blocker clicks by the picked actor index (the driver already logs `hovered_snapshot`) instead of box centre, and (c) uses the terrain dumps `survey/terrain-node{3,4,8}.txt` to pick Builder sites on the real route. A win on node 3 opens node 8 (MM_W_012) immediately. Only a demonstrated failure of a correctly targeted and played level justifies a production fix.

## Checks
* `python tools/agent_control/verify.py --profile main` (foreground, moved to background at 600 s, not restarted; exit 0): **131/131 registered tests passed**. New tests: `playableexpansion_survey` 12.63 s, `play_1` 35.06 s, `restart_1` 0.27 s, `play_2` 93.73 s, `restart_2` 0.03 s, `play_3` 94.15 s, `restart_3` 0.03 s, `play_4` 12.97 s, `restart_4` 0.03 s. These assert driver integrity only (status 0, entered level == target, results observed, outputs written), never gameplay success; passing 131/131 does **not** mean the levels were won.
* `python tools/agent_control/verify.py --profile recovery` (foreground, exit 0; its `git diff --check` step printed no findings): **84/84 passed**, 30.3 s. I did not run `--profile all` as a single command; main and recovery were run separately, and the supervisor's independent run is authoritative for the combined profile. Existing assertions and timeouts were not changed.
* Changed files: `CMakeLists.txt` (registration only), `src/__test__/test_playableexpansion.cpp` (the retained driver, unchanged by this resume), this report.
* Process note: this worker used read-only `ls/wc/sed/sha256sum` Bash calls before noticing the restricted command set; no write, build or game command was run other than the verifier.

## Unknowns
Whether a better-targeted Blocker/Builder policy wins either level; whether `blocker-crossed` is a real pass-through; what two of four Builder assignments per attempt did; whether Builder `bricksLeft` initial value matches retail; all gadget behaviour; persistence of any core win (no win occurred).
