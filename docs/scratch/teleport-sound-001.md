# Playtest 30 W1 · Teleport sound

**The Lone Ranger (`MM_TELE_003`, node 38) has no authored teleport sound.** The installed `COMPILED\MM_TELE_003.LVL` from `lemmings.box` leaves both sample fields empty on all four pads (IDs 3–6). `names[0]` and `names[1]` contain `cartwheel` and map to the two other string fields in the original reader. The clone’s `runApplicationLeg → World::GadgetUse → BackdropScene::triggerGadget` returns an empty name at both events, so the guard in `main.cpp` skips `CuePlayback::playGadgetSample` before archive lookup, decoding or spatial gains. The task changes only this report and its supporting evidence. [Playtest 30 W1](../playtest/2026-11-23-playtest-30.md#defects) · [Clone trace](../../build/teleport-sound-001/lone-ranger-trace.log)

Static evidence comes from the registered x86 `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`. Ghidra 12.1.4 pseudocode was checked against the instructions. The REA session closed without a snapshot. [Static evidence](../../build/teleport-sound-001/rea.json)

| Original path | Static result |
| --- | --- |
| Reader `0x004347EC` | Reads the two `cartwheel` strings into `+0xA8/+0x128` at `0x00434998/0x004349AD`. Reads `names[2]` and `names[3]` at `0x004349CE/0x00434A23`, stores `-1` for empty names at `0x004349ED/0x00434A42`, and calls `0x0047428C` to resolve nonempty names into `+0x1A8/+0x1AC`. |
| Departure in `0x00434B0F` | Tests the source `+0x1A8` at `0x0043557A`, then submits its handle at `0x004355F5 → 0x0044535B` if eligible. The Lone Ranger skips the call. |
| Arrival in `0x00434B0F` | Resolves the partner at `0x00434BCD–0x00434BF3`. Tests the partner’s `+0x1AC` at `0x00435198` and uses the source’s `+0x1AC` for lookup and the call at `0x00435207 → 0x0044535B`. Both pads have `-1` on The Lone Ranger, so the call is skipped. |
| FX draw `0x00434AAF` | The per-side `+0x1D0/+0x1D1` gate at `0x00434AE0` controls the model draw at `0x00434B06`. The draw contains no sample call. |

Both pads on **Water way to go** (`SHOW_KF3`) supply `teleport` on departure and `teleport2` on arrival. The installed overlay resolves and decodes `SOUNDS\TELEPORT.WAV` and `SOUNDS\TELEPORT2.WAV` from `lemmings.box`. Those names belong to that level. The clone trace uses a scratch adapter around the existing replay driver with SDL dummy audio. It records **194 departures, 194 arrivals, and zero gadget sample requests** on the promoted Lone Ranger win. Physical audio, original runtime and the original’s source/partner behavior with different nonempty arrival samples were untested. [Probe source](../../build/teleport-sound-001/teleport_trace.cpp) · [Trace build](../../build/teleport-sound-001/trace-build.log) · [Clone trace](../../build/teleport-sound-001/lone-ranger-trace.log)

The stock `webolution --game-dir "Lemmings Revolution" --replay` passed both promoted Lone Ranger recordings using task-private `LOCALAPPDATA`. [CLI log](../../build/teleport-sound-001/promoted-replays.log)

| Recording | Recorded result reproduced |
| --- | --- |
| [Playtest 29 attempt 1](../playtest/replays/2026-11-22-playtest-29/MM_TELE_003-1795374979657686700-1.replay) | Step 12893, 0/90, returned to map |
| [Playtest 30 attempt 7](../playtest/replays/2026-11-23-playtest-30/MM_TELE_003-1795419339808551800-7.replay) | Step 25486, 90/90, completed win |

The `smoke_tests` target was up to date. Five focused smoke-labelled tests passed (`cueplayback`, `teleportlinks`, `backdropscene`, `teleportapp`, `wav`). The probe executable and object were deleted. [Build log](../../build/teleport-sound-001/smoke-target-build.log) · [CTest log](../../build/teleport-sound-001/focused-smoke.log)

Handoff **needs_review**. The supervisor smoke profile is pending.  
Next action is independent review of the installed fields and the two original play guards.

**What to check.** On The Lone Ranger (node 38), send a lemming through a linked pad. Expect the visual effect with silence on departure and arrival. On Water way to go (node 46), send a lemming through a pad and listen for the two authored cues.
