# C1ah: the armed click did not call the shift

Queue `c1ah-shift-count-armed`. The last stock start in the pool. Worker [Read armed shift counts](9674eeba-b0a3-4ce2-8696-c6bc1267ad98). `prediction.md` 17:32:28, `Popen` 17:32:40. REA was the Frida attach only. Dispatched 17:27 MDT.

## Result

One `Popen`, pid 42720, session 1. The probe waited for `hook-armed.flag` at 17:33:10. The flag was present at 17:33:38. The click was 17:33:39, and `ready.json` was written after it. `C1AE_HOOK` is `hits` 0, `distinct` 0, `triples` []. The attach was in place for the click and for the following report. `0x473897` did not run in that window.

`select_level` is logged at 17:33:10, before the arm wait. Menu `RETURN` is earlier still. Install hashes match. The process is gone.

## Facilitator check

Read `hook-live.json`: `hits` 0, `triples` []. `run.log` has one `Popen:` line. The arm wait at 17:33:10 precedes the click at 17:33:39, and `ready.json` follows the click. Re-hashed the exe, `SavedGame.dat`, and `dgVoodoo.conf`; they match `cleanup-report.json`. No `*Lemming*` process. No source change. The suite on this tree was already 34/34.

Narrowing: the click is not the call. The steps before the arm wait were not hooked.

## Stop

Round 6. This was start 3 of 3. The counts are unread. Another read needs a start, and the pool is spent.
