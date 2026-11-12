# Executable recovery map

This map records where the source recovery currently meets the larger
reference executable. It is a navigation aid, not a claim that these larger
functions have been ported. The reference is the installed x86
`Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The REA/Ghidra inventory reported 6,322 analyzed procedures, including CRT
code, import thunks, and external functions (`ev_3c905cbda0eac7c0404434965cd835246e300d86eca76c2b4e2e352d64462052`).
That count is not the number of game functions. Some inherited Ghidra names
are implausible for this executable; addresses and machine code remain the
authority.

## Control flow landmarks

| Address | Observed role | Evidence |
| --- | --- | --- |
| `0x0049D518` | CRT entry calls the game entry at `0x00450696` | `ev_f1a3e1e54d4dc57f2ad135cac3390baaa925e2ace048f241d32f6cc16b2eace3` |
| `0x00450696` | Game initialization, media setup, then call to `0x0045CFF9` | `ev_b6f48cbe2efc38061c13b4bca26e7881cee85b46514338070df55c3b58d31f63` |
| `0x0045CFF9` | Win32 message/frame loop; computes elapsed time and calls `0x004423D2` for world updates | `ev_567e30c9895490a5a8a7f97d5893d1218be529cc29d9ae2c8eb80ce6c6122e73` |
| `0x004423D2` | World update; traverses multiple object collections and calls virtual update slot `+0x14` | `ev_4aa6a2db0c09db5e080486d49fe6169682a5fc8d9c9a19d3b452b918c59427e5` |
| `0x0045C9AB` | Opens a compiled level path and calls `0x004432C1` to load its data, then `0x00444D1E` to start play | `ev_d0b958d91a6ff650d628ec710fcb39e24443001ffa244a9fc5a862efa007fc0e` |
| `0x004432C1` | Level loader reads a `"Level 23"` header, five name tables, object data, and world data before later sections | [loader prefix note](evidence/004432c1.md) |
| `0x00439FD3` | Reads the world data section after the fifth name table; file-read path recovered | [world geometry note](evidence/world_geometry_prefix.md) |
| `0x00437D48` | Reads two objects, their recursive children, and auxiliary records; file-read path recovered | [object base note](evidence/object_base_prefix.md) |
| `0x00444D1E` | Level-start setup; changes top-level state to 2 | `ev_5572ef5c08cf747553ad09ed7e79bbe998581de3c606e2a89c71db8e67fcb080` |
| `0x0045F43C` | Opens `Compiled\LevelMap.dat`, then `SavedGame.dat`; loops over 5 groups of 25 records in each path | `ev_2a7126459d8d031de3d06bc09cbade20ebdaaa6bc0cad6bcd970d7355d23a34a` |
| `0x0044F8EF` | Loads `Compiled\LevelsScreen.pc` and level-select screen objects | `ev_ca740b38a3b96a58a8ac48cb86394716f98bf6a781aa40a39dbdc9f4f07603b8` |
| `0x0043F510` | Terrain blast: cell clearing, debris sites and effect ordering recovered | [terrain blast](evidence/terrain_blast.md) |

The level-map and level-select filenames are defined strings in the binary;
their references were observed at `0x0045F460` and `0x0044F914`
(`ev_dc1c580f459e61b4cd1d110b9f943d61b95b9e6e1821f8438fad8f8a7603860c`,
`ev_978c3b8880373a52831aa790ec59e466bb1856606a4b4801e5684905acee6b2c`).
No file format is claimed from those names alone.
The installed `.box` archive index is documented [separately](evidence/box_archive.md),
with a verified compiled-level fixture for further loader recovery.

## World object and actor collections

The world object passed into the main loop's update path is rooted at
`0x004C6A08`. Within `0x004423D2`, its retained-actor list is at
`this + 0x1EC8 = 0x004C88D0` and its actor-pointer registry is at
`this + 0x1FA0 = 0x004C89A8`. The former is traversed by the recovered
gadget actor searches; the latter is traversed by the recovered nearby-timer
effect. Their separate purposes follow from the call sites. The source
currently passes these collections explicitly, so it does not depend on
fixed process addresses. `0x004C6A10` is world `+8` and is the base used by
coordinate and collision-grid helpers.

## Suggested next recovery slices

Actor behavior slices now include falling, walking, climbing, topping, bouncing,
and terminal type `0x1A`, plus membership/attachment/cleanup/removal and wall
contact. Their ledger rows distinguish original-byte comparisons from live-game
runtime. The scheduler's tail at `0x0042E58C` is now recovered separately in
`actor_finalization`: model orientation/position, the `drown` clip check and
terrain sample triple assignment have native comparisons. Its call through
actor `+344` to `0x00458186` can now use the recovered shadow update and its
terrain segment builder `0x00459557`; list/resource operations remain explicit.
The model at
`+78` is inline storage, not a pointer field. Spawn monitor `0x2F` and vertical
terrain probes now provide additional concrete dependencies for actor updates.

Actor construction `0x0042DE51`, base initialization `0x004230A2`, actor/base
destruction, registry operations and forced/drowning/region-terminal behaviors
now have native comparisons. Inline model and shadow constructor chains are
concrete on normal return, and model destruction preserves conditional resource
release. The full world update `0x004423D2` composes the recovered collection
pass and preserves timed sequence, subsystem and removal order. Flapper type
`0x20` and its following explosion type `2` are recovered separately. These components
still require integration with original resource construction and rendering
before they form a playable runtime.

1. Continue runtime construction and resource resolution in `0x004432C1`.
   Its category-resource selection block now registers the observed
   Lemmings/LemmingsWater/LemmingsAcid/EvilRat names with Lo/Hi suffixes;
   slot registration and actual resource loading remain separate dependencies.
   Cache search/loading, slot registration/growth, manager construction and
   eviction/unload/recreation are recovered. The sprite factory now constructs
   resources and preserves the original file/reader/cleanup call order. The
   installed resource harness composes these with archive and decoded-vector
   adapters across all 19 sprites. Runtime sprite arrays, texture registration,
   nested destruction and rectangle geometry are now concrete too. File core
   reads/close, open-finalization blocks and original open selection/retry are
   recovered; `file_stream_open` reuses those finalization blocks. Packed bitmap
   envelope and codec decoding have retained original-byte comparisons across
   installed BMPs. Texture factory quality/alpha choices, BMP conversion and
   pixel-format selection are recovered, registered and rowed in
   [the function ledger](FUNCTIONS.md); their retained offline synthetic
   comparisons are not in-process gameplay evidence or new measurements.
   Graphics resource creation `472F99`, the DirectDraw/Direct3D device, driver
   and enumeration, GDI, texture destruction, the Win32 file class and packed-file
   I/O remain explicit adapters or boundaries. So do drive/registry preparation
   `47DD54` and discovery, filesystem existence `47E89B`, archive
   readiness/reset/lookup/reload (`47EF48`, `47EF92`, `47F759`, `47E2F3`,
   `47E23B`, `47EFE6`), archive-name resolution and index loading, MessageBox,
   CRT file operations and MSVC exception/allocation-failure recovery.
   Its serialized fields now parse all 105 indexed `.LVL` entries to exact
   EOF, including the [entity tail](evidence/level_entity_tail.md). Auxiliary
   records retain serialized bytes; their nested in-memory fields still
   need recovery.
2. Rendering now runs from rectangle geometry `47A190` through submission
   `414920`, clipping `46D3C1` and device packing `46D460` in recovered C++.
   Adjacent trapezoid `47A565` and anchored rectangle `47A75D` are no longer
   inspected context: both are recovered in `src/sprite_render_shapes.cpp`,
   registered in the build and rowed in
   [the function ledger](FUNCTIONS.md) with their retained offline comparisons.
   Texture manager vtable `4B19E8` routes indexed creation through `471720` to
   factory `4743DD` and constructor `47218A`; graphics setup `472F99` remains
   unrecovered, as do the DirectDraw/Direct3D device and driver, texture
   destruction, archive index loading and MSVC exception/allocation-failure
   recovery, which stay explicit adapters or boundaries. Static bindings are
   recorded in [texture evidence](evidence/texture_resource.md).
3. Implement the effect classes and renderer/resource calls behind the recovered
   full `0x0043F510` terrain-blast orchestration. Its cell edits, debris sites,
   notification and effect/entity call order now have native comparisons.
4. Identify virtual update implementations reached from `0x004423D2`,
   then reconstruct actor and gadget lifecycle order against native bytes.
5. Trace level completion and failure state changes from `0x0045CFF9` to
   connect gameplay updates to the frontend state machine.

Each slice should add exact function-byte digests to
`tools/verify_reference.py`, source with known offsets, and an evidence note
in [the function ledger](FUNCTIONS.md). The installed executable imports
`PATCH.DLL` for DirectDraw, so the exact SHA-256 above is required before
transferring behavior claims to another executable build.
