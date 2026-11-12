# Remaining Level 23 entity records

## Reference and boundary

The reference is the installed x86 executable with SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
This slice begins at the call to `0x00448650` in `0x004432C1` and ends
after `0x0044D429`. Together with the preceding recovered sections, the
reader consumes **exactly the complete file** for all 105 installed compiled
levels. This establishes the serialized read layout for those files, not the
loader's resource resolution or resulting game state.

Source is in `level_model.cpp` and `level_gadget_records.cpp`. Names such as
`Entity42acac` identify the original reader until the class's gameplay meaning
is established. Field suffixes record original destination offsets. Doubles
are retained as binary64 bits.

## Dispatch and shared readers

`0x00465924` reads a tag, constructs a model, and calls vtable slot `+0x50`:

| Tag | Constructor / vtable | Reader | Serialized path |
| --- | --- | --- | --- |
| 0 | `0x00450BA0` / `0x004B1688` | `0x00450F57` | capacity-255 name, three doubles, shared object, flag and optional capacity-128 name |
| 1 | `0x0043CE00` / `0x004B14A8` | `0x0043CEDF` | four integers and capacity-255 name through `0x00468BF2` |
| 2 | `0x00465AD0` / `0x004B18AC` | `0x00466410` | seven doubles, integer, counted ten-double records, two integers, two names and recursive models |

The type-two record reader is `0x004673E0`: two triples followed by
`0x00481B2E`'s four doubles. `0x004664E0` supplies the two capacity-128 names
and recursive child count. Allocation counts and a discarded integer are
retained even though the source does not allocate original runtime objects.

Shared entity reader `0x00432283` consumes the 44-byte actor base, flag
`+0x40`, model-present byte, flag `+0x41`, then an optional model.
`0x0043130E` adds two triples and two counted triple arrays.

The shared geometry reader now accepts class tags zero through three.
Class three (`0x0047AFB1`) reads ten doubles before the usual base.
Polygon type six dispatches through `0x00469B6B`, `0x0046A190`,
`0x0046B250`, `0x0046B280`, and `0x0047A004`: an integer, four
plane doubles, and four vertices each containing an integer and two doubles
(116 serialized bytes). The existing type-four path uses three such vertices
(96 bytes). Types five/seven retain six-double vertices.

## Entity lists and installed coverage

| Loader call | Virtual reader / path | Installed records |
| --- | --- | ---: |
| `0x00448650` | `0x00432283` | 56 |
| `0x00448710` | `0x0042DA25` | 0 |
| `0x004487F0` | `0x00425E50` | 149 |
| `0x004488F0` | `0x00426C6B` | 114 |
| `0x004489F0` | `0x0043130E` | 0 |
| `0x00448AF0` | `0x00424187` via `0x00427706` | 235 |
| `0x00448BD0` | `0x0042ACAC` | 532 |
| `0x00448CB0` | `0x004347EC` | 54 |
| `0x00448D90` | `0x004342BF` | 13 |
| `0x00448E70` | `0x0041E038` | 0 |
| counted `0x00433DD9` | five tagged subclasses | 252 |
| name dispatch | STAT / DYNA / FALL | 25 / 19 / 0 |

The model trees contain 2,327 type-zero models and 56 type-two models.
The latter contain 456 children and 456 ten-double array records. No type-one
model appears in these installed entity sections. Their geometry contains
795 class-zero, 4,747 class-one and 132 class-two nodes; no class-three node
appears. Polygon counts are 3,673 type four, 1,949 type five, 3,821 type six
and 1,839 type seven.

The tagged dispatch `0x00433DD9` maps tags 0/1/2/3/4 to readers
`0x00431B30`, `0x00436FED`, `0x0043732D`, `0x0041D730`,
`0x00432938`. All share `0x00433CEE`. Installed counts are 47/132/7/50/16.
Their counted four-integer arrays contain 580 records, read by `0x00433FB9`.

Startup routines `0x00440D9F`, `0x00440DE3`, `0x00440E27` initialize
the name-dispatch strings from `0x004BFFE0`, `0x004BFFE8`, `0x004BFFF0`:
STAT, DYNA, FALL. `0x00436680` uses strcmp for equality. Their readers are
`0x004333D2`, `0x0042576F`, `0x00427CCD`, with shared
`0x004364F5` and spatial base.

After the lists, `0x0045B86D` reads eleven integers; `0x00447880` reads a
counted integer array (253 values across installed levels). The loader reads
five flags, a 16-bit field and an integer, then `0x0044D429` delegates to
`0x0044D570` / `0x0044D670` for counted three-integer records. That final
list is empty in all installed levels.

## Evidence and verification limits

All new reader byte ranges, mapped PE file offsets and SHA-256 digests are
pinned in `tools/verify_reference.py`. The dispatch slots and name constants
are also pinned as data. Selected REA observations:

- Type-zero model: `ev_cac8018a093d7b5e7647a04ba24cc83201e2d526f138bc6806530a62ba00655a`.
- Type-two model: `ev_a98278ea2dcbd4dc1d0f24a90a22db7f61b63fc51aab1c64d1680327dcc64f3f`.
- Shared model entity: `ev_da1302ea1cedc65d9b88b76575a6ab501fd5f67dbf47b5fe484ba887e4c2800c`.
- Compound reader `0x0042ACAC`: `ev_45787c985d2ac61dad2b452c876c9b5f7bd17b24e0726d14735b4f760efd9ee8`.
- Compound reader `0x004347EC`: `ev_bbe6c52d5ab8c3ba7d414c1a8fc77081aad10246dae838920a2d98bc334204a5`.
- Tagged dispatch: `ev_8e87a3093781494496768b76e37ed0c805a26a1dbea87eaa8704f0b4916f92dc`.
- Named common reader: `ev_822356a3df2a658a30f95429b27462f2abaaba25961b0f55319acdba9862fcff`.
- Final record reader: `ev_13f39a0a371cc6629fade9924caf2b31902bf432f8d4eebedc2c23e5531d4ca4`.

Synthetic unit tests exercise the installed-data gaps: populated linked and
spatial lists, FALL records, type-one models, class-three objects, and final
records. They also check truncated records, corrupt counts, unknown tags,
embedded NUL names, and model nesting limits. These tests validate the source
against the statically recovered layout; they are not native comparisons.

The independent Python fixture decoder compares the C++ fields, hashes,
counts and cursors on x64 and x86. It now requires EOF, so a skipped or
overconsumed tail fails. Installed data does not exercise the zero-count
paths listed above or arbitrary malformed input. Bounded reads, depth limits
and exceptions are reconstruction safeguards, not recovered crash behavior.
Original name/resource lookup, object transforms, memory allocation,
registration, texture conversion, and animation state remain outside this
serialized-data API. The first actor readers have a separate
[30,000-case native-byte comparison](entity_native_oracle.md); that result
does not extend to the entire loader.
