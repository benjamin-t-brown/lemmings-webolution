# Phase A6d14: `0x42F0DC` is `0x43CC68`, not `0x41BDD4` (READ)

Brief `build/tmp/a6d14-walk-budget-ptr-brief.md`. Packet
`build/tmp/a6d14-walk-budget-ptr/`. Cursor `round-agent` on
`composer-2.5-fast`, dispatch `548925e2-7903-46f0-bc8d-259639706a92`.
Offline. No `ActorAnim` hook.

## Result

- **READ:** `0x42DEC4` constructs `lemming+0x78` via `0x43CE00`, which
  stores vtable **`0x4B14A8`**. Slot +4 at **`0x4B14AC`** is
  **`0x43CC68`**. `0x42F0DC` therefore does **not** enter `0x41BDD4`.
- **READ:** `0x43CC68` loads **`[this+0xC8]`**, subtracts **`0.0003`**
  (`0x4B1498`), and calls `0x43CDA0` (`ecx = this+0xB0`).
- **READ:** `0x41BDD4` is slot +4 on vtable **`0x4B0D28`**
  (`0x4B0D2C`). Sim `0x42E2BF` copies the caller double into
  `[ebp-0x1C]` at `0x42E308`, loads **`[lemming+0x340]`** at
  `0x42E32B`, and pushes `LEA [ebp-0x1C]` at `0x42E3CF` before
  `call [vtable+4]`.
- **Unsettled:** the incoming double to `0x426414` / `0x42E2BF`, and
  the writer of render `[this+0xC8]`.

Prediction that `0x42F0DC` slot +4 is `0x41BDD4` is **falsified**.

## Facilitator check

2026-10-20, Cursor facilitator.

- Re-read `0x42DEC4`/`0x43CE3C`, `0x4B14AC` (`68 cc 43 00`),
  `0x43CC94`–`0x43CCCA`, `0x42E308`–`0x42E331`, `0x42E3CF`–`0x42E3DF`,
  `0x41BD72`, `0x4B0D2C`. `0x4B1498` file bytes unpack to **0.0003**.
- **Narrowed:** `0x43CE00`’s `this` is `lemming+0x78`, not the lemming
  root. Slot +4 is still `0x43CC68` on that object.
- **Narrowed:** `0x42F8B9` as the +0x340 attach was not re-read. The
  consume path uses `[lemming+0x340]` at `0x42E32B`.
- No production change. No game. No CTest.

## Next

Round 3: writer of **`[+0x78+0xC8]`** (render SPR dt) or of the
incoming seconds to **`0x42E2BF`**. Do not treat `[game+0x26028]` as
either. Evidence: this note, [A6d13](phase-a6-render-clock-dt.md).
