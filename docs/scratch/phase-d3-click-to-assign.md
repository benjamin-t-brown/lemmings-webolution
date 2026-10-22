# D3c: click-to-assign wired into `webolution`, offline

Queue id `d3c-click-to-assign`. Round 1. Offline: the new test, CTest, and the
saved A6/A7 shots. No stock start. Worker note:
`build/tmp/d3c-click-to-assign/round-note.md`.

## Question

Can `--sim` pick a live lemming from that lemming's own on-screen sprite box
through the already-ported `pickHoveredLemming`, and call `World::assign`?

## Answer

The pick half is tested. `TubeScene::injectedWalkerScreenBoxes` returns one
optional placement per injected walker, from `prepareInjectedWalker`, the same
helper `drawInjectedWalkers` uses before `drawActorSprite`. Draw still walks
in reverse and still passes the same frame, texture, placement, mirror, tint
and `alphaTest=true`. The accessor walks forward and does not draw.

`test_clicktoassign` loads `MM_CLIMB_002`, ticks 25 steps, and with the A1
camera fixture reports two alive lemmings. A cursor box equal to lemming 0's
own box selects that lemming. The same box shifted +10000 in x selects nothing.
`priority` and `distanceSquared` are 0. That is D2a's named tie limitation,
not a reading of `0x004C05D0` or `screen+0x58`.

`World::assign` is called from `debugClickToAssign` in the window loop, and
only when `--sim` is on and the focused window receives a left click. Keys
1–8 select a debug skill in `sim::Skill` declaration order. That path was not
executed: every render in this round was `--shot`, which returns before the
window loop. The debug keys are not the original skill panel.

A6 and A7 PNG bytes are unchanged. `--sim-ticks` 15 and 75 stderr are
unchanged. 28/28 tests pass.

## Facilitator check (2026-10-22)

Independently hashed the four saved PNGs: A6 before and after are both
`c4e6cf477e7f803d92329e5b223fe296b014801abb0552132d65133fb1a09ebf`, A7 before
and after are both `78305c5c4911cb55071cd21fa982d99764aafd951b183e6315b5bef691f77a72`.
Those are the hashes already recorded for D3b. The saved `--sim-ticks` 15 and
75 stderr files are byte-identical before and after (116 and 355 bytes).

Re-read `0x0045578C`. The bytes are `6a 20 6a 20`: two `push 0x20` instructions,
the 32×32 sprite size `Pick.h` already names. This round adds no new original
measurement.

Ran `test_clicktoassign` with `WEBOLUTION_GAME_DIR` set. It printed tick 25,
two alive lemmings at (44,63) walker and (32,57) faller, boxes
`[385,292,397,310]` and `[313,270,325,288]`, and `chosen=0 far=nullptr`. A
direct run without that environment variable skips, which is the existing
archive-test behavior, not a pass of the pick.

Own mutate-one-alone: replaced `boxes[i] = prepared->placement` with
`boxes[i] = std::nullopt` in `injectedWalkerScreenBoxes` only. The test
failed with `FAIL: each alive lemming has a screen box`. Restored
`src/app/TubeScene.cpp` to SHA-256 `ec3a3afc9043855b31775f7f5f92c88b29612671ed1c4a3b97f9ac85fe981874`
and the same test passed again. The cursor in this test is built from the
returned box, so a coordinate tweak would not have failed it. The empty-box
mutation is the discriminator. Full `ctest --preset dev-debug`: 28/28.
No Lemmings Revolution process was started.

Narrowing: the landed acceptance test does not call `World::assign`. The call
is present in the unexecuted window handler. D3's playable-level gate is not
claimed. D2's live 20-cursor gate is still open.

**Landed.**
