# C1j: remaining named writers for the C1 live-read list (directive part 1a, offline, round 1 of 2)

Queue `c1j-named-writers-read`. Worker packet `build/tmp/c1j-named-writers-read/round-note.md`, `prediction.md`, `verify.py` /
`verify.log`. Dispatched 01:51:34 MDT, packet 02:30, landed 02:36 MDT (worker ran about 39 min against the 30-minute limit,
the overrun spent re-decoding its citations). Capstone only, REA not used, no game, no debugger, no `src/**` or `tools/**` edit.
`G = 0x4C6A08`.

| Item | Tag |
| --- | --- |
| 1 `[[0x4CBFB0]+0x10]` store and feed | PARTLY READ: writers and feed READ, stock 640x480 value UNREAD |
| 2 frame-table loader behind `0x47ABC0` | READ: loader, record layout, file source; which sheet and row per icon UNREAD |
| 3 `[0x4CBF7C]` `obj+8` writer and font | writer READ, font name `_FontSmall` READ; the file it resolves to and the colour UNREAD |
| 4 `0x4E3310/14/18`, `0x4E335C` | READ: registration stores, no copy exists; runtime values UNREAD |

**1.** `0x4CBFB0` = `G+0x55A8`; 49 references in `.text`, all loads. The pointer is stored at `0x45C89E` (object `new(0x18F8)`,
ctor `0x452BC2` sets `+0x10 = 0`). `[obj+0x10]` is written at `0x4450EB` (store 0, reset path) and `0x45D184` (every level frame,
from `[obj+0x14]`). `[obj+0x14]` is written at `0x45700F` (message type 6, `byte[msg+0x1e]==0`), `0x453369` / `0x45337D`
(chosen by flag table `0x5D5850` through `0x455D30` / `0x455CF0`) and the setter `0x455C4F`. So it is a runtime mode field fed by
messages and a flag table, not a constant and not a device query. Live remainder: `[[0x4CBFB0]+0x10]` and `+0x14` during `[G]==2`,
plus `[0x58D53C]` and `[0x58CB38]`.

**2.** `0x47ABC0` indexes `arg*0x48 + [this+0xC]`; filler `0x485CE0` reads a stream: u32s at `+0..+0xC`, doubles at
`+0x10/+0x18/+0x20/+0x28`, u32s at `+0x30/+0x34/+0x38/+0x3C`, string at `+0x40`. That is `SpriteFrame` in
`src/assets/Sprite.h` field for field (`+0x30/+0x34` = F.dx / F.dy, `+0x38/+0x3C` = width / height), so F is derivable from the
`.SPR` file once the sheet and row are known. Live remainder: per icon `+0x1B8`, `+0x1CC`, `+0x1F8`, `+0x1FC`.

**3. Corrects C1h(c).** `[0x4CBF7C]` is `[G+0x5574]`. The level frame function `0x45CFF9` is called once, at `0x450A53` with
`ecx = 0x4C6A08`; it saves `ecx` (`0x45D017`) and loads `[ecx+0x5574]` at `0x45D03E`, then calls `0x4403E5` (`0x45D044`), which
pushes `0x4BFF78` = `_FontSmall` and sets `obj+8` through `0x474519`. C1h's "no read call loads `[0x4CBF7C]`" was wrong: C1h had
the caller in its list and did not make the equation. The `^Font` thunks serve `[0x4CBF78]`. Live remainder: object `+4`, `+8`,
`+0xC` and the resource `_FontSmall` resolves to.

**4.** `0x4E3308..0x4E331C` are `G+0x1C900..0x1C914` themselves, set by one registration function: `0x45E775` Bits ->
`G+0x1C900`; `0x45E78D` GreyIcons -> `0x4E3310`; `0x45E7A5` BlackIcons -> `0x4E3314`; `0x45E7BD` AdditiveIcons -> `0x4E3318`;
`0x45E7D5` Axe; `0x45E7ED` Smoke. The Grey / Black / Additive mapping moves from INFERRED to READ. `0x4E335C[0..3]` = `G+0x1C954`,
set to -1 at `0x45E756`, stored at `0x44387F`, reset at `0x44388E` / `0x441601`.

## Live-read list after this round
1. `[[0x4CBFB0]+0x10]`, `+0x14` (in level, `[G]==2`), `[0x58D53C]`, `[0x58CB38]`.
2. Per icon object (`0x4C91D8 + 0x1150 + i*0x208`): `+0x1B8`, `+0x1CC`, `+0x1F8`, `+0x1FC`, frame count.
3. `[0x4E3310]`, `[0x4E3314]`, `[0x4E3318]`, `[0x4E335C..0x4E3368]`.
4. `[0x4CBF7C]` object (`+4`, `+8`, `+0xC`) and what `_FontSmall` resolves to.
Unread offline (round 2 bodies if taken): registrar `0x446040` -> `0x449170` (name to file); `0x474519` -> `0x449B30` / `0x45FE10` /
`0x4607D0`; consumers of flag table `0x5D5850`; indirect callers of `0x45E51F`.

## Facilitator check
Re-decoded with `read_steppers.py --at`: `0x45D184`, `0x4450EB`, `0x45D03E`, `0x45D044`, `0x45D017`, `0x450A4E`, `0x450A53`,
`0x44033A`, `0x4403F3`, `0x440427`, `0x45E78D`, `0x45E7A5`, `0x44387F`, `0x485D79`. Read the strings in file bytes: `Bits`,
`GreyIcons`, `BlackIcons`, `AdditiveIcons` at `0x4C0D3C/44/50/5C`, `_FontSmall` at `0x4BFF78`. Independent raw `.text` scans
(fresh script, `build/tmp/c1j-named-writers-read/fac_scan.py`): `0x4CBFB0` has 49 references and none is store-shaped; `0x4E335C` has 13;
`0x4CBF7C` has 4 (`0x450930`, `0x45A813`, `0x45B611`, `0x486AA8`), matching the worker. **Narrowed:** the `SpriteFrame` match is on
field offsets and widths from the filler's reads, not a decode of a real `.SPR` file; the `0x45CFF9` single-caller argument is over
direct calls only (the worker said indirect callers are not excluded); "message type 6 branch" and the flag-table feed were taken from
the worker's decodes and not re-traced here. Offline round: no mutation applies.
