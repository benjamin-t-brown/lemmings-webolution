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
* Where the ring stride `Y` comes from. Not stored; searched geometrically,
  and the corpus survey below shows the search is what separates the 13
  playable levels from the other 88.
* Why `grid0` and `grid1` differ in role. On `MM_CLIMB_002` `grid0` has 194
  populated slots against `grid1`'s 6,141, yet the walked links were found
  in `grid0` and only `grid0` gets the dropped-edge pass.

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
kinds/sizes remain; they do not change that reading. The live-process check
below is what proves the sparse lattice is what the original actually walks
on.

#### Live-process terrain dump -- the harness, and two false starts

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
  Those are 640x480 frames, which is the clue the next two rounds missed: that
  run was against the stock exe, and the rounds that then failed to click a map
  node were against the 2560x1440 one.

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

*That experiment is done.* The shim now counts `GetDeviceData` calls, every
`GetDeviceState` call, and the last `cb` it was asked for. Watching those
across a held click settles two things and overturns one assumption:

* **`GetDeviceData` is never called. Not once.** `data_frame` stays at 0
  through menus, the map, and a button held for over a second. Buffered input
  is ruled out, and the shim's original note -- that the game's whole input
  surface is two `GetDeviceState` calls -- is confirmed rather than assumed.
* **The mouse is polled as often as the keyboard**, exactly: the two counters
  advance in lockstep (578/578, 780/780, 834/834), one of each per frame. So
  the mouse path is live and the button bytes are being offered to a game that
  is reading that structure every frame.
* **`PrintWindow` returns an all-black bitmap whenever the game window is not
  foreground.** This is the important one, because it is invisible: the
  capture succeeds, the image is simply black, and every vision check then
  reports whatever it reports about darkness. It is almost certainly what
  produced the earlier "treated dark in-level frames as main menu" symptom,
  and it means an unknown number of the aiming and clicking results above were
  measured against frames showing nothing at all. `aim.require_live_frame`
  now raises `BlankFrame` rather than letting a run proceed on one, and
  `aim.focus` reports whether Windows actually granted the foreground change
  (it often refuses).

**Next step for this item -- answered below.** The click test was redone under
`require_live_frame` with the foreground confirmed, and it overturns the
reading above: the button bytes *do* reach the game.

#### Live-process terrain compare -- done, and it passes

Settled. The short version: the engine keeps **no occupancy buffer at all**,
so there is nothing to diff against `lrtool lvlmask`; but it does keep the
.LVL surface lattice byte-for-byte, and the lemmings it simulates on that
lattice stand exactly where our raster says the floor is.

**Mouse buttons reach the game. The earlier negative was wrong.**

The engine's own mouse structure says so directly. `InputManager::Update`
(`.text:0048316B`) polls two devices through `this + 0x08` (keyboard) and
`this + 0x0C` (mouse), and the mouse poll at `.text:00483B5D` copies
`DIMOUSESTATE` into that object:

```
+0x00 int   button0 down          (rgbButtons[0] & 0x80) != 0
+0x04 int   button1 down
+0x08 int   button0 down, previous frame
+0x0C int   button1 down, previous frame
+0x10 byte  button0 press edge    down now && !down last frame
+0x11 byte  button1 press edge
+0x14 int   time of the last button0 release
+0x1C byte  button0 "click"       cleared every poll; set when two releases
                                  land less than 500 ms apart
+0x20 int   dx          +0x24 int dy
```

Writing `0x80` into the shim's button byte flips `+0x00` from 0 to 1 and back,
and a quick press-release-press-release zeroes `+0x14` -- which only happens
inside the `< 500 ms` branch that also sets `+0x1C`. So both the plain button
state and the click edge arrive. `+0x1C` is a **double**-click, not a single
one: nothing writes `+0x14` on a press, only on a release, so the comparison
at `.text:00483CD9` measures release-to-release. Its one consumer
(`.text:00454739`) is the nuke button -- `hovered == 9`, double-click, count
`> 0`, then `GameObject::StartNuke` -- which is exactly the original's rule.

**What actually blocked level entry: the resolution patch.**

With clicks proven, the main menu, the language submenu and the level map's
"Back to Main Menu" all respond to a scripted click. The level map's **gold
nodes do not** -- not to a short click, a 0.6 s hold, a double click, a right
click, a drag across them, or a 3x3 grid of positions around the centre of
each of the three start nodes, all with the crosshair measured dead on target
and every frame confirmed live.

They respond immediately under the **stock `Lemmings Revolution.exe`**. The
map's text widgets are the 2D layer, which `lr_patch.py` already documents as
*not* scaling with resolution; the nodes are 3D-picked, and at 2560x1440 the
pick misses. So: **drive the original at its native 640x480 for anything
involving the level map.** dgVoodoo still presents a 2560x1440 window and
scales the 640x480 image into it, pillarboxed, so capture pixels map to game
pixels as `image = (320 + 3*gx, 3*gy)`.

**Driving by memory instead of by screenshot.**

Three statics make the harness independent of `PrintWindow`, which is the
component that has produced every wrong conclusion in this item:

```
0x004C36A0 / 0x004C36A4   cursor x, y in render pixels (clamped to render size)
0x004CC134                UI state: 0 = playing, 3 = frontend, 5 = results
0x004C6A08                the game object; +0x55B8 InputManager, +0x572C state
```

Both were found by experiment, not inference -- the cursor pair by injecting a
marker delta and diffing `.data`, the mouse object by searching the whole
address space for that same marker. `tools/harness/lrstate.py` reads them and
`drive.py` aims with them. Aiming is also simpler than the screenshot aimer
suggested: frame-synced motion is **linear**, about 1.7 px per unit at any
distance, because acceleration only bites when several units land in one poll.
The whole screen is two or three iterations away.

Two further facts worth keeping. The poll rate reports the foreground state
for free -- 144 Hz when the window has focus, 15 Hz when it does not. And
keyboard taps need a **long** hold (about 20 polled frames); 2-4 frames is
reliably ignored, which is why an earlier round concluded RETURN did nothing.

**There is no occupancy buffer in the engine. Scanned for, not assumed.**

With `MM_CLIMB_002` loaded and simulating, a sweep of all 182 MB of committed
memory finds:

* **no byte-per-cell mask.** Not one page outside a DLL is wholly `{0, 1}`
  with any set cells; the only such page is 4 KB inside a system DLL.
* **no small-alphabet grid either.** Widening to "at most 4 distinct byte
  values, 2-45% non-zero" only turns up 4 KB texture/alpha pages (the
  `{0, 126, 255}` family) and one record array whose non-zero bytes fall on a
  fixed stride -- a struct array, not a bitmap.
* **no face list over the surface pool.** No run of 256 or more `u32` lying
  entirely below the 897-vertex count exists in the application heap. The only
  such runs are in the graphics driver's address range, and the big one is
  897 x 6 -- a D3D index buffer.

So `Terrain` is **our** representation. The original does not rasterise.

**What the engine does keep is our input, verbatim.**

`find_level_data.py` hits all four needles with a level loaded (and still
nothing at the menu):

* all **897 surface vertices**, bit-identical IEEE doubles, contiguous at
  `0x0286E7A8` -- 897/897 equal to the file;
* `grid0` byte-identical at `0x00B90048`, with `grid1` immediately after it;
* a cylinder tile texture name.

So the engine's terrain input is exactly the lattice our `SurfaceMesh` reads.

