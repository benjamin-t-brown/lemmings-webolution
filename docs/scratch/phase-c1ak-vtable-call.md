# C1ak: the upload slot is called at 0x471746, and that phase is unnamed

Queue `c1ak-vtable-call`. Offline. No stock start. Worker [Find upload vtable callers](cc938003-86a4-48a9-9162-f797659769d7). `prediction.md` 18:34:20, before the reads, expected unnamed. REA was used. Dispatched 18:33 MDT.

## Result

`0x471746` is `ff 52 14` (`call dword ptr [edx+0x14]`) inside `0x471720`. `0x471741` loads `edx` from `[this]`. Nothing in this function stores `[this]` before that load. `0x471720` is the dword at `0x4B19F4` and at `0x4B1A0C`. On vtable `0x4B19E8`, `[edx+0x14]` is `0x4B19FC`, which holds `0x4743DD`. On vtable `0x4B1A00`, the same instruction calls `0x49C134`.

There is no `E8` to `0x471720` or to `0x4743DD`. The bytes `20 17 47 00` occur only at those two slots. The bytes `dd 43 47 00` occur only at `0x4B19FC`. No immediate of `0x4B19FC` is in the file-backed image. The phase is unnamed.

`0x471230` stores `0x4B19E8` and then calls `0x471400`, which stores `0x4B1A00` over it before that constructor returns. `0x474380` stores `0x4B19E8` and returns. Its only direct caller is `0x46F16F`.

## Facilitator check

Re-read `0x471720` through `0x47174C`, `0x47123A`, `0x47141F`, and the dwords at `0x4B19F4`, `0x4B19FC`, `0x4B1A0C`, and `0x4B1A14`. An independent scan of the file-backed image found the same two vtable slots, the same two stores of `0x4B19E8`, no `E8` to `0x4743DD` or `0x471720`, and one `E8` to `0x474380` at `0x46F16F`. Expecting `ff 52 15` at `0x471746` failed; the bytes are `ff 52 14`. The exe was not written. No source change, so the suite was not rebuilt. No Lemmings process.

Narrowing: the call reaches the upload only when the receiver's vtable is still `0x4B19E8`. The chain from `0x46F16F` to a phase was not re-walked. The shift counts stay unread.

## Stop

Alpha-count round 2 of 3 this run. The upload is still unplaced in time, so no stock start. The next read follows `0x46F16F`.
