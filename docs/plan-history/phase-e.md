# Plan history — Phase E

Moved verbatim from `PLAN.md` on 2026-10-24 (user, overlord review 2026-10-24). Only link
paths were rebased, and a `###` heading was added above each entry so the plan can link to
it. The live milestone table, exit gates, status cells and user decisions are in [the
plan](../../PLAN.md#phase-e--the-remaining-debts). New round notes are appended under the
milestone they serve, newest last.

## Phase E — the remaining debts

Short, and explicitly parked rather than cancelled. **The gadget backlog is gone** — rounds
13–31 closed every one, including eleven clean negatives where a cue turned out not to be a
gadget at all. See the gadget table in `docs/LESSONS.md` before assuming anything here is
open.

### B1 texture loader and render

- **Generic texture loader / B1 render** (Q1 parked 2026-10-23 at the three-round cap: B1p, B1q,
  B1r). `^Font` converges on `lemmings.box` `TEXTURES\^FONT.BMP` + `.ALF` only if the registrar
  gate `[0x5D5610]` is nonzero (setter unread, manager ctor `0x47DB02`/`0x47DB23`), a loose probe
  misses (`0x47E89B`, `0x49DD81`, `0x47E105`, `0x47EF48`, `0x47EF92`, cwd `0x450600`/`0x4A8EF4`
  unread) and the `0x414B90` -> `0x45019E` link is the path. Q2 (render) not started; never
  choose a sheet by score. See `docs/scratch/phase-b1r-load-order.md`.
  **Live read (B1t, 2026-10-23, one stock start): no hit at `0x47E4AD`; source not observed.** The
  loose route, a late breakpoint, a missed thread and a non-matching condition are not separable. Q2
  still not started. Resume brief: arm `{0x47E4AD, 0x47E463}` together (needs a fresh start grant), or
  read the gate setter and loose helpers offline first. See `docs/scratch/phase-b1t-font-live-read.md`.

### B1 source both arms

- **B1 source, both arms** (b1u landed 2026-10-23): script arms `{0x47E4AD, 0x47E463}`, proves liveness, reads hit counts; hardware breakpoints at the `starti` stop do not reach the main thread (throwaway), so the start used a one-shot entry-point int3 (authorized by the user). See `docs/scratch/phase-b1u-font-both-arms-preflight.md`.
  **Live read (B1v, 2026-10-23, one stock start): `^Font` source observed, exactly one:** `LEMMINGS.BOX` archive, `TEXTURES\^FONT.BMP` (77 bytes) and `.ALF` (61440), zero loose, gate `[0x5D5610]`=1; stopped at hit 2, so a later third source is not excluded. Q2 (render and score) may proceed. See `docs/scratch/phase-b1v-font-live-read.md`.
  **Render and score (B1w, 2026-10-23): full-frame 0.005244 <= 0.02, gate passes on the facilitator's independent re-score** (numerator 1232493, 185447 changed pixels). Centre anchor read at `0x48A747`; glyph table, Y rule, labels read; sheet and X word observed. **The five panel rectangles and colours are measured from the scored reference, not read** (18.3% of the numerator); black text is a partly read default; crosshair, glyph blend and a backdrop dim of up to 7 levels are unresolved. One screen, UK, highlight 0. See `docs/scratch/phase-b1w-menu-render.md`.

### C3 backdrop scenery

- **C3 backdrop scenery** (parked 2026-10-23 at the three-round cap on this question: C3b, C3d, C3e). Read: the non-tube scene is an LVL-embedded model tree (root `thenull` 933 vertices, sand_1/box2/sand_2/matches/wood1/sky/hose slots, matrix diag 10) dispatched at `0x441879` before the tube at `0x441884`; `0x47496d` pushes stack `0x58D5A0`, and `0x46b6cc` composes a node matrix with the stack top once through `0x43bc10` and nests its children. Unread: operand order in `0x482210`, ctor `0x46b590` and the per-type load vcall `+0xc` (where the f64[12] sits, whether `sand7`/`base` are children of the root), the stack bottom at draw, blend/Z state, the floor ring (UNBOUNDED). Nothing drawn. Resume brief: `0x482210` and its callees `0x4128e0`, `0x40eb50`, `0x402680`, then `0x46b590` and the vcall `+0xc` of the type-0 object; if the stack bottom cannot be read offline, one live memory read of `0x58D5A0` during `0x4749a7`. See `docs/scratch/phase-c3b-backdrop-read.md`, `phase-c3d-scene-transform-read.md`, `phase-c3e-scene-callee-read.md`. **Re-parked 2026-10-23 (directive 943cd7e, c3g/c3i/c3h/c3j).** Since read: the type-1 child load, node trees and kind-6 record 116 B (C3g); the type-1 draw skeleton (`0x46c55d`: visibility byte, compose onto the stack top, vertex loop in file order) (C3i); stack element 0 = per-frame push of `[0x58D3D8]` at `0x45DD7B`, element 1 = push at `0x474979`, pop copies nothing (C3h). Still unread: `0x40eb50` row/column convention and axis order, per-kind draws from `0x46a280`, kind 5/7 readers `0x46af80`/`0x46b3d0`, type-2 load `0x47b320`, door/visibility state. The live stack read is NOT done: the one start failed in A7p `enter_level` (`no menu highlight`, `goto_level.py:200` `find_menu_index` found no candidate) before any read ([C3j](../scratch/phase-c3j-stack-bottom-live.md)). Resume: a fresh start grant (pending question `q2-second-start`), preceded by an offline round on the menu-entry failure; then render only once `0x40eb50` and the per-kind draws are READ. **Re-parked 2026-10-24 (directive 2026-10-23 overlord review, c3k/c3l/c3m; last C3 scenery pass).** Now READ: the stack bottom (nothing writes `[0x58D3D8]` in the push-to-scene window; value from the A1 camera, MEASURED-FROM-REFERENCE), the `0x40eb50` convention, kinds 5 (160 B) and 7 (212 B), the type-2 load `0x47b44c`, the kind-1 draw order (record order), and the scenery record parses past `0x967a2` to `0x98d41`. Still UNREAD: the `0x43bd80` bind, `0x418350` (kind-5 skip test), `0x46d372` (vertex field consumer), the type-2 draw `0x47d0c0`, the object at `0x58d498`, kinds 2/3/4, the pause-frame values of `[game+0]` and `[game+0x5139]`, a 16259 B block after `0x98d41`. The live stack read was not needed and the entry failure is not named ([C3m](../scratch/phase-c3m-menu-entry-diagnosis.md); `fix.patch` retry and logging unapplied). Resume: the five unread bodies above, then render. Two of three rounds exceeded the ten-body cap (c3k about 12, c3l about 17).

### C3 entrance and exit models

- **C3 entrance and exit models** (parked 2026-10-23 at the two-round cap on this question: C3c, C3f). Read: `0x482210` composes node first, stack top second (`0x4128e0` R = R_node x R_top row-major, `0x40eb50` t_node.R_top, `0x402680` + top.t); the type-0 load (vtable `0x4b19a0` +0xc, `0x46bef0` -> `0x46bb73`) reads the file's f64[12] to `this+8`, a name, two doubles, a u32 child count n and n factory children. Under "own matrix once" Entrance `Top` projects x 302.3-338.28, y 234.58-260.92 (within 1 px of the reference on three edges), Exit `Top` x 72.2-87.1 (about 15 px against about 40 measured); these are hypotheses. Unread: whether Entrance/Exit `Top` have a parent (the stack top at draw), the stack bottom `0x58D5A0` element 0, the type-1 child load (`0x47af20`, vtable `0x4b1660`) and kind-6 PolyList stride, the vertex convention at draw, the Exit width gap, door animation. Nothing drawn. Resume brief: read the type-1 load (`0x47af20` and its vtable `0x4b1660` +0xc) and the kind-6 reader; one live read of the first `0x60` bytes at `0x58D5A0` during `0x47496d` settles the stack bottom (the one unspent start). See `docs/scratch/phase-c3c-entrance-exit-read.md`, `phase-c3f-entrance-exit-chain.md`. **Re-parked 2026-10-23 (directive 943cd7e):** the type-1 child load is READ (C3g, root scale 10 does not reach Entrance/Exit `Top`, Exit `cyl2` projects wider than the reference) and the draw skeleton is READ (C3i); the Exit width gap does not close; the stack bottom is named from bytes (C3h) but its live read failed before any read (C3j). Resume as in the scenery entry above. **Re-parked 2026-10-24:** see the scenery entry above. The Entrance/Exit trees are READ; the Exit width gap still does not close; no render. **`tools/harness/measure_d2_pick.py:149-156` swaps the matrix and vector roles of `0x40eae0`** (C3k, call at `0x456360..0x456377`): check it when the C2 cursor clause resumes.

### C2 Paused caption text and blend

- **C2 Paused caption text and blend** (parked 2026-10-23 at the two-round cap on this question: C2j, C2k; earlier C2e/C2f/C2h/C2i). The panel `[277,422,363,454)` is
  MEASURED/READ and the "Paused" text is drawn (string READ at `LANGUAGE\UK.LTX` index `0x54`, width 70 READ, white ink MEASURED-FROM-REFERENCE). Fill black and alpha 128 are READ immediates;
  `0x46D460` packs them as a D3D diffuse dword (alpha `0x80`) and makes one device call, so the blend is the device's. With the caption exclusion removed and the ten skill boxes excluded,
  A7 passes on **9 of 10 views**; `MM_DIG_000` fails by 2,660 (2686464 vs 2683804). Unread: the device render state for the quad (alpha-blend enable, blend factors, texture stage), the vertex
  lookup `0x414A40`, 0.00225, the `1` at `0x44DBA0`, the exe's text colour. Resume brief: read the state setup that precedes the `0x46D460` device call (the `0x472860..0x472975` states are not
  tied to this quad), then `0x414A40`; if the state cannot be read offline, one live read of the device state. See `docs/scratch/phase-c2i-caption-render.md`,
  `docs/scratch/phase-c2j-caption-text-blend.md` and `docs/scratch/phase-c2k-caption-blend-read.md`.

