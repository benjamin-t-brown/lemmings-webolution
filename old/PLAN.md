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

**Read "The collision grid is stored in the `.LVL`" below before anything else
in this section.** The terrain mask is a length-prefixed `u32` array in the
level file, 256 x 96, byte-identical to what the running engine collides
against. Several subsections above it were written while that was unknown and
are superseded; each says so where it stands. In particular the surface
lattice, which most of this section is about, is what the engine *draws*, not
what a lemming stands on.

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
  rebuilt from the lattice in SurfaceMesh). This is a *rendering* question
  now -- the collision mask is a separate chunk, read below.
* The collision grid's seven unread flag bits, its anchor in the chunk order,
  and where its row count is read.
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

#### The terrain is geometry, not a bitmap -- WITHDRAWN

**This conclusion is wrong, and the section below is kept for its reasoning
rather than its answer.** The game does store a grid: 256 x 96 `u32` cells,
length-prefixed, in the level file, read verbatim into the object the engine
collides against. It is simply nowhere near the two index arrays this
subsection was reasoning about, which is why factoring *those* as a grid kept
failing -- the inference "no rectangular factorisation here, therefore no
bitmap anywhere" was a step too far. See "The collision grid is stored in the
`.LVL`" below. What stands is the narrower reading: the two `u32` arrays are
per-face data for the surface, and the surface is geometry.

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

**That conclusion is withdrawn. The engine does keep a collision grid, and
the sweep above could not have found it.** The cells are 4-byte flag words,
so a page of them is neither `{0, 1}` nor a small-alphabet byte grid --
both filters above exclude it by construction. The sweep's findings stand as
written; what does not stand is the inference from them.

The grid is reached through the cell probe, which is a plain read of the
shipped bytes (`tools/harness/read_grid.py` re-runs all of this):

```
0040de6a  25 ff 000000    and  eax, 0xff            ; x wraps at 256
0040de78  3b 91 18040000  cmp  edx, [ecx + 0x418]   ; y clamped to the row count
0040de9f  c1 e2 08        shl  edx, 8               ; y * 256
0040dea5  03 c2           add  eax, edx             ; + x
0040deab  81 c1 1c040000  add  ecx, 0x41c           ; the container
0040deb1  e8 5a6effff     call 0x00404d10
```

and `0x00404D10` is a flat `u32` array accessor, `0x00404D20 8d 04 91`,
`lea eax, [ecx + edx*4]`, over the data pointer at the container's `+0x0C`.
`FUN_0043AA63(rows)` -- one caller, `0x0043A048`, inside the level load --
resizes it to `rows * 256` (`0x0043AA6D c1 e0 08`, `shl eax, 8`) and stores
the row count at `+0x418`, which is the same field `FUN_0040BAA0` returns
and the fall stepper clamps against. So the world object at `0x004C6A10`
holds a **256-wide, row-major grid of `u32` flag cells**: row count at
`0x004C6E28`, data pointer at `0x004C6E38`.

**Read live on MM_CLIMB_002, and it is dense.** `read_grid.py` against the
stock exe in a running level:

```
grid       256 x 96   (24,576 cells)
solid      15,563 = 63.33%     (non-zero and not 0x80 -- the engine's own
                                landing test at 0x0041A99A)
distinct cell words   0x00000001  9,339  38.00%   solid
                      0x00000000  8,973  36.51%   empty
                      0x00000003  6,224  25.33%   solid | indestructible
                      0x00000082     40   0.16%   indestructible | not-landable
rows with any solid cell   96 of 96
```

Two things fall out of that shape. The grid is **96 rows for a 96-unit tube,
one cell per world unit** -- so our 192-row mask supersamples at two rows per
unit, and that factor is *our* choice, not the engine's. And only four
distinct words appear, none of them the `0x200` / `0x2000` / `0x4000` zone
bits, so on this level the grid is terrain and nothing else.

**The flag bits, each read off the instruction that tests or sets it.**

| bit | meaning | read at |
| --- | --- | --- |
| `0x0001` | solid, destructible | tested `0x00424905` `and edx, 1`; set by the builder `0x004101F8` `or edx, 1` |
| `0x0002` | indestructible | tested first at `0x004248E6` `and edx, 2`; such a cell is never cleared |
| `0x0080` | not landable | the faller skips it, `0x0041A99A` `and edx, 0x80`; stamped by `FUN_0043119B` at `0x004311AE` |
| `0x0200` / `0x2000` / `0x4000` | object zone, by the object's type field `+0x128` = 0 / 1 / 2 | stamped at `0x00428D63`, `0x00428D84`, `0x00428DA5`, `0x0042AFA3`, `0x0042AFC4`, `0x0042B395`, `0x0042B3B6`, `0x0042B4AB`, `0x0042B4CC` |
| `0x0800` | the blocker's own bit | cleared at `0x0040E1D5` `and eax, 0xf7ff` when a blocker goes away |

A removal clears `0x1`, `0x2`, `0x80` and `0x800` together --
`0x0041B783 and edx, 0xf77c` -- in the stepper `bash`, `dig` and `mine`
share. So the grid is live and mutable, which is the other half of why it is
not just a parse artefact.

`FUN_004311D0(flags)` is the stamp: an axis-aligned box in (column, row)
space, bounds from the doubles at the object's `+0x48` / `+0x50` and `+0x60`
/ `+0x68`, wrapping by adding `0x100` to the end column when it crosses the
seam (`0x00431201 add edx, 0x100`), ORing `flags` into every cell.
`FUN_0043127E` is its mirror, `&= ~flags`. Those ten sites stamp only the
zone bits and `0x80`.

**Where the solid bit comes from at load is still unread.** That is the open
question this read leaves: 15,563 cells carry `0x1` the moment a level is
running, and none of the ten box stamps writes it -- the builder's `or 1` is
runtime. Candidates examined and ruled out: `FUN_0043A4CA` is a setter for
`+0x408` / `+0x40C`; `0x00439DBF` and `0x00439F71` are the world object's
constructor and destructor; `0x00410E30` / `0x00410E44` write 3 and 1 on an
0x88-byte stride, which is a record array and not the grid.

#### The live grid against our raster — our mask is the problem

**Still true, and the cause is now known:** the grid our raster falls short of
is read verbatim out of the `.LVL`, so the shortfall is not a tuning problem.
See "The collision grid is stored in the `.LVL`" below.

`read_grid.py --compare` diffs the live grid against `rasteriseSurface`'s
output for the same level, a live row against the two of our rows that cover
it, counting ours as solid if either is -- the generous reading, so the miss
below is a floor:

```
agree                11,054/24,576 = 44.98%
solid in both         4,114
live only (we miss)  11,449
ours only (we add)    2,073
recall on the engine's solid cells   26.4%
```

We find **a quarter** of the floor the engine has, and invent 2,073 cells it
does not. **This is not an alignment error.** Sweeping a vertical flip, row
offsets −6..+6 and all 32 column offsets leaves every combination between
41% and 47% agreement and 22% and 27% recall, with the identity alignment as
good as any -- which separately confirms the `atan2(-x, z)` unroll and the Y
orientation are right. There is no rotation or offset that rescues it.

So the question the corpus survey left open is answered: the 88 sparse levels
are **our bug, not the terrain**. The engine's MM_CLIMB_002 is 63.33% solid
where ours is 22.99%. The 1,231/1,231 foot-cell result in Phase 3 is not
contradicted -- it sampled one ledge, and a mask with a quarter of the floor
can still be right on the strip the walkers used -- but it is now clear how
little of the level that was.

The redirection for the remaining `.LVL` work: the engine's collision is this
flag grid, and our job is to reproduce the grid, not to stroke lattice edges.
The lattice is the engine's *render* input (it is held byte-for-byte, as
above) and the ten box stamps show the grid being written from **object
bounding boxes** rather than from lattice quads, which is a different source
of geometry than `SurfaceMesh` uses. The 417 placed `.a3d` models are already
parsed with their poses (1.6 Entrances, exits...); what is missing is each
model's own bounds. Whether the load-time solid fill works that way too is
the unread part named above, and it should be read before any more of
`rasteriseSurface` is tuned.

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

**Reframed by the grid finding below.** These numbers measure our raster, not
the levels: the engine's own grid is 35–63% solid on every level sampled, so
there are no sparse levels in the original and the stride families this
section leans on are our artefact. The survey keeps its value as a change
detector, and `test_corpus` still pins it.

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

#### The collision grid is stored in the `.LVL`, not derived — settled

The engine neither rasterises the surface lattice nor stamps the grid from
object bounds. It **reads the grid out of the level file, verbatim.**

`FUN_0043B640` is the loader for a length-prefixed `u32` array:

```
0043b654  e8 07 6f fc ff   call  0x00402560     ; N = stream.readU32()
0043b680  c1 e2 02         shl   edx, 2         ; N * 4 bytes
0043b68e  e8 1d ff ff ff   call  0x0043b5b0     ; stream.read(data, N*4)
```

It is called on the world object's `+0x41C` at `0x0043A0C4`, inside the level
loader `FUN_00439FD3`. `+0x41C` is the container `FUN_0040DE60` probes for
collision, so the bytes in the file are the bytes a lemming stands on, with no
transformation in between. `FUN_004311D0`'s box stamps and the builder's
`or 1` are runtime edits on top of a mask that arrived whole.

**Checked against the live process, not argued.** The 98,304 bytes at offset
240,011 of `COMPILED\MM_CLIMB_002.LVL` are byte-identical to the grid
`read_grid.py` reads out of the running game — 0 of 98,304 bytes differ — and
that sequence occurs exactly once in the file. The `u32` immediately before it
is 24,576, which is 256 x 96: the array carries its own cell count.

Confirmed on **five levels**, each entered independently through the record
swap of §2.2 and dumped from its own fresh process. In every case the live
grid is found byte-identical inside the level file:

| level | offset in file | engine solid | our raster |
| --- | --- | --- | --- |
| MM_CLIMB_002 | 240,011 | 63.33% | 22.99% |
| MM_BUILD | 242,656 | 55.66% | 1.91% |
| MM_FLOAT | 259,037 | 45.40% | 9.16% |
| MM_BOMB | 255,157 | 40.79% | 1.88% |
| MM_DIG_000 | 240,600 | 35.03% | 1.35% |

**Every retail level has one.** Scanning for a length-prefixed array whose
payload is entirely small flag words matches exactly once in each of the 102
levels, every one 256 x 96. Median 39.7% solid, from 5.5% (`VH13_KF_1`) to
77.2% (`H23_MG_1`). Exactly one match per level and never two is itself
evidence the fingerprint is specific.

`tools/harness/read_lvl_grid.py` is the whole read, re-runnable against a
`.LVL` with no game and no exe. `tools/harness/dump_grids.py` is the live
side.

#### What that overturns

* **The surface lattice is render input, not collision.** `grid0`/`grid1`,
  `SurfaceMesh` and `rasteriseSurface` describe what the engine draws. The
  ring stride `Y`, the quad rule, the half-step wall rule and the `lo < 31.0`
  cut have no bearing on what a lemming stands on, and the 49 levels where
  `detectRingStride` returns 0 stop mattering for terrain.
* **Ring stride predicts nothing.** `MM_BUILD`, where `detectRingStride`
  returns 0 and our mask is 1.91% solid, is the engine's second densest level
  at 55.66%. Treating 8224 as the explanation, or the stride-0 levels as a
  family, was reading our own artefact.
* **The engine has no sparse levels.** 35–63% solid on every level sampled,
  against our 1.35–23%. The corpus survey's "88 sparse levels" are one missing
  step, not 88 problems, and its 22-of-101 releasable figure measures our
  raster rather than the terrain. It stays useful as a change detector.
* **`VH13_KF_1` settles it on its own.** The `header[3]` window rasterised
  *nothing* for that level; the file carries a real if minimal 5.5% mask.
  The full index array does rasterise it. That does not put the mask back
  into the lattice.

#### The grid's flag bits, and what is still unread

Known from the instructions that test or set them (§2.3): `0x1` solid, `0x2`
indestructible, `0x80` not-landable, `0x200`/`0x2000`/`0x4000` object zones by
lemming type, `0x800` blocker.

Four of the seven have since been cited to a cell instruction; `0x4`, `0x8`,
and `0x20` have not. The close-out below has the addresses. They are not rare
— across the corpus `0x403` appears on 29,654 cells and `0x21` on 20,896.
Two specifics worth keeping: `0x82` appears **exactly 40 times in every level
sampled**, which makes it a fixed feature rather than terrain, and it is the
known pair `0x80|0x02` rather than a new bit; and `0x20` does not occur in
`MM_CLIMB_002` at all while accounting for 876 cells on `MM_DIG_000` and
3,024 on `MM_FLOAT`.

#### The sibling containers, and the grid's anchor

Both siblings load in the same pass of `FUN_00439FD3` and neither is the
collision grid. `+0x42C` (`0x0043A07C`) is another length-prefixed bulk-read
`u32` array through the same `FUN_0043B640`. `+0x43C` (`0x0043A0A0`) goes
through `FUN_0043B9C0`: a count, then a loop deserialising 12-byte records one
at a time (`0x0043BA1D imul ecx, ecx, 0xc`).

In the file the array immediately after the collision grid is a count of
12,288 — 256 x 48, half the grid's height — whose payload was first read as
floats. **That reading is withdrawn.** The count is a byte length, loaded
into `+0x664`. The close-out below has the instruction.

Still unread, and both wanted before a production reader:

