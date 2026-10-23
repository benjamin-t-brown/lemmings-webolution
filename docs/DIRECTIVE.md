# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **active**
Issued: 2026-10-22 (user, overlord review 2026-10-22)
Supersedes: the C1 directive, closed 2026-10-22 (`c1-skill-panel`, parked at its round cap).

## Scope

Text is the critical path: B1, the C1 counts, C2, C3 and B3 all draw it, and five pixel-fit
rounds failed to get a glyph layout. This directive changes the approach from fitting to
reading. Two questions, in this order:

1. **Font charmap read (offline, `disassembly read`).** How the original maps a character
   code to a glyph cell in `_FONT.BMP` / `_FONTFE.BMP`, and how it advances and places each
   glyph, read from the original's text-draw code. Gate relevance: it unblocks the B1
   full-frame score and the C1 counts half. Stop when the charmap and advance rule are read
   from bytes and checked against one string drawn from retail text, or park it.
   Before dispatch, search `docs/LESSONS.md`, `PLAN.md` Phase E and `docs/scratch/phase-b1*`
   for text-draw reads already made; reuse them, do not re-derive.
2. **C1 live read (one stock start).** Use the resume brief in `PLAN.md` Phase E ("C1
   skill-panel icon-draw mechanism"): which widget `+8` icon index holds what value during an
   authorized stock start, and whether an indirect dispatch reaches the other nine widgets.
   Scope it as a live read, not another static byte or literal search. Stop when the read is
   recorded or the start budget is spent.

Question 1 may run alongside question 2 (offline and live, disjoint paths). Only one live
owner at a time.

## Budget

The grants of `d-playable-3`: per-round commits, up to three concurrent offline rounds,
stock starts within each brief's own budget. Three rounds per question. The C1 live read
gets **one** stock start. Disassembly-read rounds use the `disasm-agent` tier per
[ROLES.md](ROLES.md#disassembly-reads-use-a-stronger-worker).

## Decisions already made (do not re-ask)

- **B1 glyph fitting is parked in Phase E** (user, overlord review 2026-10-22). Five rounds
  against a cap of three. Do not resume pixel or scalar fitting, a menu-vtable trace, or
  the B1h matcher. The Quit-row 0.0202 scalar is not a score.
- **C1's icon-draw static search is exhausted** by two independent methods. Do not run
  another static literal scan for the widget `+8` writer.
- B1 and C1 stay **not accepted**; their gates are unchanged.
- English menus are the reference; do not reset the install language.

## Stop point

Stop and report when both questions are answered (landed, or parked at their cap), or on
any stop condition in ORCHESTRATION. Do not advance to B2, C2 or C3 on this directive, and
do not re-attempt the B1 or C1 gate until the overlord ranks it. A passing gate on your own
independent re-score also stops the run.
