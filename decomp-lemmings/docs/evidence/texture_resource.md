# Packed texture BMP recovery

## Authority and exact scope

Reference executable: installed Windows `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Archives and executable are read in place. No binary assets or decoded images
are written into the repository.

| Address | Bytes | Recovered scope |
| --- | ---: | --- |
| `0x467DE8` | 43 | Decoder wrapper's normal returned output length |
| `0x467E13` | 164 | Three-table setup, literal runs and overlapping backreferences |
| `0x467EB7` | 310 | Valid Huffman code-length table interpretation |
| `0x467FED` | 135 | Huffman/unary integer decoding |
| `0x45015A` | 845 | In-memory bitmap envelope transformation after file read/close; file lifecycle and original fatal handling excluded from portable API |

The five routine pins, PE offsets and SHA-256 values are in
[the native runner](../../tools/run_native_texture_resource_oracle.py).
[Original bytes and disassembly](texture_resource.asm) and
[REA metadata](texture_resource_metadata.json) retain both recovered routines
and the additional factory/graphics routines inspected to establish context.
The block decoder dossier is
`ev_b9aa88e316ebb54c79a3bfced31425cf4efd9eead11eb7a0b1094522e56c0a97`;
table builder is
`ev_a1ce9cfa3593244a32a26309546fea9fee9b15f5946e55c40663819208ab247c`;
container loader is
`ev_7f08944382514d0416a9360208bd417c58309ad9aa69ac196208aec0065d8081`.

## Observed texture factory route

The texture resolver `0x474519` operates on manager `0x58CB48`. Constructor
`0x46F14D` places this manager at engine-object offset `+8`, calls `0x474380`,
which installs vtable `0x4B19E8` after base construction. The relevant slots are:

| Slot | Target | Static observation |
| --- | --- | --- |
| `+0x0C` | `0x471720` | Registration index to name, then virtual factory |
| `+0x10` | `0x4744E2` | Null-tolerant texture destruction |
| `+0x14` | `0x4743DD` | Concrete texture factory, 261 bytes |

[Vtable bytes and digest](texture_resource_bindings.json) pin those targets.
Factory classification uses the first basename character following the last
backslash: `_`, `$`, `~`, `^`, `+` select flags 1, 2, 4, 8, 16 respectively;
other characters select zero. It allocates 40 bytes and calls constructor
`0x47218A` (705 bytes).

That constructor selects optional `Lo`/`Hi` filename suffixes, probes a BMP,
and falls back to the unsuffixed name. `Hi` selection tests the substrings
`INN_` or `OUT_`. Bitmap paths are `Textures\%s.bmp`. Flags `0x08` / `0x10`
also load a `Textures\%s.alf` byte plane. Bitmap conversion `0x472490` calls
`0x45015A`, copies its resulting BMP, creates a GDI DIB section, and copies
pixel rows. The later `0x472F99` graphics routine is 2,949 bytes.

These factory, suffix, alpha-plane, GDI and graphics details are **static
observations only** in this slice. They are not implemented or claimed as
native-verified texture-object lifecycle behavior.

## Packed bitmap format and decoder

`decode_texture_bitmap` returns BMP file bytes. An input beginning with `BM`
passes through unchanged. Otherwise the first byte is the block count, followed
by records:

| Bytes | Meaning |
| --- | --- |
| 2, little endian | Packed block size, including this five-byte header |
| 2, little endian | Decoded block size |
| 1 | Codec: 3 for raw bytes, 4 for compressed bytes |
| Packed size minus 5 | Payload |

The original first sums all decoded lengths, then produces the concatenated
output. It ignores trailing input beyond the declared block count. Codec 3
copies payload bytes; the supported valid domain requires payload length to
equal decoded length. Codec 4 uses the recovered block decoder.

Each compressed block begins with three integer-code tables, for literal-run
length, match length minus two, and match distance minus one. A zero byte
selects the unary-prefixed integer mode. Otherwise the low nibble plus two is
the number of symbols (3 through 17), the high nibble is the first code length,
and later lengths occupy low then high nibbles of successive bytes. Zero-length
symbols are absent. The original builds a compact binary tree; the port uses
equivalent canonical codes ordered by length and symbol index. Bits are read
most significant first.

Huffman symbols zero and one are direct values. Larger symbols select a
leading one followed by `symbol-1` extra bits. Unary mode counts leading ones
until zero, reads that many value bits after an implicit leading one, and
subtracts one. Supported unary widths are at most 15, avoiding the original
16-bit accumulator's wrap domain.

Decoding alternates a literal run with a backreference. Literals are eight-bit
values from the same bitstream. A match copies `decoded_length+2` bytes from
`decoded_distance+1` bytes behind the current output. Copying proceeds forward
one byte at a time, preserving overlapping repetition. Decoding stops when the
declared output size is reached. The wrapper returns the written byte count;
the bitmap loader checks it against the block's declared decoded length.

## Supported domain and explicit differences

This is a portable, bounded decoder for valid streams. It requires positive
codec-4 output sizes, valid non-oversubscribed code tables, sufficient input,
valid backward distances, and output runs within the declared size. Invalid
tables, unsupported codes, truncation, mismatched raw lengths, and out-of-range
copies throw `std::invalid_argument`. Heap exhaustion can throw `std::bad_alloc`.
Original malformed-input memory accesses, global scratch-state corruption,
16-bit integer-wrap encodings, exception unwinding, and fatal report/exit
behavior are not emulated. The original table builder's exact invalid-table
acceptance is not claimed. The port uses per-call state instead of the original
shared scratch globals near `0x4C0E94` and `0x4C1190`.

File I/O, heap callback order, graphics allocation, BMP palette interpretation,
GPU upload, and ALF application are excluded. Returning BMP bytes preserves
their original headers, palettes, row padding and pixel payloads without
inventing a rendering conversion.

## Verification

### Static reading and unit tests

Assembly establishes the nonstandard wrapper ABI: four cdecl stack arguments
are input pointer, output pointer, an unused third argument, and decoded size.
Ghidra's inferred fastcall signature is inaccurate; the native oracle follows
the actual stack loads. The canonical-code interpretation is derived from the
length/symbol iteration and tested against the original tree builder.

x64 and x86 units pass 2,048 literal/backreference/Huffman cases, plus envelope,
passthrough, ignored trailing data, empty-container and malformed-input cases.
The unit tests generate known literal bytes, overlapping single-byte matches,
and a fixed canonical-table `ABABAB` example independently of the decoder.

### Native comparisons

Both `-O0` and `-O2` builds compare the original four decoder routines without
any adapters inside the decoder. The native process also executes the **entire
unchanged 845-byte `0x45015A` loader**, adapting only these dependencies:

| Address | Adapter |
| --- | --- |
| `0x47DB73` | Opaque file constructor |
| `0x47E40D` | Successful file open |
| `0x47EE27` | Input byte length |
| `0x43B5B0` | Read the supplied archive member bytes |
| `0x47E844` | File close |
| `0x47DC28` | File destructor |
| `0x49C1A8` | Allocation |
| `0x49C13D` | Free, retaining storage for comparison |
| `0x49D010` | Memory copy |

The runner validates executable identity and all five code spans, then streams
installed archive bytes in memory. It checks exact decoded bytes and sizes,
plus decoder output canaries. File adapters enforce constructor/open/size/read/
close/destructor order. Original decoder scratch globals remain writable;
executable code is protected against writes after setup.

Installed data coverage includes both archives independently, including patch
duplicates: **10,671 BMPs**, all packed; **10,935 codec-4 blocks** produce
335,154,530 bytes, and **one codec-3 block** produces 3,126 bytes. Complete
envelopes produce **335,157,656 decoded bytes**. The probe verifies that every
result has a BMP signature and at least the basic 54-byte header.

Native synthetic coverage adds 256 unary literal blocks, 256 overlapping-match
blocks, 396 canonical-table cases spanning 3 through 17 symbols, plain-BMP
passthrough, an empty container and a two-raw-block container with trailing
bytes. Total native gates per optimization level are **11,843 codec blocks /
335,236,052 decoded bytes** and **10,674 complete envelopes / 335,157,668
decoded bytes**. Every byte matches the original. The parent installed-resource
gate separately applies patch precedence; its smaller unique-resource totals
are expected.
