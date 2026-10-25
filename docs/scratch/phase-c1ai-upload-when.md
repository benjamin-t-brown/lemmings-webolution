# C1ai: ^tab is registered at startup; the upload phase is unnamed

Queue `c1ai-upload-when`. Offline. No stock start. Worker [Trace ^tab upload callers](9d876d5f-a943-42a1-85d8-9e91988b7ea5). `prediction.md` 18:21:35 expected the frontend. REA was used for callers. Dispatched 18:20 MDT.

## Result

Seventeen direct `E8` calls target `0x45FE10`. Sixteen are in `0x45E51F`, including `0x45E901` (`^tab`, push `0x4C0E00`) and `0x45E919` after the push at `0x45E90F` (`^tab_selected`, `0x4C0E08`). The seventeenth is `0x47453C` in `0x474519`, and its phase is unnamed.

`0x45E51F` has one direct caller, `0x450895`, inside `0x450696`. That function has one direct caller, `0x49D5E1`, inside the PE entry point `0x49D518`. Both arms of `0x49D5C9` reach that call. No jump in `0x450696` before `0x450895` targets an address after it. Those sixteen registrations run at process startup.

`0x45FE10` does not call `0x472F99`. Its calls are direct, to `0x449930`, `0x414D00`, `0x4602D0`, `0x460330`, `0x44BDE0`, and `0x44B9F0`. The only direct call of `0x472F99` is `0x47241A`, inside `0x47218A`. The only direct call of `0x47218A` is `0x4744B1`, inside `0x4743DD`. There is no `E8` to `0x4743DD` and no `.text` immediate of that address. `0x4B19FC` holds `0x4743DD`, slot `+0x14` of vtable `0x4B19E8`. The upload's phase is unnamed.

## Facilitator check

An independent `.text` scan found the same seventeen calls, the same single callers of `0x45E51F`, `0x450696`, `0x472F99`, and `0x47218A`, and no `E8` to `0x4743DD`. `AddressOfEntryPoint` is `0x49D518`. Re-read `0x45E901`, `0x45E90F`, `0x45E919`, `0x4C0E00`, `0x4C0E08`, `0x450895`, `0x49D5E1`, `0x47241A`, `0x4744B1`, `0x4B19FC`, and `0x47123A`. The calls inside `0x45FE10` match the six targets above. Expecting `e8 0a 15 00 01` at `0x45E901` failed; the byte is `e8 0a 15 00 00`. The exe was not written. No source change, so the suite was not rebuilt. No Lemmings process.

Narrowing: `0x474392` is a second `mov [eax], 0x4B19E8`, another constructor, not a call of the upload. The startup path registers the name. It does not show that `0x472F99` runs then.

## Stop

Alpha-count round 1 of 3 this run. The upload is still unplaced in time, so no stock start. The next read is who calls vtable slot `0x4B19FC`.
