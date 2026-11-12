# scene-branch-research-001: type-2 branch-1 scenery drawing and nested Secondary dispatch

2026-10-31. **Research only; status needs_review.** No production C++, test, CMake,
queue, control or completion file was changed. No original executable, copied original
instructions, oracle, hook, debugger, capture, launcher, installation or save write was
used. The executable and exports were read as bytes/text only. This is static research
acceptance, not runtime implementation, rendering acceptance, original fidelity or natural
gameplay completion.

**Result in one line:** the branch-1 *load* contract and the *placement/geometry* half of
the branch-1 draw are address-established; the *final sprite selection and draw state* half
is not. Critical gaps remain (section "Critical gaps"), so a production implementation is
**not approved** by this pass. The 18-level nested-dispatch category is a separate follow-up.

## Identity and inputs

| Item | Value |
| --- | --- |
| Reference executable (read as bytes) | `Lemmings Revolution/Lemmings Revolution.exe`, PE32 i386, SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f` (re-hashed in this run; equals `build/save-research-input/reference.json`) |
| Retained disassembly | `build/save-research-input/reference-disassembly.txt`, SHA-256 `36bc7d4522fcfac733e73080246c43d85126463cc39d7f595fac0b72c8cff9b1` (re-hashed, matches) |
| Retained headers | `reference-headers.txt`, SHA-256 `2b25e3794add3210ffa0ca2fe2e1a277950c0ea27084210262c24dadb74adb4b` (re-hashed, matches) |
| Newly read this run | disassembly text at the addresses below (instruction lines quoted); `build/scene-admission-001/census.txt`; text/regex views of the installed-level copy `build/dig.lvl` (1,288,566 bytes, equals the census size for `MM_DIG_000`) |
| Retained, not re-measured | `docs/scratch/scene-roots-001.md`, `scene-admission-001.md`, `phase-c3l-scene-reads.md`, `phase-c3y/c3bd/c3r`, `phase-c1m-sheet-file-read.md`, `decomp-lemmings/docs/evidence/object_base_prefix.md`, `src/assets/Sprite.h` |

Disassembly line numbers are lines of `reference-disassembly.txt`. Addresses are virtual
addresses. `this` is ECX for these thiscall methods. "node" is the type-2 object; "sub" is the
embedded sub-object at **node+0x118** (the reader passes `ecx = node+0x118`, e.g. `0x47B572`).

## 1. Serialized form (retained plus re-read)

Reader `0x0047B44C` (disasm 160304..160424; vtable `0x4B1AD8` slot +0xC, retained). Order:

| Step | Address | Effect |
| --- | --- | --- |
| clear flag | `0x47B461` | `byte [node+0x350] = 0` (see "random frame" below; reset on every load) |
| 3 x f64 | `0x47B491/A5/BF` (`call 0x402580`, `fstp`) | `node+0x320/0x328/0x330` (A, B, H) |
| u8 | `0x47B4C8..4DC` | normalized to 0/1 into `node+0x110` (**visibility**) |
| u8 | `0x47B4E5..4F9` | normalized to 0/1 into `node+0x338` (**branch**) |
| branch 0 | `0x47B515..5AE` | string (cap 0x104, `0x47D940`), `0x474519` name lookup, 4 x f64, `0x440C30`, `0x440C50` |
| **branch 1** | `0x47B5B0..5C0` | `push stream; ecx = node+0x118; call 0x468BF2` |
| both | `0x47B5C8..5DA` | u32 via `0x402560`, `call 0x406240` with `ecx=node+0x118`: stores the u32 at **sub+0x00** (`0x406240`: `mov [eax],ecx`) |
| base | `0x47B5F9` | `call 0x46BB73` (12 f64 matrix, name, 2 f64, child count, recursive children) |

Branch-1 reader `0x00468BF2` (disasm 134928..134975), `ecx = sub`:

| Serialized item (in file order) | Instruction evidence | Stored at |
| --- | --- | --- |
| u32 #1 | `0x468C04 call 0x402560` -> `0x468C0F mov [ecx+0x1b0],eax` | sub+0x1B0 (node+0x2C8) |
| u32 #2 | `0x468C18` -> `0x468C23 mov [edx+0x1c8],eax` | sub+0x1C8 (node+0x2E0) |
| u32 #3 | `0x468C2C` -> `0x468C37 mov [ecx+0x1cc],eax` | sub+0x1CC (node+0x2E4) |
| u32 #4 | `0x468C40` -> `0x468C4B mov [edx+0x1b8],eax` | sub+0x1B8 (node+0x2D0) |
| string, capacity 0xFF | `0x468C51 push 0xff`, `0x468C60 call 0x47D940` (u32 length then bytes, bounded by capacity) | local buffer |
| registry lookup | `0x468C6C mov ecx,0x58D23C`, `0x468C71 call 0x449F20`; if absent `0x468C8B call 0x446040`; then `0x468C9C call 0x4050F0` | `sub+0x1FC = index` (`0x468CA7`) |
| handle | `0x468CB3 mov dword [edx+0x1f8],0` | sub+0x1F8 = 0 (lazy handle, see below) |

So a branch-1 type-2 record is: `u32 type(2)`, `f64 A,B,H`, `u8 visibility`, `u8 branch=1`,
`u32 f1,f2,f3,f4`, `u32 len + len bytes name` (**no** four UV doubles, unlike branch 0), `u32
trailing`, then the common base. This matches `SceneNode.cpp:157-158` (rejects branch != 0 at the branch byte
offset, which is why the census reports `0xa7572` as the branch byte) and the retained
`object_base_prefix.md` statement (installed data: branch 0 x 398, branch 1 x 1,396).

The registry `0x58D23C` is the SPR sprite-sheet registry: retained `phase-c1m-sheet-file-read.md` records
`0x446040(0x58D23C, name)` as a name-only registrar (`0x449170`) with the file opened lazily
(`0x484EB0`), and `old/docs/scratch/phase3-round23-axe.md` registered `Axe` through the same call
(`push 0x4C0D6C; mov ecx,0x58D23C; call 0x446040`). `0x4050F0` (disasm 6085..6124) is a linear
search of the entry table at registry+0x30, skipping entries whose byte at +0x84 is 0, comparing
by `call 0x49C870` against the entry name (`0x403720`). It returns the entry index or the table count.
**Inference:** `0x49C870` is a CRT string compare (callee not read); case sensitivity is unknown.

Lazy resolution: `0x468833` (disasm 134665..) tests `[sub+0x1f8]==0` and, when zero, stores
`0x42FF90(0x58D23C, [sub+0x1fc], 0)` into `sub+0x1F8` (`0x468859..468861`). `0x46836D/0x4683B1` is the
setter variant. Callers of `0x468833` seen: `0x43CF48`, `0x45AFE0`, `0x476568`, `0x495400`. `0x476568`
loops a list at `holder+0xCC` and calls it on `node+0x118` of each entry (disasm 153728..153748), i.e. a
holder-level resolve pass over type-2 nodes. The owner/entry of that pass and what `0x42FF90` returns
(the loaded sheet object, which `0x468D00` indexes through `[+0xC]`) were **not read**.

## 2. Draw routine `0x0047D0C0` (type-2 slot +4), disasm 162833..163254

Order of effects, with evidence:

1. **Visibility gate** `0x47D0D9..E3`: `movzx cl,[node+0x110]; test; jne` else `jmp 0x47D866` (return,
   nothing drawn, children also skipped). The current source ignores `sourceVisibility`
   (`BackdropScene.cpp:187`); all 17 installed branch-0 records have visibility 1 (retained), so this is
   not yet an observed difference, but branch 1 must honour it.
2. **Matrix push and camera-space centre** `0x47D0E8..0x47D126`: `0x43BC10(node+8)` (push node matrix),
   a local 12-double matrix built by `0x412C40/0x46BE80`, combined with the global camera object `0x58D498` through
   `0x43BCA0`, copied by `0x43BC60`. Result is the camera-space centre `P` (vector class, data pointer via
   `0x412C20`). This is the same chain C3af reproduced for branch 0 (`buildBillboardCameraChain`).
3. **Four corners**, constants `0x4B03F8` (= 2.0, retained note `old/PLAN.md:2905`; the value itself was not re-read from the image): with `A=[node+0x320]`,
   `B=[node+0x328]`, `H=[node+0x330]`:
   corner0 `P + (-A/2, -H/2, -A/2)` (`0x47D158..1BE`), corner1 `P + (-B/2, +H/2, -B/2)` (`0x47D1E3..249`),
   corner2 `P + (+B/2, +H/2, -B/2)` (`0x47D268..2D4`), corner3 `P + (+A/2, -H/2, -A/2)` (`0x47D2F9..35F`).
   Identical to `SceneBillboardSource.cpp:70-71`.
4. **Branch-1 only: `0x47D379..0x47D47D`** (`cmp byte [node+0x338]`; `je 0x47D47D` skips for branch 0):
   a. **First-draw random frame.** If `byte [node+0x350]==0` (`0x47D391..399`): `call 0x4040A0`
      (disasm 4722..4731: `call 0x49C84A` then `fild`, `fdiv [0x4B03B0]`; i.e. a CRT-call-derived value
      scaled by a constant, **inference:** `rand()/32768.0`, neither `0x49C84A` nor the constant were read);
      `ecx=[node+0x1f8]` (`0x41A500`), index `[node+0x2E0]` (field #2) -> `0x41A4E0` returns
      `table[+0xC] + idx*0x98`; `0x45B8F0` returns `[that+0]` (an entry count); then
      `node+0x2E4 (field #3) = ftol(double(count-1) * r)` via `0x49C5AC` (`0x47D3C4..3F2`), and
      `byte [node+0x350] = 1` (`0x47D3F8`). Consequence: **field #3 as serialized is overwritten on first
      draw by a pseudo-random value in `[0, count-2]` (never `count-1`)**. Whether `srand` is called and with
      what seed is not established; the original frame choice is therefore not reproducible from the file.
   b. **Anchor shift.** `0x47D40B..477`: `H/2` is added to the y component of **all four** corners
      (locals `-0x180/-0x168/-0x150/-0x138`). Branch 1 is therefore bottom-anchored (y range `[0,H]`);
      branch 0 stays centred (`[-H/2,+H/2]`).
5. **Projection** `0x47D47D..0x47D4F8`: each corner through `0x40EAE0(out,in,0x58D438)` (the projection matrix
   already modelled by `projectionTransform`).
6. **Screen slot formulas** `0x47D4FB..0x47D613`: identical to branch 0 and to `SceneBillboardSource.cpp:89-99`
   (shared `1/p0.z` and `1/p1.z`, crossed Y sources, centre/offset in `[0x58D294]/[0x58D298]`; x slots
   `node+0x138/1A0/208/270`, y slots `node+0x140/1A8/210/278`).
7. **Tint copy** `0x47D61F..0x47D641`: 16 bytes from `node+0x340` to globals `0x58D2A0..AC`, both branches.
   Initial `node+0x340` comes from constructor `0x4821D0` (unread); after the draw `0x47D828..856` overwrites
   the globals with the object built by `0x404D40(0,0,0,0)`.
8. **Branch 0 tail** `0x47D65D..0x47D6DA`: store z values `p0,p1,p2,p3` into `node+0x130/198/200/268`,
   then `0x414920(ecx=node+0x118)` (textured quad from the UV rectangle written by `0x440C50`).
9. **Branch 1 tail** `0x47D6DF..0x47D81B`:
   * `0x46840B(ecx=sub, arg = camera-matrix data pointer from 0x412C20(local))` (disasm 134344..): with
     `[sub+0x1f8]`, field #2, field #3 it fetches the animation entry (`0x468D00`, then `0x468CE0`:
     `table[+0xC] + idx*0x10`), tests its element count (`0x468CD0 > 1`, `jle 0x46881E`), and if more than one
     computes `sin/cos` (`0x426DB0/0x426D90`) of an angle built from `[sub+0x1D8]` plus constants `0x4B1950/0x4B1948`
     and the vector `sub+0x1E0..0x1F0`. This is consistent with selecting among **directional images**. The body
     after `0x46850B` was not read. Not established.
   * z permutation `0x47D6F9..0x47D76B`: `node+0x130 = p1.z`, `0x198 = p0.z`, `0x200 = p3.z`, `0x268 = p2.z` (branch 0
     stores `p0,p1,p2,p3`). The slots are permuted consistently with the crossed Y sources; they differ in value
     only if `A != B` (z offsets `-A/2` versus `-B/2`).
   * `0x47A565(ecx=sub, 5 doubles)` (`0x47D771..81B`): the five doubles are, in callee order
     (`[ebp+8..0x28]`): `a0 = (x0+x1+x2+x3)/4.0` (`0x4B02C8`), `a1 = v1.y` (`node+0x1A8`),
     `a2 = x2 - x1` (`node+0x208 - 0x1A0`), `a3 = x3 - x0` (`node+0x270 - 0x138`), `a4 = y1 - y0`
     (`node+0x1A8 - 0x140`). It is a **screen-space sprite rectangle**, not a textured 3-D quad.
