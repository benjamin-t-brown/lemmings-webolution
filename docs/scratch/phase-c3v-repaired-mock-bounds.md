# C3v — repaired draft mock bounds; live preflight still incomplete

2026-10-25. Source-only VM tests, not stock execution or observed device state.
Packet `build/tmp/c3v-state-format-preflight/round-note.md`. Original five files
preserved byte-for-byte under `before-repair/`; their original mutation/safety
headlines remain **rejected**, not retroactively accepted by this repair.

## Accepted repair scope

Draft remains disabled. It charges one **attempt** before runtime, pointer or
getter work, separately limits successful samples/reports and counts errors.
It checks Windows/ia32/pointerSize4 plus readable device/vtable/slot ranges and
executable function ranges before constructing a callable test double. Failed
HRESULT produces explicit hr/value:null, no successful sample, no retry storm.
These are mapping/bounds checks only: **not COM interface/type validation**.
No surface descriptor query, Lock, QI, setter/Blt/GetDC, launcher or live call.

## Facilitator check

`node build/tmp/c3v-facilitator-repair-check.js` supplies a separately written VM
fixture without importing worker mocks. It verifies all32 synthetic calls:
19 render selectors then13 stage0 selectors, exact this/selector/out argument
order, stdcall/int signatures, selected stub identity and returned output words.
Seventeen failed-path fixtures test null/FFFFFFFF/misaligned/unmapped values,
unreadable/short ranges, non-executable functions, throwing getters, HRESULT
failure on call1 and call20, stack and runtime rejection. Each failed attempt
produces exactly one error/report over ten draws; invalid pointers reach zero
NativeFunction constructors/calls. Recursive entry adds no nested attempt.
Attach failure emits no attached marker. Missing Process is independently
verified to reject installation rather than treated as a successful sample.

The original facilitator failure fixture predates Process guards, so it now
cannot reach its old draw loop; its incompatible run is **not repair evidence**.
The new fixture replaces that coverage explicitly, preserving the old file.
Worker's two Node tests rerun pass and supplement this independent coverage.

An actual private source mutation changes only GetRenderState slot21→22. The VM
executes it, reaches the wrong callable stub, and the same independent behavioral
ABI assertion fails; original source executes successfully afterward. On-disk
SHA256 remains `f3625cdf8decb423e86836697cf3f4333581d52660aea04dd4774808a249daf8`.
All preserved original hashes match. The first independent hash check treated
sha256sum's `*` marker as a filename; corrected checker parsing and reran all cases.

Independently hashed installed d3d.h and enumerated the Device3 method list:
GetRenderState index21/offset84, GetTextureStageState39/156, explicit DWORD output
parameters. This establishes **conditional header ABI**, not the live pointer's
identity. No original code/body read was added by V. Fresh wrapper build/CTest
**35/35 passed** on the same unchanged source snapshot as U (unaccepted sibling
X is present). Six install hashes unchanged; no Lemmings/live resources.

## Not accepted / next boundary

No completed or authorized live recipe. Still missing: source-class/frame/resource
correlation, actual Device3 route/interface compatibility, concrete texture-surface
interface provenance, validated real runtime/range semantics and attach/unload
cleanup coverage. Generic first-draw capture cannot close scene state; returned
format alone would not prove uploaded texel words. No request/default/score fit
may fill these. A separate bounded provenance/payload round is required before
any live brief; optional surface locking still lacks authorization.

C3 score remains0.2050523>0.02. Original dispatch14:53:56; repair15:05:07,
worker complete15:14:29; accepted15:22:34 MDT. Total **28m38s**, repair **17m27s**.
