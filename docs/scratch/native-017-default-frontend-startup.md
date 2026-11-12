# native-017: default connected frontend startup

2026-10-30 implementation and observer-harness revision; **needs_review**, pending
supervisor re-verification and separate acceptance. This is a portable application CLI policy, not a newly
recovered original function, ABI, startup sequence or fidelity pass.

## Runtime change and path

Before, ordinary startup loaded `MM_CLIMB_002` with diagnostic injected walkers.
Now an invocation with no options, or only `--game-dir PATH`, opens the existing
connected frontend. Other tokens keep their prior routing, including ignored
unknown options. `--direct-level` explicitly requests the previous bare-launch
diagnostic. Explicit `--frontend` wins regardless of option order, including with
`--direct-level`; its existing map/map-flags conflict rejection remains active.
An empty/missing game-directory argument does not acquire the new default.

The production path is `main` -> `runApplication` -> `parseArgs` -> one persistent
`FrontendSession::create` -> `runApplicationLeg` -> `runFrontendWindow`. Both app
layers obtain the frontend decision from `parseArgs`. The existing Return-driven
Menu/confirmation path reaches map selection; `runApplicationLeg` takes the
selected level name, enables simulation, clears diagnostic `simTicks`, and uses
the existing `sim::loadRetailLevel`/`World`/SDL rendering path. The session remains
owned by the existing outer loop for results, retry and return-to-map legs.
No additional session lifecycle or gameplay implementation was introduced.

Only `src/app/main.cpp` changes production behavior. `README.md` and help explain
the limited default rule and explicit diagnostic entry. The test extension is
in `src/__test__/test_frontendentry.cpp`; build registrations/timeouts are intact.

## Authentic source-built before/after evidence

Ignored `build/native-017/main.before.cpp` was frozen before production edits,
SHA-256 `f12ea30d01ddce1cf2a9e3d34a38b0f39b050fe21be1befaf89b836cfdcddc20`.
`startup_probe.cpp` and `run_regressions.py` compile this authentic source and the
current source as separate objects, link the shared source-built application
objects, and call public `runApplication`. Production files were never swapped.
Both phases use repository-local `WEBOLUTION_GAME_DIR` and SDL dummy video/audio.
The revised results are under ignored `build/native-017/observer-revision/`.
Every invocation must exit zero after exactly one observer-queued `SDL_QUIT`;
the external eight-second timeout is a failure guard, never an accepted outcome.

The frozen diagnostic does not call `LevelFrameObserver`, which is nested in
the simulation/HUD branch. The source-test compiler therefore force-includes
`startup_present_observer.h` into only the frozen/current main translation unit.
It includes SDL's declarations, then substitutes that translation unit's
`SDL_RenderPresent` calls with `native017ObservePresent`. The probe reads the
640x480 software-renderer image before presentation, calls real SDL presentation,
and queues `SDL_QUIT` after the first frame. The unchanged application loop
consumes the event and performs its normal cleanup. FrontendWindow is compiled
normally and retains its existing session observer. No production observer,
gameplay branch, frozen source bytes, invocation options or runtime libraries
are changed. This is compile-time source-test instrumentation, with no runtime
patching, DLL replacement, original execution or installed hook.

| Invocation | Frozen result | Current result |
| --- | --- | --- |
| No options | Diagnostic frame0; frontend/level/presentation callbacks 0/0/1; exit0 in 1.192s | Menu frame0; callbacks 1/0/0; exit0 in 0.208s |
| Only `--game-dir PATH` | Diagnostic frame0; callbacks 0/0/1; exit0 in 1.155s | Menu frame0; callbacks 1/0/0; exit0 in 0.175s |
| Explicit `--frontend` | Menu frame0; callbacks 1/0/0; exit0 in 0.177s | Menu frame0; callbacks 1/0/0; exit0 in 0.174s |

The current Menu has construction counter3, focus0, no selected level, zero
completion flags, focused pointer320,240 and released buttons. Its composed
640x480 RGBA image for both default invocations is byte-identical to the frozen
explicit frontend image, as is the current explicit frontend image. All four
also match the original probe's frozen explicit image (SHA-256
`da3c81daf9371429f1c134d7bc27f03771e1fc03d68bdcb015ffc4eed8e14622`).
Current `--direct-level` and ignored-unknown-only runs exit0 in 1.110s/1.100s
with callbacks0/0/1 and one queued quit each. Their initial level statistics
and first presented images match both frozen default runs (image SHA-256
`48450297130d19d47b78b4a0a832d3e5b8c587ed8336018e14eea53e9b00fb27`).
The JSON summaries and logs retain actual argument lists, callback counts,
queued-event counts, image hashes, exit codes and external bounds.

