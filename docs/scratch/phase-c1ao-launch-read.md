# C1ao: the launch-time start exited before the level

Queue `c1ao-launch-read`. One stock start. Worker [Launch-time alpha read](2170e4fe-5f23-4593-87b7-8f977c8ce2aa). REA was not used. Dispatched 21:43 MDT.

## Result

`frida.spawn` pid 20612 at 21:43:52, `hook.js` loaded while suspended, `frida.resume` at 21:43:53. At 21:43:58 the process was gone, before `MM_CLIMB_002`. `hits.log` has 46 lines from that window: `0x472F99` 9, `0x473B1E` 28, `0x4736B6` 9, `0x473897` 0. The first eight uploads each logged one format record and mask 0. The ninth logged 20 format records and then mask 32768. No `(mask, sar, shl)` triple. Every upload name is null. Install hashes match. The probe was not run again.

## Facilitator check

Recounted `build/tmp/c1ao-launch-read/hits.log`: 46 lines, sites 9 / 28 / 9 / 0, masks `0, 0, 0, 0, 0, 0, 0, 0, 32768`. Each `record32` is 64 hex characters. Re-hashed the exe, `SavedGame.dat` and `dgVoodoo.conf`; all three equal the cleanup report's before and after hashes, and `hashes_match` is true. `run.log` shows one spawn. No Lemmings process remains. No source change, so the suite was not rebuilt.

Narrowing: this is the startup window, not a paused level. The non-zero mask was logged and the `sar` was not. No exit code was captured, so the early exit is not named as an access violation. The shift counts at the level stay unread.

## Stop

Round 3 of 3. Start 1 of 2. The same probe is not authorized to run again. The gate was not scored.
