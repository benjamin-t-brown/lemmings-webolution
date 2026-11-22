# Each hatch uses its own release rate

**needs_review.** Playtest 26 R3 reported that Penelope Lem Stop (`M04_KF_1`,
node 34) gave its 30-lemming hatch the two-lemming hatch's rate of 30. The
30-lemming hatch now starts at its stored rate of 99, which is also its fixed
minimum and maximum. [Playtest ledger](../playtest/2026-11-21-playtest-26.md#defects)

## Production path

`loadRetailLevelReport` gives each `Entrance` its stored quota and its own
bounded RELEASERATE.PC row. `World::stepRelease`, reached by both
`advanceElapsedMsWithReleaseGate` and `tick`, uses the existing arm and first
countdown seed. Each hatch then subtracts the same update dt from its own
countdown, releases at most once per update and adds its own interval to the
residual. Stream order still resolves same-update births. The elapsed path for
one hatch does the same countdown arithmetic as before. Hand-built tick
setups still release in batches.

`runApplicationLeg` sends the picked hatch index to
`RuntimeReleaseRateControl`, which changes that hatch's rate within its own
bounds through `World::setHatchReleaseRate`. The pending countdown carries
through a click or hold. The held hover readout uses the rendered target's
hatch index. Replay v1 records changes by hatch index, including two hatches
changed to the same value, and playback applies each command to its indexed
hatch under the existing strict checks.

## Static evidence

Registered x86 reference `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Ghidra 12.1.4 opened this reference through REA and closed it without a
snapshot. The picker publishes the selected hatch at screen `+0x1894`
(`0x45525C`). Both click branches at `0x453A42..0x453AAF` read that hatch's
`+0xA0` and pass the same hatch to `0x425DE3`. The held-button wrappers
reach `0x455DF0` for +1 and `0x455DB0` for −1. The record reader writes the
stored rate at `0x425F40` and calls the setter at `0x425FD5`. The setter clamps
against `+0xA4/+0xA8`, copies the table row to `+0xB8/+0xBC` and writes
`+0xA0` at `0x425E44`. The other direct caller at `0x457095` resolves one
indexed hatch. Retained instructions show the per-hatch remaining word
(`0x425B72..0x425BCB`), residual plus interval (`0x425D3C..0x425D4B`) and
same dt dispatch (`0x442BC6..0x442BDC`). The original door phase and exact
same-update insertion order remain open. [Fresh instructions, xrefs and pseudocode](../../build/multi-hatch-rate-001/static/selected-evidence.json) · [Retained release evidence](multi-hatch-simultaneous-001.md#production-change-and-count-evidence)

## Clone observations

The census compares 75 hatches on 33 installed levels under the previous shared
cadence and the current per-hatch cadence. All 99 level/partition cases released
their total before the limit, and the first release update matched in every
case. Times below use 20 ms updates after a five-second entry hold. The CSV
also includes direct 200 ms updates and legacy ticks. [Census](../../build/multi-hatch-rate-001/census.csv)

| Level | Released | Before → after last birth | Limit |
| --- | ---: | ---: | ---: |
| Penelope Lem Stop (`M04_KF_1`) | 32/32 | 75.60 → 12.12 s | 360 s |
| Bridges (`M20_MT_1`) | 53/53 | 40.16 → 203.82 s | 210 s |
| My friend the end. (`VH21_JC`) | 50/50 | 206.02 → 206.02 s | 210 s |

The earlier Bridges attempt fell to 45/53 because the 50-lemming hatch used
its own stored rate 1, one birth every five seconds. It released 42 on the
earlier direct route and 41 with the current entry hold. The old shared row
had applied the three-lemming hatch's rate of 80 to both. The provisional
capacity floor now applies only to the 50-lemming hatch (1 → 5). The only
other floor is the 49-lemming hatch on `VH21_JC` (1 → 4), where the one-lemming
hatch now keeps rate 1. The capacity samples confirm that one rate step below
each floor still misses the final birth with the entry hold. Player bounds
are the authored bounds. [Earlier attempt](../../build/lead/multi-hatch-release-001-attempt1/multi-hatch-release-001.md) · [Capacity samples](../../build/multi-hatch-rate-001/capacity.csv)

On Penelope Lem Stop, both hatches first released at 6.32 s in the census
schedule. The next birth from the two-lemming hatch was 2.40 s later. The
30-lemming hatch then released every 200 ms and emptied at 12.12 s. Two
source-built application framebuffers show the held RATE label above the
corresponding remaining count. [Birth trace](../../build/multi-hatch-rate-001/M04_KF_1-release.csv) · [RATE 30 frame](../../build/multi-hatch-rate-001/M04_KF_1-hover-30.png) · [RATE 99 frame](../../build/multi-hatch-rate-001/M04_KF_1-hover-99.png)

## Promoted replays and checks

| Recording | Strict replay result |
| --- | --- |
| `M06_KF_1-1794888660414351700-3` | PASS at 1544, 91/50 saved |
| `E19_KF_2-1794930172017377200-7` | PASS at 6034, 24/20 saved |
| `MM_E1_5_SKILLS-1795028230307591600-5` | DIVERGED at 753. Expected four births, saw three at 7.657 s because the second hatch uses rate 5 instead of 10. |
| `E16_KF_1-1795150696218607200-8` | PASS at 24344, 46/45 saved |
| `M08_KF_1-1795150361515499600-4` | PASS at 33240, 42/40 saved |
| `M04_KF_1-1795301005161850400-3` | DIVERGED at 1187. Expected two births, saw three at 4.303 s because the 30-lemming hatch uses rate 99. |

Registered UCRT64 GCC built `smoke_tests` and both legacy release units. Six
focused smoke checks passed (`world`, `retaillevel`, `sharedtiming`,
`frontendentry`, `runtimehud`, `runtimeskillpanel`). Both release units
passed. Their previous shared-rate and recurring simultaneous-birth assertions
encoded R3 and now check target isolation and independent cadence. The replay
files and strict comparisons are intact. Clone checks used private local app
data. The previous supervisor run passed all 95 smoke checks and 84 recovery
checks. The supervisor gate for this revision is pending.
[Build](../../build/multi-hatch-rate-001/build.log) · [Smoke subset](../../build/multi-hatch-rate-001/focused-smoke.log) · [Scheduler unit](../../build/multi-hatch-rate-001/scheduler.log) · [Rate unit](../../build/multi-hatch-rate-001/rate-unit.log) · [Replay logs](../../build/multi-hatch-rate-001/replays-after/results.tsv)
[Previous supervisor run](../../build/agent-runs/20261122T003152-24104-9e019c03/verification.log)

The review found that the single-hatch `SimReleaseTrace --real-seconds`
override reached `LevelSetup` but `World::stepRelease` read the loaded
entrance interval first. The trace tool now writes the explicit interval to
both fields for a sole entrance before constructing `World`. Multi-hatch
elapsed traces continue to use each hatch's loaded interval. On `MM_CLIMB_002`,
which stores a 2.8865 s interval, 0.8 s and 0.4 s overrides both gave a first
birth on tick 4, then births every four and two ticks, respectively. A zero
interval still selected tick timing and released on ticks 1, 8, 15 and 22.
[Trace build](../../build/multi-hatch-rate-001/revision/build-SimReleaseTrace.log) · [Commands and birth checks](../../build/multi-hatch-rate-001/revision/override-check.log)

Changed the retail loader, World release scheduler, runtime hatch control,
application hover wiring, replay command handling and the two legacy units.
The review revision updates only `SimReleaseTrace.cpp` and this report. Human
acceptance of the new cadence remains pending the next playtest.

- Review the single-hatch trace override and the supervisor smoke result for this revision.
- Confirm the new timing in the next human playtest.

**What to check.** Start Penelope Lem Stop (node 34). Hold right click on the
30-lemming hatch and expect RATE 99 and rapid releases. Check the two-lemming
hatch with a brief click and expect it to change from 30 by one step while the
other remains at 99. Expect the two initial births together.
