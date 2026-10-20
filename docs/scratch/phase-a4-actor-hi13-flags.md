# A4w: `$LemSheetHi13` material flags are `0x2`

Queue `a4w-hi13-material-flags`. Live, read-only round on `MM_CLIMB_002`: one owned game
start, session-1 task `LemWeb-a4w-run1`, process suspend/resume only, no debugger. The
only memory write is the established `enter_fast.py` level-record swap used to enter the
level (`capture-run1/enter-log.json`, `slot_restored: true`); the measurement made none. Worker packet: `build/tmp/a4w-hi13-material-flags/round-note.md`. The
prediction `prediction.md` was written before the launch (00:46:28 against 00:48:08).
Raw report: `capture-run1/report.json`.

## Result (MEASURED, live, suspended, two identical reads)

- Actor `0x2752300` (action 7, temporal 0, direction 2), submit at `actor+0x128`, handle
  `[submit+0x1A8]` = 119, equal to the direction record `+0x40`.
- Pool base `[0x58CB84]` = `0x0FF19464`, count `0x8B` (139). Entry `0x0FF1D39C` =
  base + 119·0x88; name `Sheets\$LemSheetHi13`; wrapper `[entry+0x80]` = `0x02741470`;
  descriptor `[wrapper]` = `0x02741490`; **`[descriptor+0x24]` = `02 00 00 00`**.
- Selector `[submit]` = 0; `[0x4C24E0]` = `a0 00 00 00`; flags cache `[0x4C24E4]` = 0.
- Full pool snapshots at level entry and at end, each coherent: all 24
  `$LemSheetHi0..23` entries carry `0x2`. Handles are session-local (here Hi13 = 119,
  Hi15 = 117, Hi12 = 120).

So A4u's row 3 for the five fixture's sheet is now measured, not assumed.

## Scope limits

- The bound actor was action 7 / temporal 0 / direction 2, not the five fixture's
  13 / 8 / 1. The flags belong to the descriptor, so they hold for any actor on Hi13;
  A4u's attribution of that frame to Hi13 was not rechecked live.
- Encoded flags only: effective device state at draw time is not read, the same limit as
  A4t. Only `MM_CLIMB_002`.

## Facilitator check (2026-10-20)

- **Independent recompute** (`build/tmp/a4w-hi13-material-flags/facilitator/frecompute.py`),
  from raw bytes only. It checks:
  - submit = actor + 0x128;
  - handle = record `+0x40` = 119 < count 139;
  - base from manager bytes `+0x3C`;
  - entry address, name, wrapper and descriptor addresses each match the next link;
  - flags at descriptor + 0x24 = `0x2`;
  - both pool snapshots coherent, 139 entries, 24 `LemSheetHi` entries all `0x2`.
- `writes` empty; `task_session` 1; the exe, `SavedGame.dat` and `dgVoodoo.conf` hashes
  match before and after (exe `32675bdb…`).
- **Live side, my own separate SSH call:** 0 Lemmings processes, 0 `LemWeb-*` tasks.
