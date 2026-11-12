# native-021: Lo scene tile preference

Prepared for `needs_review`. The worker leaves the queue unchanged; the
supervisor must run full verification and a separate read-only acceptance review.
No original executable or copied original instructions were executed.

## Production behavior and evidence

`src/app/main.cpp` lines 190-193 parse `--tile-detail low` into
`highDetailTiles=false`. `runApplicationLeg` passes that value to
`TubeScene::build` at lines 445-448; its presentation callback calls
`scene->render` at line 612. The default remains high detail.

The existing `loadBlock` in `src/app/TubeScene.cpp` still parses the original
tile reference with `parseTilePosition` before filename selection. Low detail
now appends `Lo` to that original reference and passes it to the unchanged
`levelTextureAssetName`. High detail still uses `levelTextureAssetNameHi`.
The selected quality candidate is checked with `AssetLibrary::contains`; only
absence selects the unsuffixed helper result. There is one `loadTexture` call.
Present-candidate read, packed-data, BMP or alpha failures retain their errors;
they do not cause an unsuffixed retry.

Each candidate uses the unchanged patch-over-base lookup independently:
base Lo beats patch plain, patch Lo overrides base Lo, and absent Lo permits
patch plain fallback. Successful loads keep the existing `toTexture`, original
position and insertion order in `outer_`/`inner_`; errors retain tile-qualified
diagnostics. `findTile` feeds the wall pass (lines 498-516), cell mesh pass
(lines 539 onward), and route `tileFor`/`drawRouteQuad` (lines 600-608).
The existing draw paths consume the retained texture. Naming helpers, accepted
names, rendering, public APIs and all high-detail behavior remain unchanged.
Actor sheets, frontend images and other texture callers are unaffected.

Before, low detail omitted Lo-only outer/inner tiles and chose the plain sibling
when both existed. After, the same public scene path loads and renders Lo.
These are source-derived scenarios, demonstrated with generated assets, not
observed retail-frame failures.

Authority is repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The retained `texture_creation.md`, `.asm`, `_metadata.json` and unchanged
recovered `texture_creation.cpp` were read together. Constructor `0x0047218A`
is 705 bytes. Instructions `0x00472221..0x00472265` inspect quality byte
`0x005CD5F2` and append the retained `Lo` string on zero quality.
`0x004722AC..0x004722FC` probe the chosen path, test the low result byte,
and restore the original name on failed open. The ECX-this/RET-8 convention,
resource layout, preserved unknown fields and allocator boundaries belong to
the recovered routine, not this portable filename adapter. No floating-point
arithmetic or x87 conversion is added. Read-only static identity checks match
the constructor and quality-predicate byte extents to metadata; see
`build/native-021/reference-static-check.json`. Historical original-byte
comparisons in the recovery evidence are retained, **not re-measured** here.

## Executed authentic regressions

Before production edits, `preserve.py` froze authentic working source under
ignored `build/native-021/TubeScene.before.cpp`, SHA-256
`3d3140978f8bdd27dda0ad59d1161ded836519f855e5ec2b62f61dcdaeef59c7`.
`TubeScene.current.cpp` matches production, SHA-256
`f9696e0cbe214ff82cf6e233fa175654b9d23e5100d64d14832de68cd28c2fe6`.
Neither frozen nor current snapshots replaced production files.

`python build/native-021/prepare_drivers.py` prepared the source-only drivers.
`python build/native-021/run_checks.py` compiled separate frozen/current
`TubeScene` objects and public-scene drivers using registered MinGW-w64 GCC,
C++23, `-O0`, and existing source-built dependencies. The driver reuses the
bounded synthetic BMP/BOX helpers and independently usable 7-ring/32-step
cylinder. Literal count, tile-qualified diagnostic and colour expectations are
independent of production candidate selection. Outer and inner banks render
separately in a small 128x128 3D wall view, with distinct Lo/plain colours.

| Source / expectations | Lo-only counts outer/inner | Preference pixels Lo/plain outer; inner | Exit |
| --- | --- | --- | --- |
| Frozen / before | 0 / 0 | 0/493; 0/450 | 0 |
| Frozen / after | 0 / 0 | 0/493; 0/450 | 1, ten expected failures |
| Current / after | 1 / 1 | 493/0; 450/0 | 0 |

Frozen Lo-only scenes report one missing plain BMP per original tile reference;
current scenes have no tile diagnostics and render 493 outer / 450 inner Lo
pixels. Optional `compose:` sprite diagnostics remain separate. These controls
demonstrate actual texture consumption without a new inspection API.

