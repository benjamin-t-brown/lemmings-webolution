# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **active**
Issued: 2026-10-23 (user, overlord review 2026-10-23)
Supersedes: the B1 main-menu render directive, closed 2026-10-22 (`b1-menu-render`,
question 1 at its cap, question 2 landed).

## Scope

B1j-B1l read everything static about the main-menu text: labels are the table at
`0x4C4214` (UK 125, 126, 120, 3, 43), measured with `^Font` (table `0x4BF968`,
`lineHeight` 29), Y = 320 + 29 x row. Two values exist only at runtime and are the last
unknowns for the B1 gate. Two questions, in order:

1. **B1 runtime read (live, one stock start).** On the main menu at highlight 0, read
   the word at `[0x58CB40]` (X is its signed half) and the texture `^Font` resolves to
   through manager `0x58CB48` (`0x474519`): which sheet, its source and dimensions. If
   cheap in the same start, confirm one label's drawn Y against 320 + 29 x row. Cap three
   rounds. Gate relevance: these are the only inputs the B1 score still lacks.
2. **B1 render and score (offline, up to two rounds, after question 1 lands).** Wire the
   read charmap, label table, Y rule and read X into our frontend renderer, draw with the
   read sheet, and score the main menu at highlight 0 full-frame against the B1b reference
   capture and the B1 gate. Use `UK.LTX` (the original asks for `Language\Uk.ltx`, C2b);
   B1a's `GERMAN.LTX` test is not the reference.

## Budget

The grants of `d-playable-3`: per-round commits, up to three concurrent offline rounds.
**One stock start** for question 1; no retry after Popen. Question 2 is offline.

## Decisions already made (do not re-ask)

- **Read, do not fit.** Placement and sheet come from reads, not from pixel or scalar
  optimisation. B1f/B1h fitting stays parked; the Quit-row 0.0202 scalar is not a score.
- Settled and not to be re-derived: B1i's charmap and advance rule; B1k's label table,
  `^Font` binding and Y rule; B1l's file-negatives (no static store to `[0x58CB40]`, no
  BMP name for `^Font` in the exe); C2a/C2b's Out, Saved and timer reads.
- **Restoration:** C1f's `probe.py` cleanup did not restore `dgVoodoo.conf` by itself (it
  was fixed by hand). Question 1's brief must restore and hash-check every install file
  explicitly, in the same process that swapped it.
- English menus are the reference; do not reset the install language.
- C1 stays held; its base-icon live read is the overlord's likely next pass, not this one.

## Stop point

Stop and report when the B1 gate passes on your own independent re-score, or when either
question reaches its cap. Also stop on any stop condition in ORCHESTRATION. Do not advance
to B2, C1, C2 or C3 on this directive.

## Sync

Pull before selecting work and push after each round commit, per
[ROLES.md](ROLES.md#opencode-on-the-mac-clone). If a push from mouv fails on credentials,
leave the commits local and say so in the handoff; the overlord carries them to GitHub.
