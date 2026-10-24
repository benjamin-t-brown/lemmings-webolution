# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **done** (2026-10-24 15:25 MDT: part 1 ended at its cap without a plate image. `^tab` colour bytes are zero. The `.alf` consumer `0x47E40D` is unread. `^tab_selected` was not decoded. The Frida start was not spent. Part 2 and part 3 were not started.)
Issued: 2026-10-24 (user, overlord review 2026-10-24; "A i approve Frida, go ahead": option A
of the c1-footprint-plate-score evaluation, with one Frida start for the blend read)
Supersedes: the footprint directive, done 2026-10-24 14:26 MDT (c1u: icon and glyph footprint;
c1v: ten plate boxes at `0x45B66A`, texture `^tab`, zero count skips the text; c1w: plates in
the footprint, `initial` 0.2918; c1x: `0x47218A` is the texture-loader entry and `0x46D5F3` is
`call [ecx+0x70]`, no colour key or blend mode named).

## Why this

The plates are translucent, and on `initial` the scenery behind them is parked C3 backdrop
(evidence and the second gate amendment in `PLAN.md`, Phase C user decisions). The user moved the C1
gate to the three A7p views where the tube wall fills the panel's background: `zoom1`,
`elev_up`, `elev_down`. Two inputs remain: the `^tab` plate pixels and the blend state the
device uses when it draws the plates.

## Scope

Three parts. 1 is offline and runs first; 2 waits for 1b; 3 waits for both.

1. **Offline (cap one round each, may run together).**
   a. Decode `^tab` (handle `[0x4E3348]`, stored at `0x45E909`) and, if it falls out of the
      same path, `^tab_selected`, as c1r decoded the sheet members.
   b. Name the hook site for part 2: where the device's blend and texture-stage state are set
      for the plate quads drawn by `0x45B66A` → `0x414920`, and which device-method slots
      carry them. Write the hook script and its prediction and falsifier; dry-check it offline.
2. **One stock start, Frida (cap one round).** At a paused `MM_CLIMB_002` level start, the
   part 1b hook logs the render state in effect for the plate draws (blend enable, source and
   destination blend, alpha test, texture-stage colour and alpha ops, texture factor or
   diffuse if used). Read-only hooks in one attach on the plate draw path; no write to game
   memory beyond the harness's restored level-table slot. Use the c1s recipe. If the attach or
   the game fails, record it, restore and park. No retry after Popen.
3. **Render and score (offline, cap two rounds).** Draw the plates with the read pixels and
   blend, and the read zero-count rule. Re-render `zoom1`, `elev_up`, `elev_down` with their
   A7p camera params (`build/tmp/a7p-unpause-then-masks/commands.txt`) and score them against
   their A7p `baseline.png` over the committed footprint. Gate: each ≤ 0.02. Also score
   `initial`, the other A7p views and `MM_DIG_000` for the record, not the gate. Report the
   scenery-only error between the plates beside each score. Do not tune to the score.

## Budget

Per-round commits, up to two concurrent offline rounds. **One stock start, part 2 only,** with
read-only Frida hooks in one attach. **No debugger.** Round limit 30 minutes from dispatch; a
round at its limit lands what it has read, names the next body and stops.

## Decisions already made (do not re-ask)

- Gate: as amended twice in `PLAN.md` C1 row (user, overlord review 2026-10-24). Footprint
  fixture `tools/harness/fixtures/c1-footprint/footprint.json` (SHA-256 `e96840e0…`) is frozen;
  do not edit it after a part-3 score. Gated views `zoom1`, `elev_up`, `elev_down`.
- The Frida start is authorized (user, overlord review 2026-10-24). If it fails, do not request
  another start on this directive.
- `MM_DIG_000`, informational only: A7q's `capture.py` takes `baseline.png` from the stock
  game's window (`capture_image`, lines 235–247; call at 565), so it is the stock frame. This
  answers the pending question `mm-dig-stock-frame` with option 1; close it.
- **Read, do not fit, and do not choose by score.** A value taken from a scored capture is
  labelled measured-from-reference and named in the report.
- **Time the pass** (standing): total elapsed wall time and each round's dispatch-to-landing
  time, recorded in the queue when each round is dispatched and landed.
- Do not reuse heap addresses from an earlier process; `levelScreen` is `u32(0x4CBFB0)` per
  process. Use `acquire_game_foreground`, never `goto_level.focus()`. Live steps run in
  session 1 per [MOUV_SSH.md](agents/MOUV_SSH.md). Close any game the round leaves running,
  after checking it. Restore and hash-check every install file in the same process that
  changed it.
- Settled and not to be re-derived: everything settled in the previous fourteen directives,
  plus c1u, c1v, c1w and c1x. The menu-entry cause stays unnamed; do not chase it.
- Parked and not to be chased: C3 scenery, the caption blend, the hatch animation, the
  `MM_DIG_000` shortfall, the HUD tick mismatch, the selection highlight and `+0x3c` writer,
  and the C2 cursor clause. B2 and B3 stay queued. Run CTest on mouv. Keep text files LF.

## Stop point

Stop and report when part 3 is scored (pass or fail), when part 1 or 2 ends without an input
the render needs, on any stop condition in ORCHESTRATION, or if the start crashes or leaves the
install unrestored. Do not advance to C3, the C2 cursor clause, B2 or B3.

## Sync

Pull before selecting work and push after each round commit, per
[ROLES.md](ROLES.md#opencode-on-the-mac-clone). If a push from mouv fails on credentials, leave
the commits local and say so in the handoff; the overlord carries them to GitHub.
