# C1l: live read of the skill-panel icon inputs, the entry worked, items 1-4 read, hook not run (directive part 2, the one stock start)

Queue `c1l-icon-live-read`. Worker packet `build/tmp/c1l-icon-live-read/round-note.md`, `prediction.md`, `run.log`, `readings.json`,
`raw-bytes.json`, `cleanup-report.json`. Dispatched 02:37:31 MDT, packet 02:41, landed 02:47 MDT. Stock starts used: **1** (Popen 02:38:56,
pid 36828, session-1 task `LemWeb-c1l-1`, no retry). No debugger, no Frida: the worker had no REA / Frida tool, so the hook did not run.

## Entry (MEASURED)

The entry that failed twice **succeeded**. `prediction.md` (mtime 02:38:28) precedes the Popen line. The 15 s idle snapshot, no key
sent: screen 3, `ui_state` 3 at all 15 samples; shim `key_frame` 541 -> 2558 (about 144 polls a second); foreground pid = game pid
36828 at every sample. So the menu is interactive from the first sample, the game polls keys and owns the foreground. The scan then
needed all three presses of attempt 1 (kept 36, 7, 2 -> `menu-index candidates: 2`), reached screen 5, clicked, paused
(`ui 1`, `Compiled\MM_CLIMB_002.lvl`) and restored the slot. **The cause of c3j's and c1i's `no menu highlight` is still not named.**
The new log rules out "menu not interactive" and "game not polling keys" for this run; one success does not show what failed before,
and it does not show the c3m retry was the fix (the first attempt already found candidates). Do not claim a fix.

The probe then raised at the screenshot (`No module named 'PIL'` on mouv) after `readings.json` and `raw-bytes.json` were written;
`r2_after_hook` was never read and there is no `panel-paused.png`. `ok:false` in `cleanup-report.json` is that error only.

## Readings (MEASURED, one process, paused level start, `levelScreen` = 0x2975aa8)

1. **`[levelScreen+0x10]` = 0.** With C1h's formula `S = [0x58D53C] - [0x58CB38]` the generic icons (0,1,3,4,5,6,7; `[0x58CB38]` = 1) are
   halved and icons 2, 8, 9 are native. The scale outcome is derived from C1h, not measured (`[0x58CB38]` is transient and was not read);
   the value 0 is measured at the paused start only.
2. **Per icon** (`+0x1B0` and `+0x1B4` are 0 for all ten). `+0x1B8 / +0x1C8 / +0x1CC`, ctx `+0x1F8`, `+0x1FC`, and F
   (`dx, dy, +0x38, +0x3C`, texture) are in `readings.json`; frame records F, for example icon 0: `+0x1B8` 2, F `0x284e494`, 8,7,23,30,
   texture 0x81; icon 6: F `0x285a314`, 32,2,47,49, texture 0x7d. Icons 0,1,3-7 share ctx `0x2846060`; icons 2, 8, 9 have their own ctx
   (`0x2814af0`, `0x2814920`, `0x28140c0`). Texture ids 0x74..0x86 for the seven generic icons, 0x19 / 0x1a / 0x1b for icons 2 / 8 / 9.
3. **Sheet handles.** `0x4E335C` = {6, -1, -1, -1}; icons 0,1,3-7 have `+0x1FC` = 6. `0x4E3308/10/14/18` = {0, 1, 2, 3}; icons 2, 8, 9 have
   `+0x1FC` = 1, 2, 3. So `+0x1FC` is the registrar handle; GreyIcons / BlackIcons / AdditiveIcons = 1 / 2 / 3 (C1j's registration stores,
   now matched by live values). Which sheet name handle 6 stands for (`Lemmings` plus `Hi` or `Lo`) is **UNREAD**: the handle-to-file
   step `0x446040` -> `0x449170` is not read. Counts `[0x4C91D8 + i*4]` = 10,0,0,0,0,0,0,0,0,1.
4. **Count font objects.** `[0x4CBF7C]` = 0x2817530: dwords `0x4b15c8, 0x4bfc10, 0x25, 0x30`, so `+8` = 37, a live handle, and `+0xC` =
   0x30, the byte `0x4403EC` pushes in the `_FontSmall` thunk. `[0x4CBF78]` = 0x2817550: `0x4b15c8, 0x4bf968, 0x24, 0x20` -> `+8` = 36.
   `+4` points to a `.data` table (first dword 14 and 29), not the name string `0x4BFF78`. The font file behind handles 37 / 36 and the
   colour are **UNREAD**. The ten count text objects hold a pointer at `+0xC` and x / y at `+0x14` / `+0x18` (66; 20, 60, ... 340), not the
   digits, so the pre-run prediction that digits sit in the raw bytes is falsified; the text sits behind the pointer, not read.
   The probe's readings keys `4CBF7C_obj_handle(+8)` and `4CBF78_obj_handle(+8)` are mislabelled (they read the neighbours `0x4CBF84` /
   `0x4CBF80`); use `4CBF7C_as_ptr` / `4CBF78_as_ptr`.
5. **Hook triples: UNREAD.** The start was spent without the hook; C1g's raw args (39,37,38,...; 40,92,121,...) stay READ only.

## Cleanup (worker, then my separate check)

Process closed; no `LemWeb-*` task left; `dgVoodoo.conf` swapped and restored; level-table slot `0x4E35D4` (280 bytes) written and
restored (`restored: true`); no other game-memory write. Install file hashes before equal after:
exe `32675bdb...`, `SavedGame.dat` `f7f86039...`, `dgVoodoo.conf` `d639e220...`.

## Facilitator check

From my own ssh call after the report: no Lemmings process, no `LemWeb` task, `certutil` SHA-256 of the three install files equals
`cleanup-report.json`'s `hashes_before`. `prediction.md` mtime 02:38:28 precedes `Popen` 02:38:56. A fresh script
(`build/tmp/c1l-icon-live-read/fac_check.py`) re-derived `B = [A+0xC] + [icon+0x1CC]*0x10` and `F = [B+0xC] + [icon+0x1B8]*0x48` for all ten icons
from the saved raw A and B blocks: 0 mismatches. I read `0x4BFF78` = `_FontSmall` and the first bytes of the `+4` tables `0x4BFC10` and
`0x4BF968` in the exe. The probe differs from c1k's only in the hook wait (170 s -> 3 s). Not re-derived: the count-object decode and the UV
double decode (the worker's reading of `raw-bytes.json`). **Narrowed:** the worker called the `+4` mismatch UNSETTLED; I record it as "points
to a table, not the name", which is what the bytes show. The hook not running is a tool gap, not a finding.

## What the render still lacks

Not everything: the sheet file behind handle 6 (and the Hi / Lo suffix), the font file and colour behind handles 37 / 36, the count text
behind the `+0xC` pointers, and the hook's on-screen x, y. The first two are offline reads (`0x446040` -> `0x449170`, the `0x474519` chain).
Q3 (render) is the next directive's.