**The compare, in the only form the engine allows: its lemmings.**

A walker is standing on terrain by definition, so each walker position read
live out of the original is one sample of "solid here". A lemming is three
consecutive `f64` in the heap on the tube (radius 54..62, height inside
0..96); that matches a few hundred addresses, mostly static geometry, so the
set is polled for a couple of seconds and only the points that move a little
every frame without changing height are kept.

Against `build/live_dump/ref_climb_mask.bin` (256x192, 12.70% solid):

```
standing on a solid mask cell   31/32 and 26/27, on two runs
solid continues below            same
clear above                      27/32, 23/27
```

The single failure in each run is the same kind of point -- near the top of
the tube at the wrap seam, `y ~ 84.4`, which reads as the entrance hatch
animation rather than a walker. Every lemming at `y = 32.0` or `32.19`, the
band they actually walk in this level, lands on a solid cell with solid
continuing three pixels below it. `tools/harness/live_terrain_check.py`
reproduces this and writes `build/live_dump/live_terrain_check.png`, an
overlay of the sampled positions on the mask: the track lies along one of the
mask's platform lines.

That is the positive result this item was for. It does not prove the raster is
correct everywhere -- it samples one ledge of one level, and a mask with extra
solid cells would pass it unchanged. What it does establish is that where the
original simulates a floor, we have one, at the same place.

**That result was withdrawn, and the re-run does not restore it.**

§2.2 withdrew the 31-of-32 figure: the finder could not tell a lemming from a
transformed surface vertex. `tools/harness/check_mask.py` is the replacement.
It keeps a billboard only when its position changes on the 200 ms step
lattice, by one of the 256 columns per change. Two runs, both with the stock
exe in the foreground (the shim polled at 144 Hz across the sample; median
tube spin 0.00000 rad/s). The reference mask was the existing
`build/live_dump/ref_climb_mask.bin`, 256×192, 6,244/49,152 cells solid
(12.70%).

The first run caught one walker (30 s, 252 steps). The second waited until
all ten had been released and caught six (20 s, 1,012 steps). They were the
same patrol both times. On the six-walker run every track sat at y = 32.190
with no height change, step gap 201.2–201.3 ms, |dcol| 0.983–0.994, columns
128.5–167.8, span 38.83–38.87.

```
column has some solid in it          1012/1012
  baseline, any solid in the column    213/256 = 83.2%
foot cell itself solid                  592/1012
  baseline, solid at row 127            186/256 = 72.7%
solid in one of the next three rows     578/1012
column empty from top to bottom         0
rows occupied                           127, all 1012 samples
floor-top of the lowest solid run       62, 94, 95, 96, 126
reversal columns                        128, 129, 166, 167
```

The script's own exit condition is the first line and the empty-column line,
and that condition was met. It is too weak to count as a confirmation. 83% of
columns already have solid somewhere, and the foot cell was solid *less*
often (58.5%) than row 127 is solid (72.7%). 420 of the 1,012 samples are in
columns 115–141, which the raster leaves empty at row 127 — the gap between
the two platforms — and the other 592 are on the platform from column 142 to
234. The reversal columns are not the raster's walls. Those walls, solid
through rows 110–125, stand at 22–26, 62–70, 78–86, 110–114, 142–146,
170–178, 186–194 and 230–234. The openings between them are 35, 7, 23, 27,
23, 7 and 35 columns wide. None is the 38.8 columns these lemmings patrol.

The offset sweep does not pick out a rotation. Shifting the samples by k
columns makes every foot cell solid at 108 values of k, in two bands,
k = 14..67 and k = 150..203 (54 each). A 40-column span dropped onto either
of the two 93-column platforms at row 127 can sit in 93 − 40 + 1 = 54
places, and the five best k the script used to print are the end of the
second band. A patrol of 38.8 columns is also about what the 35-column
openings would be with the radius-2 raster brush taken back off each wall,
and that length fits two of those openings, so it is not a unique angle
either. A search for a second in-memory copy of a uniquely radiused surface
vertex, rotated by one common yaw, found only the file's own coordinates
(angle difference 0). The loader's unroll was left alone.

One attempt to read the column of a lemming as it left the entrance — a bulk
snapshot of the billboard window across a later release — found no
monotonically falling f64, so that did not disambiguate an angle either. It
is not a fall measurement.

The withdrawn confirmation stays withdrawn. This run replaces the pending
re-check, and the raster is not verified. `Rules` was not touched.

**The miss is a dropped edge, not a wrong angle.** `nearest_lattice.py`
unrolls the same six tracks and the `.LVL` surface with the loader's
`atan2(-x, z)` and no fitted yaw.

Every sample is at y = 32.190. The nearest surface vertex is on that ring:
`|dy|` is 0.190, and the signed column residual has median −0.03. Those
vertices sit on the 8-column angular step. One sample per column has median
`|dcol|` 2.00, which is what a uniform walk between posts eight columns
apart measures. There is no constant offset to apply.

The mask is a faithful picture of the edges `rasteriseSurface` strokes, and
an unfaithful picture of the ledge. Samples within two columns of a stroked
edge: 592/1012, the same 592 whose foot cell is solid. The other 420 are
0/420 that close to a stroke. They sit at the left reversal, median 15
columns from the nearest stroke, and still on the ring.

The vertices under that hole are in the lattice. The outer y = 32 vertices
at columns 120, 128 and 136 are `grid0` neighbours at key+16 (slots 16689,
16705, 16721), and the walkers travel 128–144. A face is only kept when the
thickness partner at key−1 is present, and that slot is empty on those
three. From column 144 the partner is present, and the strokes resume
(144–192). The hole is the builder discarding an outer-only angular link.

Not every dropped +16 link is floor. Counting them without a radius cut
gives 32 at y = 32, a complete ring, and stroking that would pave the
tube. The outer ones (radius past the midpoint of 50 and 60) are ten
links in two arcs: columns 112–144 and 232–24. The 420 samples that
missed the old mask sit on that first arc, 420/420, median column
distance 0.

The left reversal is the same kind of omission, turned vertical. At
column 128, key+8224 joins y = 32 to y = 48 on the outer skin and the
thickness partner is empty at both ends, so no quad was emitted. The
same shape at columns 0, 8, 16, 240 and 248 sits on the other arc.
Links that drop from y = 32 to y = 16 exist at every angular step and
are not walls; those stay out.

`rasteriseSurface` now strokes both sets, from grid0 only. grid1 is the
dense index of the same pool and running the pass on it fills the ring.
The climb mask goes from 6,244 solid cells to 9,496 (19.3%). Column 132
on the ledge is solid, the column-128 wall reaches y = 40, and column
136 — which the walkers cross — does not grow a wall. The right
reversal is untouched: they turn at ~168, the strokes continue to 192,
and there is no outer-only vertical at 168. The nearest one is the
already-emitted wall at column 172, four columns past where they stop.

#### Corpus survey: the mask is playable on 22 of 101 levels

The live compare above is one ledge of one level. This is the breadth check,
and it needs no running original, which is what makes it cheap enough to run
on every level: **a lemming released from an entrance has to land on
something, and a drop past `Rules::splatDistance` kills it.** So for each
parsed entrance, is there a solid cell below it in the rasterised mask, and
is the drop survivable? `lrtool lvlstats` asks that of all 102 levels and
`test_corpus` holds the answers as a baseline. Both share
`src/assets/LevelStats.cpp`, and the splat distance comes from
`World::Rules` rather than a literal, so the oracle tracks the simulation.

