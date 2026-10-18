# Phase 3, the assign round — a skill onto a chosen lemming, through the panel

2026-10-15. Question: rounds 5 (skills), 6 (builder), 7 (fuse) and 8 (blocker)
all need to assign a skill to a chosen lemming in the running original, and
three rounds in a row failed to and wrote the lemming's flag bytes instead.
Make assignment work, reproducibly, and leave something callable.

**It works, and it is not marginal: 38 clicks sent at a lemming, 38 landed.**
No `Rules` field was touched, no stepper, nothing in `src/`. The only files
changed are harness scripts; the suite was not run because nothing it covers
moved (see "Left the tree").

Module: `tools/harness/assign_skill.py` (new). Stock exe
`C:\progs\Lemmings Revolution\Lemmings Revolution.exe`, SHA-256
`32675bdb…3737a2f` as `read_steppers.py` checks it.

---

## SUMMARY

| Thing | Status | Where |
| --- | --- | --- |
| `0x004CBFB0` is the **level screen object**, not a crosshair object | **READ.** `0x0045DA9E` loads it as `[0x004C6A08 + 0x55A8]` and passes it as `this` to the screen update `0x004532E2`. | static |
| cursor X / Y, the values the engine integrates | **READ + MEASURED.** `screen + 0x16A0` / `+ 0x16A8`, doubles, render pixels. `0x00453FD1` / `0x00453FF8` add the mouse delta; `0x00454016` / `0x004540B0` clamp to `[0, [0x0058CB40]-1]` and `[0, [0x0058CB44]-1]`. Live: a `glide(50,0)` moved `+0x16A0` 318.0 → 418.0. | `assign_skill.cursor` |
| the drawn crosshair is a **copy** of those | **READ.** `0x004540F0` truncates both through `0x0049C5AC` and calls `0x004577CD` on `[screen+0x3C]`. The quirk `.claude/rules/match-the-original.md` records stands; the doubles are the master and are one frame ahead. | static |
| **the lemming under the cursor** | **READ + USED.** `screen + 0x16D8`, a lemming pointer, 0 for none. Written at `0x0045521E`, cleared at `0x00454DC2`. Non-zero means a click right now assigns. | `assign_skill.hovered` |
| **each lemming's own screen rectangle** | **READ + MEASURED.** `lemming + 0x128`, fields `+0x20` left, `+0x28` top, `+0xF0` right, `+0x90` bottom, doubles. This is the stored screen pair earlier rounds searched for and did not find. | `assign_skill.lemming_box` |
| the pick is an AABB overlap, not a point test | **READ.** `0x00454D33` → `0x00454CB9` compares that box against the cursor sprite's box (`[screen+0x3C] + 0x10`), the cursor side shrunk 4 px in x and grown 4 px in y. The cursor box is 32×32 (`0x0045578C`), so the aim tolerance is about ±12 px. | static |
| the assignment itself | **READ.** `0x00455657`: bail on `+0x16D8 == 0`; `0x0045AFEB` makes the selected skill's state; `0x0045A0E0` re-checks it may be applied; `0x0042E696` is `Lemming::setState`; `0x0045B594` spends one of the stock; a paused level is **unpaused**. | static |
| the current state, for proof | **READ + MEASURED.** `0x0042E6FE` writes `[lemming + 0x340] = state`, so `[[lemming+0x340]]` is the state's vtable. `walk` is `0x004B0D28`, `build` is `0x004B08DC` — the two `read_steppers.py --states` cannot name. | `assign_skill.STATE_NAMES` |
| the stock array | **READ + MEASURED.** `0x004C91D8`, ten ints; `0x004C9200` is its own `+0x28`, the selected index (`0x0045A119`). `0x00455611` refuses to hover a button whose count is ≤ 0. | `assign_skill.counts` |
| the panel buttons | **READ + MEASURED.** Ten objects at `0x004C9208`, stride `0x1B0` (`0x004554DA`), same box fields. Live on five levels: x 18..90, y 12+40k..44+40k. | `assign_skill.panel_box` |
| panel order | **MEASURED, 5 of 10.** 0 climber, 1 parasol, 3 blocker, 4 builder, 7 digger. 2 / 5 / 6 are the classic bomber / basher / miner by elimination and are **not** read; left unnamed. | `assign_skill.PANEL_HINT` |
| `measure_dropoff.is_lemming` | **SUPERSEDED, and labelled a fit.** Not widened, not replaced by another box. | below |
| hit rate | **38 clicks at a hovered lemming, 38 spent stock.** 27 of those under the final code: 27 of 27 `ok`, every one on the first attempt. | below |

