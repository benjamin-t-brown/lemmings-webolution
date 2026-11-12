# lever-timing-research-001: the time contract for the MM_W_012 lever and linked fluid gates

Status: **static trace complete; the time input, dispatch order, per-update arithmetic and reset behaviour are instruction-backed; one bounded implementation proposal with a stated, limited fidelity claim; the remaining dependency is the parked base-clock cadence, named precisely in section 7.**
Static analysis only. The original was never run; no oracle bytes, debugger or capture. No production file, test, queue, verifier, permission or save was changed.
Reference: `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f` (returned by `open_binary`, Ghidra 12.1.4, x86 PE, image base 0x400000). The REA session was closed with `close_binary` without snapshot arguments.
Retained evidence: `build/lever-timing-research-001/static-trace.md` (sections M-R: instructions, addresses, `evidence_id`s). Builds on, and does not repeat, `docs/scratch/lever-links-research-001.md` (links, receiver class, rate table lead) and `docs/scratch/lever-init-research-001.md` (load order, exact cell traversal, 3072/384/528 counts).
Labels: ASM = instructions read; DECOMP = Ghidra pseudocode (ASM-checked where marked); BYTES = `read_bytes`; XREF = static references; INFER = inference; HYPOTHETICAL = an input chosen for illustration, not an observation.

## 1. Answers

1. **Unit of `dt` into `0x0042B24D`: seconds, as a 64-bit double.** It is `integer milliseconds / 1000.0`, with the integer taken from `timeGetTime` (WINMM.DLL) deltas between two iterations of the frame loop `0x0045CFF9`, capped at **100 ms**. This replaces the earlier "unit of dt is parked" statement; what stays parked is the clone's tick cadence (section 6).
2. **Cadence:** once per world update `0x004423D2`, i.e. once per frame (or per timeline-event segment of a frame), not on a fixed 200 ms grid. The receiver step is `vt+0x14` of every object in world list `+0x1F70`, called only while `[obj+0x38] != 0`, with dt forwarded unchanged.
3. **Order in one update:** lemmings (`+0x1EC8`) -> levers (`+0x1F50`: cooldown, contact, activate, message dispatch) -> `+0x1F60` -> receivers (`+0x1F70`). A type-4 message from a lever therefore reaches the receiver in the same update and its first raise step uses that update's full dt.
4. **Pause / fast-forward:** paused (or game state != 2) -> the update still runs but with **dt = 0.0** (no ramp progress). Fast-forward -> **7 updates per frame, each with the same full dt** (the ramp advances 7x faster per real second). The 100 ms clamp applies before either.
5. **Ramp arithmetic:** `h' = dt * rate + h` (raise, kind 0 `rate = 2.0`), with the comparison `h < high` done *before* the add and **no clamp**; the update that crosses `high` overshoots by less than `dt*rate`, and the next update snaps `h = high`. Cells are painted over the exclusive rectangle `[ftol(min.x), ftol(max.x)) x [ftol(min.y), ftol(min.y + h))` (lever-init) on every update, set-only while raising.
6. **MM_W_012 ids 5/6:** overshoot can never change the visited rows (section 5), so the settled mask equals lever-init's 192 cells per gate (384 for the pair) and nothing is cleared after a raise.
7. **The clone cannot satisfy the contract as per-frame fidelity** (its only update point is `World::tick`, a fixed 0.2 s step), but it can supply the *input* (seconds, summing to real time under the unclamped, unpaused conditions) at tick granularity. Whether to accept tick-quantized sampling is a product decision; nothing here authorizes an instantaneous or approximated ramp (section 7).

## 2. Time producer to receiver (ASM/DECOMP; detail in static-trace M-N)

