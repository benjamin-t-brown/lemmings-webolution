# scene-branch-research-002: MM_DIG_000 type-2 branch-1 drawing contract and measured records

2026-10-31. **Research task; status needs_review.** Static reading of the retained disassembly,
byte reads of the reference executable, and one new registered read-only measurement
(`scenebranch_measure`). No production C++ (parser, renderer, scene, app), queue, control,
permission, verifier, completion or tool-registry file changed. No original executable,
copied original instruction, oracle, hook, debugger, capture, launcher, installation or save
write was used. This is research evidence, not implementation, rendering acceptance or
original-fidelity proof; it continues (does not replace) `scene-branch-research-001.md`.

## Result

The branch-1 load, geometry, frame selection, animation advance, texture/material state and
blend state for the **actual MM_DIG_000 records** are now address-backed and measured.
All 19 branch-1 records in `MM_DIG_000`'s Secondary tree use one sprite sheet (`fire`), one
clip, a texture whose name prefix selects flags 1, an additive blend selector (2) and a single
directional image per temporal entry, so the unread directional-selection arithmetic is
provably not on this route. A production contract is in section 10.

**Revision 2 (review feedback addressed).** The two items the review returned are now traced
from the retained disassembly: (1) the `srand` seed is `timeGetTime()` elapsed milliseconds
(import `0x4B0220`) since the global timer object `[0x4CBF84]` was constructed, so the
original's first-frame values are *wall-clock derived and not reproducible*; the contract now
uses a clock-derived seed with injectable state instead of a default state of 1 (section 5);
(2) the animation `dt` is `int_ms / 1000.0` (measured divisor `0x4B1758 = 1000.0`), i.e.
seconds, with a cap, a pause rule and a fast-forward burst count (section 5a). The diffuse
colour is proven white/opaque by the sprite-quad constructor (section 4a) and the `0x58D53C`
bracket is read and bounded (section 7). One input stays an explicit bounded limit: the
per-node `node+0x340` UV-offset pair (section 4a).

## Identities