### B2 level to map

- **B2 level-to-map** (parked 2026-10-23, directive Q3: B2j and B2k named **no** key or click path, so the
  start was skipped and `Frontend` has nothing new to replay). Read: screen 0x20 handler `0x486510` -> `0x4886E4`
  (selector 0x23, map by inference from B2h's node `0x4B3EB8`) -> factory; producer `0x48FCDD`; screen 0x20 built when
  the mode word `[0x4C6A08]` is 4 (`0x45DF5C`). Unread: the writer of that word, the caller and widget choice of
  `0x48FCDD`, the mouse hit-test, whether the old head node is later removed. The 0x1F dialog never builds the map.
  Resume brief: read the writers of `[0x4C6A08+0]` through the 50-plus `ecx=0x4C6A08` members, then the per-frame
  caller of `0x48FCDD`; a start needs a driven level end. B2 gate row stays unmet. See
  `docs/scratch/phase-b2j-level-to-map-preflight.md` and `docs/scratch/phase-b2k-level-to-map-input-end.md`.

### B2 screen state machine

- **B2 screen state machine** (parked 2026-10-23 at the three-round cap; B2a preflight failed
  closed, B2b read the key-poll flag `[0x5D5970]` and the focus-store shape, B2c read the
  file-backed chain `[0x5D59E4]` -> `+4` -> node with vtable `0x4B2528`). **Zero stock starts
  used; the one granted start was not spent.** Resume brief: (1) read the producer of the event
  number passed to `0x48896A` (key -> event, incl. why RETURN acts only at highlight 0);
  (2) establish the writer of `[menu+0x10]` or the real highlight accessor; (3) confirm, map,
  level entry, pause and back; (4) only then a complete byte-based route preflight and one
  stock start in session 1. See `docs/scratch/phase-b2c-menu-chain.md`.

### C1 live icon inputs

- **C1 live icon inputs (read 2026-10-24, C1l; render not yet run).** The base icon draw is READ (C1g, C1h: panel object `0x4C91D8`, `0x45B639` -> `0x47A75D`); c1l then READ `[levelScreen+0x10]` = 0, the sheet handles (`0x4E335C` = {6,-1,-1,-1}; `+0x1FC` = 6 for the seven generic icons, 1 / 2 / 3 for icons 2 / 8 / 9) and all ten frame records live ([note](../scratch/phase-c1l-icon-live-read.md)). What stays UNREAD: the sheet file behind handle 6 and its `Hi` / `Lo` suffix, the count font file and colour behind handles 37 / 36, the count text behind the objects' `+0xC` pointers, and the hook's on-screen x, y. Resume: offline reads c1m (handle 6 -> file) and c1n (count font) under the 2026-10-24 directive; a hook read needs a worker with Frida (c1o preflight). The menu-entry cause (c3j, c1i) is still unnamed; do not chase it.

### C1 skill-panel icon-draw mechanism

- **C1 skill-panel icon-draw mechanism** (parked 2026-10-22; superseded by C1f's live
  read, same date, still parked). C1b–C1e (4 static-only rounds, including one recorded
  cap exception) located `levelScreen`'s ten identical sub-widgets (`0x452bc2`, stride
  `0x238`), their shared constructor (`FUN_004576b0`) and vtable (`0x4b1774`), and slot+4's
  confirmed icon draw `FUN_00457868` — but could not statically find a writer of any
  widget's `+8` index or a dispatch reaching widgets other than 0. **C1f (one live stock
  start, authorized by `docs/DIRECTIVE.md`) overturned both of those static-era
  conclusions with live counter-evidence**: `+8` *is* written, to `widget_index + 8` for
  all ten; `levelScreen+0x3c` *is* written at runtime (observed moving from widget 0 to
  widget 6 across one unpause/pause cycle, writer still unidentified — candidate: cursor
  hover/hit-test against `panel_boxes` at `0x4C9208`, unverified); and `FUN_00457868`
  fires once per frame for exactly the one widget `+0x3c` currently names (461 Frida hits,
  `distinct_ecx_count=1`), while a screenshot taken before any widget-0-only dispatch
  already shows ~9 distinct panel icons — strong evidence `FUN_00457868` is a
  single-target draw (selection highlight?), **not** the base per-button icon renderer
  this question was chasing. `GreyIcons`/`BlackIcons`/`AdditiveIcons`/`Bits`/`LemmingsLO`
  remain independently ruled out as the icon sheet or draw path (C1b/C1c). **Still open,
  genuinely unresolved**: the `+0x3c` writer, and — now the real open question — where the
  other icons' base artwork actually comes from, if not `FUN_00457868`. Resume needs
  either a correlated live read (cursor position + hit-test + `+0x3c` at the same
  timestamps) or a new static search specifically for the real base-icon draw call, not a
  repeat of C1e's two already-exhausted stride-`0x238`/`+8`-literal scans (those answered
  "does `FUN_00457868` dispatch to all ten", not "what else draws the other nine").
  Evidence: [C1b](../scratch/phase-c1-panel-icon-render.md#result),
  [C1c](../scratch/phase-c1-panel-icon-render.md#c1c-both-pre-registered-leads-also-falsified-one-concrete-unresolved-thread-found-round-2-of-3),
  [C1d](../scratch/phase-c1-panel-icon-render.md#c1d-the-ten-widget-array-and-its-real-icon-draw-are-located-the-full-dispatch-to-all-ten-and-the-per-widget-icon-index-remain-open-round-3-of-3),
  [C1e](../scratch/phase-c1-panel-icon-render.md#c1e-both-cap-exception-sub-questions-are-clean-negatives--question-parked-round-4),
  [C1f](../scratch/phase-c1-panel-icon-render.md#c1f-live-read-falsifies-two-static-era-assumptions-fun_00457868-is-not-the-per-button-renderer-round-1-of-a-new-live-read-question-one-stock-start).
  C1's render gate also depends on the B1 digit-font question for the counts half of the
  same clause (see the B1f/B1h/B1i entries below) — both halves stay parked, so C1 cannot
  progress further without new authorization on either, per the directive's stop point.

### B1f glyph matching

- **B1f fixed-cell glyph-matching method** (parked after3 dispatches,2026-10-22).
  Unsupported16x16 ASCII32 source packing, independent target-component reuse and
  double resizing make the glyph/scale inference invalid. Diagnostic frame omits text;
  its passing scalar is not B1 acceptance. [Evidence/limits](../scratch/phase-b1-template-match-attempts.md).
  Do not repeat this method or a menu-vtable trace. Resume only with a valid source-
  packing/geometry-preserving matcher; B1h's fourth-dispatch exception brief explicitly
  scopes that test in build/tmp/b1h-atlas-independent-match-brief.md. No automatic fifth.
  **Superseded by B1i's reading-based approach** (above) — a future resume should render
  text using B1i's measured charmap/advance rule, not revive this fitting method.

### B1h Quit registration

- **B1h atlas-independent Quit registration** (returned incomplete/parked2026-10-22,
  user stop after B1h). Two dispatches including explicit quota continuation, same
  glyph question total5. Mac continuation over budget~65/35min, interrupted, no final
  worker report. Actual pixel attempts preserved but arbitrary/incoherent letters are
  not glyph evidence; [independent check](../scratch/phase-b1h-interrupted-quit-registration.md).
  Resume only under new explicit authorization after reviewing source identity/target
  boundaries and a semantic glyph discriminator, not scalar-only optimizer fits.
  No continuation/repair is currently authorized or queued. **B1i (above) answers the
  glyph rule from bytes. B1j found that none of `FUN_004404fa`'s direct callers draws
  the five menu labels. B1k found the labels themselves: the dwords at `0x4C4214`,
  walked by `FUN_0048aa5e`.** Do not revive glyph fitting, and do not wire those 17
  direct calls as the menu. B1l found no file store of the horizontal word
  `[0x58CB40]` (63 immediates, all reads or address-takes) and no `^FONT.BMP`
  string. A resume is a live read of that word on the English menu, plus the
  resource manager behind `0x58CB48`, not another scan of the same immediate
  and not a fitted X.
  **B1m runtime resume (2026-10-23):** X640→320 was observed without a valid
  highlight memory bracket; manager capture failed. Its offline repair identifies
  missing `[0x58CB84]`, selected record/wrapper/descriptor and source storage.
  [Evidence and resume boundary](../scratch/phase-b1m-runtime-font-read.md).
  One-start grant exhausted; no automatic retry. A new authorization should require
  offline pointer/name/highlight preflight before spending another start.
  **B1n/B1o new directive outcome (2026-10-23):** round3 table test did not
  identify a source (five candidates pass its bounded predicates; full inventory
  and table extents unproved). Explicit round4 fallback reached eight bodies
  without proving highlight or actual-source path, so did not launch. B1 is
  re-parked with zero starts this run, no renderer/B1 score. Resume only on new
  authorization at [the exact native boundaries](../scratch/phase-b1o-font-binding-preflight.md#park-and-resume-boundary);
  no fifth dispatch or automatic carryover of its unused conditional start.

### D2b digger bite timing

- **D2(b) digger bite-timing** (parked 2026-10-22 after D2i repair 1). Climber, blocker
  and floater are all full 50/50 matches on D2 part (b)'s live-vs-sim comparison;
  digger matches only 16/50. The native stepper (`0x0041B077`, shared byte-identically
  by bash/dig/mine) is a continuous, real-valued percent-of-cycle clock
  (`lemming+0x128`'s `0x00468B04`/`0x00468B5D`) that fires at most one scheduled
  record per call and hands the unspent time budget back to a generic per-lemming
  dispatch loop (`0x0042E2BF`), which re-invokes the same stepper with that leftover
  until exhausted or the state changes -- the real, same-tick, cross-call mechanism
  behind multiple records firing in one tick. The clock does not reset at state entry
  (no caller found from digger's enter hook to the one zeroing function located in a
  full `.text` scan), so a newly-assigned digger inherits whatever phase the prior
  state's animation was at, not a fresh 0%. Our own `World::stepDigger` is a single,
  self-contained, discrete-frame call with no cross-call leftover-budget handoff and
  no persisted animation phase -- an architectural mismatch, not a tunable constant or
  a one-line reorder like climber's/faller's fixes. Closing it needs a persistent,
  cross-state, real-valued percent clock on `Lemming` plus a generic leftover-budget
  re-invocation dispatch touching every stepper's call site, not just digger's -- a
  materially larger change than one bounded round. Resume only with that
  architectural change explicitly scoped and budgeted as its own multi-round effort,
  not another "disassembly plus one-line fix" attempt; the `+1`-at-the-call-site
  hypothesis is already disproven, not just untried. Evidence:
  [D2i round 1](../scratch/phase-d2i-skill-assign-digger.md),
  [D2i repair 1](../scratch/phase-d2i-skill-assign-digger.md#round-2-repair-1-the-1-hypothesis-corrected-not-confirmed--no-fix-applied).

### D3 whole milestone

- **D3 one retail level end to end — whole milestone parked** (parked 2026-10-22 after
  D3f, user decision, CEO review). Saved count matches the original exactly (10/10); the
  end time does not. D3f's own live run directly measured that two live runs of the
  identical scripted solution end 75 real seconds apart (208 s vs 133 s) — the gate's own
  premise (a reproducible end time to match) does not hold for the original itself. The
  self-consistent comparison this round could actually run (a run's own recorded assignment
  times fed back through `World`, compared to that same run's own result) matches to the
  whole second on the quantity `World::finished()` computes. Against the gate's literal
  field, the residual (-8 s) splits into a named, twice-measured delay `World` does not
  model (-6.3 s) and a small remainder inside D1's own already-parked budget (-1.7 s /
  -0.53 s). Per the user's decision (no gate amendment, no further click-model round), no
  further D3 round is authorized. Evidence:
  [D3f](../scratch/phase-d3-timed-solution.md),
  [D3e](../scratch/phase-d3-sim-solution-run.md),
  [D3d](../scratch/phase-d3-solution-discovery.md).

### D1 whole milestone

- **D1 release and walk-out — whole milestone parked** (parked 2026-10-22 after D1o, user
  decision, CEO review). D1's gate does not pass, measured literally on a fresh live
  capture with the continuous-stepper-clock fix (D1n) both off and on: 657 and 305 total
  divergences respectively. The fix is confirmed, on new data, to meaningfully help and not
  be an artifact of the three saved captures it was built against, but it does not close the
  gate. Three named residual families remain, each already independently characterized by
  its own evidence chain, none unexplained: the single-tick `facing`-label residual (below),
  spawn-tick drift (a `World::spawn()` structural limit — the tick is fixed before any
  continuous budget exists), and landing-tick drift (reduced, not closed, and
  capture-dependent in size). Per the user's decision this supersedes the narrower,
  facing-only framing below as the operative parking record for the whole milestone; no
  further D1 round is authorized. Evidence:
  [D1o](../scratch/phase-d1-continuous-live-recheck.md),
  [D1n](../scratch/phase-d1-continuous-stepper-clock.md),
  [D1m](../scratch/phase-d1-stepper-clock.md).

### D1 facing-label residual

- **D1 single-tick `facing`-label residual** (parked 2026-10-21 after D1h). Both named D1
  blockers (release timing, D1a-e; the left-turn tick, `0x0041C67E`, D1f — the line
  formerly here listing it as open is retired) are fixed and independently verified. One
  smaller residual remains: a self-healing, single-tick `facing`-only mismatch at turn
  instants, found on a fresh live capture (D1g) and fully characterized by D1h — every
  instance traces to the live-capture harness's one pooled global phase differing from
  that lemming's own measured phase (D1a's own "Open limitation"), not to a defect in
  `World`; `x` is exact on every tick before and after, and an independent full-trace
  scan found zero reappearance of the stall D1f fixed. Closing it fully would need
  sub-20 ms live sampling or a per-lemming-aware harness (a tooling investment, not a sim
  fix) and needs its own gate justification before a future round is spent on it. D1's
  gate, as literally written ("every lemming's cell X/Y, facing and state match... each
  tick"), does not pass with this residual present; whether to amend the gate to exclude
  a self-healing, x-unaffected, harness-precision-bounded label mismatch, or to invest in
  harness precision to close it outright, is a user decision (`standing_run.pending_questions`),
  not a gate change made unilaterally by the facilitator. Evidence:
  [D1a](../scratch/phase-d1-release-walkout-trace.md),
  [D1b](../scratch/phase-d1-release-trigger.md),
  [D1c](../scratch/phase-d1-door-open-duration.md),
  [D1d](../scratch/phase-d1-hatch-step-clock.md),
  [D1e](../scratch/phase-d1-release-timing-implementation.md),
  [D1f](../scratch/phase-d1-left-turn-fix.md),
  [D1g/D1h](../scratch/phase-d1-full-gate-recheck.md).

### Other debts

- 14 of the 19 call sites of the object-overlap predicate `0x00430BA1` are still
  unidentified. Anti-grav, exit, teleport, SpeedPad and lever account for five.
- C1 panel composition/counts remain; the old panel-index debt is paid by
  [C1a](../scratch/phase-c1-panel-indices.md), including command slots8/9.
- Platform targets: Linux, macOS, WebAssembly. The old GLES3/WebGL2 claim was a code
  property that was never actually run.
- Intro, music and FMV. The `.avi` codecs are still unidentified.

### A7 lower-band cause

- **A7 lower-band cause** (open after A7l: the scale integer is a runtime tick).
  B=`[0,432,640,448)` has stable E302804 over maskN3491. Removing every B
  error would reach 0.0198391255 on the one scored view; eight other views
  and a second level still have no mask. `[G+0x25C64]` is either
  `0x4657D1([G+0x557C]) - [G+0x25C60]` or a conditional `100`. `0x4657D1`
  calls `[0x4B0220]` and subtracts `[ecx+8]`. A paused read of that dword
  can place the rectangle; it cannot finish A7 while the other masks are
  missing. Do not repeat the writer scan. Evidence:
  [A7a](../scratch/phase-a7-wider-mask-localize.md),
  [A7b](../scratch/phase-a7-lower-band-characterize.md),
  [A7c](../scratch/phase-a7-pause-caption-native.md),
  [A7d](../scratch/phase-a7-draw-loop-caption-bridge.md),
  [A7e](../scratch/phase-a7-base-list-producer.md),
  [A7f](../scratch/phase-a7-caller-dest-offset.md),
  [A7g](../scratch/phase-a7-25c68-to-h.md),
  [A7h](../scratch/phase-a7-pause-list-draw.md),
  [A7i](../scratch/phase-a7-rect-26000.md),
  [A7j](../scratch/phase-a7-25ff8-stores.md),
  [A7k](../scratch/phase-a7-1c968-source.md),
  [A7l](../scratch/phase-a7-25c64-writers.md).

### Actor filter and alpha at zoom 1

- **Actor filter/alpha state at zoom 1** (parked 2026-10-20 after seven rounds, A4z–A4af).
  The full-scale zoom 1 fixture fails (27 candidate-only pixels, support MAE 0.0284529
  against 0.0151558); A4af READ that nonzero zoom skips the pre-draw `0x470F29(0)` and
  draws under the last `0x470F29(1)` (LINEAR, `0xA0`). Resume with the written offline
  replay brief `build/tmp/a4ag-zoom1-linear-a0-offline-replay-brief.md`, only if the A6
  gate fails at zoom 1. Evidence: [A4z](../scratch/phase-a4-actor-third-fixture.md),
  [A4aa](../scratch/phase-a4-zoom1-filter-provenance.md),
  [A4ae](../scratch/phase-a4-pre-actor-subcalls-static.md),
  [A4af](../scratch/phase-a4-tube-child-renderers-filter-static.md).

### A6d3 slope walker

- **A6d3 slope walker clause 2** (parked 2026-10-20 when A6 was accepted on the repair 4 pause).
  Actor 2 at `+0x08` `(32, 34.4796, 0.5)` scores 0.10089786756453423 tube-only versus
  0.11246451442529874 actors-on. View, temporal and `+0x1D8` (A6d4–A6d6), pose, screen Y
  (A6d7, A6d20–A6d23) and first row (A6d17–A6d19) are all parked. `originY − 1` beats the
  box but is a pixel fit with no address; do not ship it. Resume only with a new native read
  of how a slope (non-integer Y) walker is drawn, or when a slope walker is needed by a later
  gate. Evidence: [A6d3](../scratch/phase-a6-matched-state-bbox.md),
  [A6e](../scratch/phase-a6-restated-gate-rescore.md),
  [repair 4](../scratch/phase-a6-fresh-pause-pixel-result.md).

### A6 sim to +0x08 mapping

- **A6 sim → `+0x08` mapping** (parked 2026-10-20 after A6d10–A6d12).
  Motion stores, hatch spawn `(+0x9A,+0x9C,0.5)`, and fall Y
  `0x41627F` = `0x40DF50([+0x10] − [+0x44]×step)` are READ. No complete
  sim mapper. Resume with first-tick arm order, walk remainder, or Z 0.75
  init — not another hatch `+0x9A` pass. Evidence:
  [A6d10](../scratch/phase-a6-sim-plus8-writer.md),
  [A6d11](../scratch/phase-a6-plus8-spawn-init.md),
  [A6d12](../scratch/phase-a6-plus8-first-y.md).

### A6 screen-Y bias

- **A6 screen-Y bias** (parked 2026-10-20 after A6d20–A6d22).
  Incoming `0x47A75D` Y is `cy − C0/C8` with no ±1. Render `+0x90`
  is the 24-byte `0x43D56C` result. `0x4C6B88` is a 96-byte copy of
  the `0x58D5A0` stack top (`0x46BE99`), not a second proven miss
  in `worldToLensedView`. `originY − 1` still beats actor 2 and is
  not a native store. A6d23 completed the bounded live-matrix comparison:
  compiled production versus cached screen Y is within 1e-4 px on three
  actors at a fresh canonical-camera pause; no screenshot or A6d3 attribution.
  The native 0.0003 subtraction stores back to render +0xC8 through an alias.
  Re-park; resume only with new paired draw/pixel evidence exposing a projection
  discrepancy, not another static writer search or fitted offset. Evidence:
  [A6d20](../scratch/phase-a6-screen-y-bias.md),
  [A6d21](../scratch/phase-a6-plus90-vector.md),
  [A6d22](../scratch/phase-a6-xform-4c6b88.md),
  [A6d23](../scratch/phase-a6-native-camera-pair.md).

### A6 render-clock dt

- **A6 render-clock dt / `[+0xC8]`** (parked 2026-10-20 after A6d13–A6d15).
  `[game+0x26028]` is a wrap phase. `0x42F0DC` is `0x43CC68`, which
  consumes `[this+0xC8]` as `1.0 / C8` UV scale after `0x40EAE0`
  writes the `+0xB8/+0xC0/+0xC8` triple. That is not the A6b clock
  (`vtable+0x2C` → `0x43CF30` → `0x468833`). Keep the integer-ms
  stand-in. Incoming seconds to `0x42E2BF` stay unread; resume only
  if a temporal change is shown to move the A6 gate. Evidence:
  [A6d13](../scratch/phase-a6-render-clock-dt.md),
  [A6d14](../scratch/phase-a6-walk-budget-ptr.md),
  [A6d15](../scratch/phase-a6-render-c8-dt.md),
  [A6b](../scratch/phase-a6-actor-anim-clock.md).

### A6 first-row floor vs ceil

- **A6 first-row `floor` vs `ceil`** (parked 2026-10-20 after A6d18–A6d19).
  No `fistp` on submit. `floor(top)` is pixel-identical to `ceil` on the
  A6d3 matched frame and fails the A4v y=2 fixture. Initial-view boxes
  move and still fail. Resume only with a float screen-Y term, not
  another cover bound. Evidence:
  [A6d18](../scratch/phase-a6-billboard-top-row.md),
  [A6d19](../scratch/phase-a6-first-row-floor.md).

### A6 per-pixel actor Z

- **A6 per-pixel actor Z versus tube** (parked 2026-10-20 after A6d16).
  Submit path has no actor-vs-tube fragment compare. Closest reject is
  A4y `ALPHAREF 0xEF`. Wrapper depth stays A1 (not an exe read).
  Resume only if wrapper depth is shown to move the A6 gate. Evidence:
  [A6d16](../scratch/phase-a6-actor-tube-pixel-z.md),
  [A6c](../scratch/phase-a6-actor-tube-composition.md),
  [A1 depth](../scratch/phase-a1-depth-clipping.md).

### A6 actor-2 sprite

- **A6 actor-2 sprite at matched pause** (parked 2026-10-20 after A6d4–A6d9).
  Temporal 0–11 cannot pass; live `+0x1D8` is 0.0; native view/mirror match
  `selectSpriteView`. −0.125 is a discarded pixel fit. Pose pair matches
  live `+0x108`. Before-tube is fully occluded. Overlay `> 0xEF` is the A4y
  READ and is pixel-identical to `a != 0` on this 0/255 walk path. Resume
  only with a new lead (not chooser, Z-order, overlay, or blend). Evidence:
  [A6d4](../scratch/phase-a6-actor2-temporal-sweep.md),
  [A6d5](../scratch/phase-a6-live-phase-1d8.md),
  [A6d6](../scratch/phase-a6-live-view-index.md),
  [A6d7](../scratch/phase-a6-live-pose-pair.md),
  [A6d8](../scratch/phase-a6-actor2-z-order.md),
  [A6d9](../scratch/phase-a6-overlay-alpha-ef.md).

### Actor basis precision

- **Actor basis precision** (parked 2026-10-20 after three A6a rounds). The facing basis
  misses its 1e-6 falsifier by up to 2.962e-5. Native values are float32 and the chain
  `0x43D56C`/`0x401830`/`0x480AC8` is not replayed. It is invisible unless a view index sits
  on a sector boundary. Resume only if the A6 gate shows a view-index mismatch. Evidence:
  [A6a](../scratch/phase-a6-actor-pose.md).

### C1 upload call

- **C1 upload call** (parked 2026-10-24 at the alpha-count cap, c1ai, c1ak, c1al). `^tab` registers from the PE entry. The upload `0x472F99` is reached only from `0x4743DD`, called by `0x471746` (`call [edx+0x14]`) when the receiver's vtable is `0x4B19E8`. That function has no direct caller. The constructor `0x474380` leaves that vtable and runs at process startup: entry `0x49D5AB` walks `[0x4BF000, 0x4BF0DC)`, the dword at `0x4BF0B0` is `0x468D20`, and `0x46F16F` calls `0x474380`. That chain does not call `0x471720`. The shift counts are unread. Stock starts this run: 0 of 2, because a start required the upload to be placed in time. Resume: one launch-time read-only attach at `0x471746`, or one offline round on other callers of slot `+0x0C`. See [C1al](../scratch/phase-c1al-ctor-caller.md).

### C1 launch-time alpha read

- **C1 launch-time alpha read** (parked 2026-10-24 at the cap, c1am–c1ao; reopened the same day by the hook-repair directive). The four hooks append on entry, and `python` imports `frida` 17.24.3. Pid 20612 exited within five seconds of `frida.resume`, before `MM_CLIMB_002`. The startup log has eight mask values of 0 and one of 32768. `0x473897` logged nothing. c1ap found no branch target inside that patch and moved the count read to `0x47383C`. c1aq pid 25964 exited before an exit code was read. c1ar moved that read into the detach callback. c1as pid 44772 exited with `0xC0000005` before the level. `skip_input` was 1 eight times and 2 once, and each value AND `0x18` is 0, so `0x47383C` was skipped. The start pool is spent. The fault address is unread. See [C1as](../scratch/phase-c1as-launch-read.md).

### Frida hook tooling

- **Frida hook tooling for live render-state traces** (parked with the above). A4ab's live
  trace crashed the stock game in D3DImm; A4ac/A4ad repaired the binder and stack layout
  offline only, never re-run live. Evidence: [A4ab](../scratch/phase-a4-zoom1-filter-trace-failed.md),
  [A4ac](../scratch/phase-a4-actor-binder-offline.md),
  [A4ad](../scratch/phase-a4-hook-stack-announce-offline.md).

### C1 plate alpha format

- **C1 plate alpha format** (parked 2026-10-25 at the three-round cap, c1ay, c1az, c1ba). `+7` is 0 on every path that reaches `0x473146`. `+6` compares the word `GetObjectA` writes at buffer `+0x12`, and that number is not in the image. `0x47255B` stores the `CreateDIBSection` return at `[object+4]`, which the upload later pushes. The bit-count word is at `0x58D5D4`, past the `.data` raw end `0x4C6000`. `0x45015A` does not write offset `0x1C`: it compares the buffer's first two bytes with `0x42` `0x4D`, keeps the pointer from `0x49C1A8` on a match, and on a mismatch calls `0x49D010` at `0x4503B3` and `0x467DE8` at `0x4503FD`. `0x4C2500` is `Textures\%s.bmp`. The name pushed at `0x4722D4` was not read. `0x472FA8` pushes the object before `DirectDrawEnumerateA`, so the creation handle is not shown to survive until `GetObjectA`. The keep rule was not applied. Resume: open `0x43B5B0` and `0x49C1A8` far enough to say whether offset `0x1C` of a `42 4D` buffer is a file `biBitCount`, and only then apply the c1aj rule. Do not pick a format from the 0.47 brightness ratio. See [C1ba](../scratch/phase-c1ba-bitcount-source.md).

### C2 cursor sprite

- **C2 cursor sprite** (parked 2026-10-25 at the three-round cap, c2l, c2m, c2n). `_curExit` is at `0x4C0DD0`. Its only reference is `push 0x4C0DD0` at `0x45E89B`, then `call 0x45FE10`. `0x45E8AD` stores the return at `[0x4C6A08+0x1C934]` = `0x4E333C`, past the `.data` file end. Two loads read that displacement: `0x45ECBF` pushes the handle and calls `0x414B90`, and `0x45EE3A` does the same into `0x414D20` when the byte at `[object+0x1C9B1]` is zero (`0x45ED95` jumps to `ret` otherwise). The direct caller of the second function is `0x450A74`, which also sets `ecx` to `0x4C6A08`. Neither callee was opened. Hover, no-hover, and the exit sprite are unread. Resume: open `0x414B90` far enough to say what it does with the handle, then `0x414D20` only if the first call does not select a sprite. Do not draw a cursor from the name. See [C2n](../scratch/phase-c2n-cursor-read.md).
