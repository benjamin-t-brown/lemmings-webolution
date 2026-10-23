# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **active**
Issued: 2026-10-22 (user, overlord review 2026-10-22)
Supersedes: the font-charmap / C1-live directive, closed 2026-10-22
(`font-charmap-c1-live`, both questions landed).

## Scope

B1i read the original's text-draw path from bytes (`FUN_004404fa`; 256 `{X, Y, width}`
records, proportional advance, one `lineHeight` per font). The five fitting rounds failed
because they assumed a fixed grid. This pass uses the read to attempt the B1 gate. Two
questions:

1. **B1 main-menu render (offline, up to three rounds).** Which of `FUN_004404fa`'s twelve
   callers draws the main menu, with which font object, sheet and label placement; then
   wire the read charmap into our renderer and score the main menu at highlight 0 full-frame
   against the B1 gate and the retained B1b reference. The main risk is the frontend sheet's
   identity: B1i found no `_FONTFE`, `^FONTH` or `^FONTP` name string, so which sheet backs
   the menu font must be read, not assumed. Reuse B1e's exports and B1b's reference capture.
2. **Paused caption and timer read (offline, `disassembly read`, in parallel).** Verify B1i's
   unverified lead: `0x44CD06` fills `H+0x758`/`H+0x77C` through `FUN_004404fa` and draws the
   MM:SS timer at `H+0x224` (560, 430). Read and record what is drawn, with which font and
   where. C2 groundwork only: no render, no C2 gate attempt. Owned paths must be disjoint
   from question 1.

## Budget

The grants of `d-playable-3`: per-round commits, up to three concurrent offline rounds.
**No stock starts** on this directive: both questions are offline. Three rounds per
question. Disassembly-read rounds use the `disasm-agent` tier per
[ROLES.md](ROLES.md#disassembly-reads-use-a-stronger-worker).

## Decisions already made (do not re-ask)

- **Read, do not fit.** Glyph placement comes from the read table and the read caller, not
  from pixel or scalar optimisation. The B1f/B1h fitting methods stay parked; the Quit-row
  0.0202 scalar is not a score.
- B1i's charmap, advance and `lineHeight` rule is settled (facilitator re-read from raw PE
  bytes). Do not re-derive it. `&` in `^Font` is a zero-width entry: preserve it.
- **C1 is held.** `FUN_00457868` draws one widget per frame (C1f, live); the base-icon source
  is a new live question and is not in this pass.
- English menus are the reference; do not reset the install language.
- Live-harness note for any future live round: C1f's `probe.py` cleanup did not restore
  `dgVoodoo.conf` on its own (fixed by hand, hashes match). Not in scope here.

## Stop point

Stop and report when the B1 gate passes on your own independent re-score, or when question
1 reaches its round cap, and question 2 is landed or parked. Also stop on any stop
condition in ORCHESTRATION. Do not advance to B2, C1, C2 or C3 on this directive.

## Sync

Pull before selecting work and push after each round commit, per
[ROLES.md](ROLES.md#opencode-on-the-mac-clone). If a push from mouv fails on credentials,
leave the commits local and say so in the handoff; the overlord carries them to GitHub.
