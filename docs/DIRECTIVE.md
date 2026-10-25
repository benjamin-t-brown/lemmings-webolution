# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date; move the stopped run to `previous_runs` in
[the queue archive](tasks-archive.json)), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **active**
Issued: 2026-10-25 (user, overlord review 2026-10-25: "approve": a long multi-milestone run,
caps park instead of stopping, a pool of 3 starts, about 8 hours or 24 rounds)
Supersedes: the C1 alpha-offline directive, done 2026-10-25 00:52 MDT (c1at, c1au, c1av
offline; c1aw's one menu-attach start exited `0xC0000005` before `^tab` reached `0x473146`;
gate not scored). Its pending question `plate-alpha-after-crash` is answered by this
directive: neither park nor amend yet. Correct the c1au reading first (below).

## Why this

Passes have run 30–60 minutes because recent directives stopped on the first crash, rationed
starts one at a time and scoped one milestone. This run follows the
[standing-run loop](ORCHESTRATION.md#standing-run) as written: a capped question is parked
and the next one ranked; a user-only decision goes to `pending_questions` while other work
continues. Rounds stay small (task sizing is unchanged); the run chains more of them.

The overlord's reading, not evidence, for C1: c1au's landing says the `0x7c`-byte fill at
`[ebp-0x18c]` ends at `[ebp-0x110]` and does not cover `[ebp-0x140]`. `-0x140` lies inside
`[-0x18c, -0x110)` (`build/tmp/overlord-2026-10-25/fillcheck.py`, arithmetic only). c1aw's
one live hit at `0x473146` read that dword as 0 (on `$LemSheetHi8`). If `+7` is 0 for `^tab`,
the landed c1au conditional keeps c1as line 29 (R `0x0F00`, G `0x00F0`, B `0x000F`, A `0xF000`).
Re-read it; do not take this paragraph as the read.

## Scope, in rank order

Rank by gate error and the round caps within this order. Work on a later milestone may run
concurrently when it does not depend on an earlier one.

1. **C1 skill panel, to its gate** (`PLAN.md` C1 row as amended twice). Start offline:
   re-read whether the fill covers `[ebp-0x140]` and whether any instruction or call between
   the fill and `0x473146` can write it. Correct `docs/LESSONS.md` and the c1au note if the
   withdrawal was wrong. If `+7` is settled, apply the keep rule to the c1as list and record
   the kept format as the measured value for this install (dgVoodoo). Then draw the plates
   (texels, alpha rule, counts, blend selector 1 from c1ad) and score `zoom1`, `elev_up`,
   `elev_down`. Report `initial`, the other A7p views, `MM_DIG_000` and the scenery-only error
   beside them. If the gate still fails, rank what is left inside the footprint.
2. **C2 cursor clause** (`PLAN.md` C2 row): cursor sprite set on hover, no-hover and exit.
   Not started. The caption blend stays parked.
3. **B3 level select and map** (`PLAN.md` B3 row). Not started.
4. **C3 full frame and B2 screen state machine:** parked. Reopen one only with a written
   reason in the brief that it moves its gate.

## Budget

- **Run:** about **8 hours wall time or 24 rounds**. Round limit 30 minutes from dispatch.
- **Rounds:** 3 per question, then park it in Phase E with a resume brief and rank the next.
  A further round needs a written reason in its brief that it moves the gate.
- **Concurrency:** up to 3 offline rounds at once, at most one live owner.
- **Stock starts:** a pool of **3** for the run, each dry-checked offline first. Normal
  `Popen`, attach at the menu **through the REA CLI** (`rea instrument-with-frida attach`,
  the c1s/c1ad/c1ah recipe in [MOUV_SSH.md](agents/MOUV_SSH.md)). Python `frida.attach` and
  `frida.spawn` or any suspended launch are retired for game starts. Read-only hooks, one
  session, no write to game memory beyond the harness's restored level-table slot.
- **A crash** (a start exits abnormally) ends live work on that question: record it, park the
  live arm, continue offline work and other milestones. **A second crash in the run ends all
  stock starts for the run.**
- **No debugger.** Per-round commits and pushes.

## Decisions already made (do not re-ask)

- **Park by default.** When a question stalls at its cap or a run of work ends without a gate
  score, park it, add any park-or-amend choice to `pending_questions` with options, evidence
  and a recommendation, and continue with other ranked work. Do not stop for it.
- Gates as signed off in `PLAN.md`. The C1 footprint fixture
  `tools/harness/fixtures/c1-footprint/footprint.json` (SHA-256 `e96840e0…`) is frozen.
- **Read, do not fit, and do not choose by score.** The 0.47 stock-over-ours plate
  brightness ratio (`build/tmp/overlord-2026-10-24/tabratio.py`) is a prediction or falsifier
  only, never a render input. A format comes from the read keep rule applied to a logged list.
- Do not reopen when the `^tab` upload runs, and do not chase the startup crash's fault
  address or the menu-entry cause.
- `frida` 17.24.3 stays installed for mouv's Python (user, 2026-10-24).
- **Time the pass** (standing), total and per round, in the queue.
- **Landing** per the 2026-10-24 restructure: detail in the scratch note, round prose in
  `docs/plan-history/`, a one- or two-line status cell, landed tasks in the queue archive.
- `levelScreen` is `u32(0x4CBFB0)` per process. Use `acquire_game_foreground`, never
  `goto_level.focus()`. Live steps run in session 1 per [MOUV_SSH.md](agents/MOUV_SSH.md).
  Close any game the round leaves running, after checking it. Restore and hash-check every
  install file in the same process that changed it.
- Settled and not to be re-derived: everything settled in earlier directives and c1ap–c1aw.
- Parked and not to be chased: C3 scenery, the caption blend, the hatch animation, the
  `MM_DIG_000` shortfall, the HUD tick mismatch, the `+0x3c` writer. Run CTest on mouv. Keep
  text files LF.

## Stop point

Stop and report only when:
- the last ranked milestone's gate passes on your own independent re-score;
- a gate, its footprint, mask, views or exclusions would need to change;
- a guardrail breach: install hashes not restored, a process you cannot attribute, a write
  left unrestored, or a worker exceeding its start budget or using a retired method;
- every remaining ranked question is parked or waits on `pending_questions`;
- the run reaches about 8 hours or 24 rounds;
- delegation or the pinned model is unavailable, or another session is running a task.

Running low on context is not a stop: leave the queue resumable, write
`standing_run.handoff`, and say the user should run `/facilitator` again.

## Sync

Pull before selecting work and push after each round commit, per
[ROLES.md](ROLES.md#opencode-on-the-mac-clone). If a push from mouv fails on credentials, leave
the commits local and say so in the handoff; the overlord carries them to GitHub.
