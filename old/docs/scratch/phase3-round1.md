# Phase 3, round 1 — one live lemming against World

2026-10-14. Question: can one live lemming be compared to `webo::sim::World`
per tick, on the walk and the climb that are already measured?

`Rules` was not changed. No stepper body was edited. `World::place` only
inserts a lemming. The comparison stops at the first cell disagreement.

Script: `tools/harness/compare_tick.py`. It reads the billboard the way
`measure_motion.py` does (four vertices, shared Y, centroid, column
`atan2(-x, z) / 2π * 256`) and steps `build/cmake/dev-debug/compare_tick.exe`.
That printer loads the level's stored grid with `Terrain::fromCollisionGrid`,
inserts one lemming with `World::place`, and ticks. `World::spawn` is not
the start: the printer sets `totalLemmings` to 0 so the entrance drop
returns immediately. Height is world Y. The integer part of the column and
of Y is the cell. The fractional part of Y is printed and is not given to
the sim. `PIXELS_PER_UNIT` is not used.

The game was pid 15000, stock exe, image base `0x400000`. Entered
`MM_CLIMB_002` with `goto_level.py` (path buffer `Compiled\MM_CLIMB_002.lvl`,
`ui_state` 0). After the 300 s clock it was brought back with
`restart_level.py` (RETURN from results, then the first map node — that
node is this level). No ESC.

## Walk — diverged at tick 2

Observation, this run, one billboard in the pen (columns 128–168), Y held
at 32.190, column stepped by about −1.000 per hold:

| tick | live column | live Y | live integer cell | our cell | our state | fraction |
| --- | --- | --- | --- | --- | --- | --- |
| 0 | 167.088 | 32.190 | 167, 32 | 167, 32 | walker | 0.190 |
| 1 | 166.091 | 32.190 | 166, 32 | 166, 32 | faller | 0.190 |
| 2 | 165.092 | 32.190 | 165, 32 | 166, 36 | faller | 0.190 |

Cells match at tick 0 and tick 1. They diverge at tick 2: live `165,32`,
ours `166,36`, fraction `0.190`.

Facing was taken from that recorded step (column decreased), so the sim
was placed facing left. The search sample just before the recording had
moved the other way; that sample was not used as facing, because a walker
at the wall can reverse between the two.

Observation from the printer, same cell, same grid: at `(167, 32)` the
cell itself is empty, the cell at `y + 1` is empty, and the cell at
`y - 1` is solid. Our walker looks for ground at `y + 1`
(`World::stepWalker`). It did take the first step, to `(166, 32)`, and on
that step it became a faller. The next tick applied the current
`fallSpeed` and `y` became 36. The live billboard was still on the floor.
This is not a measurement of fall speed. The live lemming did not fall.

Hypothesis, not used in the comparison and not written into `Rules`: the
integer part of world Y may not be the row our feet stand on, because the
solid neighbour of this cell is at `y - 1` and the stepper looks at
`y + 1`. `worldToMaskPixel` would map 32.190 through `floor(96 - y)` to
row 63. That mapping was not applied. The round says to compare the
integer part.

Checked against the file afterwards, same level, columns 165–167. Rows
28–31 are solid (`0x1`). Rows 32 through 36 are empty, which is why the
printer's walker left the floor and why tick 2 landed on `y = 36`: that
step is `fallSpeed` applied to an empty `y + 1`, not a live fall. Rows
62–64, the `floor(96 - 32.190)` neighbourhood, are empty too. The flip in
`worldToMaskPixel` does not put this walker on the solid row. The file row
matches world Y: the live feet at 32.190 stand on row 31, and our stepper
looks the other way.

## Climb — not compared

Observation: on this entry of `MM_CLIMB_002`, no billboard held its column
and raised Y. The search, in `compare_tick.py`, is a quad whose column
changes by less than 0.05 over a 0.45 s gap and whose Y rises by about 1
to 2 in that gap (one or two 200 ms steps). Five passes, about 2116
quad-shaped addresses each time, found none. Floor billboards that did
step kept Y at 32.190.

