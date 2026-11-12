# File open orchestration

Authority: installed `Lemmings Revolution.exe` SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The child REA session independently verified that identity. Complete function
identity, reported body and pseudocode are retained in
[file_stream_open_metadata.json](file_stream_open_metadata.json), including
main evidence `ev_2b432ee4d4eac6ea1a7d34a2a00a9feb3edf1f19d1b7d05e92894a334d33ce9b`.
[Original byte disassembly](file_stream_open.asm) records all instructions and
separately pinned switch-table/string bytes. Original instructions, not the
existing portable archive parser, establish the behavior below.

| Routine | VA / file offset | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| Complete file open | `0x47E40D` / `0x7E40D` | 1039 | `bcd189abe77f85f8b72d15c05f6b9f92f55f71645fd21b8f2663d9501a1b2c7a` |
| Archive name accessor used in replay | `0x47F0B0` / `0x7F0B0` | 22 | `6800f39676c99dc7ae1c648bed624d161bdc6dcdb1c69632b6163e4852328140` |

The native runner also checks switch table `47E81C..47E843` (40 bytes) and
string data `4C2A84..4C2AFB` (120 bytes). Existing unchanged helpers are
`47DAA9` (31-byte buffer reset), `406500` (14-byte name data accessor), and the
complete shared CRT copy span `49C750..49C83F` (240 bytes). Pins and literal
expected data are in the runner; all are verified before execution.

## Static byte verification, 2026-10-30 (native-005)

Read-only checks used the repository-local
`Lemmings Revolution/Lemmings Revolution.exe`, relative to the repository root,
and confirmed the full-image SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The existing `pe_file_offset` function in
[`verify_reference.py`](../../tools/verify_reference.py) mapped all four spans:

| Span | VA | PE file offset (decimal / hex) | Bytes | SHA-256 |
| --- | --- | --- | ---: | --- |
| Complete file open | `0x0047E40D` | `517133` / `0x7E40D` | 1039 | `bcd189abe77f85f8b72d15c05f6b9f92f55f71645fd21b8f2663d9501a1b2c7a` |
| Archive name accessor | `0x0047F0B0` | `520368` / `0x7F0B0` | 22 | `6800f39676c99dc7ae1c648bed624d161bdc6dcdb1c69632b6163e4852328140` |
| Switch table | `0x0047E81C` | `518172` / `0x7E81C` | 40 | `2b9a9e9ed3b35715798c084d70af86f56208c851b4eb096b4e8e5329341bfedf` |
| Message, caption and mode strings | `0x004C2A84` | `797316` / `0xC2A84` | 120 | `c8133acb8a7695fa688e980a230ed7a3ce77d0e0620b790a3803d4794ce5fada` |

Both primary spans matched the table above, the complete instruction bytes and
headers in [file_stream_open.asm](file_stream_open.asm), and the literal `PINS`
in [`run_native_file_stream_open_oracle.py`](../../tools/run_native_file_stream_open_oracle.py).
The two data digests were derived from the mapped reference bytes after exact
byte comparison with that runner's literal `CONSTANTS`; the assembly data and
headers independently agreed. The runner was parsed as text only, without
importing or executing it. No discrepancies were found.

The static verifier now appends these two `FUNCTIONS` and two `DATA` entries.
Its existing `0x0047E6CC..0x0047E6F4` (41 bytes) and
`0x0047E770..0x0047E81B` (172 bytes) pins remain unchanged: both are subranges
of the complete `0x0047E40D..0x0047E81B` span. Existing helper pins for
`0x0047DAA9`, `0x00406500` and `0x0049C750` are reused without duplication.
Overlap adds no recovered-function coverage; the `372 / 6,186` baseline is
unchanged. Per-function digests remain optional under recovery `AGENTS.md`;
these additional pins impose no new fidelity or completion requirement.

The registered Python ran the static verifier directly against that local
reference, checking the full image, PE mappings and every existing and new
span. Output and the literal/assembly comparison checks are retained under
ignored `build/native-005-worker/`. This establishes byte identity only, with
no runtime behavior or original-game fidelity established. No original game,
copied original instructions or oracle ran. The behavioral comparisons below
are retained historical evidence and were not repeated by these static checks.

## Static control flow recovered

`open_file_stream` recovers the complete normal-return orchestration of
`47E40D` for its established initialized-mode domain:

| Input mode | CRT text |
| ---: | --- |
| 1 | `rb` |
| 2 | `wb` |
| 3 | `wb+` |
| 6 | `ab` |
| 7 | `ab+` |
| 10 | `wt` |

Other original mode values leave the local mode-text buffer uninitialized.
They are outside the typed API's preconditions; no outcome is invented for them.

The routine first calls `47DD54(mode, nullptr)` for drive/registry preparation.
It then samples global prefix flag `5D5604`. A nonzero flag selects direct-file
opening immediately. This function does not prepend a path; naming/prefix
construction is a separate recovered operation.

When that initial flag is zero, each attempt performs this sequence:

1. If mode is not one, select direct-file opening without an existence check.
   Otherwise call `47E89B` with the current name; a nonzero low byte also selects
   the direct branch. Direct selection clears the archive byte.
2. If the direct file was not found, query `47EF48`. A zero low byte triggers
   `47EF92` before archive lookup `47F759` with the current name.
3. A matching archive record copies size from `+104` to file `+10`, offset from
   `+108` to file `+124`, and archive index from `+00` to file `+128`, then sets
   archive byte one. This is a typed projection of known fields, not an ABI
   layout claim for the intervening name bytes.
