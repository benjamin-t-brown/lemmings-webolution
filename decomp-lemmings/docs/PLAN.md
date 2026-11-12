# Plan to a standalone build

Goal: an executable built from this source that plays Lemmings Revolution,
without the original `Lemmings Revolution.exe` at runtime. Asset archives
(`lemmings.box`, `patch.box`) remain required — that is true of every project
of this kind and is not a target to remove.

Matching the original compiler's output is not a goal.

## Where the project actually stands

Verification is solved. The hybrid harness gives a correctness oracle that
costs nothing per function: run the recovered routine beside the original
inside the real process, on real data. All four ABI hazards are closed with
evidence, and gameplay can be driven from a script. The remaining problems are
**volume** and **three subsystems with no recovery at all**.

| | State |
| --- | --- |
| Level format | Complete — all 105 installed `.LVL` entries parse to exact EOF |
| Sprites | Complete — 19 SPR resources, 132 clips, 1,526 frames, 7,354 pieces |
| Textures | Packed BMP envelope and Huffman/backreference decoder recovered |
| Resource lifecycle | Registration, cache, eviction, factory recovered |
| Actor simulation | Many slices recovered, with unrecovered dependencies exposed as callbacks |
| World update | `0x004423D2` orchestration recovered |
| **Rendering** | **Not started.** Strings and vtable bindings identified only |
| **Frontend state machine** | **Not started.** Screens, menus, level map, results |
| **Sound** | **Not started.** `SoundManagerPC` / DirectSound |

Documented coverage is 372 of 6,186 functions. That denominator is the problem
with planning from here, which is why step 0 exists.

## Step 0 is done; here is what it found

| Measure | Functions |
| --- | ---: |
| Eligible per the old counting rules | 6,186 |
| Minus unwind/catch funclets (not functions) | −1,294 |
| Minus Ghidra FID-matched library routines | −57 |
| **Recoverable upper bound** | **4,835** |
| **Executed on a playable path** (frontend + `MM_DIG_000`) | **2,024** |
| Never executed | 2,811 |
| Already in the ledger *and* executed | 239 |
| **Remaining executed work** | **1,785** |
| — of which at or above `0x498000`, i.e. probably CRT | 208 |
| **Working target** | **~1,577** |

Coverage of the code a playable build needs is **11.8%**, not 6.0%. Details and
caveats in [PROGRESS.md](PROGRESS.md); the prioritised list is
[worklist.txt](benchmarks/worklist.txt), in first-execution order with each
entry marked `F` (frontend) or `G` (first seen in a level).

The headline is that **the project is roughly a third the size it looked**.
Two caveats that matter: 2,024 is a floor measured from one level with no
explosions, and the CRT boundary at `0x498000` is inferred from known anchors
rather than established.

Saturate the coverage curve before scheduling — trace several levels across
difficulty groups, with gadgets, skills and explosions, until new runs stop
adding functions. That number, not 2,024, is the real denominator.

## How Step 0 was measured (for repeating it)

Do this before committing to any schedule. `docs/PROGRESS.md` already concedes
that the recovered / partial / inspected / unknown split has never been
audited, and 6,186 includes statically linked CRT code. Three measurements,
each cheap, each cutting the number:

**0a. Exclude the CRT.** The build is Visual C++ 6.0 (linker 6.0). Signature-
match against period `LIBC.LIB` / `LIBCMT.LIB` and drop the matches from the
denominator outright. Expect several hundred. Nothing downstream should ever
plan to port them.

**0b. Reachability from the game entry.** Walk the call graph from the CRT
entry's call to `0x00450696` and mark what is reachable. Anything unreachable
is dead code, an editor leftover, or data misclassified as a procedure, and is
not work. Record the result rather than re-deriving it per slice.

**0c. Runtime coverage of a real play session — the important one.** Static
reachability over-counts badly; what matters is which functions actually
execute. The harness already has everything needed:

- Take the 6,186 entry addresses from the REA inventory.
- Write `0xCC` over each entry byte, keeping the original byte in a table.
- Install a vectored exception handler. On each `INT3`, record the address,
  restore the original byte, rewind `EIP`, and continue.
- Drive a level with `tools/drive_hook_session.py`, then dump the hit set.

Each function reports itself once and then costs nothing, so a whole play
session can be traced. The output is the set that a playable standalone must
contain, **in discovery order** — which is also a sensible porting order. Run
it separately for the frontend, the level map, gameplay, and the results
screen, and the four sets partition the work.

Deliverable: a machine-readable target list with, per function, whether it is
CRT, reachable, executed in gameplay, and already recovered. Everything below
is scheduled against that list, not against 6,186.

## Phase 2: bulk port behind the harness

Five tracks. The harness makes them independent, so order by what unblocks the
standalone cutover rather than by subsystem tidiness.

**Track A — simulation.** Finish actors, gadgets, collision, the world update
and its virtual dispatch. Most advanced track, and the one the hook oracle
verifies best because gameplay calls these routines hundreds of thousands of
times per minute. Each routine: port from decompiler draft, hook in `shadow`,
drive a level, promote to `replace`. The immediate next item is `0x00430F4D`
(gadget actor-ID collection), now unblocked by the allocator indirection.

**Track B — assets.** Mostly done. Remaining: resolve recovered level
resource names to loaded resources, finish auxiliary records currently retained
as raw serialized bytes, and models (`Models\%s`), structures
(`Structure\%s.tga`), cylinder textures (`Cylinder\%s\%s_%d_%d`) and language
files (`Language\%s.ltx`).

