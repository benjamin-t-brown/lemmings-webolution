# Already-open file finalization blocks

Authority: installed executable SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
These are **blocks inside `47E40D`, not complete recovered functions**.
The full enclosing function's REA record
`ev_2b432ee4d4eac6ea1a7d34a2a00a9feb3edf1f19d1b7d05e92894a334d33ce9b`
is retained in [file_stream_runtime_metadata.json](file_stream_runtime_metadata.json).
[Block byte disassembly](file_stream_runtime_open.asm) corroborates arguments,
field rereads, success writes and control flow.

| Block | VA / file offset | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| Archive seek branch | `0x47E6CC` / `0x7E6CC` | 41 | `5788559ea557c7e7290a940732e627d1333c3eed890f4d8edd3bf1d80e008f58` |
| Direct measure/restore and shared success tail | `0x47E770` / `0x7E770` | 172 | `3a69b43387be5f2f815de749e31a157c4b8628068e511caf2bb73a54a8110b49` |

The archive block ends at `47E6F4` with a jump to `47E7E0`. The direct block
covers `47E770..47E81B`, including the shared tail at `47E7E0..47E81B` and the
original function epilogue. Existing reset helper `47DAA9` is also loaded
unchanged, pinned at 31 bytes with SHA-256
`3911d4ea905910cfff6f69dae9b45866a66d81e313790b703e961dd6100daf17`.

## Recovered behavior and entry preconditions

`finish_direct_file_stream_open` begins after the direct file has already been
opened successfully and assigned to object `+118`. It performs:

1. `saved_position = tell(handle)`.
2. `seek(live_handle, 0, SEEK_END)`.
3. `size_10 = tell(live_handle)` with the returned 32-bit pattern preserved.
4. `seek(live_handle, saved_position, SEEK_SET)`.
5. The shared success tail described below.

Each call rereads the live handle from the object; only the first returned
position is captured across later callbacks. The second tell result overwrites
size after tell returns. A size mutation in the final seek callback therefore
persists. Negative tell values, including `-1`, are not checked: the second
value is stored as raw 32-bit size and the first is forwarded as the signed
seek offset. Both seek return values are ignored.

`finish_archive_file_stream_open` begins after an archive handle has already
been selected/opened or reused. It calls
`seek(handle, bit_cast<int32>(archive_offset_124), SEEK_SET)` and goes directly
to the shared tail. It does not measure or overwrite the entry size. It also
ignores the seek result.

The shared tail writes ended byte zero, checks the **live mode** for exact
value one, resets the shared buffer pointers only in that mode, writes opened
byte one, and returns low byte one. It does **not** reset logical byte cursor
`+120`. No source error handling, cursor reset, or archive-bounds check is added.
Other object fields and callback mutations remain intact unless specifically
overwritten by these instructions.

These functions require the caller to have performed the earlier open work
and supplied a valid backend handle. They do not perform mode selection,
filename lookup, archive discovery, file opening, close-before-reopen, required
file dialogs, retries, or failed-open handling. The full `47E40D` orchestration
remains unrecovered. CRT tell/seek and OS state are explicit callbacks, not
reimplemented here. The buffer base argument is the typed representation of
original global base `5CD5FC`.

## Native execution boundary

The native oracle executes the exact original blocks without patching any
instruction inside them. A small declared x86 bridge sets up the enclosing
routine's stack locals: object pointer at `[ebp-128]`, space for captured
position at `[ebp-124]`, and the normal two stack arguments expected by the
original `ret 8`. It jumps to `47E770` for a direct file or `47E6CC` for an
archive. The original block and epilogue perform the return. This is controlled
block replay with explicit entry state, not execution of the full open routine.

Only CRT `49DB9D` (tell) and `49DCF5` (seek) are adapted. The original shared
buffer reset helper runs unchanged. The runner validates the full executable
identity and all loaded byte pins before the bridge runs.

## Verification

Units pass at x64 O0/O2 and x86 O2. They check tell/seek ordering and arguments,
preservation of negative tell values and signed archive-offset bits, ignored
seek failures, cursor/size behavior, final flags, live handle rereads, callback
mutations and mode-dependent global buffer reset.

At **both x86 O0 and O2**, **24,000 comparisons pass**, split equally between
direct and archive blocks. They compare every original `0x134` object byte,
low-byte return, complete tell/seek argument/state traces, and shared buffer
pointers. Eight callback scripts vary handles, cursor, size, mode, ended/opened
flags, archive offsets and shared buffer pointers. Tell returns cover zero,
`-1`, `INT_MIN`, `INT_MAX`, positive positions and other negative values; seek
returns cover success and failure. Initial/callback mode values cover read
mode one and several other modes.

The script mutations are adversarial controlled-boundary checks, not a claim
that the original CRT normally mutates the game file object. This evidence
establishes these two blocks under their declared entry conditions; actual CRT
seek behavior and pre-entry archive/file resolution remain outside the claim.

```powershell
python tools/run_native_file_stream_runtime_open_oracle.py --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' --program build/x86/native_file_stream_runtime_open_oracle.exe
python tools/run_native_file_stream_runtime_open_oracle.py --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' --program build/x86/native_file_stream_runtime_open_oracle_o2.exe
```
