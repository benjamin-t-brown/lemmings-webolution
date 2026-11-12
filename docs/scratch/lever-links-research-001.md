# lever-links-research-001: serialized lever links and original receiving behavior

Status: **static trace complete for MM_W_012 including the gate fields; one bounded implementation proposal; the registered `leverlinks_measure` driver compiled and passed (see "Verification and limits").**
Static work only; the original was never run. No production behavior changed.
Reference: `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f` (returned by `open_binary`, Ghidra 12.1.4, x86 PE, image base 0x400000).
Retained evidence: `build/lever-links-research-001/static-trace.md` (sections A-F) and the ctest working-dir output
`build/agent-native/main/lever-links-research-001/primary-MM_W_012.txt` / `census.txt`.
Labels: DECOMP = Ghidra pseudocode checked against the listed instructions; ASM = instructions read; BYTES = `read_bytes`; MEASURED = source-built driver on installed bytes;
INFER = inference; HIST = earlier runtime note (`old/docs/scratch/phase3-round21-lever.md`), not new evidence.

## Outcome

The MM_W_012 lever targets are **not** STAT/DYNA/FALL objects. All 16 link records point (type 4) at **`entities_42acac` objects: a fluid-zone gate class
(vtable `0x004B1020`, ctor `0x00428680`, reader `0x0042ACAC`, step `0x0042B24D`)**. Type 4 sets the gate's *requested state* `[+0x18c] = 0`; type 5 sets it to 1.
The step function then ramps a height `[+0x1a8]` toward `[+0x98]` (request 0) or `[+0xa0]` (request 1) and, on every step of the ramp, **sets or clears a zone bit in the
world cell grid (`0x004C6A10`) over the object's AABB**: bit `0x200` for kind `[+0x128]==0`, bit `0x2000` for kind 1. So the effect is a **dynamic fluid-zone cell mask plus a
moving fluid-surface model**, not a door, bridge or terrain edit. The clone already names those bits (`kCellZoneType0 = 0x200`, `kCellZoneType1 = 0x2000`) but only copies them
statically from the grid at load (`Terrain.cpp:370`), and parses nothing about the gates or the lever links.

Proposal (section "Proposal"): one sim-level change - parse the gate and lever-link records and drive the zone-cell mask from lever messages - with the
tick/rate mapping explicitly left to the parked base-clock work. Render of the moving surface is a separate, un-traced dependency.

## 1. Activation, fan-out, id resolution (DECOMP+ASM; unchanged from first pass, validated against instructions)

* Lever vtable `0x004B1288`; contact `0x00436D30` (actor list `0x004C88D0`, AABB `0x00430BA1`, requires pulling actor facing `A+0x40 == -1` if `this+0xB8 == 1`, else `+1`).
* Step `0x004339FC` fires activate `0x00436E14` when cool-down double `[+0xA8] <= 0` and reloads it from serialized `[+0x98]`. MEASURED cool-down for all four MM_W_012 levers = 0.0.
* `0x00436E6A(face)`: face `+1` -> `[+0xB8]=1`, list `+0xCC`; face `-1` -> `[+0xB8]=-1`, list `+0xBC`; other values do nothing. **-1 is a valid face** (a correction of the review note that treated -1 as ignored).
* Dispatch `0x00433AC0` -> per element `0x00433B2E`: `elem+4 >= 0` -> `target = idList[elem+4]` (`0x00444BA1`); type 0 -> `vt+0xC(p1,p2)`; **type 4 -> `vt+4()`; type 5 -> `vt+8()`**.
* Stream layout (reader `0x00436FED`): base, `f64 [+0x98]`, `u8 [+0xB0]`, `u8 [+0xA0]`, `u32 [+0xDC]` (initial face), list `+0xBC` (**face -1**), list `+0xCC` (**face +1**); each list = `u32 count` + `count x (id, type, p1, p2)` (`0x00433FB9`).
* Target = the actor of any collection whose base `int16 id` equals the record id (later construction wins; slot 0 skipped).
* Initialization: `0x00444417` calls vt+0x28 = `0x0043703F` = `0x00436E6A(this, [+0xDC])` per tagged object, so the stored face is applied at load and **the matching list is fanned out once at load**.

