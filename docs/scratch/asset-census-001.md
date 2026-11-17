# asset-census-001 — installed asset census

**needs_review; mandatory smoke pending supervisor execution.** Playable v1 research
exception, Codex override; difficulty difficult, recommendation Opus. No fixes or
tasks were queued. The next dependency is the lead selecting a bounded implementation
from the missing gameplay classes below. Before evidence is playtest
[8](../playtest/2026-11-16-playtest-8.md),
[11](../playtest/2026-11-16-playtest-11.md) and
[12](../playtest/2026-11-17-playtest-12.md): omitted hazards/pads/pipes, silent work,
and an unselected digger facing. Their accepted fixes were read before this census;
they are not proposed again.

The full [assets.csv](../../build/asset-census-001/assets.csv) contains **17,736 data
rows**, covering **10,972 physical archive entries** (10,661 base, 311 patch),
**63 loose files**, **618 sprite clip/facing rows** across 132 clips/19 resources,
**2,431 inline model/template records**, **724 model clips**, **2,636 localized
string slots**, **270 format-field declarations** and **22 object/collection kinds**.
The archive overlay has **10,673 effective names** and **299 replaced base entries**.
Models live inside levels; their `.a3d` names are not missing loose files.
Arrays are enumerated as fields, with their underlying geometry/records retained
in the structural export. Counts include source-location metadata where identified.

Each row gives the clone used/unused verdict, production call site for used content,
original static address/trigger, level count, complete `node:level_id` list, and
proposal group. `used` means a selected production consumer; shared files/packed
arrays can be partially used. Sprite facings and model clips expose that distinction.
The revision resolves the former `read_only` rows; none remains in the final CSV.
`apparently_unused` records a bounded negative with its search limit, while
`dormant_on_installed_routes` distinguishes implemented original machinery with no
installed instances. Superseded entries and installation tools/backups are separate.

The [ranked task table](../../build/asset-census-001/proposed-tasks.csv) supplies each
title, implementation scope, all affected levels/nodes, evidence, winning impact,
difficulty, recommendation and model reason. These are proposals requiring lead
selection; they grant no implementation permission and reopen no timing/controller parks.

| Rank | Proposed work | Installed reach; first route | Impact; difficulty/model |
| --- | --- | --- | --- |
| 1 | Crossing-count gates, counters, collision and Rumble | **231 gates / 52 levels**; node **25**, `MM_ALL_8_003`, **The Iron Curtain** | Missing puzzle rules; difficult/Opus |
| 2 | Placed evil rats, attacks and presentation | **45 rats / 17 levels**; node **28**, `E11_KF_1`, **Escape to victory** | Missing lethal actors; difficult/Opus |
| 3 | Teleporter/speed-pad models, FX and clips | **22 levels**: teleports 18, speed pads 9; first node **38**, `MM_TELE_003`, **The Lone Ranger** | Existing simulation lacks visible landmarks/activation; difficult/Opus |
| 4 | Occupancy sensors and their two message lists | **7 sensors / 2 levels**: **55 `M12_MT_1`**, **80 `H08_MT_1`** | Potential required trigger chains; difficult/Opus |
| 5 | Scrape/MineMetal refusal sounds | **83 stocked levels**, first node **5 `MM_MINE`** | Explain steel/one-way refusal; routine/Sonnet |
| 6 | Fluid light overlays and separate glow strips | **520 omitted roots / 77 levels**, first node **1 `MM_FLOAT`** | Texture-bound secondary draw now traced; difficult/Opus |
| 7 | Primary/secondary authored scenery tracks | **105 `myanim` clips / 102 levels**, first node **0 `MM_CLIMB_002`** | UV/transform/rotation animation; difficult/Opus |
| 8 | Installed miner Axe effect | **58 Miner-stocked levels**, first node **5 `MM_MINE`** | Missing work effect; difficult/Opus |
| 9 | Explosion surface mark (`~splat`) | **102 potentially affected levels** through stocked nuke; ordinary Bomber check at node **2 `MM_BOMB`** | Additional missing explosion feedback; difficult/Opus |
| 10 | Music lifecycle | **13 WAVs / all levels and frontend** | Mixer support exists without production callers; routine/Sonnet |
| 11 | Compound fish groups | **55 groups, 451 child fish / 39 levels**, first node **1 `MM_FLOAT`** | Decorative content; difficult/Opus |
| 12 | Colored indicator lights | **9 lights**, nodes **93 `VH16_MT`**, **98 `VH22_KF1`** | Linked state/blink feedback; difficult/Opus |
| 13–19 | HUD icons/preview shadow; cursor producers; localization; audio options/Ting; movies; Witch easter egg; low-detail actors | Full route lists in proposed-tasks.csv; Witch only node **91 `E10_KF_1`** | Lower gameplay priority; individual grades/reasons in CSV |

