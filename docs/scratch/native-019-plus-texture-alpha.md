# Plus-texture sibling alpha loading (native-019)

2026-10-30. Implementation handoff: **needs_review**, subject to the enclosing
supervisor's full verification and separate read-only acceptance. This worker
does not edit the queue. No original executable or original-byte oracle ran.
Full verification is currently blocked by the repeated `levelreturn` timeout
described below; neither implementation acceptance nor a full-profile pass is
claimed.

## Runtime change and traced consumers

`src/assets/Bitmap.cpp::loadTexture` now sends both `AlphaMode::AlfFile` and
`AlphaMode::Embedded` through its existing sibling `.ALF` read and
`applyAlfAlpha` branch. Before this change, plus-prefixed 24-bit textures kept
opaque alpha, and plus-prefixed 32-bit textures kept their BMP alpha. Afterward,
the returned RGBA contains the sibling plane in top-down pixel order.

The inspected application paths are:

- `src/app/main.cpp::runApplicationLeg` -> `RetailSkillPanel::create` ->
  `loadTexture(library, sheetAssetName(frame.sheet))`. The returned image is
  moved into `panel.icons_`. `drawRuntimeSkillPanel` -> `RetailSkillPanel::draw`
  -> `drawSkillPanelIcon` reads `sheet.rgba`, including `src[3]`, into the
  destination image. The focused test feeds an archive-loaded plus image into
  this unchanged icon draw and the existing `compositeIconRegion` primitive.
- `main.cpp`'s scheduled scene frame -> `TubeScene::render` ->
  `drawInjectedWalkers` -> `prepareInjectedWalker` -> `loadTexture` ->
  `PreparedInjectedWalker::texture` -> `drawActorSprite`. That consumer reads
  all four texture bytes, then applies its existing alpha-test/composition
  policy. Its manually constructed `Embedded` overlay remains unchanged.

These establish reachable shared loader consumers. They do not establish that
either route selects the retail `+SHADOW_NEW` texture, or that a retail frame
has been repaired.

## Retained instruction evidence and portable boundary

Reference: repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
confirmed by reading bytes only. Read together: the unchanged
[texture creation note](../../decomp-lemmings/docs/evidence/texture_creation.md),
[assembly](../../decomp-lemmings/docs/evidence/texture_creation.asm),
[metadata](../../decomp-lemmings/docs/evidence/texture_creation_metadata.json),
[recovered source](../../decomp-lemmings/src/texture_creation.cpp) and header.
The metadata's old external installation paths are historical provenance; this
task reads the repository-local reference and archives.
Retained REA evidence IDs are
`ev_a16d1fc63069e026e0f908736e8f283746add118a6469bbf8610a1eb503aab7a`
for the factory and
`ev_f8c9a925e461d91e3d308f0c2ccc2db3f8db06890754cd80c9415ac8847411be`
for the constructor. This retry applies the reverse-engineering skill's
evidence distinctions to those existing records; it opens no new REA session.

Factory `0x004743DD` compares the basename byte to `0x2B` at `0x00474480`
and stores flag `0x10` at `0x00474485`. Constructor `0x0047218A` reads its
captured flags at `0x00472325`, masks `0x18` at `0x00472328`, and skips the
alpha branch only when zero (`0x0047232D`). The branch allocates width*height,
constructs the selected sibling ALF path, opens it, and reads width*height
bytes at `0x004723CA..0x004723EE`, before the call to graphics initialization
`0x00472F99` at `0x0047241A`.

Original x86 facts stay separate from portable `Image`: the factory is a
callee-cleaned one-stack-argument routine (`ret 4`); the constructor uses ECX
for `this` and two stack arguments (`ret 8`). The original allocation is 40
bytes, with alpha pointer +0C, width +14, height +18 and flags +24. Original
area multiplication is 32-bit; unknown byte/word preservation and allocation
ordering stay in the recovered module. This change adds no x87 operations,
precision changes, native layout adaptation or CRT reconstruction.

Public enum identities, `alphaModeFor` and `alphaModeName` are unchanged.
`Embedded` / `"embedded"` remains the historical plus classification, also used
by manual overlays. Header comments now distinguish that label from archive
ALF loading and accurately describe direct 32-bit BMP alpha preservation.
The existing invalid-argument catch, BMP/envelope conversion, colour keys,
additive handling, quality selection, archive precedence and renderer code are
unchanged. Missing/short planes use the existing asset-qualified `expected`
errors; extra bytes remain ignored by `applyAlfAlpha`. Original fatal-open and
malformed-input behavior are not emulated.

## Authentic regressions and focused verification

Before production edits, the actual working-copy `Bitmap.cpp` and `Bitmap.h`
were copied to ignored `build/native-019/`, with initial hashes and preservation
manifests. Frozen identities:

