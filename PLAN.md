# Lemmings Webolution — Reimplementation Plan

An open reimplementation of **Lemmings Revolution** (Psygnosis Leeds / Take-Two, 2000)
in C++23 on SDL2 + OpenGL, building natively on MSYS2/UCRT64, Linux, macOS, and
WebAssembly.

The goal is a **1:1 mechanical match** with the original, starting at its native
640×480 and only later taking advantage of SDL2/GL for resolution and presentation.

No game data is redistributed. The engine reads `lemmings.box` and `patch.box`
from an existing installation at runtime, in the manner of OpenRCT2 / devilutionX /
OpenTTD.

---

## 1. What we already know

### 1.1 The game binary

`Lemmings Revolution.exe` — x86, MSVC, 832 KB, unpacked, no protection.
DirectDraw4 + Direct3D immediate mode (HAL). Mouse and keyboard via DirectInput.

**Relocations are stripped, so it always loads at `0x400000`.** This is the single
most useful fact in the project: every address below is stable across runs, and the
live process can be read with `ReadProcessMemory` without any address discovery.

Addresses mapped so far (verified against the running game):

| Address | Meaning |
|---|---|
| `0045F1FE` | `push 1E0h` — render height, the game's only resolution decision |
| `0045F203` | `push 280h` — render width |
| `0046EA17` | `Renderer::Init(w, h, bpp, bWindowed)` |
| `0046F507` | Display init: surfaces, viewport, `SetDisplayMode` |
| `004702A7` | `D3DVIEWPORT2` fill — `dwWidth/dwHeight` from render size, clip volume `[-1,1]` |
| `0047BF16` | `FindMode(w, h, bpp)` over the enumerated list |
| `0047BE9B` | `EnumDisplayModes` callback — accepts every mode, no filtering |
| `0058CD80` | Renderer singleton. `+0B8/+0BC` render w/h, `+17C/+180/+184` requested w/h/bpp, `+2D` windowed flag, `+3C` screen object, `+9C/+A0` centring offsets |
| `0058CB40/44` | Cursor bounds (= render w/h) |
| `0058D294/98` | Cursor centre (= render w/2, h/2) |
| `0058CE0C/10` | Window origin |
| `0045BFED` | Settings load from registry |
| `0045C7E2` | Cursor configuration (`CursorMode`, `FloatingCursorDir`, `ShowIcons`) |
| `0047B178` | `Cursor::SetAspect(w, h)` |
| `00455F40` | Per-frame cursor update (fixed vs free-floating paths) |
| `004567EF` | Mirrors the internal cursor onto the system cursor |

Original module names, recovered from retained `ASSERT` source paths:

```
3DEngine\HardwarePC.cpp      3DEngine\RawBitmap.cpp     3DEngine\TexturePC.cpp
SkyIslands\SkyIslands.cpp    SkyIslands\LoadBMPImage.cpp
Sprites\SpriteSheets.cpp     Sound\SoundManagerPC.cpp   File\FilePC.cpp
```

`SkyIslands` is the game layer; `LoadBMPImage.cpp` is where the texture decoder lives.

### 1.2 The asset container — solved

Derived independently from the bytes and cross-checked against ccexplore's
documentation on Lemmings Forums; the two agree field for field.

```
char[6] "LEMBOX"
u32     count
u32     nameBlockBytes
u8[nameBlockBytes]  count × (u32 len, char[len])   -- no NUL terminators
u32 count ; u32[count] offsets   -- absolute from start of file
u32 count ; u32[count] sizes
payload
```

`lemmings.box` parses to 10,661 entries with **zero gaps and zero slack** — the
parser is exact, not approximate. `patch.box` is the same format: 311 entries, 299
of which override `lemmings.box` by name, 12 new, plus a `.\PATCH.LST` manifest.
Load order is therefore: base, then patch overlay.

Contents:

| Ext | Count | Notes |
|---|---|---|
| `.BMP` | 10,376 | **Not** Windows BMP. LRZ-compressed (see below) |
| `.LVL` | 102 | Levels. 699 KB – 1.9 MB each. **The unsolved format** |
| `.ALF` | 81 | 8-bit alpha plane, dimensions match the sibling `.BMP` |
| `.WAV` | 73 | Standard RIFF PCM (solved, §1.7). 17 rates from 3179–44100 Hz, 8/16-bit, mono/stereo — not a single house rate |
| `.SPR` | 19 | Sprite sheets; reference texture sheets with frame rects (solved, 1.6) |
| `.LTX` | 6 | Localised string tables |
| misc | 4 | `LEVELS.MAP`, `LEVELMAP.DAT`, `RELEASERATE.PC`, `PSPBRWSE.JBF` |

### 1.3 Texture format — solved by the community

`.BMP` entries use **LRZ**, a custom scheme with canonical-Huffman trees for
literal-run lengths, copy lengths and distances. A reference C implementation is
published in the Lemmings Forums thread, but only as a login-gated attachment,
and the prose description of the tree construction is too loose to reimplement
from. So `src/assets/Lrz.cpp` is transcribed directly from the hand-written
assembly instead:

```
.text:00467DE8  decode(src, dst, packedSize, unpackedSize) -> bytes produced
.text:00467EB7  read one tree (nibble depth list)
.text:00467F0D  build it into the node arena
.text:00467FED  decode one number, with or without a tree
```

The chunk header's third field, undocumented as "usually 4", is a **method id**:
3 = stored, 4 = compressed. Decoded texture entries are ordinary Windows BMPs.

`tools/lrz-oracle` maps the original exe at its fixed base and calls that routine
directly, giving a reference implementation to diff against -- the same
no-relocations trick the differential oracle relies on. Our decoder is **byte
identical** to it, and decodes all 10,383 texture entries to valid bitmaps.

Filename prefixes are semantic, and the data matches the documented meanings:

| Prefix | Meaning |
|---|---|
| `^` | A sibling `.ALF` alpha file exists |
| `+` | Has an embedded alpha channel |
| `$` | Colour `#404040` is transparent |
| `_` | Colour `#000000` is transparent |
| `~` | Additive blending |

### 1.4 Metadata formats — solved

All four are implemented in `src/assets/GameData.cpp` and parse the retail files
exactly, consuming every byte. Two of the published descriptions turned out to be
wrong, both caught by parsing the real data:

- `LEVELMAP.DAT` — 125 entries: id, display name, time limit, save target, total
  lemmings. Unused slots are present and named `blank`.
- `LEVELS.MAP` — **five counted groups of 25**, not one flat list of 125. The
  groups are the difficulty tiers, in play order. Each group restarts MFC's
  per-archive class table, so its first slot carries the full `CLevelSlot` class
  header (`FFFF`, schema, name) and the rest use the `8001` "same class" tag.
  Every slot id resolves against `LEVELMAP.DAT`.
- `RELEASERATE.PC` — **99 doubles**, not `u32` + float pairs: seconds per lemming,
  falling from 5.0 at rate 1 to 0.2 at rate 99.
- `.LTX` — `u32 count`, `u32[count]` offsets, then NUL-terminated strings; 232 per
  language across 8 languages. The encoding is **not** uniformly Windows-1252:
  each localisation uses its locale's ANSI codepage, so Polish and Czech need
  1250 and Hebrew needs 1255. A single table would silently corrupt three of the
  eight. A `^` inside a string is the game's line break.

### 1.5 `.SPR` — solved

Undocumented; derived by parsing until all 19 retail files were consumed
exactly. Nested counts, then a trailing palette:

```
u32 spriteCount
  u32 animationCount
    u32 frameCount
      u32    bottom, top, left, right      source rect in pixels
      double bottom, top, left, right      the same, normalised
      u32    unknown, unknown
      u32    width, height                 == right-left, bottom-top
      u32 len; char[len] sheet             "Sheets\$LemSheet8"
  u32 len; char[len] name                  "walk", "bash", ...
  double rate
u32 paletteCount ; each { u32; double[48] }
```

Two internal consistencies pin the field order down, and both were needed:
`width`/`height` must equal the rectangle's extents, and dividing each integer
edge by its normalised twin must recover the sheet's real pixel size. The
second is what caught the axes being stored **Y before X** -- the wrong reading
yields a plausible rectangle but transposes the sheet dimensions. Verified over
all 7,354 frames, and by rendering animation strips and looking at them.

`LEMMINGSLO.SPR` holds the 15 lemming states -- walk, bash, build, climb, dig,
drown, explode, fall, flip, mine, shrug, float, umbopen, cartwheel, block --
each with 12 animations, which is how a 2D game on a cylinder shows a lemming
from several angles. Only the six lemming sheets carry a palette, all the same
16 colours, the first being 404040: the shade the `$` sigil keys out.

### 1.6 `.LVL` — the open problem

Not publicly documented. Structural probe of `MM_CLIMB_002.LVL` (699 KB):

```
0x000000  (8)  "Level 23"
0x00002c  (9)  "_curNoLem"      script variables, sigil-typed (_ ^ ~ + @ ! $ & *)
0x0000b7  (11) "+shadow_new"
0x00010f  (42) "Cylinder\MM_CLIMB_002\INN_MM_CLIMB_002_0_0"
0x00013d  (42) "Cylinder\MM_CLIMB_002\OUT_MM_CLIMB_002_0_0"
          ...  INNer / OUTer cylinder surfaces, tiled grid [col]_[row]
0x0a5d21  (11) "PolyList 09"    many geometry chunks
0x0aaa27  (4)  "cyl2"
```

Strings are `u32`-length-prefixed. Layout appears to be: header → script variables →
cylinder texture grid → polygon lists → terrain/collision data.

**The key structural insight for the whole project:** Revolution is a 2D game wrapped
onto a cylinder. The simulation is a flat terrain mask with horizontal wraparound;
the cylinder is presentation. We can therefore build and test the entire simulation
before writing a single line of 3D rendering.

#### What is read so far (`src/assets/LevelIndex.cpp`)

Verified across all 102 levels:

* the length-prefixed title at offset 0, and the header words after it;
* every asset reference -- 9,792 cylinder tiles and 16,229 model textures,
  **26,021 in total, all of which resolve** to an entry in the archive under
  `TEXTURES\<reference>.BMP`. That is the evidence the scan reads real fields:
  a mis-scan yields references that do not resolve;
* a `Cylinder` chunk followed by two equal `u32` arrays whose byte length the
  header states twice (110,592 bytes = 27,648 cells in the retail data).

What a level references, beyond textures: placed models (`Models\exitasket`,
`Models\entrance\lem_box_default`, scenery), sound cues (`Splat`, `Yippee`,
`Ohno`, `Letsgo`), sprite sheets (`Axe`, `Bits`, `GreyIcons`), and sigil-typed
script variables.

#### The cylinder is 8 tiles around by 3 tall -- confirmed

Read straight off the loop bounds in the routine that builds tile names
(`.text:0043AA91`, format string `Cylinder\%s\%s_%d_%d`):

```
for (col = 0; col < 8; ++col)        ; cmp dword [ebp-4], 8
  for (row = 0; row < 3; ++row)      ; cmp dword [ebp-110h], 3
```

Tiles are stored at index `col + row * 8`, with the second surface offset by
24 (`lea eax, [edx + ecx*8 + 18h]`). That accounts exactly for the 48 tile
names seen per block: 24 positions, inner and outer.

#### The dense array is NOT a terrain grid -- ruled out

Worth recording as a negative result, because it looked convincing. The
second array is dense (about 6,100 of 27,648 entries set, against 194 in the
first), its values are a small set, and rendered at a plausible width it shows
sky above and a ground band below. All of that fits a terrain mask.

It is not one. Across the 102 levels the arrays take 30 distinct sizes, and
the greatest common divisor of their entry counts is **16** -- so no constant
width above 16 can divide them all. Worse, individual counts factor badly:
5,392 = 16 x 337 with 337 prime, which admits no sensible rectangle. A
candidate dimension field in the `Cylinder` chunk (always 96) divides only 9
of the 102 counts.

So the array is far more likely per-face or per-polygon data, which fits the
`PolyList` chunks sitting next to it. Anything built on the grid reading would
have looked right and played wrong.

#### Still open

* How the two `u32` arrays are keyed (their values are now known; faces are
  rebuilt from the lattice in SurfaceMesh).
* Remaining PolyList kinds / record tails; hazard sim from placed models; script.

#### A correction: `[004CBF78]` and `[004CBF7C]` are fonts

An earlier note in this plan read the two 16-byte objects allocated at
`004508A4` and `004508F2` as the two grid objects, on the strength of their
sitting beside the level pointer at `[004CBF84]`. They are not. Their class
(vtable `004B15C8`, constructor `00440330`) initialises `[+8]` to
`0FFFFFFFFh` and its one hot method, `004404FA` -- twelve of the thirteen
calls through those globals -- walks a NUL-terminated string a byte at a
time, looks up a per-character advance, and handles an escape character.
That is a text renderer, and the `.data` strings `^Font`, `^Font` and
`_FontSmall` at `004BFF68` confirm it: exactly two fonts, exactly two
objects.

So this route is closed, and the globals are retagged in the address table
above. Noted at length because the circumstantial case was good and the
objects are reachable from the level loader, so it would otherwise be
re-walked.

#### The level header, and the cylinder geometry -- both solved

Chasing the chunk reader in the binary turned out to be unnecessary: `.LVL`
is a flat, self-describing format, not a chunked or MFC-serialised one. (It
was worth checking: `LEVELS.MAP` *is* MFC-serialised, but a scan for MFC's
`FFFF` new-class tag finds none in any level, and there are no chunk-name
strings in the executable to dispatch on.) The header reads directly:

```
counted string   title              "Level 23"
u32 x 7          header words
u32              manifestCount
manifestCount x  counted string     asset names, always starting "_curNoLem"
```

That consumes exactly, in all 102 levels. It also corrects the earlier
reading of an eight-word header with a mysterious ninth word "always 9": the
eighth word was the manifest count, and the 9 was the length prefix of
`_curNoLem`. Invariants over all 102: `word[3] == word[4]` (the array byte
lengths), `word[2] - word[1]` is always `0x1B0000`, `word[0]` repeats the
manifest count. `word[5]` is distinct in every level and always even, which
reads as a seed or checksum.

After the manifest come further counted-string blocks -- the skill-panel icon
sheets (`Bits`, `GreyIcons`, `BlackIcons`, `AdditiveIcons`, `Axe`, `Smoke`),
22 sound cues (`Letsgo`, `Yippee`, `Chink`, `Explode`, `Ohno`, ...), and the
level's own path -- and then the cylinder mesh, which **is** solved and is
implemented in `src/assets/Cylinder.{h,cpp}`:

```
u32   faceCount      112        unidentified; recorded, not interpreted
f64   thickness      10.0       outerRadius - innerRadius
u32   ringCount      7
ringCount x 32 x 2 x { f64 x, f64 y, f64 z }    ring vertices
u32   ringCount      again
ringCount x 32 x f64                            per-vertex inner radii
u32   one            1          unidentified; recorded, not interpreted
u32   surfaceCount   505..2772  THE PER-LEVEL PART
surfaceCount x { f64 x, f64 y, f64 z }          the level's surface
u32   zero           0
u32   48
48 x counted string                             tile texture names
```