The added `testLowTileQualitySelection` runs before retail early returns in
`test_actorcompose.cpp`. It uses fresh directories beneath `build/native-021`
and synthetic bytes only. Both banks cover Lo-only, Lo preference, absent-Lo
plain fallback, patch Lo override, base Lo over patch plain, patch plain
fallback, both absent, and malformed existing Lo with valid plain siblings.
High-detail controls retain HI preference, plain fallback even when Lo exists,
and rejection of Lo-only as a high-detail fallback. Failed selected candidates
must also render zero pixels from the valid plain sibling. The pre-existing
explicit-low cases contain no Lo and remain unchanged.

Current asset-free actorcompose: **287 checks, zero failures, exit 0**, with
`WEBOLUTION_GAME_DIR` unset. The same test object linked against frozen scene
source: **287 checks, 24 expected failures, exit 1**. Thus new failures propagate
through the no-retail-assets early return. The original 104 asset-free checks
and all retail assertions/goldens remain verbatim after removal of the new
function/call and reversal of the fixture-directory parameterization.

`commands.jsonl` retains complete argument arrays, cwd, normal exits, elapsed
time and external failure bounds. Compile/link/driver bounds are 120 seconds,
the focused build is 180 seconds, inventory and retail CTest are 240 seconds.
Existing CTest properties/timeouts are unchanged. These bounds are failure
guards, not original-game timing evidence. `driver-identities.json` and
`source-build-identities.json` retain source, object, library and executable
identities. An initial PowerShell inline-Python setup command lost its string
quotes and failed with SyntaxError; file-backed Python then succeeded before
the freeze and production edits. No regression/build/inventory failure was
unexpected, and no evidence/source disagreement was found.

## Measured installed inventory and verification

The new inventory uses production `AssetLibrary::names` through
`levelFileNames`, overlay `read`, `findCylinderMesh`, `indexLevel`, `contains`
and `loadTexture`. It measures the effective installed levels, not raw archive
entries or all possible non-tile resources. Candidate origins come from the
same library's patch/base directory lookups.

- Effective levels: **102**; valid nonempty tile references: **4,896**;
  unique normalized tile references: **4,896**; invalid positions: **0**.
- Lo present: **0**; plain present: **4,896**; absent-Lo/plain fallback:
  **4,896**; both absent: **0**. These are newly measured Lo figures.
- Selected Lo from patch: **0**; selected plain from patch: **144**;
  selected plain from base: **4,752**.
- Selected production `loadTexture` successes: **4,896**, no errors.
  All **102** public low-detail scenes retain **24 outer / 24 inner** tiles,
  no diagnostics, and pass **204** separate small wall renders with nonblack
  pixels. This exercises rendering per bank/scene; it does not prove every
  installed tile pixel was visible or establish full-frame fidelity.

`retail-tile-references.tsv` records every candidate's presence, origin,
selected name/origin, load outcome and dimensions. `retail-scenes.tsv` records
public counts, diagnostics and render pixel counts; `retail-inventory.log`
retains the execution. There is no installed Lo variant in these references
to exercise; synthetic archives establish that branch. No frozen/current
low-detail image equality was imposed. This work does not claim a naturally
used default Lo variant or a repaired retail frame.

Existing retail high-detail actorcompose CTest passed: **348 checks, zero
failures**, 3.24 seconds (3.25 seconds CTest total). All nine golden image
expectations remain unchanged. Historical actor bbox/fidelity limitations
still appear in the retained log. Focused checks do not replace full verification.
`python build/native-021/final_checks.py` checks static identity, preservation
and CTest inventories: **70 main / 84 recovery**, empty
`LEMMINGS_REFERENCE_EXE:FILEPATH=`, `native_int_array` as the only recovery
`native_`-prefixed registration. The supervisor will run
`python tools/agent_control/verify.py --profile all` after worker exit; its full
result is pending here and is not claimed as passed.

## Preservation, limits and next step

Changed files: `src/app/TubeScene.cpp`, `src/__test__/test_actorcompose.cpp`,
and this note. No header change was needed. Ignored reports, authentic source
snapshots, task-only diffs, inventories, drivers and logs are in
`build/native-021`. Text edits use LF. The initial 1,639-file manifest covers
tracked working contents and installed files; final comparison permits only
the two intended existing-source edits. The supervisor's pre-worker source
manifest additionally checks untracked source/control/note files; its queue
digest is reproduced after reversing only its known dispatch update in memory.
Queue entries, native-006 deferral,
supervisor, verifier, registry, build registration, source inventory, bitmap,
archive, renderer and recovery code, STATUS.md, recovery PROGRESS.md, 372 / 6,186
baseline, historical failures and D1/D3/digger parks remain preserved.

Archive presence implements only absent-entry fallback. Original filesystem
open failures, drive preparation, callbacks, global quality-setting integration,
non-tile quality variants, GDI, graphics initialization `0x00472F99`, allocation,
device behavior, timing and full-frame fidelity remain unresolved. No game,
copied instructions, oracle, hook, debugger, capture, launcher or installation
modification was used. Next action: supervisor full verification, then independent
read-only acceptance review before the controller marks the task done.