Measured 2026-10-14, splat distance 60 mask rows:

| | |
|---|---|
| level entries, and how many parse | 102 of 102 |
| of those, rasterising to a non-empty mask | **101** — `VH13_KF_1` yields nothing |
| median solid coverage | **3.9%** |
| median drop from an entrance to the first solid cell | **132 mask rows = 66 world units** |
| entrances | 143, of which 132 have a floor below them |
| entrances whose drop is survivable | **35 of 132** |
| levels releasable — every entrance with a floor survivable | **22 of 101** |
| entrances with nothing solid below them at all | 11, across 10 levels |

So on most levels a lemming leaving the hatch would fall two thirds of the
tube's 96 units and splat. The split is not spread evenly: it tracks the
per-level ring stride that `detectRingStride()` settles on.

| ring stride | levels | median solid | median drop | releasable |
| --- | --- | --- | --- | --- |
| **8224** | 13 | **22.0%** | **26 rows** | **12 of 13** |
| 0, none detected | 49 | 3.2% | 126 rows | 5 of 49 |
| 4096 | 12 | 3.2% | 135 rows | 3 of 12 |
| ten others, 1–5 levels each | 27 | 3.9–14.6% | 26–172 rows | 2 of 27 |

Stride 8224 is 12 of 13 releasable; every other stride together is 10 of 88.
`MM_CLIMB_002` is in the 8224 family (23.0% solid, a 22-row entrance drop)
and so is `MM_BASH_002` (27.7%, 24 rows).

**What this supports.** The lattice reading produces terrain a lemming could
be released onto for one family of 13 levels, and sparse scaffolding for the
other 88. The level the raster was tuned against is inside that family and
is near the top of it by coverage, so the live confirmation in the previous
item — real as it is — was taken on the most favourable level in the corpus.

**What it does not support, stated rather than smoothed.** This is evidence,
not proof, and there is a real counter-case. A tall hatch is perfectly
legitimate on `MM_FLOAT`: a floater survives any drop, and that level's
entrance sits 140 rows up. The survey cannot tell a missing floor from a
level designed around a long fall, and the 50 `anti_grav` placements in the
corpus are another way a drop could be broken by something we do not
simulate. What the counter-case does not cover is the rest of the tutorial
set, where the skill rules it out: `MM_DIG_000` (1.35% solid, a 144-row =
72-unit drop), `MM_BOMB` (1.88%, 142 rows) and `MM_BUILD` (1.91%, 124 rows)
have no floater and need a floor under the hatch for the level to be
playable at all. Three levels is a small number, so read this as a strong
indication that floors are missing, and the stride table as where to look —
not as a measurement of the loader being wrong.

The numbers are also a baseline rather than a target. `test_corpus` pins the
parse counts exactly and everything else directionally, so the next change
to the face rule in `SurfaceMesh.cpp` moves one of them visibly.

#### Where to go next

* **The ring stride, read rather than searched.** This is the critical
  path, and the corpus survey above is why: the stride families separate
  the playable levels from the scaffolding almost perfectly. `Y` is not a
  stored `.LVL` field -- established across all 102 levels -- so
  `detectRingStride()` searches multiples of 16 for a median pair distance
  near one ring spacing, and returns 0 on 49 of them. The spread it finds
  (8224, 4096, 4128, 3104, 1520, 1568, 2032, 2544, 2592, 4608, 5120, 1008)
  reads like a geometric best fit landing on the real lattice 13 times and
  on a coincidence otherwise; the recurring low bytes `0x20` and `0xF0`
  are a hint and nothing more. The way to settle it is the one §2.3 used
  for the fall: read how the original indexes its own mask. The entry
  points are already known from that work -- the mask probe is
  `FUN_0040DE60` on the world object `0x004C6A10`, and `FUN_0040BAA0`
  returns its height.
* **De-fit `rasteriseSurface` / `collectFromGrid`.** Three things in
  `src/assets/SurfaceMesh.cpp` were fitted to `MM_CLIMB_002` and should
  come out of the same code read, not out of matching a ledge. They are
  left in place deliberately until there is something better to replace
  them with, because they are what makes the climb ledge pass.
  * the vertical-link filter requires the foot of a wall to sit at
    `y >= 31`, which is that level's ledge at y = 32 written as a
    constant. A ledge at y = 16 elsewhere is discarded as bottom skirt.
  * the half-step rule -- a populated `key+8` ends the floor at this
    vertex and raises a wall -- came from one turn at one column.
  * the chord window 8.0–13.5 and the 1.5 distance tolerances are tuned,
    not derived.
* **The fall, then the other skills.** The climb ledge, the climb rate and
  now the fall are settled (Phase 3; the fall is read off the binary, not
  measured, and still owes a live series). Gadgets stay unmeasured.
* **Hazard / gadget behaviour, after the mask.** The harness can reach a
  level and read world positions. Placed-model poses are already parsed
  (`LevelObjects::placed`); anti-grav, teleports, deathrollers, and the
  rest are not simulated.
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

### 2.2 Reading the rules off the original -- partly done

The `World::Rules` constants were classic-Lemmings values standing in for this
engine's. Three of them have now been settled against the running original, and
one was wrong by a factor of four. The rest are still guesses, and
`src/sim/World.h` now says so field by field.

**Entry, which is the part that actually costs time.** Both capture paths are
blind here -- `PrintWindow` *and* a desktop `BitBlt` return an all-black bitmap,
because dgVoodoo presents through a D3D11 swapchain -- so navigation is entirely
memory-driven. Four facts make it repeatable (`tools/harness/goto_level.py`,
`restart_level.py`):

* `0x004C416C` is the frontend's own screen id: 3 = main menu, 4 = the Ja/Nein
  confirm, 5 = the level map. `ui_state` is 3 for all of them, so it cannot tell
  them apart and nothing else could either.
* The main menu's highlight index is a **heap** variable, so it moves every run;
  `find_menu_index` locates it by diffing memory across a DOWN press and keeping
  the slot that cycles 0..4. This matters: the fifth item is "Beenden", and a
  blind RETURN closed the game outright during this work.
* The first gold node is at render pixel **(206, 204)**, established by clicking.
  1.6's (825, 612) was measured under the 2560x1440 build and scales as
  `(x*640/2560, y*480/1440)` -- *not* through the dgVoodoo letterbox, which is
  what the earlier note implied and what sent four rounds of clicks to (168, 204).
* `consume_deltas` is **0** in the live shim block, so the last relative motion
  written is re-read every frame and the cursor keeps sliding -- about 60 px a
  second -- until it clamps at the screen edge. Every click of an earlier sweep
  landed somewhere other than where it was aimed. `drive.aim_at` now writes a
  zero delta before it returns.

**Statics found, by known-value and rate diffs over all writable memory:**

| Address | Meaning | How |
|---|---|---|
| `004CBA78` | lemmings released so far | the only application slot that goes 0 -> 10 across a level's releases |
| `004CBAA8` | the level's time limit, whole seconds (300) | matches `LEVELMAP.DAT` |
| `004CBAAC` | the clock, whole seconds remaining | the only static counting down at 1/s |
| `004CB920` + 0x18k | skill panel buttons, x = 20, 60 ... 340 | strided table beside the clock |

