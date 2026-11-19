# opengl-plan-001 — Renderer conversion plan

The [master plan](../OPENGL_PLAN.md) proposes a runtime choice between the software renderer and one OpenGL 3.3 core / WebGL2 implementation. Both consume the same prepared frame, including the first-draw sprite choices. The plan gives each implementation phase a visible acceptance check, risk and model recommendation. Implementation awaits playable v1 and user approval.

**Production path examined.** `runApplicationLeg → renderFrame → scheduleSceneFrame` submits backdrop, tube, pipes, fluid and secondary scenery. Decals, actors and particles follow, then the level loop composes the HUD and cursor for SDL. Hatch hover and pointer zoom consume the displayed frame's depth. This task changed the plan and this report only. [Source locations](../../build/opengl-plan-001/source-locations.txt) · [Checkout at inspection](../../build/opengl-plan-001/git-show-head.txt)

**Evidence and limits.** Playtest 7 W1 reported minor z clipping. The later side-on basher, Loggerheads wall-turn and bridge-feet reports received software fixes. Any residual W1 needs a reproducible view. Playtest 11 A4 is a separate appearance decision. The plan uses retained native 144 Hz and Node/Wasm camera measurements. Browser GPU frame pacing, visual parity and original device depth state remain unverified. [W1](../playtest/2026-11-15-playtest-7.md) · [Occlusion repair](basher-occlusion-001.md) · [A4](../playtest/2026-11-16-playtest-11.md) · [Native timing inventory](../../build/frame-rate-144-001/timing-inventory.json) · [Wasm timing inventory](../../build/web-perf-001/measurements.tsv)

**Checks.** Static source and document inspection only. The planning change affects zero installed-level routes. Build, replay, game execution and save access were unnecessary. [Document check](../../build/opengl-plan-001/document-check.txt)

- **Handoff** — `needs_review`. Next action is the supervisor's pending `verify.py --profile smoke` gate and independent plan review.
- **What to check** — During review, trace the proposed Kriss Kross (`E19_KF_2`) check. After implementation, rotate past the tank and hover the front hatch. Expect correctly layered actors, water and pipes, a responsive hatch target and smooth rotation with sound.
