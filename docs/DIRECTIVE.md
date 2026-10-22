# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **active**
Issued: 2026-10-22 (user, overlord review 2026-10-22)

## Scope

- **Milestone:** C1 skill panel ([PLAN.md Phase C](../PLAN.md)). Gate is signed off as written:
  the ten buttons and their counts render inside `x [18,90)` at <= 0.02 against the A7p/A7q
  captures, with the A7 panel exclusion removed.
- **Approach:** offline first. Reuse C1a's physical panel indices (0 climber .. 9
  Armageddon). Render the ten button sprites before the counts. Counts need digit glyphs,
  which is where B1 stalled; if they need the retail font and the font mapping is not
  already read from bytes, score the buttons alone, report that, and do not start glyph
  fitting.
- **Budget:** the standing-run grants of `d-playable-3` (stock starts within each brief's
  budget, per-round commits, up to three offline rounds). Three rounds per question.

## Decisions already made (do not re-ask)

- **B1 glyph question is parked in Phase E** (user, overlord review 2026-10-22). Five rounds
  against a cap of three; B1h ran about 65 of 35 minutes. Park it with the resume brief
  already in PLAN.md. No glyph fitting, no menu-vtable search, no fifth automatic pass.
  Resume only under new explicit authorization after a source-identity review and one
  semantic glyph discriminator.
- B1 stays **not accepted**; its gate is unchanged. Do not use the Quit-row 0.0202 scalar
  as a score.
- English menus are the reference; do not reset the install language.

## Stop point

Stop and report when the C1 gate passes on your own independent re-score, or when C1 is
parked at its round cap, or on any stop condition in ORCHESTRATION. Do not advance to C2 or
B2 on this directive. The overlord ranks the next pass.