10. **Sprite frame lookup and quad fill** `0x47A565..0x47A75A` (disasm 159058..159212), `ecx=sub`:
    * `X = 0x47ABC0( 0x468CE0( 0x468D00([sub+0x1F8], field#2), field#3 ), field#4 )` =
      `((sheet.table[+0xC] + f2*0x98).table[+0xC] + f3*0x10).table[+0xC] + f4*0x48`. The three strides give a
      three-level hierarchy that matches `Sprite.h`: clip (`Sprite`, 0x98 bytes) -> temporal entry
      (`SpriteAnimation`, 0x10) -> directional image (`SpriteFrame`, 0x48). So **#2 = clip index, #3 = temporal
      entry index (randomised on first draw), #4 = directional image index**. The strides/names are address-backed;
      the identification with `Sprite.h` is inference from matching shape only.
    * **Field #1 is a horizontal mirror flag:** `cmp [sub+0x1B0],0` (`0x47A5AE`); zero and non-zero push the
      same four frame doubles `X+0x10, X+0x18, X+0x20, X+0x28` to `0x440C50` with `X+0x10` and `X+0x18` swapped
      (`0x47A5B7..5E6` versus `0x47A5ED..61C`). `0x440C50` (disasm 84097..84143) stores arg0 to
      `sub+0x98,+0x30`, arg1 to `+0x168,+0x100`, arg2 to `+0x108,+0xA0`, arg3 to `+0x170,+0x38` (each value
      is an 8-byte double written as two dwords).
    * Screen rectangle: `a0 += (double)int[X+0x30]`; `a1 -= (double)int[X+0x34]` (`0x47A621..636`); vertex x
      values `sub+0x20/0x88/0xF0/0x158` are `a0 -/-/+/+ a2/2 or a3/2` each less `int[X+0x38]/2` (`0x47A639..6BC`);
      `sub+0x28` is `a1 - int[X+0x3C]/2 - a4/2`, plus `int[X+0x3C]/4.0` only when the global dword `[0x58D53C]==0`
      (`0x47A6C2..703`). `[0x58D53C]` is a device/mode global whose meaning is **not established**. `sub+0x28`
      is mirrored to `sub+0x160` and, plus `a4`, to `sub+0xF8`/`sub+0x90` (`0x47A703..745`).
    * `0x47AB96(sub, X)`: `0x440C30(sub, [X+0x40])` stores the piece's texture id in `sub+0x1A8`
      (`0x440C30`: `mov [eax+0x1a8],ecx`), then `0x414920(sub)` is called (shared with branch 0).
