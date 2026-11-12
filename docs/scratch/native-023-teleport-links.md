# native-023 — teleport pads paired from serialized link ids

Reference: `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`
(verified by reading bytes only; `build/save-research-input/reference.json`).
The original was never run. No claim of original fidelity, a repaired frame,
natural gameplay or timing is made.

Milestone step: play an existing installed level. Route: `--level` / map
selection -> `sim::loadRetailLevel` -> `loadRetailLevelReport` ->
`webo::findLevelObjects` (structured teleport-link projection) ->
`LevelSetup::teleports` -> `World` teleport arm (unchanged).

## Changed files

Production: `src/assets/LevelObjects.h`, `src/assets/LevelObjects.cpp`,
`src/sim/RetailLevel.cpp`. Tests/build: `src/__test__/test_teleportlinks.cpp`,
`test_teleportlinks_measure.cpp`, `test_teleportdemo.cpp`, `test_teleportapp.cpp`,
`CMakeLists.txt`, `cmake/native023_freeze.cmake`, `cmake/native023_identity.cmake`.
Ignored: `build/native-023/` (frozen sources, `before/`, `after/` tables, `report.md`).

## Step 1 — evidence (static, then source-built measurement)

| item | address / source | status |
| --- | --- | --- |
| pad vtable install, `+0x260` cleared | `0x00434474`, `0x0043447D` | established (static) |
| partner resolve | `0x00434BCD`: if `[pad+0x260]==0` then `[pad+0x260] = list(0x4C6A08+0x5110)[ [pad+0x98] ]` (call `0x00434BE8` -> `0x00444BA1`, store `0x00434BF3`) | established (static) |
| `0x00444BA1` | not pad-specific: generic `list[index]` accessor (`0x004461D0`: `data[+0xC] + 4*index`), absolute list `0x004CBB18` | established (static) |
| **list store by id** (new this attempt) | base reader `0x00423291`: `[obj+0x3A] = int16 from stream` (`0x0042330C`, via `0x00423360`), then `0x00423310..0x00423325`: `list@0x4CBB18[movsx [obj+0x3A]] = obj` (`0x00423380` returns the slot, overwriting). Destructor clears the slot (`0x004231A0`). Slots are zeroed up to a stream-read count (`0x00443A5E..0x00443ABD`) | established (static) |
| consequence | partner = the actor whose base id equals the pad's `+0x98` word; with a repeated id the later-read actor wins; an id nobody filled gives 0 (no partner) | established by the two sites above; construction == stream order within `read_level_gadget_records` is **inferred** |
| reader | `0x00448CB0` -> `0x004347EC` (`Entity4347ec`): base `id` (`0x00423291`), `value_98` at `+0x98`, two `teleport_fx.a3d` models | established (recovered reader) |
| id list is shared by all actor classes | the store is in the shared base reader | established (static); measured below |

The earlier note's gap (id==index rested only on the retained table) is closed
statically by `0x00423325`. Remaining unknowns: the bound of the list
(`0x00402560` count) vs. an out-of-range id; what the original pad does with a null
partner; global ordering of construction across collections.

Measured by `teleportlinks_measure` (source-built, unchanged
`read_compiled_level(bytes,false)`; `build/agent-native/main/native-023/step1-measure.txt`):

- All 54 `entities_4347ec` records are `teleport_fx.a3d` pads on exactly the 18
  retained-census levels; locator count == structured count on every level (none odd/zero);
  locator Y order == stream order.
- **New:** across every actor collection of those 18 levels, base ids are all
  distinct (`idsSharedByMoreThanOneActor=0`, `padLinkTargetsSharedById=0`), so the
  shared-list collision case does not occur on installed data (the code does not see
  non-pad ids; that limit is stated in `LevelObjects.h`).
- MM_TELE_003 newly measured (structured): id/v98/(col,y) = 3/5 (219,30), 4/6 (241,64),
  5/3 (102,38), 6/4 (79,61). **Retained** live table (round 16, not re-measured): v98 5 at
  (219,30), 6 at (241,64), 3 at (102,38), 4 at (79,61); partners 4<->6, 5<->3. They agree.
- Pairing by ids differs from file order on **6 levels**; MM_TELE_003 and 11 others are the same:

| level | before (file order) | after (ids) |
| --- | --- | --- |
| H08_MT_1 | 0>2 1>3 2>0 3>1 | 0>1 1>0 2>3 3>2 |
| M18_MT_1 | 0>2 1>3 2>0 3>1 | 0>3 1>2 2>1 3>0 |
| M19_MT_1 | 0>2 1>3 2>0 3>1 | 0>1 1>0 2>3 3>2 |
| VH12_KF1 | 0>2 1>3 2>0 3>1 | 0>2 1>2 2>3 3>2 (ids non-reciprocal) |
| VH18_JC | 0>2 1>3 2>0 3>1 | 0>2 1>3 2>0 3>2 (ids non-reciprocal) |
| VH21_JC | 0>2 1>3 2>0 3>1 | 0>1 1>0 2>3 3>2 |

Unchanged: H06_MT_1, H10_MT_1, M12_MT_1, M16_MG_1, M17_MT_1, M21_KF_1, MM_TELE_003,
SHOW_KF3, SHOW_MT3, VH08_MT, VH10_MT, VH13_MT. No stop condition applied.

## Step 2 — production

