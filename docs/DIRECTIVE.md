# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **done** (2026-10-24 14:26 MDT: part 2 ended at its cap. `0x47218A` is the texture-loader entry, not a colour-key test. `0x46D5F3` is `call [ecx+0x70]`; the blend mode is unread. `^tab` pixels are unread. Part 3 was not started. No further round on this directive.)
Issued: 2026-10-24 (user, overlord review 2026-10-24; "A approve commit and push", option A of
the c1-sheet-pixels-hook-render evaluation)
Supersedes: the sheet-pixels-and-hook directive, done 2026-10-24 13:43 MDT (c1r: sheet members
decode through `0x467DE8`; c1s: icon draw x, y and white count quads, measured with one Frida
hook; c1t: icons and counts drawn, gate failed at 0.1077 / 0.0554, unchanged from before the
panel because the scorer's mask meets the panel in 0 pixels).

## Why this

C1's inputs for icons and counts are read, but the scorer never looked at them. The user
amended the gate's measurement in `PLAN.md` (C1 row and the note above C1a): same 0.02, same
region and captures, scored over the panel's own footprint. What still differs from the
stock frame, by eye: the oval button plates are not drawn, and the stock frame shows a count
only on button 0 (`10`) where we draw `00` on the others.

## Scope

Three parts, in order. 1 and 2 may run together; 3 waits for both.

1. **Footprint mask (offline, cap one round).** Build the C1 footprint from read geometry only:
   the c1s icon positions with the c1p/c1r frame sizes, the count glyph rectangles at the c1s
   text-object positions, and the plate rectangles once part 2 reads them (land the icon and
   glyph part first if the plates are not yet read; the plate part joins before part 3).
   Commit it as a fixture with its derivation, and score the c1t render and the no-panel
   render against it **before** any new drawing, so the baseline is on record. The mask is
   never edited after a part-3 score.
2. **Remaining draw reads (offline, cap two rounds).** The button plate (sheet, frame,
   position, draw call), the condition that hides a zero count (the c1s quads are white, so
   look for a visibility flag or a skipped draw), the colour key at `0x47218A` and the blend
   at `0x46D5F3`. Run the exe's own bytes where c1q and c1r did. If a value can only be read
   live, name the value and address and stop the part; do not ask for a start on this
   directive.
3. **Render and score (offline, cap two rounds).** Draw the plates and apply the read count
   rule, then score both views against the committed footprint. Report pass or fail and the
   share of the error that falls on translucent plate pixels over parked backdrop. Do not
   tune to the score.

## Budget

Per-round commits, up to two concurrent offline rounds. **No stock start, no debugger, no
Frida.** Round limit 30 minutes from dispatch; a round at its limit lands what it has read,
names the next body and stops.

## Decisions already made (do not re-ask)

- The C1 gate's measurement is amended as written in `PLAN.md` (user, overlord review
  2026-10-24). Do not reopen the threshold or the region; do not use the C1b
  `baseline != no-cell-routes` mask for acceptance.
- **Read, do not fit, and do not choose by score.** A value taken from a scored capture is
  labelled measured-from-reference and named in the report. A footprint edge taken from the
  capture instead of from read geometry is such a value.
- **Time the pass** (standing): total elapsed wall time and each round's dispatch-to-landing
  time, recorded in the queue when each round is dispatched and landed.
- Fill the queue fields the last pass left empty: `stopped.last_commit`, and a worker output
  path that exists.
- Settled and not to be re-derived: everything settled in the previous thirteen directives,
  plus c1r, c1s and c1t. The menu-entry cause stays unnamed; do not chase it.
- Parked and not to be chased: C3 scenery, the caption blend, the hatch animation, the
  `MM_DIG_000` shortfall, the HUD tick mismatch, the selection highlight and `+0x3c` writer,
  and the C2 cursor clause. B2 and B3 stay queued. Run CTest on mouv. Keep text files LF.

## Stop point

Stop and report when part 3 is scored (pass or fail), when part 2 ends without an input the
render needs, or on any stop condition in ORCHESTRATION. Do not advance to C3, the C2 cursor
clause, B2 or B3.

## Sync

Pull before selecting work and push after each round commit, per
[ROLES.md](ROLES.md#opencode-on-the-mac-clone). If a push from mouv fails on credentials, leave
the commits local and say so in the handoff; the overlord carries them to GitHub.
