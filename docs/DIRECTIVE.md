# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **active**
Issued: 2026-10-23 (user, overlord review 2026-10-23)
Supersedes: the B1 sheet-identity directive, closed 2026-10-23 (B1n ambiguous, B1o
preflight failed before launch, B1 parked).

## Scope

Three questions. 1 and 3 may run concurrently (offline and live, disjoint paths); 2
waits on 1.

1. **Texture-loader read (offline, `disassembly read`, new question, cap three).** B1o
   found the generic loader `0x47218A`: `^`-prefix test (`0x47446C`), conditional
   `Lo`/`Hi` suffix on byte `[0x5CD5F2]` (`0x472223`), `Textures\%s.bmp` at `0x472282`
   with a fallback at `0x4722EB`, and an `.alf` branch. Read its unresolved callees
   `0x47244B`, `0x445830` and `0x472490`, and what writes `[0x5CD5F2]` and with what
   value on a stock install, as far as bytes allow. Apply the rule to the name `^Font`.
   Land when exactly one archive file follows from the bytes; otherwise park. This is a
   new question (the loader, shared by every texture) rather than a fifth B1 binding
   round; record the reason in the brief.
2. **B1 render and score (offline, up to two rounds; only if question 1 yields exactly
   one sheet).** Main menu at highlight 0 from the read charmap, label table `0x4C4214`,
   Y = 320 + 29 x row, X = 320 (B1m) and the read sheet, with `UK.LTX`. Score full-frame
   against the B1b reference and the B1 gate. Rendering does not need a highlight
   accessor: B1b's reference was live-bracketed at highlight 0.
3. **B2 screen state machine (new milestone, up to three rounds, one stock start).**
   Round 1: offline preflight from bytes of the screen-id read at `0x004C416C` and the
   key-input path the harness will drive, then one stock start to record the original's
   screen-id sequence for the gate's scripted key sequences (menu, confirm, map, into a
   level, back), including the RETURN-only-at-highlight-0 and short-ESC-in-pause quirks.
   Later rounds: implement our state machine offline to match the recorded sequence.

## Budget

The grants of `d-playable-3`: per-round commits, up to three concurrent offline rounds,
at most one live owner. Questions 1 and 2 use **no** stock starts. Question 3 gets
**one**, no retry after Popen. Three rounds per question.

## Decisions already made (do not re-ask)

- **Read, do not fit, and do not choose by score.** If question 1 leaves more than one
  sheet, park it; do not render the candidates and pick the best scorer (user, overlord
  review 2026-10-23). B1f/B1h fitting stays parked; the Quit-row 0.0202 scalar is not a
  score.
- Settled and not to be re-derived: B1i's charmap and advance rule, with B1n's correction
  (the 256-record extent is unestablished; menu cells and the Quit replay stand); B1k's
  label table, `^Font` binding and Y rule; B1l's file-negatives; B1m's X word and pointer
  chain offsets; B1n's five-sheet ambiguity under bounds/ink/blank predicates; B1o's
  eight-body read; C2a/C2b's Out, Saved and timer reads.
- `0x004C416C` is a monotonic screen-construction counter from a shared base ctor
  (`0x004882c0`), not an enum (B1 recon). Matching its values means matching construction
  order.
- **Live preflight is mandatory.** Validate every address a live round will read, offline
  and from bytes, before launch. Do not reuse heap addresses from an earlier process. Use
  `acquire_game_foreground`, not `goto_level.focus()`.
- **Restoration:** restore and hash-check every install file in the same process that
  changed it.
- English menus are the reference; do not reset the install language.
- C1 stays held.

## Stop point

Stop and report when every question is landed or parked (the B1 gate passing on your own
independent re-score counts as landed). Also stop on any stop condition in ORCHESTRATION.
Do not advance to B3, C1, C2 or C3 on this directive.

## Sync

Pull before selecting work and push after each round commit, per
[ROLES.md](ROLES.md#opencode-on-the-mac-clone). If a push from mouv fails on credentials,
leave the commits local and say so in the handoff; the overlord carries them to GitHub.