- `LevelObjects.{h,cpp}`: `readTeleportLinks` (unchanged `read_compiled_level(bytes,false)`,
  caller storage kept alive, only `std::invalid_argument` translated); `resolveTeleportPartners`
  (changed this attempt: repeated id -> last in stream order, per the list overwrite at
  `0x00423325`, with a diagnostic; unmatched id -> -1 and a diagnostic); `findLevelObjects`
  fills `teleportLinks`/`teleportPartner`/`teleportLinkDiagnostic` only when count and Y per
  index match the locator (model name cannot be compared: records carry `teleport_fx.a3d`).
- `RetailLevel.cpp`: teleports built from `teleportPartner`; the file-order loop remains only
  as the labelled fallback when links were rejected/uncorrelated. A pad whose id nobody carries
  gets no teleport.
- Unchanged: `World` teleport mechanics, `teleportTriggerFromPose`, 4-tick delay, destination Y.

## Tests (all registered; verification run: main 88/88, recovery 84/84)

- `teleportlinks` (146 checks): resolver cases; **asset-free synthetic compiled level** through
  `findLevelObjects`: non-adjacent ids (40/7/12/99), odd count (3 pads pair by ids),
  duplicate id (last wins + diagnostic), unpaired id (-1 + diagnostic), Y mismatch and count
  mismatch (no links + diagnostic), truncated level (reader rejection -> links empty,
  diagnostic starts `teleport links: `, pads still located), junk input, unchanged input bytes,
  trailing input; retail literal partner tables for all 18 levels.
- `teleportdemo_before` / `teleportdemo_after` (80 checks each): public `loadRetailLevel` on all 18
  levels against literal partner tables. Before: file-order on all 18 (6 differ from ids).
  After: id pairing on all 18. Output tables: `build/native-023/{before,after}/teleports.txt`.
- `teleportapp`: `runApplication --level MM_TELE_003 --sim --native --frames 40 --no-audio`
  (dummy SDL; `--native`/`--frames`/`--no-audio` only bound the run), observer-queued `SDL_QUIT`
  at frame 3; `World::setup().teleports` has 4 pads on columns 219/241/102/79 with
  destinations the partner pad's column. MM_TELE_003 pairs the same before and after.
- `native023_identity_report`: reads the three frozen files' bytes and **fails** (FATAL_ERROR)
  if any SHA-256 differs from the supervisor entry manifest or the recorded entry hashes, or a
  file is missing. `cmake/native023_freeze.cmake` no longer regenerates anything; it only
  reports state, and the `native023_before_base` / `teleportdemo_before` targets are built only
  when all three hashes match.
- Superseded assertions: none (no existing assertion pinned file-order pairing on the 6 levels).
  The synthetic "duplicate id" expectation written in the previous attempt (ambiguous pad gets
  none) was replaced by last-wins, because of `0x00423325`.

## Not met / limitations

- **Frozen identity (history and resolution).** Earlier attempts of this task built the
  "before" side from a text-derived reconstruction whose SHA-256 values did **not** match the
  entry manifest (LevelObjects.cpp 05804d9c… vs 7e1b7d62…, LevelObjects.h a76f6288… vs
  98f45bb2…, RetailLevel.cpp 4bfa3b63… vs 1a589a3e…); the "before" outputs of those attempts
  came from different bytes and are superseded. The blocker was repaired outside this worker:
  `build/native-023/frozen/` now holds the exact task-entry bytes recovered from Git HEAD plus
  the retained before-diff hunks (`build/native-023/baseline-recovery.json`). This run
  (a) left those files untouched, (b) removed the reconstruction code from
  `cmake/native023_freeze.cmake`, and (c) re-ran every before/after demonstration against them.
  Executed result of this run: the three frozen files hash to
  LevelObjects.cpp 7e1b7d62…, LevelObjects.h 98f45bb2…, RetailLevel.cpp 1a589a3e… ==
  `build/agent-runs/20261031T050823-22828-d5a958b6/before-source-hashes.json`
  (CMake configure reported "native-023 frozen sources: present (entry hashes match)" and
  `native023_identity_report` passed on the match). I did not independently re-derive the
  files; the match is by SHA-256 against the supervisor manifest. No production file was swapped.
- The fallback inside `RetailLevel.cpp` itself has no asset-free test (it needs an archive);
  the fallback diagnostic is covered at `findLevelObjects`.
- Non-reciprocal ids (VH12_KF1, VH18_JC) are applied as literal lookups; two pads sharing a
  destination is what the lookup does, original behavior after arrival is not examined.
- Still open: teleport Y+2.0 snap, `teleport_fx`, facing term at `0x0043503C`, natural
  play-through, timing, bound/null-partner behavior.

## Verification (this run)

`python tools/agent_control/verify.py --profile all`: reference SHA-256 verified by reading
bytes only; main 88/88 passed (82 baseline + teleportlinks, teleportlinks_measure, teleportapp,
native023_identity_report, teleportdemo_before, teleportdemo_after); recovery 84/84 passed
(empty `LEMMINGS_REFERENCE_EXE`, `native_int_array` the only `native_` source test).
`teleportdemo_before` ran against the authentic frozen sources: file-order pairing on all 18
levels, 6 differing from the id rule (tables in `build/native-023/before/teleports.txt`).
`teleportdemo_after`: id pairing on all 18 (`after/teleports.txt`). The literal expectations in
`test_teleportdemo.cpp` come from the retained table and the step-1 rule, not from the new code.

Next dependency: statically read the list bound `0x00402560` / the null-partner path of the pad
at `0x00434BCD` (out-of-range and unpaired ids), then the Y+2.0 snap and facing term at
`0x0043503C`.