Winning impact above is a source/data inference. No new route was played to establish
that a level cannot be won. Refusal reach means a relevant skill is stocked; it does
not prove a natural player encounters protected terrain on every listed level.

The production path is `runApplication -> AssetLibrary` (patch before base) ->
`loadRetailLevelReport` / `TubeScene::build` / `BackdropScene::build`, with audio through
`World::takeSoundEvents -> CuePlayback -> Mixer`. The critical omission is explicit:
[RetailLevel.cpp:83](../../src/sim/RetailLevel.cpp#L83) registers counter gates as
`Other`; [World.cpp:332](../../src/sim/World.cpp#L332) handles named hazards and
[World.cpp:353](../../src/sim/World.cpp#L353) handles fluid gates. There is no counter-gate
state/update. No production consumer projects `subclass_actors` into World. Backdrop
extraction admits hatches/exits, tagged types 0/1/3, named hazards and fluid base/pipes;
it omits fish, lights, tagged type 2/4, teleport models and receiver `model_15c`.
Its scenery clock advances sprite nodes, without applying primary/secondary `myanim`.

Static authority is only the registered repository-local **Lemmings Revolution.exe**,
SHA-256 **32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f**;
Ghidra **12.1.4**, profile
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
Used reverse-engineer-anything with the static profile. Complete returned evidence
and limitations are in [static/](../../build/asset-census-001/static/), indexed by
[static-evidence.csv](../../build/asset-census-001/static-evidence.csv).
[Instruction cross-checks](../../build/asset-census-001/static/crosscheck.md) retain
the decisive operands, layout and state transitions:

- **Gates:** reader `424187`, update/count `423749/423858`, counter draw `423E69`,
  collision animation `4270DC`, stamp `4275C1 OR 403`, clear `427641 AND FBFC`.
  This is a separate serialized class from the accepted fluid receivers.
- **Rats:** `448550 -> 448630`, vtable `4B0F6C`, reader `4265E8`, attack `426414`;
  `408ECC` removes the retained victim on completion. Installed sheet type is 3.
- **Sensors:** `433DD9` tag 2, `43725A` occupancy, `4339FC` cooldown,
  `437285/43729A -> 4372AF -> 433AC0` messages.
- **Correction to retained Axe research:** old round23's UI/HUD label is too broad.
  Miner construction reaches `496E40`; vtable `4B0B88+4 = 4196C0` calls
  `493761`, which submits the live child model draw. Its motion remains unported.
- **AntiGrav audio:** registration `45E51F` stores handle `4E32F4`; new xrefs are
  empty. Its movement contact at `41C1E3` is not an audio play site.

Revision after independent review: the same **17,736 identities** remain, with
**12,287 clone-used / 5,449 clone-unused** rows and **19 ranked proposals**.
[Detailed revision evidence](../../build/asset-census-001/revision-evidence.md)
retains the targeted traces, instruction checks and negative-search boundaries;
[field-contract-audit.csv](../../build/asset-census-001/field-contract-audit.csv)
audits all **270** schema rows. The two named errors are corrected:
`models_1c8_1cc` semantically selects teleport links at **LevelObjects.cpp:220**;
`first_integer_array` becomes the surface lattice at **LevelIndex.cpp:147**.
Other corrections include selected-hatch camera, lever reversal, hazard subclass
selection and independently decoded geometry fields. Teleport/FX/speed-pad model
rows distinguish identity/trigger use from their still-omitted geometry. Each
installed first FX name matches: the second name test is short-circuited, so its
**54** individual model rows remain unused despite the aggregate field consumer.

The classifier's former assertion that native diggers always select front view
was wrong. Accepted [digger evidence](digger-facing-001.md) establishes phase
**0.75** at `4159B4 -> 4683E9`, followed by camera-dependent selection at `42EEF2`.
The **12 unused Hi side-view rows are native-used**, with an accepted-policy
disposition. Preserve the clone's provisional fixed view 4; this is no new
playtest defect or proposal to undo the user's requested presentation.

The new positive traces resolve the most significant attribution gaps:
`42B0B8 -> 424290/4C602C -> 441E26 -> 43FCAA/43FB14 -> 4622FD -> 43FA93`
binds each named glow texture and draws two triangles. The separate +15C overlay
is submitted by `441845 -> 42C5A7`; `42C556` draws pipes only. Explosion entry
`408227` calls `43B31E` with **slot 0 (`~splat`)**, reaching that secondary draw
path. Its sole traced caller never selects slot 1 (`~splat2`). These are static
output traces, not observed original pixels.

Apparently unused/dormant content has its own
[CSV](../../build/asset-census-001/apparently-unused-original.csv): spare hazard
textures for absent boot/glove/bird/frog/etc. models, old/unregistered WAVs,
registration-only Balloon/Fire/ForceF/Splat/AntiGrav samples, Smoke/TestIcons,
Paint Shop Pro `PSPBRWSE.JBF`, patch manifest and unreferenced `gameover.avi`.
Trampoline art has an original skill path, but no established installed activation;
pressure pads are not trampolines. Spark/electricity has an original path-effect
constructor, but all 102 installed name-pair lists are empty. No new hazard was
invented from a filename.

Corrections to the earlier limits: three caret splash/splat families have no exact
EXE/installed dependency in the audited inputs; `~splat2` has a preload but no
selector in the traced producer. They are bounded negatives, separately documented
from the now-positive `~splat` and glow traces. Polish/Czech/Hebrew tables and P/H
art have no producer in the pinned five-language loader. This says nothing about
other installed EXEs/DLLs, which were inventoried without analysis. Native map
titles come from DAT records; LTX title slots 130–231 have no consumer in the audited
title paths. Tutorial slots 87–114 have an implemented native display path, but
every installed scripted-widget list is empty. Slot 124 is unconditionally used
by native HUD setup and has been corrected accordingly.

Scalar negatives also retain their boundaries: the audited installed class paths
do not consume three named-hazard scalars, tagged `flag_b0`, or the zero-valued
rat `field_20_bits`/`field_30_bits`/`flag_5f`. The instruction/pseudocode audit and
all 45 rat values are retained in
[negative-field-audit.csv](../../build/asset-census-001/negative-field-audit.csv).
This is no proof against every editor, absent-class or arbitrary indirect path.
Packed fields can be partially used; a used array does not recover every scalar's
meaning. Final pixels/depth state, native timing and effects on human wins remain
unobserved. The previous report and tables are preserved under
`build/asset-census-001/revision-entry/`; these corrections supersede their unresolved
classifications without turning them into original-fidelity passes.

Changed task report: this file. All scripts, CSVs, evidence and source-built
read-only exporter are under ignored `build/asset-census-001/`. The exporter used
registered UCRT64 GCC and production parsers: **102/102 levels through exact EOF**,
every exported scene tree ending at its parser extent. Sprite parsing consumes all
19 resources through EOF. Coverage checks, file identities and sample headers are
recorded in [validation.json](../../build/asset-census-001/validation.json).
The revision's source-built field exporter independently reread **102/102** levels
through exact EOF. Task-local checks retain the stable **17,736-row identity set**,
all source locations, and the review's corrected contracts. These checks validate
the research output, not gameplay. Last completed supervisor smoke was
**95/95 main and 84/84 recovery**, at
`build/agent-runs/20261117T170339-19536-66c19450/verification.log`.
A new smoke run remains **pending supervisor execution after this revision**.
No production code, test, replay, queue, verifier, controller, registry, shared
instruction or save was changed. No original execution, copied instructions/oracle,
hook, debugger, capture, installation, subagent, detached work, commit or push.
Save files were only listed by metadata; no save content was read or written.
Initial CIM process detail was denied; Get-Process showed no clone/game runner.
The existing queue/status/hook-log edits were preserved. No protected edit required
a separate lease. Early exporter link/site-lookup errors and their corrections remain
in the task logs. No mandatory suite was duplicated. Replays were not run because
this research changes no gameplay route; there is no divergence or retirement request.

Handoff: revised census and 19 ranked proposals ready; gates and rats remain the earliest missing gameplay classes.
Next: supervisor smoke and independent attribution review, then lead selection; status needs_review.

**What to check:** after a separately authorized fix, **node 25 / MM_ALL_8_003 /
The Iron Curtain**: operate the switch and send lemmings across the gate; the visible
counter, barrier and collision should change together. Then **node 28 / E11_KF_1 /
Escape to victory**: rats should be visible and attack contacting lemmings while
remaining outside rescue totals. No after-fix behavior is claimed by this census.
