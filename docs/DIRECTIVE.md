# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **active**
Issued: 2026-10-23 (user, overlord review 2026-10-23)
Supersedes: the B1 runtime-read directive, closed 2026-10-23 (`b1-runtime-render`, B1m
spent its one start without the sheet binding).

## Scope

The B1 gate still lacks one input: which sheet backs `^Font`. B1m read X = 320 (word 640
at `0x58CB40`, without a highlight bracket) and mapped the runtime chain to the sheet
(pool base `[0x58CB84]`, record index*0x88 + pool+0x0C, descriptor +0x14/+0x18), but
captured none of its values. Three questions, in order:

1. **Sheet identity from the read glyph tables (offline; round 3 of 3 on the B1 runtime
   question).** Test B1i's `^Font` table (`0x4BF968`, `lineHeight` 29) and `_FontSmall`
   table (`0x4BFC10`, `lineHeight` 14) against every retail font sheet in the archives.
   Each cell's `{X, Y, width} x lineHeight` rectangle must lie inside the sheet; ink
   cells (`Q`, `M`, digits) must hold ink; zero-width and blank cells (`&`, space) must
   fall where the read table predicts. Accept a sheet only if exactly one is consistent
   with every cell, by a re-runnable script. Record it as an **inference from read
   tables**, not a runtime read. This is the semantic glyph discriminator B1h's resume
   brief asks for. It is not fitting: no optimiser, no scalar score, no free parameters.
2. **B1 render and score (offline, up to two rounds; only if question 1 accepts one
   sheet).** Draw the main menu at highlight 0 from the read charmap, the label table at
   `0x4C4214`, Y = 320 + 29 x row, X from B1m (320, signed half of 640) and the accepted
   sheet, with `UK.LTX`. Score full-frame against the B1b reference and the B1 gate. A pass
   is also the check on B1m's unbracketed X.
3. **Live fallback (pre-authorized by the user; only if question 1 is ambiguous or
   negative).** One stock start, recorded as a cap exception (round 4 on the runtime
   question). Before Popen, an offline preflight must prove, from bytes, the
   current-process highlight accessor and the full pointer chain from `[0x58CB84]` to the
   record, wrapper, descriptor dimensions and the source name or data. Do not reuse
   B1m's heap addresses or index 36. If the preflight cannot close, park instead of
   launching. Then return to question 2 with the read sheet.

## Budget

The grants of `d-playable-3`: per-round commits, up to three concurrent offline rounds.
Questions 1 and 2 use **no** stock starts. Question 3 gets **one**, only on its
condition, no retry after Popen.

## Decisions already made (do not re-ask)

- **Read, do not fit.** Placement and sheet come from reads or from the table test in
  question 1, never from pixel or scalar optimisation. B1f/B1h fitting stays parked; the
  Quit-row 0.0202 scalar is not a score.
- Settled and not to be re-derived: B1i's charmap and advance rule; B1k's label table,
  `^Font` binding and Y rule; B1l's file-negatives; B1m's X word and pointer chain
  (offsets only); C2a/C2b's Out, Saved and timer reads.
- **Live preflight is mandatory.** B1m spent its start on an unchecked pointer reading.
  Any live round validates every address it will read, offline, before launch.
- **Restoration:** restore and hash-check every install file in the same process that
  changed it.
- English menus are the reference; do not reset the install language.
- C1 stays held.

## Stop point

Stop and report when the B1 gate passes on your own independent re-score, or when B1 is
parked (a question at its cap, question 3's preflight failing, or its start spent without
the binding). Also stop on any stop condition in ORCHESTRATION. Do not advance to B2, C1,
C2 or C3 on this directive.

## Sync

Pull before selecting work and push after each round commit, per
[ROLES.md](ROLES.md#opencode-on-the-mac-clone). If a push from mouv fails on credentials,
leave the commits local and say so in the handoff; the overlord carries them to GitHub.
