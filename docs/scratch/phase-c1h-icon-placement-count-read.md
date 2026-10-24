# C1h: icon scale mode, sheet-variant budget loop, count font object (Q1 round 2 of 2, offline)

Queue `c1h-icon-placement-count-read`. Worker packet `build/tmp/c1h-icon-placement-count-read/round-note.md`
and `prediction.md`. Offline, capstone only (REA not used), no game, no debugger, no `src/**` change.
Dispatched 2026-10-24 01:00:48 MDT, ready 01:26, landed 01:28 MDT (limit 30 min met).

## Result

**(a) Placement, partly READ.** `0x47A75D(icon, x, y)` takes the frame record `F` from `0x468D00 -> 0x468CE0 -> 0x47ABC0`
(`0x47A798..0x47A7B5`) and sets `y1 = y - F.dy` (`+0x34`, `0x47A7BB..0x47A7C1`). Mode `S = [0x58D53C] - [0x58CB38]`
(`0x47A7CD..0x47A7D8`) scales the frame offsets `F.dx` (`+0x30`), `F.dy`, `F+0x38`, `F+0x3c`: **halved at S == -1,
native at S == 0, doubled at S >= 1** (constant `0x4B03F8` = 2.0, bytes read). Byte `[icon+0x1b4]` picks the mirror
block (`x - F.dx`). `[0x58D53C]` is stored from `[[0x4CBFB0]+0x10]` at `0x44186B` (and zeroed at `0x4419A0`);
`[0x58CB38]` is 0 for icons 2, 8, 9 and 1 for the other seven (`0x45B725..0x45B743`). **UNREAD:** the value of
`[[0x4CBFB0]+0x10]` (it decides native versus half scale for the seven generic icons) and the sheet-supplied `F`
values, so no on-screen rectangle is derived. The "640 versus other width" guess is not supported; it is a 2x scale mode.

**(b) Variant, mostly READ.** The `0x4E335C` fill is the loader `0x4432C1..0x4438B8`: presence flags per prefix,
size table `0x4BFF88` (stride 16; prefix 0 `{0xB34BE,0xD | 0x163510,0x18}`, others `{0xB012E,0xD | 0x15EA10,0x18}`, bytes
read), every present prefix starts at suffix index 1 (`Hi`, `0x443560`), and the loop `0x443602..0x443733` steps
prefixes 3, 2, 1, 0 down to index 0 (`Lo`) while the size sum exceeds a budget from `0x47099B` (a device query,
caps arg `0x1000`). The panel takes the lowest-index present prefix. So the suffix is `Hi` unless the budget test
fails; the budget value on this machine and the source of the presence flags are UNREAD. `0x4E3310/14/18` have one
read each and no absolute store; `0x45E6F0..0x45E7DB` registers `Bits`, `GreyIcons`, `BlackIcons`,
`AdditiveIcons`, `Axe` into `G+0x1C900..0x1C914`. That `0x4E3310` = GreyIcons, `0x4E3314` = BlackIcons,
`0x4E3318` = AdditiveIcons is INFERRED, not read (the copy into `0x4E33xx` is UNREAD).

**(c) Count font and colour, not answered.** `[0x4CBF7C]` is a 16-byte object stored once (`0x450930`, handle
`+8 = -1`), separate from `[0x4CBF78]`. The `^Font` / `_FontSmall` loader thunks (`0x4403A7`, `0x4403C6`,
`0x4403E5`) have callers only for `[0x4CBF78]` (`0x486C4E`, `0x486C70`) and `[G+0x5574]` (`0x45D044`). No read call
loads `[0x4CBF7C]`; its font file and the colour are UNREAD. The hypothesis "same font as C2g's HUD" is not supported.

## UNREAD, named

The store to `[[0x4CBFB0]+0x10]`; the sheet frame-table loader (`F` values); the `obj+8` writer for `[0x4CBF7C]`;
the budget value and flag source; the `0x4E33xx` copy; `0x4840F3`; rest of `0x4404FA`.

## Facilitator check

Re-decoded with my own capstone install and `read_steppers.py --at`: `0x47A7BB`, `0x47A7CD`, `0x44186B`,
`0x45B725`, `0x441845`, `0x4419A0`, `0x450930`, `0x443498..0x4437A7` (loop). Parsed from file bytes: `0x4B03F8`
= 2.0, `0x4BFF88` size table. My own `.text` scan: `0x4CBF7C` has exactly four absolute references
(`0x450932`, `0x45A815`, `0x45B613`, `0x486AAA`); the three font-loader thunks have exactly the callers the
worker named. **Narrowed:** the step-down order is read at the shape level (init to `Hi`, per-prefix sums,
`jle` exits); I did not trace each iteration, so "3, 2, 1, 0" is the worker's reading of it. "Budget is video
memory" is inference. The worker's "no read call loads `[0x4CBF7C]`" is a negative over absolute references and
direct calls, not proof that nothing writes its handle through a register-held pointer. Offline round: no
mutation applies, no `src/**` diff, no game process.
