# C3b: backdrop scenery read (Q1 round 1 of 3, offline)

2026-10-23. Directive ac03584 Q1. Worker note `build/tmp/c3b-backdrop-read/round-note.md`; `decode_backdrop.py` (stdlib plus `tools/proto/box.py`), outputs `decode_output.txt` (`MM_CLIMB_002`) and `decode_output_MM_DIG_000.txt`. No start, no debugger, no `src/` change. REA not used (none on the Mac); pefile and capstone. 5 of 8 bodies read.

## Result

| Value | Tag | Source |
| --- | --- | --- |
| the non-tube scene is model-driven and LVL-embedded; `lemmings.box` has 0 `.A3D` entries | DECODED | script |
| first `Object 09` `0x59637`; root `thenull` `0x59648`, 933 vertices, 795 records, 340 blocks, texture slots sand_1, box2, sand_2, matches, wood1, sky, hose, none; stored matrix diag 10, translation 0 | READ (LVL bytes) / DECODED | script |
| small nodes `sand7` (64 v, `0x8ad4c`), `base` (64 v, `0x8e220`); billboard, sprite and water nodes after them | DECODED (partial for the tail layout) | script |
| frame method `0x441845` calls scene-root dispatch `0x437ba3` (`this+0x5140`) at `0x441879`, then tube walker `0x43a4ec` at `0x441884` | READ | exe |
| depth interaction between backdrop and tube, blend/alpha/Z state | UNREAD (no ZENABLE/ZWRITE/ZFUNC found) | |
| root effective world transform, node-walk order, texture-slot to pixel assignment | UNREAD | `0x437d48` (callback `0x444400`), `0x47d0c0`, matrix stack `0x58D5A0` |
| floor ring | **UNBOUNDED**: no green or purple texture name; a vertex-colour census (not saved in the script, not re-runnable) found no green-dominant vertices; not evidence the ring is there | |

The second level parses with the same layout (`MM_DIG_000` root `null2` `0x59891`, 1167 vertices), so the node format generalises. Projections the script prints at scale 1 and 10 with zero translation are hypotheses, not screen bounds.

## Facilitator check

Re-read on mouv: `0x441870..0x441889` (`call 0x437ba3` at `0x441879` before `call 0x43a4ec` at `0x441884`) and the `0x437ba3` prologue. Re-ran `decode_backdrop.py` offline: output identical to the saved file. Own scan of the LVL: four `Object 09` at `0x5963b`, `0x91779`, `0x9ccc8`, `0xa0cd4`; root matrix diag 10 with zero translation at `0x59648`; `nv = 933` at `0x596cb`; the seven slot textures found as strings. Narrowed: the order claim is call order only; the "draw-order index" in the script is file order; the census is not reproducible and is not used. The script's default camera is `a6f-fresh-pause-gate/repair4/live-camera-params.json`, not A6d24's file (not compared; the projections are hypothesis-only anyway). The `lemmings.box` zero-A3D count was not independently re-counted. No `src/` diff, no game process on mouv.