Vertices are unaligned (the preceding `u32` leaves them on a 4-byte
boundary), ordered by angle, inner before outer at each step. The whole
chain parses exactly in all 102 levels, and every non-empty tile name
resolves to a texture that exists in the archive. Empty tile names are
normal: a slot with no texture on that surface, and most levels skin only
one of the two.

#### The terrain is geometry, not a bitmap

This is the round's real finding, and it falls out of the asymmetry above.
Everything in the chain is identical across the 102 retail levels except
`surfaceCount` and its vertices, which take **100 distinct values from 505
to 2772** -- effectively unique per level.

So the terrain Lemmings walk on is carried as polygon geometry on the tube,
not as a bitmap mask. That is what finally explains the failure recorded
further up this section: the two equal-sized `u32` arrays were never going
to factor as a rectangular grid, because the game does not store a grid.
The arrays are something else -- per-face or per-polygon data is the natural
reading now, and it fits both the `0xFFFFFFFF` sentinels and the `PolyList`
chunks sitting beside them.

It also has a consequence for the simulation, which currently rasterises a
`Terrain` bitmap. That is still the right internal representation -- digging
and building need a mutable mask -- but the loader will have to rasterise
the polygon surface into it rather than copy a stored grid, and the
rasterisation resolution becomes ours to choose rather than the file's to
dictate.

The surface vertices open with a quad -- `(0,0,60) (0,16,60)
(-11.74,16,59.02) (-11.74,0,59.02)` in the first level, spanning one angular
step and one ring band on the outer skin -- and then continue in pairs, so
they read as a strip. The exact primitive and winding are not pinned down
yet, so `Cylinder.h` exposes the vertices without claiming a topology.

The finding is that **every retail level has the identical mesh**: 7 rings at
y = 0, 16, ... 96, 32 angular steps of exactly 11.25 degrees, inner radius
50, outer radius 60. The tube is effectively a constant of the game that
happens to be stored per level, so the renderer can treat it as fixed and
only the skin and terrain vary.

It also pins the coordinate system, which is the part the simulation needs.
The skin is addressed as 8 tile columns by 3 tile rows (read off the loop
bounds at `.text:0043AA91`), and the mesh divides evenly by both: 32 / 8 = 4
angular steps per tile column, and 6 bands / 3 = 2 bands per tile row. That
is why `Terrain` wraps in X and not in Y.

`test_cylinder` locates the mesh by signature -- the first vertex is
`(0, 0, innerRadius)`, and a run is accepted only if it is a whole number of
equal-width rings -- then asserts all of the above in all 102 levels. A
false positive would have to fake the entire structure and then agree with
the other 101 by accident.

#### The two `u32` arrays hold surface vertex indices

Settled, and it follows directly from having the surface. Every non-empty
entry in the first array is a valid index into the cylinder's surface
vertices, in all 102 levels -- and the largest index lands just under the
vertex count every time: 87% to 99.8% of it, median 98.6%. That margin is
the evidence. An index array covers nearly all of its pool, so the maximum
sits a little below the bound; unrelated data would not land in that narrow
window 102 times running, and would not stay in range at all.

`test_cylinder` cross-checks the two modules for this: it parses the mesh
with `findCylinderMesh` and the arrays with `indexLevel`, and asserts both
the in-range property and the coverage margin across the whole corpus.

What is still open is how the arrays are **keyed**. They are sparse -- 194 to
516 non-empty entries out of 5,392 to 27,648 -- so each is a lookup from
something to a vertex, not a dense face list, and the something is not yet
identified. Their size is also still unexplained, which is consistent with
that: the key space is larger than the number of entries used.

The second array behaves the same way in 92 of the 102 levels and carries
larger values in the other 10, so it is probably not the same kind of thing.
Only the first is asserted.

#### The array key is a cylindrical lattice — settled

The slots are not a hash and not a table of 16-word records (an earlier
reading that fit `MM_CLIMB_002`'s sparse stride of 16 and failed on the denser
levels). They are a **spatial lattice on the tube**, and neighbouring keys
name neighbouring surface vertices. Measured on `grid0` across the corpus,
and locked down on `MM_CLIMB_002` by reconstructing quads:

| Key step | Geometry | Median distance |
|---|---|---|
| `+1` | inner ↔ outer at the same (ang, y) | ≈ thickness **10** |
| `+16` | next angular step (11.25°) | ≈ chord at r=60 (**11.74**) or r=50 (**9.80**) |
| `+Y` | next ring (Y is per-level; 8224 on the ~27k arrays) | ≈ ring spacing **16** |

That is why every array length is a multiple of 16: each angular step owns a
block of 16 slots. `lrtool lvlkeys` reports the median distances; about
two-thirds of the retail levels hit the chord test on `+16`, and whenever
`+1` has enough pairs its median is the thickness.

