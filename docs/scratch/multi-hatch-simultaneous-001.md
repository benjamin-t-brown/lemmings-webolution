# multi-hatch-simultaneous-001

Status: **needs_review**. Playable v1, [playtest 10 Z6](../playtest/2026-11-16-playtest-10.md):
Kriss Kross (node 19, E19_KF_2) alternated its two hatches instead of releasing
from both together. Difficulty: difficult; recommended Opus, assigned Codex
override used. **Smoke/recovery verification is pending supervisor execution.**

## Production change and count evidence

`runApplicationLeg` -> `loadRetailLevelReport` -> `findLevelObjects` /
`readHatchRecords` -> `Entrance.releaseCount` -> `RuntimeSimState::advanceElapsed`
-> `World::advanceElapsedMsWithReleaseGate` -> `updateSegment` -> `stepRelease`
-> `spawn` -> `spawnFromEntrance` -> existing Faller update/rendering.
Legacy `World::tick` reaches the same batch release function.

Each shared release tick now emits once from **every nonempty hatch**. Each
birth retains its hatch's position, type and direction/alternation, and receives
the same fractional first-fall budget as the other births in that tick. A hatch
stops at its stored quota; LevelInfo still caps the level total. Stream order
provisionally determines actor indices for simultaneous births. Shared countdown,
rollover, first-release seed, release gate and player rate bounds are retained.
Hatch hover now uses the actual quota minus that hatch's emitted count.
Hand-built setups without stored counts provisionally divide the unassigned
population evenly, with earlier hatches receiving any remainder.

