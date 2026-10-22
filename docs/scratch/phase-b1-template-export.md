# B1e: decoded retail templates exported, image matching still next

Queue `b1e-menu-image-layout`; original and split-repair briefs under build/tmp.
Worker outputs: `build/tmp/b1e-menu-image-layout/{round-note.md,repair1-note.md}`.
The original broad pass returned only a prediction, no code/matches/score. It was
split, not promoted or called an external blocker. Dispatch 2 implemented just the
retail-pixel export prerequisite. Zero stock starts across both dispatches.

## Result

`FrontendAssetDump <game-dir> <out-dir>` is a standalone CMake tool using the existing
production Box/Bitmap/GameData/PNG APIs, with no reader changes. It exports a sorted
JSON source/dimension/alpha manifest, indexed UK strings and these candidate images:

| Archive texture basename | Size | Alpha |
| --- | --- | --- |
| MAIN_LOGO.BMP | 640x480 | opaque |
| ^FONT.BMP | 256x240 | ALF |
| ^FONTH.BMP | 256x252 | ALF |
| ^FONTP.BMP | 256x256 | ALF |
| _FONT.BMP | 256x240 | key-black |
| _FONTFE.BMP | 256x240 | key-black |
| _FONTSMALL.BMP | 64x16 | key-black |

These are candidates, not an identification of the rendered menu font. All live under
`TEXTURES\`. The 232-entry UK table includes the five visible labels at indices 125
(Start New Game), 126 (Play Current Game), 120 (Language Selection), 3 (Options),
43 (Quit). No glyph layout, template match, reconstruction or B1 gate score exists yet.

## Facilitator check

Reviewed helper and CMake diff: one additive standalone target, only existing production
readers called. Worker exports from its no-space install copy were deterministic across
two runs; invalid directory returned exit 2. Independently built and exported from the
actual `C:\progs\Lemmings Revolution` authority install. Every generated file was
byte-identical to the worker packet, resolving the worker's path workaround without
assuming two installs' complete contents are identical.

Independent checker `build/tmp/b1e-facilitator/verify.py` reads PNGs with Pillow and
compares dimensions/alpha support against the manifest. For the text headline it
bypasses StringTable::parse: `lrtool extract` obtains raw UK.LTX, then a separate Python
u32-offset/NUL/CP1252 parser confirms **all 232 strings**, including all five indices
above, not just the worker's few examples. Texture decoding still relies on the
already-tested production readers; no independent LRZ/bitmap decoder was written.
The checker initially assumed every PNG was RGBA, then was corrected to accept the
PNG writer's documented RGB output for wholly opaque images; no production fix needed.

Own isolated mutation: changed only the helper's loadStrings language UK -> GERMAN,
rebuilt and exported. Independent raw-UK comparison failed at index 1 (Press X button
versus Weiter mit X-Taste), proving the expected language is protected. Restored source
SHA-256 `b1bac8fb6e2ba4ef1037fc314c3710fb677a00404fee1b02feacb8baafe996c7`;
after server interruption, resumed at this restore/build checkpoint rather than
repeating the measurement. Fresh full build and CTest: **29/29**. Rebuilt restored
helper exports match the verified pre-mutation authority export byte-for-byte.

No game, debugger, REA or install write in this round or these checks. B1f now measures
the retained reference against these templates; production renderer follows those
verified image measurements. The full-frame 0.02 gate is unchanged and unassessed.