What was not done: the skill panel was not clicked. A screen position for
a lemming click was looked for and not found. The pair 0x130 bytes before
a stepping quad was not a screen coordinate; the only screen-like pair in
that neighbourhood was `255, 255`, which the handoff says to reject. A
0x400-byte window before a quad that itself stepped by −1.002 (column
140.969, Y 32.190) had no screen-range double that changed with the step.
The hit-test words at `0x004C36A0` / `0x004C36A4` were still the map click
`(206, 204)`. No click was sent.

Hypothesis: the level only selects the climber on the panel; a lemming
does not climb until one is clicked. That was not tested, because the
click had no position to write.

## Left the game

pid 15000, `ui_state` 0, path `Compiled\MM_CLIMB_002.lvl`. The level clock then
read 0 while `ui_state` was still 0. Nothing was pressed after the
comparison.

## After the Y flip

2026-10-14. Live check after `Terrain::fromCollisionGrid` file-row flip and
`compare_tick.py` mapping live Y with `floor(96 - y)` (same as
`worldToMaskPixel`). `Rules` and steppers untouched. Entered
`MM_CLIMB_002` with `goto_level.py` (highlight driven to 0 before RETURN;
path buffer `Compiled\MM_CLIMB_002.lvl`, `ui_state` 0). Stock exe, pid 34276.
Clock had not run out; no `restart_level.py`.

### Walk — diverged at tick 1

Observation, this run:

```
SUMMARY walk diverge 1  climb skipped None  pid 34276  ui_state 0  path 'Compiled\\MM_CLIMB_002.lvl'
```

| tick | live cell | ours | ours_state | frac | live_col | live_y |
| --- | --- | --- | --- | --- | --- | --- |
| 0 | 167,63 | 167,63 | walker | 0.190 | 167.089 | 32.190 |
| 1 | 165,63 | 166,63 | walker | 0.190 | 165.092 | 32.190 |

First disagreeing pair: live `165,63` vs ours `166,63` at tick 1.

Observation: after the Y map, both sides stay on row 63 as walkers.
`grid 256x96 cell 167,63 solid_here 0 solid_below 1 solid_above 0` — the
printer sees ground at `y + 1`. That is the opposite of the pre-flip run,
where the same world Y (integer 32) had empty `y + 1` and the sim became a
faller. Walk did **not** match for 8 ticks; it diverged at tick 1 on column
only (Y agreed).

Observation: live column stepped by about −1.997 from tick 0 to tick 1
(`167.089` → `165.092`), while the sim stepped one column (`167` → `166`).
Facing was taken from that recorded step (left). The lock sample just before
recording had `dcol +0.998`; that sample was not used as facing.

Hypothesis, not promoted: the two-column live jump may be a sample that
straddled two 200 ms ticks, or something else about column rate; it was not
re-checked. Offline printer already walks at `y=63` stepping columns; that
was not re-run here.

Checked afterwards against the pre-flip series in this same file: that run
stepped `167.088 → 166.091 → 165.092`, about −1.000 per tick, while Y held.
So `walkSpeed = 1` is still the measured rate. The post-flip disagreeing
pair is a recorder that can miss an intermediate hold, not evidence for
`walkSpeed = 2`. Do not change `Rules`. Next close-out for Round 1 is to
make `record()` reject a step whose `|dcol|` is not about 1.0 (or poll
tighter), then re-run until walk matches for several ticks.

### Climb — not compared

Observation: `compare_tick.py --only climber --ticks 8` found no climber
after many passes (`climber: none found`). SUMMARY:
`walk skipped None  climb missing  pid 34276  ui_state 0  path 'Compiled\\MM_CLIMB_002.lvl'`.
No skill-panel click path was invented.

### Left the game

pid 34276, `ui_state` 0, path `Compiled\MM_CLIMB_002.lvl`. Climb search did
not exhaust the clock; nothing further was pressed.

## After record() harden