11. **After both branches** `0x47D820..0x47D861`: tint globals reset (above), `0x46B6CC(node)` (draw children,
    see section 4), `0x43BBE0` (matrix pop).

Not read, and needed to draw branch 1: `0x414920` (the quad draw: blend, alpha test, depth write, filter,
address mode, texture binding), the second half of `0x46840B`, the remainder of `0x47A75D..` (a second variant
at `0x47A75D` that also reads `[0x58D53C]` and `[0x58CB38]`), `0x42FF90`, `0x484EB0` and the SPR loader consumers.

## 3. What can be reused versus what is new

Reusable, already accepted: the camera/projection chain, corner offsets, slot formulas, `SceneRaster`,
`conditionalCaretFlags8Policy` (branch 0 only), `SpriteSheet`/`loadSpriteSheet` (`src/assets/Sprite.h`),
descending-key `SceneDispatch`.

New for branch 1 (nothing below exists in source): reading the four u32 and the name in `SceneNode`
(`SceneNode.h` has no fields for them); the y-anchor shift; the permuted z; SPR sheet lookup by name; frame
selection (`f2`, `f3`, `f4`, mirror, directional choice); the `X+0x30..0x3C` offsets; texture/material state
for sheet textures (the C3r caret rule is keyed to branch-0 resource names and must not be applied by assumption).