* **The chunk's anchor.** The scan above is a fingerprint, not a parse. The
  framing is consistent — the cell count immediately before, a 12,288 count
  immediately after — but what precedes it varies between levels (an index
  array's `0xffffffff` tail on most, ordinary records on `VH13_KF_1`).
* **Where the row count 96 is read.** `FUN_00439FD3` passes `0x00402560`'s
  result to `setHeight` at `0x0043A048`, so it is a separate `u32` earlier in
  the stream than the grid's own cell count. Not located in the file.

#### The grid's anchor, found: the third array after the "Cylinder" chunk

`read_lvl_grid.py` located the grid by fingerprint, honest for a harness and
not good enough for a loader. The chunk order turns out to be fixed, and the
walk is short. From the literal length-prefixed `"Cylinder"` string:

```
u32 8, "Cylinder"
u32[5]            all zero, in all 102 levels
u32 height        96, in all 102 levels
f64               0.36, in all 102 levels
u32 N, u32[N]     N = 49858 in all 102 levels -- the tube, identical per level
u32 R, R x 12     the records the engine loads into +0x43C
                  (FUN_0043B9C0 at 0x0043A0A0, `imul ecx, ecx, 0xc` at 0x0043BA1D)
u32 C, u32[C]     the collision grid; C == 256 * height
```

That walk lands on the grid in **all 102 retail levels** — arithmetic, no
search — and `src/assets/CollisionGrid.h` implements it and fails loudly
rather than falling back to a scan.

Two things fall out. **The row count is located**: the `u32` in the preamble
equals `C / 256` in every level, and it is what `FUN_00439FD3` hands to
`setHeight` at `0x0043A048`, which this section listed as unread. One caveat
kept in the header — every retail level is 96 rows, so "this field is the
height" is consistent with the data rather than proven by it. And **the
12-byte record array is empty on most levels** (`R = 0`), which is why the
`u32` before the grid's count reads as zero almost everywhere; `E14_KF_1` has
15 of them.

The array before the records is 49,858 `u32`s in every level. The count is
constant; the payload is not — it is the per-level surface-index array, and
it is what `indexLevel` now reads into `grid0`. An earlier pass treated it as
the identical ring mesh and looked for `grid0`/`grid1` somewhere else.
`header[3]` is still not this array's length. The close-out below has the
placement.

#### The simulation now runs on the stored grid

`--level` builds its terrain from `CollisionGrid`, not from
`rasteriseSurface`. The lattice path is kept and still exercised by
`lrtool lvlmap` / `lvlmask` / `lvlstats`, and `main.cpp` records in a comment
that it is the engine's render input, so nobody re-tunes it for collision.

**Resolution is now the engine's: 256 x 96, one cell per world unit.** Our
raster's 192 rows were two per unit, a supersampling that was ours alone and
that the original cannot represent. Dropping it makes every constant in
`Rules` the world-unit number read straight off the binary, with no
conversion: `climbSpeed` 2 → **1**, `fallSpeed` 8 → **4**, `floatSpeed`
4 → **2**, `floatAfter` 20 → **10**, `splatDistance` 60 → **30**.

The provisional *distances* were classic-Lemmings pixel values carried at the
old resolution, so they were halved to keep the physical size they stood in
for: `maxStepUp` 6→3, `brickLength` 6→3, `basherReach` 4→2,
`explosionRadius` 9→5, `lemmingHeight` 8→4. Halving a guess leaves a guess,
and `World.h` says so.

**Correction, Phase 3 Round 4.** Two of those five are no longer guesses, and
neither landed on the halved value: `maxStepUp` is **2**, from the two
unrolled arms of the mask probe `0x0041BFFC`, and `lemmingHeight` is **3**,
from the double 3.0 at the lemming's `+0x358`. The halving was a reasonable
way to hold a physical size, and it was wrong in both cases — which is the
argument for reading the rest (`brickLength`, `basherReach`,
`explosionRadius`) rather than halving them again. See §1.6. **`diggerDepth` stays 1 because it cannot halve**,
which makes a digger sink twice as fast per tick as before — flagged in the
header rather than fudged, and a reason to read it off the original.

`worldToMaskPixel`'s `pixelsPerUnit` **default was removed**. The two grids
disagree — the stored mask is 1 cell per unit, the lattice raster 2 — and a
silent default there halves or doubles every coordinate, which looks like bad
level data rather than a bug.

`Terrain::fromCollisionGrid` maps cells by the engine's semantics: not solid
(zero, or `0x80` not-landable) is `Empty`, `0x2` indestructible is `Steel`,
everything else solid is `Earth`. **`Rock` has no counterpart** — the engine
draws one line, destructible or not, so a second one here would be ours. It
is left unused by this path rather than given invented meaning.

#### The entrance survey against the real mask

`lrtool lvlstats` reports both readings side by side and `test_corpus` pins
both. The difference is the whole result:

| | our lattice raster | the stored grid |
| --- | --- | --- |
| levels with a mask | 101 of 102 | **102 of 102** |
| median solid | 3.9% | **39.7%** |
| median entrance drop | 132 rows = 66 world units | **13 cells** |
| entrances with floor below | 132 of 143 | **144 of 144** |
| of those, survivable | 35 | **138** |
| levels releasable | 22 of 101 | **99 of 102** |

On the five levels with live dumps the drop falls from a fatal 62–72 world
units to 0–12 cells, and the solid percentages match the live grids exactly.

The stored-grid column was re-measured 2026-10-14 after
`Terrain::fromCollisionGrid` started flipping file rows onto the sim's y-down
axis; the earlier pin (median drop 9, 134 with floor, 116 survivable, 81
releasable) searched +y on an unflipped grid from a flipped entrance row and
so often found a different solid band. `test_corpus` pins the figures above.

**The three levels still not releasable are not a terrain defect count, and
this survey is no longer a statement about the sim's spawn.** The question
that prompted that caveat is settled: the constructor writes two stored
int16s, not the `Entrance*.a3d` translation (close-out below), and since
Phase 3 Round 3 the simulation releases from those words. This survey still
drops from the *translation*, because that is what it was built to measure
and `LevelStats` was deliberately left on it — so read these numbers as a
reading of the entrance translation against the real mask, not as where
lemmings now start.

`src/__test__/test_collisiongrid.cpp` pins the parser against the original
itself: all five live dumps reproduce **byte for byte** from the `.LVL`
alone, and all 102 levels yield a 256 x 96 grid. Unlike `test_corpus` these
are not a baseline to improve — a disagreement with a live dump is a bug in
the parser. It degrades to the corpus checks when the dumps are absent, so it
is useful on a machine that has never run the original.

#### Still unwired, after the grid landed

* `0x4`, `0x8`, and `0x20` still have no cell instruction. `0x40`, `0x100`,
  and `0x400` are cited and not given a terrain meaning. **Correction
  (round 9):** `0x8000` is anti-grav — see "Where to go next" below.
* `0x200`/`0x2000`/`0x4000` object zones are still named and ignored.
  `0x800` blocker is Round 8 (sim acts). `0x8000` anti-grav is Round 9.
* Sprite scale against the halved cell height. The camera and renderer read
  terrain dimensions dynamically and adapt, but a lemming sprite's size
  relative to a cell has not been checked since the rows halved.
* `header[3]` / `header[4]` are still unnamed. They are not the index array.

#### Where to go next

* **Phase 3 polish (deferred).** Residue rounds 29–31 closed the open
  Phase 3 mechanic list. These leftovers are still named and unread or
  stand-ins — not blocking Phase 5:
  - Bash refuse bit `0x4` (right-facing half) unread live
  - Faller landing in a blocker-stamped column (`0x80` air / round 8 note)
  - Hatch facing `+0x40` direction isolation (Round 3)
  - Teleport partner by stream id (sim uses file-order `i↔i+n/2`)
  - Lever message fanout (toggle only)
  - `"Cylinder"` chunk still located by a linear byte scan
* **Done: the grid chunk is parsed, anchored and wired in**, at the engine's
  own 256 x 96, with `Rules` in world units. See the three subsections above.
* **Done, as far as Phase 2 asked: the format leftovers in the close-out.**
  Three cell bits, the meaning of the four that were cited, `header[3]`, and
  the PolyList trailing `u32` are the leftovers that stayed unnamed.
* **Done: the sim releases from the hatch's stored words (Phase 3 Round 3).**
  `findHatchRecords` parses the `(count, x, y)` triple the hatch loader reads
  at `0x00425F1C` / `0x00425F2E`, found by the shape of that read run rather
  than by a remembered offset, and `World::spawn` drops from them.
  `Entrance::source` records which read filled each slot; the translation is
  the fallback for hand-built levels, of which retail has none.
  **102 of 102 levels carry a hatch record, 144 records in all**, and on every
  one the y word is the entrance world Y minus 2 — zero exceptions, so there
  is no level to report, but the minus 2 is still an **observation across the
  corpus and not a rule the code uses**: no instruction computes it, and
  `hatchSpawnCell` reads the stored word. Independent corroboration: the
  per-hatch count words sum to `LEVELMAP.DAT`'s total on 102 of 102.
  `docs/scratch/phase3-round3.md`; re-run with
  `measure_spawn.py --hatch-words`.
* **Done: the fuse, Ohno and the crater (Phase 3 Round 7), and both remaining
  provisional fields are gone.** `tools/harness/measure_bomb.py`;
  `docs/scratch/phase3-round7-live.md`.

  **The fuse is 5000 ms**, measured click→flapper six times: 4.9967, 4.9985,
  4.9981, 4.9976, 4.9998, 4.9980 s. `push 5` at `0x0045B1B3`, `fild`'d at
  `0x00413D74`, times the dword 1000 at `0x004657B2`. The countdown object's
  own double, read live off the lemming's `+0x374` list, fell
  **4.9940 → 0.1330 over 4.8616 s** — so it really is a clock, which closes
  the static note's "not traced". `bomberFuse = 5*5` ticks became
  **`bomberFuseMs = 5000`**.

  **Ohno fires at the end of the fuse, and our sim was a second early by
  construction.** The 5000 ms event is one function, `0x00414370`, that plays
  the sample *and* installs the flapper. The `OhNo` cue has moved.

  **The explosion is not at the end of the fuse either.** The fuse installs a
  "flapper" (`0x004B0A58`, animation `explode`) and the crater arrives a whole
  animation cycle later: measured 1.0211 / 1.0193 / 1.0226 / 1.0206 / 1.0191 /
  1.0201 s against strip 6, **17 frames**, divisor 1.0 = **1.020 s**. New
  `exploderCycleMs = 1020`; total click→crater 6.018–6.021 s. So a bomb is
  three stages, not two.
* **`explosionRadius` and `clearCircle` are both deleted: the blast footprint
  is a stored 7×7 bitmap.** It is at `0x004B1528`, **37 of 49 bytes set**, a
  rounded disc, anchored at `col0 = trunc(X − 3.5)` and
  `row0 = trunc(Y + 1.5 − 3.5)`, with the cell written by
  `mov dword ptr [edx], 0` at `0x0043F7F5`. Facilitator read the bitmap
  straight out of the exe and confirms both the count and the shape:

  ```
  ..###..
  .#####.
  #######
  #######
  #######
  .#####.
  ..###..
  ```

  **A radius-5 circle clears 81 cells where the original clears 37**, so the
  halved classic guess was wrong by more than a factor of two in area and
  wrong in shape. Five of six live runs matched the predicted change set
  **exactly** (anchors 109/67, 175/47, 162/75, 84/75, 116/67), 75 cells in
  all, every one `0x1 → 0x0`, with nothing outside the window ever touched.
  The sixth also took two neighbouring blockers' four-cell fields, which is
  consistent with the `0x800` skip but cannot be separated from the crater
  inside one 1.5 ms sample.
* **A claim in a scratch note was wrong, and the correction is a lesson about
  reading the image.** Round 7's static note said Ohno is gated by the byte at
  `0x004CBB40` being non-zero, `0xFF` in the image. It reads **`0x00`** and
  Ohno always plays. The reason is a file-offset error: `.data` has vsize
  `0x1185B8` but **rawsize only `0x7000`**, and `0x004CBB40` is at delta
  `0xCB40` — past the raw bytes, so there is no image byte at all and the
  loader zero-fills it. Facilitator verified the PE section table. With
  `test edx,edx / jne`, zero is the arm. **There is no quirk to preserve
  here** — and the general lesson is that a `.data` address beyond a section's
  rawsize has no value in the file, so reading one yields whatever follows in
  the file rather than what the process will see.
* **Three incidental measurements from round 7.** The lemming's Y is **one
  unit lower in the blast state than in the flapper** (4 of 4, `0x00416B5B`),
  which is why the crater straddles the feet row symmetrically. A primed
  walker kept walking and covered exactly **25.000 columns** in the 5 s fuse
  (144.770 → 119.770), independently consistent with `walkSpeed = 1` per
  200 ms tick. And a bombed blocker's field clears **3.7 and 4.2 ms after the
  flapper**, which pins round 8's observation to the fuse's end rather than to
  the blast.
* **Done: the builder's brick (Phase 3 Round 6).** The static read in
  `docs/scratch/phase3-round6.md` was confirmed live on every point it
  claimed, and three things beyond it. `tools/harness/measure_build.py`;
  `docs/scratch/phase3-round6-live.md`.

  A brick is **two cells**, at `(p + facing, row)` and `(p + 2·facing, row)`
  on the builder's **own** row — not the row below, the way the dig bite is —
  where `p = trunc(X at the click)` in **both** facings, because the enter
  hook's left-facing `+1` snap and the `-1.0` X bias cancel, exactly as in
  the bash bite. So **`brickLength` is 2, not the classic 3**. Measured on
  3 builders × 12 bricks = 72 cells.

  **The value written is plain `0x00000001`, 72 of 72 cells — so a brick is
  destructible**, and the `or edx, 0x403` paint at `0x004275C1` belongs to
  something else. That question was open in `CollisionGrid.h` and the grid
  diff settled it.

  The stamp fires at animation percentage **81** (36 of 36 stamps), the
  advance at **90**: world Y **+1.000** and X **+1·facing**, in that order, in
  the same callback as the brick-count decrement. The count starts at **12** —
  facilitator verified `0x0040F948` is `mov dword ptr [eax+0x3c], 0xc` — and
  the twelfth decrement leaves for the **shrug** state `0x004B0C78`, which
  runs 0.966 s and then walks.

  **"Interval in ticks" fails for the third time.** Build is animation strip
  2, **16 frames**, so `16 × 0.06` = **0.960 s = 4.8 ticks** — measured over
  33 gaps, mean 0.95982 s. `builderInterval` is **deleted**; `Rules` carries
  `builderCycleMs = 960` and `shrugCycleMs = 960`.
* **The `16.0` at `0x004B08F8` is an enter-time seek, and it was predicted
  before it was measured.** It advances the animation by one sixteenth of a
  cycle — 6.25% — so the **first** brick lands at 75%, not 81%, i.e. 0.718 s
  after the click. Measured 0.715 s. The constant is 16.0 (verified by
  reading the exe), and round 6's static note had already said it was applied
  once on enter and was not the gap between bricks.
* **The animation divisor's role is now settled, and it is not where round 5
  left it.** The shrug is strip 10, **8 frames, divisor 0.5**, giving
  `8 × 0.06 / 0.5` = 0.96 s against a measured 0.966 s. So a duration is
  `frames × 0.06 / divisor`, and **the divisor belongs in the duration even
  though it cancels in the percentage** — the first non-1.0 divisor seen in
  this project. **This does not explain dig's `0.75`:** dig's cycle measured
  1.2005 s and `20 × 0.06` is 1.2, which implies a divisor of 1.0, so the
  0.75 written to `controller+0x1D8` by dig's enter hook is still something
  else and still unread. Do not let the shrug's 0.5 be cited as having
  resolved it.
* **Two more findings from round 6.** A non-zero step-into cell **turns the
  builder and takes it out of the build state without spending the brick** —
  read, with the first of its two exits measured and predicted. And, usefully
  for every later round: **the arrow keys are the engine's own direction
  filter** — holding RIGHT sets `[screen+0x16B1]`, LEFT sets
  `[screen+0x16B2]`. That is pure input, and it is how a mirrored facing was
  reachable on a level with twenty lemmings in one pen.
* **Done: the blocker is terrain, not a list scan (Phase 3 Round 8).** The
  hypothesis in the round brief was right and is now measured: **a blocker
  never touches the lemming list.** `World::blockedBy` is **deleted**.
  `tools/harness/measure_blocker.py`; `docs/scratch/phase3-round8-blocker.md`.

  **The stamp.** Block state vtable `0x004B07B8`, enter hook `0x0040E20C`,
  exit hook `0x0040E0DF` — neither with an `e8` caller anywhere in `.text`,
  reached only through those vtable slots. The enter hook stamps **one column
  × four rows**: the blocker's own `trunc(X)`, rows `trunc(Y)..trunc(Y)+3`
  upward in the Y-up live grid. Facilitator verified the citations:
  `0x0040E312` is `80 cc 08` = `or ah, 8` (AH being bits 8..15, so `0x800`),
  and the loop bound at `0x004B10B8` is the dword **3**, inclusive from 0,
  which is the four rows. The column is `trunc(X)` in **both** facings
  because the `sete`/`add` at `0x0040E29A`/`0x0040E2A4` cancels `[+0x360]`'s
  −1.0 bias — the same bias round 4 found on the walk probe.

  Measured on 3 blockers / 12 cells, every cell predicted **before** the click
  and matched, `0 → 0x00000800` within **5 ms** of the press, with nothing
  else in the 256×96 grid ever touched.

  **The erase** is the exit hook: `0x0040E1D5` is `25 ff f7 00 00` =
  `and eax, 0xf7ff`, which clears exactly `0x800`, immediately followed by
  `mov dword ptr [ecx], eax` storing the cell back — verified. It runs via
  `0x0042E696`'s `call [edx+0xc]` on **any** departure, not just death.
  Measured once: a bombed blocker's four cells returned to `0` **2 ms after**
  its state became `explode`, a second before the crater.

  **The turn rule.** Three sites inside the walk probe `0x0041BFFC`
  (`0x0041C430`, `0x0041C4C9`, `0x0041C563`), each taken *before* that cell's
  own `and 1`, each falling through to `0x0042F66B` — the turn. Measured on
  **10 walkers, both facings, two levels**: a right-facing walker turns at
  world X = C and a left-facing one at X = C + 1, the two edges of the
  blocker's own cell. **The old `|dx| <= 3` heuristic turned a walker two
  cells early on both sides.**
* **`0x800` is the first cell bit shown to be runtime state rather than level
  authoring.** It appears in **0 of 2,506,752** cells across all 102 shipped
  grids — re-run with `measure_blocker.py --audit`, which the facilitator
  did. Every other named bit arrives in the `.LVL` bytes. Consequence for
  `test_collisiongrid`: a live dump taken while a blocker is alive **would**
  contain `0x800` and would not match the `.LVL`. That is a **bad dump, not a
  parser bug** — the five fixtures are untouched and safe.
* **One of round 4's five open differences is closed, by accident.** The walk
  probe's asymmetric X bias collapses to `cell + facing` in both directions
  for in-cell comparisons, so our `l.x + dir` was **already right**. The other
  four (step-down before falling, climber top-out, hoist hold, delayed float
  halving) all fell in round 10, below.

* **Done: all four remaining stepper differences settled (Phase 3 Round 10),
  and round 1's headline number is retracted.** Four `Rules` fields added —
  `climberTopOut` 2, `hoistCycleMs` 840, `hoistLift` 2, `floatOpenMs` 480 —
  no measured field moved, `lemmingHeight` is now the ceiling bias only, and
  `stepWalker` / `stepClimber` / `stepHoister` / `stepFaller` all changed.
  `tools/harness/measure_steppers.py`; `docs/scratch/phase3-round10-steppers.md`.

  **The walk's X is snapped to an exact integer before every probe, and that
  is what put the climber on the wall column.** Facilitator re-read it:
  `0x0041BECE` truncates X through the chop helper `0x0049C5AC` and
  `0x0041BEDC` stores the integer **back into `[+0x8]`**, then `0x0041BEE6`
  calls the probe — so X is on a cell edge whenever the probe runs.
  `0x0041BF61` picks the next edge as `trunc(X) + 1` facing right (gated on
  `[+0x40] == 1`) and `trunc(X)` facing left, dividing the gap by the 5.0 at
  `0x004B10B0`. Truncation gives a shared edge to the right-hand cell, so a
  right-facing walker stops **on** the wall column and a left-facing one at
  wall + 1. Live, 31 turns: right at X 64.000 against wall column 64, left at
  X 24.000 against wall column 23; over 346,293 samples no walker left
  `[24.000, 64.000]`. Five panel-assigned climbers held X 64.000 / 80.000 /
  96.000 / 112.000 / 120.000 — the wall column itself.

  **The step-down arm, read instruction by instruction.** `0x0041C6EF` tests
  the cell under the feet and jumps to `0x0041C838` (Y unchanged) when it is
  nonzero; `0x0041C6FF` subtracts `[+0x44]` and re-probes, `0x0041C732`
  compares the **whole dword** with zero and `0x0041C809` writes Y one unit
  down; `0x0041C741` repeats it and the second miss falls. Live, holes cut in
  the pen floor: 7 lemmings over a 1-cell hole all stepped `dy -1.000` and
  stayed `walk`, 8 over a 2-cell hole all went `walk → fall`, and a 3-cell
  hole traps them, because a 3-rise is a wall.

  **The hoist is a state, not a wait.** Vtable `0x004B0AF8`, animation `flip`
  = strip 8, 14 frames, so `14 × 0.06 = 840 ms`; held live 0.840, 0.840,
  0.833, 0.840, 0.839 s. Its net move is **X unchanged, Y + 2** — the step's
  `X += facing, Y += 3` and the exit hook `0x00417DD7`'s `X -= facing,
  Y -= 1` compose to that, and five placements were every one
  `dx 0.000 dy +2.000`. Round 5's lesson repeated: a "wait of N ticks" was an
  animation cycle again.

  **The float halving is a fixed animation length, 480 ms.** `umbopen` is
  strip 12, 8 frames; live the strip went 7 (`fall`) → 12 (`umbopen`) as the
  distance passed 10, then 12 → 11 (`float`) with `[+0x4b]` flipping 0.479 s
  and 9.564 units later. A 16-unit drop in the same run never halved at all.

  **Round 1's "walk matched 8 ticks" is withdrawn, and so is its tick-8
  divergence.** Both came from `compare_tick.py`'s billboard fit, and the quad
  it locks is **not a lemming**: at the same instant the actor list
  (`0x004CBB18`) held eight lemmings at X 28.264–63.033 all at Y exactly
  32.000, while the locked quad sat at column 168.042 / Y 32.190 and drifted
  at −3.8 units a second where a walker moves 5.0, with no lemming shadowing
  it at a constant offset over 1.06 s. The script now carries a SUPERSEDED
  banner. Against the actor list instead: walker **24 ticks** on open floor,
  **12 ticks through the right-facing wall turn** (live and ours both 63 → 64
  → 63), climber **16 ticks** left-facing and **13** right-facing — the climb
  compare owed since round 1.

  **Correction.** The facilitator's own brief for this round guessed that the
  column-159 disagreement was the missing step-down arm, and offered the
  stepped-onto-the-wall-column reading as the alternative. Neither was the
  cause: the sample was not a lemming. The brief's grid read was right and its
  attribution was wrong, which is the second time a facilitator brief has
  supplied a correct observation with a wrong reason attached.

  **`hoistCycleMs` is a read our sim cannot feel.** Facilitator check: flipping
  it to 800 ms (4 × 200) changes **nothing** — at a 200 ms tick every value
  from 800 through 999 gives the same four-tick hold, and only 1000 and up
  fails a test. So 840 rests on the strip read and the five live holds, not on
  any behaviour we can distinguish; the original holds 4.2 ticks and we cannot
  represent the 0.2. Recorded rather than tuned.

  **Left open, with citations.** A left-facing turn costs us a tick and the
  original nothing — walk matched 27 ticks then diverged at the turn (live 24,
  25, 26; ours 24, 24, 25), cause read at `0x0041C67E`, left alone because
  fixing it perturbs round 8's left-hand expectation. Also: animation strips
  1, 5 and 14 unnamed (strip 14 is a **candidate** for cartwheel after round
  9 — seen on every anti-grav flip, not proven), `[+0x4a]`, and
  `floatOpenCells()` rounding 9.6 to 10. The `0x8000` arm at `0x0041C151` is
  **done** in round 9 (anti-grav).
* **Done: bash, dig and mine measured live (Phase 3 Round 5), and the
  "interval in ticks" concept did not survive.** Three `Rules` fields were
  **removed**, four added, two corrected, and all three steppers rewritten
  against the measured footprints.
  `tools/harness/measure_dig.py`; `docs/scratch/phase3-round5-live.md`.

  **The removal is a whole-dword store of zero.** 38 of 38 observed cells went
  `0x21 → 0`, `0x5 → 0`, `0x1 → 0`. Facilitator re-read the instruction:
  `0x0040DE60` returns the cell *pointer* and `0x00415B84` is
  `mov dword ptr [edx], 0`. That settles the long-standing question about
  `FUN_0041B683`'s `and 0xf77c` **twice over** — by instruction, and by
  observation, since an AND would have left `0x20` standing on MM_DIG_000's
  cells. `FUN_0041B683` is still caller-less and still not the bite.

  **The clock is the animation, not a tick count.** `cycle = frames × 0.06 s`,
  where 0.06 is the double at `0x004B1940` (verified) and the frame count
  comes off the controller at `lemming+0x128`: **bash 24, dig 20, mine 25**,
  giving **1.440 / 1.200 / 1.500 s** — matching every measured cycle to 0.3%.
  Only dig is a whole number of 200 ms ticks. So `basherInterval`,
  `minerInterval` and `diggerInterval` are **gone**, replaced by
  `basherCycleMs` 1440, `diggerCycleMs` 1200, `minerCycleMs` 1500, with
  `World::bitesThisTick` sampling the real cycle so a basher acts every 7 or
  8 ticks and averages exactly 7.2. A tick-interval field could not have
  expressed that.

  **The footprints, per cycle, relative to the lemming's own cell:**

  | skill | cells | timing |
  | --- | --- | --- |
  | dig | columns `trunc(X)−1, 0, +1` on the row **below** the feet, then Y −1 | bite at animation percent 25, Y step at 30 |
  | bash | the leading column `trunc(X)+facing` only, **4 rows** from the feet row up | percents 12, 16, 60, 64 → rows +0, +1, +3, +2 |
  | mine | columns +1, +2 ahead × rows feet−1…+3 less one skipped cell, then X +1, Y −1 | percents 12, 12, 16, 16, 20 |

  So `basherReach` is **1**, not the classic 2; `basherHeight` 4 and
  `diggerWidth` 3 are new; `minerReach` is 2; `diggerDepth` 1 is now measured
  rather than "it could not halve". **Two real bugs in our sim fell out of
  this**: our digger was clearing **5** cells wide, and our basher was
  missing the feet row entirely.
* **`kCellUnreadBits` is now 0 — every bit in a retail grid has an
  instruction behind it.** `0x4`, `0x8`, `0x10` and `0x20` are the bites'
  **directional refusal** masks, and the reason earlier scans missed them is
  that each bite builds its mask **on the stack**. Facilitator verified dig's:
  `mov dword ptr [ebp-0x10], 0x10` at `0x00415A86`, then
  `cmp [ecx+0x44], -1` → `mov dword ptr [ebp-0x10], 0x20` at `0x00415A96`,
  tested by `and edx, [ebp-0x10]` at `0x00415B2A`. `[+0x44]` is the vertical
  direction, so the pair is upright-versus-inverted. The permissive halves of
  two bits were measured live; **the right-facing and inverted halves are
  read, not measured**, and the note says so — `0x5`'s left faces are off the
  camera arc on MM_BASH_002, and inverted needs anti-grav (round 9).
* **"A digger gives up after one tick" is explained, and it was not a bug.**
  It is bit `0x2` refusing at percent 25. The refusal path is visible in the
  same instruction stream: `and eax, 2` then `jne` to the give-up branch,
  which calls `0x0042F8B9` to revert the state. Every solid cell on
  MM_DIG_000 is `0x3` or `0x21`, and `0x3` contains `0x2` — a predicted-refuse
  digger changed **zero** cells grid-wide and was back to `walk` at 0.277 s.
  Panel indices **5 = basher** and **6 = miner** are now known too; 2, 8 and 9
  are still unidentified.
* **Done, and it unblocks rounds 5–8: a skill can be assigned through the
  panel, 38 clicks for 38 assignments.** Three rounds in a row had failed at
  this and fallen back to writing the lemming's flag bytes.
  `tools/harness/assign_skill.py` — `assign(h, inp, skill, choose=...)` is the
  call a later round makes. `docs/scratch/phase3-assign-skill.md`.

  **`0x004CBFB0` is the level screen object, not a crosshair** — `0x0045DA9E`
  reads it as `[0x004C6A08 + 0x55A8]` and passes it as `this` to the screen
  update. Everything needed was already stored on it:

  - `screen+0x16A0` / `+0x16A8` — the **cursor**, as doubles in render
    pixels, integrated from the mouse delta at `0x00453FD1` / `0x00453FF8`.
    The drawn crosshair is written *from* these by truncation, which is why
    writing the sprite never moved the aim.
  - `screen+0x16D8` — the **lemming under the cursor**, 0 for none. The hover
    pass already applies `0x0045A0E0`, the same "may this skill apply" test
    the click uses, so a non-zero value means a click will assign. That is
    aim feedback from the engine instead of a guess of ours.
  - `lemming+0x128` — each lemming's **own screen rectangle**. This is the
    stored screen pair earlier rounds searched for and concluded did not
    exist; the pick is a plain AABB overlap against it, so no projection or
    correlation of ours is needed.

  Facilitator re-read: the click path `0x00455657` bails on
  `cmp [eax+0x16d8], 0`, loads the stock array `0x004C91D8`, and passes the
  hovered lemming to `0x0045A0E0` — verified byte for byte. Stock array
  `+0x28` is `0x004C9200`, the selected index.
* **Why Round 1's session was wasted, now settled rather than suspected.**
  `0x004C36A0` has **exactly two references in all of `.text`** (byte scan of
  the section), both its own accessors `0x00455EA0` / `0x00455EC0`, whose
  readers are frontend and level-map and whose writer is gated on
  `byte [0x004C369D]`. Nothing in the level screen reads it, so writing it
  during play cannot work. `.claude/rules/match-the-original.md` called it
  "the click hit-test" and has been corrected to say frontend and level-map.
  The quirk it illustrated survives: the drawn crosshair is derived from the
  cursor doubles, so the two can still disagree.
* **Why `is_lemming` found nothing, measured.** Its screen box requires
  `220 <= cy <= 420`. The lemming rectangles actually sit at y 77–126 on
  `MM_DIG_000`, y 182–306 on `MM_BUILD` and y 80–133 on `MM_FLOAT` — all
  outside it. The box was a fit to one level's camera, and the camera orbits.
  It is left unchanged and now labelled a fit, so `assign_climber` stays
  intact as the regression case.
* **Two traps the skill rounds must respect.** The **dig state lives 271 ms**
  (`dig` at t = 0.000, gone by 0.271 at kHz polling), so `[lemming+0x340]`
  must be *sampled*, never slept-on and read. And the engine **pre-selects a
  level's only stocked skill**, so `0x004C9200` matching your intent proves
  nothing — the proof of assignment is the **stock count going down**, which
  is the engine's own receipt. Panel indices confirmed by assignment: 0
  climber, 1 parasol, 3 blocker, 4 builder, 7 digger. 2, 5, 6, 8 and 9 are
  **not** identified and were deliberately not filled in from the classic
  order.
* **Done: `maxStepUp` is 2 and `lemmingHeight` is 3 (Phase 3 Round 4).** Both
  moved off their classic-Lemmings values (3 and 4), each with a static read
  and a two-sided live series. `docs/scratch/phase3-round4-live.md`;
  `tools/harness/measure_stepup.py`.

  **`maxStepUp` = 2.** The walk stepper `0x0041BDD4` loads no step height —
  only 2.0, 0.0 and 5.0, so the handoff's standing instruction was right. The
  step-up is in the mask probe `0x0041BFFC`, called at `0x0041BEE6`, and that
  arm is **unrolled exactly twice**: `add ecx,[eax+0x44]` at `0x0041C48E`
  then `and ecx,1` at `0x0041C517` adopts a rise of 1, and
  `add eax,[edx+0x44]` at `0x0041C528` then `and eax,1` at `0x0041C5B1`
  adopts a rise of 2, after which `mov dl,[ecx+0x48]` at `0x0041C5C1` reads
  the climber byte and otherwise the walker turns. Facilitator re-read: there
  is **no third rise in that chain**. There *is* a third `add [+0x44]` in the
  function, at `0x0041C81B`, but it sits in a different arm — gated on
  `[eax+0x4a]` and feeding the setter `0x0042EE74` — so it is not a third
  step-up probe. (The round's note first claimed no third `add` existed
  anywhere before the `ret`; that was too strong, and the narrower claim is
  the one the bytes support.)

  **`lemmingHeight` = 3.** The head test `0x004139BF` loads no constant: the
  probe position comes from `0x0042F35F` as `Y + [+0x358]`, and `[+0x358]` is
  the double **3.0** — `mov dword [ecx+0x35c], 0x40080000` at `0x0042E0FC`
  with the low dword zeroed just above, re-written at `0x0042F616`. Verified
  byte for byte.

  Live, as a labelled probe: the corpus has **no reachable rise of exactly
  3** (the offline search named one on E07_JC_1 and the live walkers all
  reached the exit 50 columns short of it — recorded as a negative), so
  single-column pillars of 1/2/3/4 cells were written into the live grid in
  front of the original's own walkers and restored afterwards. On **VH13_MT**
  rises of 1 and 2 were adopted (231 of them a `dy +2.000` in a single
  sample) and **114 walkers turned at the pillar of 3**, none reaching the 4;
  **E07_JC_1** repeated it. For the head: 12 of 12 climbers hit a ceiling at
  feet Y = 55.000 under a cell at row 58, i.e. a bias of 3.
* **New, and it affects our walk probe: the facing carries an X bias.**
  `0x0042F66B` writes the double **−1.0** into `[+0x350]`/`[+0x360]` when
  facing goes to −1 (`0x0042F697`, `0x0042F6AE`) and **0.0** at +1
  (`0x0042F6BD`, `0x0042F6D4`) — verified byte for byte, the high dwords
  being `0xbff00000` and `0`. So the original's walk probe reads column
  `trunc(X)` going right and `trunc(X) - 1` going left. Confirmed live on
  both levels (a left-walker whose X truncated to 168 adopted column 167's
  row). Our stepper has no such bias; this is an unmodelled difference, not
  yet a `Rules` field.
* **Three more cited differences from Round 4, none of them changed.** Each
  is a stepper rewrite rather than a constant, which is why the round left
  them alone. `World::stepClimber` uses `lemmingHeight` for both the ceiling
  and the top-out, but the original's top-out is **one cell lower**
  (`sub eax,[edx+0x44]` at `0x00413B22`; 12 of 12 climbers topped out at
  Y = 61 over a solid row 62). The original's hoist holds Y and X for about
  0.834 s — roughly 4 ticks against our 2 — then places the feet at +2. And
  the original's walk **steps down one cell before falling**
  (`0x0041C6FF`, `0x0041C741`, `0x0041C77D` — the `sub [+0x44]` arms are
  visible in the same probe); our `stepWalker` has no step-down arm, which is
  the likeliest cause of the remaining open-floor edge disagreements.
* **Round 8's blocker turn is in this same probe**, not a separate stepper:
  bit `0x800` is tested at `0x0041C430`, `0x0041C4C9`/`0x0041C4D6` and
  `0x0041C563`/`0x0041C570`. Start there rather than looking for a blocker
  state.
* **Done: `floatSpeed`, `floatAfter` and `splatDistance` hold at 2, 10 and 30
  (Phase 3, float/splat round).** No `Rules` value moved. Live series on
  `E10_KF_1` (step halves to −2.010 ± 0.03 for fourteen consecutive ticks on
  the tick `[lemming+0x4b]` flips; umbrella latch set where the fall
  accumulator reaches 10.0) and a threshold probe that held that accumulator
  at 29 (lemmings survived a 75-unit drop) and at 31 (died at the untouched
  rate). `docs/scratch/phase3-round3b-float-splat.md`;
  `tools/harness/measure_float_splat.py`.

  Facilitator re-check: the **static side is the stronger evidence** and was
  re-read independently. `0x00416038` branches on the lemming's `[+0x4b]`
  and, when set, computes `fld [0x004B0A18]` (20.0) `fdiv [0x004B03F8]`
  (2.0) — so **`floatSpeed` is not an independent constant at all, it is
  `fallSpeed / 2`**, a division in the accessor. `0x004B03E8` is 10.0 and
  `0x004B10A8` is 30.0, confirmed by reading the exe (SHA-256 matches
  stock). The live series corroborates; the division fixes the value.
* **A floater never splats, and that is proven statically.** At `0x00416607`
  the `fallen <= 30.0` test jumps to `0x004166A8`; at `0x00416615` the
  has-floater byte `[eax+0x49]` is loaded and `0x0041661A` jumps to **the
  same** `0x004166A8` regardless of distance. Two paths, one safe landing.
  The round's live run agrees (11 of 11, then 17 of 17 of the lemmings that
  died were controls; none of the floaters).
* **Open difference, not yet modelled: the float halving is delayed.** Two
  different bytes do two different jobs. `[+0x49]` is set when the floater is
  assigned and is what exempts the lemming from the splat. `[+0x4b]` is set
  when the `umbopen` animation *finishes* — about 9.6 world units, 0.48 s,
  later — and only that one halves the speed, because it is the byte
  `0x00416038` branches on. So the original keeps falling at 4 for those
  ticks. `World::stepFaller` halves immediately on `canFloat &&
  fallDistance >= floatAfter`, so our floater is slower than the original's
  over the first half-second of umbrella. `floatAfter = 10` is still right —
  it is where the umbrella *opens*. No field encodes the delay and none was
  invented; a later round should read the animation length and decide whether
  the sim needs one.
* **Lead for the gadget round, found while checking the splat branch.** The
  landing path also tests the mask's object-zone bits against a lemming
  dword: `and edx, 0x200` with `cmp [eax+0x4c], 1` at `0x0041662B`, and
  `and ecx, 0x4000` with `cmp [edx+0x4c], 2` at `0x00416641`, each jumping to
  the same safe-landing target. Observation, with the addresses. Hypothesis,
  untested: those are the zone bits `CollisionGrid.h` already names and
  ignores, and `+0x4C` selects which zone exempts which lemming type. This is
  the first place the parsed zone bits have been seen doing anything.
* **Correction to a citation, not to a value.** `0x004B0C60` is a **vtable**,
  not a double — reading eight bytes there gives 1.96e-307. The discarded
  25.0 is at **`0x004B0C40`**, where it reads back as 25.0 and where
  `0x0041A758` multiplies by it (`fmul qword ptr [0x004b0c40]`, already in
  §2.3). So the number was real and the address it was filed under was not.
  The state that vtable belongs to is a different descent, which is the part
  that mattered and still stands. Cite `0x004B0C40` for the constant and
  `0x004B0C60` for the vtable.
* **The faller vtable `0x004B0A20` is now confirmed live**, closing the open
  negative from the fall round: heap objects with a `+0x18` that climbs one
  per world unit of descent. The fall state keeps the distance fallen at its
  own `+0x18`; the lemming holds no pointer to it, which is why finding it
  needs a heap scan.
* **A world Y becomes a file row by truncation, not by `floor(worldH - y)`.**
  The engine's float→int helper `0x0049C5AC` sets the x87 rounding field to
  chop (`or ah, 0xc`) before `fistp`, so the word 42 *is* file row 42 and the
  terrain row is `height - 1 - 42`. The two mappings agree on a fractional y
  and differ by one cell on an integer one, and the spawn words are always
  integers. The live birth sample decides it — `+0x10` is the word itself,
  0.0013 off 42 on MM_CLIMB_002, so the release is at world Y 42 and not 41.
  **The landing does not decide it**: from either start row the fall rests on
  the same floor, so it is equally consistent with both. `worldToMaskPixel`
  was deliberately left alone (it feeds `LevelStats`); whoever next owns the
  translation path should decide whether its half-open interval suits its own
  callers.
* **Check a lemming's sprite scale against the halved cell height.**
* **The `SurfaceMesh` fits stay labeled as fits.** They affect rendering
  only. Replacing them wants an engine face rule, not another constant.
* **The fall step is live-confirmed (Phase 3).** Walk-off from y = 48.190
  stepped −4.000 three times (`docs/scratch/phase3-round2.md`). Enter
  vtable, floater, and splat still owe live reads. Gadgets stay unmeasured.
* **Hazard / gadget behaviour, after the mask.** The harness can reach a
  level and read world positions. Placed-model poses are already parsed
  (`LevelObjects::placed`); anti-grav, teleports, deathrollers, and the
  rest are not simulated.
* **PolyList kinds 4 and 6** do not use the kind 5/7 stride and are unread.
* **Phase 3 trail (2026-10-14).** Dispatch and status live in
  `docs/PHASE3_HANDOFF.md`. Scratch notes `docs/scratch/phase3-round*.md`.
  Terrain Y: file grid is Y-up; `Terrain::fromCollisionGrid` flips onto the
  sim's Y-down axis (same map as `worldToMaskPixel`). Per-tick oracle:
  `tools/harness/compare_tick.py` — open-floor walk matched 8 ticks
  (168→160). Wall-mass cols 128–158 at Y 32 are solid in the file; live
  walkers still appear there and must not be used for compare. Fall:
  `tools/harness/measure_dropoff.py`. Static skill/fuse/step-up notes in
  rounds 4–7 are not promoted into `Rules` yet. Round 3 spawn and the
  float/splat round, Round 4's two geometry constants and the skill-assignment
  harness are done (see above). Next Phase 3 work: the skill live series
  (round 5), the blocker (round 8), the builder (round 6) and the fuse
  (round 7) are **done**, and round 10 closed the stepper differences. Round 9
  closed **one** gadget (anti-grav); trampoline / water / acid / teleport /
  SpeedPad / Lever are done (13–17, 21); Balloon/Fire/Witch/ForceF/Axe/
  Bubble/Scrape/Rumble/Changeop/Die are non-gadget cues (18–20, 22–28).
  Round-9 cue/gadget repeats are closed. Still provisional in `Rules`:
  **nothing.** `explosionRadius` and `bomberFuse` were the last two, and
  round 7 replaced both — the radius by a stored stencil, the fuse by a
  millisecond clock. Every `Rules` field is now a read or a measurement,
  which is the first time that has been true.
* **Done: anti-grav is cell bit `0x8000` (Phase 3 Round 9).** Corpus: 18/18
  levels with `anti_grav.a3d` carry the bit and no other level does
  (`measure_antigrav.py --audit`). The walk probe tests it on the ground under
  the feet (`0x0041C154`), requires a matching anti-grav object from the list
  at `0x004C8958` (painting the bit alone onto MM_CLIMB_002 flipped nothing),
  attaches cartwheel `0x004B0CA8` (type `0x2b`), whose enter calls
  `0x0042F5E3` and negates `[lemming+0x44]`. Live on `m06_kf_1`: 27 flips,
  every one at pad column 68 with `+1 → -1` and world Y increasing after.
  Side finding: hatch type word 1 → live `[+0x4c] = 1`. Sim:
  `Terrain::antiGrav`, `Lemming::verticalDir`. Scratch:
  `docs/scratch/phase3-round9-antigrav.md`.

  **Facilitator check, 2026-10-16.** The 18/18 corpus audit was re-run
  independently (not through `measure_antigrav.py`): 102 levels scanned, 18
  carry `anti_grav.a3d`, 18 carry a `0x8000` cell, and the two sets are equal
  with no mismatch in either direction. The four static claims were re-read
  instruction by instruction and all hold: `0x0041C104` fetches the ground
  cell and `0x0041C154` masks it with `0x8000`; `0x0041C162` pushes `0x2b`
  through the gate at `0x0042ECC9`; the loop at `0x0041C192` walks the
  container at `0x004C8958` and `0x0041C1E3` calls the overlap predicate
  `0x00430BA1` per object — so the object requirement is **read**, not merely
  inferred from the clean negative; and `0x0042F5E3` is
  `imul [+0x44], -1` followed by `[+0x5c] = 1`. Disabling `Terrain::antiGrav`
  alone fails two checks behaviourally. **Gap (closed in round 11):**
  `testAntiGravFromCollisionGrid` now pins the `word & kCellFlag0x8000`
  extraction in `fromCollisionGrid` on a hand-built grid word.

* **Done: the exit save test is a 2×3 object AABB (Phase 3 Round 11).** Not
  the invented `8×8` / `12×14` rectangles. Overlap predicate `0x00430BA1`
  (inclusive on every axis) tests the lemming's raw `(X,Y,Z)` at `+8`
  against the exit object's box at `+0x48..+0x70`; exit step `0x0042688D`
  (vtable `0x004B0F90`) walks list `0x004C88D0` and attaches save state
  `0x004B0B28` / type `0x2a`, which increments `0x004CBAA4` when its timer
  reaches `>= 1.0`. Z is the fixed slab `[0, 1]` from setter `0x00430FE1`.
  Live dumps: MM_CLIMB_002 `X[220..222] Y[31..34]`, MM_FLOAT
  `X[176..178] Y[6..9]` — both 2×3. Hold-probe edges hit the exact max
  doubles and miss 0.5 outside. Relative to the pose: `[col±1] ×
  [ty−9 .. ty−6]` with column centre **`round(ang×width)`** (round 12;
  this bullet originally said `floor`, which disagreed with MM_FLOAT's
  own dump). `_curExit` is a UI string only, not a second box. Sim: `Exit` defaults 2×3, `atExit` closed (`<=`),
  `exitTriggerFromPose`, all five `main.cpp` sites. Scratch:
  `docs/scratch/phase3-round11-exit.md`. Harness:
  `tools/harness/measure_exit.py`. Suite 16/16; `testExit` fails if
  `atExit` is half-open alone.

  **Facilitator check, 2026-10-16.** `0x00430BA1` was re-read instruction by
  instruction and the inclusivity holds exactly: mins at `+0x48/+0x50/+0x58`
  use `fcomp` / `test ah, 1` / `jne` (fail when the point is below), maxes at
  `+0x60/+0x68/+0x70` use `test ah, 0x41` / `je` (pass on below **or** equal),
  so every edge is closed. Making `atExit` half-open fails two checks
  behaviourally. Two things to narrow, though:

  **The offsets are measured for `Exit.a3d` only, not `Exit*.a3d`.** An
  offline census of the 102 retail levels finds **four** exit models —
  `Exit.a3d` in 84 levels (78 exclusively), `Exit_water.a3d` in 14,
  `Exit_any.a3d` in 9, `Exit_acid.a3d` in 1 — and both dumped levels
  (MM_CLIMB_002, MM_FLOAT) use `Exit.a3d`. MM_DIG_000, the third level the
  round failed to enter, is also `Exit.a3d`, so it would not have added
  coverage. If the AABB comes from the model's own geometry, a different
  model is a different box: `[col±1] × [ty−9..ty−6]` is **measured for 84
  levels and assumed for 18**. The useful next sample is a water or an "any"
  exit, not a third `Exit.a3d`.

  **The extent itself is untested.** `testExit` pins the closed interval, but
  nothing exercises `exitTriggerFromPose`: widening it back to the old
  12-column guess (`e.x = col - 6; e.width = 12`) leaves all 16 tests green.
  So the round's headline number is in the code and in this note, but not in
  the suite — the same gap round 9 left for the `0x8000` extraction, which
  round 11 did correctly close (`testAntiGravFromCollisionGrid`, verified to
  fail 2 checks in `test_terrain` when the extraction line is disabled).
  **Closed by round 12** (extent pin + all four models); see the next bullet.

* **Done: all four exit models share the round-11 2×3 AABB (Phase 3 Round 12).**
  Offline census of 102 levels: `Exit.a3d` 84, `Exit_water.a3d` 14,
  `Exit_any.a3d` 9, `Exit_acid.a3d` 1. Live dumps on alone levels
  `e14_kf_1` / `MM_W_012` (water), `E16_KF_1` (any), `E23_JC_1` (acid): one
  object vtable `0x004B0F90`, size 2×3, Y `[ty−9 .. ty−6]`, Z `[0,1]`. Not
  per-model. Column centre is **`round(unroll×width)`**, not `floor` —
  round 11 wrote `floor`; 46 of 111 placements disagree, and MM_FLOAT's own
  round-11 dump `X[176..178]` already centred on round (176.999→177).
  `exitTriggerFromPose` uses `std::lround`. `testExitTriggerFromPose` in
  `test_world` fails if the extent is widened alone (`col−6`/`width=12` →
  2 FAILs) or if column uses `floor` alone (e14 → 1 FAIL). Scratch:
  `docs/scratch/phase3-round12-exit-models.md`. Suite 16/16. Left open:
  natural walk-in; acid save-state attach; `.a3d`→AABB writer chain.

  **Facilitator check, 2026-10-16.** Census re-run: 84/14/9/1 and the alone
  lists match. Pose parse (Object 09 doubles [9..11]): 111 placements,
  **46** floor≠round; e14 col 10.989, E16 210.995, MM_FLOAT 176.999 —
  matching the live centres witnessed during the round (water X[10..12],
  any X[210..212], acid X[82..84]). Vtable install `mov [edx], 0x4b0f90` at
  `0x00426783` re-read. Mutate-alone in `test_world`: width 12 fails both
  box checks; `lround`→`floor` fails e14 alone (`got 9,76,2x3`). Restored
  (touch source after bak copy — ninja otherwise kept the mutated .obj) and
  suite green. **Narrowed:** the agent's water save-state HIT is not
  independently re-run here; acid attach was left open by the agent and
  stays open. The same-box and round-vs-floor claims are what this check
  promotes.

* **Done: trampoline / bounce is cell bit `0x1000` (Phase 3 Round 13).** Fall
  land at `0x00416560` (`and edx, 0x1000`) attaches bounce vtable
  `0x004B0810` / type `0x21` **before** the splat arm. Bounce re-hit of the
  same bit at `0x0040f309` when `t` exceeds the live threshold at
  `0x004c55a8` (BSS double **1.4**, not in `.data` rawsize). Bounce step
  uses `-25.0` at `0x004B0800` and `35.0` at `0x004B0808` (closed form
  `-12.5 t² + 35 t`, peak 24.5 at t=1.4). The bit is **runtime-only**:
  **0 of 102** `.LVL` grids carry it; trampoline-state enter `0x0041B838`
  (vtable `0x004B0D08`, type `0xe`) paints `|0x1083` then clears solid →
  `0x1082`; exit `and 0xf77c` at `0x0041B783`. `pressure_pad.a3d` (23
  levels) is a **clean negative** on H01_KF_1 — not the pad source.
  Sim: `Terrain::trampoline`, `State::Bounce`. Scratch:
  `docs/scratch/phase3-round13-trampoline.md`. Harness:
  `measure_trampoline.py`. `testTrampolineBounce` in `test_world` fails if
  the fall-land branch is removed alone (3 FAILs). Suite 16/16.

  **Facilitator check, 2026-10-16.** Offline audit re-run: 0/102 cells with
  `0x1000`, 23 pressure_pad levels. `0x00416560` / `0x0040f309` /
  `0x0041B783` re-read byte for byte; `-25.0` / `35.0` confirmed at
  `0x004B0800` / `0x004B0808`. Mutate-alone: deleting the
  `trampoline()` branch fails three behavioural checks; restored with
  touch. **Narrowed:** `bounceRiseCells = 24` and `bounceSpeed = 7` are
  integer stand-ins for the parabola — flipping either alone leaves all
  279 checks green (`testTrampolineBounce` only asserts that a rise
  happens). Same class as `hoistCycleMs`: the read and the live attach
  are the evidence, not those two fields. Detection and the runtime bit
  are what this check promotes.

* **Done: water hazard is cell bit `0x40` (Phase 3 Round 14).** Not
  `Exit_water` (save exit, round 12). Every spawn attaches side-effect type
  `0x2f` (vtable `0x004B09C0`); its step probes ground
  `(trunc(X), trunc(Y)−[+0x44])` and `and eax, 0x40` at `0x00415386`, then
  attaches drown vtable `0x004B0570` / type `0x22` (ctor `0x00408700`, enter
  sets `"drown"` at `0x004BF130`). Drown step `0x00408766` accumulates dt at
  `state+8` and compares against **2.0** at `0x004B0568`; the lemming then
  leaves the actor list. Live paint on MM_CLIMB_002: five fall→drown, each
  ~2.0 s. Authored in **3/102** grids (`H18_MG_1`, `M12_MT_1`, `M19_MT_1`);
  **0** of 20 `Entrance_water` levels — that census correlates with zone
  `0x200` (18/20), not `0x40`. MM_W_012 watch: 0 live `0x40`, 0 drown.
  Landing-path splat arms `0x200`/`0x4000` are **not** this mechanic. Sibling
  type `0x2e` when the object loop matches — ~~left open~~ **closed unused
  in retail by round 15**. Sim: `Terrain::waterHazard`, `State::Drown`,
  `Rules::drownMs=2000`. Scratch: `docs/scratch/phase3-round14-water.md`.
  Harness: `measure_water.py`. `testWaterHazardDrown` in `test_world`;
  extraction in `test_terrain`. Suite 16/16.

  **Facilitator check, 2026-10-16.** Offline audit re-run: 3/102 with
  `0x40`, Entrance_water ∩ `0x40` = 0, Entrance_water ∩ `0x200` = 18/20.
  `0x00415386` (`and eax, 0x40`) and drown compare vs `0x004B0568` (2.0)
  re-read. Mutate-alone: `drownMs 2000→200` fails the 10-tick duration
  check; stripping both `waterHazard()` attach sites fails four behavioural
  checks. Restored with touch; suite green. Promotes detection + duration;
  natural walk-onto authored pads (H18/M19) was inconclusive this round and
  stays observation-only.

* **Done: acid-as-hazard is the same `0x40` drown path (Phase 3 Round 15).**
  No separate acid state or duration. Paint `0x40` on H02_JC_1 (`^acid_glow`)
  and under Entrance_acid walkers on H23_mg_1 (`[+0x4c]=1`) → drown type
  `0x22`, **0** type-`0x2e`. Offline: **0** of 9 `^acid*` / 6
  `Entrance_acid` levels author `0x40` or `0x100`. Type `0x2e` Fluid-match
  sibling (`push 0x2e` at `0x00415425`, ctor `0x00406C20`, vtable
  `0x004B04D0`) never attached live — list `0x004C89D8` has only two
  absolute refs (both probe reads). Cell `0x100` sets `lem[+0x5f]=1` at
  `0x00415627`, not death (MM_CLIMB paint). `Entrance_acid` /
  `LemmingsAcid` are the `[+0x4c]=1` sheet; `^acid_glow` is Fluid subtype
  2 visual (`0x0042A8EA`). Not `Exit_acid` (save exit). Sim unchanged
  beyond comments. Scratch: `docs/scratch/phase3-round15-acid.md`.
  Harness: `measure_acid.py`. `testAcidHazardSharesDrown` in `test_world`.
  Suite 16/16.

  **Facilitator check, 2026-10-16.** Census re-run matches (6 EA, 9
  acid_glow, 3×`0x40`, 3×`0x100`, empty intersections). Fork at
  `0x00415419`/`0x00415425` (`push 0x2e`) and `mov [edx+0x5f],1` at
  `0x00415627` re-read; Fluid-list imm refs only `0x4153d6` /
  `0x4153f8`. Mutate-alone `drownMs 2000→200` fails **both** water and
  acid duration checks (`delta=1`); restored with touch. **Narrowed:** the
  agent's "Fluid list empty" wording overstates — live words were often
  `[0,0,1,0,0,0,1,0]`; what holds is **zero type-`0x2e` attaches** across
  the runs. Unit `testAcidHazardSharesDrown` re-pins shared `drownMs`, not
  acid theming — acid-specific evidence is the live paint series.

* **Done: teleport is object-only AABB (Phase 3 Round 16).** `teleport.a3d`
  pads (vtable `0x004B1250`) use overlap `0x00430BA1` at `0x00435467` /
  `0x004354CB`; partner at `+0x260` from stream id `+0x98` via lookup
  `0x00444BA1` (write at `0x00434BF3`); attach type `0x28` then `0x2c`
  (countdown push **4** at `0x0041A317`); relocate `0x0043503C` with
  Y = partner[+0x1b8] + **2.0** (literal `0x004B1238`). Live AABB 12×8
  (col±6 × [ty..ty+8]); facing preserved. `teleport_fx.a3d` FX-only; **0**
  exclusive cell bits (18/18). Sim: `State::Teleporting`,
  `teleportDelayTicks=4`, pair stand-in `i↔i+n/2` (id word not in .LVL
  parse yet). Scratch: `docs/scratch/phase3-round16-teleport.md`. Harness:
  `measure_teleport.py`. `testTeleportPad` in `test_world`. Suite 16/16.

  **Facilitator check, 2026-10-16.** Offline `--audit` re-run: 18/18 same
  set, exclusive bits `[]`, MM_TELE_003 four poses. Re-read vtable install
  `mov [ecx], 0x4b1250` at `0x00434474`; overlap calls at `0x435467` /
  `0x4354cb`; `push 4` at `0x0041A317`; relocate `fld` of `2.0` from
  `0x004B1238`; partner write `mov [ecx+0x260], eax` at `0x00434BF3` after
  id `+0x98` → `0x00444BA1`. Mutate-alone: `#if 0` the `teleportAt()` arm →
  FAIL relocate; restored + touch → suite **16/16**. Promotes detection,
  delay, and relocate; pairing via file-order half is a stand-in, not a
  read of the stream id.

* **Done: SpeedPad is object AABB → FASTER (Phase 3 Round 17).**
  `speedup_two_way.a3d` (vtable `0x004B1178`); overlap `FUN_00432742` /
  `0x00430BA1` on list `0x004C88D0` with facing (`+0xbc`) and vdir
  (`+0xa0`) filters; activate `FUN_00432632` dispatches embed `(−2,6,1)` →
  FASTER ctor `0x00416730` (vtable `0x004B0A38`, type `0x10`); enter
  `0x0041688C` sets `+0x50=2` and 5.0 s; `0x0042E2BF` doubles the main
  state while `+0x50==2`. Cue `SpeedPad` `0x004C0D10` → `DAT_004E32F8`
  plays on activate. Live VH21_JC AABB 2×6 (col±1 × [ty..ty+6]); timer
  5.0→4.44; poke ~2× column rate. **0** exclusive cell bits (9/9).
  `pressure_pad.a3d` not this object. Sim: `SpeedPad`,
  `Rules::fasterMs=5000`. Scratch: `docs/scratch/phase3-round17-speedpad.md`.
  Harness: `measure_speedpad.py`. `testSpeedPadFaster`. Suite 16/16.

  **Facilitator check, 2026-10-16.** Offline `--audit` re-run: 9 levels /
  16 placements, exclusive bits `[]`, speed-only H10/SHOW_MT3/VH21_JC.
  Re-read enter `mov [eax+0x50],2` + timer hi `0x40140000` (5.0) at
  `0x0041688C`; ctor vtable `0x4b0a38` / type `0x10` at `0x00416742`;
  `cmp [eax+0x50],2` → `mov [ebp-0x10],2` at `0x0042E2E7`; cue play via
  `[0x4e32f8]` at `0x004326D1`. Mutate-alone: `#if 0` the `speedPadAt`
  arm → 3 behavioural FAILs; restored + touch → suite **16/16**. Natural
  H10 watch was inconclusive (pad culled); VH21 forced-on-pad series is
  the live pin. SLOWER sibling (`+0x50=3`) noted, not modelled.

* **Done: Balloon cue is unused in retail (Phase 3 Round 18, clean
  negative).** Cue `Balloon` `0x004C0D24` → register `FUN_0045e51f` slot
  `+0x1c8f8` / `DAT_004E3300` (brief's `+0x1c8f4` was Action). Only sites:
  write `0x0045E704`, preload `0x0045EB2B`, unload `0x0045F172`. **0**
  absolute refs to `DAT_004E3300`; **0** of 23 `FUN_0044535b` play callers
  load it. No `balloon.a3d`. `SOUNDS\BALLOON.WAV` asset-only. Exit
  `Models\exit\balloon_*` are decorations, not this gadget. Sim:
  `kBalloonGadgetInRetail=false`. Scratch:
  `docs/scratch/phase3-round18-balloon.md`. Harness: `measure_balloon.py
  --audit`. `testBalloonCueUnused`. Suite 16/16.

  **Facilitator check, 2026-10-16.** `--audit` re-run AUDIT PASS. Re-read:
  `push 0x4c0d24` then `mov [edx+0x1c8f8],eax` at `0x0045E6F0`/`0x0045E702`;
  Action store is `+0x1c8f4` just before; **0** `.text` imm refs to
  `0x004E3300`; `+0x1c8f8` only the three register/preload/unload sites;
  Action contrast `mov eax,[0x4e32fc]` at `0x004556AA`. Mutate-alone:
  `kBalloonGadgetInRetail=true` → FAIL; restored + touch → suite **16/16**.

* **Done: Fire cue is unused in retail (Phase 3 Round 19, clean
  negative).** Cue `Fire` `0x004C0D2C` → register `FUN_0045e51f` slot
  `+0x1c8fc` / `DAT_004E3304`; preload `0x0045EB41`; unload `0x0045F186`.
  **0** absolute refs to `DAT_004E3304`; `+0x1c8fc` only those three sites;
  no fire/flame/burn a3d; `SOUNDS\FIRE.WAV` / `SPRITES\FIRE.SPR`
  asset-only. Sim: `kFireGadgetInRetail=false`. Scratch:
  `docs/scratch/phase3-round19-fire.md`. Harness: `measure_fire.py
  --audit`. `testFireCueUnused`. Suite 16/16.

  **Facilitator check, 2026-10-16.** `--audit` AUDIT PASS. Re-read
  `push 0x4c0d2c` / `mov [ecx+0x1c8fc],eax` at `0x0045E708`/`0x0045E71A`;
  **0** refs to `0x004E3304`; `+0x1c8fc` only three sites. Mutate-alone
  `kFireGadgetInRetail=true` → FAIL; restored + touch → suite **16/16**.

* **Done: Witch cue is UI-only (Phase 3 Round 20; no lemming gadget).**
  Cue `Witch` `0x004C0D34` → register slot `+0x1c8a8` / `DAT_004E32B0`
  (not sequential `+0x1c900`); preload `0x0045EB57`; unload `0x0045F19A`.
  Sole absolute play: `0x0045D200` → `FUN_0044521b` inside pause arm
  `FUN_0045cff9`, gated on `strcmpi` vs `compiled\e10_kf_1.lvl` at
  `0x004C0BD0` — plays Witch and skips pause UI on that level. No
  `witch.a3d`; `SOUNDS\WITCH.WAV` for that easter egg. Sim:
  `kWitchGadgetInRetail=false`. Scratch:
  `docs/scratch/phase3-round20-witch.md`. Harness: `measure_witch.py
  --audit`. `testWitchCueNoLemmingsGadget`. Suite 16/16.

  **Facilitator check, 2026-10-16.** `--audit` AUDIT PASS. Re-read
  register store `+0x1c8a8` at `0x0045E732`; play arm `push 0x4c0bd0` /
  `push 0x4cbbf8` / `call` then `mov ecx,[0x4e32b0]` at `0x0045D1E8`–
  `0x0045D20C`; sole abs ref to `0x004E32B0`; level string
  `compiled\e10_kf_1.lvl`. Mutate-alone `kWitchGadgetInRetail=true` →
  FAIL; restored + touch → suite **16/16**.

* **Done: Lever is object AABB → facing-gated toggle + message fanout
  (Phase 3 Round 21).** `lever.a3d` in 55 levels / 130 placements; no
  exclusive cell bit. Vtable `0x004B1288`; overlap `0x00436D30` (list
  `0x004C88D0` + AABB `0x00430BA1`) filters facing vs `+0xb8`; activate
  `0x00436E14` plays cue `Lever` `0x004C0CE0` / `DAT_004E32E4`, toggles
  `+0xb8`, fans `+0xbc`/`+0xcc` via `0x00433AC0` → `FUN_00433B2E` (type 5
  / type 4 to linked ids). No lemming attach. Live E16 AABB 2×6; proof
  `+0xb8 1→−1`. Sim: `Lever` state toggle (message fanout not modelled).
  Scratch: `docs/scratch/phase3-round21-lever.md`. Harness:
  `measure_lever.py`. `testLeverToggle`. Suite 16/16.

  **Facilitator check, 2026-10-16.** `--audit` 55/130, exclusive bits
  `[]`. Re-read ctor `mov [eax],0x4b1288` at `0x00436CE6`; list
  `0x004C88D0` in overlap; cue play `[0x4e32e4]` at `0x00436E29` (sole
  abs ref). Mutate-alone `#if 0` `leverAt` arm → 2 FAILs; restored +
  touch → suite **16/16**. Promotes detection + toggle; linked-object
  fanout is READ, not yet simulated.

* **Done: ForceF cue is unused in retail (Phase 3 Round 22, clean
  negative).** Cue `ForceF` `0x004C0CC8` → slot `+0x1c8d0` /
  `DAT_004E32D8`; preload `0x0045EA39`; unload `0x0045F0AA`. **0**
  absolute refs to `DAT_004E32D8`; no force/fan/blow a3d;
  `SOUNDS\FORCEF.WAV` asset-only. Sim: `kForceFGadgetInRetail=false`.
  Scratch: `docs/scratch/phase3-round22-forcef.md`. Harness:
  `measure_forcef.py --audit`. `testForceFCueUnused`. Suite 16/16.

  **Facilitator check, 2026-10-16.** `--audit` AUDIT PASS. Re-read
  `push 0x4c0cc8` / `mov [edx+0x1c8d0],eax` at `0x0045E600`/`0x0045E612`;
  **0** refs to `0x004E32D8`; `+0x1c8d0` only three sites. Mutate-alone
  `kForceFGadgetInRetail=true` → FAIL; restored + touch → suite **16/16**.

* **Done: Axe is a HUD sprite sheet (Phase 3 Round 23; no lemming
  gadget).** Name `Axe` `0x004C0D6C` → sprite registrar `FUN_00446040`
  slot `+0x1c914` / `DAT_004E331C` (not SFX); sole abs use `0x00496E40`
  → `FUN_00496d5b` UI bind. `SPRITES\AXE.SPR`; no `axe.a3d`. Sim:
  `kAxeGadgetInRetail=false`. Scratch: `docs/scratch/phase3-round23-axe.md`.
  Harness: `measure_axe.py --audit`. `testAxeSheetNoLemmingsGadget`.
  Suite 16/16.

  **Facilitator check, 2026-10-16.** `--audit` AUDIT PASS. Re-read
  `call 0x446040` / store `+0x1c914` at `0x0045E7CD`/`0x0045E7D5`;
  `mov eax,[0x4e331c]` at `0x00496E40`. Mutate-alone
  `kAxeGadgetInRetail=true` → FAIL; restored + touch → suite **16/16**.

* **Done: Bubble cue is SFX on fluid type-0x23 (Phase 3 Round 24; no
  Bubble gadget).** Cue `Bubble` `0x004C0C9C` → `+0x1c8bc` /
  `DAT_004E32C4`; sole abs play `0x00404697` / `0x004046C4` inside type
  `0x23` step `FUN_00404372` (vtable `0x004B03C0`). Attach ctor
  `FUN_004041fd` from walker @ `0x0041C3BA` on fluid mismatch. Distinct
  from drown `0x22`. No `bubble.a3d`. Sim: `kBubbleGadgetInRetail=false`.
  Scratch: `docs/scratch/phase3-round24-bubble.md`. Harness:
  `measure_bubble.py --audit`. `testBubbleCueNoGadget`. Suite 16/16.

  **Facilitator check, 2026-10-16.** `--audit` AUDIT PASS. Re-read ctor
  `mov [eax],0x4b03c0` / type `0x23` at `0x0040420F`; attach `call
  0x4041fd` at `0x0041C3BA`; play refs only those two. Mutate-alone
  `kBubbleGadgetInRetail=true` → FAIL; restored + touch → suite **16/16**.

* **Done: Scrape cue is dig/bash refuse SFX (Phase 3 Round 25; no Scrape
  gadget).** Cue `Scrape` `0x004C0CA4` → `+0x1c8c0` / `DAT_004E32C8`;
  sole abs play `0x0040C139` (bash) and `0x00415B33` (dig). Mine refuse
  is `MineMetal`, not Scrape. No `scrape.a3d`. Sim:
  `kScrapeGadgetInRetail=false`. Scratch:
  `docs/scratch/phase3-round25-scrape.md`. Harness: `measure_scrape.py
  --audit`. `testScrapeCueNoGadget`. Suite 16/16.

  **Facilitator check, 2026-10-16.** `--audit` AUDIT PASS. Re-read
  register store `+0x1c8c0` at `0x0045E5B2`; refs only `0x40c13a` /
  `0x415b34`. Mutate-alone `kScrapeGadgetInRetail=true` → FAIL; restored
  + touch → suite **16/16**.

* **Done: Rumble cue is object state-commit SFX (Phase 3 Round 26; no Rumble
  gadget).** Cue `Rumble` `0x004C0CFC` → `+0x1c8e8` / `DAT_004E32F0`;
  sole play `0x00423823` inside step `FUN_00423749` (vtable `0x004B0F00`)
  when deferred commit finishes and prior `+0xa4 ≠ 0` → `FUN_00406a00` →
  `FUN_0044535b`. Imm hits `0x46dfc3` / `0x489453` are call+`0x1c8`
  false positives. No `rumble.a3d`. Sim: `kRumbleGadgetInRetail=false`.
  Scratch: `docs/scratch/phase3-round26-rumble.md`. Harness:
  `measure_rumble.py --audit`. `testRumbleCueNoGadget`. Suite 16/16.

  **Facilitator check, 2026-10-16.** `--audit` AUDIT PASS. Re-read
  `MOV EAX,[0x004E32F0]` at `0x00423822` inside `FUN_00423749`; register
  store `+0x1c8e8` at `0x0045E6A4`; call-FP at `0x46dfc3` / `0x489453`.
  Mutate-alone `kRumbleGadgetInRetail=true` → FAIL; restored + touch →
  suite **16/16**.

* **Done: Changeop cue is UI skill/volume SFX (Phase 3 Round 27; no Changeop
  gadget).** Cue `Changeop` `0x004C0CB4` → `+0x1c8c8` / `DAT_004E32D0`;
  play loads `0x0045AD07` (skill-panel `FUN_0045ac5c`, gate
  `dword[0x004C6A08]==2` → `FUN_0044521b`) and `0x0048EB0C` (options
  slider `FUN_0048e8d5` / vtable `0x004B24D4` → `FUN_00404cc0` →
  `FUN_00497f39`). No `changeop.a3d`. Sim: `kChangeopGadgetInRetail=false`.
  Scratch: `docs/scratch/phase3-round27-changeop.md`. Harness:
  `measure_changeop.py --audit`. `testChangeopCueNoGadget`. Suite 16/16.

  **Facilitator check, 2026-10-16.** `--audit` AUDIT PASS. Re-read
  `MOV EDX,[0x004E32D0]` at `0x0045AD05` in `FUN_0045ac5c`;
  `MOV EAX,[0x004E32D0]` at `0x0048EB0B` in `FUN_0048e8d5`. Mutate-alone
  `kChangeopGadgetInRetail=true` → FAIL; restored + touch → suite **16/16**.

* **Done: Die cue is SFX on type-0x23 enter (Phase 3 Round 28; no Die
  gadget).** Cue `Die` `0x004C0CD0` → `+0x1c8d4` / `DAT_004E32DC`; sole
  abs play imm `0x004042CF` inside enter `FUN_00404261` when fluid
  `+0x128==1` → `FUN_00406a00` → `FUN_0044535b`. Distinct from FallSplat
  `DAT_004E32E0` (splat enter `FUN_0040b3ec` / type `0x1a`) and unused
  Splat `DAT_004E32D4` (0 abs play refs). No `die.a3d`. Sim:
  `kDieGadgetInRetail=false`. Scratch: `docs/scratch/phase3-round28-die.md`.
  Harness: `measure_die.py --audit`. `testDieCueNoGadget`. Suite 16/16.

  **Facilitator check, 2026-10-16.** `--audit` AUDIT PASS. Re-read
  `MOV ECX,[0x004E32DC]` at `0x004042CD` in `FUN_00404261`; Splat 0 abs
  refs; FallSplat sole play imm `0x0040B4B6`. Mutate-alone
  `kDieGadgetInRetail=true` → FAIL; restored + touch → suite **16/16**.

* **Done: splat-exemption zones (Phase 3 Round 29).** Fluid
  `object[+0x128]` 0/1/2 stamps `0x200`/`0x2000`/`0x4000`; hatch type →
  `[+0x4c]` (sheet table `0x004BFFC8`: Lemmings / LemmingsWater /
  LemmingsAcid). Fall land `FUN_00416433` skips splat for
  `0x200`+`[+0x4c]==1` and `0x4000`+`[+0x4c]==2` (lava `0x2000` has no
  arm). Not drown `0x40`. Scratch:
  `docs/scratch/phase3-round29-splat-exempt.md`. Harness:
  `measure_splat_exempt.py`. Suite 16/16.

  **Facilitator check, 2026-10-16.** `--audit` matches (Fluid 77; zones
  63/11/9 all ∩ Fluid; hatch types `{0:117,1:20,2:7}`). Re-read
  `and 0x200`/`cmp [+0x4c],1` at `0x00416623`/`0x0041662B` and
  `and 0x4000`/`cmp [+0x4c],2` at `0x00416639`/`0x00416641`; Fluid stamps
  `push 0x200`/`0x2000`/`0x4000` at `0x00428D58`/`0x00428D79`/`0x00428D9A`.
  Mutate-alone `zoneExempt=false` → FAIL `testSplatZoneExempt` (4);
  restored + touch → suite **16/16**.

* **Done: inverted steppers (Phase 3 Round 30).** After anti-grav
  (`[+0x44]==-1`), step-up (dy ±1/±2), climb/hoist (net +2·vdir), and dig
  (`Y -= vdir`, refuse mask `0x20`) match upright with gravity flipped.
  Dig mask `0x10` upright / `0x20` inverted at `0x00415A86`/`0x00415A96`;
  refuse live on MM_DIG_000; step-up/climb on m06_kf_1. Retail `0x10` is
  0/102 (paint-only). Bash `0x4` right-facing still unread. Sim:
  `verticalDir` in walk/climb/hoist/dig; `Terrain::digRefused`. Scratch:
  `docs/scratch/phase3-round30-inverted.md`. Harness:
  `measure_inverted.py`. Suite 16/16.

  **Facilitator check, 2026-10-16.** `--audit`: `0x10`=0 cells; `0x20` in
  8 levels. Re-read `mov [ebp-0x10],0x10` at `0x00415A86`, inverted
  `0x20` at `0x00415A96`, walk `add …,[+0x44]` at `0x0041C48E`.
  Mutate-alone `digRefused→false` → FAIL `testInvertedDigRefuseAndClear`
  (5); restored + touch → suite **16/16**.

* **Done: unread cell-flag writers (Phase 3 Round 31).** `0x100` is
  authored-only (3/102: H08_MT_1, H22_MT_1, VH06_MT) — no runtime
  `or`/`and`-imm / stamp-fed setter in `.text`; round-15 reader
  `0x00415552` → `lem[+0x5f]` unchanged (not death). `0x400` is the
  door/gate solid stamp (vtable `0x004B0F00`): paint `|0x403` at
  `0x004275C1`, erase `&0xfbfc` at `0x00427641`; mesh scan
  `0x004615E9` / `0x00461704`. Same object as Round 26 Rumble host. No
  sim change. Phase 3 residue list closed. Scratch:
  `docs/scratch/phase3-round31-cell-writers.md`. Harness:
  `measure_cell_writers.py --audit`. Suite 16/16.

  **Facilitator check, 2026-10-16.** `--audit` green (0×`or 0x100`; paint
  callers `0x4271d5`/`0x427509`/`0x43b1f8`; 3×`0x100` levels; 14827×
  `0x403`). Byte re-read `and edx,0x100` at `0x00415552`, `or edx,0x403`
  at `0x004275C1`, `and edx,0xfbfc` at `0x00427641`, `and eax,0x400` at
  `0x00461704`. No sim mutate-alone (comments + harness only); suite
  **16/16**.

* **Read, not modelled** (from round 8): a faller landing in a blocker's
  stamped column should stop four rows up, because `0x0041A99A` accepts any
  non-zero cell with `0x80` clear and a stamped air cell reads `0x00000800`.
  Two attempts to catch it live failed on the hover naming a neighbour; the
  exact run that would settle it is in the round-8 note. ~~The step-down arm,
  the climber top-out and the hoist hold are cited open differences.~~
  **Closed by round 10**, along with the delayed float halving; the climb
  compare it also owed is done, through the actor list rather than
  `compare_tick`, which is superseded. Still unverified after Round 3:
  **which column direction the constructor's `+0x40 = 1` is.** It matches our
  `Facing::Right` numerically, but the first post-landing step went toward
  increasing column on two levels and decreasing on the third, and a wall can
  turn a walker on its first step, so three samples do not isolate it.
* **Two environment fixes landed 2026-10-14.** `scripts/Invoke-Ucrt64.ps1`
  used `msys2_shell.cmd -no-start`, which detaches into its own console: it
  printed nothing and exited 0 **even for a command that never ran**, so a
  failed build reported success. It now calls `usr/bin/bash.exe` directly
  with `MSYSTEM=UCRT64`, which propagates both stdout and the exit code.
  Separately, `test_collisiongrid`'s Terrain-mapping check compared
  `terrain.solid(x, y)` against `grid->solid(x, y)` on the *same* row, which
  the y-flip breaks by construction; it now compares across the axis. Fixing
  only the axis took its 10046 mismatches to 0, which is itself the evidence
  that the flip was the whole story and no parser bug was hiding under it.
  The live-dump comparisons in that file are untouched and still byte for
  byte. Suite is 16 of 16.

#### Phase 2 close-out, 2026-10-14

The six leftovers from the handoff. Each one below was checked again against
the exe or the `.LVL` bytes, not taken from the note that first wrote it.
Re-run scripts: `read_steppers.py --cell-flags`, `read_grid_siblings.py`,
`read_polylist_tail.py`, `measure_spawn.py`.

**Cell bits.** Observation. After `call 0x40DE60` and a load of the returned
word:

| bit | instruction | what it does |
| --- | --- | --- |
| `0x40` | `0x00415386` `and eax, 0x40` | step of vtable `0x004B09C0`. Set back onto a cell at `0x0042092E` and `0x00420A0F` |
| `0x100` | `0x00415552` `and edx, 0x100` | the same step, on the branch where `0x40` is clear. No setter found |
| `0x400` | `0x004275C1` `or edx, 0x403`, stored back | painted with solid and indestructible. Cleared by `and edx, 0xfbfc` at `0x00427641`. Tested at `0x00461704` |
| `0x8000` | `0x0041C154` `and edx, 0x8000` | walk step `0x0041BDD4` calls `0x0041BFFC` at `0x0041BEE6`. The word tested is the probe at `0x0041C104`, saved at `0x0041C10B`. The later probe at `0x0041C147` is a different cell |

None of the four is named as a terrain kind. `kCellUnreadBits` is now
`0x4|0x8|0x20`. The search that left them there is file-backed `.text` only:
`test`/`and`/`or`/`xor` immediates of exactly those bits, and the same forms
within 16 instructions of `call 0x40DE60`. The hits that search does find are
static bytes (`0x004C55C8`, `0x004ECA78`), `or ah, 8` at `0x0040E312` (the
blocker bit `0x800`, because AH is bits 8..15), and fields that are not the
word `0x40DE60` returned. Not searched: `test reg, reg`, `bt reg, reg`, and
indirect calls.

**The two neighbours.** Observation. `FUN_00439FD3` loads, in order,
`+0x42C` through `FUN_0043B640` (`shl edx, 2` at `0x0043B680`), `+0x43C`
through `FUN_0043B9C0`, `+0x41C` the grid through `FUN_0043B640`, then
`+0x664` through `FUN_0043B700` (`0x0043B73D` pushes the count with no
shift). In all 102 levels the first array's count is 49858 and the byte
array's count is 12288. On `MM_CLIMB_002`, `MM_FLOAT`, `MM_DIG_000`, and
`VH13_KF_1` those 12288 bytes are integers in `0..12`, not floats.
`FUN_00403F40` indexes `y * 128 + (x & 0xff) / 2`, reads one byte, and
multiplies by 24 to select a record at `+0x678`. The movement steppers
`0x00408227` and `0x00416C49` call it. The 49858-word array is the surface
index: `FUN_0043C930` computes `arg3 + 2 * (arg1 + 257 * arg2)`, and
`2 * 257 * 97 = 49858`. Hypothesis, not promoted: those bounds are x, y, and
an inner/outer slot. The two `movsx` words at the one call site are not
shown to be x and y.

**Spawn.** Observation. A lemming is not created at the entrance object's
translation. `0x00425C30` sign-extends hatch `+0x9A` and `+0x9C` and calls
`0x0042DE51`, which pushes the constant 0.5 (`0x3FE00000` at `0x0042DE6F`).
The level loader writes those words at `0x00425F1C` and `0x00425F2E`. The
file triples, count then x then y, are `(10, 32, 42)` at `0xA0C5A` in
`MM_CLIMB_002`, `(10, 65, 85)` at `0x13C197` in `MM_FLOAT`, and
`(20, 84, 79)` at `0x130AE3` in `MM_DIG_000`. On those three levels the x
word is the entrance column and the y word is the entrance world Y minus 2.
Nothing in the constructor subtracts 2. The entrance Y is a different field,
the hatch double at `+0x10`. The first live sample of each birth was already
a few hundredths below the y word, with `+0x18` still 0.5. Hypothesis: that
gap is a few milliseconds of the 20 units/s fall. Not concluded: that every
level's y word is the entrance Y minus 2. `World::spawn` still uses the
entrance translation, and the 81-of-102 survey was not retuned.

**Lattice placement.** Observation. `grid0` is the 49858-word index.
`grid1` is empty. The old reader started at the first `0xFFFFFFFF` after
`"Cylinder"` and copied `header[3]` bytes twice. That window sits inside
this array. On 65 levels it started 1–4 words late. On 10 levels, including
`MM_CLIMB_002`, the second slice ran past `N` into the collision grid.
`header[3] == header[4]` in all 102, a multiple of 64 from 20480 to 111936,
and it is not `N * 4`. What those two words are is still open.

Neighbour distances on the full array, recomputed without `SurfaceMesh` on
`MM_CLIMB_002`, `MM_FLOAT`, and `MM_BUILD`: slot `+1` is about 10 (the
thickness pair), `+514` is 1.00, `+8224` is 16.00. `16 * 514 = 8224` and
`8 * 2 = 16`, so the ring stride is 16 rows of the same map, and `key+16`
is eight columns, one 11.25° step. `test_corpus`'s lattice side was rebased
after this (median solid 27.5%, 93 of 102 releasable). That rebase records
the wider window. It is not a claim that the raster matches the original.
The stored-grid side was not touched by that rebase; its own figures moved
later, with the y-flip (99 of 102, median solid 39.7%, median drop 13).

**Surface fits.** The chord window, the half-step wall, and `lo < 31.0`
remain in `SurfaceMesh.cpp`, each commented as a render-only fit from
`MM_CLIMB_002` and not an engine rule. They were not replaced with a new
constant.

**PolyList tails.** Observation. A kind 5 record is three 52-byte vertices
plus a `u32` (160). A kind 7 record is four vertices plus a `u32` (212).
One vertex is the index and six doubles already parsed. Checked on
`MM_CLIMB_002` and `MM_FLOAT`; `read_polylist_tail.py` reports the same
shape on all 102. The trailing `u32` is stored as `endWord` and not named.
Kind 4 and kind 6 do not use this stride.

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

**Any level can now be entered, and not by finding its node.** Navigation was
stuck on one level because the only known gold node was the first, at render
pixel (206, 204), found by clicking. Finding the other 101 that way was never
going to work: the level map is a **3D scene the camera orbits**, so a node's
screen position is a projection, not stored data. Three checks agree there is
no table to look up -- `LEVELMAP.DAT` holds only id, name, limit, total and
save target; `LEVELS.MAP`'s trailing integers are 1/0/0 throughout; and the
pair (206, 204) appears nowhere in the exe as i16, i32, f32 or f64, in either
order.

What the engine does keep is a flat record table that the map merely indexes,
at **`0x004E35D4`** -- 5 groups of 25, record size `0x118`, with the level id
at `+0x000`, the display name at `+0x080`, and the time limit / total / save
target at `+0x100`. It is zero in the image and filled at startup from
`LEVELMAP.DAT`; live it holds 102 named records and 23 reading `blank`, which
is exactly the 102 `.LVL` files. The selection path:

```
0049b190  call 0x00460610          ; group index <- the global 0x005D5A00
0049b19d  call 0x00460630          ; slot index  <- the global 0x005D5A04
0049b197  imul esi, esi, 0x1b58    ; group stride, 25 records
0049b1a2  imul eax, eax, 0x118     ; slot stride, and the record size
0049b1a8  lea  ecx, [esi+eax+0x4e35d4]
0045d89d  call 0x0045c9ab          ; sprintf "Compiled\%s%s" -> 0x004CBBF8
```

So the node does not have to be found: **the record it points at can be
changed.** `goto_level.select_level` copies the wanted level's whole `0x118`
record over slot `[0][0]`, the one node whose position is known, and the
existing click loads it. Copying the whole record rather than just the id
means the clock and the lemming counts come from the real level too, and
`enter_level` restores the slot in a `finally`.

**Do not write the index globals instead.** `0x0049B14F` and `0x0049B15B` set
slot and group from the picked node immediately before `0x0049B190` reads them
back, so a value written from outside is overwritten inside the same click.
The record swap has no such race -- it is read once, at click time.

**The swap is checked, because it mutates the game's own table.** That a level
entered this way is the genuine level was confirmed three independent ways on
`MM_DIG_000`: the path the engine itself built, at `0x004CBBF8`, reads
`Compiled\MM_DIG_000.lvl`; the save target at `0x004CBAB0` reads 10 where
`MM_CLIMB_002` reads 1, matching `LEVELMAP.DAT`; and the release counter
reached 18, consistent with 20 lemmings and not with 10. `dump_grids.py`
re-checks the path buffer before writing any dump, so a mis-navigation cannot
be recorded as a measurement of the wrong level.

Changing level means **restarting the process**: there is no route from inside
a running level back to the map that does not go through ESC, and ESC is the
one key that has quit the process outright. A relaunch costs about fifteen
seconds, so `dump_grids.py` restarts between levels.

Still fragile: `find_menu_index` found no candidate on one run in six and the
script refused to press RETURN blind rather than risk "Beenden". That is the
safety working; a retry succeeded immediately.

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

**Not measured, and why.** *(Superseded — Phase 3 measured all five. The
blocker described here was level-map navigation, which `goto_level.py` solved;
`fallSpeed` 4, `floatAfter` 10 and `splatDistance` 30 have live series, and
`maxStepUp` 2 and `lemmingHeight` 3 were read and then bounded live. Kept
because the reasoning about what cannot be measured from one level still
applies to the rounds that are left.)* Everything that needs a lemming to
leave the ground: `fallSpeed`, `splatDistance`, `floatAfter`, `maxStepUp`,
`lemmingHeight`. The
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
set from this section. That was a weaker warrant than the measured fields
have and `Rules` said so in those words: a read of the shipped bytes, with
no live series behind it. **All four now have one** (§1.6), and the reads
held — `fallSpeed` 4 on three walk-off edges, `floatSpeed` 2 over fourteen
ticks, `floatAfter` 10 at the umbrella latch, `splatDistance` 30 either side
of the threshold. `floatSpeed` turned out not to be a constant at all but
`fallSpeed / 2`, a division in the accessor `0x00416038`, which is a stronger
guarantee than either number having been read separately. What makes them more than plausible numbers is
that the same arithmetic reproduces two *measured* fields exactly off the
same instruction shape -- so the units are not a guess even though the fall
itself was never sampled. Nothing else in `Rules` may be promoted on a
static read alone without that kind of cross-check, and the fall still owes
its live run.

**`fallSpeed` was first set to 10, off the wrong state, and is now 8.**
*(Two corrections. The value is **4** since the raster halving — the 8 here is
the old two-mask-rows-per-unit framing. And the 25.0 is at **`0x004B0C40`**,
read back as 25.0 from the shipped bytes; `0x004B0C60` is the state's
**vtable** and eight bytes there are 1.96e-307. The substance — that it is
the wrong state — stands.)* The
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

**Tension with the unexplained snap, stated rather than smoothed.** An
earlier sample was 48.190 → 32.190 with no intermediates in 0.39 s. A
4-unit step would take ~0.8 s. That snap is still not the fall. A later
walk-off (`docs/scratch/phase3-round2.md`) showed the intermediates:
48.190 → 44.190 → 40.190 → 36.190, each **−4.000** world units. So 20.0 /
`fallSpeed = 4` is live-confirmed for the step size; the snap remains a
separate unread transition.

**Next.** Fall *step* is done. Still owed live: which vtable a walk-off
enters, umbrella at 10, splat past 30, `floatSpeed`. Then skill constants
from the enter hooks above, and Phase 3 Round 3 spawn. Do not remeasure
the climb rate.

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
- **Done on the file side.** Cylinder + surface lattice, skills trailer,
  entrance/exit + 417 other placed models (parse only), skin ≠ solidity,
  PolyList 5/7 = draw attrs, ring stride **derived not stored** — and the
  collision grid, below, which supersedes the lattice as the mask source.
- **What is left to close the phase — mostly nothing, corrected 2026-10-16.**
  This list said "parse the grid chunk through the level's chunk order rather
  than by fingerprint, read its seven unread flag bits, and take
  `rasteriseSurface` out of the collision path". All three are done or down to
  residue, and the list was stale:
  - *Chunk order:* **done.** `CollisionGrid.cpp` walks the documented sequence
    from the `"Cylinder"` chunk and lands on the grid in all 102 levels; the
    multiple-of-256 fingerprint is gone from the C++ loader. Residue: the
    `"Cylinder"` chunk itself is still located by a linear byte scan rather
    than from a chunk table.
  - *Flag bits:* **done.** Every bit in a retail grid now has a reader
    instruction behind it (`CollisionGrid.h`), including the four directional
    refusal bits from round 5 and `0x8000` from round 9. Three keep
    placeholder names — `kCellFlag0x40`, `kCellFlag0x100`, `kCellFlag0x400` —
    and `0x1000` is unlisted; what is unread for these is which code, if any,
    **sets** them, not what reads them.
  - *`rasteriseSurface`:* **out of the collision path already.** Its only
    remaining callers are `LevelStats` (the lattice survey columns that
    `test_corpus` asserts on) and `lrtool`. Nothing collides against it.
- **The terrain mask is a chunk in the `.LVL`, and that is the finding this
  phase turned on** (§1.6). 256 x 96 `u32` cells, length-prefixed, read
  verbatim by `FUN_0043B640` into the object `FUN_0040DE60` collides against.
  Byte-identical to the live grid on five independently entered levels, and
  present exactly once in each of the 102 files. So the surface lattice is
  what the engine *draws*; the mask is not derived from it at all.
- **An earlier claim here was wrong and is withdrawn:** "the original keeps no
  occupancy buffer — scanned for across all 182 MB and absent". The sweep
  looked for a byte-per-cell bitmap or a grid of at most four distinct byte
  values, and the grid is neither — it is `u32` flag words. What does stand
  from that round is that the engine holds the `.LVL` surface lattice
  byte-for-byte, which confirmed the file reading.
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
  unit per 200 ms step, so `Rules::climbSpeed` is **1** — one cell is one
  world unit. **Corrected 2026-10-16**: this paragraph said "two mask rows, so
  climbSpeed is 2", which is the retired lattice doubling, and it cited the
  billboard series 34.190 / 35.190 / 36.190, whose source round 10 retracted
  (that quad is not a lemming). The rate is confirmed against the actor list
  instead — a climber matched our sim 16 ticks on one wall and 13 on another.
  Height is the
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
  - **Fall step confirmed live** (`docs/scratch/phase3-round2.md`):
    walk-off 48.190 → 44.190 → 40.190 → 36.190, ΔY = **−4.000** each.
    Agrees with `fallSpeed = 4`. Which of the six fall-ish states is
    entered is still unread (heap vtable scan failed that pass).
  - `LEMMINGSLO.SPR`'s `fall` rate is 0.467, the same value as `float`;
    `walk` is 1 and `climb` is 0.5. Those are the animation clock, not
    mask rows per tick. Do not copy 0.467 into `fallSpeed`.
- **Handoff (2026-10-14).** Full dispatch: `docs/PHASE3_HANDOFF.md`. Do
  not remeasure the climb rate. Do not invent `Rules`. Measured:
  `ticksPerSecond`, `walkSpeed`, `climbSpeed`. Live-confirmed step:
  `fallSpeed`. Still read-only (no live series): `floatSpeed`,
  `floatAfter`, `splatDistance`. Static notes not in Rules: step-up 2,
  head bias 3.0, bash/dig/mine animation-% bites, builder count 12 /
  stamp 2 cells, fuse 5.0 s (`docs/scratch/phase3-round4.md` …
  `round7.md`). Everything else in `Rules` is still classic. Per-tick
  walk oracle works on open floor only (`compare_tick.py`).
  - **Assigning a climber.** Panel select + glide-to-lemming works
    (`measure_dropoff.py`). Writing `0x004C36A0` / `0x004C36A4` is dead
    in play (stays at the map click). Prefer panel index `0x004C9200`.
    Avoid `goto_level.focus()` on this machine — it has crashed dgVoodoo;
    shim-only entry still polls ~144 Hz. RETURN only when the menu
    highlight is exactly 0. ESC from a post-level screen can quit.
  - Open-floor walk compare: columns ≈160–171 at y = 32.190
    (`solid_here 0`). Cols 128–158 at that height are solid in the file;
    live walkers still appear there — do not start a compare on them.
  - Next: Round 3 spawn (hatch int16s), then float/splat live, promote or
    reject round-4 constants with a series, then skill live series, then
    gadgets. Climb *rate* is measured; climb *compare* via
    `compare_tick` is still owed.
  - Do not assume a prior pid is still up. `ui_state`: 0 playing, 2 pause,
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

### Phase 5 — Game shell — **done** (FMV still open)

Menus, level select, progress saves, `.LTX` localisation, SFX + music.
FMV / `.avi` codecs remain unread and deferred.

- **Shell** (`src/shell/Shell`): Main (five LTX items), Ja/Nein new-game
  confirm, Options (language / music / SFX), five-group level map from
  `LEVELS.MAP`, Results. Default language **UK** when that `.LTX` is
  present. `--level` still bypasses into play; `--lang` forces a table.
- **Progress** (`webolution_progress.dat`): unlock-on-clear chain; not the
  stock `SavedGame.dat` layout. `test_progress` pins unlock + serialize.
- **Text**: baked 8×8 font + `Renderer::drawText` (no SDL_ttf). In-level HUD
  uses LTX skill / Out / Saved / Time strings.
- **Audio**: existing SFX cues plus looping `MUSIC\LEMMING01..13`.

### Phase 6 — Platform targets
Linux, macOS, and WebAssembly via Emscripten.

---

## 4. Risks, ranked

1. **`.LVL` format** — **largely retired for terrain.** The collision mask is a
   length-prefixed `u32` chunk in the level file, verified byte-identical to
   the running engine's grid on five levels (§1.6), so the mask is a parsing
   job rather than a reconstruction. What remains is the chunk's anchor, its
   seven unread flag bits, and the row count's location. The surface lattice
   keeps its open questions, but those are now rendering risks, not
   simulation ones.
2. **Exact physics constants** — medium, and partly retired. The step rate,
   the walking speed, the climb ledge and the climb rate are measured. The
   fall *step* is now live-confirmed at 4 world units per tick (20.0 at
   `0x004B0A18` × 0.2 s; series in `docs/scratch/phase3-round2.md`).
   Float, umbrella distance and splat are still binary reads without a
   dedicated live series; which fall-ish state a walk-off enters is still
   unread. One earlier fall read took the wrong state (25.0 sibling) and
   was off by a quarter — that shape of risk remains for the unread
   fields. The skill constants are untouched classic values (static notes
   in `docs/scratch/phase3-round5.md` … `round7.md`).
3. **3D model geometry** — low/medium. Assumed to live in the `.LVL` PolyLists;
   unconfirmed. `TEXTURES\MODELS\...` implies separate model data somewhere.
4. **FMV codecs** — low. Cosmetic, and deferrable to the very end.

---

## 5. Sources

- [Lemmings Revolution file formats — Lemmings Forums (ccexplore)](https://www.lemmingsforums.net/index.php?topic=1439.0)
- [Lemmings Tools & Documentation — The Lemmings Archive](https://www.camanis.net/lemmings/tools.php)
- [Lemmings Revolution — Wikipedia](https://en.wikipedia.org/wiki/Lemmings_Revolution)
