# C1ag: the probe waits for the hook before it clicks

Queue `c1ag-arm-before-click`. Offline. No stock start. Worker [Arm hook before click](0342974a-ff8b-458d-8043-3647af9a37ae). `prediction.md` 17:21:39, before the probe edit. Dispatched 17:19 MDT.

## Result

Inside `enter_level`, after `select_level` and before `aim_at`, the copy waits for `hook-armed.flag`. If the flag is absent when the wait ends, it raises and does not click. `main` calls `enter_level()` and only then writes `ready.json`. The hook is still one attach at `0x473897`. The dry check parses the probe and does not run it.

Menu `RETURN` and `select_level` stay before that wait. The hook's report is still `setTimeout(..., 6000)` from the moment the script loads.

## Facilitator check

Re-ran `python build/tmp/c1ag-arm-before-click/dry_check.py` (exit 0). Read `enter_level`: the wait and the raise at the flag are before `aim_at` and `inp.button(0, True)`. Read `main`: `enter_level()` returns before `ready.json` is written. Replaced only that `raise` with an assignment; the dry check exited 1, `refuse-if-absent is missing before the click`. Restored. No game process. No source change. The suite on this tree was already 34/34.

Narrowing: this is source order. It does not show that `0x473897` runs on the click. The 6000 ms report starts when the script loads, so a later start has to write the arm flag while the attach is still running.

## Stop

Round 5. The counts are unread. One stock start remains, and it can use this order.