## 4. Nested Secondary dispatch (18 levels): what is and is not established

Established:

* Root slot 8 (`0x46B73F..0x46BADF`, retained C3y) pushes the root matrix (`0x46B753..759`, `call 0x43BC10`),
  sorts only the root's **direct** children by a camera-composed key plus `child+0x108`, and calls each child's
  slot +4 (`0x46BAD1`). It does not draw the root's own geometry.
* Child traversal helper `0x46B6CC` (disasm 139171..139210; called from the type-2 draw at `0x47D85C`): gate on
  `byte [this+0x110]` (`0x46B6DA`, return if 0); `0x43BC10(this+8)` push; for `i in [0, count(this+0x68))`
  call vtable slot +4 of child `i` in **vector (file) order**, no sort (`0x46B715..734`); `0x43BBE0` pop. So a
  type-2 node draws itself and then its nested children, in file order, inside its own matrix.
* Census qualification (from `build/scene-admission-001/census.txt`): the 18 "nested/non-leaf" levels are
  levels whose Secondary tree **parsed completely** (no branch 1; the nested check runs after `readSceneNode`
  succeeds, `BackdropScene.cpp:75-78`). For those 18 the nested check is the sole recorded blocker, independent
  of branch 1. The 74 branch-1 levels stop at their **first** failing node, so some of the 74 may also contain
  nested children; the 74/18 split is a first-failure partition, not exclusive categories.