## 2. Receiver class of the MM_W_012 targets (new; DECOMP+ASM+BYTES)

Level loader call `0x00448BD0` (list "42acac", 532 installed records; `level_entity_tail.md`) allocates `0x2A8` bytes, runs ctor **`0x00428680`**, then calls `vt+0x18`.

| Item | Evidence |
| --- | --- |
| Vtable | ASM `0x428739: MOV [EAX], 0x4b1020`. BYTES `0x4B1020` (8 dwords: `00c94200 c0c84200 e0c84200 40dc4100 c9c54200 4db24200 acac4200 b8b04200`): `[0]0x42c900 [1]0x42c8c0 [2]0x42c8e0 [3]0x41dc40 [4]0x42c5c9 [5]0x42b24d [6]0x42acac [7]0x42b0b8` (the last two dwords are at `0x4B1038..`). The earlier "data block at `0x4B1038`" is the *tail* of this vtable, not a separate structure. |
| ctor init | ASM `0x4287ae..0x4287de`: `[+0x38]=1`, `[+0x1b1]=[+0x38]`, `[+0x188]=2`, `[+0x18c]=0`, `[+0x1b0]=0`; list headers at `+0x12c` and `+0x170/+0x194/+0x1b8/+0x218/+0x278` arrays |
| **vt+4 `0x0042C8C0` (message type 4)** | ASM: `[this+0x18c] = 0` |
| **vt+8 `0x0042C8E0` (message type 5)** | ASM: `[this+0x18c] = 1` |
| reader `0x0042ACAC` | DECOMP: spatial base `0x0043130E` (two triples: min `+0x48/+0x50/+0x58`, max `+0x60/+0x68/+0x70`); three models (`+0x158`,`+0x15c`, optional `+0x160`,`+0x164`); `u32 [+0x18c]` (initial request), `u32 [+0x128]` (kind), `u32 [+0x168]`; doubles `[+0x98]` (high), `[+0x1a8]` (overwritten below), `[+0xa0]` (low). The stream does **not** contain a list for `+0x12c`. |
| load-time height | DECOMP: if `[+0x18c]==1`: `[+0x1a8] := [+0xa0]` and *clear* bit (kind 0 -> `0x200`, kind 1 -> `0x2000`) via `0x0043127e`; else `[+0x1a8] := [+0x98]` and *set* the bit via `0x004311d0`. |
| rect painter `0x004311d0` / `0x0043127e` | ASM: x from `ftol([+0x48])` to `ftol([+0x60])` (set variant adds 0x100 if wrapped), y from `ftol([+0x50])` to `ftol([+0x68])`; cell = `0x0040DE60(0x4C6A10, x, y)` (x masked to 8 bits); OR (set) or AND-NOT (clear) the argument. Both take `this` in ECX; in the reader/step the clear call `FUN_0043127e(0x200/0x2000)` is shown by Ghidra without an explicit `this` (ECX carried over; INFER, not re-checked in the caller assembly). |

## 3. Step `0x0042B24D` state machine (DECOMP; written from the pseudocode, not re-derived from every instruction)

`h = [+0x1a8]`, `req = [+0x18c]`, `done = [+0x188]` (starts 2), `armed = [+0x1b0]` (starts 0), `rate = double [0x4BF6F8 + kind*0x48]`, `dt` = step arguments. While `req != done`:

* **req 0 (message type 4)**: if `[+0x98] <= h`: `h := [+0x98]`; if `armed`: `armed := 0; done := 0`. Else `h += dt*rate`. Then `[+0x68] := [+0x50] + h`; **set zone bit over the AABB**.
* **req 1 (message type 5)**: if `h <= [+0xa0]`: `h := [+0xa0]`; if `!armed`: dispatch the `+0x12c` list (empty for serialized objects, INFER), `armed := 1; done := 1`. Else: **clear zone bit over the current AABB**, `h -= dt*rate`, `[+0x68] := [+0x50] + h`; if `[+0xa0] < h`: **set zone bit over the new AABB**.
* After that, for all states: animated wave phase `[+0x148]`/`[+0x150]` and child-model matrix/colour updates (`0x42CC10`, `0x42CC40`, `0x42CB70`, `+0x118` slots), and for each id in the id list `+0x194` (lines `0x404D30/0x404D10`) an object in table `0x4CBB08` gets position `+0xd8`/`+0x120` and flag `+0x138`. These are the render / secondary-object consequences; **not traced further**.
* Rate table written by startup code `0x00428848` (the file image is zero): kind 0 `[0x4BF6F8] = 2.0`, kind 1 `[0x4BF740] = 0.5` (DECOMP of the stores `_DAT_004bf6f8 = 0 / _DAT_004bf6fc = 0x40000000` and `_DAT_004bf740/744 = 0 / 0x3fe00000`). Recorded as evidence only; the unit of `dt` is the world step's elapsed time, which is the **parked base-clock** quantity.

