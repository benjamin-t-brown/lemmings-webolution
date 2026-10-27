# Phase C — historical campaign authorizations

Relocated verbatim from PLAN.md, 2026-10-27, to keep the live plan concise under
its unchanged size limit. These are historical decisions, not renewed pools or
current launch authority. The live campaign is governed by `../DIRECTIVE.md`.

- **Milestone-scoped standing runs (user, overlord review 2026-10-24, "good i approve").**
  A run may take a milestone to its gate: the facilitator ranks and splits each next question,
  and an unread input is queued, not a stop. Budget: 3 rounds per question, 3 concurrent
  offline rounds, a pool of 3 stock starts (each dry-checked offline first, no retry after a
  failed launch), about 4 hours or 12 rounds per run. No debugger, no choosing by score.
  First use: the C1 directive of 2026-10-24.
- **C1 alpha-count run: 2 more starts and a launch-time attach (user, overlord review
  2026-10-24, "ya sure, 2 yes").** Two stock starts after c1ah spent the pool, so one failed
  launch can be repaired. A read-only attach may go in at launch, before the frontend loads
  textures (right after `Popen`, or with the game spawned suspended), when an offline round has
  placed the `^tab` upload before the level click. Everything else in the milestone-scoped
  budget is unchanged.
- **C1 alpha counts: attach at launch without placing the upload first (user, overlord review
  2026-10-24, "i approve").** c1ai–c1al could not place the `^tab` upload in time offline. The
  next start attaches at launch and stays attached through level entry and pause, logging
  continuously, with read-only hooks at the format callback, the upload entry, the mask zero
  test and the shift. The 2 unspent starts stand; the alpha-count question gets up to 3 more
  rounds.
- **C1 alpha counts after the c1ao exit (user, overlord review 2026-10-24, "yep all are good
  i approve").** c1ao's launch start exited during startup, after a 16-bit upload kept mask
  `0x8000` and before any shift hit. Up to 3 more alpha-count rounds; the start pool is 2 again
  (1 left plus 1). The `frida` 17.24.3 package c1an installed for mouv's Python stays; the
  facilitator updates `docs/agents/MOUV_SSH.md` to say so.
- **C1 alpha counts: offline first, menu-attach only, then a convergence check (user,
  overlord review 2026-10-25, "yep all are good i approve").** c1aq and c1as, spawned
  suspended, both died at startup (`0xC0000005` on c1as) before `^tab` uploaded. Spawn mode
  is retired. Up to 3 offline rounds and 1 live round; a pool of 1 start, attached at the menu
  before the level click (the c1s/c1ah recipe); a crash is a hard stop. If this run ends
  without a gate score, the overlord brings the user a choice: park the plate-alpha question
  or amend the C1 gate.
- **Long multi-milestone run (user, overlord review 2026-10-25, "approve").** After c1aw's
  crash: neither park nor amend yet; first re-read c1au's `+7` fill coverage (the withdrawal
  looks wrong), then render C1. Scope C1, then C2 cursor, then B3; C3 and B2 reopen only with
  a written gate reason. Caps park and the run continues; park-or-amend goes to
  `pending_questions`. About 8 hours or 24 rounds; a pool of 3 starts, menu attach through
  the REA CLI; a crash ends that question's live arm, a second ends starts for the run.
- **DERIVED values, value-framed questions, C1 only (user, overlord review 2026-10-25,
  "approve all").** After run `c1-c2-b3` parked every question with no gate scored. A value
  may be DERIVED when each link is read from the exe or install or is documented Win32 /
  DirectX behaviour (struct, field, offset cited); labelled DERIVED, never MEASURED, and a
  disagreeing live read replaces it. Questions name the value needed and a round follows the
  chain to it. Next run: C1 only (`+6` and kept format, the `^tab` texel word, render and
  score); same budget as the previous run.
- **C1 gated views amended a third time (user, overlord review 2026-10-25, "i approve your
  recommendations").** After C1bh (0.01617 / 0.03275 / 0.01622): plate 0 carries 420292 of
  `elev_up`'s 580420; behind it stock shows sand (mean 325 at alpha-0 texels) where we draw
  black (27), the parked C3 backdrop. `elev_up` moves to reported, not gated; `zoom1` and
  `elev_down` stay gated at ≤ 0.02. Threshold, footprint and region unchanged. Overlord
  scripts `build/tmp/overlord-2026-10-25/elevup.py`, `plate0.py` (saved PNGs; evaluation).
- **Close C1, then C3 backdrop scenery (user, overlord review 2026-10-25, "yes i approve
  2").** After run `c1-derived-values` stopped on `elev_up` (C1bj: the scene through the plate
  interior). The next run re-scores and closes C1 under the third amendment, then reopens the
  parked C3 scene reads with DERIVED values and value-framed questions, renders and scores the
  full frame. No body cap; 30-minute rounds. C2, B2 and B3 stay out. About 8 hours or 24
  rounds; a pool of 3 starts, menu attach through the REA CLI.
- **C3 steering: render now, spend a start on pause values (user, overlord review
  2026-10-25, "i approve 1 and 2").** After C3n–C3z (16 rounds, 0 starts, no scene drawn,
  full frame 0.2050523 on repair4). The run renders the scene from the read/DERIVED values and
  scores it before adding more readers. One live round from the existing pool of 3 lifts the C3z
  observation park for the pause values (`[game+0]`, `[game+0x5139]`, `0x58d498`, effective
  texture format). Steering was in `docs/DIRECTIVE.md` at `884798a`.
- **C3: fix the known render gaps, then one live start (user, overlord review 2026-10-25,
  "i approve go ahead").** Run `c1-close-c3-scene` stopped on its 24-round budget with C3ae's
  conditional scene render at 0.0923688 (was 0.2050523) and the live controller rejected in
  dry-check (C3ad), 0 starts. Next Sol run: fix C3ae's listed integration defects from existing
  evidence, one per brief with a concrete check, re-scoring after each; repair and fully mock
  the controller, then one start for the pause values. About 6 hours or 16 rounds, pool of 3
  starts. Round limit becomes **45 minutes, repairs included**. Gate unchanged.
