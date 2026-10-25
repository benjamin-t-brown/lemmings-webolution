# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date; move the stopped run to `previous_runs` in
[the queue archive](tasks-archive.json)), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **active**
Issued: 2026-10-24 (user, overlord review 2026-10-24: "ya sure, 2 yes, whatever gets this
done more efficiently": a pool of 2 more starts, and a launch-time attach allowed)
Supersedes: the C1-to-its-gate directive, done 2026-10-24 17:35 MDT (c1aa–c1ah: the `.alf`
byte is `sar` by `8 − popcount(mask)` then `shl` by the mask's trailing zeros; the mask
comes through a 32-byte copy from a COM method outside the exe; paused plates use blend
selector 1; the hook at `0x473897` logged 0 hits both paused and armed before the level
click; start pool 3 of 3 spent; gate not scored).

## Why this

Everything the plate render needs is in hand except the two shift counts. Two starts went
to guesses about *when* the upload runs, a question an offline read can answer. This run
places the upload in time offline first, and only then spends a start.

## Scope

**C1 skill panel, to its gate** (`PLAN.md` C1 row, amended twice). The standing-run loop
applies: you rank and split; an unread input is queued, not a stop. A starting chain, not a
script:

1. **Offline:** find when the `^tab` upload at `0x472F99` runs: the callers of `0x45FE10`
   and of the registration at `0x45E90F`, and whether the upload happens at startup, in the
   frontend or on level entry. Read the format callback at `0x473B1E` too: which enumerated
   format it keeps and when the enumeration stops.
2. **Offline:** dry-check a read-only hook placed before that upload. If it runs before the
   level click, the attach goes in at launch (see Budget). Log the callback's format masks
   and the `(mask, sar, shl)` triples.
3. **One stock start:** take that read.
4. **Offline:** draw the plates with the read texels, alpha rule, counts and blend
   (selector 1, c1ad). Score `zoom1`, `elev_up` and `elev_down` with their A7p camera
   params against their A7p `baseline.png` over the frozen footprint. Report `initial`, the
   other A7p views, `MM_DIG_000` and the scenery-only error beside them.

If the gate fails after the plates are drawn, rank what is left inside the C1 footprint
(count tint, plate selection state, icon edges) and keep going within the budget.

## Budget

- **Rounds:** the alpha-count question ran 6 rounds last pass. It gets up to **3 more** in
  this run, the start included. Each other question gets 3. A further round needs a written
  reason in its brief that it moves the gate.
- **Concurrency:** up to 3 offline rounds at once, at most one live owner.
- **Stock starts:** a pool of **2**. No start until an offline round has placed the upload
  in time and its hook and entry have passed a dry check. Read-only Frida, one attach, no
  write to game memory beyond the harness's restored level-table slot. **A launch-time
  attach is allowed** (user, 2026-10-24): attach right after `Popen`, or spawn the game
  suspended and resume it after the attach, before the frontend loads textures. If a launch
  or attach fails, record it, restore, and do not retry that start; the second start needs
  an offline repair round first.
- **Run:** up to about **4 hours wall time or 12 rounds**. Round limit 30 minutes from
  dispatch.
- **No debugger.** Per-round commits and pushes.

## Decisions already made (do not re-ask)

- Gate: the C1 row as amended twice. The footprint fixture
  `tools/harness/fixtures/c1-footprint/footprint.json` (SHA-256 `e96840e0…`) is frozen.
  Gated views `zoom1`, `elev_up`, `elev_down`, each ≤ 0.02.
- **Read, do not fit, and do not choose by score.** Measured from the reference, not
  evidence: stock-over-ours brightness inside the plates is 0.47 on every plate
  (`build/tmp/overlord-2026-10-24/tabratio.py`). The overlord's reading, which is not
  evidence either: `[format+0x1c]` sits where `DDPIXELFORMAT.dwRGBAlphaBitMask` sits, so the
  counts may come from the 16-bit alpha format the device offers. A 4-bit alpha with
  `.alf` 128 predicts 0.467, a 1-bit alpha predicts 0, `alf/255` predicts 0.50. A worker may
  cite these as a prediction or falsifier, never as a render input. The counts come from
  the live read.
- If the read shows the counts depend on the wrapper's format list (dgVoodoo), not on the
  exe, record that as the measured value for this install and say so in the note.
- **Time the pass** (standing), total and per round, in the queue.
- **Landing** per the 2026-10-24 restructure: detail in the scratch note, round prose in
  `docs/plan-history/phase-c.md`, a one- or two-line C1 status cell, landed tasks in the
  queue archive. The C1 status cell is about 1,500 bytes now; trim it to two lines.
- `levelScreen` is `u32(0x4CBFB0)` per process. Use `acquire_game_foreground`, never
  `goto_level.focus()`. Live steps run in session 1 per [MOUV_SSH.md](agents/MOUV_SSH.md).
  Close any game the round leaves running, after checking it. Restore and hash-check every
  install file in the same process that changed it.
- Settled and not to be re-derived: everything settled in the previous sixteen directives,
  plus c1aa–c1ah. The menu-entry cause stays unnamed; do not chase it.
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
