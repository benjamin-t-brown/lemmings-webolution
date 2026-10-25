# C1bf: zero-count text skipped; frozen C1 gate still fails

2026-10-25, `c1bf-zero-count`, OpenCode round-agent session
`ses_e8be216d8ffe04fzmirQtZ5aGo`. Dispatched 11:30:58 MDT, worker idle
`succeeded` 11:35:49. Landed 12:05 MDT after the OpenCode facilitator
`ses_e8c346bc4ffe01LaYRaXoqOAkl` failed at 11:45:02. Worker packet:
`build/tmp/c1bf-zero-count/round-note.md`. Zero stock starts. This adds only
the native zero-count text skip.

## What changed

`drawSkillPanelCounts` now skips a count of 0 before it builds glyphs. The
native site is `0x45B6E4` `cmp dword ptr [edx+ecx*4], 0` / `0x45B6E8` `je
0x45B6FF`, which skips `call 0x44E150` and rejoins. The loop bound is `cmp
[ebp-8], 9` at `0x45B6CC`. Plates and icons are outside that branch. The
permanent test uses an opaque font: count 0 leaves the 12x14 destination
unchanged, and counts 1 and 10 still ink both digit cells.

## Independently reproduced scores

Frozen footprint SHA-256
`e96840e02c3046705c5d1dd19e947a47d7ea4aa65edbe34e31cccafd44b76892`,
23166 pixels, denominator 17721990. Checker:
`python build/tmp/c1bf-facilitator-check.py` (harness `channel_mae`, not the
worker scorer). Fresh restored renders match the worker PNGs byte for byte.

| Gated view | Numerator | MAE | <=0.02 |
|---|---:|---:|---|
| zoom1 | 414292 | 0.02337728437946303 | fail |
| elev_up | 784866 | 0.04428769003932403 | fail |
| elev_down | 415022 | 0.02341847614178769 | fail |

C1bd numerators on the same denominator were 609151, 981264 and 610758. All
three improved. None passes. No footprint or gate change.

## Facilitator check

Re-disassembled `0x45B6CC` for 0x50 bytes. The compare, the skip, and the
`0x44E150` call are the bytes above. Exe SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

Removed only `if (count == 0) continue;`. `test_skillpanel` then reported
35 checks and 1 failure, exactly `count zero leaves ink-capable destination
pixels unchanged`. Own mutant zoom1 is SHA-256
`144c6f925489842e624607c8cf2b07191952b19b156c87c889799243dc7e971b` and scores
609151/17721990 = 0.03437260713949167, the C1bd zoom1 figure, worse than
414292. Restored source SHA-256
`1067b72ee6fd354aa11b2c6d6c76c608757dfc8d783a064111880faa3114de39`.

The first full CTest after that restore ran the mutant binary: the backup's
timestamp was older than the object, so ninja reported no work and skillpanel
failed. Touching the restored file rebuilt it. The second CTest is 34/34.
No Lemmings process was left running. Selected-plate and filter state were
not changed.
