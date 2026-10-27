# C2p — runtime counters and pause composition connected

2026-10-27. Campaignattempts5/8, runtime questionattempts1/2. Packets
`build/tmp/c2p-runtime-hud/{round-note.md,repair1/round-note.md}` and the initial
source snapshot remain. **Implementation/own-app verification, not native C2 or
Phase D acceptance.**

Windowed `--sim` now loads the existing font once, reads counters/timer from the
same World as actors and composites the existing HUD. P toggles gameplay pause;
paused ESC resumes. Unpaused ESC still closes our app (B2 dialog incomplete).
No-sim and legacy headless fixture flags keep their previous behavior. Ten skill
buttons/cursor are not newly integrated by this change.

`RuntimeSimState`, called by the actual main loop, suppresses existing World tick
calls while paused. Transitions clear its outer SimClock accumulator and reset the
performance-counter origin; paused wall time is not caught up. Clearing pre-pause
fractional remainder is a declared SOURCE policy, not a measured native timing rule.
World/SimClock algorithms and their parked Phase D fidelity are unchanged. Existing
Out/target/default-color/caption-blend uncertainties remain explicit.

## Facilitator check

Initial helper-only packet lacked stateful pause protection and any mutation;
returned to the worker rather than silently completed. Repair supplies the exact
production state gate, permanent run→pause→elapsed→resume tests and separately
discriminated pause/caption mutations.

Own separate C++ driver, linked against the **current graph's** production objects,
checks two running ticks,20s paused with World/HUD frozen, paused-ESC-equivalent
resume without catchup, and one subsequent tick. It also checks actual caption
padding **RGB**45/60/75 over90/120/150, not merely changed alpha. Own pause-guard
mutation fails `20 paused seconds advanced World`; restore byte-exactly, then
isolated caption omission fails the RGB check. Restored RuntimeHud SHA256
`d8ebd2f5b8eca36375cce926b1e9ada31ecc448b8385de6950e5e0cf4d8f9cdc`.

Independent real SDL-window smoke, only after native worker/resource release:
owned app PID53116, executable path/hash checked, visible SDL_app window and
foreground PID checked before each input/capture. Ordinary scancoded P/ESC input
produces running changes, identical paused frames1.65s apart, paused ESC resume,
a second identical pause pair, P resume, then unpaused ESC clean exit0. Nine640x480
PNGs and exact timestamps/hashes in
`build/tmp/integrated-retail-2026-10-27/hud-window-check-scancode/`.
This verifies our event/composition connection, not original screenshots or native
time equivalence; the independent state driver supplies the longer pause check.

Earlier independent-smoke failures are retained: foreground requests were refused
until the owned console app used a hidden/no-console launch and explicit SDL window
selection; zero-scancode virtual input then failed to deliver the intended SDL P.
Mapped scan25/ESC1 resolved that verification-method defect. No failed smoke is
silently called a production regression or stock crash. Initial checker compile/
stale-object link failures are retained alongside corrected current-graph replay.

Full restored suite/fixed-gate results are recorded at landing. No original game,
debugger/Frida/injection/install change or native memory mutation in this task;
our app smokes do not consume stock starts. All owned apps/input are closed/released.
C2 remains unaccepted until its original no-HUD-exclusion/hover gate passes. Next:
qualified B2 navigation and B3 map, not Phase D clock work or caption pixel fitting.
