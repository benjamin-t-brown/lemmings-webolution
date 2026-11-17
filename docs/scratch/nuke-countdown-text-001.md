# nuke-countdown-text-001

Status: **needs_review**, supervisor smoke pending. Playable v1, playtest 11
[A3](../playtest/2026-11-16-playtest-11.md): the nuke timer must cover the centre
in big text. General defect; check **MM_CLIMB_002 / Just Climb** and **MM_BOMB**.
Before: only small `NUKE 5..1` at (100,380), explicitly provisional in accepted
[nuke-all-001](nuke-all-001.md). This is the next presentation dependency;
neither nuke timing nor the parked Armageddon cadence is reopened.
Difficulty routine; recommendation Sonnet, established HUD compositor and timer;
assigned Codex override used.

Production path: `RuntimeNukeInput::release -> World::nuke -> RuntimeSimState::
advanceElapsed -> World::stepNuke` already owns the five-second timer. The
`runApplicationLeg` HUD block now reads `nukeCountdownMs()` and sends one digit
to `drawEntryStartCue -> entryCueGlyphRects -> ^Font texture composition`, the
same production text path as Let's Go. Digits are drawn after Let's Go and
actor fuse labels, before menus, cursor and pause caption. The small control
hint remains `CLICK AGAIN`/`NUKE`, without duplicate small countdown digits.
No new timer, simulation mutation, release change or audio request.

Static authority: registered repository-local EXE SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
REA/Ghidra **12.1.4**. Complete returned packets, IDs and limitations are under
[static/](../../build/nuke-countdown-text-001/static/). Pseudocode was checked
against instructions; analysis closed without snapshots. No original execution.

- Retained `build/nuke-all-001/retained-static/0x4423D2.asm.txt` and tracked
  `decomp-lemmings/docs/evidence/world_entity_pass.asm`: `4426BF..4426C9`
  construct `413D40(5,5)`; `44279A..4427B2` pass `(320,240,1)` to `414132`.
  `anchor-instructions.json`, evidence
  `ev_c025258ccbdf6aaacec08d027c73499219e5e057cf76ea13e6dee8def955e6af`,
  proves the centred path adds (5,6), giving centre (325,246).
- `update-instructions.json`, evidence
  `ev_3583b71dbe03058cf34c7441a631b46788bae35c3581ad1c01cd948e71c030d3`:
  `413F50..413F58` subtract dt; `413F74..413F7D` truncate `t+1`;
  `414009` uses `%d` at `4BF34C`; `41401A..414020` blank 6;
  `414034/41403B` obtain the level font and build decimal text.
  `font-instructions.json` proves `4151D0` returns level+5570.
  This is font text, not a separate countdown sprite sheet.
- Installed `TEXTURES\\^FONT.BMP` is loaded through `loadTexture` into
  `runtimeFont`. Existing `shell::fontCells()` transcribes table `4BF968`:
  each digit is 13 pixels wide and 29 high. Accepted
  [entry text research](level-entry-start-cue.md) connects the same getter,
  glyph builder `4404FA`, caret-font asset and software compositor.
- `413F61..413F6B` selects factor 10 for kind 5;
  `414040..414067` sets growth `g=(digit-t)*15*(mode+1)*10`.
  `layout-instructions.json`, evidence
  `ev_312852151d245362a417f6f2425ecda987c733b9c31f3cdf2cc3e971f06a2297`:
  `4141AB..4141C2`
  truncate g and 1.3*g, and the centred branch `414209..414244` produces
  `(320-dx,240-dy,330+dx,252+dy)`. `414284..414297` requests additive mode 3.
  `aspect.json` confirms double 1.3 at `4B09B0`. `draw-caller-instructions.json`
  proves `441A99` draws the global overlay via `41419F`.

Clone policy: show **5,4,3,2,1**, one per existing second, and stop drawing at
zero as the existing staggered Exploder phase starts. Blasts/craters still
follow their existing animation delay. Pause holds both digit and expansion;
fast-forward spends the existing timer faster. Each digit expands from 10x12
to roughly 310x400 logical pixels during its second; at half a second its
rectangle is (245,143,405,349), 160x206. It uses the existing additive,
texture-alpha, point-sampled compositor, with no filled background.

