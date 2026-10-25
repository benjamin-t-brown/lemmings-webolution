# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date; move the stopped run to `previous_runs` in
[the queue archive](tasks-archive.json)), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **active**
Issued: 2026-10-25 (user, overlord review 2026-10-25: "yep all are good i approve": 3 offline
rounds and 1 live round, a pool of 1 menu-attach start, a convergence check at the stop)
Supersedes: the hook-repair directive, done 2026-10-24 23:42 MDT (c1ap: no branch into the
`0x473897` patch; the count read moved to `0x47383C`; `0x4737E6` skips counts and shift when
`[obj+0x24] & 0x18` is 0. c1aq and c1as: spawned suspended, both died about 2 s after
resume, c1as with `0xC0000005`; the nine startup uploads are eight `_btn_*` and
`Sheets\$LemBits0`, every one with `& 0x18` 0; `$LemBits0` kept mask `0x8000`; `^tab` never
uploaded. Starts 2 of 2; gate not scored).

## Why this

The alpha-count question has taken 16 rounds and 5 starts. Part of it may now be offline:
c1ad read the plate descriptor's `+0x24` flags as 8 (`8 & 0x18` is nonzero), c1aj read the
format keep rule, and c1as logged the device's full format list. The overlord's reading, not
evidence: if the c1ad descriptor is the object `0x472F99` receives, `^tab` takes the shift
path and its counts follow from the keep rule and that list. Read that first. Spawn mode
crashed the original twice; menu-attach (c1s, c1ah) ran without a crash.

## Scope

**C1 skill panel, to its gate** (`PLAN.md` C1 row, amended twice). The standing-run loop
applies: you rank and split; an unread input is queued, not a stop. A starting chain, not a
script:

1. **Offline, up to three rounds, parallel where independent:**
   - Is the c1ad plate descriptor (`[ebp-4]` at `0x414944`, `+0x24` = 8) the same object, or
     the same field, that `0x472F99` reads at `[[ebp+8]+0x24]`? Read the bytes that link them.
   - How are context bytes `+4..+7` computed at `0x473146..0x4731C7`, and from what? With
     `^tab`'s inputs, which record of the c1as format list does the c1aj keep rule keep, and
     which mask?
   - What does the skip path at `0x47395A` write, so the alpha rule is known on either branch?
2. **Live, only if an input the render needs is still unread:** one stock start by the
   c1s/c1ah recipe (normal `Popen`, attach at the menu, armed before the level click, kept
   through the pause at `MM_CLIMB_002`). Read-only hooks as in c1as: upload entry with the
   texture name, mask with `skip_input`, counts at `0x47383C`. Dry-check it offline first.
3. **Offline:** draw the plates with the read texels, alpha rule, counts and blend (selector
   1, c1ad). Score `zoom1`, `elev_up` and `elev_down` with their A7p camera params against
   their A7p `baseline.png` over the frozen footprint. Report `initial`, the other A7p views,
   `MM_DIG_000` and the scenery-only error beside them.

If the gate fails after the plates are drawn, rank what is left inside the C1 footprint
(count tint, plate selection state, icon edges) and keep going within the budget.

## Budget

- **Rounds:** the alpha-count question gets up to **3 offline rounds and 1 live round**, and
  no more without the user. Each other question gets 3. A further round on another question
  needs a written reason in its brief that it moves the gate.
- **Concurrency:** up to 3 offline rounds at once, at most one live owner.
- **Stock starts:** a pool of **1** (user, 2026-10-25). **Menu-attach only:** a normal
  `Popen` and an attach at the menu. `frida.spawn` or any suspended launch is retired.
  Read-only Frida, one session, no write to game memory beyond the harness's restored
  level-table slot.
- **Run:** up to about **4 hours wall time or 12 rounds**. Round limit 30 minutes from
  dispatch.
- **No debugger.** Per-round commits and pushes.

## Decisions already made (do not re-ask)

- Gate: the C1 row as amended twice. The footprint fixture
  `tools/harness/fixtures/c1-footprint/footprint.json` (SHA-256 `e96840e0…`) is frozen.
  Gated views `zoom1`, `elev_up`, `elev_down`, each ≤ 0.02.
- **Read, do not fit, and do not choose by score.** Measured from the reference, not
  evidence: stock-over-ours brightness inside the plates is 0.47 on every plate
  (`build/tmp/overlord-2026-10-24/tabratio.py`). Cite it only as a prediction or falsifier,
  never as a render input. A format inferred offline must come from the read keep rule
  applied to the logged list, not from which format fits that ratio.
- If the counts depend on the wrapper's format list (dgVoodoo), record them as the measured
  value for this install and say so in the note.
- Do not reopen when the upload runs, and do not chase the startup crash's fault address.
- `frida` 17.24.3 stays installed for mouv's Python (user, 2026-10-24).
- **Time the pass** (standing), total and per round, in the queue.
- **Landing** per the 2026-10-24 restructure: detail in the scratch note, round prose in
  `docs/plan-history/phase-c.md`, a one- or two-line C1 status cell, landed tasks in the
  queue archive.
- `levelScreen` is `u32(0x4CBFB0)` per process. Use `acquire_game_foreground`, never
  `goto_level.focus()`. Live steps run in session 1 per [MOUV_SSH.md](agents/MOUV_SSH.md).
  Close any game the round leaves running, after checking it. Restore and hash-check every
  install file in the same process that changed it.
- Settled and not to be re-derived: everything settled in the previous nineteen directives,
  plus c1ap–c1as. The menu-entry cause stays unnamed; do not chase it.
- Parked and not to be chased: C3 scenery, the caption blend, the hatch animation, the
  `MM_DIG_000` shortfall, the HUD tick mismatch, the `+0x3c` writer, and the C2 cursor
  clause. B2 and B3 stay queued. Run CTest on mouv. Keep text files LF.

## Stop point

Stop and report when:
- the C1 gate passes on your own independent re-score;
- the gate, its footprint or its views would need to change;
- the alpha-count question has used its 3 offline and 1 live rounds without the counts or
  the rule the render needs;
- a question reaches its cap with no written reason for another round;
- the start pool is spent and the next ranked question needs a start;
- **any start crashes the game, or leaves the install unrestored. This is a hard stop: no
  repair round and no further start in this run;**
- the run reaches about 4 hours or 12 rounds;
- any other stop condition in ORCHESTRATION holds, or a decision only the user can make.

If the run stops without a gate score, the overlord brings the user a choice between parking
the plate-alpha question and amending the C1 gate (user, 2026-10-25). Do not pick either.

Do not advance to C3, the C2 cursor clause, B2 or B3.

## Sync

Pull before selecting work and push after each round commit, per
[ROLES.md](ROLES.md#opencode-on-the-mac-clone). If a push from mouv fails on credentials, leave
the commits local and say so in the handoff; the overlord carries them to GitHub.