Re-run, in order. Restart the process between levels — there is still no route
from inside a level back to the map.

```
python tools/harness/assign_skill.py --enter MM_DIG_000     --probe
python tools/harness/assign_skill.py --skill 7 --repeat 3 --attempts 3
# restart the process
python tools/harness/assign_skill.py --enter MM_BLOCK_001_A --skill 3 --repeat 4 --attempts 3
# restart the process
python tools/harness/assign_skill.py --enter MM_FLOAT       --skill 1 --repeat 5 --attempts 3
# restart the process
python tools/harness/assign_skill.py --enter MM_CLIMB_002   --skill 0 --repeat 5 --attempts 3
# restart the process
python tools/harness/assign_skill.py --enter MM_BUILD       --skill 4 --repeat 8 --attempts 3
```

`--probe` sends no input and needs no write handle. Only `--enter` writes, and
the only write is `goto_level`'s level-record swap, which is put back.

---

## What was wrong, and what the fix is

`is_lemming` was a **fit**, and the brief was right to say so. It was

```python
return (8.0 <= w <= 20.0 and 12.0 <= ht <= 26.0
        and 180.0 <= cx <= 580.0 and 220.0 <= cy <= 420.0)
```

and it is **unchanged** — not widened, because widening would only move the
fit, and `measure_dropoff.assign_climber` is now the regression case and must
keep working as it did. Its docstring says it is superseded and prints the
measurement that condemns it. Measured with `assign_skill.py --probe`:

| level | lemmings | box x | box y |
| --- | --- | --- | --- |
| MM_DIG_000 | 20 | 92 … 361 | 77 … 126 |
| MM_BUILD | 14 | 98 … 322 | 182 … 306 |
| MM_FLOAT | 2 | 344 … 429 | 80 … 133 |
| MM_CLIMB_002 (climbers) | 10 | 270 … 534 | 57 … 284 |

Every lemming on MM_DIG_000, MM_BUILD and MM_FLOAT is outside `220 <= cy`.
That is the whole of the float/splat round's failure: `lemming_slots` returned
nothing, so `click_lemming` returned `(None, None)` and the clicks were never
sent. It was never an aiming problem.

The replacement is **not another box**. It is the engine's own rectangle, per
lemming, at `lemming + 0x128`, plus the engine's own answer to "would a click
here assign" at `[screen + 0x16D8]`. Both are addressed by pointer from the
actor list, so there is no camera dependence to get wrong and nothing to
re-derive when the camera moves. The correlation route the brief proposed
(watch a world position, find the sprite record that moves with it, derive the
projection) was not needed: the projection is already stored, and the picking
routine reads it, which the brief named as the better outcome.

### The route, step by step

`0x004CBFB0` is the lever. It had been called "the crosshair object" and it is
not: `0x0045DA9E` reads `[0x004C6A08 + 0x55A8]` into `ecx` and calls
`0x004532E2` with it, and `0x004532E2` is the in-level screen's per-frame
update (it reads `[0x004CC134]`, `lrstate.UI_STATE`, at `0x00455478`). So the
whole of the cursor, the panel hover and the lemming hover are fields of one
object the harness already had the address of.

`0x004C36A0` / `0x004C36A4` are a different cursor entirely. `0x00455EA0` is
its setter, `0x00455EC0` / `0x00455ED0` its getters, and the only caller that
writes it is `0x00453D2E`, inside a block gated at `0x00453BF6` on
`0x00455EE0` → `byte [0x004C369D]`. Four readers, all in the frontend /
level-map range (`0x00453C08`, `0x0048F9C3`, `0x0049B277`, `0x0049B300`).
Round 1's conclusion that writing it is dead in play is confirmed by the code,
not just by the negative result: nothing in the level screen reads it.

---

