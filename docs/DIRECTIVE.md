# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **active**
Issued: 2026-10-23 (user, overlord review 2026-10-23)
Supersedes: the C2 caption text / C3 breakdown directive, done 2026-10-23 (Paused text drawn,
A7 without the caption exclusion passes 9 of 10, `MM_DIG_000` fails by 2,660; `0x46D460`
packs a D3D diffuse dword, render state unread, caption parked at its cap; C3 full frame
0.2098 with `rest` 84.7% and skill boxes 11.3%).

## Why this, not C1

C3a's `rest` is 84.7% of the full-frame numerator, and 94.5% of it lies on pixels where our
render is exactly black. Outside the tube the original draws sand, sky and grass, the green
hose and base surround, the green and purple floor ring, the entrance hatch and the exit
balloon; ours draws none of them. The A2 omitted-scene inventory
(`docs/scratch/phase-a2-tube.md`) listed them, and A7's gate never measured them. That is
roughly 0.178 of 0.2098. The skill boxes are about 0.0236, so C1 is also needed before C3
can pass, but it is an eighth the size and comes after.

## Scope

Two questions. Both start offline and may run together; at most one owns the live game.

1. **C3 backdrop scenery (cap three rounds).** Read how the level loads and draws its
   non-tube scene: the background model or models the LVL names (`Models\bg_sand_medium\sand_1`,
   `sky`, `hose`, per the A2 inventory), their transform, texture binding and draw order,
   and the green and purple floor ring below the tube. Draw them in the production render.
   Bound the floor ring as its own region from the read geometry, not from the reference.
   Re-score C3 full frame on the A6 repair 4 pause with actors (A6d24 frame and scorer) and
   report it split by the C3a regions plus the floor ring. Also re-score the amended A7 gate
   (nine views + second level); it must still pass. Report whatever it is; do not tune.
2. **C3 entrance and exit models (cap two rounds).** Read the placement of `Entrance.a3d`
   and `Exit.a3d` (the balloon and basket) in the level and their draw, and draw both. The
   exit model and its live boxes are settled (round 12); this is its render only. The
   hatch's animated door and count are in scope only if the read reaches them; otherwise
   draw the static model and record the animation as parked. Re-score as in question 1.

## Budget

Per-round commits, up to three concurrent offline rounds, at most one live owner. **Offline
first. One stock start**, across both questions, and only if an offline read cannot settle a
value the render needs: name the value and the address to read before asking for it, no
retry after Popen. Memory reads only; no debugger probe. If neither question needs the start,
it is not spent.

## Decisions already made (do not re-ask)

- Authorized as scoped above (user, overlord review 2026-10-23): scenery before C1, one
  conditional start.
- **Read, do not fit, and do not choose by score.** A value taken from a scored capture is
  labelled measured-from-reference and named in the report. Sensitivity renders are reported,
  never used to pick a value.
- **Time the pass** (standing): total elapsed wall time and each round's dispatch-to-landing
  time, recorded in the queue when each round is dispatched and landed.
- `MM_DIG_000`'s 2,660 shortfall without the caption exclusion stays parked; report its
  re-score after the scenery lands, do not chase it.
- Our HUD reading Out 5 / 04:46 against the original's 3 / 04:50 is a tick-count mismatch,
  not a HUD defect; do not chase it here.
- Do not reuse heap addresses from an earlier process. Use `acquire_game_foreground`, never
  `goto_level.focus()`. Live steps run in session 1 per [MOUV_SSH.md](agents/MOUV_SSH.md).
  Close any game the round leaves running, after checking it. Restore and hash-check every
  install file in the same process that changed it.
- Settled and not to be re-derived: everything settled in the previous six directives,
  plus C2j (Paused string and white ink measured from reference), C2k (`0x46D460` packs a
  D3D diffuse dword; the blend is the device's; `Paused` is UK.LTX index `0x54`) and C3a
  (region bounds and shares).
- C1 stays held. B2 and B3 stay queued. Run CTest on mouv. Keep text files LF.

## Stop point

Stop and report when both questions are landed or parked, on any stop condition in
ORCHESTRATION, or if the live start crashes or leaves the install unrestored. Do not advance
to C1, the C2 cursor clause, the caption blend, B2 or B3 on this directive.

## Sync

Run from the Mac clone `~/progs/lemmings-webolution`. Pull before selecting work and push
after each round commit, per [ROLES.md](ROLES.md#opencode-on-the-mac-clone). If a push from
mouv fails on credentials, leave the commits local and say so in the handoff; the overlord
carries them to GitHub.