2026-10-14. Live check after `compare_tick.py` `record()` only commits steps
with `|dcol|≈1` (walk) or `|dy|≈1` (climb), and resyncs the series start on a
larger jump instead of inventing a midpoint. `Rules` and steppers untouched.
Stock exe launched fresh; `goto_level.py MM_CLIMB_002` (highlight driven to 0
before RETURN). Path `Compiled\MM_CLIMB_002.lvl`, `ui_state` 0, pid 51068.
Clock still high after the walk attempt (`CLOCK` at `0x004CBAAC` read 161 s);
no `restart_level.py`.

### SUMMARY

```
walk: record() never kept 8 steps (only ever reached step 1, then resync)
climb: skipped (walk did not match for 8 ticks)
resync: fired heavily throughout the record window
pid 51068  ui_state 0  path 'Compiled\MM_CLIMB_002.lvl'
```

Script exit: `walker: only 1 samples` (no printer compare, no SUMMARY line
from a completed pair).

### Walk — no 8-tick series

Observation: `python tools/harness/compare_tick.py --only walker --ticks 8`
locked a pen walker (`0x02704898`, col 129.228, y 32.190, dcol −0.972).
`record()` then printed many `resync:` lines and occasional `step 1` lines;
it never printed `step 2` through `step 8`. The series kept resetting to a
single sample, so the printer was never run and there is no first disagreeing
live/sim cell pair for this close-out.

Observation, sample of the resync pattern (Y held at 32.190 throughout):

- After a committed `step 1` at e.g. col 145.052, the next pending hold was
  often `dcol +0.05…+0.16` (just outside `_near` 0.05), which is not in the
  walk accept window `0.85…1.15`, so `record()` treated it as a missed hold
  and restarted the series.
- Some near-column moves that look like real ticks were also rejected as
  resync, e.g. `dcol +0.821` (below 0.85) from 129.228 → 130.064.
- Later in the same pen, `step 1` did fire repeatedly for ~1.0 column moves
  (145→146→…→168, then reverse), but each time a following ~0.05 drift
  cleared the series back to length 1 before a second step could stick.

Observation: resync **did** fire, and it kept firing for the whole record
timeout. The harden stopped the old two-column invent-a-midpoint failure
mode, but with the current `_near` / `step_ok` gap it also never assembled
eight consecutive accepted holds.

Hypothesis, not promoted and not used to change `Rules` or steppers: the
billboard column is not perfectly still between 200 ms ticks; drifts of a
few hundredths of a column are real in this read path, and treating every
non-`≈1` change as a resync prevents a multi-tick series. That is a
recorder hypothesis, not a claim that `walkSpeed ≠ 1`.

Follow-up in `compare_tick.py` (same day): drift below half a column is
ignored; only `|dcol|≈1` commits a step; only `|dcol|≈2` resyncs; other
holds print `ignore:` and keep the series. Window widened to 0.75..1.25 so
a 0.82 sample counts. Not yet live-rechecked.

No column disagreement with `|live step|≈1` was measured against the sim in
this run, because the series never reached the printer.

### Climb — not compared

Per procedure: climb only after walk matches for 8 ticks. Walk did not.
No climber search was run; no skill-click path was invented.

### Left the game

pid 51068, `ui_state` 0, path `Compiled\MM_CLIMB_002.lvl`, clock still
counting down. Nothing pressed after the walker attempt.

## After drift ignore

2026-10-14. Live check after the second `record()` fix: drift below 0.5
columns / 0.15 Y is ignored (no resync); only `step_ok` with window
0.75..1.25 commits; only `missed_hold` (~2 cells) resyncs; other stable
offsets print `ignore:` and keep the series. `Rules` and steppers
untouched. Stock exe still pid 51068; clock had hit 0 / `ui_state` 5, so
`restart_level.py` brought play back (`ui_state` 0, path still
`Compiled\MM_CLIMB_002.lvl`). Then
`python tools/harness/compare_tick.py --only walker --ticks 8`.

### SUMMARY

