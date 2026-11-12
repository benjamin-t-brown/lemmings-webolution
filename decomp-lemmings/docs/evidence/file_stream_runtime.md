# File object runtime primitives

Authority: installed `Lemmings Revolution.exe` SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The child REA session independently verified this identity. Selected full
procedure information and pseudocode are retained in
[file_stream_runtime_metadata.json](file_stream_runtime_metadata.json), with
REA IDs and limitations. Complete inline evidence is retained locally in
`build/file_stream_runtime_full_evidence.json`.
[Original byte disassembly](file_stream_runtime.asm) corroborates field writes,
branches, call arguments and mutation order.

| Routine | VA / file offset | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| Default constructor | `0x47DB2F` / `0x7DB2F` | 68 | `58a31fa3db446f8568be5b3f9c4a825de8b90add559be12f6a30d7c89efb4a58` |
| Named constructor | `0x47DB73` / `0x7DB73` | 127 | `07fa7b1c6b1312d93bd37eafda9db96b451f01d33f272d3bc0624b9f64233232` |
| Filename setter | `0x47E395` / `0x7E395` | 120 | `efb411804ff8b80768f70b471282eb9849d919cb709b4ef9ab03166393c204a5` |
| Empty base constructor | `0x47F040` / `0x7F040` | 14 | `befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c` |
| Close | `0x47E844` / `0x7E844` | 87 | `d0f2429aa3e193d964dc67437a108b2de4dbbb541e2134eea74c8c11dd503446` |
| Block read | `0x47EEF7` / `0x7EEF7` | 70 | `70601e5ef5678f380b7f37bac8241e2962339b38963c2e1c97408c26823acd39` |
| Block-read forwarder | `0x47DAC8` / `0x7DAC8` | 29 | `aec40f1e210009a794b263010b78bce2e4aa490c9c5560c2cd8d25330b64dfc7` |
| Destructor | `0x47DC28` / `0x7DC28` | 72 | `aff14242f418c9727edf3f555fd0daf4b70029be83de56454d342b2f4a47394a` |
| Empty base destructor | `0x47EF3D` / `0x7EF3D` | 11 | `012a7b6ce927f083ba98a546e3d3f750b2a1d82f350ca27efbd30a9c3c7718a5` |
| Size query | `0x47EE27` / `0x7EE27` | 17 | `d500a64daa2531474397cfd9958b6e1b88fc9b0b86d04aa2ea4015b50431bc91` |
| End query | `0x47EE38` / `0x7EE38` | 84 | `f001d4cc9ee000cc449d69d658a34d67bdcc2fe4216db4707487f96ac5c6e44f` |
| Open query | `0x47EE8C` / `0x7EE8C` | 20 | `9a0a6ccfb070e91f4254c606398f78e0d8299c783663935500cb20bd7ba7afd7` |
| Shared-buffer reset | `0x47DAA9` / `0x7DAA9` | 31 | `3911d4ea905910cfff6f69dae9b45866a66d81e313790b703e961dd6100daf17` |

## Static behavior recovered

`FileStreamRuntime` retains the original known fields through `+130` and names
unknown byte regions explicitly. Its original x86 size is `0x134`; its host
layout may differ on x64. Pointer `+118` is an opaque CRT `FILE*` boundary.
Other fields include 260 filename bytes at `+14`, size `+10`, ended byte
`+11C`, archive byte `+11D`, byte cursor `+120`, archive offset/index at
`+124/+128`, opened byte `+12C`, and mode `+130`.

The default constructor `47DB2F` calls an empty base constructor, then writes,
in order: handle null, cursor zero, ended byte one, opened byte zero. It leaves
the name, size, archive metadata, mode and all other bytes untouched. The named
constructor `47DB73` first calls `47E395` to set the name, then makes the same
four field writes. Neither constructor opens a file or frees an existing one.

The filename setter tests global byte `5D5604`. When zero, it copies the input
name directly. Otherwise it copies the global prefix at `5D5628` into a local
260-byte array, appends the input name with no separator insertion, then copies
that local result into the object. Only bytes through the resulting NUL are
written. Exact source/name self-alias works on both paths; partial overlaps and
names exceeding the fixed buffer are outside the contract. No path
normalization, case folding or filesystem lookup is added.

`47EEF7` calls original CRT `49DA82` as `fread(destination, 1, count, handle)`.
It then **rereads the live cursor**, adds the returned byte count with 32-bit
wrap, and returns that count. Its `47DAC8` wrapper forwards arguments and
ignores the result. The game wrapper does not enforce declared file/archive
size, reject a zero request, set the ended byte on short reads, or update the
opened flag. Those behaviors must not be inferred from the field names.

