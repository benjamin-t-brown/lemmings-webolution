# C3c: Entrance.a3d and Exit.a3d placement read (Q2 round 1 of 2, offline)

2026-10-23. Directive ac03584 Q2. Worker note `build/tmp/c3c-entrance-exit-read/round-note.md`, scripts `c3c_decode.py` (stdlib, re-runnable), `ref_measure.py` (Pillow). No start, no debugger, no `src/` change. REA not used (none on the Mac); capstone reads via `x86read.py`. Level `MM_CLIMB_002.LVL`, camera `a6d3-matched-state-bbox/live-camera-params.json` (the A6d24 camera).

## Result

| Value | Entrance | Exit | Tag |
| --- | --- | --- | --- |
| `Object 09` / 12 doubles (LVL file offsets) | `0x9ccc4` / `0x9ccd5` | `0xa0cd0` / `0xa0ce1` | READ |
| rotation | `[[-.6258,0,-.7800],[0,1,0],[-.7800,0,.6258]]`, det -1 (reflection) | `[[-.6078,0,.7941],[0,1,0],[.7941,0,.6078]]`, det -1 | READ |
| translation | (-35.5469, 44, 35.5469) | (49.0620, 40, 42.3539) | READ |
| `Top` node vertices | 8 (`cube8`, `cube9` 8 each; `cube13` 0) | 137 | READ |
| `Top` projected, 640x480 | x 302.3-338.3, y 234.6-260.9 | x 72.2-87.1, y 157.7-236.1 | DECODED (node-local transform UNREAD) |
| textures | `Models\entrance\lem_box_default{,_side,_door}` | `Models\exit\{basket,rope,balloon_default_corded}` | READ strings |
| reference blue bbox | hatch [303,235,338,276] | balloon [50,167,92,235] | MEASURED-FROM-REFERENCE (coarse colour test) |
| draw order, door/count state, kind-6 record stride | | | UNREAD |

Entrance: left, right and top edges of the projected `Top` agree with the reference within 1 px, nothing fitted; the legs (to y 276) come from no decoded node. Exit: vertical extent roughly agrees, horizontal is about 15 px projected against about 40 measured; cause unchecked (node-local transform after the vertex array, or other geometry). Hatch loader `0x425e50` reads the model as sub-object `hatch+0x44`; animation state needs a memory read of `[hatch+0xAC]`, `[hatch+0xC0]` and the door fields behind `[[hatch+0x44]+0x88]` at the pause. Door-open at the pause is INFERRED (D1b plus a lemming in frame), not read.

## Facilitator check

Re-read on mouv with `read_steppers.py`: `0x4749DD` (`mov eax,[0x4b1a58]`, the "Object 09" push), `0x425E6A` (`mov edx,[ecx+0x44]`, `cmp [edx+0x88],0`), `0x425FE6`/`0x425FF2` (`+0xc0`/`+0xc4` stores), `0x426023` (`call [edx+0x34]`). Matches. Own 40-line projection (`/tmp/c3cfac/proj.py`, a fresh port of `buildCamera` and `TubeScene.cpp::project`, not the worker's script) reproduces both projected boxes to two decimals and both Object 09 doubles from the LVL bytes. Mutation: using the pose instead of its transpose (the convention `project()` does not use) moves the Entrance to x 522-550, so the agreement depends on the convention. Own Pillow loop reproduces both reference boxes. Narrowed: "Entrance consistent" is a 1-px edge agreement on a coarse blue threshold and covers three of four edges and one node; the exit window is clipped at x=48 and the tube edge, so its x0 is a floor and its reference width is not clean; the worker's ordering note (prediction written after reading the translation bytes and strings, before any decode) stands. No game process on mouv; no `src/` diff, so no ctest needed. Not verified: the 4 partial function bodies.
