# Project status

Updated 2026-11-12 from accepted queue/review evidence. Native Windows C++ build;
not complete original-game fidelity. Current phase: [playable v1; capped verification work first](../agent/IMPLEMENTATION_PLAN.md).

## Current implementation
| Area | Accepted scope | Still unproven or incomplete |
| --- | --- | --- |
| Startup/frontend | Ordinary startup, Play Current Game navigation and existing map/game/result routes connected. | Broader input/return/options parity and ordinary end-to-end play. |
| Saves | Original-format codec and clone-owned persistent progress across restarts accepted. | Three scripted level wins persisted across restart; broader compatibility and natural-play coverage remain unproven. |
| Level data | Structured assets, hatches, collision/light/surface data, skill stock and cylinder loading connected. | Loading success is not gameplay correctness. |
| Rendering | Structural/nested scenery support admits all 102 installed levels; selected-hatch camera integration and MM_W_012 base-fluid rendering accepted. | All-level visual correctness unproved; C2 open, historical C3 FAIL unchanged. |
| Gameplay | World/skills/gadgets exist; serialized teleport pairing and fast-forward accepted. | Timing/release/digger discrepancies, broad play-through evidence, Armageddon. |
| Audio/media | Gameplay cue routing/mixer integration accepted. | Full cue coverage, music/media and fidelity remain incomplete. |
| Platforms | Registered MinGW-w64 Windows builds and source tests. | Emscripten and other platforms are not accepted. |

Evidence authorities: [task records](../agent/tasks.json), accepted per-task reports in
scratch/, and independent decisions under build/agent-runs/.
Live queue/run state and exact verification links: [generated ledger](../agent/RUN_STATUS.md).
Latest accepted task: fluid-lever-assessment-001 (2026-11-12).
[Final review](../build/agent-runs/loop-7bcd0ea7da8241b58ce68791670dbbc5/0008-review-73effc/decision.json)
accepted the three-stage fluid milestone; the loop stopped after4h25m32s.
The ordinary MM_W_012 route activated lever(183,60), visibly filled the tank,
and won20/20. Reload drained it; private saves and fresh-process persistence
were preserved. Reviewed images cover initial/rising/settled/reload and two cameras.
No ordinary second contact was observed: review accepted explicit source/data
inference that another pull is visually inert. Do not call that an observed repeat.
[Assessment report](scratch/fluid-lever-assessment-001.md) retains reproduction
steps, image index and limitations.
[Final verification](../build/agent-runs/20261112T212549-7804-08300df1/verification.log):
**148 main /84 recovery tests passed**; not a gameplay-completion percentage.
Original depth state remains unresolved under provisional Less/write-on. Overlays,
pipes, spray, drowning, other levers/levels and broader hybrid timing are unproven
or omitted within this milestone. Historical gates below remain unchanged.
Earlier Blocker28/30 and scripted Climber/Floater/Bomber wins remain scoped evidence;
all102 map spellings resolve/load, without implying all-level gameplay correctness.

Selected reports: [startup](scratch/native-017-default-frontend-startup.md),
[saves](scratch/save-codec-001.md), [commands](scratch/gameplay-commands-research-001.md),
[fast-forward](scratch/fastforward-impl-001.md).
The older product summary is archived in plan-history/documentation-2026-11-02/STATUS.md.

## What happens next
User-approved direction (2026-11-12): playable v1, every installed level winnable in
the clone from the ordinary frontend route. Clone wins recorded so far: 5/102 (map
nodes 0-3 and 8). First: test-parallel-isolation-001 (queued, not started) and a
four-job benchmark; preflight and verification reuse are deferred. Then a user
playtest and/or an all-level smoke sweep to rank defects. Timing, full-frame and
original-comparison gates below are set aside for this milestone, not resolved, and
remain required for full completion. Compile jobs12 /tests1 remain unchanged.

## Evidence labels
Implemented = connected source; source-tested = bounded executable test;
clone-observed = recorded clone route; original-verified = explicitly scoped
comparison. None automatically implies the others.
Historical gates below are retained verbatim; this update is not a new measurement.