## Observation 1 — the cursor doubles are the cursor (MM_BUILD, live)

`assign_skill.aim_at` closes the loop on `[screen+0x16A0]` / `+0x16A8`. Three
glides through the shim, read back off those doubles:

```
cursor (318.0, 216.00006103515625)
glide(50,0)      (318.0, 216.000) -> (418.0, 216.000)   px/unit 2.0   -
glide(0,50)      (418.0, 216.000) -> (418.0, 335.9998)  px/unit -     2.3999
glide(-100,-30)  (418.0, 335.999) -> (218.0, 263.9999)  px/unit 2.0   2.3999
```

Two things follow. The scale is **2.0 px per mouse unit in x and 2.4 in y** —
per axis, not one number, which is why `aim_at` re-estimates each axis from
the motion it caused. And it is *not* the double at `0x004C03A8`, which is
**0.2**: `0x00453FD1` multiplies that by `[ebp-0x14]`, which is not the raw
delta. `PX_PER_UNIT = 2.0` in the module is only the first step's size and is
labelled as such.

`measure_dropoff`'s `Pointer` class — ten pool slots, "believe the one that
moved", the 2.0-vs-0.425 ghost — is unnecessary with this. Those ten slots are
the screen's own embedded sprite objects at `[0x004CBFB0] + 0x70 + k*0x238`
(`0x00455774` positions three of them explicitly, at `+0x70`, `+0x2A8`,
`+0x4E0`), each with its corner 0 at `+0x30`; `+0x70 + 0x30 = 0xA0`, which is
the offset that class starts from. Only the active cursor graphic is
positioned, which is exactly why reading a fixed one went stale.

## Observation 2 — `+0x16D8` is the aim feedback (five levels, live)

`assign_skill.nearest_hoverable` glides to a lemming's own box centre, re-reads
the box, re-aims, and then reads `[screen+0x16D8]`. On MM_BUILD:

```
hovered 0x26d9150  aim (200.0069, 244.7352)  cursor (200.0, 244.7999)
```

The cursor landed 0.07 px from the box centre and the engine named the lemming.
Across the five levels below, **every single attempt that found a non-zero
`+0x16D8` then converted**: 38 of 38. There was no case of hovering and missing.

The failures all look the same and all happen *before* a click:
`reason="not hoverable"`, on a level entered a second earlier where the actor
list is empty or holds only lemmings still coming out of the hatch (the pick
rejects those — `byte [lemming+0x29]` at `0x00455104`). They cost no stock and
no clock. `wait_for_lemmings` and a 0.6 s retry handle them.

## Observation 3 — MM_DIG_000, the digger (index 7)

Stock on entry `[0,0,0,0,0,0,0,10,0,1]`, so index 7 is the digger: it is the
only skill the level ships, and it is the index the count sits at.

Nine clicks, nine units of stock spent, 10 → 1. Under the final code:

```
attempt 0: 0x02853c50 state walk -> dig -> walk, stock[7] 4 -> 3
attempt 0: 0x02859870 state walk -> dig -> walk, stock[7] 3 -> 2
attempt 0: 0x02859870 state walk -> dig -> walk, stock[7] 2 -> 1
```

**The dig state lives about one tick on this level.** Polled at kHz straight
after the click:

```
 0.000s state 0x02861e10 vt 0x004b09f0 dig  y 59.0
 0.271s state 0x02861e10 vt 0x0276237c ?    y 59.0     <- object freed
 0.271s state 0x02860010 vt 0x004b0d28 walk y 59.0
```

So the engine entered `dig` (vtable `0x004B09F0`, which
`read_steppers.py --states` names) and gave up again 271 ms later with the
world Y unmoved. **Why it gives up is round 5's question and was not pursued.**
It is recorded here only because it is the trap: the first version of `assign`
read the state once, 0.45 s after the click, and reported two landed
assignments out of three as "no state change". `assign` now samples
`[lemming+0x340]` in a tight loop for 0.8 s (`watch_states`) and prints the
whole path. **Any later round reading a state after a click must sample, not
sleep.**

## Observation 4 — MM_BLOCK_001_A, the blocker (index 3)

Stock `[0,0,0,10,0,0,0,0,0,1]`. Four clicks, four landed, first attempt each:

```
attempt 0: 0x02980170 state walk -> block, stock[3] 10 -> 9, y 81.000
attempt 0: 0x02982870 state walk -> block, stock[3]  9 -> 8, y 81.000
attempt 0: 0x029820e0 state walk -> block, stock[3]  8 -> 7, y 71.000
attempt 0: 0x02983870 state walk -> block, stock[3]  7 -> 6, y 71.000
```

The blocker is the cleanest proof available, because it is permanent. A
`--probe` afterwards:

```
0x02980170 world (117.918, 81.000) state block
0x02982870 world (119.106, 81.000) state block
0x029820e0 world (154.907, 71.000) state block
0x02983870 world (153.184, 71.000) state block
```

Four lemmings in state `block` (vtable `0x004B07B8`), world X frozen, while the
rest of the level walks. That is behaviour, read off the actor list, with
nothing written to any lemming.

## Observation 5 — MM_FLOAT, the parasol (index 1) — the round-3b gap, closed

Stock `[0,10,0,0,0,0,0,0,0,1]`. Five clicks, five landed, first attempt each:

```
attempt 0: 0x026a2c50 walk, stock[1] 10 -> 9, +49 0 -> 1, y 73.000
attempt 0: 0x026a24c0 walk -> fall -> walk, stock[1] 9 -> 8, +49 0 -> 1, y 81.000 -> 73.000
attempt 0: 0x026a3720 walk, stock[1] 8 -> 7, +49 0 -> 1, y 81.000
attempt 0: 0x026a14c0 walk, stock[1] 7 -> 6, +49 0 -> 1, y 65.000
attempt 0: 0x026a1c50 fall -> walk, stock[1] 6 -> 5, +49 0 -> 1, y 12.955 -> 7.000
```

`[lemming+0x49]` — the byte the float/splat round had to **write**, and
labelled a probe for it — is now set by the engine, on a lemming the engine
chose to accept, with the stock decremented. The last row is a lemming that was
**already falling** when it got the umbrella, which is the case that round
actually wanted.

The parasol never becomes `[lemming+0x340]`: `0x0042E696` queues a state whose
`byte [state+0x11]` is set (`0x00430A50`) into `lemming+0x374` instead of
making it current. That is why the climber and the parasol show as attribute
changes and the digger / blocker / builder as state changes, and why `assign`
checks both.

## Observation 6 — MM_CLIMB_002, the climber (index 0) — the regression

Stock `[10,0,0,0,0,0,0,0,0,1]`. Ten clicks over two runs, ten units spent,
10 → 0. `[lemming+0x48]` 0 → 1 every time. The first run reported 4 of 5
because one verification read failed, not one click — see "Harness notes".
The second run, under the final code, 5 of 5. A `--probe` afterwards:

```
0x02734c50 world (172.000, 50.501) +48/1 state climb
0x027344c0 world (172.000, 36.069) +48/1 state climb
0x02735c50 world ( 24.000, 53.137) +48/1 state climb
0x027354c0 world ( 80.000, 40.505) +48/1 state climb
0x02736c50 world ( 64.000, 34.022) +48/1 state climb
0x027364c0 world (172.000, 58.000) +48/1 state flip
0x02737c50 world (172.000, 43.907) +48/1 state climb
```

Six in state `climb` (`0x004B0960`) and one in `flip` (the top-out), world X
pinned to an integer column, Y between integers. This is the behaviour Round 1
could not produce — "no climber appeared in 120 s (needs skill assign / click
path)" — arrived at through the panel, and it closes that line.

## Observation 7 — MM_BUILD, the builder (index 4)

Stock `[0,0,0,0,10,0,0,0,0,1]`. Ten clicks over two runs, ten landed, first
attempt each, 10 → 0:

```
attempt 0: 0x026da0e0 walk -> build, stock[4] 7 -> 6, y 25.000 -> 26.000
attempt 0: 0x026d8670 walk -> build, stock[4] 6 -> 5, y 23.000 -> 24.000
attempt 0: 0x026db0e0 walk -> build, stock[4] 5 -> 4, y 23.000 -> 24.000
attempt 0: 0x026d98e0 walk -> build, stock[4] 4 -> 3, y 25.000 -> 26.000
attempt 0: 0x026d70e0 walk -> build, stock[4] 3 -> 2, y 27.000 -> 28.000
attempt 0: 0x026d7870 walk -> build, stock[4] 1 -> 0, y 23.000 -> 24.000
```

