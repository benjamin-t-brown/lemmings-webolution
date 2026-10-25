# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date; move the stopped run to `previous_runs` in
[the queue archive](tasks-archive.json)), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **done** (2026-10-25 13:05 MDT). C1 gate still fails on elev_up. The remainder is the scene visible through the plate interior, and this directive excludes that scene. The gate was not changed.
Issued: 2026-10-25 (user, overlord review 2026-10-25: "approve all": DERIVED values from
documented Win32 behaviour, value-framed questions, C1 only)
Supersedes: the long multi-milestone directive, done 2026-10-25 03:29 MDT (run `c1-c2-b3`:
14 rounds, 0 starts, every ranked question parked, no gate scored).

## Why this

The last run read one step back per round and parked each question at "the number is
unread". This run frames each question by the value it needs, and allows one new evidence
class so the C1 chain can close offline.

The overlord's reading, not evidence. Landed: c1ba (`0x45015A` tests the buffer for `BM`),
c1ba and c1au (the name is `Textures\%s.bmp` with `^tab`), c1ay (`+6` compares word `+0x12`
of a `0x18`-byte `GetObjectA` buffer), c1az (the handle is `CreateDIBSection`'s return,
stored at `[object+4]`), C1y (`^TAB.BMP` is 24 bpp). Documented: a `0x18`-byte `GetObjectA`
on a DIB-section `HBITMAP` fills a `BITMAP` whose `bmBitsPixel` is the word at `+0x12`. So
`+6` = (24 <= 8) = 0. With `+4` = 1, `+5` = 0 and `+7` = 0 (c1ax), the c1aj popcount arm keeps
exactly one record of the c1as list: line 29, A4R4G4B4 (A `0xF000`)
(`build/tmp/overlord-2026-10-25/keepcheck.py` re-applies the arm to the 20 `$LemBits0`
records; the A1R5G5B5 record (n=11) fails the popcount test).
Verify this chain; do not take this paragraph as the verification.

## New evidence class: DERIVED (user, overlord review 2026-10-25)

A value may be **DERIVED** when every link is either read from this exe or the install, or is
documented Win32 / DirectX behaviour (cite the struct, field and offset from the Microsoft
documentation or SDK header). Label it DERIVED in code and notes, never MEASURED or READ,
and name the documented link. A later live read that disagrees replaces it. Add this class
to `.claude/rules/match-the-original.md` (facilitator-owned) in the first landing round.

## Scope

**C1 skill panel, to its gate** (`PLAN.md` C1 row as amended twice). Questions are framed by
the value needed. A round follows the chain until it has the value or reaches its time
limit; "the next link is unread" ends a round only when the limit is hit, not by choice.

1. **What is `+6` for `^tab`, and which format record is kept?** Close the c1az and c1ba
   narrowings: that the buffer copied onto `0x58D5B8` is `^TAB.BMP`'s, and that `[object+4]`
   is still the `CreateDIBSection` handle when `0x473040` runs (the `0x472FAD` import and
   `0x472FB2` `call eax` sit between). Record `+6` and the kept record as DERIVED, and as the
   value for this install's format list (dgVoodoo).
2. **What 16-bit word does the upload store for each `^tab` texel?** The shift path
   (`0x473866`..`0x473955`) with the kept masks: the alpha shifts and how each colour
   channel is reduced. Read it, do not assume.
3. **Render and score.** Draw the plates (C1y texels, item 2's words, blend selector 1 from
   c1ad, counts and icons as landed). Score `zoom1`, `elev_up`, `elev_down` against their A7p
   `baseline.png` over the frozen footprint. Report `initial`, the other A7p views,
   `MM_DIG_000` and the scenery-only error beside them.
4. If the gate fails, rank what is left inside the footprint (count tint, selection state,
   icon edges) by error and keep going.