Resulting semantics (INFER from the above): type 4 = **raise the fluid to `[+0x98]`**, type 5 = **drain to `[+0xa0]`**; the zone is empty at the drained level; starting mode 0 means "full at load", 1 means "drained at load". A repeated type 4 on a settled gate (`req == done == 0`) does nothing.

## 4. MM_W_012 per-lever table (MEASURED, `primary-MM_W_012.txt`)

Member `COMPILED\MM_W_012.LVL` in `lemmings.box` (patch.box has no such member): entry offset 68363014 (0x4132306), size 846980, FNV-1a64 `0x378dfb7c2f0fe1c0`; structured reader consumed all bytes; 26 actors / 26 distinct ids; 4 tagged (all `lever.a3d`), 0 named. Pairing with clone levers is by exact pose transform (`x = pose.x-1`, `y = 95-(pose.y+6)`), not file order.

| Lever (stream) | clone box | record @ | init face `[+0xDC]` @ | cool-down | face -1 list (`+0xBC`) @count | face +1 list (`+0xCC`) @count | load-time fan-out | targets (class) |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| tagged[0] id 0 | (79,60) | +0xC4D71 | -1 @+0xC74D7 | 0 | empty @+0xC74DB | **ids 8,7 type 4** @+0xC74DF (recs +0xC74E3, +0xC74F3) | list -1 (empty): none | `entities_42acac`[6],[7] |
| tagged[1] id 1 | (72,34) | +0xC7503 | +1 @+0xC9C69 | 0 | **ids 9,18,19,20,21,22 type 4** @+0xC9C6D | empty @+0xC9CD1 | list +1 (empty): none | `entities_42acac`[5],[4],[3],[2],[1],[0] |
| **tagged[2] id 2 (observed (183,60))** | (183,60) | +0xC9CD5 | +1 @+0xCC43B | 0 | **ids 5,6 type 4** @+0xCC43F (recs +0xCC443, +0xCC453) | empty @+0xCC463 | list +1 (empty): none | `entities_42acac`[12],[11] |
| tagged[3] id 3 | (189,35) | +0xCC467 | -1 @+0xCEBCD | 0 | empty @+0xCEBD1 | **ids 4,17,16,15,14,13 type 4** @+0xCEBD5 | list -1 (empty): none | `entities_42acac`[13]..[18] |

Every record has `p1 = p2 = 0`. **No lever sends type 5**; every non-empty list is type 4 only.

Direction consequences (DECOMP + MEASURED lists):
* tagged[2] (183,60): initial face +1 (`+0xB8 = 1`), so contact requires the pulling actor's facing = **-1**, which fires the **face -1 list = ids 5 and 6, type 4**. The `+0xCC` list (face +1) is empty, so a pull in the other direction sends nothing. After the first fire `+0xB8 = -1`; the next fire requires facing +1 and sends nothing; the third (facing -1) re-sends type 4 to the same gates, which is idempotent once settled. No lever in MM_W_012 can send type 5, so **no lever in this level can lower a gate that it raised**.
* No lever has a non-empty list for its initial face, so nothing is dispatched at load.
* Cool-down is 0 for all four, so the cool-down gate does not limit repeats; the opposite-facing/state gate does. This is why the toggle in `World.cpp` is a bare alternation with no downstream effect.

### Gate fields for the 16 distinct target ids (MEASURED, `primary-MM_W_012.txt`, registered `leverlinks_measure`)

