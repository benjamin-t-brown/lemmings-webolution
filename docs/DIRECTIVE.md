# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: **active**
Issued: 2026-10-25 (user, overlord review 2026-10-25, "yes i approve 2": close C1 under the
amended gate, then C3 backdrop scenery)
Supersedes: the C1 DERIVED directive, done 2026-10-25 13:05 MDT (run `c1-derived-values`:
11 rounds, 0 starts, stopped on `elev_up`; the gate was amended after the stop).

Client: **OpenCode** (user, 2026-10-25). `/facilitator` on mouv (`openai/gpt-6.1-sol`) or
`/facilitator-mac` on the Mac clone (`llmgw/gpt-6.1-sol`), per [ROLES.md](ROLES.md). The last
OpenCode pass, `ses_e8c346bc4ffe01LaYRaXoqOAkl`, failed at 11:45:02 after c1bg wrote its
packet; check for a running OpenCode or Cursor session before claiming the run.

## Why this

The amended C1 gate (`zoom1`, `elev_down`) passes on C1bi's own scores, 0.01609 and 0.01612.
What remains on `elev_up` is the scene through the plate interior (C1bj), and the same scene is
most of the full frame: C3a puts `rest` at 84.7% of 0.2098, 94.5% of it on pixels we draw
black. The scene reads were parked three times (C3b–C3e, C3g–C3j, C3k–C3m) at body caps with
five bodies unread. This run applies the DERIVED class and value-framed questions to them.

## Scope

Milestones in order: **C1 skill panel** (close only), then **C3 full frame**.

0. **Close C1.** Re-score `zoom1` and `elev_down` yourself on the landed render (`SkillPanel.cpp`
   SHA-256 `3cb8c08e…`) over the frozen footprint. If both are ≤ 0.02, mark C1 done in
   `PLAN.md`; report `elev_up`, `initial`, the other A7p views and `MM_DIG_000` beside them.
   If either fails, stop and report. No panel rounds.
1. **What does the original draw for each scene node in the A6 repair 4 pause frame?** The
   values needed per node: composed matrix, vertex positions and the per-vertex fields, the
   bound texture, the draw kind and whether it is culled or skipped. Unread in the chain (Phase E
   C3 backdrop scenery): the `0x43bd80` bind, `0x418350` (kind-5 skip), `0x46d372` (vertex
   field consumer), the type-2 draw `0x47d0c0`, kinds 2/3/4, the object at `0x58d498`, and the
   pause-frame values of `[game+0]` and `[game+0x5139]`. Read bodies are listed there; do not
   re-read them. These bodies are separable: split them across concurrent offline rounds.
2. **What device state does each scene draw run under?** Blend, Z and filter state set on the
   path to each kind's draw. Device enums may be DERIVED from the SDK headers.
3. **Render and score.** Draw the scene in our renderer with the read values. Score the A6d24
   full frame on the A6 repair 4 pause with `score_c3_regions.py` (per-region table as C3a),
   and re-score C1 `elev_up` and the other A7p views as reported. Bound the floor ring
   (UNBOUNDED in C3a) from read geometry before it is scored.
4. If the gate fails, rank what is left by region error: entrance and exit models (Phase E,
   Exit width gap), the 16259 B block after `0x98d41`, HUD residue. Keep going.

The C3 gate is unchanged: the A6d24 full-frame score ≤ 0.02 on the A6 repair 4 pause with
actors, the floor ring included.

Not in scope: C2, B2, B3. Do not advance to them; stop and report when C3 passes.

## Budget

- **Run:** about **8 hours or 24 rounds**. Round limit 30 minutes from dispatch. No body cap;
  a round follows its chain until it has the value or reaches the limit, and states its count.
- **Rounds:** 3 per question, then park it with a resume brief and rank the next. A further
  round needs a written reason in its brief that it moves the gate.
- **Concurrency:** up to 3 offline rounds at once, at most one live owner.
- **Stock starts:** a pool of **3**, each dry-checked offline first, only for a value the
  offline chain cannot reach (for example `[game+0x5139]` or `0x58d498` in the pause). Normal
  `Popen`, menu attach through the REA CLI (`rea instrument-with-frida attach`,
  [MOUV_SSH.md](agents/MOUV_SSH.md)). Python `frida.attach`, `frida.spawn` and suspended
  launches are retired for game starts. Read-only hooks, one session, no game-memory write
  beyond the restored level-table slot. Apply or reject C3m's unapplied `fix.patch` (menu-scan
  retry and logging) offline before the first start. A crash ends that question's live arm; a
  second crash ends starts for the run.
- **No debugger.** Per-round commits and pushes.

## Decisions already made (do not re-ask)

- **C1 gated views** (user, overlord review 2026-10-25, third amendment): `zoom1` and
  `elev_down` gated, `elev_up` reported until C3 draws the scene. Threshold, footprint
  (`tools/harness/fixtures/c1-footprint/footprint.json`, SHA-256 `e96840e0…`) and region frozen.
- **DERIVED** as in `.claude/rules/match-the-original.md`. Label it; a disagreeing live read
  replaces it.
- **Park by default.** A capped question is parked and the next ranked; a park-or-amend choice
  goes to `pending_questions` with a recommendation. Do not stop for it.
- **Do not choose by score.** A value comes from the read chain, not from fit. c3g's
  identity-bottom Entrance/Exit boxes stay a hypothesis.
- The stack bottom (element 0 of `0x58D5A0`) is the camera block from
  `camera_a1/reference-state.json`, MEASURED-FROM-REFERENCE (C3k). Do not spend a start on it.
- `tools/harness/measure_d2_pick.py:149-156` swaps the `0x40eae0` roles (C3k). Out of scope;
  note it, do not fix it here.
- **Time the pass** (standing), total and per round, in the queue.
- **Landing** per the 2026-10-24 restructure: detail in the scratch note, round prose in
  `docs/plan-history/`, a one- or two-line status cell, landed tasks in the queue archive.
- `levelScreen` is `u32(0x4CBFB0)` per process. Use `acquire_game_foreground`, never
  `goto_level.focus()`. Live steps run in session 1 per [MOUV_SSH.md](agents/MOUV_SSH.md).
  Close any game the round leaves running, after checking it. Restore and hash-check every
  install file in the same process that changed it.
- `frida` 17.24.3 stays installed for mouv's Python (user, 2026-10-24).
- Settled: everything in earlier directives, c1ap–c1bj and C3a–C3m. Parked and out of scope:
  the caption blend, the hatch animation, the `MM_DIG_000` shortfall, the HUD tick mismatch,
  the `+0x3c` writer, C2 cursor (c2l–c2n), B3 map and level pick (b3a–b3g). Run CTest on mouv.
  LF only.

## Stop point

Stop and report only when:
- the C3 gate passes on your own independent re-score;
- C1 fails its re-score in step 0;
- a gate, its footprint, views or exclusions would need to change;
- a guardrail breach: install hashes not restored, an unattributed process, a write left
  unrestored, or a worker exceeding its start budget or using a retired method;
- every remaining C3 question is parked or waits on `pending_questions`;
- the run reaches about 8 hours or 24 rounds;
- delegation or the pinned model is unavailable, or another session is running a task.

A landed render that still fails the gate is not a stop; rank and continue. Running low on
context is not a stop: leave the queue resumable, write `standing_run.handoff`, and say the
user should run `/facilitator` again.

## Sync

Pull before selecting work and push after each round commit, per
[ROLES.md](ROLES.md#opencode-on-the-mac-clone). If a push from mouv fails on credentials, leave
the commits local and say so in the handoff; the overlord carries them to GitHub.