Quads fall out of the same adjacency. On `MM_CLIMB_002`, every fully-populated
`(k, k+1, k+17, k+16)` is a thickness×chord face, and every
`(k, k+1, k+Y+1, k+Y)` with `Y=8224` is a thickness×ring face — 28 and 12
quads respectively, no exceptions among the populated ones.

`grid1` indexes the same surface pool. On the ten levels whose arrays are
~27k cells long, `grid0 ∪ grid1` partitions the surface vertices almost
exactly (overlap of a handful). On the other 92 the two arrays only cover a
minority of the vertices between them — consistent with the sparse clusters
`lvlmap` already showed. When both are set at the same key they never agree.

#### Faces → terrain mask — done enough to play

`src/assets/SurfaceMesh` walks both index arrays, keeps every lattice quad
whose edge lengths match thickness / chord / ring spacing, derives the
per-level ring stride `Y` by searching multiples of 16, and rasterises the
**angular and ring edges** into a wraparound occupancy mask (radius is
discarded by the unroll, so thickness edges collapse to points and are not
stroked). `--level` loads that mask as the simulation terrain;
`lrtool lvlmask` writes it as a PNG.

`Y` is **not a stored .LVL field** — it is only derivable. Correlated across
all 102 levels against the header words, the Cylinder-chunk preamble (the
constant `…, 96, f64, 49858, …` block before the grids), words after the
grids, and bytes near `surfaceCount`: none predict `detectRingStride()`. Of
the 52 levels that expose ring faces, 35 contain no aligned `u32` equal to
`Y` anywhere in the file; on `MM_CLIMB_002` neither `8224` nor `514`
(`Y/16`) appears as an aligned `u32` at all. Same cell-count levels also
disagree on `Y` (e.g. 5200 → 1008/1520/3104/4096/4608), so it is not a
function of array size either. The geometric search stays the loader.

Coverage across the corpus is low — a few percent solid, 12.7% on
`MM_CLIMB_002` — which matches the sparse clusters `lvlmap` already showed.
So the surface is platforms / detail, not a filled tube. Skin tiles remain
scenery until proven otherwise.

#### Entrances, exits, and other placed models — solved (parse)

Each level embeds `.a3d` placements. Immediately after the length-prefixed
model name a nearby `Object 09` opens the instance; four **zero** pad bytes
and a run of doubles follow, with indices 9, 10, 11 the world-space
translation and an earlier unit cos/sin pair for yaw. Entrances sit near
the inner radius (~50), exits just outside the outer (~65).
`findLevelObjects` partitions entrance/exit variants into
`LevelObjects::entrances` / `exits` (unchanged API) and everything else into
`placed` (`PlacedModel` = name + pose + `PlacedKind`).
`worldToMaskPixel` maps them onto the occupancy mask; `--level` logs all
three and still only wires entrance/exit into the sim.

Corpus (102 levels): 417 non-entrance/exit world placements across 73
levels — `lever` (130), `teleport` (54), `anti_grav` (50), `pressure_pad`
(45), `pipe_drainage`/`pipe_supply` (37/34), `deathroller` (22),
`speedup_two_way` (16), creatures (`Crab`/`robin`/`Scorp`), plus a little
scenery (`PlaceHolder`, one `light_green`). Skipped on purpose: `fish.a3d`
and most `light_*.a3d` (non-zero Object 09 pad — different subtype), and
`teleport_fx.a3d` (identity at the origin). No hazard simulation yet.

#### Skill counts — solved

Near EOF every `.LVL` carries eight little-endian u32 skill-panel counts in
classic order (climber, floater, bomber, blocker, builder, basher, miner,
digger), four pad bytes, then the marker `01 00 00 00 08 00 00 00`.
`findLevelSkills` reads that trailer; `--level` loads the counts into
`LevelSetup::skills` and `World::assign` depletes them. Spot-checked against
the tutorial singles (`Just Climb` = 10 climbers, …) and all 102 retail
levels parse.

#### Skin tiles do not encode solidity — ruled out

Every named INN/OUT skin tile across all 102 levels (4,896 tiles, ~20M
texels) loads as `AlphaMode::Opaque` with **zero** translucent pixels and
essentially no near-black colour-key holes (~0.006%). HI siblings are the
same. The 8×3 outer grid is a full opaque wrap; the surface occupancy mask
stays sparse (a few percent). Filling unrolled lattice quads also fails —
thickness edges collapse when radius is discarded — which is why
`rasteriseSurface` strokes angular/ring edges instead.

So the artwork is scenery. Walkable cells have to come from geometry, not
from tile alpha.

#### PolyList 09 — render attributes, not collision

Each level carries hundreds of length-prefixed `PolyList 09` chunks. The
common kinds obey a fixed size formula after the name:

* kind **5**: payload = `16 + 160 * count`
* kind **7**: payload = `16 + 212 * count`

Each of the `count` records starts with a `u32` index into
`CylinderMesh::surface`, then UV and RGBA stored as doubles. Across the
corpus ~87% of those indices fall inside the surface pool (95%+ on
`MM_CLIMB_002`), and most unique indices also appear in `grid0`/`grid1`.
`findPolyLists` reads the formula-matched blocks.