**Measured -- the release interval is real seconds, from the shipped table.**
`RELEASERATE.PC`'s doubles are literal seconds and are used as such. On
MM_CLIMB_002 the engine released a lemming every **2.889 s** (mean of 9 gaps,
min 2.865, max 2.925); the table's entry 20 is **2.8865 s**. So that level's
release rate is 20, and the `ReleaseRates` -> `releaseIntervalTicks` derivation
is right.

**Measured -- the simulation steps 5 times a second, and is not frame-locked.**
The render and input loop runs at the display rate: the shim's `GetDeviceState`
counter and the engine's own transformed-vertex buffer both tick at 144 Hz with
the window foreground. A lemming's position does not. It changes on a flat
**200 ms** period, and the same 200 ms holds at 144 fps, at 30 fps (dgVoodoo
`FPSLimit = 30`; 200.03 ms median, no jitter at all) and at the ~27 Hz the game
drops to when it loses the foreground. The release interval is likewise
unchanged across that five-fold frame-rate change (2.888 s at 29 Hz against
2.889 s at 144 Hz). So the engine runs its simulation off a real-time
accumulator, and **5 steps per second is the rate**, not a frame-rate artefact.

That was the factor-of-four error: `kTicksPerSecond` was 20.

**Measured -- a walker advances exactly one lattice column per step.** Taking
the centroid of the four vertices the engine writes for a lemming, and unrolling
it the way `SurfaceMesh` does, the change per step is 1.000 of MM_CLIMB_002's
256 surface columns -- repeatedly, on several lemmings, in both directions,
never a fraction of one. So `walkSpeed = 1` was right *per step*; it was the
step rate that was wrong, and together they had walkers moving four times too
fast. It also says the horizontal mask resolution is the engine's own unit
rather than a choice of ours, which is independent corroboration of 1.6's
lattice reading.

**Not measured, and why.** Everything that needs a lemming to leave the ground:
`fallSpeed`, `splatDistance`, `floatAfter`, `maxStepUp`, `lemmingHeight`. The
only level reachable without solving level-map navigation for arbitrary nodes is
MM_CLIMB_002, whose lemmings walk out of the entrance onto flat ground and never
fall -- a scan for any f64 in the application heap falling monotonically through
the tube's height band, triggered on a release, finds nothing, repeatedly.
`bomberFuse` and the builder / basher / miner / digger intervals and reach also
need a skill assigned, which needs the skill panel driven and a lemming picked;
the panel's button table is located (above) but that work was not done. Those
fields keep the classic values and `World.h` labels them provisional.

**Correction.** The climb was measured after the paragraph above; Phase 3 has
the series and the click path. An unassigned release still appears at
y = 32.190, so the paragraph's scan stands. The fall is one constant for the
hatch and a walk-off, and it is still unread.

**A correction to 1.6.** `live_terrain_check` identifies lemmings as "an f64
triple on the tube that moves smoothly at constant height". On a level whose
tube is turning -- an earlier session saw the camera orbit at about 0.25
rad/s -- that description fits hundreds of transformed *surface vertices*:
they move smoothly, at constant height, at exactly a lemming's radius. Its
31-of-32 result is therefore not safe as stated; it may have sampled scenery.
The spin is the discriminator (`tools/harness/find_lemmings.py`): every
surface vertex shares one angular velocity, so a lemming is an outlier
against the median. Re-running that check properly was listed under Phase 3.
It has since been run, with `check_mask.py` rather than `find_lemmings.py`.
On MM_CLIMB_002 the camera was still (median spin 0.00000 rad/s), so the spin
test degenerates, and the 200 ms lattice is what separated the walkers. The
numbers are in §1.6. The re-check after the half-step wall was added
does confirm the ledge: ten walkers, 1,231/1,231 foot cells solid, turns
at both walls. The unroll was not changed.

Harness added, all re-runnable: `goto_level.py`, `restart_level.py`,
`fresh_level.py`, `find_counters.py`, `find_clock.py`, `find_lemming_count.py`,
`find_fallers.py`, `find_lemming_record.py`, `find_lemmings.py`,
`measure_release.py`, `measure_tick.py`, `measure_motion.py`, with
`diffscan.py`, `dump_records.py` and `sample_fast.py` as general tools.
`read_steppers.py` is the odd one out and needs no running game: it is the
static read of 2.3 against the shipped exe.

### 2.3 Static reading with REA — in progress

