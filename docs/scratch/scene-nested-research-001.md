# scene-nested-research-001: nested Secondary scenery traversal for the 18 remaining installed levels

2026-11-01. **Research task; finishes `needs_review`.** Static reading of the retained disassembly, byte reads of the
reference executable through the project-owned `rea_static` Ghidra profile (open_binary with the registered SHA, `read_bytes`
only, closed without a snapshot), and one new registered read-only measurement (`scenenested_measure`). No production C++
(parser, renderer, scene, app), queue, control, permission, verifier, completion or tool-registry file changed. No original
executable, copied original instruction, oracle, hook, debugger, capture, launcher, installation or save write was used. This
is research evidence, not implementation, rendering acceptance or original-fidelity proof.

## Result

The 18 `unsupported nested/non-leaf secondary dispatch` levels are **one structural class**: every one has a Secondary tree
of 30 nodes (1 type-0 root, 17 type-1, 12 type-2 branch-0) in which exactly **one** direct child (`cube1_trans`, a type-1 mesh,
the windmill body) has exactly **one** child (`windmill_roll_0.2_trans`, a type-1 mesh, the windmill blades). There is no
type-2 parent, no type-0 non-root, no hidden node, no unsupported type and no other first-failure reason; all resources
resolve. The original traversal for this shape is established from instructions: sorted direct children by the descending
**binary-insertion** order of `0x0046B8F4..0x0046BA72` (tie behaviour closed and shown to differ from `std::sort`), each
type-1 node draws its own meshes and then its children in file order (`0x0046C55D`), gated by byte `+0x110` (constructor
default 1; no serialized or runtime writer touches it for these nodes). Representative: **E11_KF_1**. Two findings outside
the 18 matter for the production task: (1) the production `std::sort` tie order already differs from the original in the 74
newly admitted branch-1 levels and all 18 targets (it matches for the 10 originally admitted levels); (2) the type-2 parent
matrix is applied **twice** to its children in the original (`0x47D0F2` and `0x0046B6EB`) - established but not exercised by
any installed level.

## Identities