**Retained failed attempt:** the earlier unadapted probe's frozen defaults timed
out after 8.047s/8.016s, with zero frontend/level callbacks; its direct diagnostic
controls also timed out. Those logs and original JSON remain at the parent
`build/native-017/` path. Independent review rejected treating those timeouts as
successful evidence. The revised probe resolves termination through a separate
presentation observer and reports its count explicitly; it never invents a
simulation-level callback. The earlier test and report copies remain under
`observer-revision/*.before` for the harness/report, and `note.before.md`.

## Source verification and retained failure

The registered GCC target `test_frontendentry` built successfully; focused CTest
`frontendentry` passed in **26.34s**, with its existing 120-second timeout.
The existing explicit frontend selection, 30-frame limits, independent camera
calculation, tick0/tick1 positive control and 20-diagnostic-tick reset render
comparisons remain intact. The additional no-option route drives the same
Menu3 -> Confirm4 -> Map5 sequence to node1 (`MM_FLOAT`, distinct from the default
`MM_CLIMB_002`), observes one simulated level frame with session counter5, and
checks selected identity, fresh saved/dead/skill state and retail setup. It quits
through the level observer. Its real production clock is not a new timing test.

Additional checks cover game-dir-only initial Menu/image equality, explicit
frontend precedence in either order, direct `--sim`/seeded `--sim-ticks`, direct
diagnostic screenshots, standalone map image composition, frontend screenshots,
map conflicts, empty map-flags, both missing-asset default forms and asset-free
`--help`/`-h`. No long additional render sweep or timeout increase was added.

The first extended test timed out at 120.03s in the missing-assets check: SDL's
Windows environment setter did not update the executable CRT's `std::getenv`
view. The test now uses `_putenv_s` on Windows (`setenv` elsewhere) and restores
the prior directory. The first failing log/JUnit and the passing rerun/full CTest
log are retained. This was a test setup failure, not an altered production error
policy. No assertions were weakened to pass it.

After the harness revision, registered CTest `frontendentry` passed again in
**26.21s**, retaining its 120-second test timeout and all source assertions.
The outer command had a separate 150-second failure guard. Actual log, JUnit
and command/elapsed record are in `observer-revision/frontendentry*`.

The previous supervisor's `python tools/agent_control/verify.py --profile all`
passed 70/70 main tests in 385.98s and 84/84 recovery tests in 29.11s; actual log:
`build/agent-runs/20261030T194247-40868-e90944ad/verification.log`. This revision
changes the ignored harness and this note only, preserving production/test source
bytes. The supervisor will rerun full verification after this worker exits;
that rerun is pending. Current read-only inventories retain 70 main / 84 recovery
registrations, empty recovery `LEMMINGS_REFERENCE_EXE`, and `native_int_array`
as the sole native-prefixed recovery source test. See ignored reports for the
revision's focused test result and preservation checks.

## Boundaries and next action

Reference identity, checked by reading bytes only:
`Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
No original execution, copied machine instructions, oracle, hook, debugger,
native capture, launcher, installed-save write or installation modification was
performed. Session-only completion, navigation, results/retry, camera arithmetic,
simulation, input, rendering and audio implementations are unchanged.

The retained [menu navigation](frontend-menu-focus-navigation-2026-10-28.md) and
[confirmation](frontend-confirmation-focus-2026-10-28.md) comparisons retain their
original scopes and failures. Default entry does not establish original startup,
full frontend fidelity, persistence, movie playback or gameplay timing/frame
agreement. `docs/STATUS.md` retains its historical opt-in wording and all gates;
this separate note records the implementation without promoting a gate. The
recovery 372 / 6,186 baseline, native-006 deferral and D1/D3/digger parks remain.

Next: supervisor full verification, then separate read-only acceptance review of
native-017, including the revised compile-time presentation observer and all
eight normally terminated probe runs. Queue entries are
untouched by this worker. Further runtime work requires a separately selected
unparked task; this task does not authorize original execution.
