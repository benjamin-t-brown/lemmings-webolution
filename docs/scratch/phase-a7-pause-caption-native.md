# A7c — native pause-state/key map; caption draw owner unresolved

2026-10-20, `decompile`. Explicit fourth-dispatch exception for a new static
evidence source after A7a/A7b/packet repair. **Bounded negative, not a caption
implementation or gate pass.** No source/permanent harness change, target launch,
live observation, original installation write or commit.

## Verdict

The worker inspected the known pause-state path and a defined string key. It did
**not** find the paused-caption draw path. Resource identity, layout at 640×480,
alpha/blend/coverage and ownership of B/U pixels all remain unknown. No native-
backed reconstruction test or error savings are supported. A7's actual score
stays 0.0220410125; A7b's ideal band correction remains only a bound.

## Independently checked READ observations

Authority: `C:/progs/Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`, PE32 x86,
image base 0x400000. Raw `.data` ends at VA **0x4C6000**; no BSS byte is claimed.

| VA | Original bytes | Supported conclusion |
| --- | --- | --- |
| `0x45D226` | `c7822c57000001000000` | stores dword1 at `[edx+0x572C]`, consistent with the previously established pause-state correction; not a caption draw |
| `0x45D230` | `68e80b4c00` | pushes pointer0x4C0BE8 |
| `0x4C0BE8` | `70617573650a00` | ASCII `pause\n\0`; nearby literal use does not establish display ownership |
| `0x45D4FE` | `85c0742a` | `test eax,eax; je 0x45D52C` |
| `0x45D508` | `c7812c57000000000000` | stores dword0 at `[ecx+0x572C]`; state reset, not a draw finding |
| `0x4C3C18` | `5354525f50415553454400` | ASCII `STR_PAUSED\0`, a key-like identifier, not confirmed localized display content |
| `0x4C35E4` | `183c4c00243c4c00383c4c004c3c4c00` | first **dword at offset0** points to0x4C3C18; table semantics and consumer unknown |

For these pins VA-to-file offset is VA−0x400000, independently derived from PE
section headers rather than assumed as a format-wide rule. All cited bytes above
were re-read. The state writes are corroboration of established facts; the string
key/pointer are new bounded READ observations, not a new caption ownership claim.

## Provider observations and negative-search scope

Ghidra resolved interior selector0x45D200 to procedure **0x45CFF9**, with analyzed
inclusive body `[0x45CFF9,0x45E486]`, 5,262 bytes. The worker inspected **one**
native function, below its maximum of12; it did not examine a whole draw graph.
Native value-flow is truncated, so no missing-path or whole-function semantic
absence proof is accepted.

The authenticated retained inventory reports one case-insensitive defined-string
match for `Paused`: `STR_PAUSED`. Its returned direct xref is the data pointer at
0x4C35E4. Returned exact xrefs to that pointer and interior0x45D200 are empty.
Those empty exact-address/provider projections do **not** establish that no
indexed/indirect consumer exists, that the surrounding table has no consumer,
or that the caption is not drawn. No archive resource was selected or decoded.

## Facilitator check

Worker session `ses_ea34e52f3ffeZncQ64LRINGY0Z` completed on the configured targeted
pin/high; local message metadata records the expected provider/model. Model
mapping remains in `docs/ROLES.md`.

The facilitator independently:

- imported/authenticated the exported REA bundle (**21** records) with
  `rea_import_evidence_bundle`, then checked its retained procedure view without
  reopening native analysis;
- parsed PE headers separately with `struct`, verified authority digest/raw bounds
  and all **four** worker pin windows, and read the additional cited `pause\n`
  literal at0x4C0BE8;
- re-read/disassembled both code contexts with `read_steppers.py` and a separate
  Capstone check, respecting the aligned0x45D4FE instruction boundary;
- independently rejected an isolated in-memory pin-expectation change, restored
  the expected manifest in memory and rechecked. The on-disk manifest remains
  SHA-256 `5d06218ae3cbb037e1ec0e9bad88d08cb5aa45d95e1dbb3bb5c08b00f3430fec`;
- rebuilt (no work) and ran fresh **24/24 CTest** passes. No production mutation
  applies because no behavior was changed.

**Narrowings/corrections:** the worker pin interpretation says the key pointer is
at `+0x0C`; bytes show it is the **first dword, +0**. The candidate note's byte-read
evidence table also mislabels the pointer-data read as an address-mapping record;
the correct pointer-data read is
`ev_4d6fcbaa58e567cdc80d2cd9b312d50b551b101a579d18f04adbe46e92026ab9`.
Neither transcription error changes bytes or the unresolved owner verdict.
These metadata/prose claims are not promoted as written. No extra research or
fifth repair was dispatched to turn this negative into a positive.

Key retained evidence:

- function dossier:
  `ev_af5f8a44ea69ca70cb44d0cf907c30c873635eff64307585531950ed4bae420d`;
- key original bytes:
  `ev_8b62baa979c957040d507b6e553b7ae75c67adb059dabbd1434e772f74982019`;
- key direct xref:
  `ev_8bf32ee74280d8048d81c381ad15fa29648be4234021c910dbc0bc1d7d9f3be6`.

Worker-owned Ghidra was closed. Independent `rea_current_document` returns
`target_unavailable`; importing retained evidence did not reopen a target.
Final process/task and lint/diff checks are recorded in the queue. Existing
retractions, image failures and the missing A7 masks are unchanged.

## Reproduction, remaining evidence and cap

Worker packet: `build/tmp/a7c-pause-caption-native/round-note.md`, `pins.json`,
`check_pins.py`, prediction and `rea-evidence-bundle.json`.
Independent packet: `build/tmp/a7c-facilitator/verify.py`, `verification.json`.

```text
python build/tmp/a7c-facilitator/verify.py
python tools/harness/read_steppers.py --at 0x0045D220 --len 0x40
python tools/harness/read_steppers.py --at 0x0045D4FE --len 0x32
```

The missing bridge is from an established in-level **draw-loop** entry and its
pause-state selection to a text/resource consumer with layout and coverage.
The pause event/state handler and an exact key xref are not that bridge.
Question count is **4** including the justified static exception. Re-park the
cause; no automatic fifth dispatch, image-fit fix, repeated event-handler search
or live allowance. Resume only with new ownership evidence or a separately
justified, concretely bounded draw-loop route. The updated parked outline is
`build/tmp/a7-lower-band-native-resume-brief.md`.