- `Bitmap.cpp`: `ec15c793a5f294a0afb487760e4912be015247d0b36b17f34f4e6f7c8aa2c009`
- `Bitmap.h`: `79ff78acac35e0b07297469e568b72dd284064682a5b51d624e93363829eb411`

`loader_driver.cpp` builds separate old/current executables against those
frozen/current public loaders, `Box.cpp`, `Png.cpp` and the unchanged recovered
texture-envelope C++ source. Production files are never swapped. The source-only
drivers use literal complete RGBA expectations independent of a recovered
invocation, and check dimensions, mode and unchanged input archive bytes.

| Probe | Frozen result | Current result |
| --- | --- | --- |
| Odd-width padded bottom-up 3x2 24-bit BMP; ALF 0/17/64/128/200/255 | exit 1; 7 checks, one complete-RGBA failure, alpha all 255 | exit 0; 7 checks, zero failures |
| Bottom-up 2x2 32-bit BMP; BMP alpha 7/8/9/10, ALF 255/0/128/64 | exit 1; 7 checks, one complete-RGBA failure, alpha 7/8/9/10 | exit 0; 7 checks, zero failures |

`test_bitmap.cpp` retains all prior assertions (only the plus-classification
label is clarified). Added asset-free BOX tests cover raw BMP, codec 3 and
codec 4, complete dimensions/RGBA, nonzero odd-width padding, bottom-up RGB vs
top-down nonuniform alpha, 32-bit alpha replacement, missing/short/extra ALF,
and patch-only BMP/base ALF versus base BMP/patch-only ALF. Controls retain
caret loading, direct/plain 32-bit alpha, black/grey keys and additive pixels.
Fresh fixtures remain in the configured binary directory's ignored
`bitmap-test/run-*` directories.

`test_skillpanel.cpp` adds a fresh asset-free BOX under ignored
`build/skillpanel-test/`, loads a plus icon with alpha 0/128/255, and checks
literal complete destination pixels through both existing icon composition
functions. A manually constructed `Embedded` overlay control also passes.
The compositor formulas are existing project policies, not newly recovered
native blending.

The registered MinGW-w64 GCC (`C:/progs/msys2/ucrt64/bin/g++.exe`) built both
focused targets. Direct source-built runs passed:

| Existing target | Without retail environment | With repository-local retail data |
| --- | --- | --- |
| bitmap | 99 checks, zero failures | 114 checks, zero failures |
| skillpanel | 51 checks, zero failures | 79 checks, zero failures |

One intermediate test build failed because the added frame fixture used
`vBottom`; the actual existing field is `uBottom`. The fixture was corrected
and rebuilt successfully. The failure log is retained. There was no
evidence/source behavior disagreement. `git diff --check` exited zero; its
warnings about pre-existing installation-file CRLF remain untouched to preserve
the installation boundary. All five task text files use LF.

The worker ran these narrower source checks to hand back early for verification.
The enclosing supervisor will run `python tools/agent_control/verify.py
--profile all` after worker exit; no full-profile pass is claimed here. Current
read-only CTest inventory retains 70 main and 84 recovery registrations,
`LEMMINGS_REFERENCE_EXE` is empty, and recovery's only `native_`-prefixed test is
the source test `native_int_array`.

## Read-only retail corpus measurement

Both public-loader drivers enumerate the selected archive overlay. Actual
corpus: **one** plus BMP, `TEXTURES\+SHADOW_NEW.BMP`, **16x16 / 256 pixels**.
Its base entry is 31 packed bytes; BMP and 256-byte sibling ALF both resolve
from `lemmings.box`; no patch BMP overrides it. Frozen loading disagrees with
the selected ALF at **256/256** alpha positions; current loading disagrees at
**0/256**. A direct byte comparison of both public-loader outputs confirms
**768 RGB bytes compared, zero differences**, and 256 changed alpha bytes.
No retail source assets were copied into the synthetic fixtures or added to Git.

Commands, source identities, actual exit codes, per-process timeout bounds,
elapsed times, stdout and comparison output are in ignored
`build/native-019/commands.jsonl`, `*-source-identities.json`, `old-*.log`,
`current-*.log`, `retail-comparison.json`, focused logs and `preservation.json`.
`run_checks.py old|current|compare|focused` reproduces the source checks using
registered tools. Compile timeouts are 120 seconds; driver runs are bounded at
30 seconds and focused executable runs at 60/120 seconds.

## Verification retry and remaining blocker

The first enclosing supervisor actually ran `verify.py --profile all` with
the registered toolchain. Its retained log is
`build/agent-runs/20261030T210313-40868-78db9398/verification.log`.
The main suite passed 69/70 tests and failed `levelreturn` at its existing
240-second per-test timeout (240.04 seconds reported); total main test time
was 396.57 seconds. Verification exited 1 before reaching the recovery suite.
Those results replace the initial worker's pending-verification statement
above; they do not establish a full-profile pass.

