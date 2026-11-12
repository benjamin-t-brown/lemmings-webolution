# scene-roots-001: serialized scenery selection for MM_BASH_002

2026-10-31 UTC. **Research only; needs_review.** No production integration or
new native comparison is claimed. The independent acceptance review is pending.

## Milestone, defect and bounded result

Milestone step: selecting an installed level must reach a populated scenery
renderer during play. Concrete route: the clone's existing direct-level route
with `--level MM_BASH_002`, reading `COMPILED\MM_BASH_002.LVL` from the local
archive overlay. The source-traced failure is `BackdropScene::build` returning
a successful **empty** scene for every ID except `MM_CLIMB_002`. This is source
inference, not an observed retail-game failure or a newly executed clone render.

The static prerequisite is resolved for **MM_BASH_002**: its two scenery roots
are selected by loader order and counted serialized records, and both trees
are byte-identical to the corresponding first-level trees. Its separate
Entrance/Exit inline models also use supported node/mesh variants, with different
serialized placement. No second offset table or marker search is necessary.
The next bounded task can implement extraction and role-based rendering using
the existing recovered readers and renderer. Actual submissions, rendered output
and first-level non-regression remain that production task's acceptance work.

Accepted native-017/native-018 startup/selection, save-codec-001 completion/restart
wiring and native-022 cue routing are retained. Their reports were read alongside
the older `docs/STATUS.md`, whose opt-in startup, silent audio and in-memory-only
save descriptions predate those integrations. This task does not redo them.
Natural complete gameplay, timing and broader fidelity remain unverified.

## Identity, inputs and evidence classification