| Stage | Address | Fact | Label |
| --- | --- | --- | --- |
| Clock | `0x4657D1` | `CALL [0x4B0220]; SUB EAX,[ECX+8]` | ASM |
| Import identity | IAT `0x4B0220`; import descriptor FirstThunk `0xB0220` | name `timeGetTime`, DLL string `WINMM.DLL` (not `patch.dll`) | BYTES |
| dt_ms | `0x45CFF9`, `[0x4C6A08+0x25C64]` | `now - previous`, previous overwritten at the top of **every** iteration, before the active-window test `DAT_0058CDAC`; time spent with the window inactive is not accumulated | DECOMP |
| Clamp | `0x45D921-0x45D96B` | `if (100.0 < dt_ms || [0x4E33BB] != 0) dt_ms = ftol(0.1 * 1000.0) = 100`; constants `0x4C08E8 = 0.1`, `0x4B1758 = 1000.0`. The byte `[0x4E33BB]` (`this+0x1C9B3`) forcing a fixed 100 ms step is untraced | ASM + BYTES |
| dt seconds | `0x45DA15-0x45DA39` | `FILD dt_ms; FDIV 1000.0; FSTP double` | ASM |
| Passes | `0x45CFF9` (DECOMP), `0x45DB0E-0x45DD5C` (earlier ASM, fastforward-impl-001) | `[[this+0x55A8]+0x20] == 0 ? 1 : 7` passes, each with `local_40 = dt_ms` | DECOMP / prior ASM |
| Event split | `0x45CFF9` | a timeline list (`this+0x5584`) can cut one frame's dt into several `0x4423D2` calls whose dts sum to dt_ms; the level clock (ms int) advances by each segment | DECOMP |
| Paused / state != 2 | `0x45CFF9` | `FUN_004423D2(this, 0, 0)`: dt = **0.0** every frame | DECOMP |
| World update | `0x4423D2` | list order in section 1.3; each collection `vt+0x14(dt)` with `[EBP+0xC]`,`[EBP+8]` pushed unchanged | ASM (lists `+0x1F50`, `+0x1F70`) |
| Receiver list | loader `0x443C7C-0x443C8C` -> `0x448BD0` | receivers are built into `this+0x1F70`; levers are in `this+0x1F50` | ASM + DECOMP |
| Receiver step | `0x42B24D` (vtable `0x4B1020`+0x14) | `(this, double dt)`: first `0x432383`, then the request/done state machine | ASM |
| Rate | `0x4BF6F8 + kind*0x48` | kind 0 = 2.0, kind 1 = 0.5; one writer (`0x42885D`/`0x428909` inside `0x428848`, run through a static-init stub referenced from data `0x4BF020`; INFER: CRT initializer table), read only by the step | ASM + XREF |
| Second caller | `0x42A250` | `PUSH 0; PUSH 0; CALL 0x42B24D` inside `0x428A47` (7,927-byte "Fluid" constructor/stamper, no resolved caller): a **dt = 0.0 priming call** | ASM |

Source of the "elapsed" meaning: the dt is real elapsed time, not a tick count. A fixed number of updates does **not** correspond to a fixed wall-clock time; only the sum of dts does.

## 3. Per-update transition specification (MM_W_012 ids 5 and 6, kind 0)

Fields (lever-links/lever-init): request `r = [+0x18C]` (installed 1), `done = [+0x188]` (ctor 2), `armed = [+0x1B0]` (ctor 0), `h = [+0x1A8]`, `high = [+0x98] = 24.5`, `low = [+0xA0] = 0`, AABB `min.y = [+0x50] = 4`, `max.y = [+0x68]` (stream 28.5), x range 8 columns, kind 0, bit `0x200`, `rate = 2.0`.

Reader (once, at load, lever-init): `r == 1` -> `h := low = 0`, CLEAR `0x200` over the stream AABB (192 cells). `max.y` stays 28.5.

**Per `0x4423D2(dt)` call, in order (ASM):**

1. Lever (every update; installed cooldown 0.0): base step `0x432383(dt)`; `cooldown <= 0` -> hook + `[+0xA4] = 0`, else `cooldown -= dt`; contact `0x436D30` (every update); on contact and `cooldown <= 0` (tested **after** the decrement): activate `0x436E14` -> `0x436E6A(facing)`: `[+0xB8] = facing`, send the matching list; `[+0xA4] = 0`; `cooldown = [+0x98]` (0.0). For (183,60) the face -1 list sends **type 4 to ids 5 and 6**; that is two writes `[+0x18C] = 0`.
2. Receiver step (for each of ids 5 and 6, in list order; `[obj+0x38] != 0`): base step; `if (r == done) return-to-tail`; `r == 0` branch:
   * `if (h < high)`: `h = dt * 2.0 + h` (x87 `FLD dt; FMUL rate; FADD h; FSTP double`, so `dt*rate` is exact and the add rounds once to a double);
   * `else`: `h = high`; `if (armed) { armed = 0; done = 0 }`;
   * then always: `max.y = min.y + h`; **SET** `0x200` over `[x0, x0+8) x [4, ftol(max.y))`.