The build vtable is `0x004B08DC`, named here for the first time: its enter hook
`0x0040FAB6` contains the `build` animation site `0x0040FB83`, which is why the
vtable-only scan in `read_steppers.py --states` leaves it blank. Walk is
`0x004B0D28` the same way (enter `0x0041BDBA`, site `0x0041BDC1`).

Every one of those rows shows **world Y rising by exactly 1.000** inside the
0.8 s window. That is a brick. **Round 6 should measure it properly and this
round deliberately did not** — six samples with no tick alignment is not a
measurement of when a brick is laid, and the numbers above are here as proof the
builder was assigned, nothing more.

---

## The hit rate

| level | skill | index | clicks at a hovered lemming | stock spent | reported `ok` | attempts per assignment |
| --- | --- | --- | --- | --- | --- | --- |
| MM_DIG_000 | digger | 7 | 9 | 9 | 3 of 3 under final code | 1 |
| MM_BLOCK_001_A | blocker | 3 | 4 | 4 | 4 of 4 | 1 |
| MM_FLOAT | parasol | 1 | 5 | 5 | 5 of 5 | 1 |
| MM_CLIMB_002 | climber | 0 | 10 | 10 | 5 of 5 under final code (4 of 5 before) | 1 |
| MM_BUILD | builder | 4 | 10 | 10 | 10 of 10 | 1 |
| **total** | | | **38** | **38** | **27 of 27 under final code** | **1** |

Read that as: **the engine consumed a skill on every click this module sent**,
38 for 38, across five levels and five skills. "Reported ok" is lower only
where a run predates a fix inside the *reporting*, never the clicking — and the
11 clicks not under the final code all spent stock, which is the engine's own
receipt.