| Item | Value |
| --- | --- |
| Reference executable (bytes only) | `Lemmings Revolution/Lemmings Revolution.exe`, 851,968 bytes, SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f` (the measurement test re-hashed it and asserts equality) |
| Retained disassembly | `build/save-research-input/reference-disassembly.txt` (line numbers below are lines of this file) |
| Level archive entry | `COMPILED\MM_DIG_000.LVL`, source `lemmings.box` (not overridden by `patch.box`), 1,288,566 bytes, SHA-256 `18a56a355a5b7eaf97cb13296bad9e14ad8d7e29772511de0d9ad9aedccc5c6e` |
| Sprite entry | `SPRITES\fire.SPR`, source `lemmings.box`, 1,388 bytes, SHA-256 `aeadd4fcd2d9a690bf7913a18dbb83bbd016d030fe676c41370a764fc290cc00` |
| Sheet texture | `TEXTURES\SHEETS\_FIRE0.BMP` exists in the effective archive |
| Measurement artifact | `build/scene-branch-research-002/records.txt` (ignored), produced by CTest `scenebranch_measure` |

`this` is ECX for thiscall methods. "node" is the type-2 object, "sub" is the embedded sub-object
at node+0x118 (so sub+X is node+0x118+X). All constants below were **read from the executable's
`.rdata`/`.data` bytes by the measurement** (section 9).

## 1. Evidence matrix: the six prior critical gaps

| Gap | Closed? | Evidence (addresses, disasm lines) |
| --- | --- | --- |
| 1 quad draw state at/below `0x414920` | **Yes for this route** (shared device-polygon submit left as a shared, branch-0-identical dependency) | section 4 |
| 2 directional logic of `0x46840B` | **Yes: provably not reached beyond its first check for MM_DIG_000** | section 3 |
| 3 loaders `0x42FF90`, `0x484EB0`; owner of `0x476568` | **Yes** | sections 2 and 5 |
| 4 installed MM_DIG_000 records and resolution | **Yes (executed measurement)** | sections 6 and 9 |
| 5 random frame, seed, temporal advance | **Yes (revision 2): seed = `timeGetTime` elapsed ms (wall clock, not reproducible); `dt` = ms/1000.0 seconds with measured cap/pause/burst constants** | sections 5, 5a |
| 6 globals `0x58D53C`, `0x58CB38` | **Yes** | section 7 |

## 2. Loading: name to sheet to frame tables (gap 3)

* Reader `0x47B44C` -> branch-1 reader `0x468BF2` (report 001, section 1) stores `sub+0x1B0 = f1`, `sub+0x1C8 = f2`,
  `sub+0x1CC = f3`, `sub+0x1B8 = f4`, the name (capacity 0xFF) via registry `0x58D23C`
  (`0x446040` register, `0x4050F0` index search), `sub+0x1FC = index`, `sub+0x1F8 = 0` (lazy handle).
  The reader normalises the branch byte (`0x47B4E5..4F9`) and the visibility byte (`0x47B4C8..4DC`) to 0/1, so
  **any nonzero branch byte is branch 1**; the installed bytes are all exactly 1.
* Sub-object constructor `0x4680F0` (disasm 134122..134195) sets: mode `+0x1D0 = 1`, `+0x200 = 0`,
  loop flag `+0x201 = 1`, heading `+0x1D8 = 0.0`, accumulator `+0x1C0 = 0.0`, `+0x1B0 = 0`, `+0x1FC = -1`,
  `+0x1B8 = 0`, `+0x1F8 = 0`, `+0x1C8 = +0x1CC = 0`, and the 3-double vector at `+0x1E0` = (0, 0, -1.0)
  (the pushed immediate `0xBFF00000` is -1.0 in the third component).
* `0x42FF90(registry, index, callback)` (disasm 60739..60793) is a reference-counted cache: slot =
  entry table (registry+0x30, stride 0x88, `0x4307A0`); if `slot+0x80 == 0` it calls the loader `0x4306F0`, stores the
  result in `slot+0x80`; otherwise it increments `[slot+0x80]+8`. It returns `[[slot+0x80]]` (the sheet object).
* `0x484EB0(name)` (disasm 173590..173660): allocates a 0x20-byte object (`0x485050`), `sprintf(buf, fmt@0x4C33C4, name)`,
  opens the file (`0x47DB73` stream ctor, `0x47E40D` open), on failure logs through `0x482A02` with the string at `0x4C33D4`
  and exits (`0x49D383(0)`), else calls the SPR reader `0x483EF0` (already recovered as `lemmings::parse_sprite_resource`).
  The executable's bytes at those addresses (measurement): `0x4C33C4 = "Sprites\%s.spr"`,
  `0x4C33D4 = "D:\lrpc\Source\Sprites\SpriteSheets.cpp"`. **So registry name `fire` resolves to the file `Sprites\fire.spr`,
  i.e. archive entry `SPRITES\fire.SPR`**; a missing file is a fatal error in the original (the clone must report it, not skip).
* Sheet accessors (disasm 134989..135026): `0x468D00(sheet, i) = table[+0xC] + i*0x98` (clip = `Sprite`), `0x468CE0(clip, j) =
  table[+0xC] + j*0x10` (temporal entry = `SpriteAnimation`), `0x468CD0(entry) = [entry]` (directional-image count),
  `0x45B8F0(clip) = [clip]` (temporal-entry count), `0x47ABC0(entry, k)` = directional image k at stride 0x48 (`SpriteFrame`).
  Clip `+0x10` is the 8-byte double read at `Sprite::rate`.
* Resolve/advance owner (disasm 153703..153749): function `0x4764C4(this=holder, double t)` computes `dt = t - [holder+0xE0]`
  (if `dt < 0.0` then `dt = t`; `0x4B0338 = 0.0`), stores `[holder+0xE0] = t`, then loops the list at `holder+0xCC` and calls
  `0x468833(sub = entry+0x118, dt)` (`0x476568`). `0x468833` is **both** the lazy sheet resolve (`0x46883C..468861`) **and the
  temporal-frame advance** (section 5). Its callers include scene holders (`0x43900F` via `0x438FCA`, `0x437C85`, `0x437D3C`, `0x450DF3` ...); the
  time value is a double the owner stores at `+0x120/+0x148` (setter `0x438F8A`, reached only through a vtable).

## 3. Directional image selection `0x46840B` (gap 2)

Disasm 134344..134664. Prologue/dispatch that matters here: it fetches the sheet (`[sub+0x1F8]`), the clip (`[sub+0x1C8]`),
the temporal entry (`[sub+0x1CC]`) and compares the entry's directional-image count with 1 (`0x468445..46844D`:
`cmp eax,1 ; jle 0x46881E`). **If count <= 1 the function does exactly one thing: `[sub+0x1B8] = 0`** (`0x46881E..468828`) and returns.
Therefore:

* serialized field #4 (stored at `sub+0x1B8` by the reader) is **dead input**: `0x46840B` always overwrites it before the draw
  (every path stores `+0x1B8`: 0 when count<=1, 0..4 when count==5 (`0x468621`), 0..7 otherwise); the draw reads `+0x1B8` back
  at `0x47A571`;
* for MM_DIG_000 all 19 records resolve to temporal entries with exactly **one** directional image (measured), so directional
  image index 0 is used and no camera-angle arithmetic runs.

For completeness (not needed for this route): when count > 1 the function rotates the vector at `+0x1E0..+0x1F0` about the Y axis
by angles `0.0625 + [sub+0x1D8]` and that plus `0.1875` (turns, `0x4B1950`, `0x4B1948`; `0x426DB0/0x426D90` are `cos/sin(2*x*K)` with `K` at `0x4B0F88`),
intersects with the camera direction passed by the caller (`0x480AC8`, `0x418350`, wrap by `2.0` at `0x4B03F8` against `-1.0`/`1.0`,
`0x4B0D68/0x4B02D0`), and picks 0..4 (count 5) or 0..7 (otherwise) by sign/magnitude comparisons against `0.0` (`0x4B0338`);
`+0x1B4` is a mirror byte set for part of the count-5 table. `0x418350`'s and `0x480AC8`'s bodies were not read, so **admission must reject
any record whose resolved temporal entry has more than one directional image** (section 8).

## 4. Quad draw state (gap 1)

Draw tail `0x47D6DF..0x47D81B` (report 001) calls `0x46840B`, then `0x47A565(sub, a0..a4)` (disasm 159058..159212) which fills the 4
vertices and the texture id and calls `0x47AB96` -> `0x440C30` (`sub+0x1A8 = X+0x40`) -> `0x414920(sub)` (disasm 26438..26503):

1. `0x414920`: texture entry = `0x414A10(0x58CB48, [sub+0x1A8])` (`0x414B90`/`0x414D00`: stride 0x88 table at +0x0C of the texture registry; the
   texture object is `[entry+0x80]`, ref-counted; created on demand through the registry's virtual factory `[vtbl+0xC]`).
2. `0x47261F(textureObject)` (disasm 148429..): material/texture state (already recovered in `phase-a4-actor-render-state.md`): skips if the
   object pointer equals cache `[0x5CD5F4]`; binds stage 0 (`SetTexture`, device `[0x58CDB8]` vtable `+0x98`); then selects a row by
   texture flags `[obj+0x24]`: bit 8 -> SELECTARG1 alpha, alpha test off; else bit 0x10 -> MODULATE alpha, test off; else if `flags & 3` ->
   **SELECTARG1(texture) alpha, alpha test ENABLED, GREATER, reference `[0x4C24E0]`**; else bit 4 -> MODULATE alpha; else SELECTARG1. Colour is always
   MODULATE(texture, diffuse).
3. `0x414D80(*(dword*)sub)` = the **blend selector, the first dword of the sub-object** = the node's trailing u32 (`0x47B5C8..5DA`,
   `0x406240`). Selector 0: blend off; **2: blend on, SRCBLEND=ONE(2), DESTBLEND=ONE(2)** (`0x414DC1..414DEA`); 3: SRCALPHA/ONE; other nonzero:
   SRCALPHA/INVSRCALPHA (`phase-a4-actor-render-state.md`). Changed-selector cache `[0x4C227C]` (image value 4).
4. `0x414B70(0x58D578)` resets a counter; the first-use guard `[0x5D6160]&1` runs one-time init `0x414F10`/atexit; `0x414A60(4)` sizes the array
   to 4; the four vertex pointers are stored as **(sub+0x08, sub+0x140, sub+0xD8, sub+0x70)** and `0x46D3C1(array)` submits the polygon
   (`0x4149A2..4149FD`). This is the same 4-vertex submit used by branch 0 and the actor path; the clone's existing fan
   `(0,3,2),(0,2,1)` over vertices v0..v3 = (sub+8, sub+0x70, sub+0xD8, sub+0x140) is the consumer. `0x46D3C1` is a shared, branch-independent
   dependency and was not re-read; nothing in the branch-1 path differs from branch 0 below the array.
### 4a. Submit `0x46D3C1`, diffuse and UV offset (revision 2)

`0x46D3C1(array)` (disasm 141625..141673, `0x46D460` 141674..141800) was read:
it converts the array to device vertices (float x `+0x18`, y `+0x20`, z `+0x10`, `rhw = 1/z` from `[0x4B19D8]`), writes
`diffuse = (ftol(v+0x50) << 24) | (ftol(color[0]) << 16) | (ftol(color[1]) << 8) | ftol(color[2])` with `color = v+0x38` (3 doubles), adds the two globals
`[0x58D2A0]`, `[0x58D2A8]` to the vertex's u and v (`0x46D596..0x46D5BC`), and draws with the device's vtable slot `+0x70` with primitive type **6 (TRIANGLEFAN)** and FVF `0x1C4` (`XYZRHW|DIFFUSE|SPECULAR|TEX1`) (`0x46D5DE..0x46D5F3`).
* **Diffuse is closed.** The sprite-quad constructor `0x440A10` (disasm 83886..84032) stores, for each of the four vertices, colour = (255.0, 255.0, 255.0)
  (`0x406FE000` high dword) at `sub+0x40/0xA8/0x110/0x178` and alpha = 255.0 at `sub+0x58/0xC0/0x128/0x190`; `0x47A565` does not touch them. Diffuse is therefore
  `0xFFFFFFFF` (opaque white) for every branch-1 vertex; the colour stage MODULATE(texture, diffuse) is texture-only. This also confirms the clone's white-diffuse convention for this route.
* **Primitive order.** A 4-vertex fan `(v0,v1,v2),(v0,v2,v3)` over `(sub+0x08, sub+0x70, sub+0xD8, sub+0x140)`. The existing renderer's `(0,3,2),(0,2,1)` is the same quad with reversed winding; keep it only if the existing cull policy requires, otherwise use the fan order and assert that both rasterise the same pixels.
* **UV offset `node+0x340` (partly closed, bounded).** The 16 bytes are two doubles added to every vertex's u and v. The type-2 node reader (`0x47B44C` region) and the no-op constructor `0x4821D0` (disasm 169484..169491) never store to `node+0x340..0x34F` in the retained disassembly (the only `+0x340` stores found belong to other classes at other bases), and the copy `0x47D61F..641` is shared with branch 0, whose accepted route already renders. After the draw the globals are reset to `0x404D40(0,0,0,0)`. **The original's source of the offset pair for a freshly read node was not located**, so the contract uses the same offset the accepted branch-0 route uses (zero) and labels it. Impact is bounded: it shifts the sprite's sampled texture window; for a zero offset the UV rectangle is exactly the measured `fire` rectangle.

5. Z-buffer, fill/cull, filter and address-mode states are **not** written anywhere in `0x414920`, `0x47261F` or `0x414D80`; they are the
   same unresolved effective-device inputs (`phase-c3q-scene-device-state.md`, "Still required") that every accepted scene draw already carries.
   The contract inherits the existing conditional policy and must label it so.

### Measured material facts for the MM_DIG_000 route

* `texture_name_flags` (`decomp-lemmings/src/texture_creation.cpp:21`): basename first char `_` -> flags **1**. `Sheets\_Fire0` -> 1. This is
  **not** the caret flags-8 row (`conditionalCaretFlags8Policy` must not be reused). Row = `flags & 3`: alpha SELECTARG1, alpha test GREATER, ref `[0x4C24E0]`.
* `[0x4C24E0]` is latched by `0x470F29(this=0x58CD80, flag)` (disasm 146246..): it stores `0xEF`, then `0xA0` only when `flag != 0 && [this+0x3A1] != 0`. The
  executable image value is `0xA0` (measured). `0x47A565` never calls `0x470F29` (the 2-double variant `0x47A75D` brackets its draw with `0x470F29(0)` ... `(1)`), so the
  reference seen by a scenery sprite is *whatever the last caller latched* (0xEF or 0xA0). Because flags 1 has no alpha plane (texture creation
  allocates one only when `flags & 0x18` is set, `0x472328..47232D`, retained `native-019`), the texel alpha is opaque (255, **inference**), and both
  references pass identically. Contract uses `0xEF` as the existing `options()` default; no pixel depends on the choice.
* Blend: all 19 records have selector **2**: additive ONE/ONE.

## 5. Frame state: random first frame, seed, advance (gap 5)

**First draw (`0x47D391..0x47D3F8`).** If `byte [node+0x350] == 0` (cleared by the reader `0x47B461`): `r = 0x4040A0()`; `count = 0x45B8F0(clip f2)`;
`node+0x2E4 (f3) = ftol(double(count-1) * r)` (`0x49C5AC` truncation); `[node+0x350] = 1`.

* `0x4040A0` (disasm 4722..4731) = `rand() / [0x4B03B0]`; **`[0x4B03B0]` is 32767.0** (measured, bits `0x40DFFFC000000000`). So `r` is in **[0, 1]** and
  `f3` is in **[0, count-1]**, with `count-1` only when `rand() == 32767`. (Report 001 said `[0, count-2]`; that was an unmeasured guess and is superseded.)
* `0x49C84A` (disasm 207673..207679) is the MSVC CRT LCG: `state = state*0x343FD + 0x269EC3; return (int32(state) >> 16) & 0x7FFF`, state at `0x4C4570`.
  The executable's image value of `0x4C4570` is **1** (measured) = the CRT default. `0x49C840` is `srand` (stores the argument at `0x4C4570`).
* The only wrapper of `srand` is `0x447580` (disasm 92205..92214): `srand(ftol(double_arg * 32767.0))`. It has two call sites, `0x4451E5` and `0x45D06C`; and **57 call
  sites** of `0x4040A0` plus `0x492A23` consume the same global stream.
* **Seed source (revision 2; disasm 88690..88725, 119855..119895, 130430..130475, 134117).** Both call sites pass `fild` of `0x4657D1()` to `0x447580`:
  * `0x4B0220` is the import `timeGetTime` (winmm; `reference-headers.txt` line 128: RVA `0xB0220`, `timeGetTime`; thunk `jmp [0x4B0220]` at `0x4680E6`).
  * `0x4657D1(this=timer)` = `timeGetTime() - [timer+8]` (32-bit ms). Timer constructor `0x465790`: `[t+8] = 0`, then `[t+8] = 0x4657D1()` (i.e. base := `timeGetTime()`), `[t] = 0x3E8` (1000 ticks/s), `[t+4] = 0` (not paused).
    `0x4657E8` = pause (`[t+0xC] = timeGetTime()`, `[t+4] = 1`); `0x465806` = resume (`[t+8] += timeGetTime() - [t+0xC]`, `[t+4] = 0`). So the value is *unpaused milliseconds since the timer was constructed*.
  * Both sites use the process-global timer `T0 = [0x4CBF84]`: it is constructed at `0x450826` in the startup function (`0x450849`: `[0x4CBF84] = result`) and the game object lives at `0x4C6A08`, so `[this+0x557C]` in `0x45CFF9` is the same slot (`0x4C6A08 + 0x557C = 0x4CBF84`).
  * **Site `0x45D06C`** is at the top of `0x45CFF9` (the main loop function called once from startup at `0x450A53`): `srand(ftol(double(ms) * 32767.0))` unconditionally, before any frame.
  * **Site `0x4451E5`** is in a function whose tail stores `[this] = 2` (`0x445207`) and which re-initialises per-run state (`[0x4CBFB0+0x10] = 0` at `0x4450EB`, a fresh timer at `0x4450A0`). The per-frame loop tests `cmp [this],2` (`0x45DB05`) to take the gameplay-step path, so this is the play-start routine (**role inferred** from the state value; its start address and caller were not located). It seeds `ms = T0 elapsed`, except `if [0x4E33C8] == 1` the double at `[0x4E33D0]` is used instead.
  * `[0x4E33C8]`: no file bytes (zero-initialised); one static writer, `0x48E2E0` (`= 1`), inside an MFC-style command-routing function and only when byte `[0x4C36A8] != 0` (image value 1; cleared at `0x45C35A`, `0x48ED20`, `0x49B0C0`). `[0x4E33D0]` has no file bytes and no static writer (so the override seed would be `0.0`). **Whether the override fires in an ordinary session is not established**; the contract treats the normal path (flag clear) as the shipped behaviour and states this.
  * `ftol` is `0x49C5AC`: `fistp` to a 64-bit slot with rounding-control forced to truncate, returns the **low 32 bits** (disasm 207431..207445). So `seed = uint32(int64(trunc(double(ms) * 32767.0)))` (wraps modulo 2^32, no saturation).
* **Consequence (established, not a gap).** The original's per-flame first frame is a function of (wall-clock milliseconds since process start at the moment of the most recent `srand`, plus every `rand()` consumed since). It is **not reproducible from level data and not fixable to one value**; there is no original value to match. Faithful behaviour is: seed the CRT LCG exactly as above from the application's own monotonic millisecond clock at play start (and at the loop-entry call), then draw each flame's first frame with the exact formula at its first draw. Tests inject explicit generator state and are formula tests only.
* Worked example (executed formula only, `records.txt`; **not** a policy and **not** a claim about original state): from LCG state 1 with no earlier call the 19 draws in file order give `f3 = 0 8 2 12 8 7 5 13 12 11 2 12 10 7 4 0 1 5 2`.

### 5a. Time unit and path of `dt` (revision 2)

Chain (disasm line numbers): per-frame loop `0x45CFF9` computes frame ms `[this+0x25C64] = T0.elapsed - [this+0x25C60]` (`119891..119898`, reset at `120378`), clamps it
(`120386..120402`: if `double(ms) > [0x4C08E8] * [0x4B1758]` (the `fcompp` with `ah&1`) **or** byte `[this+0x1C9B3] != 0` then `ms = ftol([0x4C08E8]*[0x4B1758])` = 100; the byte's meaning was not traced, nonzero would force a fixed 100 ms step), then in the play state (`[this]==2`, byte `[this+0x55A0]==0`)
calls `0x4423D2(this, double(ms) / [0x4B1758])` (`120553`, `120606`, loop `120513..120636`); in any other state or when `[this+0x55A0] != 0` it calls `0x4423D2(this, [0x4B17B8])` (`120638..120643`).
`0x4423D2(this, dt)` (`85809..`) returns early unless byte `[this+0x5139]` is set (level loaded), stores `dt` at `[this+0x5120]` and calls `0x437BDE(this+0x5140, dt)` (`85837`).
`0x437BDE(set, dt)` (`71348..71396`, `71397..71445`) does, for each of two scene lists, `set.t98 += dt`; `while (t98 > scene.duration[+0xB0]) t98 -= duration`; then `0x4764C4(scene, t98)`;
`0x4764C4` forms `d = t - [scene+0xE0]` (`d = t` after a wrap, when `d < 0`) and calls `0x468833(node+0x118, d)` for each list entry (`153703..153749`). Therefore:

* **Measured constants (bytes only, `records.txt`):** `0x4B1758 = 1000.0` (so `dt = ms / 1000.0`: **seconds**), `0x4C08E8 = 0.1` (frame cap = 100 ms), `0x4B1800 = 0.08`, `0x4B15F8 = 7`, `0x4B17B8 = 0.0`.
* `dt` is **seconds**; per-frame `dt <= 0.1 s` (cap); with `rate = 1.0` and the 0.06 threshold the fire clip advances one image per 0.06 s of game time.
* In non-play states and while `[this+0x55A0] != 0`, `dt = [0x4B17B8] = 0.0`: the scenery clock does not advance (but `0x4423D2` still runs, so draws are unchanged).
* The scene clock is a looping accumulator modulo the scene's duration (`scene+0xB0`, serialized in the scene object, not measured here for MM_DIG_000 because the duration comes from a header field not yet parsed by the production reader); the node advance sees the *difference* of the wrapped clock, and on the wrap frame it sees the wrapped value itself (`0x4764F7`), so the remainder is not carried exactly across a wrap.
* Fast-forward: when `[[this+0x55A8]+0x20] != 0` the loop runs the step `[0x4B15F8] = 7` times per frame (`120504..120518`), each with the same `ms`; otherwise once. This scales the scenery clock too; it is only relevant when the clone connects its existing fast-forward command (priority 4), and is recorded as the evidenced behaviour.
* Sub-stepping by queued event timestamps (`[this+0x5584]` list, `120519..120636`) splits `ms` at event times; the sum of `dt` per frame is `ms/1000`. The clone has no such queue on this route; the contract uses the frame total.
* Unestablished by this chain: the exact serialized `duration` of MM_DIG_000's scene (read from the file header into `scene+0xB0`; location in the level file not yet mapped to the production parser). If it is not mapped, the contract must clamp nothing and wrap nothing, i.e. admission is limited to the clock `t98` without wrap, which differs only when the level's elapsed time exceeds the duration. This is listed as a bounded limit in section 11.

**Serialized f3 is never used for the first frame** (overwritten at first draw). **Serialized f4 is never used** (section 3).

**Temporal advance `0x468833(sub, dt)`** (disasm 134665..134826), after the lazy resolve:
`rate = double [clip+0x10]` (`0x468879..468888`); if `[sub+0x1D0] == 1` (constructor value): `acc = [sub+0x1C0] + dt*rate`; `[sub+0x200] = 0`;
`while acc > 0.06 (0x4B1940 = 0.06 measured)`: `f3 += 1`; if `f3 == count`: `f3 = loop? 0 : count-1` (loop flag `[sub+0x201]`, constructor value 1) and
`[sub+0x200] = 1`; `acc -= 0.06`; if `acc <= 1e-8 (0x4B1928 = 9.99999993922529e-09)` then `acc = 0`. (Mode != 1 runs the mirrored backward form,
`0x4689B8..468AAD`; the constructor value is 1 and nothing in the reader changes it.) The clip rate for `fire` is **1.0** (measured), so a frame advances every 0.06 time units.

* Unit of `dt`: it is `t - previous t` of the double the owner passes to `0x4764C4`; the time base was not traced (vtable-only setter `0x438F8A`). With the
  threshold 0.06 and rate 1.0 the unit is plausibly seconds (about 16.7 frames/s, 16 frames per 0.96 s), but that is **inference**. This affects only temporal
  animation speed, not admission, geometry, material or the first-draw frame.

## 6. Branch-1 draw geometry (gap 4 support)

Report 001 sections 2.2-2.9 stand. This pass verifies and completes the arithmetic of `0x47A565` (`ecx = sub`, doubles `a0..a4` at `[ebp+8,0x10,0x18,0x20,0x28]`),
with `X = directional image (f2, f3, +0x1B8)`:

* `a0 += (double)int[X+0x30]`; `a1 = a1 - (double)int[X+0x34]` (`0x47A621..636`). Frame ints (measured for `fire`): `X+0x30=9, X+0x34=0, X+0x38=19 (w), X+0x3C=25 (h)`.
* Vertex x (`sub+0x20, +0x88, +0xF0, +0x158`) = `a0 - a2/2 - w/2`, `a0 - a3/2 - w/2`, `a0 + a3/2 - w/2`, `a0 + a2/2 - w/2` (divisor `0x4B03F8 = 2.0`; `w = int[X+0x38]`).
* Vertex y: `y0 = (a1 - h/2) - a4/2`; if `[0x58D53C] == 0` then `y0 += h / 0x4B02C8` (**`0x4B02C8 = 4.0`**, so `+h/4`). Stores: v0.y(`+0x28`) = v3.y(`+0x160`) = `y0`;
  v1.y(`+0x90`) = v2.y(`+0xF8`) = `y0 + a4`. The z slots were stored before the call by `0x47D6F9..76B` (report 001).
* Texture coordinates: `0x440C50(P0,P1,P2,P3)` (arguments are doubles at `[ebp+8,0x10,0x18,0x20]`) stores, per vertex (u slot = vertex+0x28, v slot = +0x30):
  v0 `(P0,P1)`, v1 `(P0,P3)`, v2 `(P2,P3)`, v3 `(P2,P1)` (disasm 84097..84143). `0x47A565` passes, with image fields `S10,S18,S20,S28` (the doubles at `X+0x10,0x18,0x20,0x28`):
  mirror(f1)==0: `(P0,P1,P2,P3) = (S28, S18, S20, S10)`; mirror(f1)!=0: `(S28, S10, S20, S18)`. (Measured `fire` image: `S10=0.686747, S18=0.385542, S20=0.117188, S28=0.265625`.)
  **Implement these literally; do not reinterpret the names `bottom/top/left/right` of `Sprite.h`**, which were assigned by the recovered reader as compatibility labels.
* `sub+0x1A8 = int[X+0x40]` (texture id), then `0x414920`.
* **Orientation check required at implementation time:** vertex-to-corner sign conventions (projected `a2`, `a3`, `a4` signs) were not evaluated numerically in this pass;
  the production test must compute them from the literal formulas on the measured `fire` record and report the resulting left/right/up orientation instead of assuming it.

## 7. Globals `0x58D53C`, `0x58CB38` (gap 6)

* `0x58D53C`: written at only two sites: `0x44186B` (`= [[0x4CBFB0]+0x10]`, inside `0x441845`, only when `byte [this+0x5139] != 0`) and `0x4419A0` (`= 0`, ending the same
  bracket); no file-image bytes (BSS, zero). Read at `0x414040`, `0x47A24C`, `0x47A6E2` (`0x47A565`), `0x47A766/0x47A7CD` (`0x47A75D`). Default and ordinary value: **0**. Meaning (revision 2,
  bracket read, disasm 84957..85131): `0x441845` is the **level render pass**. It sets `[0x58D53C] = [[0x4CBFB0]+0x10]`, then calls `0x437BA3(this+0x5140)` (begin: `0x47496D(set+0x10)`),
  `0x43A4EC`, the object lists at `+0x1F70` (vtable `+0x10` and `0x42C556/0x42C5A7`) and `+0x1FD0`, then `0x437BB9` (end), and resets `[0x58D53C] = 0` at `0x4419A0`. So the scenery
  draw **is inside** the bracket and sees `[[0x4CBFB0]+0x10]`. That field is a camera/view index of the camera object `0x4CBFB0`: its only absolute-address store is `0x4450EB` (`= 0`, play start);
  the other stores through `this=0x4CBFB0` write `+0x04, +0x08, +0x14, +0x18, +0x20, +0x38` (`0x456783`, `0x45692A..0x45697F`, `0x457009`, `0x45702B`, `0x455B2B`), never `+0x10`, and readers
  (`0x420B37`, `0x437622`, `0x453077`) only compare it with 0. Hence on this route `[0x58D53C] = 0` and the `+h/4` shift applies. The retained disassembly has no other view-index writer; a non-zero
  value would need a second view that the single-player route never creates (evidence: no store).
* `0x58CB38`: written 0/1 around draw routines (`0x40E7ED..`, `0x42F076..`, `0x45B737..`, `0x496C0B..`); read only at `0x47A252` and `0x47A7D2`, i.e. by `0x47A24C`/`0x47A75D`, **not by `0x47A565`**.
  `0x47A75D` (disasm 159213..159539) is the *2-double* sprite-rectangle variant (used for other sprite classes) that scales the quad by `0x58D53C - 0x58CB38` in {0, -1, other} (x1, /2, x2).
  It is **not** called by the type-2 branch-1 draw, so `0x58CB38` is irrelevant to this route (excluded with evidence).

## 8. Measured MM_DIG_000 records (executed; gap 4)

Command: `python tools/agent_control/verify.py --profile all` (CTest `scenebranch_measure`, 0.12 s, passed). Traversal is a measurement-only reader (types 0/1/2, branch 0/1, bounds-checked,
name capacity 0xFF) over the retained root boundary: typed root `0x98F84`, tree end `0xC3056`; the traversal ended exactly at `0xC3056` (assertion).

* 86 nodes: 1 type-0 root (`null2`, 85 direct children, matrix translation 0), **66 type-1 mesh nodes, 19 type-2 branch-1 nodes, 0 type-2 branch-0**, 0 hidden, **0 nodes at depth >= 2,
  0 branch-1 or type-1 nodes with children.** So nested Secondary dispatch (`0x46B6CC`) does not apply to this level: no nested node exists.
* Branch-1 records (offset of the node start; branch byte at node+0x1D):

| node idx | node start | idx | node start | idx | node start |
| --- | --- | --- | --- | --- | --- |
| 13 | 0xA7555 | 60 | 0xBCA5E | 79 | 0xC207F |
| 56 | 0xBC6EA | 61 | 0xBCB3B | 80 | 0xC215C |
| 57 | 0xBC7C7 | 62 | 0xBCC18 | 81 | 0xC2239 |
| 58 | 0xBC8A4 | 63 | 0xBCCF5 | 82 | 0xC2316 |
| 59 | 0xBC981 | 64 | 0xBCDD2 | 83 | 0xC23F3 |
| 76 | 0xC1DE8 | 77 | 0xC1EC5 | 78 | 0xC1FA2 |
| 84 | 0xC24D0 | | | | |

  Every record: `A=B=8.463`, `H=15.7628691`, visibility 1, branch byte 1, **`f1=0, f2=0, f3=0, f4=0`**, sprite name `"fire"`, trailing/blend selector **2**. Node names
  `transsprite_animating_fire_fire_additive_N`; matrix translations in `records.txt` (e.g. first record (2.40671, 2.8099, 9.68455)). First record: branch byte `0xA7572`,
  fields at `0xA7573`, name at `0xA7583`, trailing at `0xA758B`, node end `0xA7630` (record length 0xDB).
* Resolution: `SPRITES\fire.SPR` (found, parsed by the production `loadSpriteSheet`): **1 clip `fire`, rate 1.0, 16 temporal entries, 1 directional image each**, palette 0.
  Every record's image 0: sheet `Sheets\_Fire0`, texture `TEXTURES\SHEETS\_FIRE0.BMP` (exists), rect ints `(left,right,top,bottom)=(15,34,32,57)`, `X+0x30..0x3C = (9,0,19,25)`,
  UV `(0.117188,0.265625,0.385542,0.686747)`. Unresolved sprites 0, clip out of range 0, serialized temporal out of range 0, missing textures 0, entries with more than one
  directional image **0**.
* Note: other 15 temporal entries' images were not individually listed in the artifact beyond the directional-count histogram (all 16 entries have exactly 1 image); per-frame image
  rects are read by the production parser at runtime.
* The 66 type-1 nodes in this tree are meshes already supported by the production mesh path (the census marks this level's only blocker as the first branch-1 node).

## 9. Reference-executable constants read (bytes only)

`.rdata`: `0x4B02C8=4.0`, `0x4B02D0=1.0`, `0x4B0338=0.0`, `0x4B03B0=32767.0`, `0x4B03F8=2.0`, `0x4B0D68=-1.0`, `0x4B1928=9.99999993922529e-09`, `0x4B1940=0.06`, `0x4B1948=0.1875`, `0x4B1950=0.0625`.
`.data` image: `0x4C4570=1`, `0x4C24E0=0xA0`, `0x4C227C=4`, `0x4C24E4=0xFFFFFFFF`; `0x58D53C`, `0x58CB38`, `0x5CD5F4` have no file bytes (zero-initialised). Section table, string bytes and all values are in
`build/scene-branch-research-002/records.txt`.

## 10. Production implementation contract (for the next task; not approved here)

Bounded route: `runApplication --level MM_DIG_000` -> `AssetLibrary` -> `BackdropScene::build` -> Secondary traversal -> branch-1 draw.

**Parser (`src/assets/SceneNode.{h,cpp}`).** For branch != 0 read `u32 f1,f2,f3,f4` into the existing `type2.branchWords`, then a length-prefixed name with capacity 255 into `branchName`
(reject longer, bounds-checked), then the trailing word; treat the branch byte as a boolean (nonzero = branch 1, as `0x47B4E5`). No four UV doubles in branch 1. Keep every other existing
check; keep exact-end validation. Existing branch-0 behaviour and the ten admitted levels' bytes are untouched (they contain no branch-1 nodes: census).

**Admission (`BackdropScene::build`).** A branch-1 record is admitted only if: `SPRITES\<name>.SPR` is in the effective archive and parses; `f2 < clips`; that clip has `>= 1` temporal entries;
every temporal entry of the clip has exactly one directional image (the one-image path of `0x46840B`; any clip with a multi-image entry yields the existing "unsupported ..." diagnostic naming the sprite
and the unread selector `0x46840B/0x418350`); the entry's image-0 texture exists. Otherwise keep the specific unsupported diagnostic (never a partial subset). Keep the nested/non-leaf check unchanged
(MM_DIG_000 has none). The visibility byte 0 -> skip draw (`0x47D0E3`); none in this level. A missing sprite file is an error diagnostic (the original exits); do not silently skip.

**State (per branch-1 node, in the scene object):** `temporal` (u32), `accumulator` (double, 0.0), `firstDrawDone` (false), `mode=1`, `loop=true`, `heading=0`, `vector=(0,0,-1)`. `f3` and `f4` serialized values
are retained for evidence output only.

**Frame selection per draw:** if not `firstDrawDone`: `temporal = trunc(double(count-1) * (rand()/32767.0))` using a `CrtRand` (exact `0x49C84A` LCG, state default 1) injected into the scene,
then `firstDrawDone = true`; directional index = 0 (count 1 invariant); mirror = `f1 != 0`.

**`CrtRand` and seeding (revision 2; replaces the earlier default-state-1 policy).** `CrtRand` is the exact `0x49C84A` LCG (`state*0x343FD+0x269EC3`, `(int32(state)>>16)&0x7FFF`) with `srand(uint32)` and `seedFromMs(double ms)` = `srand(uint32(int64(trunc(ms*32767.0))))` (`0x447580` + `0x49C5AC`).
The application owns one `CrtRand` for the process, and seeds it **from its own monotonic unpaused millisecond clock measured from application start** (the analogue of `T0`) at the two places the original does: (a) once at main-loop entry (`0x45D06C`), (b) at play start (`0x4451E5`), before the first scenery draw of the level. The seed is **not** a constant; the default-constructed state (1) exists only so unit tests can construct an object. The state at the first draw is recorded in the run diagnostics so a render can be reproduced. Because every `rand()` consumer shares the stream in the original (57 `0x4040A0` call sites plus `0x492A23`), the clone must route any ported `rand()` consumer through the same `CrtRand`; unported consumers cannot be replayed and this is a stated difference. Because the original value is wall-clock derived, **no pixel-level agreement with a single original render is claimed or possible for flame phases**; acceptance tests inject explicit generator state (and say so) and assert the formula, the once-per-node latch and the range `[0, count-1]`.
**Per update (`dt` in seconds, evidenced, section 5a):** the clone's frame loop supplies `dt = frameMs / 1000.0` with `frameMs = min(frameMs, 100)`; `dt = 0.0` outside the in-level state; the scenery clock is `t98 += dt` and `dt_node = t98 - prev` per `0x4764C4` (with the duration wrap if the scene duration is parsed; otherwise no wrap, a limit). Then `0x468833` forward algorithm of section 5 exactly (`0.06`, `1e-8`, loop wrap, finished flag). The fast-forward factor (7 sub-steps) is applied only when the clone's existing fast-forward command is connected (priority 4).

**Geometry:** `SceneBillboardSource` gains a branch-1 builder: report 001 steps 2-9 (corner offsets, `+H/2` y shift on all four corners, projection, slot formulas, permuted z) producing `a0..a4` and the
vertices exactly as section 6 (`+h/4` when `[0x58D53C]==0`, taken as 0), UV by the `(P0..P3)` rule above. Quad polygon order and fan as section 4.

**Render state (conditional, labelled):** texture = `loadTexture(TEXTURES\SHEETS\_FIRE0.BMP)` (via `sheetAssetName`), alpha operation SelectTexture, alpha test StrictGreater ref `0xEF`, RGB Modulate with white diffuse
(the accepted branch-0 convention), blend enabled with factors chosen from the node's selector by the `0x414D80` table (selector 2 -> One/One), filter/address/depth as the existing `options()` conditional policy.
Do **not** reuse `conditionalCaretFlags8Policy` (flags 8). The test must assert the flags are derived from the texture name by `texture_name_flags` and equal 1 here.

**Ordering:** unchanged `dispatchSecondaryChildren` (type-2 items already dispatched by source offset; branch-1 nodes must be present in `secondaryBillboards`).

**Future public-route demonstration (planned; none executed here):**
1. Before (from the accepted frozen-source technique): `MM_DIG_000` yields a successful empty partial scene with exactly one diagnostic naming `MM_DIG_000`, `Secondary`, `unsupported nonzero type-2 branch`.
2. After: Secondary admitted; roots 3 or 4 as the loader reports; report per-role raster calls and accepted colour/depth stores, **branch-1 submissions listed apart** (19 nodes x 2 triangles = at most 38 raster calls
   before culling, with each node's offset, chosen `temporal`, mirror, rect, culled/rejected reasons) and mesh submissions; missing sprites/textures 0; scene-on vs scene-off RGB and depth artifacts and PNGs under
   `build/`; two renders byte-identical under an injected `CrtRand`; a different `CrtRand` state changes only the sprite pixels (mutation check); `runApplication --level MM_DIG_000 --sim scenario` under SDL dummy
   drivers exiting via the observer-queued `SDL_QUIT`.
3. Preservation: the ten admitted levels (`E06_KF_1, H17_MG_1, M06_KF_1, M08_KF_1, MM_BASH_002, MM_CLIMB_002, MM_E1_3_SKILLS, MM_E1_4SKILLS, MM_W_012, VH22_JC`) render byte-identical RGB+depth to the accepted
   artifacts; `MM_CLIMB_002` keeps Primary 923/0, Legacy 172/0, Secondary 82/17, 1177 records, 17 billboards and primary hash `0x1fd0bfb82ab39e95`; existing timeouts unchanged. The remaining 73 branch-1 levels and
   the 18 nested levels must be re-censused (first-failure partition; no promise of conversion).
4. Out of scope and unchanged: nested Secondary dispatch for other levels, digger gameplay, D1/D3/digger/clipping/controller/capture parks, C3 FAIL, native-006, oracle-001.

## 11. Nonblocking limits and unknowns (explicit)

* Original PRNG state at first draw: **established as not reproducible** (wall-clock seed from `timeGetTime`, section 5); not an open question. Residual bounded unknown: whether the `[0x4E33C8]` override (seed `[0x4E33D0]`, zero-initialised) fires in an ordinary session (single static writer `0x48E2E0`, MFC-style routing, guarded by `[0x4C36A8]`); the play-start function's start address and caller were not located (role inferred from `[this]=2`).
* `dt` unit: **closed** (seconds, `ms/1000.0`, cap 0.1, zero outside play, 7x burst when fast-forward; section 5a). Residual: the serialized scene duration (`scene+0xB0`) for MM_DIG_000's scene is not mapped to the production parser; byte `[this+0x1C9B3]` and `[this+0x55A0]` meanings untraced (the latter read only as "not paused"-like gate).
* Effective-device Z/cull/filter/address state: same unresolved C3q inputs as every other scene draw (`0x46D3C1` is now read: it sets none of them, section 4a).
* `0x58D53C` bracket: **closed** (section 7): value 0 on the single-view route; evidence is absence of any non-zero store.
* `node+0x340` UV-offset pair: source of a fresh node's value not located; contract uses zero as the accepted branch-0 route does (section 4a).
* `[0x4C24E0]` latch history (0xEF/0xA0) irrelevant under opaque texture alpha (inference from the no-alpha-plane rule).
* Orientation signs of the vertex-store formulas must be computed and reported by the future test (not assumed).
* `0x418350`, `0x480AC8` (multi-image directional selection) unread: excluded because no MM_DIG_000 record reaches them (measured); needed for other levels' sprites.
* Diffuse vertex colour: **closed** (255,255,255 alpha 255 from `0x440A10`, section 4a).
* The 74/18 census is a first-failure partition; this pass does not claim any other level converts.

## 12. Verification and changed files

* Executed once after the final source edit (revision 2, which extended `scenebranch_measure` to read `0x4B1758, 0x4B17B8, 0x4C08E8, 0x4B1800, 0x4B15F8, 0x4E33C8/D0/D4, 0x4C36A8`): `python tools/agent_control/verify.py --profile all` -> reference SHA verified by reading bytes; **main 93/93 passed, recovery 84/84 passed**
  (main had 92 before the one added test `scenebranch_measure`). Revision-2 measured values (`records.txt`): `0x4B1758 = 1000`, `0x4B17B8 = 0`, `0x4C08E8 = 0.1`, `0x4B1800 = 0.08`, `0x4B15F8 = 7`, `0x4E33C8/D0/D4` = no file bytes, `0x4C36A8 = 1`.
* Revision-2 static reads (no execution): disasm 88560..88730, 104900..105080, 119855..119895, 120360..120640, 85740..85840, 84957..85131, 71336..71450, 130380..130475, 134105..134135, 141625..141800, 83886..84032, 169484..169491, 207431..207445, 187380..187476, 153703..153749; changed file list unchanged except the extra constants in `src/__test__/test_scenebranch_measure.cpp`.
* Changed files: `src/__test__/test_scenebranch_measure.cpp` (new, read-only measurement), `CMakeLists.txt` (one `add_test` block inside the existing `WEBOLUTION_GAME_DIR` guard, after
  `sceneadmission_census`), `docs/scratch/scene-branch-research-002.md`. Ignored outputs: `build/scene-branch-research-002/records.txt`, `build/scene-branch-research-002/report.md`.
* Not run: anything involving the original executable, hooks, debugger, capture, installation or save writes. The `git show HEAD:src/app/main.cpp` read was not needed. The supervisor's own verification
  and the independent review remain required.
