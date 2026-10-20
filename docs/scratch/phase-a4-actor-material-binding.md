# A4s: retained actor selector bound; material flags still unresolved

Queue `a4s-actor-material-binding`; offline research only. Authority SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Worker packet: `build/tmp/a4s-actor-material-binding/round-note.md` and
`binding-table.json`. This note narrows that packet as described below.

## Accepted result

For the two retained A4f fixtures, the recorded controller first dword **is the
submit-object blend selector**, and both values are **0**. The selected direction
records carry resource handles **119** (five) and **120** (eight). These are
retained live observations joined to an independently read static identity chain,
not a new runtime observation.

Selector 0 takes A4r's **blend-disabled encoded branch**, provided the selector
cache misses. It does not assign SRCBLEND or DESTBLEND on that branch. This is not
proof of effective device state, and does not yet establish the actor's alpha-test
or texture-alpha operation. In particular, it does not justify changing the
candidate raster's alpha behavior without further evidence.

## Identity and binding evidence

Source observations are in
`build/tmp/a4f-actor-billboard/material-live-2/report.json`:

| JSON pointer suffix under `/pairs` | five (`/0`) | eight (`/1`) |
| --- | --- | --- |
| `/actor_row/actor` | `0x27e3c50` | `0x27e3c50` |
| `/actor_row/blend_selector_word` | 0 | 0 |
| `/actor_row/frame_record` | `0x27ce5cc` | `0x27bab9c` |
| `/actor_row/frame_bytes_0x48`, u32 at byte `0x40` | 119 | 120 |
| `/suppression/flag_before` | `00` | `00` |

The capture reader `tools/harness/capture_actor_background_a4f.py:67–83`
reads that selector at `actor+0x128`, and its direction record at
`[temporal+0xC] + direction*0x48`. Independent verification also matches each
record's first 64-byte SHA-256 and action/temporal/direction to the accepted A4f
`resource-match.json` packet. The numeric handles are session-local, not retail
file constants.

Original-byte chain:

- `0x42DEC4 83c178` passes `actor+0x78` to constructor `0x43CE00`.
  `0x43CE3C c700a8144b00` installs vtable `0x4B14A8`; its `+4` dword is
  `68cc4300` (`0x43CC68`).
- `0x42F088 8a918c030000` reads `actor+0x38C`, and `0x42F090 7438`
  takes the zero path to `0x42F0DC ff5204`, the draw-subobject vcall at `+4`.
  Both retained suppression flags are zero before suppression.
- That draw method adds `0xB0` (`0x43CD48 81c1b0000000`) and calls
  `0x47A75D` (`0x43CD4E e80ada0300`). Its submit object is therefore
  `actor+0x78+0xB0 = actor+0x128`.
- `0x47A763 894dec` preserves this pointer. The frame lookup at
  `0x47A7A2 e859e5feff`, `0x47A7A9 e832e5feff`, and
  `0x47A7B0 e80b040000` uses action/temporal/direction and helpers with strides
  `0x98`, `0x10`, `0x48`; the record is retained at `[ebp-4]`.
- `0x47AB7C 8b4dec` reloads the same submit pointer before
  `0x47AB7F e812000000` calls `0x47AB96`. There,
  `0x47ABA0 8b4840` reads the direction record's resource handle and
  `0x47ABA7 e88460fcff` calls its setter. The store is
  `0x440C3D 8988a8010000`, into submit `+0x1A8`.
- `0x47ABAF e86c9df9ff` then calls `0x414920` with that same submit object.
  `0x41492D 8b88a8010000` reads its resource handle;
  `0x41494C 8b02` reads its first dword and
  `0x41494F e82c040000` passes it to `0x414D80`.
- On selector-cache miss, `0x414D9F 0f8496000000` sends selector zero to
  `0x414E3B 6a006a1b` (value 0, render state 27); the call at
  `0x414E4E ff5058` uses the Device3 layout interpretation established in
  [A4r](phase-a4-actor-render-state.md). Cache equality returns without assignment.

## Unresolved flags, and corrections to the worker packet

`0x414920` resolves the resource handle through `0x414A10` / `0x414B90`
before passing the resulting descriptor to `0x47261F`. That descriptor's `+0x24`
flags were not recovered from the checked retained packets, so **no row of A4r's
material/alpha table is selected yet**.

The resolver's entry pool is reached through **manager+0x30**, whose internal
array pointer is at `+0x0C`; its stride multiply is at **`0x414D0A`**, not
`0x414D00`. The cached `entry+0x80` points to a wrapper; the return path
dereferences its first dword to get the material (`0x414C34..0x414C3A`). Lazy
creation uses `0x414E98 ff520c`, a virtual factory call.

The manager at `0x58CB48` is beyond file-backed `.data` (raw end `0x4C6000`).
That establishes only that its runtime value is not a stored file byte. It
**does not prove static provenance is exhausted or that the flags can only be
learned live**. The packet's stronger runtime-only conclusion is not accepted.
The claims about the scaled alternative lacking all binding are also unnecessary
to this result and are not promoted.

Next bounded proposal: a correlated read-only observation from submit object and
handle to resolved descriptor and `+0x24` flags for these two fixture kinds. An
uncorrelated hit on the general material updater would not establish actor
ownership. A static producer continuation is another option; neither is run here.

## Facilitator check (2026-10-19)

Wrote a separate offline fixture join and stock PE reader,
`build/tmp/a4s-actor-material-binding/facilitator_check.py`; run with `--summary`
for compact output. It checks the full authority digest, independently joins the
two retained direction records and selectors, and retains original bytes and
Capstone disassembly in `facilitator-verification.json`. Reread the identity,
frame lookup, resource setter, dispatch, selector branch, and resolver windows
above. Also reran the existing independent A4r checker: **34 setter tuples,
21 byte anchors, 3 SDK offsets and 20 enums pass**. The worker's 31-check count
is not used as the independent acceptance gate.

Build reported no work; the complete CTest suite passed **20/20** through
`scripts/Invoke-Ucrt64.ps1`. No production behavior changed, so source/pixel
mutation gates are not applicable; no new pixel acceptance is claimed. No game
was started or attached, no live writes or owned instrumentation need restoration,
and an independent process query found no Lemmings or `webolution` process. The
first process-query attempt had shell expansion errors; the correctly quoted
repeat completed with no matches. Existing unrelated framework edits remain
untouched. A2 wider/cavity/rim, A3 sensitivity, actor raster residual and app
integration remain open.