All executable addresses below are virtual addresses in repository-local x86
PE32 `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The executable was read as bytes only. Fresh SHA-256 checks matched this digest
and both entries in `build/save-research-input/reference.json`:

| Static export | SHA-256 |
| --- | --- |
| `reference-disassembly.txt` | `36bc7d4522fcfac733e73080246c43d85126463cc39d7f595fac0b72c8cff9b1` |
| `reference-headers.txt` | `2b25e3794add3210ffa0ca2fe2e1a277950c0ea27084210262c24dadb74adb4b` |

The headers identify i386, image base `0x00400000`, entry `0x0049D518`.
The exported assembly was inspected at the addresses cited below; strings and
historical pseudocode were treated as data, not instructions. Instruction
addresses and operand widths, rather than decompiler variable names, establish
the load/stage associations. No REA provider setup, download or new decompilation
was needed. Codex exec inspection is documented separately from Claude's shell
allowlist in `agent/README.md:192`; no permission change was made.

Retained static evidence reused:

- [Compiled-level composition](../../decomp-lemmings/docs/evidence/compiled_level_composition.md),
  [loader prefix](../../decomp-lemmings/docs/evidence/004432c1.md),
  [paired object readers](../../decomp-lemmings/docs/evidence/object_base_prefix.md).
- [C3u load/holder associations](phase-c3u-pause-scene-reachability.md),
  [C3y secondary slot8](phase-c3y-secondary-slot8.md),
  [C3bd stage integration](phase-c3bd-stage-order.md),
  [C3x strict scene reader](phase-c3x-scene-node-reader.md),
  [C3r resource conditions](phase-c3r-scene-texture-values.md) and
  [render frame map](../RENDER_FRAME_MAP.md).

Fresh **static asset inspection**, not a source-built render demonstration:
`build/scene-roots-001/inspect_static.py` uses the existing read-only
`tools/proto/box.py` archive reader and the pure layout functions from
`decomp-lemmings/tools/check_installed_level_prefixes.py` and
`parse_level_entities.py`. It advances from byte zero through prefix/world,
paired objects, entity prefix and gadget/model records; it does not search for
root names or Object markers. A metadata walk checks every node end against the
existing fixture parser. Complete cursor/node/material inventories are retained
in `build/scene-roots-001/static-inspection.json`. This ignored helper and JSON
are supporting static evidence; they are not registered CTest demonstrations.
Existing registered corpus checks are reused for source verification below.
No new test or build registration is changed in this notes-only task.

## Current application and renderer path

Source locations refer to this working checkout, including pre-task changes:

| Location | Current behavior / consequence |
| --- | --- |
| `src/app/main.cpp:446` onward | Resolves selected LVL through `levelFileNames` and `AssetLibrary::read`, then builds the tube and index. |
| `src/app/main.cpp:470`–475 | Unless `sceneOff`, calls `BackdropScene::build(*library, *blob, args.level)` and retains the result; unsupported/error diagnostics go to stderr. A successful empty result is still retained. |
| `src/app/BackdropScene.cpp:18`–19, `:36`–41 | Fixed first-level roots/ends, non-first-level early return, exact `readSceneNode` extent checks. The existing diagnostic says end/hash mismatch but this function checks an end offset, not a digest. |
| `src/assets/SceneNode.cpp:82`–210 | Reads exactly one explicitly positioned typed tree, recursively. Accepts node0/1/2, type2 branch0, mesh5/6/7; reports unsupported/truncated input with offsets and retains bounded allocations. No root selection. |
| `src/app/BackdropScene.cpp:67`–73, `:97`, `:110`, `:130` onward | Uses first-level offsets for primary/secondary selection, billboard partition and secondary visitor. Fixing only the build guard would leave wrong stages and dispatch. |
| `src/app/main.cpp:626`–630; `src/app/SceneFrame.h:9` | On one shared target/depth buffer: Primary, LegacyUnattributed models, tube including actors, Secondary. |
| `src/app/BackdropScene.cpp:70`–116 | Builds mesh records, evaluates supported kind5 predicate, loads named textures, projects and calls `rasterizeSceneTriangle`; secondary mixes mesh/billboard children through `dispatchSecondaryChildren`. |
| `src/app/SceneMeshSource.cpp:33`; `src/app/SceneBillboardSource.cpp:50`; `src/app/SceneDispatch.cpp:9` | Existing matrix/index/material, billboard and descending-key adapters to reuse. They do not establish original runtime admission. |

`BackdropStats.stageMeshRecords`/`stageBillboards` count partitioned candidates,
not actual raster calls or visible pixels. `pixels` counts depth changes, not all
color writes. Future demonstration must observe actual submissions and compare
framebuffer output, rather than treating these counters alone as acceptance.

## Original serialized selection and ownership

There is no evidenced file offset directory naming these four render roots.
Selection is the caller's position in a sequential, typed loader:

1. `0x004432C1` consumes the Level23 prefix/resource tables, its earlier embedded
   class1 object and the following name tables/short string. That early object
   belongs to a different loader section; it is not the primary scenery root.
   `0x004439F3` reads the final short string into `G+0x530A`.
2. At `0x00443A0A`, the same stream is passed to `0x00439FD3` with receiver
   `G+8` (world/tube data). At `0x00443A30`–36 the receiver becomes `G+0x5140`
   and `0x00437D48` reads the scenery pair. Thus world-reader completion, not
   a literal offset or a name match, determines the primary Object start.
3. `0x00437D64`–67 calls `0x004749BF` with receiver+`0x10` (primary holder).
   The holder consumes the length-prefixed Object09 marker at `0x004749E6`,
   then calls factory `0x0046BCA4` at `0x00474A1D`. The factory consumes a
   **u32** class tag via `0x00402560` at `0x0046BCC2`, constructs the selected
   class and calls its vtable **+0x0C** file reader at `0x0046BE29`.
   `0x00474A28` stores the returned **32-bit pointer** at holder+4.
4. After the entire recursive root tree, holder loader reads two **u8** flags
   via `0x00402540` at `0x00474A3C`/`0x00474A53`. The first is normalized into
   holder+`0x2C`; the second controls the auxiliary/animation list read
   `0x00474B0D -> 0x00474EB0`, starting with a **u32** count. Do not mistake
   these flags/list bytes for the next root or use the allocation word to skip
   serialized geometry.
5. Only after that holder load, `0x00437DC5` reads one **u8** second-presence
   flag, normalizes it into receiver+`0xA8` at `0x00437DD6`, and branches at
   `0x00437DE9`. If nonzero, `0x00437DFA`–FD calls the same holder loader on
   receiver+`0x54`. This is the secondary source root. Missing presence means
   absent secondary, not a failed scan and not permission to invent one.

The first holder flag is also used by the post-load transform path
`0x00437D83`–`0x00437DBD` (and the secondary counterpart
`0x00437E19`–`0x00437E56`); C3u calls this the mirror flag. It is not a generic
node visibility flag. The current source renderer uses retained serialized
transforms and conditional adapters; this research does not certify all native
post-load construction effects or silently add another reflection.

All these receiver methods use x86 ECX `this`; holder/scenery load methods have
two explicit stack arguments. Factory `0x0046BCA4` has five stack arguments,
returns the pointer in EAX, and its caller cleans 20 bytes at `0x00474A22`.
Shared base loader `0x0046BB73` and type0 wrapper `0x0046BEF0` clean 20 bytes
with `ret 0x14`; these are file-reader virtual-method boundaries, not portable
C++ object layouts to copy into the app.

Ownership is also separate from render stage. Holder construction sets holder+4
null and ownership bytes holder+`0x2D/+0x2E` to 1 (`0x00474754`–73); loading
again stores 1 into both at `0x00474A2E/35`. Holder destruction
`0x00474792`, conditional on root nonnull and ownership+`0x2D`, invokes root
vtable+0 with argument1 at `0x004747F4` and clears holder+4 at `0x00474805`.
Base-node destruction `0x0046B666 -> 0x0046C080` walks/deletes children; the
base loader stores their parent as a **32-bit pointer** at node+`0x78`
(`0x0046BB83`). A portable adapter should own its trees/vectors and source
metadata, not borrow original heap pointers or recreate the original allocator.

## Node and model extraction contract

All serialized integers below are little-endian; f64 means eight bytes of
binary64, not compiler-native long double or a new arithmetic calculation.

| Serialized component | Width / order | Original reader and source mapping |
| --- | --- | --- |
| Object wrapper | u32 length + 9 Object09 bytes; then typed root; then two u8 flags and optional counted auxiliary records | `0x004749BF`; `read_serialized_object` in `decomp-lemmings/src/object_base_prefix.cpp:259`. The class word starts 13 bytes after this marker start for these exact files; derive it from the string reader, never from a level table. |
| Type0 node | u32 type; common base below | Factory table at `0x0046BE3D` maps tag0 to ctor `0x0046BED0`; reader `0x0046BEF0 -> 0x0046BB73`. |
| Type1 node | u32 type; two u32-counted arrays of f64[3]; u32-counted material references (string + u32 selector); u32 block count, blocks; common base | ctor `0x00413290`, reader `0x00413450 -> 0x00469578`; retained `004432c1.md` and `object_base_prefix.md`. |
| Mesh block | u32 allocation word; length-prefixed PolyList09; u32 kind, u32 record count, u32 material slot | `0x00469BAF` dispatch; original allocation word is not file byte length. Source `SceneMeshBlock` stores it explicitly. |
| Kind5 / kind7 record | u32 normal index; respectively 3 / 4 ordered corners, each u32 vertex index + six f64 (UV/RGBA): 160 / 212 bytes | `0x00469B49 -> 0x0046A140 -> 0x0046B0D0 -> 0x00479FAD`; kind7 `0x00469B8D -> 0x0046A1E0 -> 0x0046B3D0 -> 0x0047A077`. |
| Kind6 record | u32 normal index, four shared f64 color/alpha, four corners of u32 index + two f64 UV: 116 bytes | C3g/C3x retained contract; existing scene reader expands shared attributes without changing byte consumption. |
| Type2 branch0 | u32 type, three f64, u8 visibility, u8 branch; string + four f64, u32 trailing selector; common base | ctor `0x0047B320`, `0x0047B44C`; f64 stores +`0x320/328/330`, visibility u8 +`0x110`, branch u8 +`0x338`. Branch1 instead calls `0x00468BF2` (four u32 + string), unsupported by current SceneNode. |
| Common base | twelve f64 matrix values; u32-length name; two f64; u32 child count; recursive typed children | `0x0046BB90 -> 0x0046BEA0` reads matrix, `0x0046BB9F` name; f64 stores +`0x100/+0x108` at `0x0046BBAF/C0`; u32 count `0x0046BBC9`, recursion `0x0046BC16`. Preserve both doubles even where unnamed. |
| Inline model type0 | u32 model type; string; three f64; complete Object wrapper; u8 final-name-present and optional string | factory `0x00465924` selects `0x00450BA0`; vtable+`0x50` reader `0x00450F57`, holder at model+`0xB0` loaded at `0x00450FE5`, pointer model+`0xB4`. Three f64 stores model+`0x100/+0x108/+0xF8` at `0x00450FA5/B9/CD`. |

Type3 and other unsupported forms must retain diagnostics. The factory knows
more classes than SceneNode/render adapters support. Similarly, recovered
`level_model.cpp:26` supports model0/1/2 serialized shapes; model1 references
resources and model2 has recursive models. Neither is automatically a static
scenery root. No missing field is evidence of uninitialized storage.

`decomp-lemmings/src/compiled_level.cpp:7` already composes prefix, world,
paired objects, entity prefix and gadget/model readers on one `LevelByteStream`.
`read_object_base_prefix` (`object_base_prefix.cpp:217`) retains both scenery
trees/flags/auxiliary bytes. `read_level_model` (`level_model.cpp:72`) retains
the separate model object. **Current recovered structs lack source extents.**
They do not yet supply the explicit start/end needed by `readSceneNode`; this
is a small production metadata addition, not missing executable evidence.

Do not derive extents by re-encoding names (embedded NULs can be normalized by
the recovered string adapter), by searching Object09, by resource-table order,
or by assuming that all model records are scenery. Capture stream cursors at
the actual reader boundaries, preserving every existing field/store.

## Additional installed level and source locations

Patch-first selection finds both entries below in **lemmings.box**; patch.box
has no override. MM_BASH_002's archive member starts at `0x03CB3C5E`.
The offsets in this section are **LVL-relative static observations**, solely
for review and regression expectations, never a production selection table.

| Entry | Payload bytes | Payload SHA-256 |
| --- | ---: | --- |
| `COMPILED\MM_CLIMB_002.LVL` | 699117 | `95f84f86fb53ab29ce2ce3aed794cf69a06e5754d42ef78657f3af1a5db4c567` |
| `COMPILED\MM_BASH_002.LVL` | 709367 | `65455693fa887f4474c66c2795526cb3b2bef2616449a5e1dc55debb9d7e5cd2` |

| Sequential boundary | MM_CLIMB_002 | MM_BASH_002 |
| --- | --- | --- |
| Prefix end / world start | `0xF4B` | `0x24C7` |
| World end / primary Object marker | `0x59637` | `0x5BE41` |
| Primary typed root..exclusive end | `0x59644..0x9176E` | `0x5BE4E..0x93F78` |
| Primary flags/list, then second-presence | `01 01 00000000 01` | `01 01 00000000 01` |
| Secondary Object marker | `0x91775` | `0x93F7F` |
| Secondary typed root..exclusive end | `0x91782..0x98D41` | `0x93F8C..0x9B54B` |
| Secondary flags / auxiliary count | `1 / 1 / 1` | `1 / 1 / 1` |
| Paired-object end, after auxiliary payload | `0x9CC49` | `0x9F453` |
| Entity prefix end / gadget lists | `0x9CC5D` | `0x9F467` |
| Entrance model-type word (u32=0) | `0x9CC98` | `0x9F4A2` |
| Entrance Object marker / typed root..end | `0x9CCC4 / 0x9CCD1..0x9DB2B` | `0x9F4CE / 0x9F4DB..0xA0335` |
| Exit model-type word (u32=0) | `0xA0CA8` | `0xA34B2` |
| Exit Object marker / typed root..end | `0xA0CD0 / 0xA0CDD..0xAAA43` | `0xA34DA / 0xA34E7..0xAD24D` |
| Complete serialized level end | `0xAAAED` | `0xAD2F7` |

For MM_BASH_002, primary first/list flags are 1/1 and list count0. The next
presence byte is at `0x93F7E`; it is 1. Secondary flag/list-count consumption
ends at `0x9F453`, not at the tree end `0x9B54B`. This concretely shows why
simply reading consecutive SceneNodes would misalign the stream.

Both levels' gadget lists start with u32 counts **0,0,1**: empty basic and linked
lists, then one hatch through `0x004487F0 -> 0x00425E50`. The model-present
spatial base `0x0043130E -> 0x00432283 -> 0x00465924` selects `Entrance.a3d`
type0. Exit models belong to `0x004488F0 -> 0x00426C6B`'s separate list.
These entity/model associations reuse C3u's address-backed ownership analysis;
equal names alone are not an admission rule.

| Role in MM_BASH_002 | Node census including root | Mesh records | Supported shape |
| --- | --- | --- | --- |
| Primary `thenull` | 1 type0 + 3 type1, all direct leaf meshes (`sand7`, `base`, `top`) | 457 kind5 + 466 kind7 = 923 | Complete tree bytes equal first-level primary. |
| Secondary `thenull` | 1 type0 + 12 type1 + 17 type2, 29 direct leaf children | 82 kind7 | Complete tree bytes equal first-level secondary; all type2 branch0, same resources/visibility/selectors. |
| Entrance `Top` | 1 type0 + 3 type1 (`cube8/9/13`) | 16 kind6 | Supported, independently placed model. |
| Exit `Top` | 1 type0 + 1 type1 (`cyl2`) | 8 kind5 + 148 kind7 | Supported, independently placed model. |

Total: 40 nodes, 19 mesh nodes, 1177 mesh records, 4243 corners and 17
billboards. This is a **serialized candidate inventory**, not a draw census.
Both scenery tree slices were compared directly as bytes, not merely by hashes
or counts; their metadata, matrices, sort bias values and texture names match.
Thus their supported resource/conditional adapter domain is the existing
first-level domain. Model slices differ: MM_BASH_002's Entrance translation is
`(30.747713495597758,75,39.397425625713296)` and Exit translation is
`(-42.421591397105885,48,49.16334428572214)`, with their own serialized yaw/basis.
Do not copy first-level placement or apply scenery-root scale to model roots.

MM_DIG_000 and the ordinary frontend's MM_FLOAT were inspected as rejected
alternatives: each secondary has 19 type2 **branch1** leaves. MM_FLOAT also
has inline model kind4 records. They cannot be admitted as complete supported
backdrops by merely removing the guard. No branch1/kind4 extension is proposed
in this task. MM_BASH_002 was selected because its serialized records stay
inside the existing renderer's supported shape, not because of a pixel score.

## Stage roles and conditional limits

With `G=0x004C6A08`, primary holder is `G+0x5150`, root pointer
`u32(G+0x5154)`; secondary holder is `G+0x5194`, pointer `u32(G+0x5198)`.
`0x00441879 -> 0x00437BA3 -> 0x0047496D -> 0x004749A7` dispatches primary
root **slot+4 before tube** (`0x00441884`). After tube/entity loops,
`0x0044199B -> 0x00437BB9` checks **u8 receiver+0xA8 = G+0x51E8**, then
`0x0047491B -> 0x00474955` calls secondary root **slot+8**.

C3y establishes secondary slot8 `0x0046B73F..0x0046BADF`: push root matrix,
form each direct child's camera-composed translation, rotate by the first nine
doubles of `0x0058D498`, add **f64 child+0x108**, sort descending and call each
child slot+4 at `0x0046BAD1`, then pop. It does not draw the root's own geometry
or test the root's own visibility inside this body. Root slot8 is not an alias
for slot4; sorting meshes and billboards independently loses the mixed order.
Existing SceneDispatch is certified only for distinct finite keys; tie order,
NaNs and animation-time matrices remain outside that seam. The selected
secondary's bytes exactly match the retained 29-leaf domain; no generalized
nested-secondary claim follows.

Entrance/Exit have **entity+0x44 -> model**, each with **model+0xB0 holder /
model+0xB4 root**, and a separate tube entity route:
`entity slot+0x10 -> 0x004323EB -> model slot+4 (0x00451865) -> 0x0047496D`.
At `0x0045186F..0x0045189E`, model+`0xF8` f64 biases global depth around the
holder draw. Collection membership, u8 entity+`0x28`, distance/angular tests,
model nonnull, animation and actual visibility remain conditions. Their root
location does not prove they were submitted or visible in any historical frame.
Keep the existing explicitly **LegacyUnattributed** software timing for these
two contributions in the bounded adapter; do not promote them to primary or
claim that their native tube/model timing has been reconstructed.

Frame gates remain **u32 G+0 mode==2** (`0x0045DD8E..9D`) and
**u8 G+0x5139 nonzero** (`0x00441853..5D`). C3u's successful-load path derives
mode2/enable1, not a historical last-writer timeline. Serialized second-presence
is load state, not an observed pause-frame byte. Actual original admission,
texture-cache history/effective device state and x87 precision are unknown.
f64 bit preservation and equal input trees do not prove agreement of arithmetic,
float packing, clipping, native submissions or pixels.

## Proposed bounded production task and before/after acceptance

Proposed scope: **replace fixed scenery selection/stage partition for
MM_CLIMB_002 and MM_BASH_002 using serialized role metadata**, connect the
existing BackdropScene renderer and demonstrate the selected additional route.
This is a proposal for the next queued production task, not work performed here.

1. Add portable source extents (marker begin, typed-root begin, recursive-tree
   end, holder/trailer end) to paired-object and `SerializedObject` results.
   Set them from `LevelByteStream.cursor` in `object_base_prefix.cpp`; existing
   `CompiledLevel`/`LevelModel` composition then carries them to callers. Keep
   them explicitly tooling metadata, not original layout fields. Preserve raw
   unknown values, flags and auxiliary bytes; no CRT or native allocator work.
2. In BackdropScene build, parse the complete structured level on caller-owned
   bytes using the existing composition. Take primary and conditional secondary
   extents from `level.objects`; take Entrance/Exit model extents from typed
   hatch `entities_425e50` and exit `entities_426c6b` records, with model-present,
   model-type0 and inline-object checks. Do not scan names to find start offsets,
   take the first four Object strings, or hardcode a second level table.
3. Call `readSceneNode(bytes, extent.nodeBegin)` and verify its exact recursive
   end against the serialized reader's `nodeEnd`. Retain owned trees plus an
   explicit role (Primary, Secondary, Legacy entrance/exit) and entity/model
   provenance. Root presence, supported shape and stage are different facts.
4. Replace **all** absolute-offset classifications in build, mesh partition,
   billboard partition and `visitSecondaryDispatch` with role/ancestry metadata.
   Preserve existing source offsets as diagnostic/observer IDs, descendant
   matrix composition, material selectors, kind5 predicate, vertex pack/raster,
   mixed secondary order and the shared no-clear frame schedule. Keep first-level
   output unchanged. Do not add different reflection, animation, clipping or
   model-timing behavior in this bounded change.
5. Fail explicitly for unsupported root/node/mesh/model/resource branches with
   level, role and source offset. A supported portion may be retained only with
   an explicit partial-scene diagnostic; never silently skip an unsupported
   root and label the complete level supported. Branch1 and other variants stay
   unsupported. Retain parser allocation/count/depth bounds and useful errors.

Register bounded **source-built CTest** demonstrations during that production
task, using repository-local archives read-only and registered UCRT64 GCC:

- Freeze authentic **current pre-integration** BackdropScene/application sources
  under ignored build storage. The old HEAD is not automatically the pre-task
  accepted state because this checkout is dirty. Before side must execute the
  current loader's real empty-return branch for actual MM_BASH_002 bytes.
- Use a fixed explicit source camera first, to isolate scenery from the next
  selected-hatch orientation dependency. Require before empty scene/zero scene
  submissions; after two scenery roots plus separately attributed models with
  the serialized candidate inventory above. These counts supplement emission
  checks; they are not success by themselves.
- Render through production `main -> BackdropScene -> SceneMeshSource /
  SceneBillboardSource -> SceneRaster` and the shared tube schedule. Record
  **actual raster submissions and accepted color/depth writes**, partitioned by
  role, and write deterministic scene-on/scene-off RGB artifacts under build.
  Require nonzero additional-level scenery submissions and changed pixels on
  a preselected view, while reporting culls, missing resources and rejected
  triangles. Compare shared tube/scene output as well as a scene-only diagnostic.
  Do not equate a candidate callback or depth-change counter with that evidence.
- Preserve first-level baseline: existing `backdropscene` primary RGB+depth
  expectation `0x1FD0BFB82AB39E95`, 923/0 primary, 172/0 legacy and 82/17
  secondary candidate partitions, 1177 total records, 17 billboards; preserve
  retained per-record predicate/secondary-order checks. Freeze and compare
  composed first-level renders too; unchanged primary alone is insufficient.
- Exercise one bounded public application route with installed MM_BASH_002,
  observer-driven SDL_QUIT and dummy drivers, proving actual renderer invocation
  from level entry. Do not unlock a map node by invented rules merely for this
  test; the existing direct-level route suffices for scenery integration. Any
  later ordinary frontend demonstration must use evidenced progress/selection.
- Add negative checks supporting this change: truncated holder/auxiliary data,
  absent secondary and unsupported type2 branch1 with explicit diagnostics.
  Existing MM_DIG_000/MM_FLOAT data can supply a real unsupported route; do not
  turn that into a new optional-format implementation or a broad test campaign.

The before/after demonstration is **planned**, not executed in this research
task. No claim of new rendered output, improved gameplay or original-game
agreement is made here. Existing C3 full-frame failure and amended gate
exclusions stay unchanged; no capture/clipping/controller campaign is reopened.

## Verification and preservation

Executed verbatim from the repository root:

```text
python tools/agent_control/verify.py --profile all
```

Exit **0**. The registered MinGW-w64 GCC build/configuration completed for both
profiles. Main **78/78 passed** (431.08 s); recovery **84/84 passed** (30.31 s).
The final registered `git diff --check` passed. Reference identity was checked
by reading bytes. Recovery was configured with empty `LEMMINGS_REFERENCE_EXE`;
the verifier's inventory check admits no original-byte oracle. Both suites
reported zero skipped/disabled tests in JUnit. Dummy SDL video/audio drivers
were supplied by the verifier.

Read back the actual JUnit and `Testing/Temporary/LastTest.log` for both
`build/agent-native/main/` and `build/agent-native/recovery/`. Relevant results:

| Registered check | Executed result / scope |
| --- | --- |
| `scenenode` | 1214 checks, 0 failures; four first-level extents and inventories match the table above. |
| `backdropscene` | 24 checks, 0 failures; `STAGE_CENSUS primary=923/0 legacy=172/0 secondary=82/17`; 1177 records, 17 billboards, 278820 changed pixels in its existing first-level scene fixture. This is an existing source-render baseline, not MM_BASH_002 output. |
| `installed_compiled_levels` | Parsed 105 installed archive level entries through the composed API. Entries include overridden base entries, not 105 unique overlay-selected levels. |
| `installed_level_prefixes` | Matched 105 installed prefixes/world/paired-object/entity records against the existing independent fixture parser; all second-root class tags0. |

JUnit truncates long BackdropScene output; the complete LastTest.log was read
for its final counters. These passes verify existing source consistency and
asset parsing only. The static inspection JSON is not newly certified as a
per-level rendered demonstration by those corpus tests. No new MM_BASH_002
submission/pixel result is claimed. Independent read-only acceptance remains
pending; the worker did not edit queue status.

The report itself was checked as LF-only (zero CR bytes). Unrelated tracked
installation backup/configuration files already trigger Git line-ending
warnings; this task did not normalize or modify those pre-existing files.

The initial checkout contained extensive staged/unstaged/untracked work,
including the moved recovery tree, save/audio integrations and an already-dirty
installed `lemmings_hooks.log`. No reset/clean/stash/commit/push was used.
The only intended non-ignored change from this task is this report; additional
inspection artifacts stay under ignored `build/scene-roots-001/`. Production,
tests, CMake, queue, controls, tool registry, verifier, permissions, installed
assets/saves and shared instructions are unchanged by this worker.

No original game, copied original instructions, native oracle, hook, debugger,
injection, capture, indirect launcher or background worker was executed. D1/D3/
digger, clipping/controller/capture parks, historical failures, recovery function
coverage and accepted audio/save work remain intact.

## Unknowns and next action

There is no unresolved critical static-selection gap for the bounded
MM_BASH_002 adapter above. Production source extents and role-based dispatch
are still absent, and actual additional-level submissions/pixels are untested.
That implementation and its before/after evidence are the next dependency.

Unknowns retained: actual original per-frame admission/cache/device state,
animation effects, exact model-holder timing/depth behavior in the app,
secondary tie/NaN ordering, general nested/unsupported variants, x87 precision,
clipping and full-frame fidelity. None requires guessing to preserve the
existing conditional renderer for these identical supported scenery trees.
After bounded integration, selected-hatch camera orientation/entry for other
levels is the next ordered milestone dependency; teleport links and command
semantics remain later. Research acceptance alone completes no runtime step.
