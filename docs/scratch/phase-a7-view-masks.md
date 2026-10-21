# A7m saved pair is not the retained initial view

2026-10-21. Capture round, blocked on the start budget and then narrowed.
The restated A7 gate is unchanged. Eight views and `MM_DIG_000` still have
no retained-view mask.

## What was saved

[A7m view masks](0eaaf76b-606d-4e2b-9e23-47706d37a93b) launched the stock game
four times (PIDs 63180, 62032, 34140, 16700). The brief allowed two launches
and no retry. The pre-launch durable check matched
`2624993/135798210`.

The folders `initial`, `mouse`, and `right` each contain the same three PNGs.
Baseline and restored SHA-256
`b80f4a06442bc14df243490d5eb34083ced83157d9f1c1f42d595b590e2b79f2`.
Suppression SHA-256
`acaa0225dfc7f161319f7ce11898cd97a9182900b919a07ac61170799206bbda`.
`left`, the five zoom-branch views, and `MM_DIG_000` have no pair.

After the initial shot the paused camera block differed in two doubles:
`+0x50` `-266.94415283203125` to `-338.4447326660156`, and `+0x60`
`266.9440612792969` to `338.4446105957031`. The log marks
`camera_held_after_shot` false on that shot.

## Scores

Fixed ROI `[100,80,280,410]`. The fresh baseline against both
`tools/harness/fixtures/camera_a1/original-0.png` and the durable A2 baseline
is `6117452/45441000 = 0.1346240619704672`. The production tube-only render
against those same two references is `708159/45441000 = 0.015584142074338152`.

The worker row `7444647/84031425 = 0.08859360649899725` (N `109845`) is that
production render scored on the fresh pair, with caption `[277,422,363,454)`
removed. The fresh difference bbox is `(136, 9, 503, 337)`. The durable mask
bbox is `(86, 24, 553, 450)`. This pair does not cover the lower band.

The same production render on the durable caption-excluded mask remains
`2624993/135798210 = 0.019330100153750186`.

The worker rows for `mouse` (`0.09137965945478135`) and `right`
(`0.08729306922975541`) use this same baseline. They are three candidates
on one frame.

## Facilitator check

`build/tmp/a7m-facilitator/verify.py` recomputes the hashes, both ROI scores,
both caption-excluded scores, and the two moved doubles with Pillow histograms.
File prologues at `0x4622FD`, `0x4623A5`, `0x462404`, and `0x462465` read
`55`. Install SHA-256 values match the log: exe
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
`SavedGame.dat`
`f7f8603928890fb13d022d828c8a602cf7a7619ab24de82c7de4ae86ec337e6a`,
`dgVoodoo.conf`
`d639e220814d80b7956b96bad0ca708b768458bd348bfd607afbf1913598e72b`.
No stock or `webolution` process was running at the check. No production edit.
Build had no work. CTest 24/24.

## Left open

A later capture has to reach each retained camera by the measured A1 input
path. A baseline whose fixed ROI against that view's retained original is
above `0.02` is not a gate pair. That round needs a new stock-start
authorization. The raw 360-byte camera write is the method that drifted here.
