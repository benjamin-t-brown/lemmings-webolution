# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **active**
Issued: 2026-10-24 (user, overlord review 2026-10-24; "yes please write the recommended
directive", after the Frida explanation)
Supersedes: the sheet-and-font directive, done 2026-10-24 12:26 MDT (c1m: handle 6 names
`Sprites\Lemmings{Hi,Lo}.spr`, Hi/Lo is a free-memory threshold; c1n: two-digit count text,
`_FONTSMALL.BMP`; c1q: font bitmap 64x16 4bpp, white digits, no tint on the path read; c1p:
`LemmingsHi.spr` parsed, 7/7 against c1l's live frame records; c1o: Frida attach through the REA
CLI from ssh works on a harmless session-1 process).

## Why this

Two render inputs remain: the sheet pixels (`$LEMSHEETHI*.BMP` box members, leading tag `02`,
not yet decoded) and the icons' on-screen x, y (arguments of `0x47A75D`, run-time only). The first
is offline; the second needs one start with a hook, now that c1o proved the attach path. With both,
the C1 gate can be scored for the first time.

## Scope

Three parts. 1 and 2 may run together; 3 waits for both.

1. **Sheet pixels (offline, cap two rounds).** Decode the `$LEMSHEETHI<n>.BMP` members the seven
   generic icons use (c1p names them, e.g. `Hi21`, `Hi5`), starting from the lead that they use
   `0x467DE8` with tag `02`; run the exe's own decoder as c1q did. Also the textures for icons 2,
   8, 9 (Grey / Black / Additive, handles 1 / 2 / 3) if they fall out of the same path.
2. **One stock start, with one Frida hook (cap one round).** Hook `0x47A75D` and log the
   per-icon draw arguments (`ECX`, x, y) at a paused level start on `MM_CLIMB_002`. In the same
   process, memory-read the count-quad colour fields (`[textobj+0xC] + k*0x1B0 + 0x40 / 0x58`),
   the packed `[vertex+0x10]` of one count quad, `[0x58CE4C]` (the Hi/Lo `free` value), and icon 9's
   text pointer. Use the c1l probe and the c1o recipe in [MOUV_SSH.md](agents/MOUV_SSH.md); the
   worker must have the REA CLI before the start, not discover its absence after. State the
   prediction and falsifier before the start. This is the first Frida attach to the stock game:
   if the attach or the game fails, record it, restore and park. No retry after Popen.
3. **Q3 render (offline, cap two rounds)**, only if 1 and 2 leave pixels and placement READ.
   Draw the ten buttons and their counts and score against the A7p / A7q captures with the A7
   panel exclusion removed (gate: `x [18,90)` ≤ 0.02). Report pass or fail; do not tune to the
   score.

## Budget

Per-round commits, up to two concurrent rounds, at most one live owner. **One stock start, part
2 only**: memory reads plus one Frida hook on `0x47A75D`; no other hook, no write to game memory
beyond the harness's restored level-table slot. **No debugger.** Round time limit 30 minutes
from dispatch; a round at its limit lands what it has read, names the next body and stops.

## Decisions already made (do not re-ask)

- Authorized as scoped above (user, overlord review 2026-10-24), including the first Frida
  attach to the stock game. If it fails, do not request another start on this directive.
- **The first c1t dispatch (12:59:10) does not count against part 3's two-round cap** (user,
  overlord review 2026-10-24). Its facilitator (`claude -p`) exited at 13:09:19 and the worker
  died with it, leaving only `crop_ref.py` / `ref_crop.png`: no prediction, code or score. Mark
  it interrupted, take over the stale owner (process gone), and redispatch c1t as round 1 of 2.
- **Termination confirmed (overlord, 2026-10-24 13:21 MDT): the `standing_run.owner` session
  and the c1t worker are gone.** The Mac Claude Code facilitator was pid 8249; its launcher log
  `build/tmp/overlord-2026-10-24/facilitator.started` records `exit 0` at 13:09:19; `ps` on the
  Mac shows no pid 8249 and no `claude -p` process. c1t ran inside it, so no worker survives. The
  queue's `running` status for `c1t-q3-panel-render` is stale. A `/facilitator` started on mouv
  (or anywhere) is the handoff: record the new owner, set c1t interrupted, and continue.
- Frida goes through the REA CLI on mouv over ssh (user, 2026-10-24). Do not change
  `~/.claude.json` on the Mac.
- Commits and pushes per round are authorized (user, 2026-10-24).
- **Read, do not fit, and do not choose by score.** A value taken from a scored capture is
  labelled measured-from-reference and named in the report.
- **Time the pass** (standing): total elapsed wall time and each round's dispatch-to-landing
  time, recorded in the queue when each round is dispatched and landed.
- Do not reuse heap addresses from an earlier process; `levelScreen` is `u32(0x4CBFB0)` per
  process. Use `acquire_game_foreground`, never `goto_level.focus()`. Live steps run in
  session 1 per [MOUV_SSH.md](agents/MOUV_SSH.md). Close any game the round leaves running,
  after checking it. Restore and hash-check every install file in the same process that
  changed it. PIL is absent on mouv: do not make the probe depend on it.
- Settled and not to be re-derived: everything settled in the previous twelve directives, plus
  c1m, c1n, c1o, c1p and c1q. The menu-entry cause stays unnamed; do not chase it.
- Parked and not to be chased: C3 scenery, the caption blend, the hatch animation, the
  `MM_DIG_000` shortfall, the HUD tick mismatch, the selection highlight and `+0x3c` writer,
  and the C2 cursor clause. B2 and B3 stay queued. Run CTest on mouv. Keep text files LF.

## Stop point

Stop and report when the render is scored (pass or fail), when part 1 or 2 ends without the
input the render needs, on any stop condition in ORCHESTRATION, or if the start crashes or
leaves the install unrestored. Do not advance to C3, the C2 cursor clause, B2 or B3.

## Sync

Run from the Mac clone `~/progs/lemmings-webolution`. Pull before selecting work and push
after each round commit, per [ROLES.md](ROLES.md#opencode-on-the-mac-clone). If a push from
mouv fails on credentials, leave the commits local and say so in the handoff; the overlord
carries them to GitHub.
