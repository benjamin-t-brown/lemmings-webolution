# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **active**
Issued: 2026-10-24 (user, overlord review 2026-10-24)
Supersedes: the C3 chain offline directive, done 2026-10-24 (C3k: stack bottom is the
per-frame camera block `[0x58D3D8]`, `0x40eb50` READ as row vector times row-major 3x3; C3l:
kinds 5/7 160 / 212 B and the type-2 load READ, scenery parses past `0x967a2`; C3m: the C3j
menu-entry failure cause not named, patch proposed and unapplied; render not run, C3 scenery
parked in Phase E as that directive required).

## Why this

C3 scenery is parked after twelve rounds; that directive sent the next pass to C1. C1 is
also needed for C3: the ten skill boxes are about 0.0236 of full frame, over 0.02 on their
own. C1 stopped at C1f: `FUN_00457868` fires for exactly the one widget `levelScreen+0x3c`
names, while about nine distinct icons are already on screen, so the base per-button icon
draw is unidentified. The counts half can use the `^Font` path B1w built and C2g uses.

## Scope

Three questions, in order. 2 and 3 wait for 1.

1. **Base icon draw (offline, cap two rounds).** Find what puts the nine or ten base icons on
   the panel each frame. Start from the level screen's frame draw (the pause arm reaches the
   HUD draws read in C2; `0x45E31C..0x45E347`) and the widget vtable `0x4b1774` slots other
   than +4, not from the stride-`0x238` / `+8`-literal scans C1e exhausted. Name the call,
   the texture or sheet it binds, how each button selects its cell (the `+8` index C1f
   measured as `widget_index + 8` is a candidate, not a conclusion), the placement against
   the panel boxes read at `0x4C9208` (stride `0x1B0`), and the count draw (font, anchor,
   colour). Each value READ, or labelled measured-from-reference and named.
2. **Live confirmation (one stock start), only if** question 1 names a single call it cannot
   settle offline. Memory reads, plus at most one Frida entry hook on that call on the
   already-running process, as C1f did; no debugger, no write to game memory. State the
   prediction and its falsifier before the start. The C3j entry failure is unexplained: apply
   C3m's `fix.patch` (retry and logging only) to the entry first, and if the entry still
   fails, park; do not retry after Popen.
3. **Render (offline, cap two rounds), only if** the icon source, cell selection, placement
   and count draw are READ (or measured-from-reference and named). Draw the ten buttons and
   their counts in the production render. Score the C1 gate (`x [18,90)` against the A7p/A7q
   captures with the panel exclusion removed); re-score the amended A7 gate without the panel
   exclusion and the C3 full frame on the A6 repair 4 pause by the C3a regions. Report
   whatever they are; do not tune.

## Budget

Per-round commits, up to three concurrent offline rounds, at most one live owner. **One
stock start, question 2 only, on its condition**, no retry after Popen. **No debugger.**
**Round time limit 30 minutes** from dispatch, replacing the function-body cap: a round at
its limit lands what it has read, names the next body and stops. Rounds still list the
bodies they read.

## Decisions already made (do not re-ask)

- Authorized as scoped above (user, overlord review 2026-10-24): C1 next; one conditional
  start with memory reads and at most one Frida entry hook; 30-minute round limit instead
  of the body cap (the cap of ten was breached in C3g, C3k and C3l and did not limit).
- **Read, do not fit, and do not choose by score.** A value taken from a scored capture is
  labelled measured-from-reference and named in the report. Sensitivity renders are reported,
  never used to pick a value. A render that matches the reference is a check, not a read.
- **Time the pass** (standing): total elapsed wall time and each round's dispatch-to-landing
  time, recorded in the queue when each round is dispatched and landed.
- Do not reuse heap addresses from an earlier process; `levelScreen` is `u32(0x4CBFB0)` per
  process. Use `acquire_game_foreground`, never `goto_level.focus()`. Live steps run in
  session 1 per [MOUV_SSH.md](agents/MOUV_SSH.md). Close any game the round leaves running,
  after checking it. Restore and hash-check every install file in the same process that
  changed it.
- Settled and not to be re-derived: everything settled in the previous nine directives,
  plus C1a–C1f (panel indices 2, 5, 6, 8, 9 via jump table `0x0045A4DD`; ten widgets at
  `levelScreen+0x70 + i*0x238`, ctor `FUN_004576b0`, vtable `0x4b1774`; `+8` written to
  `widget_index + 8`; `FUN_00457868` single-target; `GreyIcons`, `BlackIcons`,
  `AdditiveIcons`, `Bits`, `LemmingsLO` ruled out as the icon sheet) and C3k–C3m.
- Parked and not to be chased: the `+0x3c` writer and selection highlight (unless question 1
  reads it on the way), C3 scenery, the caption blend, the hatch animation, the
  `MM_DIG_000` shortfall, the HUD tick mismatch, and the `measure_d2_pick.py` matrix/vector
  swap (C2 cursor clause). B2 and B3 stay queued. Run CTest on mouv. Keep text files LF.

## Stop point

Stop and report when every question is landed, parked or not triggered, on any stop
condition in ORCHESTRATION, or if a live start crashes or leaves the install unrestored. Do
not advance to C3, the C2 cursor clause, the selection highlight, B2 or B3 on this directive.

## Sync

Run from the Mac clone `~/progs/lemmings-webolution`. Pull before selecting work and push
after each round commit, per [ROLES.md](ROLES.md#opencode-on-the-mac-clone). If a push from
mouv fails on credentials, leave the commits local and say so in the handoff; the overlord
carries them to GitHub.
