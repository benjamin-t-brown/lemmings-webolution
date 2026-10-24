# C1ad: paused plate blend, selector 1

Queue `c1ad-plate-blend-live`. One stock start. Worker [Log live plate blend](1497895c-cb69-443e-98eb-aec1c3fec31a). `prediction.md` 16:47:57, `Popen` 16:48:18. REA was the Frida attach only. Dispatched 16:44 MDT.

## Result

One `Popen`, pid 3996, session 1. Paused `Compiled\MM_CLIMB_002.lvl`. The level-table slot was restored. The attach exited 0 with `cleanup_error` null. The probe's wait did not see `hook-done.flag`; the flag was written after the attach returned, and the probe then closed pid 3996.

`C1AB_HOOK`: 8650 plate hits at each site, caller `0x45B66F`, out of 49305 calls. The 40 stored records are four copies of plates 0–9. Quads are `0x4C9208 + index*0x1B0`. Every record has descriptor `0x2627270`, flags 8, selector 1, blend cache `0x10620C1F`. The flag cache is 8 on all 40. The descriptor cache is `0x2673150` on each index 0 and `0x2627270` on indices 1–9. `[0x4C24E0]` is 160 (`0xA0`) on both snapshots.

Index 0 fails the descriptor-cache compare and meets the flag-cache compare. Indices 1–9 meet the descriptor-cache compare. Selector 1 differs from the blend cache on all 40, so the blend-enable arm is not skipped. The calls after those compares were not sampled. The stage-state writes that sit past the flag compare were not shown to run. `[0x4C24E0]` was not shown to be stored.

Exe, `SavedGame.dat`, and `dgVoodoo.conf` hashes match the pre-launch values. No game process and no `LemWeb-*` task afterwards.

## Facilitator check

Parsed `hook-output.txt` independently: 40 and 40 records, caller `0x45B66F`, quads match `0x4C9208 + i*0x1B0`, the four frames repeat the index-0 descriptor-cache miss, flags equal the flag cache, selector 1 differs from `0x10620C1F`. Both `readings.json` copies of `4C24E0` are 160. `run.log` has one `Popen:` line. The copied `hook.js` matches the dry-checked script byte for byte. Re-hashed the three install files; they match `cleanup-report.json`, including the restored level-table slot. No `*Lemming*` process and no `LemWeb-*` task. No source change. The suite on this tree was already 34/34.

Narrowing: this is one paused start. Records after the first 40 are unread. The hook samples before the calls, so a cache miss says the arm is not skipped; it does not show the `SetRenderState` arguments executing. Selector 1 is the landed nonzero arm from C1z (enable 1, source 5, destination 6). That mapping was not re-derived here.

## Stop

Round 1 of 3 on the live blend question. One start was spent. The mask dword's writer is a separate round and was not part of this start.