`47E844` does nothing at all if the handle is null. Otherwise it captures the
handle, calls `49DA2C` (`fclose`), ignores its result, and writes handle null,
ended one, cursor zero, opened zero in that order. Other callback mutations
persist. Destructor `47DC28` invokes close then an empty base destructor;
Ghidra's library name for this destructor is a misleading signature match,
not evidence that this game object is an ATL module.

The size query returns the raw 32-bit size. The open query returns the raw
opened byte. The end query `47EE38` treats mode one specially: it returns a
canonical byte 0/1 from `(shared_next == shared_end && ended != 0)`. Other modes
return the raw ended byte, including noncanonical values. The two pointers are
globals `5D55FC` and `5D5600`; reset `47DAA9` writes end first, then next, to
shared buffer base `5CD5FC`.

## Explicit boundaries and unimplemented open path

`FileStreamRuntimeHooks` exposes CRT read and close. It passes the captured
handle, count and destination; the original element size is always one.
Callers provide live destination storage and a valid backend handle for reads.
Read results, callback effects and 32-bit cursor arithmetic remain concrete in
the game wrapper. The original CRT buffering, Win32 handles, file sharing,
filesystem errors and exceptional unwind are not reconstructed here. These
callbacks return normally; no synthetic error handling is added.

`FileStreamPathEnvironment` supplies the two original prefix globals, while
`FileStreamBufferPointers` represents the shared read-buffer pointers. The
source reset takes an explicit base pointer instead of hard-coding a host
address. The native comparison checks the original fixed-address writes.
C++ member defaults are not evidence of original initialization.

**Complete open `47E40D` remains unrecovered.** The already-open positioning
blocks are recovered separately in [file_stream_runtime_open.md](file_stream_runtime_open.md). Its complete
1,039-byte REA body is retained under evidence
`ev_2b432ee4d4eac6ea1a7d34a2a00a9feb3edf1f19d1b7d05e92894a334d33ce9b`.
It combines mode preparation, direct-file existence testing, archive lookup,
required-file message/retry behavior, archive reloads, CRT open/close/tell/seek,
and shared-buffer reset. The archive branch reuses an existing handle when
nonnull; the direct-file branch closes an existing handle before reopening.
Those details preclude replacing the full function with a generic portable
file-open call. Existing `box_archive` index/patch-first lookup remains a
separate recovered component; this slice does not claim their full open-path
composition or safe bounds for arbitrary archive reads.

## Unit and native evidence

Units pass at x64 O0/O2 and x86 O2. They check constructor partial writes,
prefix concatenation with exact self-alias, actual-byte cursor addition after a
callback changes the cursor, unsigned wrap, reads larger than declared size,
null-handle close as a complete no-op, reset ordering despite a failing close
return, callback writes to unrelated fields, and canonical versus raw-byte
query results.

At **both x86 O0 and O2**, the native oracle passes:

- **34,320** full `0x134`-byte object comparisons across constructors, filename
  assignment, read/forwarder, close/destructor, queries and buffer reset.
- **10,000** composed original string-reader comparisons executing original
  `47D940 -> 402540/402560 -> 47DAC8 -> 47EEF7 -> fread`.

The runner verifies the full executable digest and every loaded byte span.
All game routines remain unchanged. It also executes the original CRT string
copy/concatenation instructions from pinned `49C750..49C83F` (240 bytes, SHA-256
`de277c53bfccd0f688de14be2f3a7258615d8c770e4bd3f7cc88a8e29afb91b9`).
Ghidra reports only the entry jump for `49C750`; the extended span includes its
shared tail and the `49C760` concatenation entry, both used by this test.
Only CRT `49DA82` and `49DA2C` are adapted.

The fixture dirties every object byte and covers empty through 259-byte path
results, source alignments, prefix flags 0/1/255, exact name self-alias,
zero/short/full reads, request counts through 64, cursor boundary values,
archive sizes smaller than read requests, null/non-null close handles, all
flag bytes 0/1/128/255, and equal/unequal shared buffer pointers. Four callback
scripts mutate the live cursor, handle, flags, name, size and archive flag.
The comparison includes all object bytes, output bytes/canaries, callback
arguments/order, return values where defined, and shared pointer globals.
Undefined upper return-register bits from byte/void routines are not compared.

The composed string test supplies complete synthetic inputs through a memory
`fread` adapter and compares output, backend position, original object cursor,
and every other object byte with the existing recovered `read_level_string`.
This lowers the earlier native string oracle's adapter boundary from the game
block reader to CRT I/O. It does not prove truncated scalar reads, CRT stream
semantics, actual filesystem access, or the unimplemented open routine.

```powershell
python tools/run_native_file_stream_runtime_oracle.py --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' --program build/x86/native_file_stream_runtime_oracle.exe
python tools/run_native_file_stream_runtime_oracle.py --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' --program build/x86/native_file_stream_runtime_oracle_o2.exe
```