Not established (follow-up, not part of this pass): the type-1 node's slot +4 (does it call `0x46B6CC` for its
nested children, and does it honour `+0x110`?), whether nested children are positioned in the parent's matrix by
`0x43BC10` composition only, tie order for equal sort keys, and ordering between sorted direct children and nested
unsorted children. Traversal order is therefore partly known (type-2 children: file order, matrix-pushed, parent
visibility gated); the unresolved item is the type-1 side and whether any separate unresolved rendering dependency
(for example animated matrices) applies. Nothing here supports "implementing one rule unlocks every level".

## 5. Installed MM_DIG_000 records and the current rejection path

From `build/scene-admission-001/census.txt` lines 358..362 (retained, produced by the accepted
`test_sceneadmission_census` on the production readers):

| Role | typed root | tree end | status |
| --- | --- | --- | --- |
| Primary `paired-primary[0]` | `0x5988D` | `0x98F70` | ok, 4 nodes, 1156 mesh records, 0 billboards |
| Secondary `paired-secondary[0]` | `0x98F84` | `0xC3056` | **unsupported: `unsupported nonzero type-2 branch 1 at 0xa7572`**, parse stops there |
| Legacy Entrance `entities_425e50[0]` | `0x12CB5A` | `0x12D9B4` | ok, 4 nodes, 16 records |
| Legacy Exit `entities_426c6b[0]` | `0x130B66` | `0x13A8CC` | ok, 2 nodes, 156 records |