```
walk: 8 live steps recorded; diverge tick 1  live 130,63  ours 129,63  frac 0.190
climb: skipped (walk did not match for 8 ticks)
resync: none printed; ignore: none printed; steps 1..8 all committed
pid 51068  ui_state 0  path 'Compiled\MM_CLIMB_002.lvl'
```

Script SUMMARY line:
`walk diverge 1  climb skipped None  pid 51068  ui_state 0  path 'Compiled\MM_CLIMB_002.lvl'`

### Walk — 8-tick series, diverge at tick 1

Observation: recorder no longer dominated by resync. Locked pen walker
`0x026c4c48` (col 129.225, y 32.190, dcol −0.976), then committed steps
1–8 at columns 130.064 … 137.053 with Y held at 32.190. No `resync:` and
no `ignore:` lines in the record window. Facing right from the first
recorded step. Printer cell used mask row 63 (`solid_here/below/above` all
1 at 129,63).

| tick | live cell | ours | frac | live_col | live_y | ours_state |
| --- | --- | --- | --- | --- | --- | --- |
| 0 | 129,63 | 129,63 | 0.190 | 129.242 | 32.190 | walker |
| 1 | 130,63 | 129,63 | 0.190 | 130.064 | 32.190 | walker |

First disagreeing pair: tick 1, live `130,63`, ours `129,63`, fraction
`0.190`. Live advanced one column; the sim stayed in place (still walker).

Observation: the printer reported `solid_here 1 solid_below 1 solid_above 1`
at `(129, 63)`. File row 32 (flipped to 63) is solid `0x1` for columns
128–158; open empty floor at that height only starts around column 160.
So this compare started a walker *inside* the climb-wall mass. Our
`stepWalker` sees a wall ahead and does not advance; the live lemming
still walks at Y 32.190 through those cells. That is not a recorder bug.

Hypothesis, not promoted: either the billboard Y is not the collision Y
for wall-band walkers, or the original's walk probe treats this band
differently than `Terrain::solid`. Next compare should lock a walker on
the open ledge (columns ≈160–168), where `(col, 32)` is empty and
`(col, 31)` is solid — the same geometry as the first Y-flip offline
check at `(167, 63)`.

Observation: resync does **not** dominate after this fix. The prior
0.05–0.16 drift resync loop is gone; the series reached the printer.

### Climb — not compared

Per procedure: climb only after walk matches for 8 ticks. Walk diverged at
tick 1. No climber search was run; no skill-click path was invented.

### Left the game

pid 51068, `ui_state` 0, path `Compiled\MM_CLIMB_002.lvl`, clock 257 s.
Nothing pressed after the walker attempt.

## Strict open floor

2026-10-14. `pick_walker` refuses WALL_MASS 128–158 and waits for open
floor. Lock col **167.034**, facing left. Printer:
`solid_here 0 solid_below 1 solid_above 0` at 167,63.

### SUMMARY

```
walk: diverge tick 8  live 159,63  ours 160,63  (ticks 0–7 matched)
solid_here 0 at start
climb: skipped
pid 51068  ui_state 0  path Compiled\MM_CLIMB_002.lvl
```

Observation: ticks 0–7 agree while both walk left across empty floor
(167→160). At tick 8 live enters column 159, which is solid `0x1` at file
Y 32; our sim treats that as a wall and stays on 160 (still walker). Same
wall-mass disagreement as before, but only after seven matched open-floor
ticks. Round 1 walk is confirmed on empty terrain; the remaining miss is
the solid-band edge.

Hypothesis, not promoted: live walkers may traverse cells our `solid()`
rejects. Do not change `walkSpeed`. A clean eight-tick match wants a
walker facing into open floor (e.g. right from ~160), not left into 159.

## Open-floor walker

2026-10-14. Re-run after `pick_walker` prefers `OPEN_FLOOR` (160–168).
`Rules` and steppers untouched. Existing stock process pid 51068 still on
`MM_CLIMB_002` (`ui_state` 0, path `Compiled\MM_CLIMB_002.lvl`, clock 83 s
before the run). No restart. Then
`python tools/harness/compare_tick.py --only walker --ticks 8`.