The cost of an assignment is about **3 seconds**: one panel click (skipped when
the index is already selected — the engine pre-selects the level's only skill),
one glide, one re-aim, one click, the 0.8 s state window and the 0.6 s
double-click wait. A `--repeat 8` run finishes inside 30 s of level clock.

What a next round should budget for is **not** the click; it is getting into the
level. `find_menu_index` failed once in six entries this session (two
candidates, highlight 5, refused to press RETURN — exactly as documented) and
the retry worked. Budget one retry per level.

## Clean negatives, and what was searched for

  * **The hit-test statics were not used, and now there is a code reason.**
    `0x004C36A0` has two references in `.text`, both inside the accessor pair
    `0x00455EA0` / `0x00455EC0`; the four callers of the getters are all
    frontend / level-map code and the one writer is gated on
    `byte [0x004C369D]`. Nothing in the level screen reads it. Round 1's
    session was not wasted on a flaky result, it was spent on dead memory.
  * **The cursor doubles were never written.** `aim_at` glides. The write is
    available (`[screen+0x16A0]` is plain memory and the integrator adds to
    it), and was left unused on purpose: the point of this round was a path
    through the shim. A later round that needs a teleport has it, and should
    say so.
  * **The correlation route was not needed.** Taking a lemming off the actor
    list, watching its world position, and finding the sprite record that
    moves in lockstep would have worked. It was not run, because
    `lemming + 0x128` is the projection already computed, the picking routine
    `0x00454CB9` reads it, and a derived mapping would be a second copy of
    something the engine stores. The "searched-but-not-exhausted" lead in the
    brief — a screen position the engine stores per lemming — is the thing
    that was found.
  * **The panel pitch is no longer assumed.** `measure_float_splat` had caught
    "index k at y ≈ 27 + 40k" failing on E10_KF_1: it read
    `0x004C9200 = 7` for a click at y = 67, which is inside button 1. The
    explanation is that **the engine pre-selects the level's only stocked
    skill** — on all five levels here `0x004C9200` already held the right
    index before any click — so that reading was the pre-selection, not the
    click. The boxes themselves are read now (`panel_box`), and on these five
    levels they are x 18..90, y 12+40k..44+40k, i.e. a 40 px pitch after all.
  * **Panel indices 2, 5, 6, 8 and 9 are not identified.** `0x0045A0E0`
    switches on `[0x004C91D8+0x28]` through a ten-arm jump table at
    `0x0045A4DD` and those arms were not followed. Every level here shipped
    exactly 1 of index 9, and `0x0045472B` gives index 9 its own click path;
    index 8's click sets `[0x004CBFB0]+0x20 = 1` (`0x00455B12`). Neither was
    pressed. Guessing bomber / basher / miner from the classic order would
    have been free and is not in the code.

## Harness notes

  * **`read_lem` reads 0x80 bytes, not 0x400.** A single long read failed for
    one lemming near the end of its heap region and `ReadProcessMemory` fails
    for the *whole* request when a block spans a boundary
    (`measure_dropoff.readable_block` says the same). That is the entire
    content of the one 4-of-5 run above: the click landed, the verification
    read returned `None`, and the module said "no attribute change seen". It
    now reads the core at 0x80, the box at `+0x128`, and the state at `+0x340`
    separately.
  * **A level entered a moment ago has an empty actor list.** Two runs spent
    all their attempts reporting "no lemming the engine will accept" with the
    cursor still at (320, 240) — it had nothing to glide to. `assign` now
    waits (`wait_for_lemmings`, default 20 s) and reports that case
    distinctly.
  * `goto_level.focus` was **never called**. `goto_level.py` is unmodified.
    Entry is `measure_float_splat.enter`, i.e. `select_level` +
    `restart_level.ensure_playing`, bracketed by the slot [0][0] restore. All
    five entries reported `slot [0][0] restored: True`.
  * `lrstate.attach` returns `(handle, pid)`. Still true, still the first
    thing to check when a level table read looks broken.
  * The process was killed and relaunched once per level, five times. No ESC
    was pressed from a results screen.
  * The shim polled throughout with the game in the background; `mouse_frame`
    was ticking at over 11,000 reads between two of our calls, so nothing here
    needed the window focused.

## Rules

**No `Rules` field changed. No stepper changed. Nothing in `src/` changed.**
This round measured nothing about the simulation on purpose: finding that
assignment works and then measuring a skill with it are two rounds, and the
second one is not this one.

## Left the tree

| file | change |
| --- | --- |
| `tools/harness/assign_skill.py` | **new.** The module. Every address above is in its docstring. |
| `tools/harness/measure_dropoff.py` | `is_lemming`'s docstring only. Marked SUPERSEDED, labelled a fit, carries the three-level measurement that condemns it. **The box itself is unchanged**, so `assign_climber` still behaves exactly as it did. |
| `tools/harness/measure_float_splat.py` | `--find-parasol`'s usage paragraph only. Points at `assign_skill` and says the parasol now assigns for real. |

No C++ was touched, so the suite was not re-run; it was 16 of 16 at the start
of the session and nothing it compiles or asserts over moved.

## Left the game

**Killed.** No process is running at the end of the session. The last level
played was `Compiled\MM_BUILD.lvl` (pid 31764), entered through the record
swap, slot [0][0] restored to `MM_CLIMB_002` — the entry printed
`slot [0][0] restored: True`, and the five entries this session each printed
the same. `pid 47992` named in the brief was already dead when the session
started; it was checked, not trusted.

The shim is installed and inert (`dinput.dll` in the game folder,
`enable = 0`). A fresh launch comes up at `ui_state 3`, main menu.

## For round 5, 6, 7 and 8

One call:

```python
from assign_skill import assign, counts, lemmings, state_of
r = assign(h, inp, skill=4)          # or choose=<a lemming pointer>
```

Read its docstring for what it needs and what it cannot do. Three things that
will otherwise cost a session:

  1. **Sample the state, do not sleep then read.** The dig state lasted 271 ms
     on MM_DIG_000. `watch_states` is there for this.
  2. **The engine pre-selects the level's only stocked skill.** `0x004C9200`
     agreeing with your intent proves nothing. The stock count going down does.
  3. **Enter, then wait for the hatch.** The actor list is empty for the first
     second or two and the pick rejects lemmings still in the hatch.
