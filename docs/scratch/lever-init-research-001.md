# lever-init-research-001: receiver initialization order and exact zone-cell boundaries

Status: **both questions answered from instructions; one implementation-ready scope that does not depend on parked timing is stated, with its limits.**
Static work only; the original was never run and no original bytes were executed. No production behavior changed.
Reference: `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f` (returned by `open_binary`, Ghidra 12.1.4).
Evidence: `build/lever-init-research-001/static-trace.md` (sections H-L, instruction excerpts and addresses) and the registered measurement output
`build/agent-native/main/lever-init-research-001/gate-cells-MM_W_012.txt` (test `leverinit_measure`, source `src/__test__/test_leverinit_measure.cpp`).
Accepted prior artifacts (`docs/scratch/lever-links-research-001.md`, `build/lever-links-research-001/static-trace.md`) are unmodified; corrections are listed under "Corrections to earlier claims".
Labels: ASM = instructions read; DECOMP = pseudocode checked against ASM; BYTES = `read_bytes`; XREF = resolved static references; MEASURED = source-built driver on installed bytes; INFER = inference.

## Answers

1. **Grid instance and load-time clear.** The cell grid is the member at `+8` of the singleton at `0x004C6A08` (`0x4C6A10`). Both painters hard-code `0x4C6A10`. The level loader `0x4432C1` runs with
   `this = 0x4C6A08` and fills that grid (`0x439FD3(this+8, ...)` -> `0x43B640(+0x41C)`, a raw bulk read of the file's cell array) **before** any gate is built (`0x448BD0` is 12 loader calls later: `0x437D48`, `0x447960`, `0x447A40`, `0x448490`, `0x448550`, `0x448650`, `0x448710`, `0x4487F0`, `0x4488F0`, `0x4489F0`, `0x448AF0`, then `0x448BD0`). The gate reader's clear therefore acts on the live,
   already-filled grid. No later step of the load was found that writes the cell array (details and the one residual gap below). The load-time clear **survives to gameplay entry** for MM_W_012.
2. **Exact traversal.** Both painters visit the half-open rectangle `[ftol(min.x), ftol(max.x)) x [ftol(min.y), ftol(max.y))`; `ftol` (`0x49C5AC`) truncates toward zero; x is masked `& 0xFF`, y is clamped to `[0, height-1]`; cells are 4-byte elements at `x + (y << 8)`.
   For every installed gate `max.y = 28.5` so the last row is 27. The earlier "200 cells" was an inclusive geometric enumeration the binary does not perform: **192 cells are visited per gate.**
3. **Counts.** The inherited predictions **3072** (load-time clears over the 16 targeted gates) and **384** (ids 5 and 6 after type 4) are **confirmed** by the traced traversal, but their stated basis ("rows 4..28 depends on the inclusive bound, unresolved") is replaced by the instruction-backed exclusive bound.

## Proof chain 1: which grid, was it filled, is the clear overwritten

| Step | Evidence |
| --- | --- |
| a. Painters use a fixed grid | ASM `0x4311D9: mov [ebp-0xC], 0x4c6a10`; `0x431287: mov [ebp-8], 0x4c6a10`; `this` (ECX) is only the **AABB owner** (`+0x48..+0x68`). Grid = `0x4C6A08 + 8`. |
| b. Loader `this` is `0x4C6A08` | BYTES `0x450A4E: b9 08 6a 4c 00` (`mov ecx,0x4c6a08`), `0x450A53: e8 a1 c5 00 00` -> `0x45CFF9` (frame loop, `__fastcall(param_1)`). The loop calls `FUN_0045c9ab(param_1)` for (re)load; `0x45C9AB` calls `0x4432C1` with its own `this` (`0x45ca63..69`). XREF callers: `0x4432C1`<-`0x45C9AB`<-`0x45CFF9`<-`0x450696` (all single caller). |
| c. Grid filled before gates | DECOMP `0x4432C1` order: `0x42FE1F` ... `0x439FD3(this+8, stream, 0x444400)` ... `0x437D48` ... `0x448BD0` (gates). `0x439FD3`: `0x43AA63` sets height `[grid+0x418]` and sizes `list(+0x41C)`; `0x43B640(+0x41C, stream, 0)` bulk-reads `n` dwords into the array (DECOMP of `0x43B640`: `n = u32; resize; 0x43B5B0(stream, data, n<<2)`). `0x43AA91` writes only name strings. |
| d. Receiver mutates the filled grid | Reader `0x42ACAC` ASM `0x42af32..0x42af3d`: `push 0x200; mov ecx,[ebp-0x1d0] (this); call 0x43127e` (request 1) / `0x42af98..0x42afa3` SET (request 0). Rect = stream AABB (reader does **not** recompute `[+0x68]`). |
| e. Nothing in the rest of load re-writes the array | (i) direct-reference list for `0x4C6A10` has no site in `0x445046..0x4516FC` (loaders `0x4487F0..0x448E70`, post-load `0x444417`, `0x4448EA`); (ii) `0x444D1E` (after `0x4432C1`, sets state 2 "Level started") touches the grid only through `FUN_0040baa0(0x4c6a10)` (a height read); (iii) gate post-load hook `0x42B0B8` (vtable slot 7, called by `0x444417`) reads the height only; (iv) `0x444417 -> 0x43A459(grid)` walks 32 lists at `+0x208`, not the cell array; (v) the painters have exactly 4 callers (set: `0x428A47, 0x42ACAC, 0x42B24D, 0x43119B`; clear: `0x42ACAC, 0x42B24D, 0x43119B`); `0x43119B` stamps `0x80` only; `0x42B24D` is the ramp step (timing). |
| f. Reload | Every (re)load re-enters `0x4432C1`; it re-reads the pristine file array (c) before re-running the readers (d). |

**Remaining missing edges (precise).** (1) `0x428A47` (7,927 bytes, 8 stack args, "Fluid"-named; older notes say it stamps 0x200/0x2000/0x4000 by kind) has **no resolved callers and no data references** (`xrefs` empty) and is not in vtable `0x4B1020`; it is not shown to run in the retail load path. A computed invocation would not appear in static references. (2) Tagged/named object readers and `vt+0x28` hooks other than the lever's were not decompiled for **direct** cell stores through `0x40DE60` (those would not appear as painter callers); the reference-list gap in (e)(i) covers the 18 gadget loaders and post-load but not those readers. Neither contradicts the result; the clone's own stored-grid fact below (every `0x200` cell in the file lies in a gate rect) is consistent with the file grid being authored with the gates full and the readers adjusting it.

## Proof chain 2: exact traversal

ASM `0x4311D0` set (`0x4311D0..0x43127B`) and `0x43127E` clear (`0x43127E..0x43130B`), excerpt in static-trace H:

* **Conversion**: every `fld double` is followed by `call 0x49C5AC` (`FSTCW; or ah,0xC; FISTP qword; FLDCW`): truncation toward zero. Only EAX is used. A C++ `static_cast<int>(double)` matches; rounding would not.
* **x**: `x = ftol(+0x48)`; loop test `cmp x, xmax; jge exit` -> exclusive. Set only: `xmax = ftol(+0x60); if ((double)xmax < [+0x48]) xmax += 0x100` (`0x4311EE..0x431207`) so a rectangle that wraps the seam is painted across columns 255->0 by the later `& 0xFF`. Clear has no such fix-up (a wrapped rect clears nothing). None of the 19 MM_W_012 rects wraps.
* **y**: `y = ftol(+0x50)`; test `cmp y, ftol(+0x68); jge exit`, the bound recomputed each iteration -> exclusive.
* **Lookup** `0x40DE60`: `x &= 0xFF`; `y >= [grid+0x418] -> height-1`; `y < 0 -> 0`; `index = x + (y << 8)`, element size 4 (`0x404D10`). Columns wrap, rows clamp.
* **Order**: outer x, inner y (column-major); for set/clear of a rectangle with no overlap the order is irrelevant.
* **Call-site receivers (ECX = the gate, `this`)**: reader `0x42af3d`/`0x42af5e` (clear), `0x42afa3`/`0x42afc4` (set); step `0x42b395`/`0x42b3b6` (set, req 0, AABB after `[+0x68] = [+0x50] + h`), `0x42b3fd`/`0x42b41e` (clear, req 1, **current** AABB before `h` is reduced), `0x42b4ab`/`0x42b4cc` (set, req 1 after reduction, only while `h > low`). Each is `mov ecx,[this-slot]` immediately before the call.
  The Ghidra pseudocode's missing `this` in the reader's clear call (`FUN_0043127e(0x200)`) was a decompiler omission; the ASM shows ECX loaded. (Resolves the earlier INFER.)
* **Step details newly established (ASM)**: the req-1 settle branch (`0x42b4d6..0x42b574`) sets `h := low`, dispatches the `+0x12c` list once, sets `armed`/`done`, and does **not** paint or update `[+0x68]`. After load a drained gate keeps `[+0x68] = 28.5` until a type-4 raise recomputes it. The first step after load for a request-1 gate is therefore a no-paint settle.

## MM_W_012 table (MEASURED: `gate-cells-MM_W_012.txt`, traced traversal over installed bytes)

All 19 `entities_42acac` objects were evaluated (16 lever-targeted + 3 untargeted). Kind 0 (bit `0x200`) for all; grid 256 x 96. "Flagged before" = cells in the visited set whose stored word has `0x200`.
The reader applies CLEAR when request = 1, SET when request = 0.

| Gate [idx] id | Cols | Rows (visited) | Request | Visited | Flagged before | Changed at init | Settled cells after type 4 (SET to `min.y + high`) | Targeted by |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| [0] 22 | 16..23 | 4..27 | 1 | 192 | 192 | 192 | 192 | lever (72,34) |
| [1] 21 | 24..31 | 4..27 | 1 | 192 | 192 | 192 | 192 | lever (72,34) |
| [2] 20 | 32..39 | 4..27 | 1 | 192 | 192 | 192 | 192 | lever (72,34) |
| [3] 19 | 40..47 | 4..27 | 1 | 192 | 192 | 192 | 192 | lever (72,34) |
| [4] 18 | 48..55 | 4..27 | 1 | 192 | 192 | 192 | 192 | lever (72,34) |
| [5] 9 | 56..63 | 4..27 | 1 | 192 | 192 | 192 | 192 | lever (72,34) |
| [6] 8 | 96..103 | 4..27 | 1 | 192 | 192 | 192 | 192 | lever (79,60) |
| [7] 7 | 104..111 | 4..27 | 1 | 192 | 192 | 192 | 192 | lever (79,60) |
| [8] 12 | 120..127 | 59..80 | **0** | 176 | 176 | **0** (SET on already-set cells) | 176 | none |
| [9] 11 | 128..135 | 59..80 | **0** | 176 | 176 | **0** | 176 | none |
| [10] 10 | 136..143 | 59..80 | **0** | 176 | 176 | **0** | 176 | none |
| **[11] 6** | **152..159** | **4..27** | 1 | **192** | **192** | **192** | **192** | **lever (183,60), face -1 list** |
| **[12] 5** | **160..167** | **4..27** | 1 | **192** | **192** | **192** | **192** | **lever (183,60), face -1 list** |
| [13] 4 | 200..207 | 4..27 | 1 | 192 | 192 | 192 | 192 | lever (189,35) |
| [14] 17 | 208..215 | 4..27 | 1 | 192 | 192 | 192 | 192 | lever (189,35) |
| [15] 16 | 216..223 | 4..27 | 1 | 192 | 192 | 192 | 192 | lever (189,35) |
| [16] 15 | 224..231 | 4..27 | 1 | 192 | 192 | 192 | 192 | lever (189,35) |
| [17] 14 | 232..239 | 4..27 | 1 | 192 | 192 | 192 | 192 | lever (189,35) |
| [18] 13 | 240..247 | 4..27 | 1 | 192 | 192 | 192 | 192 | lever (189,35) |
| **16 targeted** | | | | **3072** | **3072** | **3072** | **3072** | |
| **ids 5 + 6** | | | | **384** | **384** | **384** | **384** | observed lever |
| 3 untargeted | | | | 528 | 528 | 0 | 528 | |

Raw offsets for the arithmetic (all MEASURED from the same file): rect min = (x0, 4), max = (x0+8, 28.5), stream height 24.5 overwritten by low 0 at load for request 1; row count = `ftol(28.5) - ftol(4) = 28 - 4 = 24`; cells = 24 x 8 = 192.

**192 versus 200 reconciled.** The earlier driver enumerated the geometric window `y in [4, ftol(4+24.5)] inclusive` = rows 4..28 = 25 rows x 8 = 200 cells and found 192 flagged "in rows y < 28". That is exactly the binary's visited set: rows 4..27. Row 28 is **never visited** by set or clear. The 8 extra cells per gate were an artifact of the inclusive enumeration. The driver's `inclusive-window` column (200 per gate, 3200 total, 3072 flagged, 0 flagged outside the settled set) reproduces this.

**Whole-grid context (MEASURED).** The stored grid has exactly **3600** cells with `0x200` and 0 with `0x2000`; all 3600 lie inside the union of the 19 rectangles (3072 + 528 = 3600, rectangles disjoint, overlap 0). After the load-time reader pass the live grid therefore has **528** `0x200` cells (the three request-0 gates) and no other.
The clone starts with all 3600 flagged.

**Inherited predictions.** *3072*: confirmed (16 x 192, all of them flagged-before and cleared). *384*: confirmed for the settled endpoint of a type-4 message to ids 5 and 6 (2 x 192; a SET on cells that init cleared). What changes is the justification (exclusive bound, instructions) and the note that the three untargeted gates are request-0 and cause **no** init change.

## Corrections to earlier claims (earlier files unmodified)

| Earlier claim | Correction |
| --- | --- |
| lever-links report "painter's y upper bound inclusive (rows 4..28) or exclusive: unresolved" | Exclusive for x and y in both painters; row 28 is never touched. |
| lever-links "clear call `FUN_0043127e(0x200)` ECX carried over; INFER" | ASM shows `mov ecx,[this]` before each painter call in reader and step (all 8 sites). |
| lever-links Proposal item 2 "gated on the unresolved ordering question" | Ordering resolved: file array is bulk-read into `0x4C6A10+0x41C` (`0x43B640`) before the gate reader runs. The residual gaps (`0x428A47`, un-decompiled object readers) are listed above. |
| lever-links "gate coverage: 16 gates' init" | The reader also runs for the 3 untargeted gates; they are request 0 (SET over already-set cells): no net change. |
| static-trace G "grid exists before the gate reader: INFER from call order only" | Now ASM/DECOMP-backed dataflow (proof chain 1 c-d). |
| lever-links "clone as shipped starts the gates raised where the original starts them drained" | Still true for the 16 request-1 gates; additionally the three request-0 gates are raised in both. |

## Implementation-ready scope (independent of parked timing)

**Title: apply the gate readers' load-time zone-bit pass to the clone's live zone mask.**

* **Established behavior it implements**: for each `entities_42acac` object in stream order, with kind 0 (bit `0x200` -> `Terrain::zoneWater`) and kind 1 (bit `0x2000`, no clone plane exists), run the reader's pass over the stored AABB exactly as in proof chain 2:
  request 1 -> clear `0x200` over `[ftol(min.x), ftol(max.x)) x [ftol(min.y), ftol(max.y))`; request 0 -> set (with the set-only x wrap fix-up). No ramp, no `dt`, no message dispatch, no `[+0x68]` recompute.
* **Production files**: `src/sim/RetailLevel.cpp`/`.h` (read `level.gadgets.entities_42acac` fields `values_18c_128_168`, `double_bits_48_60`, kind), `src/sim/Terrain.cpp`/`.h` (new bulk `applyGateZonePass`-style helper calling the existing `Terrain::setZoneWater` with x `& 0xFF`, y clamp; `setZoneWater` currently ignores `y >= height_`, the original clamps - the installed rects are in range so either is equal here).
  Order: after `Terrain::fromCollisionGrid` copies the file bits (`Terrain.cpp:~370`), before the first world step.
* **Actual-level acceptance (MM_W_012, source-built, no original run)**: derive rects/requests from the level bytes; (a) before: `zoneWaterCells() == 3600`; (b) after: `== 528`, the 16 request-1 rects each go from 192 flagged to 0 (3072 cleared, including ids 5 and 6 = rows 4..27 x cols 152..167 = 384 cells), the three request-0 rects (cols 120..143, rows 59..80, 528 cells) stay flagged, row 28 and every cell outside the 19 rects are untouched (row 28 of the 16 rects was never flagged); (c) the reload of the same level restores the same post-pass state. Test via the public `loadRetailLevel`, not a mirror of the helper.
* **What it does NOT do**: it does not implement lever activation or any raise. Pulling lever (183,60) still changes nothing in the zone mask, so after this change the 16 drained gates stay drained forever while the original refills them over time. **That is a different divergence from the current one (always raised).** Whether trading one divergence for the other is acceptable is a product decision; there is **no demonstrated gameplay failure** that this fixes (targeted-play-001: easy 20/20 win is not effect evidence), and the visible consumer is only the fall-landing splat exemption (`Terrain::splatExempt`, `actor_landing.md`) and terrain-probe `0x200` rule.
* **Excluded (still parked/untraced)**: the activation ramp (`h += dt * rate`, rate 2.0 for kind 0) is parked with the base clock. A settled-endpoint assertion (192 cells re-set per gate after a type-4) does **not** authorize instantaneous activation, an arbitrary ramp, a stand-in clock or an "enough ticks" approximation. Message dispatch, `armed`/`done` bookkeeping, cool-down, facing gate and the visible surface/emitter (`0x42B0B8`, `0x441E26`, models) remain unimplemented unknowns. The `0x2000` plane is not needed by MM_W_012 (0 stored `0x2000` cells).

**Explicit dependency blocker for the rest** (full lever effect): the tick-to-time mapping that supplies `dt` (parked base-clock work); no part of the activation can be time-faithful without it. The state machine and endpoints are known (lever-links section 3 + this report's step details), the producer of `dt` is not decided.

## Verification and limits

* Registered measurement `leverinit_measure` (source-built, read-only) was added to `CMakeLists.txt` next to `leverlinks_measure`; it asserts only measurement integrity (level readable, 16 targeted gates, ids 5/6 present). No existing assertion, timeout or test was changed.
* Registered verifier `python tools/agent_control/verify.py --profile all`: see "Verifier result" below (filled after the run).
* Static REA: registered target only, Ghidra provider, session closed with `close_binary` and no snapshot arguments. The original was not executed; the only artifacts consulted are the installed `.LVL` bytes already read by the repo's own asset library.
* Source tests and measurement rows are not original-fidelity proof; the binary-fidelity claims rest on the cited instructions.
* Unknowns: `0x428A47` reachability; direct cell stores in un-decompiled tagged/named readers; `dt` unit and producer (parked); render/emitter effects (`0x42B0B8`, `0x441E26`); consumers of bit `0x2000`; HIST note: an older record says five levels' file grids matched the live process byte-for-byte, which would need re-checking against any request-1 gate level before being cited as evidence for or against the load-time clear.

## Verifier result

Worker run of `python tools/agent_control/verify.py --profile all` (main profile first; labelled **worker**, not supervisor):

* Configure + build succeeded (1,324-step graph; the new `test_leverinit_measure` compiled and linked).
* ctest `--parallel 1`, 138 registered tests. At the time this report was last edited the log (`C:\progs\msys2\tmp\claude\...\tasks\bl0loylqs.output`) showed **tests 1-84 Passed**, including `leverlinks_measure` (28.80 s) and the new `leverinit_measure` (0.07 s), and test 85 (`scenenestedimpl_after`) still running.
* The foreground call hit its 600 s limit and the harness moved the run to the background, where it was **killed at the background time limit** (task notification `bl0loylqs`, status killed) during test 85 (`scenenestedimpl_after`). It was not restarted (it was already at the longest allowed timeout). I therefore do **not** claim that the full main profile (138 tests), or the recovery profile (not reached; `verify.py --profile all` runs it after main), passed.
* The supervisor's independent registered verifier run is the authority for full pass/fail. The previous task (lever-links-research-001) recorded the same 10-minute wall at test 84 on its first attempt; the long-running tests earlier in this run were `targetedplay_play_1` 167.76 s and `levelreturn` 238.65 s.
* Nothing else changed in tests, assertions, timeouts or production code, so the unrun remainder is expected to be unaffected, but that expectation is not a result.
