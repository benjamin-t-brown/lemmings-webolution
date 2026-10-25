# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date; move the stopped run to `previous_runs` in
[the queue archive](tasks-archive.json)), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **active**
Issued: 2026-10-24 (user, overlord review 2026-10-24: "i approve": attach at launch without
placing the upload first; the 2 unspent starts stand; up to 3 more alpha-count rounds)
Supersedes: the upload-timing directive, done 2026-10-24 18:53 MDT (c1ai–c1al: `^tab`
registers at startup; the upload `0x472F99` is reached only through slot `+0x14` of vtable
`0x4B19E8`, called at `0x471746`; the object is built at startup and nothing on that chain
calls the upload; format callback `0x473B1E` keeps a 32-byte record, the popcount arm only
for a 16-bit format with more than one alpha bit; 0 of 2 starts spent; gate not scored).

## Why this

Static reads cannot place the upload in time: the chain runs through runtime objects and
vtable calls with no direct callers. A hook that is in place from launch to the paused level
does not need to know when the upload runs. Both earlier starts hooked only the `sar`; if the
kept format has no alpha mask, the upload branches at `0x4736B6` to `0x473B18` and never
reaches it. This start watches the whole path.

## Scope

**C1 skill panel, to its gate** (`PLAN.md` C1 row, amended twice). The standing-run loop
applies: you rank and split; an unread input is queued, not a stop. A starting chain, not a
script:

1. **Offline:** write and dry-check one read-only Frida script for one session, attached at
   launch (spawned suspended and resumed after the attach, or right after `Popen`, before the
   frontend loads textures) and kept attached through level entry and the pause. It appends
   to a log file as it goes, not one report on a timer, and the probe reads that file after
   the pause. Hooks, all `onEnter`, reads only:
   - `0x473B1E`, the format callback: each offered record's 32 bytes and the context bytes
     `+4..+8`, with a hit count;
   - `0x472F99`, the upload entry: call count, and the texture name or descriptor if it can
     be read from the arguments;
   - `0x4736B6`, the mask zero test: the mask dword `[ebp-0x128]`;
   - `0x473897`, the shift: `(mask, sar, shl)` triples with hit counts.
2. **One stock start:** take that read at a paused `MM_CLIMB_002`.
3. **Offline:** draw the plates with the read texels, alpha rule, counts and blend (selector
   1, c1ad). Score `zoom1`, `elev_up` and `elev_down` with their A7p camera params against
   their A7p `baseline.png` over the frozen footprint. Report `initial`, the other A7p views,
   `MM_DIG_000` and the scenery-only error beside them.

If the read shows the plates take the zero-mask branch, that branch is the rule; render from
it. If the gate fails after the plates are drawn, rank what is left inside the C1 footprint
(count tint, plate selection state, icon edges) and keep going within the budget.

## Budget

- **Rounds:** the alpha-count question gets up to **3 more** in this run (dry check, start,
  and one repair or second start). Each other question gets 3. A further round needs a
  written reason in its brief that it moves the gate.
- **Concurrency:** up to 3 offline rounds at once, at most one live owner.
- **Stock starts:** the pool of **2** stands. Each needs an offline dry check of its script
  and entry first. Read-only Frida, one session, no write to game memory beyond the harness's
  restored level-table slot. The launch-time attach is allowed (user, 2026-10-24). If a launch
  or attach fails, record it, restore, and do not retry that start; the second start needs an
  offline repair round first.
- **Run:** up to about **4 hours wall time or 12 rounds**. Round limit 30 minutes from
  dispatch.
- **No debugger.** Per-round commits and pushes.

## Decisions already made (do not re-ask)

- Gate: the C1 row as amended twice. The footprint fixture
  `tools/harness/fixtures/c1-footprint/footprint.json` (SHA-256 `e96840e0…`) is frozen.
  Gated views `zoom1`, `elev_up`, `elev_down`, each ≤ 0.02.
- **Read, do not fit, and do not choose by score.** Measured from the reference, not
  evidence: stock-over-ours brightness inside the plates is 0.47 on every plate
  (`build/tmp/overlord-2026-10-24/tabratio.py`). The overlord's reading, not evidence: a
  4-bit alpha with `.alf` 128 predicts 0.467, a 1-bit alpha 0, `alf/255` 0.50. A worker may
  cite these as a prediction or falsifier, never as a render input.
- If the counts depend on the wrapper's format list (dgVoodoo), not on the exe, record them
  as the measured value for this install and say so in the note.
- Do not reopen when the upload runs; the launch attach replaces that question.
- **Time the pass** (standing), total and per round, in the queue.
- **Landing** per the 2026-10-24 restructure: detail in the scratch note, round prose in
  `docs/plan-history/phase-c.md`, a one- or two-line C1 status cell, landed tasks in the
  queue archive.
- `levelScreen` is `u32(0x4CBFB0)` per process. Use `acquire_game_foreground`, never
  `goto_level.focus()`. Live steps run in session 1 per [MOUV_SSH.md](agents/MOUV_SSH.md).
  Close any game the round leaves running, after checking it. Restore and hash-check every
  install file in the same process that changed it.
- Settled and not to be re-derived: everything settled in the previous seventeen directives,
  plus c1ai–c1al. The menu-entry cause stays unnamed; do not chase it.
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