4. On an archive miss, optional files do not display a dialog. Required files
   format the original required-file message and call `MessageBoxA` with null
   owner, caption `Lemmings Revolution`, and type `0x2131`. Only result exactly
   one requests another attempt. The archive byte is cleared after the dialog.
5. A retry calls `47E2F3`, `47E23B`, `47EFE6`, then `47EF92`, in that order,
   before another existence/archive attempt. It does not rerun `47DD54` and
   does not resample the prefix flag. There is no retry limit.

If selection fails without retry, the routine returns low byte zero. It does
not close an existing handle or reset size, cursor, mode, ended, or opened
fields; callback changes remain. On selection success, it writes the requested
mode to `+130` and chooses the corresponding CRT mode text.

### Direct and archive handles

The direct branch closes a nonnull existing handle through CRT `49DA2C`,
ignores the result, then clears only the handle. It calls `49DB6A` with the
current filename, captured mode text, and share value `0x20`, and stores the
returned handle. A null result sets ended byte one and returns low byte zero.
It does not clear an old opened flag or reset the cursor on this failure path.
A nonnull handle enters the previously recovered
[direct measure/restore finalizer](file_stream_runtime_open.md).

The archive branch reuses a nonnull handle. If null, it retrieves the selected
archive's name through `47F210`/`47F0B0`, opens it with the same mode text and
share value, and stores the result. It then invokes the
[archive-offset finalizer](file_stream_runtime_open.md), even if that result
is null. There is no explicit null-open check before the archive seek. A real
CRT may reject or fault on that handle; the recovered routine does not invent
successful filesystem behavior for it.

Both finalizers preserve the logical cursor, ignore seek results, clear ended,
reset shared buffer pointers only when the **live** mode is one, set opened,
and return byte one. Direct size/tell/seek sequencing and callback rereads are
concrete reused source. Changes to handle/name during close, archive-name lookup
or open are observed where the original rereads them. Local mode text remains
the initially selected text even if callbacks change the file's mode field.

## Explicit boundaries and preconditions

`FileStreamOpenGlobals` references the existing path and shared-buffer views.
`FileStreamOpenHooks` retains drive/registry preparation (`47DD54`), filesystem
existence (`47E89B`), archive readiness/reset/lookup/reload (`47EF48`, `47EF92`,
`47F759`, `47E2F3`, `47E23B`, `47EFE6`), archive-name resolution, MessageBox,
and CRT open/close/tell/seek as explicit boundaries. Those implementations,
registry access, drive discovery, actual filesystem sharing, archive index
loading and OS errors are not reconstructed by this file.

The archive-name hook combines pointer-slot lookup with the original pure name
accessor. Original `47F0B0` returns the name data at archive `+38` through
`406500`; it has no additional effects. Selected fresh helper dossiers are
retained locally in `build/file_stream_open_helper_evidence.json` as static
context. Their presence is not a claim that every helper body is implemented.

Hooks return normally, preserve live referenced storage, and may mutate the
file and global views. Returned archive-match storage must remain valid and
must not overlap file fields during the three ordered copies. Names must be
terminated and, when the required-file message is formatted, the entire message
must fit the original 260-byte local buffer. Arbitrary partial aliasing,
unsupported modes, overlong message buffers, exception paths, and callbacks
that invalidate live storage are outside the contract. Infinite retry behavior
is intentionally retained when callbacks never select a file or cancel.

## Retained unit and native evidence

Units pass at x64 O0/O2 and x86 O2. They check all six mode strings and share
values, optional misses, exact retry effect order, retry-to-archive, cancellation
after repeated attempts, prefix sampling after preparation, no resampling on
retry, archive metadata transfer, direct-open failure state, and forwarding a
null archive-open result to seek under a controlled CRT adapter.

At **both x86 O0 and O2**, **27,648 complete original function comparisons
pass**. The oracle invokes unchanged `47E40D` at its true entry with its normal
calling convention; no stack-block bridge or branch rewrite is used. The
original switch, retry loop, mode copies, archive-name accessor, finalization
blocks and shared-buffer reset run as original instructions.

Adapters replace only the explicit effect boundaries above. For name retrieval,
`47F210` is adapted while original `47F0B0`/`406500` run. The original message
formatter `49CA6A` is replaced by a bounded host formatter for valid messages;
its format string is verified, and the resulting complete text/caption/type
are compared at the UI boundary. No real dialog, drive preparation, archive
mutation, or file access occurs in this test.

Twelve scenarios cover initial bypass, direct existence, archive lookup with
new or reused handles, optional/required misses, cancellation, one/two/three
retry sequences, direct open failure, unchecked archive open failure, and
non-read modes. Eight mutation scripts change handles, filenames, size, cursor,
mode, archive metadata, flags and shared buffer pointers at callback boundaries.
Tests vary canonical/noncanonical readiness/required bytes, negative tell
values, ignored seek failures and initial existing handles. They compare every
original `0x134` object byte, callback order/arguments and pre-callback state,
low-byte return, prefix flag and shared-buffer pointers. Undefined upper return
register bits are excluded.

The null archive-open scenario demonstrates the unchecked seek argument with
a mock that returns normally; it is not evidence that the original CRT accepts
null streams. This is native control-flow comparison with declared adapters,
not an original game process or a complete native filesystem stack.

```powershell
python tools/run_native_file_stream_open_oracle.py --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' --program build/x86/native_file_stream_open_oracle.exe
python tools/run_native_file_stream_open_oracle.py --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' --program build/x86/native_file_stream_open_oracle_o2.exe
```
