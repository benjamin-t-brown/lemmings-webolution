# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **done** (2026-10-24 12:30 MDT: part 1 at cap, part 2 landed, Q3 render not run; see docs/tasks.json standing_run.stopped)
Issued: 2026-10-24 (user, overlord review 2026-10-24; "i approve 1": Frida through the REA CLI
over ssh)
Supersedes: the observable-entry directive, done 2026-10-24 (c1j: `[[0x4CBFB0]+0x10]` feed,
SpriteFrame loader, `_FontSmall` load, Grey/Black/Additive stores READ; c1k: entry teed to
`run.log`; c1l: the start entered, `[levelScreen+0x10]` = 0, sheet handles, ten frame records and
count font objects read live; hook not run, the worker had no Frida tool).

## Why this

C1l left four render inputs UNREAD: the sheet file behind handle 6 (and its `Hi`/`Lo` suffix),
the count font file and colour behind handles 37 / 36, the count text behind the `+0xC`
pointers, and the on-screen x, y. The first two have named offline bodies. The gate has not
moved because nothing has been drawn. This pass closes what is readable offline, renders and
scores, and proves the Frida path so that any later hook read is not lost to a tool gap again.

## Scope

Three parts. 1 and 2 may run together; 3 waits for 1.

1. **Offline reads (up to two concurrent rounds, cap two rounds each).**
   a. Handle to sheet file: registrar `0x446040` -> `0x449170`, which file handle 6 names and
      how `Hi` / `Lo` is chosen at 640x480.
   b. Count font: `0x474519` -> `0x449B30` / `0x45FE10` / `0x4607D0`, the file behind handles
      37 / 36 and the colour. Read the count text source too if it falls out of the same chain.
2. **Frida preflight (no stock start).** Prove `rea instrument-with-frida attach` from a worker
   on the Mac clone, over ssh to mouv: attach to a harmless session-1 process, hook a known
   export, detach, and check the process survives. If session 0 cannot attach to session 1,
   run the same call from a session-1 scheduled task. Record the working recipe in
   [MOUV_SSH.md](agents/MOUV_SSH.md) (REA bundles Frida; Python `frida` is not needed).
3. **Q3 render (offline, cap two rounds)**, only if 1a and 1b leave the sheet file and font
   READ. Draw the ten buttons and counts from the c1l readings and score against the A7p / A7q
   captures with the A7 panel exclusion removed. If placement cannot be settled without the
   hook's x, y, say so and stop; do not fit it.

## Budget

Per-round commits, up to two concurrent offline rounds. **No stock start** and no live game.
No debugger. Round time limit 30 minutes from dispatch; a round at its limit lands what it has
read, names the next body and stops.

## Decisions already made (do not re-ask)

- Frida for workers goes through the REA CLI on mouv over ssh (user, overlord review
  2026-10-24). Do not change `~/.claude.json` on the Mac.
- Commits and pushes per round are authorized (user, 2026-10-24).
- **Read, do not fit, and do not choose by score.** A value taken from a scored capture is
  labelled measured-from-reference and named in the report.
- **Time the pass** (standing): total elapsed wall time and each round's dispatch-to-landing
  time, recorded in the queue when each round is dispatched and landed.
- Settled and not to be re-derived: everything settled in the previous eleven directives, plus
  c1j, c1k and c1l. The menu-entry cause stays unnamed; do not chase it on this directive.
- Parked and not to be chased: C3 scenery, the caption blend, the hatch animation, the
  `MM_DIG_000` shortfall, the HUD tick mismatch, the selection highlight and `+0x3c` writer,
  and the C2 cursor clause. B2 and B3 stay queued. Run CTest on mouv. Keep text files LF.
- Fix the stale C1 Phase E entry in `PLAN.md` (it still lists c1l's reads as UNREAD).

## Stop point

Stop and report when the render is scored (gate pass or fail), when part 1 reaches its cap
without a READ sheet file and font, or on any stop condition in ORCHESTRATION. If the render
needs the hook's x, y, stop and name it; a live start is the next directive's decision. Do not
advance to C3, the C2 cursor clause, B2 or B3.

## Sync

Run from the Mac clone `~/progs/lemmings-webolution`. Pull before selecting work and push
after each round commit, per [ROLES.md](ROLES.md#opencode-on-the-mac-clone). If a push from
mouv fails on credentials, leave the commits local and say so in the handoff; the overlord
carries them to GitHub.