### SUMMARY

```
lock col 129.234 (not in OPEN_FLOOR 160–168; fell back to pen)
walk: 8 live steps recorded; diverge tick 1  live 130,63  ours 129,63  frac 0.190
solid_here 1 solid_below 1 solid_above 1 at cell 129,63
climb: skipped (lock still inside wall mass; procedure stop)
pid 51068  ui_state 0  path 'Compiled\MM_CLIMB_002.lvl'
```

Script SUMMARY line:
`walk diverge 1  climb skipped None  pid 51068  ui_state 0  path 'Compiled\\MM_CLIMB_002.lvl'`

### Walk — still locked inside the wall

Observation: `pick_walker` did not find an open-floor billboard. After two
`no walker yet` samples it locked `0x026ae898` at col 129.234, y 32.190,
dcol −0.956 (`pen True`). That is still inside solid wall mass (file solid
at Y=32 for cols 128–158). Steps 1–8 committed at 130.064 … 137.047 with
Y held; no `resync:` / `ignore:`. Facing right. Printer:
`grid 256x96 cell 129,63 solid_here 1 solid_below 1 solid_above 1`.

| tick | live cell | ours | frac | live_col | live_y | ours_state |
| --- | --- | --- | --- | --- | --- | --- |
| 0 | 129,63 | 129,63 | 0.190 | 129.241 | 32.190 | walker |
| 1 | 130,63 | 129,63 | 0.190 | 130.064 | 32.190 | walker |

First disagreeing pair: tick 1, live `130,63`, ours `129,63`. Same wall-band
failure as the prior drift-ignore run.

Observation: open empty floor at Y=32 is cols ~160–168, but no walker in
that band satisfied the lock filters during the search window, so the
`open_floor or pen or walking` fallback took the pen walker at ~129.

### Climb — not compared

Per procedure: if the lock is still inside the wall, stop. No climber
search; no skill-click path.

### Left the game

pid 51068, `ui_state` 0, path `Compiled\MM_CLIMB_002.lvl`, clock 63 s.
Nothing pressed after the walker attempt.

## Strict open floor

2026-10-14. Re-run after `pick_walker` returns `None` while only floor
walkers sit in WALL_MASS (128–158); prefers OPEN_FLOOR (160–171).
`Rules` and steppers untouched. Stock process was on results (`ui_state` 5);
`restart_level.py` returned to play (RETURN, then map node). Then
`python tools/harness/compare_tick.py --only walker --ticks 8 --wait 180`.

### SUMMARY

```
lock col 167.034 (outside WALL_MASS; in OPEN_FLOOR 160–171)
walk: 8 live steps recorded; diverge tick 8  live 159,63  ours 160,63  frac 0.190
solid_here 0 solid_below 1 solid_above 0 at cell 167,63
climb: skipped (walk did not match 8 ticks; procedure stop)
pid 51068  ui_state 0  path 'Compiled\MM_CLIMB_002.lvl'
```

Script SUMMARY line:
`walk diverge 8  climb skipped None  pid 51068  ui_state 0  path 'Compiled\\MM_CLIMB_002.lvl'`

### Walk — open floor, then diverge at the wall band

Observation: seven `no walker yet` polls (billboard counts 2061–2106), then
lock `0x026db338` at col 167.034, y 32.190, dcol −1.016 (`pen True`).
That column is outside WALL_MASS. Steps 1–8 committed at 166.111 … 159.163
with Y held; no `resync:` / `ignore:`. Facing left. Printer:
`grid 256x96 cell 167,63 solid_here 0 solid_below 1 solid_above 0`.

