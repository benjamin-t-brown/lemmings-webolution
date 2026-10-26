# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. Only a directive with `Status: **active**`
authorizes a run.

Status: **active**
Issued: 2026-10-25 (user, overlord review 2026-10-25, "approved": C3 host-tested live
re-capture, scene orientation and tube ends, remaining known gaps)
Supersedes: the C3 fix-known-gaps directive, done 2026-10-25 21:34 MDT (run
`c3-known-gaps-pause-values`: 0.0923688 → 0.0635482, 16/16 rounds, 0/3 starts; the one-shot
was rejected before Popen on a numeric ScheduledTask state).

Client: **OpenCode**. Tier: **Sol**, `/facilitator` on mouv (`openai/gpt-6.1-sol`) or
`/facilitator-mac` on the Mac. The score is still converging; Astra is not warranted.

## Why this

C3ao: **14934330/235008000 = 0.0635482**, gate ≤ 4700160. Regions: rest/floor 12497635,
skills 1495773, tube 674411, counters 175829, caption 75548, actor boxes 15134. Several
read-supported fixes (C3al alpha, C3an raw diffuse, C3ao order) made the screenshot worse,
which points at an upstream error rather than at those values.

The user compared the frames (2026-10-25) and the overlord checked each point on the saved
PNGs (`build/tmp/overlord-2026-10-25/c3-compare-ao.png`, `c3-strips.png`, `c3-actors.png`):

- **The reference holds state our candidate does not reproduce.** The original shows four
  walkers and an out-count of 4; ours shows three and 5. `capture_matched_pause.py` persists
  only `actors[:3]` (`a6f-fresh-pause-gate/repair4/packet-scope-review.md:72-75`), so the
  fourth walker (near (345,330)) was never recorded. The cursor hovers the hatch, so the
  reference also carries the hover highlight ("E", green marker, "20"), about 350k of
  numerator in a 70×90 box at (285,228).
- **The scene may be posed wrongly.** The checker patch and balloon on the left and the
  horizon on the right sit differently. Overlord 40-px grid mean errors: most of the frame
  0.012–0.025; left edge x 0–90 at y 280–360 0.198–0.283; right edge x 560–640 0.26–0.28
  (partly the orange discs over the counters).
- **The tube's top and bottom bands are worse than its body**: y 0–40 0.043–0.070,
  y 440–480 0.064–0.152. This is consistent with the open primary-before-tube ordering debt
  or undrawn cylinder ends; it is not established.

These are observations and leads. Values still come from the read chain.

## Scope

Milestone: **C3 full frame** only. Big errors first (user, overlord review 2026-10-25: "get
the big rocks better before refining"). Overlord split of C3ao's 14.93M: right edge x 560–640
4.98M (orange discs 2.14M of it), left edge x 0–90 3.59M, middle 3.99M, tube top/bottom bands
2.39M, hatch cursor 0.35M. The two edge strips hold 57% of the error. Run the offline items
**alongside** the live arm, which needs only the live owner.

1. **Live arm, tested on the host.** Repair the controller's ScheduledTask state handling
   (numeric enum, C3 one-shot note). Then dry-run the actual controller on mouv, in session 1,
   through every check up to the line before `Popen`, instead of only against mocks. Then spend
   one start to do both of these:
   - read the pause values (`[game+0]`, `[game+0x5139]`, `0x58d498`, the effective texture
     format and binding);
   - **re-capture the repair4 pause** with the recipe that made it. Park the cursor over plain
     scenery, clear of actors, hatch and HUD. Persist the **full actor list** and the HUD
     counter values with the capture.
2. **Scene pose** (offline, parallel to 1). Check the scene roots' pose and yaw against the
   read camera chain, using the checker patch, the balloon and the horizon as landmarks to test
   against, not as targets to fit.
3. **Orange discs over the counters** (offline, parallel to 1). Which node, texture and blend
   draws them, and what the original draws there instead.
4. **Tube ends.** Find what the original draws for the cylinder's top and bottom and in what
   order relative to the tube: the primary-before-tube debt.
5. **Remaining known gaps**: type1 radius/near dispatch; clipping; wiring the C3ak packer into
   the app. One defect per brief, each with a concrete value or pixel acceptance check.
   Re-score after each landing and re-rank by region error; a later measurement outranks
   this order.

### Reference change (user, overlord review 2026-10-25)

The new capture becomes the C3 reference only if it matches repair4 everywhere outside three
areas: the cursor and hatch-highlight boxes, the fourth walker's box and the counter glyphs.
Use the same metric to check this, and record the check in the round note. When it qualifies:
- score against it, render the candidate with the full captured actor list and counters, and
  report repair4 beside it for continuity;
- the threshold (0.02), metric, denominator, region partition and "floor ring included" rule
  are unchanged.

If it does not qualify, repair4 stays the reference and the new capture is only reported. In
either case, stop and report if any other gate parameter would need to change.

Report C1 `zoom1`/`elev_down` (gated, must keep passing), `elev_up` and the other A7p views
with each full-frame score. Not in scope: C2, B2, B3.

## Budget

- **Run:** about **6 hours or 16 rounds**. A repair counts as a round.
- **Round limit:** 45 minutes from dispatch, repairs included.
- **Rounds:** 3 per question, then park it with a resume brief and rank the next.
- **Concurrency:** up to 3 offline rounds at once, at most one live owner.
- **Stock starts:** a pool of **3**, each dry-checked first, on the host for the controller
  path. Normal `Popen`, menu attach through the REA CLI (`rea instrument-with-frida attach`,
  [MOUV_SSH.md](agents/MOUV_SSH.md)). Python `frida.attach`, `frida.spawn` and suspended
  launches stay retired. Read-only hooks and one session. The only game-memory write allowed
  is the restored level-table slot. Moving the cursor is input, not a write. A crash ends the
  live arm; a second ends starts for the run. No further user authorization is needed.
- **No debugger.** Per-round commits and pushes.

## Decisions already made (do not re-ask)

- **C1 is done** under the third amendment; its gate, footprint and views are frozen.
- **DERIVED** as in `.claude/rules/match-the-original.md`. Label it.
- **Do not choose by score.** Keep read-supported values that worsen the screenshot (C3al,
  C3an, C3ao), as the last run did. The C3ae software raster choices remain declared
  conventions.
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
- Settled: everything in earlier directives, c1ap–c1bj, C3a–C3ao and the C3 one-shot.
  Parked and out of scope: the caption blend, the hatch animation, the `MM_DIG_000`
  shortfall, the HUD tick mismatch, the `+0x3c` writer, C2 cursor, B3 map and level pick.
  Run CTest on mouv. LF only.

## Stop point

Stop and report only when:
- the C3 gate passes on your own independent re-score;
- a gate parameter other than the authorized reference swap would need to change;
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
