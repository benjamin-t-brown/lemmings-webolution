# A7n input path discarded all eight baselines

2026-10-21. One stock start of `MM_CLIMB_002`, pid 23460. Dispatch
`79ec7415-8a19-41c4-a6fa-aea56e3ed819`. The prediction file was written at
01:20:42 and the launch was 01:23:19. The prediction was a pass for every
view. The discard rule was an ROI numerator above 908820 on denominator
45441000. Every view hit that rule. No route prologue was patched. There is
no caption-excluded wider-mask score. The restated A7 gate is unchanged.

## ROI against the retained original

Half-open ROI `[100,80,280,410)`. Denominator `180*330*255*3 = 45441000`.

| View | Numerator | MAE |
| --- | ---: | ---: |
| mouse | 6804746 | 0.1497490372130895 |
| right | 7028355 | 0.15466990163068595 |
| left | 6476525 | 0.14252602275478093 |
| canonical | 6965204 | 0.1532801654893158 |
| zoom1 | 6626362 | 0.14582341937897494 |
| elev_up | 10623185 | 0.2337797363614357 |
| elev_down | 21855840 | 0.48097180959926056 |
| elev0_neg | 24405373 | 0.5370782553200854 |

The paused frame before the unpause, `banner-check.png`, scores
`10956868/45441000 = 0.2411229506392905` against `original-canonical.png`.
`banner-check-after.png` is byte-identical to `canonical/baseline.png`
(SHA-256 `9f44ff9cd05038b474af8a63ec84a777a9de0f75ce6aef4f20ee9803aff1e5a8`).
`elev_down/baseline.png` and `elev0_neg/baseline.png` are one file
(SHA-256 `dc8443309d4631cd5f3ebe39c1ff5daf35203a9d19b54ee1c12f0b9127d056ba`);
their MAE figures differ because the retained originals differ. The eight
folders hold seven distinct baselines. Right versus canonical inside the ROI
is `2156669/45441000 = 0.04746086133667833`.

The capture log records grabbed NUMPAD6 moving yaw `+0x40` from 32 to 28,
and NUMPAD4 moving it from 32 to 36. Those two baselines are the prescribed
key cases, and both fail. The mouse view's logged dx 24 left yaw at 32; the
saved mouse baseline was taken at yaw 22 after later numpad steps. The logged
zoom i32 is `control+0x0C`. `0x45CC61` is `mov ecx, [eax+0x10]`, which is the
measured zoom field. The zoom and elevation rows did not observe that field.
The logged elevation at `+0x18` stayed 18.5. The 360-byte block at `0x4CBFC8`
was not written and was not saved. The retained canonical params file records
camera SHA-256 `7e6e1a8a77c29f860f82dd8cde3f60c9f89ecf3f1352d3c2a6801cc9867d0fff`.
The log's canonical hash is `2b33a7083df4f7524ee245ed4d1c832f29174038e9151c6a607900eb22d3b5b5`.
The differing offsets were not compared.

## Facilitator check

`build/tmp/a7n-facilitator/verify.py` recomputes all eight numerators, both
banner scores, and the right-versus-canonical numerator with a Pillow
histogram. Scoring a retained original against itself returns numerator 0.
A black-filled 6px x shift of the canonical baseline scores
`7196538/45441000 = 0.158371030567109`. The round note's ±6 figure
`0.1502430624326049` and its mean-bias residual `0.1017641380435627` were
not reproduced. File prologues `0x4622FD`, `0x4623A5`, `0x462404`,
`0x462465`, and `0x43A4EC` read `55`. `state.json` `writes` is empty and no
`no-cell-routes.png` exists. Install SHA-256 matches the pre-launch set: exe
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
`SavedGame.dat`
`f7f8603928890fb13d022d828c8a602cf7a7619ab24de82c7de4ae86ec337e6a`,
`dgVoodoo.conf`
`d639e220814d80b7956b96bad0ca708b768458bd348bfd607afbf1913598e72b`.
The save and conf backups match those install files.
`savedgame_restored` is false because the save already matched its backup.
No stock or `webolution` process was running. No production edit. Ninja had
no work. CTest 24/24.

## Left open

Do not repeat this eight-view input list. Left and right already missed 0.02
on the prescribed keys. Another stock start is not authorized. A later
capture has to read zoom at `control+0x10` and keep the camera-block bytes.
This packet cannot name the differing offsets.

## Facilitator diagnosis (hypothesis)

The pause came about 1 s after entry (timer 04:59). The logged camera hash
`2b33a7083df4f752…` differs from the reference `7e6e1a8a…` that A6f repair 4
reached after a 14 s unpaused walk. A7m's drift likely has the same cause. A
start check before any view input is the A7o test
(`build/tmp/a7o-settled-camera-masks-brief.md`).