**Provisional limits:** the 640x480 adapter fixes the native mode factor at 0;
the precise mode value during the original updater is not established by this
bounded read. Digit selection uses integer-ms ceil rather than reproducing
native `trunc(t+1)` at exact-second boundaries or its blank 6 at exactly t=5.
Native vertex-alpha requests remain subject to the already documented font
material state; no independent device/pixel-fidelity claim. No new rendered
image or human playtest is claimed. Ordinary per-actor Bomber fuses stay
independent; the central timer describes the global nuke phase.

Registered UCRT64 GCC/CMake/Ninja, `-O2 -g`, assertions, eight jobs:
[build.log](../../build/nuke-countdown-text-001/build.log), exit 0.
[Focused checks](../../build/nuke-countdown-text-001/focused.log): **2/2 PASS**,
0.07 s: unchanged `entrystartcue` compositor and `nuke_units` (including pause
and 4999/5000 ms timing). No new tests, replay fixtures or scripted routes.
Supervisor `verify.py --profile smoke` remains **pending after worker exit**.

All **17 active promoted replays PASS**, exit 0, won=1, completed=1, sequential
`webolution --game-dir "Lemmings Revolution" --replay <file>`:
[each exact file, command, exit and output](../../build/nuke-countdown-text-001/replays.json).

| Level | Result; saved/needed |
| --- | --- |
| MM_BASH_002 | PASS; 10/10 |
| MM_BASH_MINE_WLR1 | PASS; 20/10 |
| MM_BLOCK_001_A | PASS; 24/15 |
| MM_BLOCK_BOMB_A | PASS; 12/10 |
| MM_BOMB | PASS; 17/10 |
| MM_BUILD | PASS; 20/10 |
| MM_CLIMB_002 | PASS; 10/1 |
| MM_DIG_000 | PASS; 19/10 |
| MM_FLOAT | PASS; 10/1 |
| MM_MINE | PASS; 20/10 |
| MM_W_012 | PASS; 16/10 |
| MM_E1_3_SKILLS | PASS; 10/5 |
| M06_KF_1 (playtest 11) | PASS; 91/50 |
| MM_E1_4SKILLS | PASS; 20/10 |
| MM_E2_4SKILLS (playtest 11) | PASS; 14/10 |
| MM_BASH_DIG_001_A | PASS; 20/10 |
| MM_BUILD_BLOCK_A | PASS; 15/10 |

No divergence or recording edit. The temporary summary wrapper first checked
stdout only while replay prints to stderr; its summary was corrected from the
retained outputs without rerunning anything. Initial summary is retained too.
Replays check simulation, not the new rendering or human playability.

Changed: `src/app/main.cpp`, `src/app/RuntimeHud.cpp`, this report.
Incoming edits preserved; backups, HEAD main and entry status retained under
[build/nuke-countdown-text-001/](../../build/nuke-countdown-text-001/).
Get-Process checked live runners (CIM metadata access denied); supervisor
PID 15712 owns RepositoryLease. No protected controls edited. Child processes
use private `build/nuke-countdown-text-001/appdata`; replay opens no save and
the existing nuke unit uses its own fixture. No installed/normal save writes,
original/oracles, debugger, hooks, capture, installs, agents, concurrent writers,
commits or pushes. Changed text uses LF; scoped diff check passes.

Handoff: large central nuke digits use the existing text compositor and unchanged timer; two focused checks and all 17 replay wins pass.
Handoff: needs_review; supervisor smoke and independent review next, then the user's playtest confirms A3.

What to check: **MM_CLIMB_002 / Just Climb** or **MM_BOMB**, allow several
lemmings out, double-click Nuke: see 5..1 expand across the screen centre,
then disappear as the explosion wave starts. Pause mid-digit: its value and
size hold; resume and fast-forward: the display follows the existing fuse
phase. Retry clears it; ordinary Bomber assignment alone has no central digit.