## Retained gate ledger

Scores below retain their linked observation/re-score dates; editing this page is
not a new measurement. Exact gate wording and rules live in [legacy-gates](acceptance/legacy-gates.md). Do not copy
them into another active table. A “pass” here retains every original exclusion.

| Gate | Retained status / scope | Evidence or next missing comparison |
| --- | --- | --- |
| A1 | PASS: four camera views/clamps; 473 control points. | [Phase A history](plan-history/phase-a.md) |
| A2 | PASS: fixed tube ROI, nine views + exposed INN. | Wider scene was not included; [Phase A](plan-history/phase-a.md). |
| A3 | PASS: 608 branch cases, 42/42 actor-bound index/mirror cases. | Selection evidence, not every animation/frame; [Phase A](plan-history/phase-a.md). |
| A4 | PASS: production 3D route and SDL controls, A4d/e. | [Phase A](plan-history/phase-a.md) |
| A5 | PASS: two zoom-0 actor fixtures. | Zoom-1 filter/alpha remains a [residual](plan-history/phase-e.md#actor-filter-and-alpha-at-zoom-1). |
| A6 | PASS: amended repair-4 pause, tube ROI 0.015584142 and three actor boxes. | Not gameplay acceptance; [slope walker](plan-history/phase-e.md#a6d3-slope-walker) unresolved. |
| A7 | PASS: nine first-level views plus one second-level view, amended HUD exclusions. | Full-frame completeness still requires C2/C3; [Phase A](plan-history/phase-a.md). |
| B1 | CONDITIONAL PASS: menu highlight 0, 0.005244. | Panel geometry/colour re-score condition; [B1w](scratch/phase-b1w-menu-render.md), [Phase B](plan-history/phase-b.md). |
| B2 | OPEN; bounded [loss/Retry/Quit](scratch/frontend-loss-retry-quit-2026-10-28.md), [No/mouse Resume](scratch/frontend-no-resume-comparison-2026-10-28.md) and earlier routes PASS. Blanket nonzero-RETURN quirk **contradicted** by row1→Map4/UI3. | [Failed negative prediction](scratch/frontend-return-negative-prediction-2026-10-28.md); signed wording unchanged pending correction. Loss Continue, broader Resume/input and startup progress parity remain open. |
| B3 | Bounded PASS: fresh/completed level-select map full frames0.007834584 /0.007834665; normal first-node selection reaches `MM_CLIMB_002`. Correct102 flags/states, natural completion rather than injected flags. | [Two-state comparison and scope](scratch/frontend-native-second-unlock-2026-10-28.md). Does not establish all map hover/input behavior, exact cursor colors or gameplay timing. |
| C1 | PASS: amended zoom1/elev_down footprint, 0.0160864 / 0.0161212. | Runtime stock connected, native icon/input acceptance separate; [runtime panel](scratch/runtime-skill-panel-integration.md). |
| C2 | OPEN: runtime HUD and bounded cursor adapter own-app tested. | No-exclusion view set and native hover/no-hover/exit; [HUD](scratch/phase-c2p-runtime-hud.md), [cursor](scratch/runtime-cursor-hover.md). |
| C3 | FAIL: 0.06354818984885621 versus 0.02. | [C3bv](scratch/phase-c3bv-production-pack.md); diagnose a shared mechanism, not three-pixel polishing. |
| D1 | FAIL / USER-PARKED: 657 / 305 divergences with clock off/on. | [Timing park](plan-history/phase-e.md#d1-whole-milestone); D1n landed, D1o still fails. |
| D2 | PARTIAL: pick 20/20; 3/4 skill sequences; digger 16/50. | [Digger clock USER-PARKED](plan-history/phase-e.md#d2b-digger-bite-timing); recent app picker work is not a new native pass. |
| D3 | FAIL / USER-PARKED: saved 10/10, end-time spread 75 s. | [Whole-level replay](plan-history/phase-e.md#d3-whole-milestone); correct saved count alone is insufficient. |