Current path: `BackdropScene::build` -> `readSceneNode(bytes, 0x98F84)` -> `SceneNode.cpp:157` fails -> `formFailure`
marks it unsupported -> `roots_` cleared -> successful empty partial scene with one diagnostic naming `MM_DIG_000`,
`Secondary` and `unsupported nonzero type-2 branch`. The branch byte is at `0xA7572`, so the first branch-1 node starts at
`0xA7555` (`0xA7572 - 1 vis - 24 leading doubles - 4 type`), with branch-1 payload at `0xA7573`.

Retained scene-roots-001 reports 19 branch-1 leaves in `MM_DIG_000`'s Secondary tree (not re-verified here).
The name list at the head of the level (`Bubbles`, `fishes`, `Bits`, `Smoke`, `Axe`, ...) is the level's
sheet table; text search of `build/dig.lvl` finds `Bubbles`/`fishes` only in that table (lines 4, 6, 8 of the
newline-split view), **not** inside the Secondary tree region. The branch-1 resource names and the four u32
values per record were therefore **not extracted**: the permitted tools (Read/Grep, two fixed Bash commands) cannot
produce byte-addressed hex, and adding a registered measurement is outside this task. Secondary-region text in the
`dig.lvl` view shows material names `Models\bg_mine01\{^WaterFall01,~Water01,~Light01,~Shadow,~Crystal01,
WaterRef01/02,~Splash01,^Vines01/02}` (type-1 material references). No claim is made about which of these, if any, relate
to the sprite names.

## 6. Supported and unsupported forms (proposed future boundary)

| Form | Status |
| --- | --- |
| Type-2 branch 0, existing | supported today, unchanged |
| Type-2 branch 1 **load** (4 u32, name <= 255, trailing u32, children) | contract established above; trivially parseable |
| Type-2 branch 1 **draw** | **not approved**: final draw state, directional selection, sheet texture state and registration-offset arithmetic incomplete |
| Visibility byte 0 | draw skipped, children skipped (`0x47D0E3`, `0x46B6DA`); established |
| Random temporal entry | nondeterministic in the original; reproduction needs an injectable policy |
| Nested Secondary children | traversal for type-2 parents established, type-1 side not |
| Everything else (type 3+, kinds 2/3/4, model kind 4) | unsupported; keep diagnostics |

## 7. Critical gaps (block implementation approval)

1. `0x414920` and the code reached from it (blend/alpha/depth/filter/binding for sheet frames) not read.
2. Second half of `0x46840B` (directional image selection from camera angle) not read; without it, frames with more
   than one directional image cannot be chosen.
