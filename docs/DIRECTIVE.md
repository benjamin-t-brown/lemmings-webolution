# Overlord directive

Written by the [overlord](OVERLORD.md); the user signs off what it authorizes. `/facilitator`
reads this file at startup, in any client. A directive with `Status: active` is the user's
authorization for the next standing run. Record it in `docs/tasks.json` yourself (new
`standing_run`, owner and date), then follow
[the standing run](ORCHESTRATION.md#standing-run). When its stop point is reached, set
`Status: done` here and stop.

Status: **done** (2026-10-24: Q1 landed c3k, c3l; Q2 landed c3m, cause not named; Q3 not triggered; Q4 not run, C3 scenery parked in Phase E)
Issued: 2026-10-23 (user, overlord review 2026-10-23)
Supersedes: the C3 scene chain directive, done 2026-10-23 (C3g type-1 load and node trees
read, kind-6 record 116 B, Entrance/Exit `Top` are their own roots, Exit `cyl2` projects wider
than the reference; C3h stack element 0 is the per-frame push of `[0x58D3D8]` at `0x45DD7B`;
C3i type-1 draw skeleton `0x46c55d` read; C3j's one start failed in A7p `enter_level` with
`no menu highlight` before any read; nothing rendered).

## Why this

C3 scenery has taken nine rounds with nothing drawn. Two of the links C3j was to settle may
already be settled: element 0 of `0x58D5A0` is a push of `[0x58D3D8]`, the A1 camera
composition, which A1 and A4f captured live (`build/tmp/a1-camera/`,
`build/tmp/a4f-actor-billboard/live-*`, `tools/harness/fixtures/camera_a1/`) and which our
camera already reproduces; and `0x40eb50`'s first terms were read in C3f and are used by
`tools/harness/measure_d2_pick.py`. What still blocks the render is offline: the per-kind
draws, the kind 5/7 readers and the type-2 load. This pass closes those, spends a start only
if the offline route cannot settle the stack bottom, and is the last C3 scenery pass before
C1.

## Scope

Four questions. 1 and 2 are offline and may run together; 3 waits for both; 4 waits for 1
(and 3 if it ran).

1. **Close the chain offline (cap two rounds).**
   - **Stack bottom:** read whether anything writes `[0x58D3D8]` between the push at
     `0x45DD7B` and the scene call at `0x441879` (and whether the scene call runs in the
     paused frame). If nothing does, element 0 at draw is `[0x58D3D8]`, and the existing live
     captures and our A1 camera give its value; state which capture and how it maps to the
     A6 repair 4 pause.
   - **`0x40eb50` convention:** reconcile C3f's reading (`mat[0]*vec[0] + mat[3]*vec[1] +
     mat[6]*vec[2]`), C3i's use at `0x40eae0`, `measure_d2_pick.py` and LESSONS A6d15. Read
     only what is missing to make it READ.
   - **Remaining reads:** the per-kind draws from `0x46a280` (with the `0x43bd80` texture
     bind), the kind 5 and 7 readers `0x46af80` / `0x46b3d0`, and the type-2 load `0x47b320`,
     so the scenery parse goes past `0x967a2`. Also `[this+0x100]` and the object at
     `0x58d498` used in the cull.
2. **Menu entry failure (offline, cap one round).** Find why `find_menu_index`
   (`tools/harness/goto_level.py:200`) returned `None` at screen id 3 / ui_state 3 in C3j
   when the same A7p entry worked in A7p and C2h. Name the cause from the run's log, the
   script diff against A7p and the screen state; do not run the game. If the cause is not
   named, say so.
3. **Stack bottom live read (one start, memory only), only if** question 1 cannot settle the
   stack bottom offline **and** question 2 named the entry failure and fixed it. Run C3h's
   reader unchanged with that fix. Otherwise it is not spent.
4. **Render (offline, cap two rounds), only if every link is READ** (stack bottom from 1 or 3,
   the projection convention, the per-kind draws, the strides and the type-2 load). Draw the
   backdrop tree, the Entrance and the Exit with the read transforms, textures and order, and
   bound the floor ring from the read geometry. Re-score C3 full frame on the A6 repair 4
   pause with actors (A6d24 frame and scorer), by the C3a regions plus the floor ring, and the
   amended A7 gate (nine views + second level), which must still pass. Report `MM_DIG_000`
   without the caption exclusion. Do not tune. If a link is still UNREAD, park.

## Budget

Per-round commits, up to three concurrent offline rounds, at most one live owner. **One
stock start, question 3 only, on its two conditions**, no retry after Popen. Memory reads
only; **no debugger**. **Function-body cap ten per round, hard:** a round that needs an
eleventh body stops, lands what it read and names the next body; it does not continue past
the cap. Thunks of five instructions or fewer do not count.

## Decisions already made (do not re-ask)

- Authorized as scoped above (user, overlord review 2026-10-23). This answers
  `q2-second-start`: neither the facilitator's option 1 nor 2 as written; an offline
  settlement first, a start only on question 3's conditions. Clear it from
  `pending_questions`.
- **Last C3 scenery pass:** if question 4 does not render, C3 scenery parks in Phase E and the
  next directive goes to C1.
- **Read, do not fit, and do not choose by score.** A value taken from a scored capture is
  labelled measured-from-reference and named in the report. A projection that matches the
  reference is a check, not a read. An earlier live capture is evidence only for the state it
  captured; say which.
- **Time the pass** (standing): total elapsed wall time and each round's dispatch-to-landing
  time, recorded in the queue when each round is dispatched and landed.
- Do not reuse heap addresses from an earlier process. Use `acquire_game_foreground`, never
  `goto_level.focus()`. Live steps run in session 1 per [MOUV_SSH.md](agents/MOUV_SSH.md).
  Close any game the round leaves running, after checking it. Restore and hash-check every
  install file in the same process that changed it.
- C3h's correction of `docs/scratch/phase-a4-pre-actor-subcalls-static.md:78` (the pop copies
  nothing) may be applied there with a pointer to C3h.
- Settled and not to be re-derived: everything settled in the previous eight directives,
  plus C3g (factory jump table, type-1 load layout, node trees, kind-6 116 B, Entrance/Exit
  roots), C3h (stack layout, pushes at `0x45DD7B` / `0x474979`, pop copies nothing,
  `[0x58CE24]` is a this-pointer) and C3i (draw skeleton `0x46c55d`).
- The hatch's door and count animation, the caption blend, the `MM_DIG_000` shortfall and the
  HUD tick mismatch stay parked. C1 stays held. B2 and B3 stay queued. Run CTest on mouv.
  Keep text files LF.

## Stop point

Stop and report when every question is landed, parked or not triggered, on any stop
condition in ORCHESTRATION, or if a live start crashes or leaves the install unrestored. Do
not advance to C1, the C2 cursor clause, the caption blend, the hatch animation, B2 or B3 on
this directive.

## Sync

Run from the Mac clone `~/progs/lemmings-webolution`. Pull before selecting work and push
after each round commit, per [ROLES.md](ROLES.md#opencode-on-the-mac-clone). If a push from
mouv fails on credentials, leave the commits local and say so in the handoff; the overlord
carries them to GitHub.