So PolyList is textured surface (and similar) draw data — colours and UVs
for vertices we already have — not a denser collision volume. Unmatched
kinds/sizes remain; they do not change that reading. A live-process terrain
dump is still the way to prove the sparse lattice matches what the original
simulates.

#### Live-process terrain dump — harness in place, compare still open

Goal: drive the original into a playable level, dump its in-memory occupancy
buffer, and correlate it with `lrtool lvlmask` / `rasteriseSurface` (e.g.
`MM_CLIMB_002` at 256×192, ~12.7% solid).

**Done**

* Shim rebuilt to also serve `DIMOUSESTATE2` (`cb == 0x14`) the same way as
  `0x10` (`tools/dinput-shim`); still polls at 144 Hz under dgVoodoo.
* Harness under `tools/harness/`:
  * `lrinput.py` / `memdump.py` — shared-memory input + `ReadProcessMemory`
  * `enter_and_dump.py` / `click_yellow.py` — PrintWindow vision: main menu →
    Continue? → level map → long-click mid yellow start node
  * Mouse scale under dgVoodoo is ~2.5–3.8 DI units per screen pixel; aim with
    feedback, not raw pixel deltas
  * `RETURN` on the level map activates **Back to Main Menu** — never use it
    there; map entry is mouse-only
  * Reference mask written: `build/live_dump/ref_climb_mask.{png,bin}` via
    `lrtool lvlmask … MM_CLIMB_002`
* End-to-end entry **verified visually**: after a long-click on a yellow map
  node the original was in the climber tutorial (10 climbers on the panel,
  cylinder + platforms, timer running). Screenshots `build/live_dump/P*.jpg`.

**Not done**

* The post-entry gate treated dark in-level frames as “main menu”
  (`mean_rgb < 45`) and **skipped the mask scan** while the level was live.
* Naïve `memdump --scan` / whole-VA walks are too slow; a fingerprint scan
  against `ref_climb_mask.bin` (`fast_mask_scan.py`) was started but not
  finished before the process exited.
* The object at `0x004CBF84` still only exposes small language-ish buffers
  (311 / 10661) on the menu and in-level — the walkable mask is not hanging
  off that head in an obvious `size,size,1,ptr` form. Needs a targeted search
  (byte 256×192, bit-packed, or other stride) while paused in-level.

**Second attempt — the search got better, the entry got worse**

*Search.* `fast_mask_scan.py` is replaced by `find_level_data.py`, which is a
different kind of search and worth preferring on both counts. It looks for
bytes we already know are in the file -- two consecutive surface vertices (48
bytes of IEEE doubles), a dense window of the index array, a tile texture
name -- using `bytes.find`, which is C rather than a Python byte loop. A full
pass over the process is **168 MB in a few seconds**, against a mask scan
that never finished.

It also avoids assuming the answer. A mask scan looks for a byte-per-cell
occupancy grid; if the engine keeps terrain as lattice geometry (which is how
the *file* stores it) there is no such grid, and finding nothing would have
told us nothing. Known plaintext does not have that problem: the needles are
in the file, so if the level is loaded they are in memory unless the loader
transforms them -- and either answer is informative.

Control run, at the main menu with no level loaded: **zero matches**. So the
needles do not fire on arbitrary memory.

*Entry.* Driving the mouse is now understood but is **not working for
clicks**, which blocks the compare:

* Pointer acceleration is on and there is no single scale -- 1.0 px per unit
  at 5 units, 2.7 at 30, 8.2 at 300. A fixed factor oscillates forever. The
  earlier "2.5-3.8 units per pixel" note was a mid-range sample of this curve
  (and its reciprocal). `aim.py` estimates the scale from the previous step
  and converges in two or three moves; it now reaches a named target to
  within 8 px.
* The shim holds one pending delta, so steps must be frame-synced
  (`glide(..., steps=max(|dx|,|dy|), per_step_frames=1)`). A tight `move()`
  loop coalesces deltas and the acceleration curve turns a 60-unit nudge into
  a 500 px jump.
* Keyboard works: RETURN drives the main menu through "Neues Spiel starten"
  and its Ja/Nein confirm onto the level map. Arrow keys do nothing on the
  map, so map entry really is mouse-only.
* **Mouse motion reaches the game; mouse buttons do not.** With the cursor
  measured onto a gold node (825, 612; node spans x 804..846), neither a
  plain press/release nor `long_click` changes anything. The same is true of
  the "Zurück zum Hauptmenü" label. The shim's header says the game's whole
  input surface is two `GetDeviceState` calls, and motion through that path
  demonstrably works, so the button bytes are either being written somewhere
  the game does not read or the game takes clicks by another route
  (`GetDeviceData`, vtable[10], is the obvious candidate and is not hooked).

**Next step for this item:** settle the button question before anything else.
Add a counter to the shim for `GetDeviceData` and for the `cb` values seen by
`GetDeviceState`, rebuild, and watch which the game actually calls while a
click is held. That distinguishes "wrong offset" from "wrong entry point" in
one run. Then re-enter a level and run `find_level_data.py` immediately.