| Item | Value |
| --- | --- |
| Reference executable (bytes only) | `Lemmings Revolution/Lemmings Revolution.exe`, 851,968 bytes, SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f` (verifier and `scenenested_measure` both hash it; REA `open_binary` reported the same digest) |
| Retained disassembly | `build/save-research-input/reference-disassembly.txt` (line numbers below are lines of this file; sha256 `36bc7d45...f9b1` per scene-branch-research-001) |
| Archives | `lemmings.box` with `patch.box` precedence via `AssetLibrary::open(lemmings.box, patch.box)`; 102 effective levels from `levelFileNames` |
| Measurement outputs (ignored) | `build/scene-nested-research-001/{summary.txt, report.md, trees/<LEVEL>.txt, trees/<LEVEL>.plan.txt}` |
| Per-level payload SHA-256 | in `summary.txt` `DETAIL` lines and the table below |

`this` is ECX (thiscall). "node" is a scene node; node+0x08 is its 12-double matrix, node+0x68 its child vector, node+0x7C
name, node+0x100 / +0x108 the two base head doubles, node+0x110 the byte under study.

## 1. Evidence matrix (instruction-backed)

| # | Question | Result | Evidence (addresses; disasm lines) |
| --- | --- | --- | --- |
| E1 | Per-type draw entry (slot +4) | type 0 -> `0x0046B6CC`; type 1 -> `0x0046C55D`; type 2 -> `0x0047D0C0`; all slot +8 -> `0x0046B73F` | vtable bytes read with REA `read_bytes`: type-0 `0x4B19A0` = `{0x46BF20, 0x46B6CC, 0x46B73F, 0x46BEF0}`; type-1 `0x4B0948` = `{0x413480, 0x46C55D, 0x46B73F, 0x413450}`; type-2 `0x4B1AD8` = `{0x47B610, 0x47D0C0, 0x46B73F, 0x47B44C}`; base `0x4B1990` slot+4 = `0x46B6CC` (constructor stores `0x4B1990` at `0x46B5DF`, derived constructors overwrite: `0x46BEE2`, `0x41331E`) |
| E2 | Where `+0x110` comes from | The base constructor `0x0046B590` stores **1** at `0x0046B645` (`mov byte [eax+0x110],1`; lines 139133). Type 0 and type 1 constructors (`0x46BED0`, `0x413290`) call it and never write `+0x110` again; the type-2 reader `0x0047B44C` overwrites it from a serialized byte (`0x47B4C8..0x47B4DC`, normalised to 0/1; lines 160331-160337) | disasm 139081-139134, 24325, 139799, 160331 |
| E3 | Every static writer of `+0x110` | exhaustive grep of `byte [reg+0x110]` stores: `0x46B645` (ctor), `0x47B4DC` (type-2 reader), `0x4390F4` (copy of the byte inside a node copy routine ending at `0x439102`, preceding block copies +0x100/+0x108), `0x4319E5` (writes 0 to every node in the list at `this+0xCC`; **no direct caller** found, `0x431950` is reachable only through a vtable), `0x45DA0D` (main loop, below) | grep over the disassembly; lines 63236, 72970, 120443, 139133, 160337 |
| E4 | Runtime writer `0x45DA0D` and its population | In the main loop function `0x45CFF9` (lines 120386-120444) a local byte starts 1 (`0x45D971`); if `frameMs > 0.08*1000` (`[0x4B1800]*[0x4B1758]` = 80, `fcompp`/`test ah,1`) and byte `[this+0x1C9B3]==0` a counter increments and on `> 0x14` (20) the byte becomes 0 (`0x45D9B7`); otherwise the counter resets (`0x45D9C4`). The byte is stored to `+0x110` of **every element of the vector at `this+0x5140`** (`0x45D9E3..0x45DA0D`; indexer `0x437FE0` = `[set+0xC]+i*4`). That vector is filled by `0x00437D48` (lines 71533-71598): for each node in the two holders' type-1 vectors (`set+0x60` and `set+0x1C`) whose name satisfies `strstr(name,"cutme")` (`0x49CE20` is the MSVC `strstr`, lines 208200-208245; strings `"cutme"` at `0x4BFEB4`/`0x4BFEBC`, bytes read), `0x438110` appends it. **So the only runtime-toggled nodes are type-1 nodes whose name contains `cutme`: a frame-time level-of-detail switch (hidden after more than 20 consecutive slow frames, visible again on the next frame at or under 80 ms).** | lines 71533-71598, 120386-120444, 208200 |
| E5 | Which nodes register in which holder vector | The type-1 reader `0x00469578` appends `this` to the vector passed as its 3rd argument when nonnull (`0x469581..0x469594`, `call 0x438110`); the type-2 reader `0x0047B44C` appends itself to the 4th (`0x47B46E..0x47B484`, `call 0x47B640`). The holder loader `0x004749BF` passes `holder+0xC` / `holder+0x1C` (`0x474A0F..0x474A1D`), recursion forwards both (`0x46BB73`, `0x46BC02..0x46BC16`). Registration is therefore per node type **at any depth** | lines 135690-135697, 160310-160317, 151266-151277, 139561-139571 |
| E6 | Type-0 (and base) slot +4 `0x0046B6CC` | gate `byte[this+0x110]` (return if 0, `0x46B6DA..0x46B6E2`); push `this+8` (`0x43BC10`); loop `i=0..count(this+0x68)-1` in **file order** calling each child's slot +4 (`0x46B715..0x46B734`); pop (`0x43BBE0`). No sort, no per-child cull | lines 139171-139209 |
| E7 | Type-1 slot +4 `0x0046C55D` | gate `byte[this+0x110]` (`0x46C56B..0x46C573`; hidden node skips itself **and its children**); push `this+8` (`0x46C580`); `0x46C440` stores the stack top into `this+0x148` and an inverse-scale vector into `this+0x1A8`; camera-space centre `out = 0x40EB50(camera 0x58D498 data, this+0x148 data)`, `z = out[2]`; **if `z + [this+0x100] <= 0.0` (`0x4B0338`) the node and its children are skipped** (`0x46C5C4..0x46C5E4`: call `0x46C50F` cleanup, pop, return); otherwise vertices are transformed, blocks drawn with the unclipped routine `0x469D71` when `z - [this+0x100] > 0` else the clipped routine `0x469EAA` (`0x46C65B..0x46C6B6`), then **children in file order through their slot +4** (`0x46C6F6..0x46C737`), `0x46C50F` cleanup, pop (`0x46C741`). **`[this+0x100]` is a bounding radius** (the first base head double). The node's own geometry is drawn before its children | lines 140435-140580 |
| E8 | Type-2 slot +4 `0x0047D0C0` and children | gate `byte[this+0x110]` (`0x47D0D9..0x47D0E3`, hidden skips draw and children); push `node+8` (`0x47D0F2`); billboard/sprite draw (reports 001/002); at `0x47D85C` `call 0x46B6CC` which **pushes `node+8` again** (`0x46B6EB`) and draws the children in file order; pops at `0x46B736` and `0x47D861`. `0x43BC10` = `push(compose(node.matrix, top))` (`0x43BC10..0x43BC4E`: `0x43BCD0` top, `0x43BCA0` multiply, `0x43BCF0` push). **Children of a type-2 node therefore see its matrix applied twice** | lines 162833-162848, 163248-163249, 139185-139191, 76947-76965 |
| E9 | Root slot +8 `0x0046B73F` | push root matrix; for every direct child i (file order) push child matrix, form `key[i] = z' + child[+0x108]` where `z'` is the third component of `0x40EB50(out, local top data, camera 0x58D498)` (`0x46B83F..0x46B85C`), pop; then order, then call each direct child's slot +4 in order, pop. Visibility of the **root** is not tested; the root's own geometry is not drawn. Only the root's **direct** children are ordered | lines 139211-139464 |
| E10 | Ordering algorithm and ties | array `idx` starts `{0}`. For `i=1..n-1`: `lo=0`, `hi=size-1`; `while(hi-lo>1){mid=(hi+lo)>>1; if(key[i] < key[idx[mid]]) lo=mid; else hi=mid;}` (`0x46B93A..0x46B9B5`, `fcomp` + `test ah,1`); then `if(key[i] < key[idx[hi]]) insert(hi+1,i)` (`0x46B9D8..0x46BA08`); `else if(key[i] > key[idx[lo]]) insert(lo,i)` (`test ah,0x41`, `0x46BA0A..0x46BA53`); `else insert(lo+1,i)` (`0x46BA5A..0x46BA6D`). Insert helper `0x46C2A0(pos,value)` copies `[0,pos)`, writes the value, shifts the rest (lines 140195-140243; argument roles from the pushes). Result: **descending by key**; for equal keys the position is a function of the search path, **neither stable nor reverse-stable** (measured below). An unordered (NaN) `fcomp` sets C0=C2=C3=1, so it counts as "less" in the search and the `hi` test; NaN handling is outside the contract (the measurement did not test keys for finiteness) | lines 139299-139433 |
| E11 | Nested vs direct ordering | nested children are never ordered: they are drawn depth-first in file order immediately after their parent's own geometry and before the next ordered direct child (E6, E7, E9) | as above |
| E12 | What does not apply to the 18 | type-2 parents with children (E8) and type-0 non-root nodes (E6) do not occur on any installed level (measured, section 2); they are established but unexercised | `summary.txt` totals |

Pseudocode-versus-instruction check: the Ghidra decompilation of `0x46C55D` was not needed; the assembly was read line by line
for E1-E11 and the `fcomp`/`test ah` flag polarity was derived from the instruction semantics (`ah&1` = ST0 < operand,
`ah&0x41` = ST0 <= operand or unordered), not from decompiler output.

## 2. Executed read-only extraction (`scenenested_measure`, CTest, 0.97 s, passed)

Command: `python tools/agent_control/verify.py --profile main` (and `--profile recovery`), results in section 7. The test
reuses the production readers (`read_compiled_level`, `readSceneNode`, `buildSceneMeshSource`, `makeSceneBillboardSource`,
`loadSpriteSheet`), asserts the reference SHA, 102 effective levels, every Secondary tree ends exactly at the serialized
`tree_end`, and that the production nested predicate selects **exactly** the 18 accepted levels.

### 2.1 Population over all 102 effective levels

* Secondary trees: depth <= 2 everywhere; **type-2 nodes with children: 0; type-0 below the root: 0; hidden type-2 (serialized
  visibility 0): 0; non-type-1/2 direct children: 0; unsupported node types: 0**. Primary and Legacy model trees: no tree
  deeper than depth 1.
* The only nested shape in the whole install is type-1 -> type-1, and it occurs only in the 18 levels.
* `cutme` type-1 nodes: 10 per level, all direct children, in the 71 branch-1 levels whose Secondary tree has 86 nodes
  (including MM_DIG_000); 0 in the 3 smaller branch-1 levels (E10_KF_2, E20_MG_1, MM_E4_4_SKILLS), in the 18 and in the
  10 originally admitted levels.

### 2.2 The 18 affected levels (all identical in structure)

Common to all: 30 nodes = 1 type-0 root `Spooky_Forest` (28 children) + 17 type-1 + 12 type-2 branch-0 (resource
`Models\bg_tree04\light02`, `vis=1`); parent `cube1_trans` (flat index 5, 3 mesh blocks, 12 records = 1 kind-5 + 11 kind-7,
material `Models\bg_tree04\Windmill_02` selector 0, radius `+0x100` = 395.07416, sort bias `+0x108` = 60.2332731, translation
(26.2226, 2.00902, -10.7449)); child `windmill_roll_0.2_trans` (flat index 6, depth 2, 1 block, 5 kind-7 records, material
`Models\bg_tree04\^Windmill_03` selector 1, radius 47.0009811, sort bias 21.8983285, translation (0.00087, 2.32932, -0.87441)).
`firstNonNestedFailure` is empty and `missingTextures` is 0 for every one.

| Level | Secondary typed root | tree end | parent `cube1_trans` [begin,end) | child `windmill_roll...` [begin,end) | payload SHA-256 |
| --- | --- | --- | --- | --- | --- |
| E11_KF_1 | 0xb33dc | 0xc0304 | 0xb60e7,0xb7576 | 0xb6ed3,0xb7576 | d589c76e...83782c |
| E22_KF_1 | 0xb0ee5 | 0xbde0d | 0xb3bf0,0xb507f | 0xb49dc,0xb507f | 0e85e6cc...bfdc |
| H05_KF_1 | 0xb2e16 | 0xbfd3e | 0xb5b21,0xb6fb0 | 0xb690d,0xb6fb0 | f37d07f2...d1d4 |
| H09_KF_1 | 0xb0ca2 | 0xbdbca | 0xb39ad,0xb4e3c | 0xb4799,0xb4e3c | dcef89e1...7842 |
| H20_KF_1 | 0xb3101 | 0xc0029 | 0xb5e0c,0xb729b | 0xb6bf8,0xb729b | fa442b2e...5747 |
| H24_KF_1 | 0xb5148 | 0xc2070 | 0xb7e53,0xb92e2 | 0xb8c3f,0xb92e2 | ffaec3a7...658e |
| M04_KF_1 | 0xb02d5 | 0xbd1fd | 0xb2fe0,0xb446f | 0xb3dcc,0xb446f | bb11e169...6714 |
| M11_KF_1 | 0xb3715 | 0xc063d | 0xb6420,0xb78af | 0xb720c,0xb78af | ec94aba5...35ad |
| M16_MG_1 | 0xaf431 | 0xbc359 | 0xb213c,0xb35cb | 0xb2f28,0xb35cb | 4da8c08f...3d9b |
| M21_KF_1 | 0xb1367 | 0xbe28f | 0xb4072,0xb5501 | 0xb4e5e,0xb5501 | 88bf4fde...0304 |
| MM_BLOCK_BOMB_A | 0xb15a8 | 0xbe4d0 | 0xb42b3,0xb5742 | 0xb509f,0xb5742 | 88f1cafe...3d78 |
| MM_FLOAT_CLIMB2 | 0xb058d | 0xbd4b5 | 0xb3298,0xb4727 | 0xb4084,0xb4727 | 517d541e...1055 |
| MM_H_003 | 0xb830d | 0xc5235 | 0xbb018,0xbc4a7 | 0xbbe04,0xbc4a7 | aa516323...4230 |
| VH04_JC1 | 0xac981 | 0xb98a9 | 0xaf68c,0xb0b1b | 0xb0478,0xb0b1b | 96c49e39...0abf |
| VH05_JC1 | 0xb54ec | 0xc2414 | 0xb81f7,0xb9686 | 0xb8fe3,0xb9686 | 45202696...7e22 |
| VH18_JC | 0xb5f47 | 0xc2e6f | 0xb8c52,0xba0e1 | 0xb9a3e,0xba0e1 | 846a2974...1e15 |
| VH19_JC | 0xb57fc | 0xc2724 | 0xb8507,0xb9996 | 0xb92f3,0xb9996 | a19e0f0b...2a6b |
| VH21_JC | 0xb67a4 | 0xc36cc | 0xb94af,0xba93e | 0xba29b,0xba93e | d42fe4c5...170a |

(Abbreviated hashes: the full 64-hex SHA-256 of each payload is in `build/scene-nested-research-001/summary.txt`; for example
VH21_JC is `d42fe4c5c5ce4d0432a3ee88e30f7bffc2956cb189eb65b3c20de0959ef2170a`. The per-level archive source (base or patch) was not
separately listed by this measurement; payloads are read through the effective patch-first `AssetLibrary`.)

### 2.3 Ordering measurements (default `buildCamera(CameraParams{})`, sort keys only)

* The 18 levels have 9 pairs of bit-identical keys among 28 direct children; the original binary-insertion order differs
  from `std::sort(key>)` in all 18. The same holds in all 74 newly admitted branch-1 levels, MM_DIG_000 included (3 tied
  pairs each; 74 of 74 differ). **For the 10 originally admitted levels (E06_KF_1, H17_MG_1, M06_KF_1, M08_KF_1, MM_BASH_002,
  MM_CLIMB_002, MM_E1_3_SKILLS, MM_E1_4SKILLS, MM_W_012, VH22_JC) there are no ties and the two orders are identical**, so
  switching the production order to the evidenced algorithm cannot change their accepted artifacts.
* E11_KF_1 direct-child order (child index: key): `4:705.052 27:694.427 26:694.427 25:693.165 24:693.165 0:691.500 1:691.500
  19:687.373 18:687.373 21:686.638 20:686.638 23:560.183 22:560.183 17:560.183 5:536.543 14:499.777 7:453.666 6:424.629
  9:406.535 8:404.091 3:385.435 2:385.435 12:381.384 13:374.718 16:356.987 15:334.607 11:312.298 10:305.593`. The tied
  pairs resolve as (27,26), (25,24), (0,1), (19,18), (21,20) and the triple (23,22,17): later-first in most pairs, earlier-first
  for (0,1), and 17 after 22/23 although it is earlier in the file - the algorithm, not the file order, decides.
* Traversal plan for E11_KF_1 (`trees/E11_KF_1.plan.txt`): 29 visits, 0 hidden; the first visit is `cube1_trans` (0xb60e7)
  immediately followed by its child `windmill_roll_0.2_trans` (0xb6ed3), then the remaining direct children in the order above.

### 2.4 Representative level

`REPRESENTATIVE E11_KF_1` (rule coded in the test: affected, no other first-failure reason, 0 missing textures, fewest nested
nodes, then name; all 18 tie at 1 nested node, so the first by name). Resources resolved: `Models\bg_tree04\Windmill_02`,
`Models\bg_tree04\^Windmill_03` and the other `bg_tree04` materials (`^Bridge_Fence`, `~Mist`), `Models\bg_tree04\light02`;
0 missing. The other 17 levels share the identical tree class and the same resource set, but this report does **not**
promise that all convert or that they render identically.

## 3. Implementation contract (for the proposed next task; not approved here)

Real route: `runApplication --level E11_KF_1` -> `runApplicationLeg` (src/app/main.cpp) -> `AssetLibrary` read of
`COMPILED\E11_KF_1.LVL` -> `BackdropScene::build` (src/app/BackdropScene.cpp:63) -> `renderStage(Secondary)`
(BackdropScene.cpp:184) -> `dispatchSecondaryChildren` (src/app/SceneDispatch.cpp) -> `drawMesh` / `drawBillboard` /
`drawSprite` -> `rasterizeSceneTriangle`.

1. **Admission (`BackdropScene::build`, the `nested/non-leaf` guard at BackdropScene.cpp:135-138).** Replace the blanket guard
   with a structural predicate: Secondary root must be type 0 (unchanged); every node must be type 0, 1 or 2 (unchanged);
   a node may have children only if it is type 0 or type 1; a **type-2 node with children is rejected** with the existing
   `unsupported ...` diagnostic (no installed level has one; the double-matrix behaviour E8 is documented but not
   implemented); depth limited by `SceneNode.cpp` `kMaxDepth`. No level-ID or offset table.
2. **Dispatch (`src/app/SceneDispatch.{h,cpp}`).** Order the root's direct children with the exact binary-insertion
   algorithm of E10 (new function, tested against the E11 order above and against `std::sort` on tie-free input), then emit
   for each ordered child the child followed by its descendants depth-first in file order. `SecondaryDispatchItem` gains
   `depth` and `parentOffset`. Keys are unchanged (`composed` translation . `sourceRotation` column + `child.headDoubles[1]`).
3. **Visibility.** A node whose `+0x110` is 0 emits nothing and its subtree is skipped. Type 0 and type 1 default to 1;
   type 2 uses its serialized byte (this also makes branch-0 billboards honour the byte, which BackdropScene.cpp:187 ignored;
   no installed level has a hidden type-2). Runtime LOD (E4): `cutme`-named type-1 nodes are visible unless the clone's frame-time
   rule (more than 20 consecutive frames over 80 ms with `[G+0x1C9B3]==0`) is connected; the integration keeps them visible and
   records that limit (no installed target level contains a `cutme` node).
4. **Type-1 subtree cull (E7).** Compute `z` = camera-space depth of the node origin (the same quantity as the sort-key z
   before adding `+0x108`) and skip the node **and its children** if `z + headDoubles[0] <= 0`. Apply it to every type-1 node as
   the original does; the acceptance check requires the ten originally admitted levels to stay byte-identical, and if any other
   change appears the rule is restricted to nodes with children and the report states the restriction.
5. **Meshes and billboards.** `buildSceneMeshSource` and `collectEyes` already compose nested matrices
   (SceneMeshSource.cpp:39,83; BackdropScene.cpp:40) and key records by `nodeSourceOffset`; the change is only that nested
   offsets must now be dispatched. `collectBillboards` needs no change (type-2 parents with children are rejected). The
   kind-5 facing predicate keeps its existing nested-aware `eyes` map.
6. **Preserved:** accepted branch-1 sprite behaviour, `conditionalCaretFlags8Policy`, render states, 640x480 fixed camera,
   all existing assertions and timeouts; superseded assertions are expected only in the tests that currently assert the
   18-level diagnostic (`test_backdropscene.cpp`, `test_sceneadmission*.cpp`, `test_sceneintegration.cpp`,
   `test_scenebranch_impl.cpp` counts "84 admitted / 18 empty"); the task must list each exactly.
7. **Admission boundary summary:** admitted: type-0 root, nested type-0/1 parents, type-1 and type-2 leaves, type-2 branch
   0/1 as today. Rejected with a level-qualified diagnostic: type-2 with children, any other node type, multi-image sprite
   forms, missing sprite/texture assets. No installed level is rejected for a new reason by this boundary (section 2.1).

### Planned verification for that task (none of this has been executed)

* **Authentic pre-change snapshot:** frozen copies of `BackdropScene.cpp/.h`, `SceneDispatch.cpp/.h`, `SceneNode.cpp/.h`,
  `main.cpp` (SHA-256 recorded in the supervisor entry manifest and re-checked by an identity CTest) under ignored
  `build/<task>/frozen/`, built into a "before" driver with the same technique as `scenebranchimpl_before`.
* **Public-route before/after on E11_KF_1:** `runApplication --level E11_KF_1 --sim` under SDL dummy drivers; before = one
  level-qualified `unsupported nested/non-leaf secondary dispatch` diagnostic and an empty partial scene; after = admitted,
  per-role raster calls and accepted colour/depth stores for Primary / LegacyUnattributed / Secondary, the nested submission
  listed separately (`windmill_roll_0.2_trans` records at 0xb6ed3: 5 kind-7 records), scene-on versus scene-off RGB and depth
  artifacts and PNGs under `build/`, identical dispatch order to `trees/E11_KF_1.plan.txt`, two renders byte-identical.
* **Mutation checks:** reverting to `std::sort` changes the tied-pair order (and pixels only where tied nodes overlap);
  hiding `cube1_trans` removes the windmill body and its child; forcing visibility 0 on the child removes only the blades.
* **Preservation:** all 84 currently admitted levels render byte-identical RGB+depth for the 10 originally admitted levels
  and keep `MM_CLIMB_002` (Primary 923/0, Legacy 172/0, Secondary 82/17, 1177 records, 17 billboards, primary hash
  `0x1FD0BFB82AB39E95`) and `MM_DIG_000` (19 fire submissions, 15 drawn, 4 culled, 975/336/628 raster calls with injected
  generator state 1); the 74 branch-1 levels are re-rendered with the new tie order and **any pixel change is reported
  as an intentional consequence of E10, not as a regression**; census becomes 102 admitted (or exact remaining reasons).
* Existing timeouts unchanged; no claim of pixel agreement with an original render.

## 4. Unresolved, non-blocking limits (explicit)

* **Animation.** The child is named `windmill_roll_0.2_trans`; the original probably animates it. Animation-time matrices
  remain an unresolved prior limit (scene-roots-001); the integration draws the serialized pose only and claims nothing about
  blade rotation.
* **Clipped type-1 draw** (`0x469EAA`, straddling the near plane) is not reconstructed; the existing per-vertex rejection stays
  (clipping park). The cull of E7 is independent of it.
* The `cutme` frame-time LOD (E4) is evidenced but not connected (needs the clone's frame-time input and the meaning of
  `[G+0x1C9B3]`, untraced); no target level contains such a node.
* The `0x4319E5` hide-all routine (E3) has no static caller; treated as unreachable on the scenery route.
* Type-2 children (double matrix push, E8) are evidenced but unexercised; implementing it without an installed example would
  be untested, so admission rejects it.
* Sort-key camera inputs for gameplay frames are the existing fixed-camera conditions; ties were measured under the default
  camera only. Tie *existence* under moving cameras is not claimed.
* No original render exists; nothing here proves pixel agreement. C3 stays FAIL.

## 5. Proposed bounded production task (for the planner)

`scene-nested-impl-001` - milestone step "select an installed level and render its scenery during play". Level E11_KF_1,
route above. Observed failure: the level yields a successful empty partial scene with one diagnostic
(`E11_KF_1 Secondary paired-secondary[0] source=0xb33dc: unsupported nested/non-leaf secondary dispatch`), reproduced
in the census (`build/scene-branch-impl-001/after/measurements.txt` line 6). Changes: items 1-5 of section 3, with the E10
order replacing `std::sort`. Production files: `src/app/BackdropScene.cpp`, `src/app/SceneDispatch.{h,cpp}`; test and CMake
registration additions; no change to parsers, assets or `main.cpp` unless the demonstration needs a hook. Dependency order:
this is the next scenery dependency; afterwards priority 2 (selected-hatch camera orientation for other levels) is unaffected.

## 6. Reproduction notes

`rea_static` was used only for `open_binary` (registered path, provider ghidra), `read_bytes` (vtables `0x4B0948`, `0x4B1990`,
`0x4B19A0`, `0x4B1AD8`; string area `0x4BFEA0`) and then `close_binary` without a snapshot. All instruction evidence was read
from the retained disassembly and cross-checked against the vtable bytes read from the loaded image.

## 7. Verification and changed files

* `python tools/agent_control/verify.py --profile main`: reference SHA verified; **98/98 registered tests passed**
  (97 before + `scenenested_measure`), 545 s. `python tools/agent_control/verify.py --profile recovery`: **84/84 passed**,
  30.6 s. Both were run separately (a single `--profile all` run was not used because the main profile alone consumes
  ~9 of the 10 allowed minutes). All existing assertions and timeouts unchanged.
* Changed files: `src/__test__/test_scenenested_measure.cpp` (new, read-only), `CMakeLists.txt` (one `add_test` block in the
  existing `WEBOLUTION_GAME_DIR` guard after `scenebranch_measure`, timeout 120 s), `docs/scratch/scene-nested-research-001.md`.
  Ignored outputs: `build/scene-nested-research-001/**`.
* Not run: anything involving the original executable, hooks, debugger, capture, installation or save writes. `git show
  HEAD:src/app/main.cpp` was not needed (no frozen-source comparison in a research task).
