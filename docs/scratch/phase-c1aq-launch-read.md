# C1aq: the repaired probe exited before the level

Queue `c1aq-launch-read`. One stock start. Worker [Launch repaired alpha probe](49561111-d92c-4f30-9879-eb0ce22f4d4f). REA was not used. Dispatched 23:18 MDT.

## Result

Pid 25964 resumed at 23:20:21 and Frida reported `process-terminated` at 23:20:23, before `MM_CLIMB_002`. `detach_crash` is null. `hits.log` has 46 lines: upload 9, format 28, mask 9, count site 0. Masks are 0 eight times, then 32768. No `(mask, sar, shl)` triple. `hashes_match` is true. The probe was not run again.

## Facilitator check

Recounted `build/tmp/c1aq-launch-read/hits.log`: 46 lines, sites 9 / 28 / 9 / 0, masks `0, 0, 0, 0, 0, 0, 0, 0, 32768`. The nine `texture` fields are `_btn_x`, `_btn_cir`, `_btn_squ`, `_btn_tri`, `_btn_up`, `_btn_down`, `_btn_left`, `_btn_right`, `Sheets\$LemBits0`. Every `name` is null. `run.log` has one `spawned suspended` line. The four copied files match `build/tmp/c1ap-hook-repair/` byte for byte. `hook.js` SHA-256 is `9f4ea08963bc33c9bad6350ac3d9a0a5d31bb5d9ffb6e4c0841a6cc9d7936e51`, the same file restored after the c1ap mutation.

Re-hashed the exe, `SavedGame.dat` and `dgVoodoo.conf`. All three equal `hashes_before` and `hashes_after`. `hashes_match` is true. No Lemmings or Frida process. No source change, so the suite was not rebuilt.

Narrowing: `exit_code` is null. `state.json` has `exit_code_error` 87, and `run.log` says `GetExitCodeProcess open failed 87` at 23:20:26, three seconds after the detach. That 87 is `OpenProcess` failing. The cleanup report does not repeat it. `detach_crash` null is not an access-violation record. The count site can also be skipped by `0x4737E6` when `[object+0x24] & 0x18` is 0, so these 0 hits do not by themselves name a crash at `0x47383C`. The paused-level shift counts stay unread.
