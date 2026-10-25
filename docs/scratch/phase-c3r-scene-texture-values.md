# C3r — scene source pixels, constructor flags and conditional uploads

2026-10-25. Offline READ + DERIVED, with own-process GDI replay; no original
target execution or stock start. Packet `build/tmp/c3r-scene-texture-values/round-note.md`;
`value-table.json` records each resource and all remaining upload conditions.

## Accepted values

For successful normal-manager construction of these exact logical names:

- 13 unprefixed mesh resources (including sand_1/sand_2/sky): texture-object+24=**0**.
- ~Water1: **4**. No ALF pointer on this path.
- 19 caret resources (two grass,17 lolli/sheep): **8**, not actor2 or16 for two carets.

READ chain: `0x4743DD` finds the last backslash through `0x49D4C0`; flag local
starts0 at `0x4743FB`. Comparisons/stores `0x474430..4485` map only the leaf's
first byte `_/$/~/^/+` to1/2/4/8/16. Call `0x4744B1` passes that argument to
`0x47218A`; `0x47220B 89 51 24` stores it. Alpha pointer+0C starts0 at
`0x4721F1`; mask18 at `0x472328` gates allocation/load. File material blend
selectors are separate values, not texture flags. Cached foreign descriptors
or external mutations are not disproved by this constructor rule.

Installed patch hook `0x6CBC1926..1932` returns0 for a null alpha pointer,
allowing the native continuation. **Opaque sand/sky are not automatically
32-bpp patch textures.** C1bb/C1bc's successful nonnull-alpha route requests
A8R8G8B8 and combines unflipped ALF rows with vertically reversed positive-height
BMP RGB, no premultiplication. All19 caret textures already have power-of-two
dimensions, so settled resizer early-return applies. Exact linear BGRA streams
are retained; logical rows additionally require returned masks/pitch=w*4 and
successful source/allocation/GDI/API completion. No pitched-copy repair is inferred.

33 unique resources map to52 base archive members (33BMP,19ALF); every decoded
BMP is24-bpp BI_RGB. Source top-left ARGB and illustrative native replay words:

| Leaf | Dimensions | Source ARGB | RGB565 replay | XRGB1555 replay |
| --- | --- | --- | --- | --- |
| sand_1 |128x128|FFFCD868|FECD|7F6D|
| sand_2 |128x128|FFD5B141|D588|6AC8|
| sky |64x64|FF4274F3|43BE|21DE|

These16-bit words are **conditional own-process GDI-DIB replay/formula results**,
not measured scene surfaces or an exhaustive set of driver formats. Actual native
enumeration/returned descriptor and wrapper GetDC/BitBlt/final Blt conversion
remain unresolved. Source dimensions for these three are invariant under the
two read cap bits; larger destination padding/stretch for other resources is not
invented. Two-archive/no-alternate-prefix source convergence stays conditional.

## Facilitator check

`python -B build/tmp/c3r-facilitator-check.py` independently parses both BOX indices,
authenticates all52 compressed/raw member slices and absence of same-name patch
overrides, and invokes the existing clean-room C++ `lrtool decode` into its own
scratch. Pillow independently decodes those BMPs. Across192512 source pixels,
separate BGRA/alpha and explicit555/565/444 bit formulas reproduce all19 patch
streams and28 conditional native variants, byte-for-byte. Descriptor flag/name
census is13/19/1 for0/8/4, covering all accepted mesh/type2 names.

All58 PE/patch anchors, five complete raw windows and the lazy-manager chain
window hashes are re-read against original bytes. Constructor/pointer initialization,
ALF mask/store, patch null-alpha exit and request masks were semantically re-read;
settled C1bb/C1bc converter facts are reused. `.data` raw-backed end stays4C6000.

Through the candidate's converter, a private ALF[0] XOR1 changes only output byte3
for each of19 patch streams and restores. A private sand_1 BMP blue-byte XOR80
through the own-process GDI replay changes exactly one565 word; original input
restores the complete stream. No original or candidate source file was written.
An initial facilitator mutation passed bytearray to ctypes and failed on type;
converted the private argument to bytes and reran the complete checker successfully.
The candidate's finally blocks release GDI objects/DCs on both paths.

Six install hashes independently match before/after ledger. No Lemmings process;
fresh wrapper build and **34/34 CTest passed**. No production or pixel acceptance.
Narrowing: this accepts constructor-route flags, source decode and conditional
streams, **not native actual format/wrapper conversion or source-registration
uniqueness**. C3 baseline remains0.2050523 against0.02; no format may be selected
by image score to remove those explicit gaps.

Dispatch14:16:53; accepted14:44:54 MDT, **28m01s** dispatch-to-acceptance.
Next verify C3q conditional state packet, then choose exact remaining runtime
inputs or independent scene-gate work before production scene integration.