3. Tail: wave phase, models, emitters (untraced here).

Request/done life cycle for a targeted gate: load (`r=1, done=2, armed=0, h=0`, mask clear) -> first update: `r != done`, lower branch, `h <= low` -> settle: `armed = 1, done = 1`, no paint -> **idle** (`r == done`). Type 4: `r = 0 != done = 1` -> ramp until the settle update sets `armed = 0, done = 0` -> **idle at `h = high`**. A second type 4 on an idle raised gate: `r = done = 0` -> nothing at all. A type 4 *during* the ramp: `r` is already 0, the ramp is not restarted. **No lever in MM_W_012 sends type 5**, so the lower branch is exercised only by the load-time settle (and by reload).

**Edge states that matter for fidelity (ASM):**
* *Zero elapsed time:* `dt*rate = 0.0`; `h` unchanged; the SET repaints the same rectangle; net effect none. Paused frames and `timeGetTime` ties both give dt = 0.0.
* *First update before the load settle:* if a type 4 arrived before the gate's first update (`done == 2, armed == 0`), the raise reaches `high` but `armed == 0` leaves `done = 2`; the gate would re-SET every update forever. Not reachable in MM_W_012 play (actors cannot touch the lever before the first update), and it is exactly the state of the three untargeted request-0 gates, which is why they re-set their rectangle every update (idempotent).
* *Overshoot / endpoint crossing:* the crossing update leaves `high <= h < high + dt*2.0` and repaints `[4, ftol(4+h))`. The next update snaps to `high` and repaints `[4, ftol(28.5)) = [4,28)`; **rows set by the overshoot are never cleared**.
* *Reload:* `0x4432C1` re-reads the file grid and reconstructs every object (lever-init proof 1f): `h = 0`, mask cleared again for request-1 gates, `done = 2`, `armed = 0`, lever `[+0xB8]` from the stream. A raised gate does not persist across reload.
* *Cooldown:* `cooldown -= dt` uses the same seconds double; the installed values are 0.0, so no nonzero-cooldown survey was made.

## 4. Numerical illustrations (HYPOTHETICAL dts unless stated; the arithmetic is the proven one)

Derived closed forms, valid for one MM_W_012 gate (kind 0, `min.y = 4`, `high = 24.5`, `rate = 2.0`, mask rows `4..27`, 8 columns):
* Row `r` (4..27) first becomes set when `ftol(4+h) > r`, i.e. when `h >= r - 3`. Row 4 at `h >= 1`, row 27 at `h >= 24`.
* Set cells after an update with height `h` (below `high`): `8 * min(24, floor(h))`. Ids 5 and 6 together: `16 * min(24, floor(h))`.
* Accumulated dt for `h = 24.0` (full 192-cell mask): `24.0 / 2.0 = 12.0` s of dt. Accumulated dt for `h = high`: `24.5 / 2.0 = 12.25` s of dt. This is a sum of dts, which equals wall-clock only for unclamped, unpaused, non-fast-forward frames.

**Example A - dt = 0.125 s every update (HYPOTHETICAL; chosen because 0.125 and 0.25 are exact binary doubles, so no accumulation error):** `delta = 0.25` per update.

| Update n after the activating update (n = 1 is the activating update) | h | set rows | cells per gate | ids 5+6 |
| --- | --- | --- | --- | --- |
| 1 | 0.25 | none | 0 | 0 |
| 4 | 1.00 | 4 | 8 | 16 |
| 8 | 2.00 | 4-5 | 16 | 32 |
| 96 | 24.00 | 4-27 | 192 | 384 |
| 97 | 24.25 | 4-27 | 192 | 384 |
| 98 | 24.50 (not `< high` at the next check) | 4-27 | 192 | 384 |
| 99 | settle: `h = 24.5`, `done = 0` | 4-27 | 192 | 384 |

Full mask after 96 updates = 12.0 s of dt; endpoint after 98 updates = 12.25 s of dt; update 99 is a no-change settle. An extra type 4 at any n changes nothing.

**Example B - clamp (HYPOTHETICAL frame of 250 ms real):** `dt_ms = 100` -> `dt = 0.1` -> `delta = 0.2`; 150 ms of real time contribute nothing to the ramp. A frame of exactly 100 ms is not clamped (`100.0 < 100` false).