All 16 targeted gates carry identical class fields: init request `[+0x18c]` = 1, kind `[+0x128]` = 0 (zone bit `0x200`), `[+0x168]` = 25, high `[+0x98]` = 24.5, stream height `[+0x1a8]` = 24.5 (overwritten by the reader to `[+0xa0]` = 0 because the init request is 1), low `[+0xa0]` = 0, AABB y min 4, y max 28.5 (width 8 in x). So every gate is **drained at load** (initial height 0, `0x200` cleared by the reader) and **type 4 raises it to 24.5**, i.e. flagged y range `[4, 4+24.5) = [4, 28.5)` over its x range. No gate is kind 1, so the `0x2000` plane is not needed by this level. (Gates `entities_42acac[8..10]` are not targeted by any MM_W_012 lever.)

| Lever (clone box) | Direction that fires | Gate (`entities_42acac` idx, id) | x range | initial h | after type 4 h | flagged y |
| --- | --- | --- | --- | --- | --- | --- |
| tagged[0] (79,60) | face +1 list (init face -1: not at load) | [6] id 8, [7] id 7 | [96,104), [104,112) | 0 | 24.5 | [4,28.5) |
| tagged[1] (72,34) | face -1 list (init face +1: not at load) | [5] id 9, [4] id 18, [3] id 19, [2] id 20, [1] id 21, [0] id 22 | [56,64), [48,56), [40,48), [32,40), [24,32), [16,24) | 0 | 24.5 | [4,28.5) |
| **tagged[2] (183,60) observed** | face -1 list (init face +1: not at load) | **[12] id 5 x[160,168); [11] id 6 x[152,160)** | [160,168), [152,160) | 0 | 24.5 | [4,28.5) |
| tagged[3] (189,35) | face +1 list (init face -1: not at load) | [13] id 4 .. [18] id 13 (ids 4,17,16,15,14,13) | [200,208) .. [240,248), 8 wide each, ascending | 0 | 24.5 | [4,28.5) |

Per-gate rows with record offsets are in `primary-MM_W_012.txt` lines 13-81; lever (183,60) is lines 44-56.

