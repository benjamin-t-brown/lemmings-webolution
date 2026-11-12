# Production serialized SPR integration (native-008)

2026-10-30. Implementation status: **needs_review**, pending independent
acceptance. This worker does not edit the queue.

## Runtime change and traced callers

`src/app/main.cpp` routes `runApplication` through `runApplicationLeg`, both for
direct level entry and persistent frontend level legs. The leg calls
`TubeScene::build` (`main.cpp:429`), whose `TubeScene.cpp:174` path calls
`loadSpriteSheet` for `SPRITES\LEMMINGSHI.SPR`. A successful load populates
`lemmingSprites_` and sets `haveLemmingSprites_`; a failure records a `compose:`
error in `missing_`. Actor preparation/composition checks that availability flag
before selecting images from the sheet.

The simulation HUD path calls `RetailSkillPanel::create` (`main.cpp:748`).
`SkillPanel.cpp:208` passes the owned archive bytes to `parseSpriteSheet` and
returns an error if parsing fails. Its special icon sheets also use
`parseSpriteSheet` (`SkillPanel.cpp:244`). The parsed nested entries supply the
existing skill icon selection and texture lookup. These consumers are unchanged.

Previously, `Sprite.cpp` discarded each trailing block's tag and always read 16
triples. A valid tag-1 block containing 256 triples therefore failed exact EOF;
a tag-2 block without triples failed while reading. Both production caller paths
now receive the recovered tag-dependent parsing through the same public APIs.
These are source-derived supported-input regressions, not observed retail-frame
failures or a claim that an entire synthetic resource is drawable by the HUD.

## Retained evidence and compatibility boundary

Reference: repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The registered verifier checks this identity by reading bytes only.

Existing authority is the recovered [serialized resource note](../../decomp-lemmings/docs/evidence/sprite_resource.md),
[runtime reader note](../../decomp-lemmings/docs/evidence/sprite_runtime_read.md),
[assembly](../../decomp-lemmings/docs/evidence/sprite_runtime_read.asm), and
[bounded-string evidence](../../decomp-lemmings/docs/evidence/004432c1.md).
`0x00483EF0` / 44 bytes uses ECX `this` and `RET 4`, reads clips first, then
the tagged array at original object offset `+0x10`. `0x0047C810` / 43 bytes
returns 16 for tag 0, 256 for tag 1 and zero otherwise. `0x0047C643` / 101 bytes
uses that count with 24-byte triples. These original conventions and offsets
are evidence for ordering, not host C++ object-layout requirements.

`webolution_sprite_resource` contains only the existing `sprite_resource.cpp`
and `level_stream.cpp`, declared in `cmake/webolution_sources.cmake`. Root CMake
links it publicly through `webolution_objects` to both application and tests,
alongside the preserved `webolution_texture_decoder`. The recovery subproject
and oracle targets are not imported. Recovery sources and headers are unchanged.

`parseSpriteSheet` calls `lemmings::parse_sprite_resource`, then projects:

| Recovered serialized value | Existing public API |
| --- | --- |
| clip, frame, piece | Sprite, SpriteAnimation, SpriteFrame, preserving nested order |
| piece `values_00[0..3]` | bottom, top, left, right |
| piece `double_bits_10[0..3]` | uBottom, uTop, uLeft, uRight |
| piece `values_30[0..3]` | unknown0, unknown1, width, height |
| clip name / texture name | name / sheet from the bounded reader |
| clip `speed_bits_10` | rate via `bit_cast<double>` |
| populated tagged triples | palette vector, flattened in block order |

All binary64 transfers use `bit_cast` without arithmetic. Tags and empty block
boundaries are intentionally absent from this existing public representation;
calling the flattened vector `palette` does not establish that all tagged arrays
are rendering palettes. Clip and texture names follow capacities 128 and 260:
embedded NULs terminate the projected name, and exact-capacity declarations
consume the payload but expose at most capacity minus one characters. The
recovered reader's other bounded-name rules are retained unchanged.

Only the recovered parser call is inside the `invalid_argument` catch. It returns
an `expected` error; `loadSpriteSheet` retains its asset-name prefix. The caller's
input storage remains alive throughout synchronous parsing, and recovered strings
and vectors own their data before projection. Allocation failures are not caught
or emulated. Dimension helpers, texture-name construction, archive precedence and
renderer consumers are unchanged.

This serialized path does not construct the original runtime objects. The separate
runtime reader has x87 signaling-NaN quieting, allocation cookies, partial
initialization, an unknown-tag allocation/read-count mismatch, and texture IDs.
None of that behavior is imported or inferred here. The original layouts (clip
stride `0x98`, frame `0x10`, piece `0x48`) remain separate from portable vectors
and strings. Existing notes' original-byte comparisons are retained historical
claims; no original instructions were executed or compared during this task.