**Example C - fast-forward (HYPOTHETICAL 16 ms frames):** 7 updates of `dt = 0.016` -> 0.112 s of dt per frame; the gate gains `0.224` height per frame instead of `0.032`.

**Example D - overshoot at the largest possible step (HYPOTHETICAL constant dt = 0.1 s; decimal values are approximate because 0.2 is not an exact double):** `delta = 0.2`; `h` first reaches `>= 24.5` at update 123 (`h ~ 24.6`), `max.y ~ 28.6`, `ftol = 28`: the visited rows are still `4..27`; update 124 snaps to 24.5. The same holds for any `delta < 0.5`, and the clamp bounds `delta <= 0.2` (fast-forward does not enlarge a single update's dt). Hence for these gates the overshoot rows are an empty set.

**Mark of a hand-calculation hazard:** when `dt*2.0` is not an exact double (any 1 ms-granular dt except multiples of 1/512 s), the accumulated `h` is the repeated *rounded* sum, and counts at integer boundaries (`floor(h)`) can land one update earlier or later than the real-number prediction (for example five additions of a rounded 0.4 may give 2.0000000000000004 or 1.9999999999999998). A test of intermediate counts must come from an independent transcription of the x87 operation order above, not from hand-rounded decimals.

**Lever-side reference (from lever-links, installed):** for lever (183,60) initial `[+0xB8] = 1`; an actor facing -1 on the trigger sends type 4 to ids 5 and 6; `[+0xB8]` becomes -1; the next activation needs facing +1 and sends nothing (empty list); the following facing -1 sends type 4 again, a no-op on a settled gate.

## 5. Why overshoot is not a hazard in MM_W_012 (arithmetic)

Kind 0, `rate = 2.0`, the clamp at 100 ms, the same dt for each of the 7 fast-forward passes. Largest raise step `0.1 * 2.0 = 0.2` (< 0.5). `max.y_overshoot = 4 + (24.5 + e)`, `e < 0.2`, so `max.y < 28.7`; `ftol(max.y) = 28`. The painted rows after an overshoot are the same `4..27`. (If `[this+0x1C9B3]` forces exactly 100 ms the bound is the same.) For a hypothetical gate with `min.y` fractional part `p` the criterion would be `p + 0.5 + e < 1` and could fail; none exist in the installed data measured by lever-init.

## 6. Clone comparison (concrete sites)

| Contract element | Clone today | Site | Gap |
| --- | --- | --- | --- |
| Real elapsed seconds | `SimClock::advance(elapsedSeconds)` accumulates `max(0, dt)`; no 100 ms clamp, no dt = 0 update | `src/app/SimClock.cpp:6-9`, `src/app/RuntimeHud.cpp:51` | 100 ms clamp is not reproduced (fastforward-impl-001 "NOT reproduced"); a long stall is drained (max 10 ticks/frame) instead of dropped |
| Update cadence | `takeStep()` returns whole 0.2 s steps; `World::tick()` has no dt argument; `tickDtSeconds() = 1/ticksPerSecond = 0.2` | `src/app/SimClock.h:stepSeconds`, `src/sim/World.cpp:587-589`, `:889-938` | original: one update per frame with variable dt; clone: fixed 0.2 s quantum; intermediate mask sampling differs, totals agree in aggregate |
| Fast-forward | `kFastForwardPasses = 7`, each pass `clock_.advance(elapsedSeconds)` with the same delta | `src/app/RuntimeHud.cpp:46-52` | consistent with the original's 7 equal-dt updates in total dt (per-pass tick quantization differs) |
| Pause | `advance` returns false before any pass | `src/app/RuntimeHud.cpp:39` | original runs dt = 0 updates; ramp-equivalent (no progress) |
| Update order | per-lemming `step()` inside the lemming loop (lever contact inside it), then `++tick_` | `src/sim/World.cpp:935-938`, lever arm `:843-851` | original: all lemmings, then the lever step (contact + dispatch), then receivers; the clone evaluates contact during each lemming's own step and has no receiver step |
| Lever state | `Lever{x,y,w,h,state}` only; `state` toggled by facing; no cooldown, no link lists, no dispatch | `src/sim/World.h:210-217`, `src/sim/World.cpp:843-851`, `src/sim/RetailLevel.cpp:231-236` | links/`[+0xDC]`/cooldown parsed by `lemmings::read_compiled_level` (`decomp-lemmings/include/lemmings/level_gadget_records.hpp:54,105` `entities_42acac`) but discarded |
| Gate / receiver | none | - | `Entity42acac` is parsed (`values_18c_128_168`, `double_bits_48_60`, doubles) and unused outside tests |
| Zone mask | `Terrain::zoneWater_` set once from the file grid; `setZoneWater(x,y,on)` exists; `y >= height_` ignored; rows flipped (`destY = height - 1 - y`) | `src/sim/Terrain.cpp:191-200`, `:349`, `:370-371` | rect painter with `x & 0xFF`, y clamp and the Y flip does not exist; original rows 4..27 map to clone rows `95-y` (68..91) |
| Reload | `loadRetailLevel` rebuilds `LevelSetup` | `src/sim/RetailLevel.cpp` | satisfies "reload restores the load state" if gates live in `LevelSetup` |

Can the current clone clock satisfy the contract? **For the input value, yes in aggregate:** each `World::tick` represents exactly 0.2 s of accumulated real time (unclamped), so passing `tickDtSeconds()` to a gate step reproduces `sum(dt)` and the endpoint heights. **For per-update fidelity, no:** the original samples the mask once per frame with a variable dt, the clone once per 0.2 s tick. The first height that satisfies `h >= 24.5` is reached at a tick boundary (62 ticks x 0.4 = 24.8 for constant 0.2 s ticks, versus exactly 12.25 s of dt in the original), so the clone completes up to one tick (0.2 s) later in the worst case and shows 5 Hz stair-steps instead of per-frame steps.

## 7. One bounded implementation proposal (not queued; requires an explicit decision)

**Title: lever-linked fluid gates, stepped per `World::tick` with `dt = tickDtSeconds()` (zone mask only).**
This is the only proposal; it contains no instantaneous raise, no stand-in "enough ticks" constant and no changed clock.

Production changes (a future task, not authorized by this one):
* `src/sim/RetailLevel.cpp` / `RetailLevel.h`: read `tagged_entities` type 1 (face lists, `[+0xDC]`, cooldown) and `entities_42acac`; build `LevelSetup::gates` (`id, kind, request, done=2, armed=false, h, high, low, min/max AABB`) and give `Lever` its link lists and cooldown. Apply lever-init's load pass (request 1 -> `h = low`, CLEAR; request 0 -> `h = high`, SET).
* `src/sim/World.h` / `World.cpp`: `FluidGate::step(double dt)` as a line-for-line transcription of section 3 (same compare-before-add order, no clamp on the raise, SET on settle, `armed`/`done` bookkeeping); `Lever` activation sends type 4/5 to gates by base id (the facing/state filter already exists at `:161-180`; add the face lists and the cooldown with `cooldown -= dt`); call all gate steps once per `tick()` **after** the lemming loop (`:935-937`), before `++tick_`, with `dt = tickDtSeconds()`.
* `src/sim/Terrain.h` / `Terrain.cpp`: a rectangle SET/CLEAR for `zoneWater_` with the original traversal (exclusive `[ftol(min), ftol(max))` bounds, `x & 0xFF`, y clamp, Y flip `destY = height - 1 - y`). Add the `0x2000` plane only if a measured level needs it (MM_W_012 does not).
* No render, fluid-surface, emitter or sound files. No change to `SimClock`, `Rules::ticksPerSecond`, D1/D3, release or digger timing.

**MM_W_012 acceptance (real level through `loadRetailLevel`, source-built; the walker injection is a synthetic fixture on the real level, not natural play):**
1. *Before (current code, derivable now):* `zoneWaterCells() == 3600` at load; pulling lever (183,60) any number of times leaves it 3600 (only `Lever::state` changes).
2. *After load:* 528 (lever-init: 3072 cleared, ids 5/6 = cols 152..167 rows 4..27 flagged -> clear; the three request-0 gates 528 cells stay).
3. *Activation:* a walker facing -1 on lever (183,60) -> after k ticks since the activating tick, the 384-cell region for ids 5/6 holds `16 * min(24, floor(h_k))` cells where `h_k = fl(h_{k-1} + fl(0.2 * 2.0))` is computed by a reference that transcribes the same operation order (section 4 hazard), `k = 1..63`; every other gate and every non-gate cell is unchanged; at the settle tick (first tick with `h_{k-1} >= 24.5`) `h == 24.5` exactly and the count is 384.
4. *Repeat:* a facing +1 pull sends nothing; a second facing -1 pull on the settled pair leaves the count at 384 and `done == 0`; a facing -1 pull *during* the ramp does not reset `h`.
5. *Fast-forward / pause (app level, via `RuntimeSimState`):* with the latch the same state is reached in 1/7 of the frames of the same elapsed seconds; paused frames add no cells. The 100 ms clamp is **not** claimed.
6. *Reload:* reloading MM_W_012 returns the count to 528 with the pair idle at `h = 0`.

**Honest limits of this proposal:** (a) the tick-quantization of section 6 (sampling every 0.2 s, completion up to one tick late); (b) the clamp and inactive-window time loss are not reproduced; (c) timeline-event dt splitting is not modelled; (d) lever contact evaluation order differs from the original's separate post-lemming lever step; (e) a zone-bit change is **not** visible fluid, emitter, drowning or any other downstream behaviour, and a scripted win does not prove complete lever behaviour or original fidelity; the only consumers of `0x200` traced so far are the fall-landing splat exemption and the terrain-probe rule (lever-links).

**Park statement.** This proposal does not reopen D1/D3/base-clock, release/digger timing, Armageddon, clipping/controller/capture. Per-frame (original-cadence) gate stepping **would**: it needs the original's frame-based variable-dt update (replacing the 0.2 s `SimClock` quantum for at least this object), the 100 ms clamp and event splitting, which is the base-clock work. That reopening needs separate user authorization and is not requested here.

**Unresolved dependencies (exact edges):**
1. Writers and meaning of `[0x4E33BB]` (forces dt = 100 ms).
2. x87 precision control at run time (affects at most the last bit of `h`; endpoint comparisons are exact for dyadic dts).
3. Which timeline events MM_W_012 contains and how often they split dt.
4. `0x428A47`'s caller (the other dt = 0.0 caller of the step).
5. Whether a stationary actor re-fires lever contact during the dt = 0.0 paused updates (the `[+0xB8]` alternation prevents an immediate repeat).
6. Lever `vt+0x14` enable byte `[+0x38]` was not re-derived (retained play shows it active).
7. Fluid surface, emitters, wave phase and every consumer of bit `0x200` beyond the two traced.

## 8. Corrections / refinements of earlier accepted claims (earlier files unmodified)

| Earlier statement | Refinement |
| --- | --- |
| lever-links section 3/Claim table: "unit of `dt` is the world step's elapsed time, parked base-clock quantity" | `dt` is seconds (`timeGetTime` ms / 1000.0, <= 0.1 s). The parked part is the clone's tick cadence, not the unit. |
| lever-links section 3: "type 4: ... `h += dt*rate`" | Confirmed, with detail: compare-before-add, no clamp; the settle update repaints and the overshoot rows are never cleared (empty for MM_W_012). |
| lever-links Proposal 3: "the end state is `[+0x98]`, independent of `dt`" | True for `h` and for MM_W_012's cell set; not true in general (overshoot rows persist when `min.y` has a suitable fraction). |
| lever-init: "first step after load for a request-1 gate is a no-paint settle" | Confirmed; additionally runs with dt = 0.0 when the first update occurs while the game is not in the playing state (paused/pre-start path of `0x45CFF9`). |
| fastforward-impl-001: "0x45D927-0x45DA3C 100 ms delta clamp NOT reproduced" | Confirmed as ASM: forces 100 ms when `dt_ms > 100` or when `[0x4E33BB] != 0`; still not reproduced in the clone. |

## 9. Verification and limits

* **Supervisor verification: pending at worker exit.** No source, CMake, test, queue or control file was changed, so no new test was registered and none was run by this worker. No development profile was run. The retained build output is the static-trace file only.
* Independent review must cite the supervisor's final verifier log for `python tools/agent_control/verify.py --profile all` (or main + recovery), not this report.
* REA used only as static analysis on the registered target (SHA-256 `32675bdb...37a2f`); Bash was used for read-only listing/reads of repository text. The original was not executed; `patch.dll`, `dinput.dll` etc. were not loaded or analyzed.
* Source-trace and arithmetic examples are not original-fidelity proof of rendering, fluid or gameplay; the unresolved list in section 7 is part of the result.
