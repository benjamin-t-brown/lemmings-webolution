# Texture factory, constructor, and BMP conversion

## Authority and scope

Authority: installed `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
`texture_creation_metadata.json` records REA identities, procedure extents,
pseudocode, assembly, PE file offsets, exact original bytes, and byte digests.
`texture_creation.asm` is an independent objdump transcription. The native
runner verifies the whole executable, each PE mapping/digest, and string bytes
before loading the original instructions from the installed executable.

| Routine | Bytes | Recovered behavior |
| --- | ---: | --- |
| `4743DD` | 261 | Classify basename prefix, allocate 40 bytes, construct or return null |
| `471720` | 47 | Indexed texture-manager name bridge into factory slot +14 |
| `414D00` / `406500` | 30 / 14 | Indexed 136-byte record and direct name address |
| `47218A` | 705 | Initialization, quality filename probe/fallback, BMP load, optional alpha load, graphics boundary |
| `47244B` | 69 | Case-sensitive `INN_` / `OUT_` substring predicate |
| `472490` | 267 | Decode-buffer transfer, BMP dimensions/stride, GDI allocation, pixel copy, buffer release |
| `450120` / `450138` | 24 / 34 | Buffer pointer initialization and unconditional free |
| `445830` | 29 | File-open forwarding; second argument is a byte |
| `473DA0` / `47C7F0` | 14 / 14 | Identity constructors with no object writes |
| `47C5B0` | 35 | Triple-array pointer initialization, preserving its type word |

The public indexed function composes the concrete factory. Its valid manager
has factory slot +14 bound to `4743DD`, as established by texture vtable
`4B19E8` in `texture_resource_bindings.json`. Arbitrary substituted virtual
factories are outside this convenience function's contract.

## Behavior established

Only backslash separates the basename. Its first byte maps `_`, `$`, `~`, `^`,
and `+` to flags 1, 2, 4, 8, and 16. Classification happens before allocation;
the constructor subsequently reads the current caller name. Allocation failure
returns null without opening files. The constructor preserves bitmap+04 until
the bitmap loader assigns it, bytes+11..13, dimensions until BMP parsing, and
triple type+1C. It clears +00/+08/+0C, byte+10, and triple pointer+20.

Low quality appends `Lo` to every name. High quality appends `Hi` only when
the name contains uppercase `INN_` or `OUT_`. The selected BMP is probed with
open arguments `(1, 0)`. Only the returned low byte decides success. A failed
probe rereads the original caller name and constructs an unsuffixed filename;
it does not close the failed probe. The probe object survives until after
graphics initialization. The alpha branch tests the captured constructor
flags, independent of mutations of resource+24. Its allocation and read sizes
each reread current width/height, with 32-bit multiplication. Alpha filename
uses the selected/fallback name. Failed alpha open reports source file and
line 90, then exits with status zero without normal cleanup.

BMP conversion writes width before height, so aliased output references retain
the original order. Row bytes are the signed arithmetic right shift by three
of `((width * bit_count + 31) & ~31)` using 32-bit wrap, then multiplied by
height modulo 2^32. The pixel source address and copy size are captured before
GDI callbacks. Bitmap+04 is assigned before deleting the DC. The decoded
buffer's live pointer is reread when freeing, including callback mutation.

## Verification

* Static: complete original assembly and REA pseudocode for the thirteen
  routines above; original class/vtable bindings from the previous texture pass.
* Unit: x64 and x86 builds each exercise 29,160 normal paths, 270 fatal paths,
  prefix and case-sensitive quality predicates, and an independently asserted
  padded 24-bit BMP copy.
* Native: x86 `-O0` and `-O2` builds each compare 29,160 complete normal traces,
  resulting resource fields, filenames, copied pixels, shared scratch, and
  caller name against unchanged original routines. Twenty isolated fatal-open
  subprocesses per build compare the complete report/exit trace before exiting
  from the adapter. No exception is unwound through original MSVC frames.
* Cases include all prefix classes, empty names, nested/trailing backslashes,
  forward slash, both quality modes including noncanonical true bytes,
  failed/successful low-byte probe results, null resource/alpha allocation,
  width-height aliasing, 8/16/24/32-bit stride, and a sweep of seventeen
  callback positions plus an unmutated case. Mutations change name, quality, flags, dimensions, alpha
  pointer, decoded-buffer pointer, and scratch bytes/pixel-offset metadata.

The native harness adapts CRT string/memory operations, allocation/free, file
operations, decoded-file loader `45015A`, GDI imports, reporting/exit, and
graphics initialization `472F99`. It executes the original wrapper, factory,
constructor, quality, BMP, and trivial helper instructions without rewriting
their bodies. `45015A` and its codec have separate native evidence in
`texture_resource.md`; this comparison supplies synthetic decoded BMP data.

## Explicit boundaries

This module does not implement the Win32 file class, packed-file I/O, GDI,
DirectDraw/Direct3D initialization `472F99`, rendering, texture destruction,
or original MSVC allocation/exception recovery. All hooks are required; normal
hooks must not throw. Names/paths must fit the original 260-byte stack buffers.
Buffers must be valid, sufficiently sized and satisfy memcpy nonoverlap;
malformed BMP rejection is not added. The source uses native pointers and an
explicit shared scratch pointer in place of the original absolute global.
Callbacks may mutate documented live state; they must not invalidate object,
buffer, or path lifetimes. Concurrent scratch access is not supported.
