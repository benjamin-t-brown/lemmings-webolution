# Phase A6b: SPR temporal-index render clock

Authority: `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Brief `build/tmp/a6b-actor-anim-clock-brief.md`; repair
`build/tmp/a6b-actor-anim-clock-repair1.md`. Packets under
`build/tmp/a6b-actor-anim-clock/` (`round-note.md`, `round-note-repair1.md`,
`prediction.md`, `prediction-repair1.md`, `live/native_series.json`). Cursor
`round-agent` on `composer-2.5-fast`, dispatch `3ef46072-db85-41f1-a297-ca06ef77a406`.
Two rounds on this question (initial plus one repair).

## Result

- **READ:** `0x468833` (controller = actor+0x128) loads the action-strip rate at
  `[strip+0x10]` (`0x46887F`/`0x468885`), does `acc[+0x1C0] += param_1 * rate`
  (`0x46889E`), then while `acc > 0.06` (`0x4688C6` fcomp `[0x4B1940]`,
  `test ah,0x41` / jne exits unless greater) increments `+0x1CC` (`0x4688E0`),
  wraps against `0x45B8F0()`, subtracts `0.06` (`0x468972`), and zeroes the
  accumulator when `<= 9.99999993922529e-09` (`0x4B1928` at `0x46898A`). Remainder
  is carried, not reset.
- **READ:** `+0x201` set wraps the index to 0 (`0x468928`); clear holds it at
  count-1 (`0x468953`). That flag is `loopAtEnd`, not ping-pong.
- **READ:** strip bind `0x468AB8` writes `+0x1C8` then clears `+0x1CC` and the
  `+0x1C0`/`+0x1C4` double (`0x468ADD`..`0x468AF4`).
- **MEASURED:** 45 samples on one `MM_CLIMB_002` walker (actor `0x2863630`,
  vtable `0x4B10E8`, controller `0x2863758`, action 13, count 12, rate 1.0).
  The elapsed implied by consecutive `(acc, index)` pairs is an **integer number
  of milliseconds** on all 44 intervals (6–9 ms, max 2.5e-6 off integer). A
  forward replay from frame 0, feeding those integer-ms deltas and never
  re-syncing, matches every later index and the accumulator within 1e-6.
- **Falsified:** the first packet's ~0.2 s sim-tick / 2–3 steps per sample.
  This clock is a render-frame clock, not `Lemming::frame`.
- **Unsettled:** the producer of the **outer** seconds budget that
  `0x41BDD4` slices. Immediate `param_1` is `min(remaining budget, time to next
  integer X at speed 5.0)`.

## Facilitator check

2026-10-20, Cursor facilitator.

- **Re-read:** `0x468833`..`0x4689AE`, `.rdata 0x4B1940` = 0.06,
  `0x4B1928` dust, `0x468AB8` reset, `0x43CF30` thunk (`add ecx,0xB0` then
  `call 0x468833`), `0x41BDD4`..`0x41BF5E`, `0x41BF61`..`0x41BFF9`,
  `0x4B10B0` = 5.0.
- **Narrowed time source:** `0x41BF61` is **time-to-next-cell**
  (`(ceil/floor(X) − X) / 5.0`), not the millisecond origin. `0x41BDD4` takes
  `min(that, *[arg+0xc])` and pushes that double to `[lemming+0x78]+0x2C` →
  `0x43CF30`. The `fld 5.0 / fmul / fimul facing / fadd +8` at `0x41BE70` is
  the walk **position** update, not the clock input. No `.text` `E8`/`E9` to
  `0x41BDD4`; the worker's `0x42EEF2 → 0x41BDD4` line is **not accepted**
  (`0x42EEF2` is `Lemming::Render`, A4af).
- **Independent replay:** recomputed `kElapsedMs` from
  `live/native_series.json` (exact match to the embedded table) and forwarded
  from frame 0 with integer-ms deltas: 0 index mismatches, accumulator within
  1e-6.
- **Isolated mutations** on `ActorAnim.cpp` (`838fd66b…`): remainder reset
  fails 45 checks; omit `* rate` fails the rate-2.0 unit check. Restored
  hash-exact; fresh full build **22/22** CTest.
- **First packet rejected:** circular replay (delta from native acc/index,
  re-sync every frame); fixture in gitignored `build/tmp`; `pingPong` name;
  unnamed `param_1`. No `dgVoodoo.conf` swap. Repair used no second live start.
- **Live side:** no game process, no `LemWeb-*` task. Install hashes equal the
  first-packet before set.
- **Scope:** `ActorAnim` is compiled into `webolution` but has no app caller
  yet. Integration is A6d. This series never wraps; wrap is READ-only. Rate ≠ 1
  and action-change were not live-sampled.

## For A6d

Pass **seconds of consumed step budget** (the same quantity `0x41BDD4` hands
to `0x468833`), not `Lemming::frame` and not a 200 ms tick. Until the outer
budget writer is named, a measured render-frame dt in seconds is the honest
stand-in. Settle sim → `+0x08` (A6a) and this dt source together in A6d.