This retry inherited the completed five-file implementation and authentic
frozen/current reports. It inspected them without replacing either frozen
source or altering production code, tests, consumers, CMake, verifier, queue,
timeouts or compiler settings. The isolated registered CTest command
`ctest --test-dir build/agent-native/main -R ^levelreturn$ --output-on-failure
--no-tests=error --parallel 1` again timed out at 240.03 seconds and returned
8; the bounded recorder completed in 240.058 seconds (outer limit 255 seconds).
The retry does not establish why that test exceeds its limit or attribute it
to the alpha change. No further timeout retries or unrelated fixes were made.

The same registered GCC/CMake configuration then successfully checked the
existing `test_bitmap` and `test_skillpanel` targets (no rebuild was needed).
Fresh source-built runs again passed: bitmap 99/114 checks without/with retail
data, and skillpanel 51/79 checks, all with zero failures. Each executable was
bounded at 120 seconds. New logs, commands, process exits, elapsed times,
CTest JUnit and preservation snapshots are separate from the original reports
under ignored `build/native-019/retry-2/`; reproducer: `retry_checks.py`.

The retry's initial manifest distinguishes the controller's `native-019`
lifecycle updates from the original attempt: all other queue entries are
identical to the initial native-019 snapshot. All production/test source bytes
are retained from that implementation; only this finding note is updated in
the retry. No installed file changes or original execution occurred.

### Third worker handoff

The second enclosing supervisor also ran the full registered verifier; see
`build/agent-runs/20261030T211909-40868-ac821ec2/verification.log` and
`result.json`. Again 69/70 main tests passed, with `levelreturn` timing out
at 240.02 seconds (394.65 seconds total main test time). Verification exited
1 before running recovery tests. Together with the isolated retry above,
this is a repeated verification blocker; its cause remains unknown.

The third worker preserved production and test source bytes and rebuilt both
public-loader drivers into a new ignored `build/native-019/retry-3/` directory.
Both frozen literal RGBA regressions again failed exactly one of seven checks
(exit 1); both current regressions passed all seven (exit 0). Fresh retail
measurements again found one 16x16 plus BMP, 256 old alpha differences versus
zero current differences against ALF, and zero differences across 768 RGB
bytes. The focused registered build and all four source test runs passed:
bitmap 99/114 and skillpanel 51/79 checks without/with retail assets.

The local reference bytes also match the retained metadata's procedure byte
ranges and digests, including the factory and constructor; this is read-only
static evidence, with no original instructions executed. The new directory
retains source identities, exact commands, exits, elapsed times, per-process
bounds, stdout, retail comparison and preservation checks. Reproducer:
`build/native-019/retry3_checks.py` (phases `snapshot`, `old`, `current`,
`focused`, `finish`; `snapshot` intentionally requires a new output directory).
Only this note changes during the third worker turn. No additional isolated
`levelreturn` retry or full verifier was run by this worker: the enclosing
supervisor owns the next full verification after exit. Full-profile success
and separate acceptance remain outstanding.

## Preservation, unknowns and next action

Changed files: `src/assets/Bitmap.cpp`, `src/assets/Bitmap.h`,
`src/__test__/test_bitmap.cpp`, `src/__test__/test_skillpanel.cpp`, and this note.
Ignored reports are confined to `build/`. Initial working-copy hashes and the
staged binary diff distinguish this task from the pre-existing dirty checkout.
Preservation checks confirm all unrelated existing files, every queue entry,
recovery sources/headers, build registration, consumers, STATUS gates, recovery
PROGRESS/FUNCTIONS, digest debts, historical failures, native-006 deferral,
`372 / 6,186` baseline and D1/D3/digger parks remain unchanged. All 63 installed
files match their initial read-only hashes. No queue, supervisor, verifier,
tool registry, shared instructions or permission setting was edited.

Historical native comparison figures in `texture_creation.md` and FUNCTIONS
remain attributed and were not re-measured. Passing source tests establishes
adapter behavior, not original-game agreement. Loading this plane does not
recover graphics initialization `0x00472F99`, native upload/flag16 blending,
original malformed-input faults, allocation order, physical input, timing or
full-frame fidelity. No original executable, copied instructions, oracle,
hook, debugger, capture, launcher or installation modification is authorized
or used.

Next action: let the enclosing supervisor run the full registered verifier,
then have its separate read-only reviewer check every native-019 criterion
against the isolated change, authentic regressions, retail comparison and
preservation reports. If `levelreturn` still times out, stop for a separately
scoped decision on investigating that verification failure; do not change
its timeout or renderer inside native-019. Completion remains pending full
verification and separate acceptance.
