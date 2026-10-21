# A7p eight views pass; zoom1 does not

2026-10-21. One stock start of `MM_CLIMB_002`, pid 26648. Dispatch
`018c156f-6ebf-4c38-8c7a-87d22bf6d9bf`, Cursor `round-agent`, requested
`grok-4.7-high-fast`. The unpause read `ui_state` 0 and the pause after
`time.sleep(14.0)` read `ui_state` 1. `tap` returned `None` and the script
did not branch on that return.

## Start check

The post-walk 360-byte block at `0x4CBFC8` is
`7e6e1a8a77c29f860f82dd8cde3f60c9f89ecf3f1352d3c2a6801cc9867d0fff`.
The canonical frame against `original-canonical.png` on ROI
`[100,80,280,410)` is `156/45441000`. The entry-pause block, taken before
the walk, is `0d03f6d4…` and is not this check. The log records
`0x4CBAAC` as 300 before the walk and 286 after it. Those dwords were not
re-read; the established unit is whole seconds remaining.

## Wider scores

Mask: pixels where `baseline.png` differs from `no-cell-routes.png`, minus
caption `[277,422,363,454)`. Score: `render.png` against that baseline.
Pass means `numerator * 50 <= denominator`.

| View | ROI numerator | Wider numerator | N | MAE | Pass |
| --- | ---: | ---: | ---: | ---: | --- |
| initial | 0 | 2616762/135529695 | 177163 | 0.01930766537916285 | yes |
| canonical | 156 | 2630102/135529695 | 177163 | 0.019406093992906867 | yes |
| mouse | 53632 | 2359764/133517745 | 174533 | 0.017673785608047828 | yes |
| right | 48632 | 2379604/134491590 | 175806 | 0.017693329374721498 | yes |
| left | 49151 | 2401793/134977365 | 176441 | 0.017794042727089836 | yes |
| zoom1 | 65575 | 5473596/228864285 | 299169 | 0.023916339764415404 | no |
| elev_up | 49270 | 3929786/208018800 | 271920 | 0.018891494422619493 | yes |
| elev_down | 103465 | 3892765/210690945 | 275413 | 0.018476185580732954 | yes |
| elev0_neg | 15014 | 2381892/134797590 | 176206 | 0.01767013787115927 | yes |

Fixed-ROI denominator is 45441000. Every ROI numerator is at or under 908820.
`zoom1`'s saved camera hash is
`c85171dc542099765b4e6d770d8e0b5a82da0640d94f28fa323167a37aa86b07`,
the hash in `params-zoom1.json`. Against the post-walk block it differs at
offsets 86, 91, 92, 93, 94, and 102. Offset `+0x10` in that block is 0 on
both. `elev0_neg`'s saved block is byte-identical to the post-walk block,
which is the hash `params-elev0_neg.json` already records.

## Skill-panel intersection

`state.json` `panel_boxes` is ten rectangles, object `0x4C9208` stride
`0x1B0`, x `[18,90)`, y bands `[12,44)` through `[372,404)`. Those
rectangles were not re-read from the running game. On the zoom1 mask they
contain 21600 pixels and 1738618 error units. The mask outside them is
`3734978/212340285`; `3734978 * 50 = 186748900`, which is within
212340285. The largest passing numerator on the full zoom1 mask is
4577285, so the full numerator is 896311 over that line. The rectangles
were not removed from the score. No timer rectangle was read, and no timer
pixel was excluded.

## Facilitator check

`build/tmp/a7p-facilitator/verify.py` recomputed every ROI numerator and
every wider numerator, denominator, and N above. Scoring the initial
baseline against itself on its mask returns numerator 0. A 6px x shift of
the zoom1 render raises that numerator from 5473596 to 8709817. File
prologues `0x4622FD`, `0x4623A5`, `0x462404`, `0x462465`, and `0x43A4EC`
read `55`. The write log's last prologue rows read back `55`. `0x43A4EC`
is absent from the write log. Install SHA-256 matches the A7n set and the
backups. Launcher stderr is one Pillow `getdata` deprecation warning.
Launcher exit is 0. Pid 26648 is gone. No production edit. Ninja had no
work. CTest 24/24.

## Left open

The restated A7 gate does not pass. Eight of the nine views pass on their
own masks. `zoom1` fails, and there is no second level. Dropping the
skill-panel rectangles would be an exclusion change. Matching those pixels
is the in-level HUD, which is Phase C. The standing run stops here.
