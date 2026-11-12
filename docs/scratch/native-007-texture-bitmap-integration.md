# Runtime bitmap-envelope integration (native-007)

2026-10-30. Implementation status: **needs_review**. Registered full verification
passed; independent acceptance remains pending. No queue status is edited by this worker.

## Missing behavior and application path

The production paths inspected before implementation are:

- `src/app/main.cpp`: `runApplication` / `runApplicationLeg` ->
  `FrontendSession::create` -> `loadTexture` for logo, background and font.
- `runApplicationLeg` -> `TubeScene::build` -> `loadTexture` for level tiles.
  `BackdropScene::renderStage` and the actor preparation path in `TubeScene.cpp`
  also use the same loader.

Previously, `src/assets/Bitmap.cpp` passed every selected archive entry to
`lrzDecode`. That API interprets the first byte as an envelope block count,
so a valid raw `BM` bitmap failed before reaching pixel conversion. The new
loader calls the existing `lemmings::decode_texture_bitmap`, accepting raw BMPs
and supported codec-3/codec-4 envelopes, then calls the existing `decodeBmp`
and alpha handling. Archive precedence, quality selection and renderer policy
are unchanged. `src/assets/Lrz.*` and its other callers are unchanged.

## Retained static evidence and adapter boundary

Reference: repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
See the retained [decoder note](../../decomp-lemmings/docs/evidence/texture_resource.md),
[assembly](../../decomp-lemmings/docs/evidence/texture_resource.asm) and
[metadata](../../decomp-lemmings/docs/evidence/texture_resource_metadata.json).

Within `0x0045015A`, instructions `0x00450233..0x0045025F` test bytes `0x42`
and `0x4D` and bypass envelope transformation. Supporting decoder spans are
`0x00467DE8` / 43 bytes, `0x00467E13` / 164, `0x00467EB7` / 310 and
`0x00467FED` / 135. These perform integer decoding; no x87 conversion is
introduced. The original wrapper has four cdecl stack arguments: input,
output, an unused third argument and decoded size. The draft fastcall
signature is not the ABI contract and is not adopted by this portable API.
No original object layout or field offset is changed here.

The existing recovered source/header remain byte-for-byte unchanged. The main
CMake source inventory declares that one source for `webolution_texture_decoder`,
a small static library linked publicly by `webolution_objects`. Application and
texture tests receive the same library transitively. No recovery subproject,
GDI layer or original-byte oracle target is imported into the application build.

The archive vector owns input while decoding. A local `vector<uint8_t>` owns
the decoded BMP through `decodeBmp`; the span adaptation does not outlive either
owner. Only the decoder call is inside the `invalid_argument` catch, which
returns the existing asset-qualified `expected` error. Allocation failures are
not translated or emulated. BMP errors and ALF handling keep their existing paths.

## Source-built regression and checks

`src/__test__/test_bitmap.cpp` constructs a literal 3x2 bottom-up 24-bit BMP
with nonzero row padding and independently literal complete RGBA expectations.
Synthetic BOX entries exercise raw `BM`, codec 3, codec 4, and two mixed blocks
that split the BMP header and ignore trailing envelope bytes. A distinct 1x1
base bitmap and a patched 3x2 bitmap establish precedence. A caret texture uses
a literal six-byte ALF plane, checked as a complete RGBA image. Malformed header,
unsupported codec, truncated codec-4 payload, raw truncated BMP and packed
truncated BMP checks call `loadTexture` and assert asset-qualified expected
errors without an escaping `invalid_argument`.

Fixtures are freshly created beneath the configured binary directory's
`bitmap-test/run-*` directories, retained for inspection. No retail bytes are
copied into fixtures. Existing BMP, sigil, ALF, PNG and retail assertions remain.

Before production integration, the extended source-built test exited 1 with
**60 checks, one failure**, exclusively `TEXTURES\RAW.BMP: loadTexture succeeds`;
the error was `TEXTURES\RAW.BMP: chunk 0 runs past the entry`. After integration,
the same test exited 0 with **63 checks, zero failures**. The extra three checks
are the now-reachable raw bitmap dimensions, complete pixels and alpha mode.
Both focused runs explicitly omitted `WEBOLUTION_GAME_DIR`.

Registered `python tools/agent_control/verify.py --profile all` exited 0 using
the registry's MinGW-w64 GCC toolchain: **main 70/70** (238.85 s) and
**recovery 84/84** (17.87 s), including bitmap, frontend/application integration
and installed texture payload checks. The reference SHA-256 was checked by
reading bytes only. Recovery kept `LEMMINGS_REFERENCE_EXE` empty and its only
`native_`-prefixed test was the ordinary source test `native_int_array`;
no original-byte oracle was registered. `git diff --check` passed.

Results and preservation checks are recorded in the ignored `build/native-007/`
report. Before/after build logs, regression logs,
initial status/hashes and original versions of the four edited existing files
are retained there. Full source verification is not a new original comparison.

## Limits and next action

The existing decoder note's original-byte comparisons are retained historical
evidence, not re-measured here. Its installed corpus reports packed BMPs. This
synthetic raw-BMP regression restores an original-supported input path; it does
not establish a repaired, previously failing retail frame. Malformed-input
rejection is portable adapter policy, not a claim about original faults.
Original allocation order/failure handling, GDI, graphics initialization
`0x00472F99`, device behavior and rendered-frame fidelity remain outside scope.
No original executable, copied original instructions, oracle, hooks, debugger,
capture or launcher is run, and installation files are not modified.

Next action: the supervisor's separate read-only review must trace these call
paths and check every native-007 acceptance criterion against source and actual
verification before marking completion. Native-006 remains deferred; STATUS
gate rows, the `372 / 6,186` baseline, historical failures and D1/D3/digger parks
remain unchanged.
