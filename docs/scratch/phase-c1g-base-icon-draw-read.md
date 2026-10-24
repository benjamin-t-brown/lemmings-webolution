# C1g: the ten base skill-panel icons are drawn by the skill-panel object, not the `0x4b1774` widgets (Q1 round 1 of 2, offline)

Queue `c1g-base-icon-draw-read`. Worker packet `build/tmp/c1g-base-icon-draw-read/round-note.md` and
`prediction.md`. Offline, capstone on the stock exe (REA unavailable to the worker), no game, no
debugger, no `src/**` change. Dispatched 2026-10-24 00:34:54 MDT, ready 00:58, landed 01:08 MDT.

## Result (all READ unless marked)

- **Draw path.** `[G]==2` arm of the frame function: `0x45E0E6 call 0x441845`; `0x441A6B add ecx,0x27D0`
  / `0x441A71 call 0x45B639` on the panel object `0x004C91D8` (`G+0x27D0`; the `0x4C9208` boxes are its
  `+0x30`). `0x45B639` draws, in order: ten box quads, the selection frame, count text for skills 0..8
  (index 9 armageddon has none), the ten icons, the highlight object (parked). The `0x45E31C..0x45E347`
  region draws only the caption quads.
- **Icon draw.** Each icon: `0x45B74D..0x45B784 call 0x47A75D(icon = this+0x1150+i*0x208, x, y)`; tail
  `0x47AB96` `0x440C30` (texture) then `0x414920`.
- **Cell selection is not `widget_index+8`.** The `+8` field belongs to the `0x4b1774` widgets. Each icon is
  selected by animation name, `0x45A991..0x45A9AF` pushing `[0x4C0694+i*4]` into `0x468AB8`: climb, float,
  bomb, block, build, bash, mine, dig, questionmark, armageddon (strings read from `.data`). Frame
  `icon+0x1CC = row.e8 * nframes / row.ec` (`0x45A9B4..0x45AA18`), table `0x4C06E8` stride `0x14`:
  `e8` = 3,0,0,0,15,0,21,1,0,6; `ec` = 18,15,1,16,16,24,25,20,1,13. `nframes` is a runtime value (UNREAD).
- **Sheet.** `i==9 -> [0x4E3318]`, `i==8 -> [0x4E3314]`, `i==2 -> [0x4E3310]`, else the first entry of
  `0x4E335C[0..3]` that is not -1 (`0x45A94F..0x45A98C`). `0x4E335C` is filled by `sprintf("%s%s")` from prefixes
  `Lemmings`, `LemmingsWater`, `LemmingsAcid`, `EvilRat` and suffixes `Lo`, `Hi`. So the generic skills bind a
  lemming animation sheet. Which variant is loaded: UNREAD.
- **Placement args** to `0x47A75D`: x = 39,37,38,39,40,39,41,37,38,38; y = 40,92,121,158,197,238,279,317,364,410
  (raw, from `0x4C06F0/0x4C06F4`, stride `0x14`). Then adjusted by `y - frame[+0x34]`; the x adjustment and
  the `[0x58D53C]-[0x58CB38]` branch are UNREAD, so on-screen pixels are **not** derived.
- **Counts.** Two ASCII digits `'0'+n/10`, `'0'+n%10`, no zero suppression (`0x45A7B6..0x45A837`, refresh
  `0x45B594`), font object `[0x4CBF7C]`, anchor x=66, y=box top+8 (`0x44DF40(0x42, boxY+8)`). Font file and
  colour: UNREAD.

## UNREAD, named

`0x47A75D` middle `0x47A8A9..0x47AA5F` (x adjustment, UV), `0x468D00`, `0x468CE0`, `0x47ABC0`, `0x4840F3`,
`0x440C50`; loaded sheet variant; writers of `0x4E3310/14/18`; count font file and colour; meaning of
`row.f8` (stored to `icon+0x1308`: 2,4,0,4,2,2,2,4,0,0); the `0x457C00` family bodies. Worker read about 30
bodies against the retired cap of ten; the 30-minute limit was met (about 23 minutes).

## Facilitator check

Re-decoded with my own capstone install (`build/tmp/c1g-verify-pylib`, `read_steppers.py --at`, not the
worker's `disx.py`): `0x45E0E0`, `0x441A68`, `0x45B74D`, `0x45A94F`, `0x45A991`, `0x45A8A5` all match the
cited instructions. Parsed the file bytes directly for the name table, `0x4C06E8` rows, `%s%s`, prefixes and
suffixes: match. My own `.text` scan: `0x441845` and `0x45B639` each have one caller; `0x4E3310/14/18` have
one reference each, all in `0x45A5FB`; `0x47A75D` has callers `0x45B784` and `0x43CD4E` (the second is not
read). Offline round: no mutation applies, no `src/**` diff, no game process, `git status` showed only
`docs/tasks.json`.

**Narrowed.** (1) The "`LemmingsLO` ruled out" finding (C1c) rested on the literal string being absent from the
exe. The name is built with `sprintf`, so absence proves nothing; it is now unproven, not ruled out. Only the
live `icon+0x1F8/+0x1FC` values or the variant flag would settle it. (2) `widget_index + 8` (C1f, MEASURED)
stands as a fact about the widgets and is not the icon selector. (3) "Placement" is the raw argument only.
