# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date; move the stopped run to `previous_runs` in
[the queue archive](tasks-archive.json)), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **active**
Issued: 2026-10-24 (user, overlord review 2026-10-24: "yep all are good i approve": up to 3
more alpha-count rounds, the start pool back to 2, keep mouv's `frida` package)
Supersedes: the launch-attach directive, done 2026-10-24 21:48 MDT (c1am dry-checked four
launch-time hooks; c1an installed `frida` 17.24.3; c1ao pid 20612 exited within five seconds
of `frida.resume`, before the level, with 9 uploads, masks 0 ×8 then `0x8000`, and 0 hits at
`0x473897`; install hashes match; gate not scored).

## Why this

c1ao's `hits.log` (sites `0x473B1E` and `0x4736B6`) shows the ninth startup upload enumerated
20 formats, including ARGB1555, ARGB4444, ARGB8888 and DXT1–5, and kept mask `0x8000`. The
process died right after that upload passed the zero test, at the point where it would reach
the shift. The overlord's reading, not evidence: Frida's patch at `0x473897` (`d3 f8`, 2 bytes,
inside the per-pixel loop) covers at least 5 bytes, so a branch into that range would crash;
and both counts are stored before the loop (`0x4737EC`, the loop at `0x473810`), so they can be
read outside it. Which texture each upload was is unread; every name was null.

## Scope

**C1 skill panel, to its gate** (`PLAN.md` C1 row, amended twice). The standing-run loop
applies: you rank and split; an unread input is queued, not a stop. A starting chain, not a
script:

1. **Offline repair:** find why c1ao's process ended and fix the probe, then dry-check it.
   At least: say from the bytes whether any branch lands inside the range a hook at
   `0x473897` would patch; read the counts at a site outside the pixel loop instead
   (`[ebp-0xae8]` and `[ebp-0xad8]` once they are stored, with the mask); identify each upload's
   texture (the `0x472F99` argument against the registration list, or another read); capture
   how the process ends (Frida detach reason or crash, and the exit code); set the spawn's
   working directory to the install.
2. **One stock start:** launch-time attach, kept through level entry and the pause at
   `MM_CLIMB_002`, logging continuously.
3. **Offline:** draw the plates with the read texels, alpha rule, counts and blend (selector
   1, c1ad). Score `zoom1`, `elev_up` and `elev_down` with their A7p camera params against
   their A7p `baseline.png` over the frozen footprint. Report `initial`, the other A7p views,
   `MM_DIG_000` and the scenery-only error beside them.

If the plates take the zero-mask branch, that branch is the rule; render from it. If the gate
fails after the plates are drawn, rank what is left inside the C1 footprint (count tint, plate
selection state, icon edges) and keep going within the budget.

Also, as a framework edit with the first round's landing: update `docs/agents/MOUV_SSH.md`,
which still says mouv's Python has no `frida`.

## Budget

- **Rounds:** the alpha-count question gets up to **3 more** in this run. Each other question
  gets 3. A further round needs a written reason in its brief that it moves the gate.
- **Concurrency:** up to 3 offline rounds at once, at most one live owner.
- **Stock starts:** a pool of **2** (user, 2026-10-24). Each needs an offline dry check of its
  script and entry first. Read-only Frida, one session, no write to game memory beyond the
  harness's restored level-table slot. Launch-time attach is allowed. If a launch or attach
  fails, record it, restore, and do not retry that start; the second start needs an offline
  repair round first.
- **Run:** up to about **4 hours wall time or 12 rounds**. Round limit 30 minutes from
  dispatch.
- **No debugger.** Per-round commits and pushes.

## Decisions already made (do not re-ask)

- Gate: the C1 row as amended twice. The footprint fixture
  `tools/harness/fixtures/c1-footprint/footprint.json` (SHA-256 `e96840e0…`) is frozen.
  Gated views `zoom1`, `elev_up`, `elev_down`, each ≤ 0.02.
- **Read, do not fit, and do not choose by score.** Measured from the reference, not
  evidence: stock-over-ours brightness inside the plates is 0.47 on every plate
  (`build/tmp/overlord-2026-10-24/tabratio.py`). A 1-bit alpha (ARGB1555) makes a plate texel
  either opaque or clear, which that ratio does not obviously fit; that tension is for the
  read to settle, not a reason to pick another format. Cite these only as a prediction or
  falsifier, never as a render input.
- If the counts depend on the wrapper's format list (dgVoodoo), not on the exe, record them
  as the measured value for this install and say so in the note.
- Do not reopen when the upload runs; the launch attach replaces that question.
- `frida` 17.24.3 stays installed for mouv's Python (user, 2026-10-24).
- **Time the pass** (standing), total and per round, in the queue.
- **Landing** per the 2026-10-24 restructure: detail in the scratch note, round prose in
  `docs/plan-history/phase-c.md`, a one- or two-line C1 status cell, landed tasks in the
  queue archive.
- `levelScreen` is `u32(0x4CBFB0)` per process. Use `acquire_game_foreground`, never
  `goto_level.focus()`. Live steps run in session 1 per [MOUV_SSH.md](agents/MOUV_SSH.md).
  Close any game the round leaves running, after checking it. Restore and hash-check every
  install file in the same process that changed it.
- Settled and not to be re-derived: everything settled in the previous eighteen directives,
  plus c1am–c1ao. The menu-entry cause stays unnamed; do not chase it.
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
