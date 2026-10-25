# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. Only a directive with `Status: **active**`
authorizes a run.

Status: **active**
Issued: 2026-10-25 (user, overlord review 2026-10-25, "i approve go ahead": C3 fix the known
render gaps, then one live start)
Supersedes: the close-C1-then-C3 directive, done 2026-10-25 17:26 MDT (run
`c1-close-c3-scene`: C1 closed, C3 0.2050523 → 0.0923688, 24/24 rounds, 0/3 starts,
stopped on round budget).

Client: **OpenCode**. Tier: **Sol**, `/facilitator` on mouv (`openai/gpt-6.1-sol`) or
`/facilitator-mac` on the Mac. Astra is not warranted: the last run stopped on budget, not on
a park. Recommend Astra only if this pass stalls well above 0.02 with error no named defect
explains ([ROLES.md](ROLES.md)).

## Why this

C3ae drew the scene for the first time and the full frame fell to **21707403/235008000 =
0.0923688**, gate ≤ 4700160. `rest` (floor included) is 19234589 of it. C3ae's note lists
integration defects that existing evidence already settles: they are fixes, not research.
C3ad's live controller was rejected in dry-check before Popen (`entry.RECORD_SIZE` undefined),
so no pause value was read and no start was spent.

## Scope

Milestone: **C3 full frame** only.

1. **Fix the known render gaps from existing evidence**, as listed in
   [C3ae](scratch/phase-c3ae-scene-render-score.md#important-narrowing-known-incomplete-integration-not-just-runtime-unknowns):
   billboard ancestor composition and source S (replay C3o's 17 tuples); camera-composed
   `collectEyes` context (reproduce C3p's census); C3y sorted child dispatch and type1
   radius/near dispatch; clipping and f32 packing/RHW; colour packing by truncating raw
   channels as C3n does, not ×255; wire the AA/AC retail tests into the CTest ENV list. One
   defect per brief, each with a concrete value or pixel acceptance check, so the first
   attempt can land. Re-score the repair4 full frame (`score_c3_regions.py --reference`,
   per-region table) after each landing and rank what remains by region error.
2. **One live round for the pause values** (`[game+0]`, `[game+0x5139]`, `0x58d498`, effective
   texture format/binding). First fix the controller's record-size contract and run its
   complete lifecycle mocked offline, failure and `finally` paths included. Then spend one
   start under the Budget rules. A disagreeing live read replaces a DERIVED value.
3. When the known gaps are fixed, rank the rest by region error: floor visibility, texture
   format and precision, entrance/exit models (Phase E, Exit width gap), HUD residue. A new
   reader round needs a written reason in its brief that it moves the gate.

The C3 gate is unchanged: the A6d24 full-frame score ≤ 0.02 on the A6 repair 4 pause with
actors, the floor ring included. Report C1 `zoom1`/`elev_down` (gated, must keep passing),
`elev_up` and the other A7p views with each full-frame score.

Not in scope: C2, B2, B3.

## Budget

- **Run:** about **6 hours or 16 rounds**. A repair counts as a round.
- **Round limit: 45 minutes from dispatch, repairs included** (user, overlord review
  2026-10-25). It replaces the 30-minute limit.
- **Rounds:** 3 per question, then park it with a resume brief and rank the next.
- **Concurrency:** up to 3 offline rounds at once, at most one live owner.
- **Stock starts:** a pool of **3**, each dry-checked offline first. Normal `Popen`, menu
  attach through the REA CLI (`rea instrument-with-frida attach`,
  [MOUV_SSH.md](agents/MOUV_SSH.md)). Python `frida.attach`, `frida.spawn` and suspended
  launches stay retired. Read-only hooks, one session, no game-memory write beyond the
  restored level-table slot. A crash ends the live arm; a second ends starts for the run.
  No further user authorization is needed for the start.
- **No debugger.** Per-round commits and pushes.

## Decisions already made (do not re-ask)

- **C1 is done** under the third amendment; its gate, footprint and views are frozen.
- **DERIVED** as in `.claude/rules/match-the-original.md`. Label it.
- **Do not choose by score.** A value comes from the read chain, not from fit. The software
  raster choices in C3ae (LINEAR/WRAP, LESS/write, MODULATE, sample 0.5) are declared
  conventions, not reads; do not tune them to the score.
- **Park by default.** A park-or-amend choice goes to `pending_questions` with a
  recommendation. Do not stop for it.
- The stack bottom is the camera block from `camera_a1/reference-state.json`,
  MEASURED-FROM-REFERENCE (C3k). `measure_d2_pick.py:149-156` role swap: note, do not fix.
- **Time the pass**, total and per round, in the queue. Landing per the 2026-10-24
  restructure.
- `levelScreen` is `u32(0x4CBFB0)` per process. Use `acquire_game_foreground`, never
  `goto_level.focus()`. Live steps run in session 1. Close any game a round leaves running,
  after checking it. Restore and hash-check every install file in the process that changed it.
- `frida` 17.24.3 stays installed for mouv's Python.
- Settled: everything in earlier directives, c1ap–c1bj and C3a–C3ae. Parked and out of scope:
  the caption blend, the hatch animation, the `MM_DIG_000` shortfall, the HUD tick mismatch,
  the `+0x3c` writer, C2 cursor, B3 map and level pick. Run CTest on mouv. LF only.

## Stop point

Stop and report only when:
- the C3 gate passes on your own independent re-score;
- a gate, its footprint, views or exclusions would need to change;
- C1 `zoom1` or `elev_down` regresses above 0.02;
- a guardrail breach: install hashes not restored, an unattributed process, a write left
  unrestored, or a worker exceeding its start budget or using a retired method;
- every remaining C3 question is parked or waits on `pending_questions`;
- the run reaches about 6 hours or 16 rounds;
- delegation or the pinned model is unavailable, or another session is running a task.

A landed render that still fails the gate is not a stop; rank and continue. Running low on
context is not a stop: leave the queue resumable, write `standing_run.handoff`, and say the
user should run `/facilitator` again.

## Sync

Pull before selecting work and push after each round commit, per
[ROLES.md](ROLES.md#opencode-on-the-mac-clone). If a push from mouv fails on credentials, leave
the commits local and say so in the handoff; the overlord carries them to GitHub.