3. Sprite sheet loader consumers for branch-1 frames (`0x42FF90`, `0x484EB0`) and the resolve-pass owner (`0x476568`'s function) not read.
4. The actual installed `MM_DIG_000` branch-1 values (4 u32 plus name per record) not extracted; unknown whether names exist as `SPRITES\*.SPR`.
5. Original frame choice is random; PRNG seed/`0x49C84A`/`0x4B03B0` unread. Animation advance of `f4` (or `f3`) between frames, if any, is not
   in the draw routine and was not located.
6. `[0x58D53C]` and `[0x58CB38]` (used by `0x47A565`/`0x47A75D`) not identified.

## 8. Draft implementation contract (conditional; NOT approved)

This is the shape of the production task to queue **after** gaps 1-4 are closed by one more bounded static pass
(read-only; first item is a registered read-only measurement of `MM_DIG_000` branch-1 records, which needs a
test registration and so is outside this research task). It is a plan, not a result.

* Files: `src/assets/SceneNode.{h,cpp}` (retain branch-1 fields `f1..f4`, name; keep the offset and bounds checks);
  `src/app/SceneBillboardSource.{h,cpp}` (branch-1 geometry: y-anchor shift, permuted z, five screen doubles);
  `src/app/BackdropScene.cpp` (draw branch-1 via sprite frame lookup; honour visibility; nested type-2 children in file order);
  `src/assets/Sprite*` read-only reuse; unit/demonstration tests in `tests/` and `CMakeLists.txt` registrations.
* Admission: remove only the branch-1 rejection (not a loosening for other forms); keep the nested-dispatch check until its own task.
* Randomness: inject a deterministic selector policy; do not claim original agreement for the chosen entry.
* MM_DIG_000 acceptance (planned, not run): fixed 640x480 camera as in `build/scene-admission-001/after/fixed-camera.json`; before =
  empty partial scene with the single diagnostic (reproduced by the frozen-source technique of scene-admission-001); after = Secondary
  roots admitted, per-role raster calls and accepted colour/depth stores reported separately for Primary, LegacyUnattributed and Secondary
  (branch-1 submissions listed apart from mesh submissions), missing sheets/textures counted, scene-on versus scene-off RGB artifacts and
  PNGs written under `build/`, two renders byte-identical, plus a `runApplication --level MM_DIG_000 --sim scenario` route under SDL dummy
  drivers that exits through the observer-queued `SDL_QUIT`.
* Preservation (planned): the ten admitted levels (`E06_KF_1, H17_MG_1, M06_KF_1, M08_KF_1, MM_BASH_002, MM_CLIMB_002, MM_E1_3_SKILLS,
  MM_E1_4SKILLS, MM_W_012, VH22_JC`) render byte-identical RGB+depth to the accepted artifacts; `MM_CLIMB_002` keeps Primary 923/0, Legacy 172/0,
  Secondary 82/17, 1177 records, 17 billboards and the `0x1fd0bfb82ab39e95` primary hash; existing timeouts unchanged.
* Out of scope: digger gameplay, D1/D3, clipping/controller/capture parks, C3 FAIL, native-006 deferral, oracle-001.

## 9. Unknowns, limitations and next action

* Unknown: everything in section 7; also the meaning of the trailing u32 at `sub+0` (consumer unread; the source calls it `blendSelector`
  by inheritance from earlier work, not re-derived here) and of the `node+0x340` tint object.
* Not claimed: original fidelity, any rendered pixel for `MM_DIG_000`, that any other branch-1 level becomes supported, or that the 74/18 census
  will convert.
* **Next action:** one more bounded static pass (read `0x414920`, `0x46840B` remainder, `0x42FF90`, the `0x476568` resolve-pass function,
  `0x47A75D`, `0x49C84A`) together with a registered read-only measurement of `MM_DIG_000` branch-1 records; then plan the production
  branch-1 task. Nested dispatch (18 levels) is a separate follow-up that can proceed independently: the 18 trees already parse, so it needs only
  the type-1 slot +4 reading (section 4).

## Verification

Executed verbatim from the repository root, once, after both reports were written:
`python tools/agent_control/verify.py --profile all`. Observed output: reference SHA-256 verified by reading
bytes only; **main 92/92 passed** and **recovery 84/84 passed** (0 failed); the final registered
`git diff --check` ran and printed nothing. No production file, test, CMake file, queue or control was
changed, so these results show that the repository still builds and its registered tests still pass; they
do not test any claim in this report (static reading only). The supervisor's independent verification and
the separate read-only review remain required. Not run: anything involving the original executable, copied
original instructions, hooks, debugger, capture or installation/save writes.

Changed files (non-ignored): `docs/scratch/scene-branch-research-001.md` only. Ignored:
`build/scene-branch-research-001/report.md`. The `git show HEAD:src/app/main.cpp` read was not needed.
