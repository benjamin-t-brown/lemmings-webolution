# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **active**
Issued: 2026-10-23 (user, overlord review 2026-10-23)
Supersedes: the B1 panel / C2 counters / C2 caption directive, done 2026-10-23 (B1 panel colours
and X pad read, score 0.005244; counters and timer drawn, amended A7 passes; caption quad read
live and drawn, A7 without the caption exclusion fails 3 of 10; the one start spent).

## Scope

Two questions, both offline. They may run together.

1. **C2 caption text and blend (cap two rounds).** Draw the `Paused` text from `STR_PAUSED`
   (key at `0x4C3C18`) through the `G+0x25C68` list, whose placement C2h read live as 285,422,
   using the `^Font` path HudText uses. Read the consumer of the key and the text colour. Read
   the blend and the panel's colour path (`0x46D460`, `0x44DBA0`, `0x4017F0`, and what 0.00225
   at `0x44DCC0` sets) so that the c2i fill, alpha and blend stop being hypotheses. Re-score
   the A7 nine views and the second level with the caption exclusion removed and only the
   ten skill boxes excluded. Report it whatever it is; do not tune. Target: all ten views pass.
2. **C3 residual breakdown (cap one round; measure only, no fixes).** On the A6 repair 4 pause
   with actors (the A6d24 frame and scorer), with today's production render including the
   C2 counters, timer and caption, re-score full-frame and split the numerator across named,
   disjoint regions: the ten skill boxes, the counters and timer boxes, the caption box, the
   floor ring, the actor boxes, the tube ROI and the rest. State how each region is bounded and
   where the bounds come from. Report each region's share. This is input for the overlord's
   ranking of C1 and C3, not a gate attempt.

## Budget

Per-round commits, up to three concurrent offline rounds. **No stock start, no debugger, no
live game.** If question 1 cannot settle a value without a live read, record it and park that
value; do not ask for a start on this directive.

## Decisions already made (do not re-ask)

- Authorized as scoped above (user, overlord review 2026-10-23).
- `q3-caption-round-cap` was answered and is closed; clear it from `pending_questions`.
- **Read, do not fit, and do not choose by score.** A value taken from a scored capture is
  labelled measured-from-reference and named in the report. Sensitivity renders are reported,
  never used to pick a value.
- **Time the pass** (standing): total elapsed wall time and each round's dispatch-to-landing
  time, recorded in the queue when each round is dispatched and landed.
- Settled and not to be re-derived: everything settled in the previous five directives, plus
  C2c/C2d/C2g (counter positions absolute, digits replace the words, Saved colour, right end
  620, y 15 / 50, timer at 560,430), C2f (caption quad at `G+0x25C90`, formula at
  `0x45D2A8..0x45D340`, `0x44E150` is the generic cell-list draw) and C2h (quad 277,422,363,454;
  `[0x58CB40]` 640, `[0x58CB44]` 480; text placement 285,422; all live).
- C1 stays held. B2 and B3 stay queued. Run CTest on mouv. Keep text files LF.

## Stop point

Stop and report when both questions are landed or parked, or on any stop condition in
ORCHESTRATION. Do not advance to C1, C3 fixes, the C2 cursor clause, B2 or B3 on this
directive.

## Sync

Run from the Mac clone `~/progs/lemmings-webolution`. Pull before selecting work and push
after each round commit, per [ROLES.md](ROLES.md#opencode-on-the-mac-clone). If a push from
mouv fails on credentials, leave the commits local and say so in the handoff; the overlord
carries them to GitHub.
