# OpenGL renderer request (2026-11-24)

User request: start the OpenGL renderer work now, on its own branch (`opengl`), ahead of the
playable-v1 milestone. This pulls the implementation phases of [OPENGL_PLAN.md](../OPENGL_PLAN.md)
forward; the plan's design, contracts and parity rules apply unchanged.

Standing user decisions (2026-11-18, recorded in the plan): GL becomes the default on both
platforms after qualification; the renderer must not rely on threads; the exact, one-byte and
boundary-review parity rules are accepted for now; the main Windows machine and a budget 2022
ThinkPad on Linux are the hardware references.

Queued in dependency order (phase 2 split in two):
- gl-boundary-001 (phase 1): prepared-frame boundary, software as its first consumer.
- gl-presenter-native-001 (phase 2a): `--renderer software|gl|auto`, OpenGL 3.3 presenter, readback.
- gl-presenter-web-001 (phase 2b): WebGL2 presenter, Start-page choice, software fallback.
- gl-opaque-meshes-001 (phase 3): tube and backdrop meshes in comparison mode.
- gl-scene-passes-001 (phase 4): billboards, secondary sprites, pipes, fluid, gates, sky fill.
- gl-actors-001 (phase 5): actors, attachments and displayed-frame queries.
- gl-effects-001 (phase 6): blast marks, drowning, debris and particles.
- gl-2d-play-001 (phase 7): 2D compositor and ordinary play on GL.

Held: phase 8 (W1 clipping) until the user reports a level and camera pose; phase 9 (measurement,
context recovery, default switch) until the user's hardware runs.

User decision (2026-11-24, after phase 2a): delay the browser until the native GL renderer works.
gl-presenter-web-001 is parked unreviewed (its WebGL2 presenter code stays in the tree; the lead's
browser run in build/gl-presenter-web-001/lead-browser-evidence.md passed), phases 3 to 7 are native
only, and gl-web-001 brings the finished renderer to WebGL2 after gl-2d-play-001.

User report (2026-11-24, after phase 7): native GL has obvious frame-rate problems on the 144 Hz main
machine and is slower than software (user play of M01_KF_1: GL 18.2 ms mean, software 4.9 ms; most GL
time is a GPU wait, not CPU work). The user expects thousands of frames per second. gl-perf-001 pulls the
native performance part of phase 9 forward: measure GL and GPU time, remove every per-frame GPU wait from
ordinary play and bring GL below software. gl-web-001 now follows it. Lead measurements:
build/gl-perf-001/lead/summary.md.

gl-perf-001 outcome (2026-11-25): blocked at its attempt limit, unreviewed work committed in 636a045.
Ordinary play no longer reads back from the GPU and GL beat software on mean and p95 in all 108
workloads, but only 40 met the 2 ms p95; the exact-matching shaders still cost about 2.5 ms of GPU
time, swaps still hitched (one 76 ms frame) and first present took about 1 s against 0.25 s.
User decision (2026-11-25): ordinary GL play uses normal GPU drawing; the exact path becomes a
comparison mode. Queued gl-perf-002 (GL p95 at most 1.5 ms and at most half the software mean in every
workload, GPU p95 at most 0.5 ms, no hitch above 6.9 ms, first present no slower than software);
gl-web-001 follows it.
