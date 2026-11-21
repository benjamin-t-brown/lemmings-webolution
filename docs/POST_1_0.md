# After 1.0: improvements on the original

Requested by the user on 2026-11-15. Deferred until playable v1 is reached (every installed
level won by a person; see agent/IMPLEMENTATION_PLAN.md). Nothing here is queued or authorized
as current work, and none of it is reconstruction: these change or extend the original game.
Order below is the lead's suggestion, not a commitment.

## Presentation and input
- OpenGL renderer in place of the software rasterizer. The user pulled a master plan forward on
  2026-11-18 (opengl-plan-001, plan only; implementation still waits for the user's approval). Expected to remove the minor z clipping
  seen in playtest 7 (W1) and to make higher resolutions cheap.
- Drawing at other resolutions (beyond the current 2x window scale).
- Hardware mouse cursor, so the pointer does not lag behind the system cursor.
- Better fast-forward: selectable 2x / 4x / 8x and a clearer control.
- Custom soundtrack.
- The six FMV clips (frontend, complete, failed, congrats, gameover, credits .avi; user request,
  playtest 25 Q4, lower priority). All are Indeo 5 (IV50) 640x360 at 25 fps with MS ADPCM stereo
  22050 Hz audio (credits has none), 48 MB in total, beside the install and in the CD root. They
  cannot ship, so they come from the player's own files like the boxes. Lead suggestion: decode
  IV50 and MS ADPCM in our own C++ (works natively and in the browser, no re-encode, store the
  originals) rather than transcoding to a smaller codec at import time; locate the original play
  sites statically first.

## New content
- Level editor.
- New lemming types, such as lava lemmings.
- New jobs, such as sliding lemmings and guard lemmings that can fight wombats.
- New mechanics, such as moving platforms.

## Notes for when this starts
- Keep each improvement separable from original behavior (an option or a distinct mode), so
  the fidelity ledger in docs/STATUS.md and the Emscripten goal are not entangled with it.
- New jobs, types and mechanics need a level format that can express them; decide the editor's
  format before adding them. Installed original levels must keep loading unchanged.
- New simulation inputs must stay recordable, so playtest replays keep working.
- If cursor lag or the fast-forward control gets in the way of finishing v1 playtests, either
  can be pulled forward as an ordinary playtest defect.

- Occlusion when a lemming falls close to a surface (playtest 11 A4). The user sees the same clipping in
  the original, so it is not a 1.0 defect; a candidate for the renderer work above.
- An option to invert the right-drag camera direction (playtest 13 C2). 1.0 reverses left/right
  drag by the user's choice; the option would let a player pick either way, per axis.