#### Where to go next

* **Live-process terrain dump** — finish the compare (harness reaches a level;
  scan + correlate still open — see above).
* **Hazard / gadget simulation** — placed-model poses are parsed
  (`LevelObjects::placed`); wire anti-grav, teleports, deathrollers, etc.
  into the sim when the behaviours are known.
* **Remaining PolyList kinds / trailing record bytes** — only the leading
  index+UV+RGBA of kinds 5 and 7 are decoded.

#### Looking at it: `lrtool lvlmap`

`lrtool lvlmap <game-dir> <level> <out.png>` unrolls a level's surface --
angle around the tube to X, height to Y -- and plots the vertices over the
ring lattice, with the 8x3 tile boundaries picked out. It joins consecutive
vertices only where they land near each other in the unrolled plane, and
fills nothing: the topology is unknown, and filled faces would be an
assertion rather than a picture.

What the pictures show is worth recording, because it narrows what the
surface can be. The geometry is **sparse and clustered** -- small structures
of a few vertices each, hugging the ring lines, with large stretches of the
tube carrying nothing at all. It is plainly level-specific (`MM_CLIMB_002`
and `MM_FLOAT` look nothing alike) and often symmetric about the tube, which
matches the 4-fold symmetry already seen in the ring radii.

That sparsity is in the vertex data itself, not an artefact of the drawing
rule -- the vertices are plotted unconditionally. So the surface array is
very unlikely to be the complete walkable terrain on its own: most of the
cylinder would then be empty. The better reading is that it is detail or
collision geometry sitting on top of a skin whose coverage comes from the
tile textures, but that is an observation from pictures and is not asserted
anywhere in the code.

The live-process fallback also stands: the executable loads at a fixed
`00400000`, `ReadProcessMemory` works, and the DirectInput shim can drive the
menus by script, so a level can be started and its in-memory terrain read
directly -- which sidesteps the file format for terrain entirely and gives a
ground truth to check any later static reading against.

### 1.7 `.WAV` — solved

Ordinary RIFF PCM, implemented in `src/assets/Wav.{h,cpp}`. The interesting
fact is the variation: all 73 retail entries decode, but they span **17 sample
rates** (3179–44100 Hz), both 8- and 16-bit, and both mono and stereo. An
earlier note that they were uniformly 22050 Hz was wrong for 35 of them.
Odd rates like 7418 / 5563 / 8363 look like tracker-era sample rates, so these
are converted assets. Chunk order is not assumed — three files carry a trailing
`smpl` or `LIST`/`DISP` after `data` — and 8-bit samples are unsigned (centred
on 128), so the decoder shifts them to signed 16-bit rather than reinterpreting.

`resampleTo` converts to a mixer rate at load time; linear interpolation is
enough for source material this low-fi. Level cues (`Letsgo`, `Yippee`, …) map
to `SOUNDS\<CUE>.WAV` via `soundAssetName`. Playback itself is Phase 5.

---

## 2. Verification strategy — the differential oracle

Because the original loads at a fixed base and we can read its memory live, we can
run it as a **reference implementation**:

1. Drive the original to a known level state.
2. Each tick, read lemming positions, states, counters and terrain out of its memory.
3. Step our simulation with identical input and assert the states match.

This converts "1:1 mechanics" from an aspiration into a regression test. It is how
we pin down sub-pixel movement, rounding, tick ordering and the exact frame on which
a builder places a brick — the things that are impossible to get right by observation.

### 2.1 Driving the original — solved

The game reads input exclusively through DirectInput and **ignores every form of
Windows input injection** (`SendInput`, `mouse_event`, `keybd_event` all have no
effect). That blocked automation entirely.

Its whole input surface turns out to be two `IDirectInputDevice::GetDeviceState`
calls:

```
.text:00483B5D   GetDeviceState(0x10,  &DIMOUSESTATE)    device ptr at this+0x28
.text:00483E1A   GetDeviceState(0x100, keys[256])        device ptr at this+0x100
```

`tools/dinput-shim` is a 32-bit `dinput.dll` proxy that forwards everything to the
real `SysWOW64\dinput.dll` and patches exactly two COM vtable slots:

```
IDirectInputA        vtable[3]  CreateDevice     -> catch each device
IDirectInputDeviceA  vtable[9]  GetDeviceState   -> serve state from shared memory
```

No wrapper COM objects, no DLL injection — `dinput.dll` is not a KnownDLL, so the
application directory wins the search order. Confirmed in the live process:

```
DINPUT.DLL   C:\progs\Lemmings Revolution\DINPUT.DLL   <- shim
dinput.dll   C:\WINDOWS\SysWOW64\dinput.dll            <- real, loaded by the shim
```

`tools/harness/lrinput.py` writes the shared block. **Verified end to end:** the game
polls at 144 Hz, and scripted `DOWN` taps walk the main menu one item per tap while
scripted relative mouse motion moves the same highlight — with no human present.

Two properties that matter for the oracle:

- **Frame-accurate.** `mouse_frame` / `key_frame` increment on every read, so the
  harness can inject exactly one tick of motion and block until it is consumed
  (`consume_deltas` zeroes the deltas after each read). No sleep-and-hope.
- **Non-invasive.** With `enable = 0` every call passes straight through, so the game
  plays completely normally and the shim can stay installed permanently.

---

## 3. Phases

### Phase 0 — Groundwork
- Repo skeleton matching Carcer conventions: C++23, CMake 3.28+ / Ninja, `src/Makefile`
  convenience front end, clangd, `Name.h`/`Name.cpp` pairing (`.hpp` = header-only),
  sources registered in `cmake/*_sources.cmake`, presets for gcc/clang debug+release,
  ucrt64, and emscripten.
- Dependencies: **plain SDL2 + a hand-rolled OpenGL layer + STL.** No SDL2W, no BMIN —
  they are 2D/sprite-oriented and would be bypassed by a GL cylinder renderer anyway.
- Ghidra install (Java 24 is already present), import the exe, seed with §1.1.

### Phase 1 — Asset layer — **done**
- `lembox` reader, base + patch overlay, reading the user's install at runtime.
- LRZ decoder ported from the published reference.
- `.ALF`, `.LTX`, `.WAV`, `LEVELMAP.DAT`, `LEVELS.MAP`, `RELEASERATE.PC`.
- `lrtool` CLI: list, extract, convert.
- **Milestone: all 10,376 textures decoded to PNG and visually verified.**
  `.WAV` closed last: decoder + load-time resampler, all 73 retail files.

### Phase 2 — `.LVL`
- Ghidra on the `SkyIslands` loader; incremental parser; debug visualiser.
- **Milestone: one level's cylinder geometry and terrain mask on screen.**
- Closed on the file side: cylinder + surface lattice → playable mask,
  skills trailer, entrance/exit + 417 other placed models (parse only),
  skin ≠ solidity, PolyList 5/7 = draw attrs, ring stride **derived not
  stored**. Open: live-process mask dump vs our raster (§1.6).

### Phase 3 — Simulation (headless, deterministic, unit-tested)
- Terrain mask with horizontal wrap; fixed timestep; no rendering dependency.
- Lemming state machine: walker, climber, floater, bomber, blocker, builder, basher,
  miner, digger — plus the Revolution-specific mechanics visible in the assets
  (anti-grav shafts, trampolines, water, acid, axes, evil rats, cylinder rotation).
- Validated continuously against the differential oracle.

### Phase 4 — Rendering — **done**
- SDL2 + OpenGL, targeting a GLES3-compatible subset so WebGL2 is nearly free.
  One shader pair, `#version 330 core` or `#version 300 es` chosen at runtime,
  nothing outside the common subset. **Caveat: never actually run on GLES or
  WebGL2.** No EMSDK is activated here, so that half of the claim is a code
  property, not an observation, and it belongs to Phase 6 to confirm.
- Flat unwrapped 2D debug view — `--native` opens it at the original's 640×480,
  `--size WxH` at anything else.
- Wrapped onto the cylinder using the INN/OUT texture grid: `C` toggles the
  projection, `T` the skin. Both surfaces are drawn, inner first, so the outer
  one covers it where it is opaque and the inner shows through its gaps —
  which is how the original shows the far side of the tube.
- `.SPR` sheet-based sprite animation, from `LEMMINGSLO.SPR`.

What is *not* in this phase, and is not pretended to be: the skin is scenery
(opaque wrap; see §1.6) and PolyList is draw data for the surface, not
collision. What a lemming can stand on is the surface-lattice mask. Whether
that matches the original's in-memory terrain is the remaining RE question.

### Phase 5 — Game shell
Menus, level select, save games, `.LTX` localisation, audio. FMV is an open question —
the `.avi` codecs are not yet identified.

Audio is started: `src/audio/Mixer` opens an SDL device, loads cues through
`Wav`, and the shell drains `World::takeCues()` onto retail names (Letsgo on
restart; Chink / Ohno / Explode / Splat / Yippee from the sim). Music, menus
and the rest of the shell are still open.

### Phase 6 — Platform targets
Linux, macOS, and WebAssembly via Emscripten.

---

## 4. Risks, ranked

1. **`.LVL` format** — high, and on the critical path. Everything downstream waits on it.
2. **Exact physics constants** — medium. The oracle de-risks this substantially, and
   driving the original is now solved (§2.1).
3. **3D model geometry** — low/medium. Assumed to live in the `.LVL` PolyLists;
   unconfirmed. `TEXTURES\MODELS\...` implies separate model data somewhere.
4. **FMV codecs** — low. Cosmetic, and deferrable to the very end.

---

## 5. Sources

- [Lemmings Revolution file formats — Lemmings Forums (ccexplore)](https://www.lemmingsforums.net/index.php?topic=1439.0)
- [Lemmings Tools & Documentation — The Lemmings Archive](https://www.camanis.net/lemmings/tools.php)
- [Lemmings Revolution — Wikipedia](https://en.wikipedia.org/wiki/Lemmings_Revolution)
