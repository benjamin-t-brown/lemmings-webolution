# Exit balloon save target

[Playtest 25 Q13](../playtest/2026-11-21-playtest-25.md#requests) requested this
clone addition. The previous `materialize` path drew switch, lemming and hatch
feedback, so pointing at the exit in **Just Climb** (`MM_CLIMB_002`, node 0)
gave no save target. **Swings Both Ways** (`MM_E1_5_SKILLS`, node 23) checks
the two-exit case. Difficulty is routine and the recorded recommendation is Sonnet.

## Production path

`runApplicationLeg` applies the current animated exit pose before `renderFrame`.
`materialize` then calls `BackdropScene::exitHoverTarget` on the completed
scenery depth. The lookup uses the mesh of each placed `entities_426c6b` exit
root, including the envelope, ropes and basket as posed by existing bob and
departure sway. It reuses the hatch's provisional half-open rectangle of
depth-matching pixels with a 0.5 world-unit tolerance. A completely hidden exit
has no target. For overlapping exit rectangles, the first visible match in
stream order supplies the anchor.

`materialize` passes `world->setup().saveTarget` to `drawExitHover`, which
uses the hatch's white HUD font and centres the number just above the visible
model. It places the label below if the top margin is too small. The number
is the level total that `World::won()` compares against saved lemmings, so both
exits show the same value. Lemming and hatch picks and any switch hover take
precedence over the exit label. The label shares the switch's focus, pointer,
menu, drag and panel guards and the existing mouse-down paths handle skill and
hatch presses. Both pause modes compose the label. [Task diff](../../build/exit-hover-count-001/task.diff)

## Static reference

Ghidra 12.1.4 opened the registered x86 EXE, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The XREF at `0x45E311` reaches `0x454A85`. Its pseudocode maps the numeric
overlay to the hatch branch, and the instructions at `0x454B86..0x454B9C` show
the hatch pointer at screen `+0x1894` and the argument from hatch `+0xA0`.
`0x454C34..0x454C64` positions that text. Retained exit instructions at
`0x426AC0..0x426B09` and `0x426993..0x426A03` support the whole-model bob
and departure tilt. The exit label and its placement are clone presentation
choices. REA closed without a snapshot. [Static packets](../../build/exit-hover-count-001/rea-static.json)
· [Bob](exit-balloon-bob-001.md) · [Tilt](balloon-gyrate-001.md)

## Clone evidence and checks

The clone's SDL frame observer drew these 640×480 images with the installed
data and a private LocalAppData. The probe read `COMPILED\LEVELMAP.DAT` and
verified its save target against the World on every frame. The pixel audit
matched each label's glyph shape across play and both pauses. It also checked
pointer departure, suppression during examine drag, return on release, and
identical glyphs on both exits. [Pixel audit](../../build/exit-hover-count-001/frame-audit.log)
· [Single-exit run](../../build/exit-hover-count-001/frames-MM_CLIMB_002.log)
· [Two-exit run](../../build/exit-hover-count-001/frames-MM_E1_5_SKILLS.log)

| Installed level | Target | Running | Paused |
| --- | ---: | --- | --- |
| Just Climb, yaw 240 | 1 | [Hover](../../build/exit-hover-count-001/frames/just-climb-play-exit-1.png), [away](../../build/exit-hover-count-001/frames/just-climb-play-away.png) | [P](../../build/exit-hover-count-001/frames/just-climb-p-exit-1.png), [examine](../../build/exit-hover-count-001/frames/just-climb-examine-exit-1.png), [drag](../../build/exit-hover-count-001/frames/just-climb-examine-drag.png) |
| Swings Both Ways, yaw 128 | 16 | [Hover](../../build/exit-hover-count-001/frames/swings-both-ways-play-exit-1.png), [away](../../build/exit-hover-count-001/frames/swings-both-ways-play-away.png) | [P](../../build/exit-hover-count-001/frames/swings-both-ways-p-exit-1.png), [examine right](../../build/exit-hover-count-001/frames/swings-both-ways-examine-exit-1.png), [examine left](../../build/exit-hover-count-001/frames/swings-both-ways-examine-exit-2.png), [examine away](../../build/exit-hover-count-001/frames/swings-both-ways-examine-away.png) |

- The registered UCRT64 GCC built `smoke_tests`. All 95 main smoke-labelled
  tests passed, including the 102-level sweep. The windowless legacy
  `exit_hover` test passed in 0.12 s, covering two exits, bob, sway, half-open
  bounds and full occlusion. [Build](../../build/exit-hover-count-001/legacy-build.log)
  · [Main smoke](../../build/exit-hover-count-001/main-smoke.log)
  · [Legacy](../../build/exit-hover-count-001/legacy-exit-hover.log)
- All 29 promoted recordings reproduced with
  `webolution --game-dir "Lemmings Revolution" --replay <file>`. Each finished
  with exit 0, `PASS won=1 completed=1`. The table gives every recording, step,
  saved/required count and command log. [Per-recording results](../../build/exit-hover-count-001/native-replays.tsv)

Changed `src/app/BackdropScene.{h,cpp}`, `src/app/HoverFeedback.h`,
`src/app/RuntimeHud.cpp`, `src/app/main.cpp`, `cmake/webolution_tests.cmake`,
`src/__test__/test_exit_hover.cpp`, and this report. Human confirmation and
browser interaction remain unverified.

**Handoff.** `exit-hover-count-001` is **needs_review**.<br>
**Next.** Supervisor smoke profile, including recovery, is pending. Independent review and human playtest follow.

**What to check.** In **Just Climb**, rotate until the blue balloon is visible
and hover it in play, after P and after Enter. Expect **1** above the balloon
and a clear view when the pointer moves away. In **Swings Both Ways**, turn
until both balloons are visible and hover each in examine pause. Expect **16**
only over the balloon under the pointer.
