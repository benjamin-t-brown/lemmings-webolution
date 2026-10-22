# D3a: production retail-level loader (.LVL -> sim::LevelSetup)

Queue id `d3a-retail-level-setup`. Round 1 of 3. Offline build/test; no live game, no
stock start. Full worker evidence:
`build/tmp/d3a-retail-level-setup/{prediction.md,sweep.txt,round-note.md}`.

## Question

Can one production function build the `sim::LevelSetup` for any retail level from the
original's data, so MM_CLIMB_002's setup is the same one D1 already traces?

## Answer: yes, on all 102 retail levels

`webo::sim::loadRetailLevel`/`loadRetailLevelReport` (`src/sim/RetailLevel.h`/`.cpp`)
assemble a `LevelSetup` straight from the archive: terrain from the stored collision grid,
one `Entrance` per hatch at `hatchSpawnCell`, release timing from the (first) hatch's own
rate/min/max clamped and looked up in `RELEASERATE.PC`, every `Exit*.a3d` via
`exitTriggerFromPose`, every teleport/speed-pad/lever via their own trigger functions,
totals/save target/time limit from `LEVELMAP.DAT`, and skills from the level's own
trailer. No reverse engineering was needed; every piece was already read or measured by an
earlier round.

- **Hatch rate words now stored**: `HatchRecord` gained `rate`/`rateMin`/`rateMax` (the
  `hatch+0xA0`/`+0xA4`/`+0xA8` tail u32s D1b named but nothing previously stored),
  parsed in `findHatchRecords` immediately after the existing count/x/y u16 triple.
- **D1b's open hypothesis is now checked and holds**: of 144 hatch records across all
  102 retail levels, 0 have `rate` outside their own `[rateMin, rateMax]`.
- **`SimReleaseTrace` rewired** onto `loadRetailLevelReport`, CLI unchanged. Run on D1g's
  own exact invocation before and after the change: byte-identical output (sha256
  `7653aedb4108d06aa0ad8026a70df645fe60273638db358638f2368ee7dde664` both times), even
  though the loader now also attaches real exits/teleports/speed-pads/levers/skills/totals
  the old hand-built setup never had — none of which the specific invocation's window
  reaches.
- **Named loss, not hidden**: 33 of 102 levels have more than one hatch record. Every
  hatch still contributes its own `Entrance` (`World::spawn()` already round-robins
  multiple entrances, unchanged), but `LevelSetup` carries only one shared release-timing
  triple, driven by the *first* hatch's own cadence. Every hatch's own rate/min/max is
  still read and reported (per-hatch detail in `sweep.txt`), not silently dropped.
  `totalLemmings` comes from `LevelInfo`, not summed hatch words — checked to agree on
  all 102 levels including the 33 multi-hatch ones.
- **`releaseArmSeconds` (`0.723061695938398`) stays the MM_CLIMB_002-only DERIVED
  constant** for every level, per the brief's instruction, not re-derived elsewhere.
- **Deliberately no guessed fallback**: a hatch-less or exit-less level is a load
  failure, not an `old/src`-style placed spawn point. The retail corpus has zero such
  levels, so this path is untested in the sense that it never fires, but it never
  silently masks a missing read either.

## Tests

`test_retaillevel.cpp` (CTest `retaillevel`): MM_CLIMB_002 acceptance (one entrance at
`hatchSpawnCell`, the rate-20 seconds exactly, `LevelInfo` totals, skills and exits
independently recomputed from the same already-tested primitives, not copied literals; a
20-tick smoke run spawns at least one lemming); an unknown-level-id negative check; the
corpus-sweep writer (deliverable `build/tmp/d3a-retail-level-setup/sweep.txt`).

## Facilitator check (2026-10-21)

Independently confirmed the new hatch tail-word offsets in `src/assets/LevelObjects.cpp`
(`kTailAt = kTripleAt + 6`, i.e. hatch+0x9A + 6 = +0xA0/+0xA4/+0xA8) match D1b's
already-landed offsets exactly — no re-disassembly needed since D1b already read these
bytes; this round only added storage for words D1b had already located.

Mutate-one-alone: swapped `RetailLevel.cpp`'s entrance `e.x`/`e.y` assignment, rebuilt, ran
`test_retaillevel` — caught exactly as expected (`FAIL  entrance at hatchSpawnCell
(32, 53) (got (53, 32))`). Restored to the worker-delivered SHA-256, rebuilt, full suite
**26/26** (including the new `retaillevel` test and the concurrently-landed `pick` test).
Confirmed the shared `CMakeLists.txt`/`cmake/webolution_sources.cmake` edits from this
round and the concurrently-dispatched D2a round merged without conflict. No Lemmings
Revolution process was started by this round or by this check.

**Landed.** `src/sim/World.*` was not touched, keeping D1's own sim trace byte-identical
as the brief required. D3b (app sim-loop integration) depends on this round and may now be
dispatched.