**Track C — rendering. The largest unknown and the critical path to Phase 3.**
Two routes, and the choice should be made deliberately and early:

- *Port the software rasteriser.* `HardwarePC.cpp` carries
  `No suitable 3D accelerator detected` and four numbered
  `Out of VRAM - Switching to software mode (N)` strings. If that path is
  self-contained, porting it yields pixel-comparable output with no Direct3D
  dependency at all, and frames can be diffed against the original. Best
  outcome, unknown cost — **trace it first and find out.**
- *Replace with a modern renderer.* Keep only the geometry and texture
  pipeline already recovered (`0x47A190` rectangles through `0x414920`
  submission) and draw with something current. Cheaper and certain, but gives
  up pixel equivalence and makes "is this correct?" a judgement call.

Decide this before investing in either. It determines how much of
`3DEngine` needs porting at all.

**Track D — frontend.** Screens, the five-group level map, results, save
games, fonts, language selection. Unrecovered here, but the sibling remake
project has done substantial work on exactly this: screen ids (`0x004C416C`),
the level record table (`0x004E35D4`, 5 groups x 25 slots x 0x118), screen
vtables, and the path builder to `0x004CBBF8`. Those findings cite original
addresses, so they can be **re-verified cheaply against the executable**
rather than re-derived — which respects the rule that the remake is context,
never evidence.

**Track E — sound.** Smallest and most replaceable. `SoundManagerPC` drives
DirectSound over `music\lemmingNN.wav` and SPR-adjacent effects. Porting the
mixer logic has little value; recover enough to know *what* is played *when*,
then drive a modern audio backend.

## Phase 3: the standalone cutover

The hybrid is not an end state — it is scaffolding that gets removed from the
inside out.

1. **Shrink the hook set until only platform code is hooked.** When the only
   detours left are rendering, input, sound and file I/O, the simulation is
   effectively yours already.
2. **Write the platform layer.** `WinMain`, window, event loop, the renderer
   chosen in Track C, input, audio, save-game paths. This is new code, not
   recovered code, and should be written as such — do not port
   `FilePC`/`HardwarePC`/`SoundManagerPC` just to replace them later.
3. **Link standalone and boot to a level.** First standalone milestone is not
   the frontend: it is loading one compiled level and simulating it with
   rendering. The frontend can stay stubbed well past that point.
4. **Retire the harness.** Keep it — it stays the best regression oracle you
   have for as long as the original executable exists.

Do not port the four `.avi` files' playback. They are ordinary video; decode
them with any current library or skip them.

## Honest effort

Comparable projects, all with multiple contributors: OpenRCT2 took roughly
three to four years from DLL-injection to standalone; devilution about two
years to a complete decompilation; Super Mario 64 about two years with a large
team and far better tooling on a far smaller binary.

This is a ~6,000-function 3D game. The useful projection is rate-based rather
than a guess, so **measure the rate** from the ledger's own history and
project against the Step 0 target list:

Step 0 has now fixed the denominator at roughly **1,577** for the measured
playable path, so the projection is against that rather than a guess:

| Sustained rate | 1,577 (measured floor) | 2,500 (if more levels saturate higher) |
| ---: | ---: | ---: |
| 25 / week | ~15 months | ~23 months |
| 50 / week | ~7 months | ~12 months |
| 100 / week | ~4 months | ~6 months |

The harness should raise the rate, because verification no longer costs a
hand-written oracle per routine. The remaining uncertainty is no longer the
denominator but **how far the coverage floor rises** once more levels, gadgets
and skills are traced, and how much the per-function cost grows as the work
moves from leaves into coupled state.

## Risks, worst first

1. **Rendering is unbounded until traced.** It is the critical path to a
   standalone build and the only track with no recovered code. If the software
   rasteriser turns out to be entangled with DirectDraw surface management,
   Track C becomes a rewrite rather than a port.
2. **The target list could be much larger than hoped.** Step 0 could reveal
   3,000+ reachable, executed functions. Better to know in a week than in six
   months.
3. **Per-function cost rises with coupling.** The routines recovered so far
   are disproportionately leaves. Deeper ones touch more unrecovered state, and
   `shadow` mode is harder to apply to routines that mutate shared structures —
   `0x0041BC10` already needed a deep-copy comparison rather than a direct one.
4. **No RTTI, so no class names.** Every type is inferred. The `Integral`
   versus `Source` split and the `*PC` platform suffix are the only structural
   facts the binary gives up, and they came from assert strings.
5. **Drift between the hybrid and a standalone build.** Code verified under
   the original's FPU control word, heap and struct layouts may behave
   differently once those are yours. Keep the offline unit tests; they are the
   only check that survives the cutover.

## Next three actions

1. **Saturate the coverage curve.** Trace several levels across difficulty
   groups, with gadgets, skills and explosions, until new runs stop adding
   functions. Replaces 2,024 with a real denominator.
2. **Settle Track C.** The whole texture and render chain executed, including
   `0x472F99`, which the recovery map still lists as unrecovered — so the
   hardware path is traceable now. The software rasteriser did **not** execute,
   because dgVoodoo supplies 3D. Force the fallback (the strings at
   `0x004C2284` and `0x004C2310` say the engine chooses it when no accelerator
   is present) and trace that separately: the resulting function set *is* the
   cost of the software-rendering option, measured rather than guessed.
3. **Establish the CRT boundary** above `0x498000` so those 208 executed
   functions can be excluded with confidence instead of by inference.

Then resume Track A against the work list, in execution order, starting with
`0x00430F4D`.