The division is **stored data**, not an inferred even split. The accepted
[structured-reader report](native-009-structured-hatch-records.md#field-mapping)
traces `425E50` / `425F02`: the first word at hatch `+98` is its count.
Started from the [retained attempt](../../build/lead/multi-hatch-release-001-attempt1/multi-hatch-release-001.md)
and its blocked review in `agent/tasks.json`; that task remains blocked.
Its independent intervals regressed M20_MT_1 to 45/53. This task deliberately
keeps the authorized shared cadence, including the shared hatch-click control.

Retained Ghidra pseudocode was cross-checked against its retained instructions:
`425B72..425B7B` tests this hatch's signed remaining word `+98`;
`425BBD..425BCB` decrements it per birth; `425D3C..425D4B` adds its interval
to its residual countdown. Dispatcher `442BC6..442BDC` forwards the same dt
to successive object updates. See the retained
[instruction packet](../../build/multi-hatch-release-001/static/0x00425A95-instructions.json),
evidence `ev_529e52e7994f42f2209413a10a0a48aac5cbb0bec1015ee42e9da22e3e9464c8`,
and this task's [cross-check index](../../build/multi-hatch-simultaneous-001/static/retained-crosscheck.txt).
Reference: repository-local `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
No fresh analysis session or original execution was needed. Static original
per-hatch intervals do **not** establish this clone's provisional shared policy
as original fidelity; original door phases and same-tick insertion order remain
unproven.

## Capacity policy and installed-level checks

One installed default conflicts with the full-release requirement: **VH21_JC**
(node 97), quotas **1/49**, shared initial rate **1** / five seconds, needs about
242 seconds against its 210-second limit. To preserve the evidenced populations,
the loader provisionally raises an initial multi-hatch rate only when necessary,
to the slowest table rate within the primary hatch's existing bounds that fits
the largest quota. It reserves five level seconds for ordinary entry (the
4.5-second cue and 2.8-second doors), the first seed, and two update periods.
Only **VH21_JC changes, rate 1 -> 4**; its bounds remain 1..99. Rate 3 fits
the direct scheduler but cannot accommodate ordinary entry, so it is insufficient.
`RuntimeReleaseRateControl::create` uses the loader's effective initial rate so
the displayed rate and subsequent clicks agree with World. Installed data is
unchanged. This is a **provisional player-facing default**, not original timing
evidence or a new mandatory minimum for player input.

The short legacy unit checks simultaneous batches, uneven quotas, independent
exhaustion, first-fall budgets, type/order, direction alternation, total cap,
nuke cancellation, and unspecified-count fallback. Its installed-data census
checks all **33 multi-hatch levels**, every quota reaching World and the shared
control's initial rate. All **99 cases** release their full totals before their
limits: explicit 200 ms, legacy tick, and explicit 20 ms with a five-second
release gate and connected-entry arm=0. These are source-built scheduler
observations, not scripted wins or human playability/fidelity evidence.

| Installed level | Stored quotas / released | Direct 200 ms completion | With 5 s entry gate, 20 ms | Limit |
| --- | --- | ---: | ---: | ---: |
| E19_KF_2, node 19 | 15+15 / **30/30** | 46.8 s | 50.94 s | 300 s |
| M20_MT_1, node 50 | 3+50 / **53/53** | 36.0 s | 40.16 s | 210 s |
| M06_KF_1, node 11 | 50+50 / **100/100** | 119.2 s | 123.38 s | 180 s |
| E10_KF_1, node 91 | 7+13+20+10 / **50/50** | 97.2 s | 101.34 s | 180 s |
| VH21_JC, node 97 | 1+49 / **50/50** | 201.8 s | 206.02 s | 210 s |

Kriss Kross's first pair appears at 2,200 ms on the direct 200 ms path; every
following release serves both hatches until each reaches 15. All levels and
partitions are retained in [final-census.csv](../../build/multi-hatch-simultaneous-001/final-census.csv).
These checks use default rates and ordinary entry; player rate reductions,
nuking, or fast-forward during the entry hold are not guarantees of full release
before timeout. No movement, skill, completion or shared-clock rules changed.

## Promoted human recordings and verification

Ran the built `webolution --game-dir "Lemmings Revolution" --replay <file>`
sequentially for all 16 promoted recordings. Exact filenames, complete output
and exit codes: [final-replays/results.json](../../build/multi-hatch-simultaneous-001/final-replays/results.json).

| Recording level | Result |
| --- | --- |
| MM_CLIMB_002 | PASS 10/1 |
| MM_BOMB | PASS 17/10 |
| MM_FLOAT | PASS 10/1 |
| MM_BLOCK_001_A | PASS 24/15 |
| MM_BUILD | PASS 20/10 |
| MM_MINE | PASS 20/10 |
| MM_BASH_002 | PASS 10/10 |
| MM_DIG_000 | PASS 19/10 |
| MM_W_012 | PASS 16/10 |
| MM_BLOCK_BOMB_A | PASS 12/10 |
| MM_BASH_MINE_WLR1 | PASS 20/10 |
| MM_BASH_DIG_001_A | PASS 20/10 |
| MM_BUILD_BLOCK_A | PASS 15/10 |
| MM_E1_3_SKILLS | PASS 10/5 |
| M06_KF_1-1794811410542247000-12 | DIVERGED step **175** |
| MM_FLOAT_CLIMB2-1794812527275449000-2 | DIVERGED step **163** |

All 14 single-hatch recordings pass with won=1/completed=1/exit 0. Both
multi-hatch recordings diverge on the intended **first simultaneous release**:
at clock 5,836 ms each expects spawned/saved/dead=1/0/0 and observes **2/0/0**.
No additional mismatch was reached after those strict failures. Neither file,
replay assertion nor replay registration was changed; the lead owns retirement.
The already retired recordings were left alone.

Registered UCRT64 GCC 16.1.0, Debug `-O2 -g` with assertions, eight build jobs.
[final-focused-checks.log](../../build/multi-hatch-simultaneous-001/final-focused-checks.log):
**5/5** pass (`world`, `retaillevel`, `sharedtiming`, `hatch_rate_units`,
`multi_hatch_simultaneous`); the new test stays legacy and runs in seconds.
[final-ctest-details.log](../../build/multi-hatch-simultaneous-001/final-ctest-details.log)
retains the census output. Full smoke/recovery is **pending the supervisor**;
no full profile was duplicated. Existing lever-initializer warnings remain.

Changed: `src/sim/World.h/.cpp`, `src/sim/RetailLevel.h/.cpp`,
`src/app/RuntimeSkillPanel.cpp`, the hatch-hover count in `src/app/main.cpp`,
`src/__test__/test_multi_hatch_simultaneous.cpp`, its legacy CMake registration,
and this report. Incoming edits are preserved under
[entry/](../../build/multi-hatch-simultaneous-001/entry/); the
[task-only diff](../../build/multi-hatch-simultaneous-001/task-source.diff) isolates this work.
The existing supervisor **PID 40668** owns RepositoryLease; Get-Process checked
live processes (CIM command-line metadata was denied, not retried).
Runs used private LocalAppData under `build/multi-hatch-simultaneous-001/appdata`;
World units and replay create no SaveStore. Installed and normal user saves
were unused. No original/oracle, debugger, hooks, capture, install, subagents,
concurrent writers, protected-control edits, commits or pushes occurred.

Handoff: simultaneous shared-cadence releases preserve stored quotas; all 33 multi-hatch levels empty in the census, including M20_MT_1.
Handoff: needs_review; supervisor smoke/recovery and independent review next, with lead retirement of the two intended replay divergences.

What to check: **Kriss Kross (node 19, E19_KF_2)**, start normally and watch both
hatches: each release tick drops one from each, both hover counts fall together
from 15 to zero, and all 30 release. Click either hatch to change the shared
rate; paired births remain synchronized. On **M20_MT_1 (node 50)**, the
three-count hatch stops after three births while the other continues to 50.
On **VH21_JC (node 97)**, the initial shared rate reads 4 and both hatches empty
(1/49) before the time limit without requiring a rate increase.