## Source checks and regression record

The extended existing `spritestate` test runs synthetic checks before its optional
retail branch. Little-endian input writers construct independent SPR/BOX bytes;
expectations use explicit integer fields, binary64 bit constants, names, counts
and ordered triple patterns, without asking the recovered reader for expectations.

Coverage includes three clips, multiple temporal entries and pieces, empty root
collections, an empty clip and an empty piece list. Every projected piece field,
name, clip speed and flattened triple component is checked. Negative-zero and
positive/negative quiet-NaN payloads are checked by integer bits. Names include
embedded NULs, empty strings and exact capacities 128/260 with a distinct final
discarded character. Mixed ordered blocks are `0, 2, 1, 0xFFFFFFFF, 0`, yielding
16 + 256 + 16 triples. Every strict prefix of the 7,699-byte populated resource
and a trailing-byte input must return an expected error. BOX-backed truncated
and trailing entries must return asset-qualified errors without leaking
`invalid_argument`.

CMake supplies `build/spritestate-test` beneath the source checkout as scratch
root. Each invocation atomically claims a fresh `run-*` directory; no existing
directory is reused or removed. Synthetic archives contain only generated bytes.
Retail archives are read in place, never copied into fixtures or modified.

Before the production change, the extended test exited 1: **7,744 checks,
9 failures**, with retail explicitly unavailable. Both archive regressions failed:

- Tag 1: `SPRITES\TAG1.SPR: consumed 396 of 6156 bytes -- the layout does not fit`.
- Tag 2: `SPRITES\TAG2.SPR: ran past the end of the file at offset 12`.

Other detected differences were mixed-block failure, old unbounded/NUL-containing
names, and acceptance of the 2,707-byte prefix under the old fixed-16 block
interpretation. After integration, the same asset-free test exited 0:
**10,290 checks, zero failures**, loading tag 1 with 256 triples and tag 2 with
zero triples. The check count increases because successful parsing reaches all
field/triple assertions. Before/after logs and initial file snapshots are in
ignored `build/native-008/`.

The current registered `spritestate` run loaded **19/19** available retail SPR
entries through `loadSpriteSheet`, retained the **15 states onto 15 sprites**
checks, and passed **10,340 checks** with zero failures. This is a current source
parsing result; the earlier evidence's installed comparisons remain historical.

The first full `python tools/agent_control/verify.py --profile all` attempt passed
69/70 main tests, with `levelreturn` timing out at its existing 240-second limit.
No assertion failure was printed; its captured output was still producing frames.
Recovery verification was not reached in that attempt. The failed log, JUnit and
CTest output are retained. A single full retry with the supported `--jobs 1`
option **passed**, exit 0: **main 70/70 (347.93 s)** and **recovery 84/84
(28.84 s)**, with the same tests/timeouts. `levelreturn` passed in 222.56 s.
Contention remains an unproven explanation for the first timeout; no code or
threshold was changed between attempts. The full pass uses the registry's
MinGW-w64 GCC toolchain and verifies the reference SHA-256 by reading bytes only.
Recovery retains an empty `LEMMINGS_REFERENCE_EXE` and only `native_int_array`
as a `native_`-prefixed source test, with no original-byte oracle registration.
`git diff --check` passed. Current inventories, JUnit, link edges, task-only diff
and preservation manifest are retained in `build/native-008/`. All 1,638 initial
files outside the five intended existing edits remain byte-for-byte unchanged;
this note is the sole new source-tree task file. No recovery implementation,
header, queue or installation file changed.

## Limits and next action

No repaired retail frame, original-game fidelity, runtime allocation equivalence,
texture registration or rendered-output acceptance is claimed. Malformed-input
rejection follows the existing portable parser policy; original failure behavior
is outside this integration. No original executable, copied instructions, oracle,
hooks, debugger, capture, launcher or installation modification was used.

Changed files: `src/assets/Sprite.cpp`, `src/assets/Sprite.h`,
`src/__test__/test_spritestate.cpp`, `CMakeLists.txt`,
`cmake/webolution_sources.cmake`, and this note. STATUS gate rows, recovery
PROGRESS/FUNCTIONS and the `372 / 6,186` baseline, native-006's deferral, other
queue entries and D1/D3/digger parks remain preserved.

Next action: the supervisor's separate read-only review must trace both caller
paths and check every native-008 acceptance criterion against source and actual
verification before marking the task done. Implementation remains **needs_review**.
