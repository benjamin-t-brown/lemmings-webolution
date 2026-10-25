# C1as: the remaining start exited with 0xC0000005

Queue `c1as-launch-read`. One stock start. Worker [Launch exit-code probe](23bfa9be-4181-4f87-b04b-8dafb7248176). REA was not used. Dispatched 23:36 MDT. The probe was not run again.

## Result

Pid 44772 resumed at 23:38:48 and Frida reported `process-terminated` at 23:38:50, before `MM_CLIMB_002`. `exit_code` is 3221225477 (`0xC0000005`). `detach_crash` is null. `hits.log` has 46 lines: upload 9, format 28, mask 9, count site 0. `skip_input` is 1 eight times and 2 once. Each value AND `0x18` is 0. `hashes_match` is true.

## Facilitator check

Recounted `build/tmp/c1as-launch-read/hits.log`: 46 lines, sites 9 / 28 / 9 / 0, masks `0, 0, 0, 0, 0, 0, 0, 0, 32768`, `skip_input` `1, 1, 1, 1, 1, 1, 1, 1, 2`. All nine AND `0x18` are 0. Texture fields match c1aq. `run.log` has one `spawned suspended` line. `state.json` and `cleanup-report.json` both store `exit_code` 3221225477. The four copied files match `build/tmp/c1ar-exit-capture/` byte for byte. `hook.js` and `probe.py` were last written at 23:32 and 23:35, before the 23:38 spawn.

Re-hashed the exe, `SavedGame.dat` and `dgVoodoo.conf`. All three equal the cleanup report. No Lemmings or Frida process. No source change, so the suite was not rebuilt.

Narrowing: `detach_crash` null is not a Frida crash record. The exit code is still `0xC0000005`. The fault address is unread. `skip_input` AND `0x18` is 0, so `0x4737E6` skips `0x47383C` on this startup path. The paused-level shift counts stay unread.
