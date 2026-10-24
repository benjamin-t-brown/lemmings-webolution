# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **done** (2026-10-23 23:36 MDT: Q1 parked at 2 of 2 rounds, Q2 start consumed with no read, Q3 not run; see standing_run.stopped)
Issued: 2026-10-23 (user, overlord review 2026-10-23)
Supersedes: the C3 scenery directive, done 2026-10-23 (both questions parked at their caps,
nothing rendered, no start spent; backdrop is an LVL-embedded model tree drawn at `0x441879`
before the tube; `0x482210` composes node first, stack top second; the type-0 load puts the
file f64[12] at `this+8`; Entrance `Top` projects within 1 px of the reference on three edges,
Exit `Top` about 15 px wide against about 40).

## Why this

C3b–C3f read most of the scene chain but not enough to draw it: the parent/child relation,
the vertex convention and the kind-6 stride (all in the type-1 child load), and the stack
bottom at `0x58D5A0` when the scene is drawn. The first three are offline; the last is live
only. Each is named, so this pass reads exactly those and then renders. The scenery is still
about 0.178 of C3's 0.2098; C1 still follows.

## Scope

Three questions, in order. 1 and 2 may run together; 3 waits for both.

1. **Child load (offline, cap two rounds).** Read the type-1 load (`0x168` object, ctor
   `0x47af20`, vtable `0x4b1660`, its load vcall) and finish the factory loop
   `0x46bb9f..0x46bc1e`. Settle: whether `sand7`, `base`, Entrance `Top` and Exit `Top` are
   children of the root `thenull` (so whether its scale 10 reaches them), whether the u32
   C3d called `a` is the child count, the vertex layout and convention at draw, the kind-6
   record stride, and what `cube13` and `cyl2` are. Re-project with the read chain and report
   whether the Exit width gap closes. Report the Entrance and Exit boxes against the
   reference; do not fit.
2. **Stack bottom (preflight, one start, memory read only).** At the A6 repair 4 pause, read
   the stack count and the first `0x60` bytes of each live element of `0x58D5A0`, plus
   `[0x58ce24]`. The preflight states what the bottom is expected to hold (from the readers
   of `0x58D5A0` offline) and how the read will tell a stable bottom from a stale one. If a
   memory read cannot show it, record that and park; do not escalate to a debugger.
3. **Render (offline, cap two rounds), only if 1 and 2 leave the chain READ.** Draw the
   backdrop tree, the Entrance and the Exit in the production render with the read
   transforms, textures and draw order. Bound the floor ring from the read geometry. Re-score
   C3 full frame on the A6 repair 4 pause with actors (A6d24 frame and scorer), split by the
   C3a regions plus the floor ring, and re-score the amended A7 gate (nine views + second
   level), which must still pass. Report `MM_DIG_000` without the caption exclusion too. Do
   not tune. If any link is still UNREAD, the render does not run and the question parks.

## Budget

Per-round commits, up to three concurrent offline rounds, at most one live owner. **One
stock start, question 2 only**, no retry after Popen. Memory reads only; **no debugger**.
**Function-body cap per round: up to ten** (C3d's cap was eight and C3f's six; both were
exceeded on the path the question asked for). Thunks of five instructions or fewer do not
count. A brief lists the bodies in order; a body off that list counts and is named.

## Decisions already made (do not re-ask)

- Authorized as scoped above (user, overlord review 2026-10-23): close the scene chain, then
  render; one memory-only start; body cap ten.
- **Read, do not fit, and do not choose by score.** A value taken from a scored capture is
  labelled measured-from-reference and named in the report. Sensitivity renders are reported,
  never used to pick a value. A projection that matches the reference is a check, not a read.
- **Time the pass** (standing): total elapsed wall time and each round's dispatch-to-landing
  time, recorded in the queue when each round is dispatched and landed.
- Do not reuse heap addresses from an earlier process (`0x58D5A0` and `0x58ce24` are static).
  Use `acquire_game_foreground`, never `goto_level.focus()`. Live steps run in session 1 per
  [MOUV_SSH.md](agents/MOUV_SSH.md). Close any game the round leaves running, after checking
  it. Restore and hash-check every install file in the same process that changed it.
- Settled and not to be re-derived: everything settled in the previous seven directives,
  plus C3b (LVL-embedded tree, `0x441879` before `0x43a4ec`, root `thenull` values), C3c
  (Entrance and Exit `Object 09` placements and textures), C3d (`0x47496d` push/pop,
  factory `0x46bca4`), C3e (type 0 callee `0x46b6cc`, compose once, child loop at `this+0x68`)
  and C3f (`0x482210` order, f64[12] at `this+8`).
- The hatch's door and count animation, the caption blend, the `MM_DIG_000` shortfall and the
  HUD tick mismatch stay parked. C1 stays held. B2 and B3 stay queued. Run CTest on mouv.
  Keep text files LF.

## Stop point

Stop and report when all three questions are landed or parked, on any stop condition in
ORCHESTRATION, or if the live start crashes or leaves the install unrestored. Do not advance
to C1, the C2 cursor clause, the caption blend, the hatch animation, B2 or B3 on this
directive.

## Sync

Run from the Mac clone `~/progs/lemmings-webolution`. Pull before selecting work and push
after each round commit, per [ROLES.md](ROLES.md#opencode-on-the-mac-clone). If a push from
mouv fails on credentials, leave the commits local and say so in the handoff; the overlord
carries them to GitHub.
