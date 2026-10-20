# Phase A6d12: first lasting post-spawn `+0x10` is fall `0x41627F` (READ)

Brief `build/tmp/a6d12-plus8-first-y-brief.md`. Packet
`build/tmp/a6d12-plus8-first-y/`. Cursor `round-agent` on `composer-2.5-fast`,
dispatch `1502a6e8-d58a-48b6-90d3-2cb3e8c7707a`. Offline. No mapper.

## Result

- **READ:** hatch ctor after `0x4230F9` has no direct `fstp` to `+0x10`.
  Factory `0x415FA0` writes vtable `0x4B0A20`; attach at `0x42E0C5` →
  `0x42E696`.
- **READ:** fall integrator `0x4160BD` stores
  `0x40DF50([+0x10] − (double)[+0x44] × step)` at **`0x41627F`**
  (`dd 59 10`). Spawn sets `[+0x44]=1` at `0x42DFE7`. Step comes from
  `0x416038` into `[ebp-0x20]`.
- **READ:** `0x416219` `fsubr [+0x10]` is intermediate; that product goes
  to `0x43DA38` and **`fstp [+0x18]`** at `0x416236`, not to `+0x10`.
- **READ:** `0x42EE9A` via `0x41C6BA` remains a separate walk/land arm.
  First-tick order vs `0x41627F` is **unsettled** (no live run).
- Worker prediction (`0x42EE9A` first) was **falsified**.

## Facilitator check

2026-10-20, Cursor facilitator.

- Re-read `0x416263`–`0x41627F`, `0x416219`–`0x416236`, `0x42DFE7`,
  `0x415FB2` (`mov [eax], 0x4B0A20`), `0x42E09D` / `0x42E0C5`,
  `0x41C6BA`, ctor `0x42DEAA`–`0x42E178` (no `fstp […+0x10]`).
- **Narrowed:** `0x4160BD` is vtable `0x4B0A20` **slot +4**
  (`bd 60 41 00`), not +0x14. File `.rdata` is inside rawsize.
- **Narrowed:** ctor ends with `call [eax+8]` at `0x42E147` on the
  `+0x78` object; that indirect was not expanded. No direct `+0x10`
  store in the ctor body.
- No production change. No game. No CTest (no code change).
  `ActorTerrain.cpp` still spawn-only (`z = 0.5`).

## Parked

Sim → `+0x08` after three rounds (A6d10–A6d12). Spawn sample, motion
writers, and fall Y store are READ. Still open: first-tick arm order,
walk remainder / render-clock dt, live Z 0.75, Y-flip of sim.y.
Resume only with a named missing term. Evidence: this note,
[A6d10](phase-a6-sim-plus8-writer.md),
[A6d11](phase-a6-plus8-spawn-init.md).