Running the stock exe under dgVoodoo is what blocked the fall. The hatch
never leaves y = 32.190, and the drop off the y = 48 ring was sampled too
coarsely to show a step (§Phase 3). [REA](https://github.com/morluto/rea)
(cloned at `.rea`, gitignored) decompiles the same exe through Ghidra, so
the constant could be read from the instruction that writes Y instead. A
decompiler line is not a measurement until the address and the bytes are
cited, which is the standard everything below is held to.

`fallSpeed`, `floatSpeed`, `floatAfter` and `splatDistance` have all been
set from this section. That is a weaker warrant than the measured fields
have and `Rules` says so in those words: a read of the shipped bytes, with
no live series behind it. What makes them more than plausible numbers is
that the same arithmetic reproduces two *measured* fields exactly off the
same instruction shape -- so the units are not a guess even though the fall
itself was never sampled. Nothing else in `Rules` may be promoted on a
static read alone without that kind of cross-check, and the fall still owes
its live run.

**`fallSpeed` was first set to 10, off the wrong state, and is now 8.** The
first read took 25.0 from the state at `0x004B0C60`, which is a faller by
its animation and is the climb's instruction-for-instruction sibling. The
lemming's own fall is the state at `0x004B0A20`: it is the one that carries
the splat test, the floater check and the umbrella, and its constant is
20.0, so 4 world units per tick and 8 mask rows. `0x004B0C60` has none of
those three, so it is some other descent. Both reads are below; the second
supersedes the first because the state it belongs to is identified by what
it does to the lemming, not by resembling the climb.

**Setup, already done on this machine.** Ghidra 12.1.4 is at
`C:\tools\ghidra_12.1.4_PUBLIC` (official zip, SHA-256
`ddac49f903da9d5bac833e5cc79395098b9c33cfd3279be5f31bd00387d2d4db`).
JDK 24 at `C:\Program Files\Java\jdk-24` is accepted. Cursor's MCP entry
is `C:\Users\bbrown\.cursor\mcp.json`; a session started before that file
existed does not have the tools, so restart Cursor or use the CLI. The
snapshot here was written by `rea-agents` 6.4.0; `@latest` has since moved
to 6.5.0, which rejects that snapshot, so pin the version (see the end of
this section). The git checkout has no Windows
native addon, so run the npm package, not `node .rea/scripts/rea.mjs`.
Hopper is not installed and is not offered on Windows. The target is the
stock 32-bit exe, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`. Snapshot: `C:\tools\lr-rea-snapshot.json`. Each
CLI invocation starts Ghidra again (about 40 s); an open MCP session
keeps the database.

```powershell
$env:GHIDRA_INSTALL_DIR = "C:\tools\ghidra_12.1.4_PUBLIC"
$env:JAVA_HOME = "C:\Program Files\Java\jdk-24"
$env:REA_ANALYSIS_PROVIDER = "ghidra"
$env:REA_GHIDRA_STARTUP_TIMEOUT_MS = "330000"
npx -y rea-agents@6.4.0 xrefs "C:\progs\Lemmings Revolution\Lemmings Revolution.exe" 0x4137ca --provider ghidra --snapshot C:\tools\lr-rea-snapshot.json --format json
```

`search`, `decompile`, and `function` take the same path, provider, and
snapshot. `search` is literal over Ghidra-defined strings.

**Read so far (static, 2026-10-14).** Not a live series.

- `FallSplat` at `0x004C0CD4` is a sound name. `FUN_0045e51f` loads it
  beside Letsgo, Yippee, Chink, Explode, Splat. It is not a step size.
- `Climber topping out` at `0x004BF310` is referenced from `FUN_004139bf`
  (`0x004139BF`). That function tests mask cells through `FUN_0040de60`
  on the object at `0x004C6A10` and logs topping out or `Climber hit head`
  (`0x004BF2FC`).
- `Lemming moved to %d, %d` at `0x004BF464` is inside `FUN_004190f3`,
  after the string `Mine collision check`. That path writes two ints
  back into doubles at `+0x08` and `+0x10`. It is not the general mover.
  Do not start from that string.

**There is no caller to decompile: the states are virtual.** `xrefs` on
`0x004137CA` returns exactly one address, `0x004B0964`, and it is in
`.rdata`, not `.text`. It is a vtable slot, so every call to the climb
stepper is a virtual dispatch. The vtables are 6 slots long, and
each is referenced from exactly two places: its constructor and its
destructor. Scanning `.text` for immediates landing in
`0x004B0900`–`0x004B0A80` is what separated the vtable starts from the
double literals the compiler parked between them — both are referenced,
but a literal is referenced from any number of sites and a vtable start
from two. Those interleaved literals also break the 0x18 stride, so
`read_steppers.py` finds the tables by their shape instead: slots 4 and 5
are the same two base stubs, `0x004011D0` and `0x004011E0`, in every one.
Slot layout, read off the climb vtable at `0x004B0960`:

| slot | climb | role |
| --- | --- | --- |
| +0x00 | `0x00413C40` | deleting destructor |
| +0x04 | `0x004137CA` | **step this state for one tick** |
| +0x08 | `0x004137A3` | **on-enter hook** — sets the animation, writes this state's constants |
| +0x0C | `0x004137BD` | on-exit hook, an empty stub in most |
| +0x10 | `0x004011D0` | common base stub |
| +0x14 | `0x004011E0` | common base stub |

46 vtables have this shape.

**Slot 2 is an enter hook, not a name getter, and the string is the `.SPR`
strip name.** The first read of it went by a fixed prologue,
`55 8b ec 51 89 4d fc 68 <str>`, which named 7 of the 46. That prologue is
incidental: what every one of them actually does is

```
68 <str>        push  <str>
8b 4d 08        mov   ecx, dword ptr [ebp+8]     ; the lemming
e8 <rel>        call  0x0042FD25
```

and `0x0042FD25` indexes a table at `0x004E335C` by the lemming's `+0x4C`,
then looks the string up in that sheet — so it selects an animation by
name. Scanning all of `.text` for those 13 bytes finds **24 sites**, and
their strings are exactly the 15 strips in `LEMMINGSLO.SPR` —
`bash block build cartwheel climb dig drown explode fall flip float mine
shrug umbopen walk` — plus `trampoline`, which has its own sheet. Checked
against our own `.SPR` parser (`lrtool sprites`), that closed set is what
makes the naming a read and not a guess: nothing else in the binary sets
an animation, so these 24 are every state entry there is.

22 of the 46 vtables are named by a site inside their enter hook. Three
sites are inside *steppers* instead, which is why a vtable-only scan misses
them, and they are the three that matter most: `build` at `0x0040FB83`,
`float` at `0x004162B2` and `umbopen` at `0x004162FE`. The remaining
vtables change no animation on entry; one of them, `0x004B0550`, is the
landing state (below), and eight share the do-nothing hook `0x00409310`.

**The fall path is the sibling at `0x004B0C60`.** Of the two `fall`
states only one steps Y:

- `0x004B0680` slot 1 is `FUN_0040bb26`, 0x40 bytes. It advances the
  animation through `+0x78` slot `0x2C`, zeroes the remaining time, calls
  `FUN_0042f5e3`, returns 1. It never touches `+0x10`. A terminal state,
  not the faller. (`FUN_0042f5e3` is the direction flip:
  `0x0042F5F0 6b c9 ff`, `imul ecx, ecx, -1` on `+0x44`, then it writes
  the double 3.0 into `+0x358` when `+0x44` lands on 1.)
- `0x004B0C60` slot 1 is `FUN_0041a6d5` (`0x0041A6D5`–`0x0041A82F`). It is
  the climb stepper instruction for instruction with one constant
  changed. Both loop while the remaining-time double at `param_2` stays
  positive, both ask a chunk function for a time slice, both take
  `min(slice, remaining)`, both step Y, both call the `+0x78` slot `0x2C`
  animation advance with the slice, both subtract the slice, and both
  leave the loop when their own probe returns non-zero.

The instruction that steps a falling lemming's Y, with the bytes:

```
0041a758  dc 0d 40 0c 4b 00    fmul  qword ptr [0x004b0c40]   ; 25.0
0041a7b7  db 40 44             fild  dword ptr [eax+0x44]     ; direction
0041a7ba  dc 4d e8             fmul  qword ptr [ebp-0x18]     ; slice * 25.0
0041a7bd  dc 42 10             fadd  qword ptr [edx+0x10]     ; + Y
0041a7cb  e8 80 37 ff ff       call  0x0040df50               ; clamp
0041a7d3  dd 59 10             fstp  qword ptr [ecx+0x10]     ; -> Y
```

and the same lines in the climb, for comparison:

```
00413869  db 40 44             fild  dword ptr [eax+0x44]
0041386c  dc 0d b0 10 4b 00    fmul  qword ptr [0x004b10b0]   ; 5.0
00413872  dc 4d e8             fmul  qword ptr [ebp-0x18]
00413875  dc 42 10             fadd  qword ptr [edx+0x10]
00413883  e8 c8 a6 ff ff       call  0x0040df50
0041388b  dd 59 10             fstp  qword ptr [ecx+0x10]
```

The qword at `0x004B0C40` is `00 00 00 00 00 00 39 40` = **25.0**, and
`.text` references it from four addresses, all inside that one state
class: `0x0041A75A` in the stepper and `0x0041AA6A`, `0x0041AA8F`,
`0x0041AABA` in its chunk function. The climb's `0x004B10B0` is
`00 00 00 00 00 00 14 40` = 5.0 and is shared with the walk.

**The constant is world units per second, and the measurements prove it.**
The chunk functions are mirror images, and they fix the sign:

- `FUN_00413924` (climb) branches on `0x0041393F 83 78 44 01`,
  `cmp dword ptr [eax+0x44], 1`, and on that branch returns
  `((int)Y + 1) - Y` divided by 5.0 — the gap *up* to the next whole
  number, expressed as a time.
- `FUN_0041aa30` (fall) branches on `0x0041AA4B 83 78 44 ff`,
  `cmp dword ptr [eax+0x44], -1`, and on that branch returns
  `Y - (int)Y` divided by 25.0 — the gap *down*, as a time.
- Each has the same on-grid fallback: if the gap quotient is exactly 0.0
  and the byte at the state's `+0x18` is set, return `1.0 / speed`
  (`0x004139A6` and `0x0041AAB2` load 1.0 from `0x004B02D0`). One whole
  world unit of travel. So the loop sub-steps one unit at a time and
  probes the mask between units. That fallback is not the per-tick
  distance, and the earlier note that read it as the climb's step size
  had the right answer for the wrong reason.

A chunk is therefore seconds and the constant is units per second. A
whole tick is 0.2 s (measured three ways, §2.2):

| state | constant | × 0.2 s | status |
| --- | --- | --- | --- |
| walk | 5.0 at `0x004B10B0`, used `0x0041BE70` | 1 column/tick | **matches the measured `walkSpeed`** |
| climb | 5.0 at `0x004B10B0`, used `0x0041386C` | 1 world unit/tick | **matches the measured `climbSpeed`** |
| fall | 20.0 at `0x004B0A18`, used `0x0041604A` | 4 world units/tick | = **8 mask rows**, and `Rules::fallSpeed` |
| float | that 20.0 halved by `0x004B03F8` | 2 world units/tick | = **4 mask rows**, and `Rules::floatSpeed` |
| the 25.0 sibling | 25.0 at `0x004B0C40`, used `0x0041A758` | 5 world units/tick | a descent, but not the lemming's own |

The two measured rows are what fix the units. Every other state is then
the same arithmetic with its own constant.

**The lemming's own fall: vtable `0x004B0A20`.** Its stepper is
`0x004160BD` and its enter hook `0x00416004`, which sets the `fall`
animation at `0x0041601E` and clears the lemming's umbrella byte,
`0x0041602E c6 40 4b 00`, `mov byte ptr [eax+0x4b], 0`.

The speed is not a bare constant here; it comes from an accessor,
`0x00416038`, called from the stepper at `0x00416202`:

```
00416043  8a 48 4b             mov   cl, byte ptr [eax+0x4b]   ; umbrella open?
0041604a  dd 05 18 0a 4b 00    fld   qword ptr [0x004b0a18]    ; 20.0
00416050  dc 35 f8 03 4b 00    fdiv  qword ptr [0x004b03f8]    ; 2.0, if it is
0041605b  8b 15 18 0a 4b 00    mov   edx, dword ptr [0x004b0a18] ; 20.0, if not
0041607b  dc 4d 0c             fmul  qword ptr [ebp+0xc]       ; × seconds
```

and the stepper subtracts the result from Y rather than adding it:

```
00416213  db 40 44             fild  dword ptr [eax+0x44]      ; direction
00416216  dc 4d e0             fmul  qword ptr [ebp-0x20]      ; × the distance
00416219  dc 6a 10             fsubr qword ptr [edx+0x10]      ; Y − that
0041622e  e8 05 78 02 00       call  0x0043da38                ; → world 0x004C6A10
```

`fsubr` is why no instruction anywhere writes a literal −1 into `+0x44`
for this state: it descends with `+0x44` = +1, which is what the spawn
path sets (`0x0042DFE7`). The 25.0 sibling, which adds, is the one that
would need −1. That was listed as unread, and this is the read.

The qword at `0x004B0A18` is `00 00 00 00 00 00 34 40` = 20.0, and
`0x004B03F8` is 2.0. The umbrella byte `+0x4B` is set by the stepper at
`0x004162C2 c6 41 4b 01` when it starts the `float` animation. So the fall
is 4 world units per tick and the float exactly half that, 2.

**The two distance thresholds, and the splat.** The faller keeps the
distance fallen as a double at its own `+0x18`, in world units,
incremented by one per sub-step (`0x00416318 dc 05 d0 02 4b 00`,
`fadd 1.0`) — the same one-unit sub-stepping the chunk functions do. It is
tested twice:

```
004162d6  dd 40 18             fld   qword ptr [eax+0x18]      ; units fallen
004162d9  dc 1d e8 03 4b 00    fcomp qword ptr [0x004b03e8]    ; 10.0
  … and if the lemming has the floater skill, byte [+0x49],
  and this state's own +0x20 is still clear:
004162fe  68 84 f3 4b 00       push  0x004bf384                ; "umbopen"
0041630e  c6 42 20 01          mov   byte ptr [edx+0x20], 1

004165fc  dd 42 18             fld   qword ptr [edx+0x18]      ; units fallen
004165ff  dc 1d a8 10 4b 00    fcomp qword ptr [0x004b10a8]    ; 30.0
00416607  f6 c4 41  / jne      ; fallen ≤ 30.0 → land safely
00416615  8a 48 49             mov   cl, byte ptr [eax+0x49]   ; has floater
00416618  85 c9  / jne         ; → land safely regardless
```

Ten and thirty world units, so **`floatAfter` = 20 mask rows** and
**`splatDistance` = 60 mask rows**. The classic values standing in were 16
and 63. Two cell flags also exempt a landing: bit `0x200` for a lemming
whose `+0x4C` is 1 and bit `0x4000` for one whose `+0x4C` is 2.

**The landing probe, `FUN_0041A849`, read the rest of the way.** It belongs
to the 25.0 sibling, not to the faller above, and it holds no splat test at
all. It reads the mask cell under the lemming through `FUN_0040DE60` on the
world object `0x004C6A10` and then:

- cell is 0, or `cell & 0x80` is set → return 0, keep falling
  (`0x0041A99A and edx, 0x80`). **Bit `0x80` means not landable.**
- otherwise → `new` 0x18 bytes, construct through `0x004081B0`, hand it to
  `0x0042E696` (the state transition), return 1.

`0x004081B0` loads the vtable `0x004B0550`, so that is the **landing
state**. It sets no animation; its enter hook `0x004081FC` reads the word
at the lemming's `+0x3A` and passes it to `0x0044535B` on `0x004C6A08`,
which is a sound. Where the splat *for that state* is decided is still
unread, and it may simply not splat.

**Lemming fields, as far as they are now read.** `+0x08` x, `+0x10` y,
`+0x3A` a sound handle, `+0x40` horizontal facing, `+0x44` vertical
direction, `+0x49` has the floater skill, `+0x4B` umbrella open, `+0x4C`
sprite set / type (also the index into `0x004E335C`), `+0x50` a state tag
the climb halves its time on when it is 3.

Supporting reads, same disassembly:

- `FUN_0040df50` (`0x0040DF50`) is a **clamp**, not a wrap: below 0.0 it
  returns 0.0, above the height from `FUN_0040baa0` it returns
  height − 1, otherwise its argument unchanged. It does not scale the
  sum. The earlier note left this unread; it is read now.
- The walk stepper `FUN_0041bdd4` uses the *other* pair — `+0x40` for
  direction, `+0x08` for position (`0x0041BE7C da 48 40` `fimul`,
  `0x0041BE95 dd 59 08` `fstp`) — and passes through `FUN_0040dec0`, the
  horizontal wrap, where the vertical steppers pass through the clamp.
  That resolves the two fields: `+0x40` is the horizontal facing and
  `+0x44` the vertical direction, and the cylinder wraps in x only.
- `+0x44` is a unit direction. The only write that changes it is the
  `imul ..., −1` at `0x0042F5F0`, which preserves magnitude, and
  `0x0042DFE7 c7 41 44 01 00 00 00` sets it to 1 in the spawn path (the
  one that bumps the counter at `0x004CBA78`). Magnitude 1 is also what
  the climb measurement requires: 1 × 5.0 × 0.2 is the 1.000 world unit
  per tick that was observed.
- The fall's loop exit is `FUN_0041a849` (`0x0041A849`), and it probes the
  terrain through `FUN_0040de60` on the world object `0x004C6A10` — the
  same mask function on the same object as the climber's head test in
  `FUN_004139bf`. That is what ties this state to the lemming and to our
  surface-lattice mask rather than to some other actor.

**Still unread.** Named, because four fields in `Rules` are now numbers
off this section and these are the ways they could still be wrong.

- The transition *into* a fall state is unread. Six of the 24 animation
  sites say `fall`, so there are six fall-ish states, and which one a
  walk-off, a hatch drop and a topped-out climber each enter is still an
  assumption. The read above identifies `0x004B0A20` as *a lemming's* fall
  by what it does — splat, umbrella, floater — not by catching the
  transition. If a walk-off enters a different one, `fallSpeed` moves
  again.
- `maxStepUp` and `lemmingHeight` are untouched classic values. The walk
  stepper loads only 2.0, 0.0 and its 5.0, so the step-up is not a double
  in that function; it is probably in the mask probe or the collision
  path, and that was not followed. The climber's head test
  `FUN_004139BF` loads no `.rdata` double at all.
- The skill constants are untouched, but their addresses are in reach.
  `bash`, `dig` and `mine` **share one stepper**, `0x0041B077`, and are
  separated only by what their enter hooks write — dig's `0x004159BD`
  pushes the double 0.75 into `[this+0x128]` through `0x004683E9`.
  `build`'s site `0x0040FB83` is followed at `0x0040FB9E` by a divide by
  the 16.0 at `0x004B08F8`. None of that is read far enough to put a
  number in `Rules`.
- Our `stepFaller` sub-steps **one mask row** at a time and probes between
  rows; the original sub-steps **one world unit**, which is two of our
  rows. The speeds now agree but the probe granularity does not, so a
  one-row ledge can be caught by us and missed by the original. That is a
  real 1:1 divergence and it wants its own pass with a test.
- Where the 25.0 sibling's splat is decided, if it has one, is unread.
  `FUN_0041A849` has no distance test, and the landing state it enters,
  `0x004B0550`, only plays a sound.
- The climb's `if ([+0x50] == 3) remaining /= 2.0` at `0x004137D6` has no
  counterpart in either faller. Which state 3 is, is still not identified.
- 24 of the 46 vtables are still unnamed. That is now a statement about
  the game and not about the scan: those states change no animation when
  they are entered.

**Tension with the one live observation, stated rather than smoothed.**
The only descent ever sampled was 48.190 → 32.190, 16 world units, the
next sample 0.39 s later with nothing in between at 30 ms polling. At the
4 world units per tick now read that fall takes 4 ticks = 0.8 s and would
have shown three intermediate heights. It showed none, and the corrected
constant makes the gap *wider*, not narrower. So the reading stays what
§Phase 3 already warned: the snap was never a fall. It is not evidence
against 20.0, and 20.0 is not evidence about the snap. Both stay open
until a walk-off is sampled.

**Next.** Sample a walk-off the y = 48 ring and look for the predicted
**4.000** world units per 200 ms step, and for the umbrella at 10 units
and the splat past 30. That is the live confirmation these four fields do
not have, and it is the same run that tests which fall state a walk-off
enters. Then the skill constants, from the enter hooks named above. Do
not remeasure the climb.

**Re-running these reads.** `tools/harness/read_steppers.py` is the whole
static read, re-runnable: it maps the sections, scans `.text` for the 24
animation-set sites, finds the 46 state vtables and names them from those
sites, disassembles the walk, climb and both fall steppers with every
`.rdata` operand annotated with the double it holds, and prints every
speed and threshold against the 0.2 s tick. `--states` prints just the
state map; `--consts <addr>` lists the `.rdata` doubles one function
loads, which is how a state's own constants surface. It checks the exe's
SHA-256 first, because every address here belongs to that one build. It
needs `capstone` and nothing else — neither Ghidra nor the game running.
REA was needed only for `xrefs`. Note that the snapshot at `C:\tools\lr-rea-snapshot.json` was
written by `rea-agents` 6.4.0, `@latest` is now 6.5.0, and 6.5.0 rejects
that snapshot with `evidence_integrity_mismatch` / `profile_mismatch`.
Pin the version:

```powershell
npx -y rea-agents@6.4.0 xrefs "C:\progs\Lemmings Revolution\Lemmings Revolution.exe" 0x4137ca --provider ghidra --snapshot C:\tools\lr-rea-snapshot.json --format json
```

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
- **Done on the file side.** Cylinder + surface lattice → playable mask, skills trailer,
  entrance/exit + 417 other placed models (parse only), skin ≠ solidity,
  PolyList 5/7 = draw attrs, ring stride **derived not stored**.
- Two live findings stand (§1.6): the original keeps **no** occupancy buffer
  — scanned for across all 182 MB and absent — and it holds the .LVL surface
  lattice byte-for-byte, which confirms the file reading outright.
- **The live confirmation of the raster does not stand.** The 31-of-32
  result was produced by a lemming finder that cannot tell a lemming from a
  transformed surface vertex, so it may have sampled scenery (see §2.2). The
  re-run is done: `check_mask.py` on MM_CLIMB_002, six step-locked walkers,
  foreground at 144 Hz (§1.6). No visited column is empty top to bottom, but
  the foot cell is solid only 592/1012, which is below the 72.7% of columns
  solid on that row, and the offset sweep ties 108 ways. The earlier result
  stays withdrawn. `nearest_lattice.py` then showed the unroll is not the
  problem: the residual to the y = 32 ring is ~0 columns and 0.19 units, and
  the 420 misses are outer-only key+16 links the quad rule drops. Those
  links, and the outer-only vertical at the left turn (column 128, y 32
  to 48), are now stroked. The right turn is the half-step at column 168:
  key+8 holds the post at 172, and the quad that would cross it is blocked
  so a wall is stroked at 168 instead. A later live check against that mask
  is below.

### Phase 3 — Simulation (headless, deterministic, unit-tested)
- Terrain mask with horizontal wrap; fixed timestep; no rendering dependency.
- Lemming state machine: walker, climber, floater, bomber, blocker, builder, basher,
  miner, digger — plus the Revolution-specific mechanics visible in the assets
  (anti-grav shafts, trampolines, water, acid, axes, evil rats, cylinder rotation).
- Validated continuously against the differential oracle.
- **Rule constants, partly settled (2.2).** The step rate is **5 Hz**, measured
  three ways and not frame-locked; it had been guessed at 20, so everything timed
  in ticks ran four times too fast. A walker moves **one lattice column per
  step**, measured. `RELEASERATE.PC` and `LEVELMAP.DAT` are confirmed to hold
  literal seconds. `src/sim/World.h` now labels every field measured or
  provisional, `kTicksPerSecond` is 5, and the one test that had encoded the old
  20 Hz fuse arithmetic reads the numbers off `Rules` instead.
- **Terrain check, confirmed on the climb ledge.** After the outer links
  and both walls were stroked, `check_mask.py` on MM_CLIMB_002 (stock exe,
  shim at 144 Hz, ten step-locked walkers, 15 s) put every foot sample on
  a solid cell: 1,231/1,231, all at row 127 (y = 32.190). Columns spanned
  128.2–167.8. Reversals at 128, 129, 166 and 167, which is the step back
  from the walls at 128 and 168. No visited column is empty. The earlier
  592/1012 figure is the mask from before those strokes.
- **Climb speed, measured.** A climber on MM_CLIMB_002 rises one world
  unit per 200 ms step (34.190, 35.190, 36.190 …), which is two mask rows,
  so `Rules::climbSpeed` is 2. The series is world space. Height is the
  shared y of the four billboard vertices (`measure_motion.centroid`);
  the column is `atan2(-x, z)` over the 256-column ring. While walking,
  the column moved and y stayed 32.190. While climbing, the column froze
  at 128 and y stepped by 1.000. Screen Y is the projection and is not a
  height. A second wall continued the same step from 34.190 through at
  least 58.190. They top out, walk along y = 48.190, and the next sample
  (0.39 s later, no intermediates) was back at 32.190.
- **The sim climbs the retail mask.** `restart()` selects the first skill
  whose stock is greater than 0, which is the climber on this level; a
  sandbox with unlimited counts stays on the digger. `testClimberTopsOut`
  and `testRetailClimb` lock it. The suite was 70 checks, 0 failures,
  with `WEBOLUTION_GAME_DIR` set. From PowerShell the UCRT64 `bin`
  directory has to be on `PATH` or the test executables fail to load.
- **The state machine is mapped, and the fall is read off the binary.**
  Slot 2 of each state vtable is an on-enter hook that selects the `.SPR`
  animation by name through `0x0042FD25`. Scanning `.text` for that call
  finds **24 sites** — every state entry in the binary — and their strings
  are exactly `LEMMINGSLO.SPR`'s 15 strips plus `trampoline`, checked
  against our own parser. 22 of the 46 vtables are named from those sites;
  `build`, `float` and `umbopen` are set from inside steppers instead.
  §2.3 has the scan and the whole table, and `read_steppers.py --states`
  prints it.
- **`fallSpeed` is 8 mask rows per tick, `floatSpeed` 4, `floatAfter` 20,
  `splatDistance` 60.** All four read, none measured. The lemming's own
  fall is the state at vtable `0x004B0A20` — identified as the lemming's
  by the fact that it holds the splat test, the floater check and the
  umbrella. Its speed accessor `0x00416038` loads the 20.0 at
  `0x004B0A18` and halves it while the umbrella is open, so 4 world units
  per tick falling and 2 floating; its own `+0x18` counts world units
  fallen and is tested against 10.0 for the umbrella and 30.0 for the
  splat. Addresses and bytes in §2.3. What makes the units safe is that
  the same arithmetic reproduces the two *measured* fields exactly: the
  walk's 5.0 gives the measured 1 column per tick and the climb's 5.0 the
  measured 1 world unit per tick.
  - **An earlier pass had `fallSpeed` at 10, off the wrong state.** It
    read 25.0 from `0x004B0C60`, the climb's instruction-for-instruction
    sibling. That state is a faller by its animation but has no splat
    test, no umbrella and no floater check, so it is some other descent.
    Do not restore 10 without catching the transition.
  - It has no live confirmation. The one descent ever sampled, the snap
    48.190 → 32.190 in under 0.39 s, showed no intermediate heights at
    30 ms polling, which a 0.8 s fall would have — and the corrected
    constant makes that gap wider, not narrower. Read it as the snap not
    being a fall. Do not set `fallSpeed` from that snap either way.
    Sixteen world units survived is consistent with the 30-unit splat but
    is not a measurement of it, and the hoist at the top of the wall was
    not a clean `lemmingHeight` measurement.
  - The confirmation still owed is a walk-off the y = 48 ring, checked for
    **4.000** world units per step. That same run is what would show which
    of the six fall-ish states a walk-off enters — which §2.3 still lists
    as an assumption, since the transition is unread.
  - `LEMMINGSLO.SPR`'s `fall` rate is 0.467, the same value as `float`;
    `walk` is 1 and `climb` is 0.5. Those are the animation clock, not
    mask rows per tick. Do not copy 0.467 into `fallSpeed`.
- **Handoff.** Do not remeasure the climb. Do not invent the remaining
  `Rules`. `ticksPerSecond`, `walkSpeed` and `climbSpeed` are the measured
  fields; `fallSpeed`, `floatSpeed`, `floatAfter` and `splatDistance` are
  read but unconfirmed. Everything below them in `Rules` is still a
  classic value. Read each from the instruction that implements it, the
  way §2.3 did the fall, and keep a live series as the check. The skill
  constants are the next ones in reach: `bash`, `dig` and `mine` share
  the stepper `0x0041B077` and differ only in what their enter hooks
  write.
  - **Assigning another climber.** The original auto-selects the climber
    on this level; do not click the skill panel. The drawn crosshair is
    the doubles at the object behind `0x004CBFB0`, offsets +0x170 and
    +0x178. The hit-test reads `0x004C36A0` / `0x004C36A4`, which stay at
    the map click (206, 204) through play. Write the lemming's screen
    position there immediately before the shim click. A screen pair has
    been found 0x130 bytes before a quad, and that offset is not stable:
    other copies read (0, 0), 152/255, or garbage. Keep a pair only when
    it moves over a few tenths of a second, sits inside about x 80–620
    and y 160–450, and is not within 1.5 of 152 or 255. Release the shim
    afterwards. A 25-frame ESC can fail to leave pause; a 40-frame ESC
    after `focus()` did. RETURN only when the menu highlight is exactly
    0. ESC from a post-level screen can quit the process.
  - Walkers are not all between columns 135 and 160. The pen is about
    128–168 at y = 32.190; others sit near columns 5–31.
  - After the fall: splat distance, the floater, `maxStepUp`,
    `lemmingHeight`, then the other skills on their own levels. Map nodes
    besides (206, 204) are not known. Then gadgets. Read each from the
    instruction that implements it (§2.3), and keep a live series as the
    check. Do not copy a classic value in because the static read is
    unfinished.
  - The last process seen was pid 12512, `ui_state` 4, screen id 9, clock
    113. Do not assume it is still up. `ui_state`: 0 playing, 2 pause,
    3 frontend, 5 results. Frontend screen id `0x004C416C` is 3 menu,
    4 confirm, 5 map, and it is not stable once a level is running.

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
collision. What a lemming can stand on is the surface-lattice mask. The live
check in §1.6 did not show the original's lemmings walking on those cells.

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
2. **Exact physics constants** — medium, and partly retired. The step rate,
   the walking speed, the climb ledge and the climb rate are measured. The
   fall, the float, the umbrella distance and the splat distance are now
   read off the binary — 20.0 world units per second at `0x004B0A18`,
   stepped at `0x00416219`, so 8 mask rows per tick, halving under the
   umbrella, with thresholds of 10 and 30 world units (§2.3) — but none
   has a live series behind it, and which of the six fall-ish states a
   walk-off enters is still unread. A walk-off the y = 48 ring is the
   check. One earlier read of the fall took the wrong state and was off by
   a quarter, which is the shape of the risk that remains. The skill
   constants are untouched classic values.
3. **3D model geometry** — low/medium. Assumed to live in the `.LVL` PolyLists;
   unconfirmed. `TEXTURES\MODELS\...` implies separate model data somewhere.
4. **FMV codecs** — low. Cosmetic, and deferrable to the very end.

---

## 5. Sources

- [Lemmings Revolution file formats — Lemmings Forums (ccexplore)](https://www.lemmingsforums.net/index.php?topic=1439.0)
- [Lemmings Tools & Documentation — The Lemmings Archive](https://www.camanis.net/lemmings/tools.php)
- [Lemmings Revolution — Wikipedia](https://en.wikipedia.org/wiki/Lemmings_Revolution)