**Stored-grid baseline (MEASURED, new driver rows `stored grid ...`).** `Terrain::fromCollisionGrid` copies cell bit `0x200` from the installed file grid (`Terrain.cpp:370`, copy to `zoneWater_`). For all 16 gate rectangles the stored grid, over x = the gate's 8 columns and y rows 4..28 inclusive (200 cells), has `0x200` on **192 cells** (rows 4..27 fully, 8 cols x 24 rows) and `0x2000` on none; row 28 carries no `0x200`. The count with the top row excluded is also 192, so the stored flagged region is exactly `[4,28)` x the gate's x range, the same region type 4 would set (`[4,28.5)`, rows 4..28 or 4..27 depending on the painter's inclusive bound, which is not resolved). So the installed grid holds each gate in its **raised** state, while the original reader clears the bit at load for init-request-1 gates (section 2). Consequence: **the clone as shipped starts MM_W_012 with the gate zones flagged (raised) where the original, by this static reading, starts them cleared (drained).** Not established: that the grid is in place before the gate reader runs (section "Claim / evidence / unknown", static-trace section G: call order only), so whether the load-time clear takes effect on the live grid is INFER.

Second installed level: **none used.** MM_W_012 has linked targets of one resolvable class, so no additional level was needed.

## 5. Clone comparison (source)

| Original behavior | Clone today | Where information is dropped |
| --- | --- | --- |
| Lever link lists + `[+0xDC]` + cool-down | `lemmings::read_compiled_level` parses `TaggedEntity::{values[0], record_arrays[2], double_98}` | `RetailLevel.cpp:231-236` builds `Lever{x,y,2,6,state=1}` from `findLevelObjects` (`LevelObjects.cpp:188`, model-name match) and discards all of it; **no production file reads `tagged_entities` or `named_entities`** |
| Toggle + fan-out | `World.cpp:843-851` toggles `Lever::state` for walkers; no cool-down, no facing/state filter, no dispatch | `Lever::state` has no consumer |
| Gate objects (`entities_42acac`) | parsed into `Entity42acac{values_18c_128_168, doubles_98_1a8_a0, double_bits_48_60,...}` | never read outside tests/tools; no runtime object exists |
| Zone cells `0x200/0x2000` | `Terrain.cpp:370` copies `0x200` (and `0x4000`) once into `zoneWater_`/`zoneAcid_`; consumer: splat exemption (`Terrain.cpp:238`, `World.cpp:313`) | `0x2000` is not copied; zone masks are never modified after load |

Original consumers of bit `0x200` per retained recovery notes (HIST-free, static): fall-landing splat exemption (`actor_landing.md`), and the terrain probe treats a cell with `0x200` as non-stopping (`actor_terrain_probe.md`: a row stops the scan only if nonzero and `0x200` clear). The clone implements the first only.

## Claim / evidence / unknown

| Claim | Evidence | Unknown |
| --- | --- | --- |
| Link record = `(id, type, p1, p2)`, list `+0xBC` = face -1, `+0xCC` = face +1 | DECOMP `0x436FED/0x433FB9/0x436E6A`; MEASURED offsets in section 4 | none for MM_W_012 |
| Targets resolve by base id across all collections | DECOMP `0x433B2E/0x444BA1`; MEASURED all 16 ids resolve to `entities_42acac` | - |
| Receiver of type 4/5 is vtable `0x4B1020` `[+0x18c] := 0/1` | ASM `0x42c8c0`, `0x42c8e0`, vtable BYTES | class's gameplay name |
| Consequence: height ramp + zone bit `0x200/0x2000` set/clear over AABB | DECOMP `0x42b24d`, ASM `0x4311d0`, `0x43127e` | the step's `dt` source and the cells' downstream effects beyond splat/probe |
| Rate 2.0 (kind 0), 0.5 (kind 1) per `dt` | DECOMP of table writer `0x428848` | units of `dt` (parked clock) |
| Observed lever (183,60) feeds gates 5 and 6 only on facing -1 | MEASURED + DECOMP | none for kind/height: gates 5 and 6 are kind 0, high 24.5, low 0, drained at load (section 4); effect beyond the `0x200` zone mask untraced |
| Stored MM_W_012 grid carries `0x200` over all 16 gate rects (192/200 cells each, rows 4..27) | MEASURED `stored grid` rows | whether the original's load-time clear acts on that grid (grid-vs-gate-reader order is call-order INFER only, static-trace G); painter's y upper bound inclusive/exclusive |
| Easy 20/20 win is not effect evidence | targeted-play-001; zone-bit change only affects falls/probes in the water zone | - |
| Moving surface / emitter rendering | DECOMP shows model matrices and `0x4CBB08` objects updated | not traced to pixels; no clone model exists |

## Proposal (one, bounded; not queued)

**Title: fluid-zone gates driven by lever links (sim-only).**
Files: `src/sim/RetailLevel.cpp` + `RetailLevel.h` (read `tagged_entities` type 1 and `entities_42acac`; build `LeverLink{faceMinus, facePlus, initFace, cooldown}` and `FluidGate{id, kind, req, high, low, aabb}`; id lookup by base id); `src/sim/World.cpp` / `World.h` (replace the bare `Lever::state` toggle at `World.cpp:843-851` with the original facing/state gate and a fan-out that sends type 4/5 to gates by id, plus the load-time `[+0xDC]` fan-out); `src/sim/Terrain.cpp` / `Terrain.h` (make `zoneWater_` mutable per rect with set/clear helpers; add the `0x2000` plane only if a measured level needs it). No render files in this step.

Acceptance (real level MM_W_012, tests source-built, no original run):
Rectangles and heights below are the measured rows in section 4; the new test derives them from the level bytes, it does not hard-code them.

1. *Before (current code, measured/derivable now)*: the clone's `zoneWater_` after load already has `0x200` on **192 cells in each of the 16 gate rectangles** (x = gate columns, y rows 4..27; section 4 stored-grid rows), including the two for lever (183,60): x[160,168) and x[152,160), rows 4..27, 384 cells. Pulling lever (183,60) any number of times leaves `zoneWaterCells` unchanged; only `Lever::state` changes.
2. *After, load*: the 16 gates start drained (init request 1, height 0), so the new loader clears `0x200` over each gate AABB: those 16 x 192 = 3072 cells go from flagged to clear (gated on the unresolved ordering question below; if the original's grid is installed after the gate reader the clear would not apply and this step must be dropped, not approximated). No other `0x200` cell changes.
3. *After, activation*: lever (183,60) starts at face +1 (`+0xB8 = 1`). A pull by an actor facing -1 fires the face -1 list: type 4 to gate ids 5 and 6 (`entities_42acac`[12], [11]). Their rects x[160,168) and x[152,160) become flagged for y rows [4,28.5) (384 cells) once settled; other gates (e.g. id 9, x[56,64)) stay clear. A following pull by an actor facing +1 sends nothing (empty list); a third pull facing -1 re-sends type 4 and changes nothing once settled. Which-gate-moves is checkable at rect level without any rate: the end state is `[+0x98]`, independent of `dt`.
4. *Load-time fan-out*: none of the four MM_W_012 levers has a non-empty list for its initial face (section 4), so zero messages are dispatched at load (a check, not an effect).
5. Tests for kind-1 gates and request-0 gates (full at load) use the first installed level that has them, chosen by data; none exists in MM_W_012 (all 16 targeted gates are kind 0, request 1).

Dependencies and parks:
* **Ramp rate (`dt * rate`) is parked.** The original ramps per elapsed-time step; the clone's tick-to-time mapping is the parked base-clock work. The implementable part is the state machine (request/settled/armed), the endpoint heights, the set-on-grow / clear-on-shrink order, the zone rect arithmetic and the lever gate. The ramp itself must not be approximated; either defer it until the clock is decided, or acceptance asserts only settled end states after "enough" ticks with an explicitly declared stand-in.
* Rendering of the moving fluid surface and `0x4CBB08` emitters is a separate, untraced dependency (section 3).
* Whether `0x2000` zones need a clone plane depends on which kinds the installed levels use (all 16 MM_W_012 targets are kind 0; other levels not counted).
* **Effect scope (honest limit).** The only downstream consumers of `0x200` traced so far are the fall-landing splat exemption (`actor_landing.md`) and the terrain-probe test on `0x200` (`actor_terrain_probe.md`). Retained notes (`CollisionGrid.h`) say this is the water *zone* bit, **not** the drown bit `0x40`. The render of the moving fluid surface and any moving-surface/collision effect of the gate model are **untraced**; the proposal therefore changes only the zone mask and must not be described as "water appears". The `dt` rate (kind 0: 2.0 per step, evidence only) is parked and is not approximated: acceptance asserts settled end states only, and the ramp is deferred until the base clock is decided.
* Timed Armageddon, controller, clipping and digger/release timing are not touched.

## Verification and limits

* The read-only driver `src/__test__/test_leverlinks_measure.cpp` (registered test `leverlinks_measure`) prints, additively, the gate fields under each `entities_42acac` target (`receiver 0x4B1020: ...`, `AABB ...`) and, in this pass, the stored-grid baseline rows (`stored grid x[..) y[..] inclusive: cells=.. with 0x200=..`) using `webo::findCollisionGrid`. Its assertions, the test timeouts and all other tests are unchanged.
* Registered run `python tools/agent_control/verify.py --profile all` (this pass, after adding the stored-grid rows): `leverlinks_measure` rebuilt and **passed (29.83 s)**; the retained output `build/agent-native/main/lever-links-research-001/primary-MM_W_012.txt` contains the gate rows and the 16 `stored grid` rows quoted in section 4. A first run of the same command was stopped at the 10-minute background limit (test 84/137, all prior passing) and is not counted. The complete second run (`verify.py --profile all`, exit 0) reported **main: 137/137 passed (`leverlinks_measure` 28.14 s) and recovery: 84/84 passed**, with the stored-grid rows present in the retained output.
* Existing tests, assertions, timeouts, queue, verifier, permissions, saves, parks and production files are untouched. REA was used only as static analysis on the registered target (SHA-256 `32675bdb...37a2f`, Ghidra 12.1.4); sessions were closed with `close_binary` without snapshot arguments. The original was never run.
* Source tests and measurement rows are not original-fidelity proof.
* Unknowns that remain: whether the original's grid is installed before the gate reader so the load-time clear takes effect (call order only, static-trace G); whether the painter's y upper bound is inclusive (rows 4..28) or exclusive; the `+0x12c` list's possible runtime population; the exact `ECX` for the clear calls; the render and moving-surface consequences; consumers of `0x2000`; the `dt` unit (parked).