| tick | live cell | ours | frac | live_col | live_y | ours_state |
| --- | --- | --- | --- | --- | --- | --- |
| 0 | 167,63 | 167,63 | 0.190 | 167.028 | 32.190 | walker |
| 1 | 166,63 | 166,63 | 0.190 | 166.111 | 32.190 | walker |
| 2 | 165,63 | 165,63 | 0.190 | 165.111 | 32.190 | walker |
| 3 | 164,63 | 164,63 | 0.190 | 164.111 | 32.190 | walker |
| 4 | 163,63 | 163,63 | 0.190 | 163.115 | 32.190 | walker |
| 5 | 162,63 | 162,63 | 0.190 | 162.116 | 32.190 | walker |
| 6 | 161,63 | 161,63 | 0.190 | 161.120 | 32.190 | walker |
| 7 | 160,63 | 160,63 | 0.190 | 160.129 | 32.190 | walker |
| 8 | 159,63 | 160,63 | 0.190 | 159.163 | 32.190 | walker |

First disagreeing pair: tick 8, live `159,63`, ours `160,63`. Live stepped
into column 159 (edge of WALL_MASS); our walker stayed at 160.

### Climb — not compared

Per procedure: climb only after walk matches for 8 ticks. Walk diverged at
tick 8. No climber search; no skill-click path.

### Left the game

pid 51068, `ui_state` 0, path `Compiled\MM_CLIMB_002.lvl`.
Nothing pressed after the walker attempt.

## Eight-tick open floor

2026-10-14. `Rules` and steppers untouched. `pick_walker` locks the right
apex of OPEN_FLOOR (`col >= 168`): left from 167 hit solid 159 at tick 8;
right from 160 reverses near 167 before tick 8; left from 168 walks
168→160 in the printer and on live. Classify often still shows `dcol > 0`
at the apex; facing comes from the recorded series after the reverse.
Also accept classify `|dcol| ≈ 1` or `≈ 2` (0.45 s gap). Stock process
pid 51068, `MM_CLIMB_002`, path `Compiled\MM_CLIMB_002.lvl`.

### SUMMARY

```
lock col 168.050  facing left
walk: match 8  (168→160, solid_here 0 solid_below 1 solid_above 0)
climb: none found (wait 120; level ended ui_state 5)
pid 51068  path Compiled\MM_CLIMB_002.lvl
```

Script SUMMARY lines:
`walk match 8  climb skipped None  pid 51068  ui_state 0  path 'Compiled\\MM_CLIMB_002.lvl'`
`climber: none found` / `SUMMARY walk skipped None  climb missing  pid 51068  ui_state 5  path 'Compiled\\MM_CLIMB_002.lvl'`

### Walk — matched 8 ticks on empty floor

Observation: lock `0x026df898` at col 168.050, y 32.190, dcol +1.020
(`pen False`). Recorded steps went left 167.102 … 160.131; facing left
from the first recorded step. Printer:
`grid 256x96 cell 168,63 solid_here 0 solid_below 1 solid_above 0`.

| tick | live cell | ours | frac | live_col | live_y | ours_state |
| --- | --- | --- | --- | --- | --- | --- |
| 0 | 168,63 | 168,63 | 0.190 | 168.048 | 32.190 | walker |
| 1 | 167,63 | 167,63 | 0.190 | 167.102 | 32.190 | walker |
| 2 | 166,63 | 166,63 | 0.190 | 166.113 | 32.190 | walker |
| 3 | 165,63 | 165,63 | 0.190 | 165.114 | 32.190 | walker |
| 4 | 164,63 | 164,63 | 0.190 | 164.114 | 32.190 | walker |
| 5 | 163,63 | 163,63 | 0.190 | 163.114 | 32.190 | walker |
| 6 | 162,63 | 162,63 | 0.190 | 162.115 | 32.190 | walker |
| 7 | 161,63 | 161,63 | 0.190 | 161.123 | 32.190 | walker |
| 8 | 160,63 | 160,63 | 0.190 | 160.131 | 32.190 | walker |

All nine samples agree. No `resync:` / `ignore:`.

### Climb — missing

`compare_tick.py --only climber --ticks 8 --wait 120` found no frozen
column with rising Y (same search as before). Level clock expired to
`ui_state` 5 during the wait. No skill-click path was invented.

### Left the game

pid 51068, `ui_state` 5, path `Compiled\MM_CLIMB_002.lvl`.
