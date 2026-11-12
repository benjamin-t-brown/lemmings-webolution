# Sprite piece runtime construction and texture registration

## Authority and scope

Original `Lemmings Revolution.exe` SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The accompanying `.asm` records original bytes, complete mapped spans, PE file
offsets and SHA-256 for every executed routine. The Python runner independently
checks whole-file identity and all 32 code spans before execution. No original
binary or asset is stored in the repository.

REA Ghidra metadata and pseudocode, including evidence IDs and provider
limitations, are in `sprite_piece_runtime_metadata.json`. Assembly and native
execution resolve the field interpretation. Automatically assigned library
names such as `Get` do not establish original C++ types or source provenance.

This extends the earlier serialized-only `SpritePiece` reader. It supplies a
separate `RuntimeSpritePiece` projection and does not change that existing API.
The runtime field at **+40 is a signed texture registration index**, not a
decoded texture or resource pointer. No decoder or graphics resource factory
is called by the recovered chain.

## Recovered behavior

| Address | Bytes | Behavior |
| --- | ---: | --- |
| `484E30` | 14 | Constructor returns its input pointer, with no writes. |
| `485CE0` | 269 | Read four words, four doubles, four words and a name; resolve ID into +40. |
| `474519` | 68 | Check whether a name exists; find it again or register it; write the resulting ID. |
| `449B30` | 136 | Search active entries for exact name equality; return whether found. |
| `4607D0` | 118 | Same search, returning the first index or the entry count on a miss. |
| `45FE10` | 312 | Register into the first inactive entry, otherwise append. |
| `4602D0` | 51 | Copy the name, clear cached pointer +80, set low active byte +84 to one. |
| `460330` | 363 | Grow when count reaches capacity, copy entries, release old storage, append. |
| `44B9F0` | 64 | Assign name through its NUL, cached pointer, and only low active byte. |
| `44BDC0` | 32 | Default entry: clear first name byte and low active byte only. |

Additional concrete native helpers implement entry addressing, string copying
and comparison, array construction/destruction, scalar stream reads and bounded
name reads. `44BDE0` and `41A4B0` have no observable destructor effects.
`44B970` releases the array cookie address after those empty element destructors.

The manager used by `474519` is the global object at `58CB48`; the source API
receives its `ResourceRegistration` view explicitly. A name matches only an
entry whose **low byte** at +84 is nonzero. Comparisons are case sensitive.
Inactive matching names are skipped, and duplicate active names return the
first index. The exists check and index search are both performed on a hit.

`45FE10` is a texture specialization of the already recovered `446040`.
Its constructor, assignment, growth and empty destructor operations have the
same observable field behavior. The source composes `register_resource_slot`
instead of duplicating that implementation. The native oracle nevertheless
executes the original **texture-specific** helpers, not the other specialization.

On growth, the old count selects the returned index before callbacks occur.
The original captures the new allocation count before allocation, initializes
the new array, copies using live source count/storage, releases old storage,
sets the new pointer and appends using the live count. Callback modifications
to counts, capacity, storage and input names are reflected at their original
read points. The name has already been copied to a temporary before allocation.

Piece fields are written in stream order. x87 loads and stores quiet signaling
double NaNs while retaining sign/payload; raw integers and other double bits
are preserved. +44 is untouched unless an external callback changes it.
All piece fields are available to allocation callbacks after reading. +40 is
written only after lookup/registration returns, so that final assignment
overwrites a callback's change to +40. Callback changes to other fields survive.

## Verification levels

**Static:** REA pseudocode and byte disassembly establish the routines and
the texture-specific registration specialization. The 132-byte `strcmp` span
includes three alignment bytes omitted from Ghidra's noncontiguous body count;
the ledger and runner pin the complete span explicitly.

**Units:** x64 `-O2` and x86 `-O0` pass constructor preservation, inactive and
duplicate-name selection, case sensitivity, miss behavior, slot reuse,
signaling-NaN quieting, callback mutation order and truncated scalar rejection.

**Native comparison:** x86 `-O0` and `-O2` each pass:

- 32,768 full unchanged `485CE0` reader-to-registration chains;
- 65,536 direct original name searches;
- 32,768 constructor identity/full-object preservation comparisons;
- 11,584 ordered callback snapshots of every piece field, plus registration
  traces, final counts/capacity/storage, all three allocation blocks and cookies.

Only `47EEF7` stream access, `49C1A8` allocation and `49C13D` release are
adapters. Name search, `strcmp`, registration, growth, element construction,
assignment, destruction and stream scalar/string helpers all execute original
bytes. The oracle maps the original manager global and updates it at callback
boundaries; no gameplay or rendering process is launched.

Cases include zero through six initial entries, active flags with preserved
upper bytes, inactive matching names, duplicate active names, null and nonnull
cached pointers, names up to 127 bytes, embedded NULs in 260-byte payloads,
zero/negative/over-capacity name lengths, all pointer alignments supplied by the
fixtures, random numeric bits, signed zeros, infinities, subnormals and NaNs.
Allocation scripts mutate counts/capacity/growth/storage and input names;
release scripts mutate destination data; callbacks also mutate piece fields.
Storage release is recorded but storage remains alive for post-call comparison.

## Domain and remaining work

This is normal-return recovery with masked x87 exceptions, nonnegative valid
entry counts, live NUL-terminated names and successful allocation. New names
must fit the original 128-byte manager entry. Growth and callback mutations
must leave every accessed element live. The original's memory corruption,
allocation failure unwinding and invalid negative-count traversals are not
reproduced. Truncated memory input throws `invalid_argument`; no native
equivalence is claimed for incomplete underlying stream reads.

Actual texture loading, decoding, lifetime management and graphics upload remain
outside these routines. Existing serialized sprite parsing and the runtime
piece reader are separate usable entry points; full resource-wide runtime array
construction is not claimed here.

## Integration

Add `src/sprite_piece_runtime.cpp` to the recovered library. It requires the
existing `resource_registration.cpp` and `level_stream.cpp`. Add unit executable
`tests/test_sprite_piece_runtime.cpp` and x86 oracle
`tools/native_sprite_piece_runtime_oracle.cpp`, with the same three sources or
the library. The oracle links at image base `0x10000000` and runs through
`tools/run_native_sprite_piece_runtime_oracle.py --exe <installed-exe> --program
<oracle>`. Test fixtures reuse the registration fixture to exercise its known
mutation scripts. Shared build files and central evidence indexes are left to
the integrating agent.
