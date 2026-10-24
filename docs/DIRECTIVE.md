# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **done** (2026-10-24 stop point: c1j, c1k landed, the one start c1l landed)
Issued: 2026-10-24 (user, overlord review 2026-10-24; "i approve all commit")
Supersedes: the C1 base icon draw directive, done 2026-10-24 (c1g: base icons drawn by panel
object `0x4C91D8` via `0x45B639` / `0x47A75D`; c1h: scale mode `S`, `Lo`/`Hi` budget loop
READ; c1i: the one start failed at `no menu highlight`, nothing read, install restored).

## Why this

C1 and C3 both wait on live-only inputs: `[[0x4CBFB0]+0x10]`, the sheet frame values `F`,
the count text and font, the `0x4E33xx` copy. The last two live starts (c3j, c1i) died in the
same shared entry code, `find_menu_index`, and the c3m patch did not fix it. Its per-press
output went to stdout, which the scheduled task does not capture, so the one diagnostic the
start could have produced was lost. Both authorized starts are spent with zero reads. A plain
retry would likely spend a third the same way. This pass makes the entry observable and
reads everything still readable offline, then spends one start.

## Scope

Two parts, in order.

1. **Offline (up to two concurrent rounds, cap two rounds each).**
   a. **Remaining named writers.** Read the store to `[[0x4CBFB0]+0x10]` (and what feeds
      it), the sheet frame-table loader behind `0x47ABC0` (the `F` values), the writer of
      `obj+8` for `[0x4CBF7C]` and the file its font comes from, and the `0x4E33xx` copy.
      Anything that becomes READ shrinks the live read list; say which items remain
      live-only.
   b. **Make the entry observable.** Route `find_menu_index`'s per-press output and the
      menu-state snapshot into `run.log` (tee, not stdout-only); have the probe call
      `save_state()` so `entry_focus` is recorded; add a no-input 15 s snapshot of screen id
      and `ui_state` after launch, before any key. Decide the dirty `goto_level.py` on mouv
      (sha256 `c28335b5…31c8`, the c3m retry/logging patch): land it with these changes or
      revert it, and leave mouv's tree clean either way. No game start in this part.
2. **One stock start**, only after part 1b is landed and its probe runs a dry check offline.
   Run the c1i probe unchanged in what it reads (`[levelScreen+0x10]`, icon fields at
   `0x4C91D8 + 0x1150 + i*0x208`, `0x4E335C..`, `[0x4CBF7C]` / `[0x4CBF78]` and the count
   text, at most one Frida entry hook on `0x47A75D`), minus anything part 1a made READ. State
   the prediction and falsifier before the start. If the entry fails again, the new log must
   name why; record that and park. No retry after Popen. No debugger.

Do not run Q3 (render) on this directive. If the start reads everything the render needs,
say so and stop; the render is the next directive.

## Budget

Per-round commits, up to two concurrent offline rounds, at most one live owner. **One stock
start, part 2 only.** Memory reads plus at most one Frida entry hook, as C1f and c1i. **No
debugger.** Round time limit 30 minutes from dispatch; a round at its limit lands what it
has read, names the next body and stops.

## Decisions already made (do not re-ask)

- Authorized as scoped above (user, overlord review 2026-10-24): a third live start, once,
  after the entry is observable. If it fails, C1 live work stops and the run parks; do not
  request a fourth.
- Commits and pushes per round are authorized (user, 2026-10-24).
- **Read, do not fit, and do not choose by score.** A value taken from a scored capture is
  labelled measured-from-reference and named in the report.
- **Time the pass** (standing): total elapsed wall time and each round's dispatch-to-landing
  time, recorded in the queue when each round is dispatched and landed.
- Do not reuse heap addresses from an earlier process; `levelScreen` is `u32(0x4CBFB0)` per
  process. Use `acquire_game_foreground`, never `goto_level.focus()`. Live steps run in
  session 1 per [MOUV_SSH.md](agents/MOUV_SSH.md). Close any game the round leaves running,
  after checking it. Restore and hash-check every install file in the same process that
  changed it.
- Settled and not to be re-derived: everything settled in the previous ten directives, plus
  c1g, c1h, c1i and c3m (the entry failure is intermittent in the logs, cause not named;
  precedent d2b pid 47392).
- Parked and not to be chased: C3 scenery, the caption blend, the hatch animation, the
  `MM_DIG_000` shortfall, the HUD tick mismatch, the selection highlight and `+0x3c` writer,
  and the C2 cursor clause. B2 and B3 stay queued. Run CTest on mouv. Keep text files LF.

## Stop point

Stop and report after the start (landed or parked), on any stop condition in ORCHESTRATION,
or if the start crashes or leaves the install unrestored. Do not advance to Q3, C3, the C2
cursor clause, B2 or B3 on this directive.

## Sync

Run from the Mac clone `~/progs/lemmings-webolution`. Pull before selecting work and push
after each round commit, per [ROLES.md](ROLES.md#opencode-on-the-mac-clone). If a push from
mouv fails on credentials, leave the commits local and say so in the handoff; the overlord
carries them to GitHub.
