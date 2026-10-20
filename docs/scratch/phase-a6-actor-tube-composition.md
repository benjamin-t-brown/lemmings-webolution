# Phase A6c: actors after all tube passes; no executable Z-state writes

Authority: `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Brief `build/tmp/a6c-actor-tube-composition-brief.md`. Packet
`build/tmp/a6c-actor-tube-composition/` (`prediction.md`, `round-note.md`,
`verify_a6c_actor_tube_composition.py`). Cursor `round-agent` on
`composer-2.5-fast`, dispatch `05b267bb-fbac-4037-a4a0-f036d68c025a`. One
static round.

## Result

- **READ:** inside `0x43A4EC`, the four A4d tube passes finish first — INN
  `0x4622FD` at `0x43A754`, OUT right `0x462404` at `0x43A7F4`, OUT left
  `0x4623A5` at `0x43A87C`, OUT centre `0x462465` at `0x43A8FA` — then
  `call 0x470FAF(0)` at `0x43A908`, then the `this+0x1E88` list walk
  (`0x43A90D`..`0x43AA4F`). No tube-pass `E8` appears after `0x43A908`.
- **READ:** the walk starts at `count-1` (`0x43A91B`) and decrements
  (`0x43A926`) until index `< 0`. No sort between `0x43A908` and the loop.
- **READ:** `fcomp [elem+0x18], 1.0` at `0x43A953` (`dc1dd0024b00`),
  `test ah,1` / `jne 0x43A9C3`. C0 set means **less than**, so
  **`[elem+0x18] >= 1.0` → `0x43A9BB`** (column path, gated on `+0x28`);
  **`[elem+0x18] < 1.0` → `0x43AA47`** (range path, gated on `+0x29`).
  That is the A4ae mapping. The worker's note reversed it; the worker's
  own `prediction.md` had the polarity right.
- **READ:** no `.text` `call [reg+0x58]` whose last immediate is D3D7
  ZENABLE/ZWRITEENABLE/ZFUNC/ZBIAS (7/14/23/47). Between OUT centre and
  the first actor draw there is only `0x470FAF` (stage ADDRESS, A4aa).
- **Unsettled:** per-pixel actor Z versus tube Z; wrapper Z defaults (A1).
  Indirect `ff 52 10` targets are the A4ae vtable pool, not lifted here.

## Facilitator check

2026-10-20, Cursor facilitator.

- Re-read `0x43A754`, `0x43A7F4`, `0x43A87C`, `0x43A8FA`, `0x43A908`,
  `0x43A90D`..`0x43AA62`, `0x43A6C0`/`0x43A760` (`[0x58D2B0]` ± `0.0003`
  at `0x4B1460`).
- Independent scan: zero tube-pass calls in `[0x43A908, 0x43AA63)`; zero
  whole-`.text` `SetRenderState` last-push hits for states 7/14/23/47.
- Re-ran `verify_a6c_actor_tube_composition.py`: `sha256_ok`, 0 problems,
  `actor_z_count=0`, `text_z_count=0`, 13 actor-path `+0x58` sites. Its
  `notes.branch_mapping_read` string is **wrong** (keeps the reversed
  polarity); the pin bytes are correct.
- Prediction was written before the loop-body read. No live start, no
  install writes, no `src/**` edits. 22/22 CTest unchanged (this round
  touched no production sources).
