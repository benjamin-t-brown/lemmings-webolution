# C1t: Q3 render of the ten skill icons and counts, gate fails (directive part 3, round 1 of 2)

Queue `c1t-q3-panel-render`. Offline. No stock start. Dispatched 2026-10-24 13:22:39 MDT. The Codex worker (`/root/c1t_panel`, session `01a16f5d`) and its facilitator (`01a16f55`) both ended at 13:32:23 with `usage_limit_exceeded`. No `round-note.md`. The code, `prediction.md` (13:24:42, before the 13:30 score) and `build/tmp/c1t-q3-panel-render/score-table.json` were on disk. This note is the landing.

## Result

**Fail.** `tools/harness/score_c1_panel.py`'s `skill_mae` on the saved renders:

| View | Numerator | Pixels | Denominator | MAE | Gate |
| --- | --- | --- | --- | --- | --- |
| `initial` (`MM_CLIMB_002`) | 15157 | 184 | 140760 | 0.10767973856209151 | fail |
| `a7q_mm_dig_000` (`MM_DIG_000`) | 3646 | 86 | 65790 | 0.0554187566499468 | fail |

Both equal the pre-panel figures from C1b. Per-box, only boxes 0–2 have mask pixels (initial 69/78/37, second view 59/11/16); boxes 3–9 have a zero denominator.

`--skill-panel` draws the ten icons from `LemmingsHi.spr` and the Grey/Black/Additive sheets, plus two `_FontSmall` digits, at the C1s positions. The new pixels inside the skill bands span x 26..76, y 19..394 (1847 pixels on `initial` versus `no-panel.png`). The scorer's mask (pixels inside the bands where A7 `baseline.png` differs from `no-cell-routes.png`) is only x 86..89: 184 pixels on `initial`, 86 on the second view. Intersection with the new pixels: 0. The masked error cannot move.

A separate full-band sum against the stock capture `tools/harness/fixtures/camera_a1/original-0.png`, every pixel of x `[18,90)` and the ten 32-row bands (23040 pixels), is 5147553/17625600 = 0.29204980255991286 with the panel, and 5545097/17625600 = 0.3146047226761075 without it. Also a fail. The second view has no separate stock-capture file in the A7q directory; its gate number is the scorer's.

The oval button plates are not drawn. Colour key `0x47218A` and blend `0x46D5F3` stay UNREAD. Icon 9's count is forced to 0 in `drawRetailSkillPanel` (the comment marks `MM_DIG_000` unread). Not tuned.

## What the placement bytes say

Re-read, not a new measurement. `0x47A7D2` is `sub eax, [0x58CB38]` after loading `[0x58D53C]` (the C1h `S`). `0x47A7BB` subtracts frame `+0x34` from y with no scale. `0x47AA53` compares `S` to -1; the -1 arm divides frame `+0x30`, the stored `+0x3c`, and frame `+0x38` by the 2.0 at `0x4B03F8` (`0x47AA5F`, `0x47AA6E`, `0x47AA7D`). `0x47AB2A` subtracts that scaled `+0x3c` from y. `skillPanelRect` uses those edges (`x - dx*scale`, `y - dy - height*scale`, extents `width*scale` and `height*scale`). Frame-index selection and the UV sample were not re-derived.

## Facilitator check

Independent loop over the saved PNGs through `build/tmp/a7p-facilitator/verify.py` `wider()` reproduced both MAEs exactly, including 15157/184 and 3646/86. A second loop counted the x spans above; it does not import `score_c1_panel.py`. `prediction.md` is timestamped before `score-table.json`.

One mutation: `skillPanelRect`'s half scale `0.5` changed to `1.0` and nothing else. `test_skillpanel` then reported 23 checks, 1 failure, the placement check only. Restored from `build/tmp/facilitator-c1t/SkillPanel.cpp.bak`; SHA-256 `8CEF8085D78E6C0BBC4661EFED338B14096E9A4862E4CA84484FFFC753D9C7CB` matches the backup. The restored test is 23 checks, 0 failures. Full `ctest --preset dev-debug` after that rebuild: 34/34. An earlier suite run failed `skillpanel` because the test binary was still the mutated one; that binary was rebuilt before the 34/34 run.

No Lemmings process. No install write. The first 12:59 dispatch stays excluded from the cap. This is round 1 of 2, and the directive stops at the score, so round 2 was not spent.