**Ranking after C1bd (user, overlord review 2026-10-25, "yes"; budget unchanged).** The
overlord's split of C1bd's saved PNGs (`build/tmp/overlord-2026-10-25/residual.py`;
evaluation, not evidence) puts the zero-count glyphs at 0.0112–0.0113 of each gated score and
plate 0 at 0.0136 / 0.0351 / 0.0141; the rest is 0.0090–0.0095. Stock draws no `00` text and
a ring on plate 0. Rank in this order, each read before it is drawn:
1. **Zero-count skip:** the landed C1v branch (a zero `[this+i*4]` skips the text submit).
2. **Selected plate:** the `0x45B6B5` `^tab_selected` draw gated on `[this+0x2750]`, its
   selected index at the A7p pause, with the C1bc selected stream. Read the index; do not pick
   plate 0 because the stock frame shows a ring there.
3. **Plate filter (c1be)** only if the gate still fails after 1 and 2.

Not in scope: C2, C3, B2, B3. Do not advance to them if C1 passes; stop and report.

## Budget

- **Run:** about **8 hours or 24 rounds**. Round limit 30 minutes from dispatch.
- **Rounds:** 3 per question, then park it with a resume brief and rank the next. A further
  round needs a written reason in its brief that it moves the gate.
- **Concurrency:** up to 3 offline rounds at once, at most one live owner.
- **Stock starts:** a pool of **3**, each dry-checked offline first, only for a value the
  offline chain cannot reach. Normal `Popen`, menu attach through the REA CLI
  (`rea instrument-with-frida attach`, [MOUV_SSH.md](agents/MOUV_SSH.md)). Python
  `frida.attach`, `frida.spawn` and suspended launches are retired for game starts.
  Read-only hooks, one session, no game-memory write beyond the restored level-table slot.
  A crash ends that question's live arm; a second crash ends starts for the run.
- **No debugger.** Per-round commits and pushes.

## Decisions already made (do not re-ask)

- `+7` = 0 is READ (c1ax). Do not re-derive it.
- **Park by default.** A capped question is parked and the next ranked; a park-or-amend choice
  goes to `pending_questions` with a recommendation. Do not stop for it.
- Gates as signed off in `PLAN.md`. The C1 footprint fixture
  `tools/harness/fixtures/c1-footprint/footprint.json` (SHA-256 `e96840e0…`) is frozen.
- **Do not choose by score.** The 0.47 plate brightness ratio
  (`build/tmp/overlord-2026-10-24/tabratio.py`) is a falsifier only. A format comes from the
  keep rule applied to the logged list, not from fit.
- Do not reopen when the `^tab` upload runs, the startup crash's fault address or the
  menu-entry cause.
- `frida` 17.24.3 stays installed for mouv's Python (user, 2026-10-24).
- **Time the pass** (standing), total and per round, in the queue.
- **Landing** per the 2026-10-24 restructure: detail in the scratch note, round prose in
  `docs/plan-history/`, a one- or two-line status cell, landed tasks in the queue archive.
- `levelScreen` is `u32(0x4CBFB0)` per process. Use `acquire_game_foreground`, never
  `goto_level.focus()`. Live steps run in session 1 per [MOUV_SSH.md](agents/MOUV_SSH.md).
  Close any game the round leaves running, after checking it. Restore and hash-check every
  install file in the same process that changed it.
- Settled: everything in earlier directives and c1ap–c1ba. Parked: C3 scenery, the caption
  blend, the hatch animation, the `MM_DIG_000` shortfall, the HUD tick mismatch, the `+0x3c`
  writer, C2 cursor (c2l–c2n), B3 map and level pick (b3a–b3g). Run CTest on mouv. LF only.

## Stop point

Stop and report only when:
- the C1 gate passes on your own independent re-score;
- a gate, its footprint, views or exclusions would need to change;
- a guardrail breach: install hashes not restored, an unattributed process, a write left
  unrestored, or a worker exceeding its start budget or using a retired method;
- every remaining C1 question is parked or waits on `pending_questions`;
- the run reaches about 8 hours or 24 rounds;
- delegation or the pinned model is unavailable, or another session is running a task.

Running low on context is not a stop: leave the queue resumable, write
`standing_run.handoff`, and say the user should run `/facilitator` again.

## Sync

Pull before selecting work and push after each round commit, per
[ROLES.md](ROLES.md#opencode-on-the-mac-clone). If a push from mouv fails on credentials, leave
the commits local and say so in the handoff; the overlord carries them to GitHub.
