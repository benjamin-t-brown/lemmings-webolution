# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date; move the stopped run to `previous_runs` in
[the queue archive](tasks-archive.json)), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **done**
Stopped: 2026-10-24 17:35 MDT. The start pool is spent, 3 of 3. The shift counts at `0x473897` were not read: a paused attach and an attach armed before the level click both logged 0 hits. The C1 gate is not scored. Another count read needs a start.
Issued: 2026-10-24 (user, overlord review 2026-10-24: "good i approve", option A of the
c1-tab-blend-render evaluation with the milestone-scoped budget below)
Supersedes: the tab-blend directive, done 2026-10-24 15:25 MDT (c1y: `^tab` is a 72×32 BMP
with zero colour bytes plus a 2304-byte `.alf`, values 0–128, consumer `0x47E40D` unread;
c1z: plate blend in `0x414D80`, stage state in `0x47261F`, hook `0x41494F`, live selector
unread; the Frida start was not spent).

## Why this

The plate image is in hand: a black colour plane and a rounded alpha mask. What the render
still needs is how the `.alf` values become texture alpha, and which blend the plate draw uses
live. Each of the last seven C1 directives stopped at the first unread input, one step from
the next obvious read. This run is scoped to the milestone instead: take C1 to its gate.

## Scope

**C1 skill panel, to its gate** (`PLAN.md` C1 row, amended twice). You pick, split and rank
each next question by gate error, as the standing-run loop says. An unread input is not a
stop: name it, queue the read and continue. The chain as the overlord sees it today, a
starting ranking and not a script:

1. **Offline:** read `0x47E40D`, how the 0–128 `.alf` bytes become texture alpha (kept,
   doubled, clamped or other), and decode `^tab_selected` (`0x45E90F`) if it falls out of the
   same path.
2. **Offline:** move or add the read-only sample so the hook sees the descriptor and flag
   caches *before* `0x47261F` (c1z's narrowing), then dry-check it.
3. **One stock start, Frida:** at a paused `MM_CLIMB_002`, log the plate draw's blend
   selector, blend cache, descriptor flags, alpha ref and stage states, read-only, one attach.
4. **Offline:** draw the plates with the read texels, alpha rule and blend, and the read
   zero-count rule. Score `zoom1`, `elev_up` and `elev_down` with their A7p camera params
   (`build/tmp/a7p-unpause-then-masks/commands.txt`) against their A7p `baseline.png` over the
   frozen footprint. Report `initial`, the other A7p views and `MM_DIG_000` beside them, and
   the scenery-only error between the plates.

If the gate fails after the plates are drawn, rank what is left inside the C1 footprint
(count tint, plate selection state, icon edges) and keep going within the budget.

## Budget

- **Rounds:** 3 per question (the ORCHESTRATION default), repairs and continuations
  included. A fourth needs a written reason in its brief that it moves the gate.
- **Concurrency:** up to 3 offline rounds at once, at most one live owner.
- **Stock starts:** a pool of **3** for the run. Each needs an offline dry check of its hook
  and entry first. Read-only Frida, one attach, no write to game memory beyond the
  harness's restored level-table slot. Use the c1s recipe. If the launch or attach fails,
  record it, restore, and do not retry that start; a later start needs an offline repair
  round first.
- **Run:** up to about **4 hours wall time or 12 rounds**, whichever comes first. Round limit
  30 minutes from dispatch; a round at its limit lands what it has read and names the next
  body.
- **No debugger.** Per-round commits and pushes.

## Decisions already made (do not re-ask)

- Gate: the C1 row as amended twice (user, overlord review 2026-10-24). The footprint fixture
  `tools/harness/fixtures/c1-footprint/footprint.json` (SHA-256 `e96840e0…`) is frozen; do
  not edit it after a score. Gated views `zoom1`, `elev_up`, `elev_down`, each ≤ 0.02.
- `MM_DIG_000`'s `baseline.png` is the stock frame (A7q `capture.py`, `capture_image`);
  informational only.
- **Read, do not fit, and do not choose by score.** The overlord's evaluation found that
  darkening our render by `alf/255` drops plate-pixel error to about 0.025 on `zoom1` and
  `elev_down`, and that stock-over-ours brightness inside the plates is 0.47 on every plate
  (`build/tmp/overlord-2026-10-24/tabcheck.py`, `tabratio.py`). That is
  measured-from-reference: a worker may cite it in a prediction or falsifier, never use it
  as a render input. The alpha rule and blend come from `0x47E40D` and the live read.
- **Time the pass** (standing): total elapsed wall time and each round's dispatch-to-landing
  time, recorded in the queue.
- **Landing** follows the 2026-10-24 restructure: detail in the scratch note, round prose
  appended to `docs/plan-history/phase-c.md` (c1y and c1z were not; append them first),
  a one- or two-line C1 status cell, and landed tasks in the queue archive.
- Do not reuse heap addresses from an earlier process; `levelScreen` is `u32(0x4CBFB0)` per
  process. Use `acquire_game_foreground`, never `goto_level.focus()`. Live steps run in
  session 1 per [MOUV_SSH.md](agents/MOUV_SSH.md). Close any game the round leaves running,
  after checking it. Restore and hash-check every install file in the same process that
  changed it.
- Settled and not to be re-derived: everything settled in the previous fifteen directives,
  plus c1y and c1z. The menu-entry cause stays unnamed; do not chase it.
- Parked and not to be chased: C3 scenery, the caption blend, the hatch animation, the
  `MM_DIG_000` shortfall, the HUD tick mismatch, the `+0x3c` writer, and the C2 cursor
  clause. B2 and B3 stay queued. Run CTest on mouv. Keep text files LF.

## Stop point

Stop and report when:
- the C1 gate passes on your own independent re-score;
- the gate, its footprint or its views would need to change;
- a question reaches its cap with no written reason for another round;
- the start pool is spent and the next ranked question needs a start;
- the run reaches about 4 hours or 12 rounds;
- a start crashes the game or leaves the install unrestored;
- any other stop condition in ORCHESTRATION holds, or a decision only the user can make.

Do not advance to C3, the C2 cursor clause, B2 or B3.

## Sync

Pull before selecting work and push after each round commit, per
[ROLES.md](ROLES.md#opencode-on-the-mac-clone). If a push from mouv fails on credentials, leave
the commits local and say so in the handoff; the overlord carries them to GitHub.
