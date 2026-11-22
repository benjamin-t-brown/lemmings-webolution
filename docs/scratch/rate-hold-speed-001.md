# Hatch release-rate hold cadence

**needs_review.** Playtest 28 T2 reported that holding right click took about
two seconds to raise the original from 1 to 99. The clone stepped on press,
waited 400 ms, then added one step every 100 ms, taking about ten seconds
for that range. The playtest used **Just Climb** (`MM_CLIMB_002`, node 0)
to check hatch input. Its installed range is 1–50. The full-range timing
probe uses **Sometimes up..sometimes down.** (`M06_KF_1`, node 11).
[Playtest 28](../playtest/2026-11-22-playtest-28.md)

## Original static trace

Ghidra 12.1.4 via static REA read the registered x86 executable,
`Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The instruction reads agree with the targeted pseudocode. REA closed without
a snapshot.

- In `0x4532E2`, the branch at `0x453A31` checks the selected hatch at `+0x1894`.
  After the call to `0x455DD0` at `0x453A42`, `0x453A5C..0x453A72` reads the
  hatch's `+0xA0`, adds one, and calls `0x425DE3`. After the call to `0x455D90`
  at `0x453A7F`, `0x453A99..0x453AAF` subtracts one and calls the same setter. The wrappers
  call `0x455DF0` at `0x455DDD` and `0x455DB0` at `0x455D9D` respectively.
  The comparisons at `0x455DFC` and `0x455DBC` read held fields `+4` and `+0`,
  populated at `0x483C70` and `0x483C56`. Each held direction steps on the
  first eligible update and every eligible update thereafter. The hatch
  branch gates on the selected hatch and held state with no repeat timer.
  [Input instructions and pseudocode](../../build/rate-hold-speed-001/static-input.json)
- Xrefs to `0x4532E2` lead to `0x45DAA4` and `0x45DABF` in frame loop
  `0x45CFF9`, entered at `0x450A53`. The conditional at `0x45DA64..0x45DA8E`
  and jump at `0x45DAA9` make the calls mutually exclusive. Both receive the
  frame delta computed at `0x45DA15..0x45DA27`, and both precede the simulation
  pass loop. The outer back edge at `0x45E435` returns to the timer read at
  `0x45D074`. That is one hatch update per eligible frame, including under
  fast-forward. [Frame instructions, xrefs and pseudocode](../../build/rate-hold-speed-001/static-frame.json)
- `0x45D074..0x45D0BA` measures elapsed `timeGetTime` milliseconds.
  `0x45D921..0x45D96B` caps the delta at 100 ms without delaying the loop.
  Presentation at `0x45E3A0` calls `0x470735`, which can wait in DirectDraw
  `Flip` (`0x470766`, `DDFLIP_WAIT`) or `Blt` (`0x4707BF`, `DDBLT_WAIT`).
  The fullscreen `SetDisplayMode` at `0x46F850` reads refresh from the selected
  mode data. The traced path establishes no fixed game-wide frame rate. The
  original's observed steps per second can depend on presentation and host
  speed. [Presentation trace](../../build/rate-hold-speed-001/static-presentation.json)

## Clone change and checks

`runApplicationLeg` uses `tryHatchPress` for the immediate step and counts
only post-press time on that first frame. For a supplied fixed frame delta
the press is sampled at the frame boundary. `advanceReleaseHold` uses
`RuntimeReleaseRateControl::advance` to carry fractional hold time and repeat
every 20 ms, a **provisional 50 steps per second** from the playtest measurement.
Left lowers and right raises. `heldTarget` keeps the selected hatch through
overlap, and `change` clamps to authored limits through `World::setHatchReleaseRate`.
`recordCommands` still sends the resolved hatch index, rate, and interval
through the existing replay format. [Input path](../../src/app/main.cpp) ·
[Rate control](../../src/app/RuntimeSkillPanel.cpp)

- The source-built probe linked the production control and checked every
  sampled rate from 1 to 99 on `M06_KF_1` with overlapping hatch rectangles.
  The rate stepped to 2 immediately, then reached 99 at 1.967 s (30 Hz),
  1.940 s (50 Hz), 1.950 s (60 Hz) and 1.944 s (144 Hz). The second hatch
  held at rate 30 throughout. [Probe source](../../build/rate-hold-speed-001/check-hold.cpp) ·
  [Completed probe run](../../build/rate-hold-speed-001/hold-probe.log)
- The legacy hatch unit encoded the defect by expecting the first repeat
  at 400 ms and the next one 100 ms later. It now checks the immediate step,
  a split 19+1 ms first interval, and successive 20 ms repeats during actor
  overlap. The unit compiled and is deferred to milestone sign-off.
  Registered UCRT64 tools built `smoke_tests`. All five focused smoke-labelled
  tests passed for level input, overlap, cursor and HUD. Checks used private
  local app data. The supervisor's full smoke and recovery gate is pending.
  [Legacy unit](../../src/__test__/test_hatch_rate.cpp) ·
  [Completed build](../../build/rate-hold-speed-001/build-smoke-tests.log) ·
  [Focused CTest run](../../build/rate-hold-speed-001/focused-smoke.log)
- All 29 promoted recordings returned `PASS won=1 completed=1`, including
  **Just Climb** at step 1252 and **What a Fantastic Pair of Birds** (`M02_MG_1`,
  playtest 28 attempt 36) at step 28031. Replay applies recorded commands.
  Physical hold timing and feedback are pending human playtest.
  [Per-recording commands and results](../../build/rate-hold-speed-001/promoted-replays.tsv)

Handoff — `main.cpp`, `RuntimeSkillPanel.cpp`, and `test_hatch_rate.cpp` implement and cover the new hold cadence.  
Next — Supervisor smoke gate, independent review, and the next human playtest.

**What to check** — On **Sometimes up..sometimes down.**, hold left on one
hatch to reach 1, then hold right on it. Expect an immediate step to 2 and
continuous increases to 99 in about two seconds, with the other hatch's
rate unaffected. Release or move off the hatch mid-hold and expect the rate
to stop changing. On **Just Climb**, expect the same behavior with a cap of 50.
