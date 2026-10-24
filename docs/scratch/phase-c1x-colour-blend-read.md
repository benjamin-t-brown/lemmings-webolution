# C1x: 0x47218A is the texture-loader entry; 0x46D5F3 is an indirect call (directive part 2, round 2 of 2)

Queue `c1x-colour-blend-read`. Offline. No stock start. Dispatched 2026-10-24 14:17 MDT. Worker [Colour key and blend](29fdd4af-8a0e-454a-a176-1568fe09f87a). `prediction.md` 14:18:28, note 14:24:42. REA was used for callers. The bytes below were re-read with `read_steppers.py`.

## Result

`0x47218A` is `push ebp`, then `mov ebp, esp`. It is the entry of the texture loader. It is not a colour-key test. The plate draw does not call it. `0x4744B1` is `call 0x47218A`. How `0x4743DD` is entered was not re-derived.

`0x46D5F3` is `call dword ptr [ecx+0x70]`. `ecx` is `[[0x58CDB8]]`. Immediately before it: `push 0x1C4`, `push 6`, then the vtable load and `push [0x58CDB8]`. This instruction is not a blend-state write. The method body is unread, so no blend mode is named. `[0x58CDB8]` and `[0x58D540]` are past the file image (`0x4C6000`); their values were not read.

The plate reaches that call only through a gated path. `0x4149F8 mov ecx, 0x5D6170` / `0x4149FD call 0x46D3C1`. Four `call 0x46DEC0` / `test eax,eax` pairs in `0x46D3C1` (`0x46D3E1`, `0x46D403`, `0x46D427`, `0x46D449`) jump to the epilogue at `0x46D45C` when `eax` is zero. The fourth nonzero path is `0x46D457 call 0x46D460`, which contains `0x46D5F3`. What those four `eax` values mean is unread, so a plate draw is not shown to take the call.

`^tab` pixels were not decoded. The loader body was not re-walked for a colour-key test.

## Facilitator check

Re-read `0x47218A` (`55`), `0x4744B1 call 0x47218A`, `0x46D5C4` through `0x46D604` (`push 0x1C4`, `push 6`, `call [ecx+0x70]`), `0x46D3D4` through `0x46D45F` (four zero-skips and `call 0x46D460`), and `0x4149F8` / `0x4149FD call 0x46D3C1`.

Not re-derived: the six live stack slots after the two helpers, the 28 unresolved calls in `0x47261F`, callers of `0x4743DD`, and the loader body. No source change. No game process. The suite was 34/34 on the unchanged tree before this round.

## Stop

Part 2 is at its cap of two rounds. The render still lacks the `^tab` pixels, a colour-key test, and a blend mode. Part 3 was not started. No further stock start was asked for.
