"""Read `Rules::floatSpeed`, `floatAfter` and `splatDistance` off a long fall.

Phase 3 round 3b. The three fields had a static read and no live series. This
script gets the series, on a level whose geometry is long enough to reach them.

Why the actor list and not a billboard
--------------------------------------
Rounds 1 and 2 tracked lemmings through their drawn billboard quads, because
that was all that had been found. Round 3 found the actor vector
(`measure_spawn.py`): `0x004CBB18` is a count at +0 and a pointer array at
+0x0C, and every lemming in it carries its world position as plain doubles:

    lemming+0x08  double  X (unrolled, 256 columns round the tube)
    lemming+0x10  double  Y, world units, up
    lemming+0x44  int     direction, the `fild` at 0x00416213
    lemming+0x49  byte    has-floater -- the skill was assigned
    lemming+0x4b  byte    float animation running -- the halving flag

That is the engine's own Y, not a projection of it, and it is addressed by
pointer rather than searched for, so there is no tracker to lose a lock and
no reacquire lag. Every sample in a series here is one `ReadProcessMemory` of
one object. `measure_dropoff.py`'s billboard follower is the fallback if the
actor list is ever unavailable; prefer this.

Which level, and why that one
-----------------------------
`splatDistance` is 30 world units, so the series needs a drop longer than
that, and nothing on record had one: `MM_FLOAT`'s entrance is 5 units above
its floor and `MM_CLIMB_002`'s climb ledge is 16.

The stored collision grid answers it offline. `read_lvl_grid.find_grid` gives
the 256 x 96 cells and `measure_spawn.hatch_words` gives each hatch's
`(count, x, y)`, so for every hatch in the corpus the drop is
`y - (highest solid row below y in column x)`. `--grid-search` prints that.
It picks out **E10_KF_1** ("LEMTRIS", 180 s, 50 lemmings, save 50) whose four
hatches sit at y 81-82 over floors at rows 6-11:

    hatch (count, x, y)    first solid row below    drop
    (20, 94, 82)           6                        76
    (13, 112, 82)          7                        75
    (10, 130, 81)          8                        73
    (7,  74, 82)           11                       71

and whose skill stock is floater 50, builder 2 -- 50 umbrellas for 50
lemmings that have to be saved. So the level's own design says a 71-unit drop
is fatal without one, which is the prediction being tested.

The second geometry is `MM_H_002`: one hatch of count 1 at (113, 88) over row
17, a 71-unit drop, on a level with 2 floaters -- a different level, a
different column, the same question.

What the binary says, so the series has something to disagree with
-----------------------------------------------------------------
The fall stepper is `0x004160BD`, a method on the fall *state* object with
the lemming at `[ebp+8]`. Disassembled with `read_steppers.py --at`:

  * `0x00416038` is the speed accessor. It branches on `byte [lemming+0x4b]`
    and divides the 20.0 at `0x004B0A18` by the 2.0 at `0x004B03F8` when it
    is set, then multiplies by the elapsed seconds. 20.0 * 0.2 = 4 world
    units a tick, 2 under the umbrella.
  * `0x004162D6` loads `[state+0x18]`, the distance fallen, and compares it
    with the 10.0 at `0x004B03E8`. Past 10, with `[lemming+0x49]` set and the
    one-shot `[state+0x20]` clear, it starts the **`umbopen`** animation
    (string at `0x004BF384`) and sets that latch.
  * `0x00416315` adds the 1.0 at `0x004B02D0` to `[state+0x18]`, once per
    sub-step, so the distance is counted in whole world units of descent.
  * `0x0041612F`-`0x00416186` then clamps the step to the time left in the
    `umbopen` animation while it plays, and `0x004162C2` sets
    `[lemming+0x4b] = 1` and starts the **`float`** animation (string at
    `0x004BF37C`) when it finishes. So the speed halves when `umbopen`
    *ends*, not at the moment the distance passes 10.
  * `0x004165FC` loads `[state+0x18]` on landing and compares it with the
    30.0 at `0x004B10A8`. `test ah, 0x41` at `0x00416607` jumps to the
    survive path on C0 or C3 -- fallen **<=** 30.0 -- so a splat needs
    strictly more than 30. Then `0x00416615` reads `byte [lemming+0x49]` and
    `0x0041661A` jumps to that same survive path when it is set: **a lemming
    with the floater skill never splats, however far it fell.** Two cell-flag
    arms follow (0x200 with `[lemming+0x4c] == 1`, 0x4000 with `== 2`) and
    only then is the death state allocated at `0x0041664C`.

Modes
-----
    python measure_float_splat.py --grid-search
        No game. Every hatch in the corpus by drop, and the walkable
        adjacent drops, straight out of the .LVL collision grids.

    python measure_float_splat.py --enter e10_kf_1
        Enter that level through goto_level's record swap, restoring slot
        [0][0] after. Never calls goto_level.focus -- it kills the process
        (measure_dropoff.py docstring). The live level table spells this one
        lower-case; `goto_level.py --list` prints the live spellings.

    python measure_float_splat.py --enter e10_kf_1 --watch 26 --hz 120
        Follow every lemming in the actor list and print each one's distinct
        (Y, X) with a timestamp and the two flags, then the whole series per
        lemming. This is the splat series; no input is sent.

    python measure_float_splat.py --enter e10_kf_1 --watch 26 --hz 120            --poke-above 78 --poke-float 82.5 --poke-every 2
        The same, but write 1 into `[lemming+0x49]` the first time a tracked
        lemming's Y is inside (78, 82.5) -- which on this level is the first
        sample of its fall -- for every other lemming. That byte is the one
        the splat test and the umbrella test both read, so it is the
        assignment those branches see, and the un-poked lemmings are a
        control falling the same distance in the same seconds.

    python measure_float_splat.py --enter e10_kf_1 --states 28 --hz 150            --poke-float 82.5 --poke-above 78
        Follow the fall *state* instead: `[state+0x18]` (the distance
        fallen), `[state+0x20]` (the umbrella one-shot) and `[state+0x21]`
        beside the lemming's Y and flags. This is the direct read of
        `floatAfter`: the latch is set on the sub-step where the accumulator
        reaches 10.0. It also reports every appearance of the death state's
        vtable 0x004B0668 in the window, which is a splat seen rather than
        inferred.

    python measure_float_splat.py --enter e10_kf_1 --states 24 --hz 120 --clamp-acc 29
    python measure_float_splat.py --enter e10_kf_1 --states 24 --hz 120 --clamp-acc 31
        The probe on `splatDistance`: hold every live faller's accumulator at
        that value, so the landing branch sees it whatever the geometry did.
        29 and 31, not 30, because the next sub-step adds 1.0 between the
        last write and the landing test. 29 -> they survive a 75-unit drop,
        31 -> they die at the untouched rate.

    python measure_float_splat.py --find-state
    python measure_float_splat.py --dump-lemming
        Where the accumulator is, and why it is not on the lemming.

    python measure_float_splat.py --find-parasol
        Walk the drawn panel buttons, record what `0x004C9200` becomes for
        each, and click lemmings after each until one gets `[+0x49]` set --
        on a level whose only stock is the floater, that button is the
        parasol. Did not land an assignment this session; see the note.

        SUPERSEDED 2026-10-15. `tools/harness/assign_skill.py` assigns the
        parasol on MM_FLOAT through the panel, 5 of 5 first-attempt, with
        `[+0x49]` set by the engine and the stock count decremented. The
        reason this one failed was `measure_dropoff.is_lemming`, whose screen
        box excluded every lemming on that camera; `assign_skill` reads the
        engine's own per-lemming rectangle at `lemming + 0x128` and its own
        hovered-lemming pointer at `[0x004CBFB0] + 0x16D8` instead. Prefer it
        for any new assignment, and prefer it over `--poke-float`, which is a
        memory write and is labelled one.

"""
from __future__ import annotations

import argparse
import glob
import os
import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from lrstate import attach, i32, read_mem, u32, ui_state  # noqa: E402
from measure_spawn import (  # noqa: E402
    ACTOR_LIST,
    HATCH_VT,
    LEMMING_VT,
    OTHER_VT,
    actor_ptrs,
    hatch_words,
    loaded_level,
)
from memdump import k32  # noqa: E402
from read_lvl_grid import GRID_WIDTH, find_grid, solid  # noqa: E402

RELEASED = 0x004CBA78
CLOCK = 0x004CBAAC
TIME_LIMIT = 0x004CBAA8
SAVE_TARGET = 0x004CBAB0
# A band of in-level statics round the released counter. Which of them is the
# death count is not assumed: --watch prints the ones that moved.
STAT_LO, STAT_HI = 0x004CBA60, 0x004CBAD0

CORPUS = "build/lvl_dump/levels/COMPILED"

# Offsets inside the lemming object, all from the fall stepper 0x004160BD.
OFF_X, OFF_Y, OFF_DIR = 0x08, 0x10, 0x44
OFF_HASFLOAT, OFF_FLOATANIM, OFF_KIND = 0x49, 0x4B, 0x4C
LEM_BYTES = 0x80

# The skill panel, as measure_dropoff measured it: ten buttons down x = 38,
# one every ~39.9 px from y = 27. Index 0 is the climber, 1 the parasol.
PANEL_X = 38.0
PANEL_Y0 = 27.0
PANEL_DY = 39.9
PARASOL_INDEX = 1
SELECTED = 0x004C9200
CROSS_PTR = 0x004CBFB0


# --- the offline grid search ------------------------------------------------


def _grid(path: str):
    found = find_grid(open(path, "rb").read())
    if not found:
        return None
    _off, cells = found
    return cells, len(cells) // GRID_WIDTH


def _first_solid_below(cells, height, x: int, y: int):
    """The highest solid row at or below `y` in column `x`, the engine's test.

    `solid` is `read_lvl_grid.solid`, which is the faller's own landing probe
    at `0x0041A99A`: non-zero and bit 0x80 clear.
    """
    for row in range(min(y, height - 1), -1, -1):
        if solid(cells[row * GRID_WIDTH + x]):
            return row
    return None


def grid_search(directory: str, max_step_up: int = 2) -> int:
    """Every hatch drop, and every walkable adjacent drop, in the corpus.

    The hatch drop is the one that matters here: it needs no skill, no
    geometry hunt and no walking, because the lemming is released into it.
    The walk-off column is printed beside it so a level can be judged on
    both.
    """
    files = sorted(glob.glob(os.path.join(directory, "*.LVL")))
    if not files:
        print(f"no .LVL files under {directory}", file=sys.stderr)
        return 1
    hatch_rows = []
    walk_rows = []
    for path in files:
        data = open(path, "rb").read()
        g = _grid(path)
        if not g:
            continue
        cells, height = g
        name = os.path.basename(path)[:-4]

        def land(x, y):
            return _first_solid_below(cells, height, x, y)

        starts = []
        for hw in hatch_words(data):
            x, y = hw["x"] % GRID_WIDTH, hw["y"]
            row = land(x, min(y, height - 1))
            if row is None:
                continue
            hatch_rows.append((y - row, name, hw["count"], x, y, row))
            starts.append((x, row))

        # Walk reachability: stand on a solid row, step up to `max_step_up`,
        # otherwise fall to the first solid row below. Crude on purpose -- it
        # is a candidate finder, not the stepper.
        seen = set(starts)
        queue = list(starts)
        best = (0,)
        while queue:
            x, row = queue.pop()
            for d in (-1, 1):
                x2 = (x + d) % GRID_WIDTH
                up = None
                for k in range(max_step_up, 0, -1):
                    if row + k < height and solid(cells[(row + k) * GRID_WIDTH + x2]):
                        up = row + k
                        break
                nxt = up if up is not None else land(x2, row)
                if nxt is None:
                    continue
                if row - nxt > best[0]:
                    best = (row - nxt, x, row, x2, nxt)
                if (x2, nxt) not in seen:
                    seen.add((x2, nxt))
                    queue.append((x2, nxt))
        walk_rows.append((best[0], name, best[1:], len(seen)))

    hatch_rows.sort(reverse=True)
    print("# hatch drops: a released lemming's own fall, no skill needed")
    print("#  drop  level                 count  spawn      lands on row")
    for drop, name, count, x, y, row in hatch_rows[:20]:
        print(f"  {drop:5d}  {name:20s}  {count:5d}  ({x:3d},{y:3d})  {row}")
    print(f"\n# {sum(1 for r in hatch_rows if r[0] > 30)} of {len(hatch_rows)}"
          f" hatch records drop further than splatDistance 30")
    walk_rows.sort(reverse=True)
    print("\n# longest walkable adjacent drop reachable from a hatch")
    print("#  drop  level                 (from_col, row, to_col, lands)  cells walked")
    for drop, name, where, reach in walk_rows[:15]:
        print(f"  {drop:5d}  {name:20s}  {where}  {reach}")
    return 0


# --- live ------------------------------------------------------------------


def write_mem(h, addr: int, data: bytes) -> bool:
    import ctypes

    n = ctypes.c_size_t(0)
    ok = k32.WriteProcessMemory(h, ctypes.c_void_p(addr), data, len(data),
                                ctypes.byref(n))
    return bool(ok) and n.value == len(data)


class Lem:
    """One lemming object, read in a single block."""

    __slots__ = ("ptr", "vt", "x", "y", "direction", "has_float", "float_anim",
                 "kind")

    def __init__(self, ptr: int, buf: bytes):
        self.ptr = ptr
        self.vt = struct.unpack_from("<I", buf, 0)[0]
        self.x = struct.unpack_from("<d", buf, OFF_X)[0]
        self.y = struct.unpack_from("<d", buf, OFF_Y)[0]
        self.direction = struct.unpack_from("<i", buf, OFF_DIR)[0]
        self.has_float = buf[OFF_HASFLOAT]
        self.float_anim = buf[OFF_FLOATANIM]
        self.kind = struct.unpack_from("<I", buf, OFF_KIND)[0]


def read_lem(h, ptr: int):
    buf = read_mem(h, ptr, LEM_BYTES)
    if not buf or len(buf) != LEM_BYTES:
        return None
    lem = Lem(ptr, buf)
    if lem.vt not in (LEMMING_VT, OTHER_VT):
        return None
    return lem


def lemmings(h) -> list[int]:
    out = []
    for p in actor_ptrs(h):
        vt = u32(h, p)
        if vt in (LEMMING_VT, OTHER_VT):
            out.append(p)
    return out


def statics(h) -> dict[int, int]:
    raw = read_mem(h, STAT_LO, STAT_HI - STAT_LO)
    if not raw:
        return {}
    return {STAT_LO + i: struct.unpack_from("<i", raw, i)[0]
            for i in range(0, len(raw) - 3, 4)}


def hatch_y_for(level: str):
    """The hatch Y words for the loaded level, read from its own .LVL.

    A series has to say where the fall *started*, and the start is the hatch
    word, not the first sample: the first sample is already a tick or two in.
    """
    stem = level.split("\\")[-1].split("/")[-1]
    if stem.lower().endswith(".lvl"):
        stem = stem[:-4]
    path = os.path.join(CORPUS, stem + ".LVL")
    if not os.path.exists(path):
        return None, None
    data = open(path, "rb").read()
    g = _grid(path)
    rows = []
    for hw in hatch_words(data):
        x, y = hw["x"] % GRID_WIDTH, hw["y"]
        row = _first_solid_below(g[0], g[1], x, y) if g else None
        rows.append((hw["count"], x, y, row))
    return rows, g


def watch(h, seconds: float, poke_below: float | None, hz: float,
          poke_above: float = -1e9, poke_every: int = 1) -> int:
    """Follow every lemming in the actor list and print its Y series.

    Each tracked pointer gets its own series, so a record being recycled
    cannot be mistaken for motion -- the pointer either is in the list or is
    not, and a vtable check rejects a slot that has been reused. Round 2's
    tracker gaps do not exist here.
    """
    level = loaded_level(h)
    print(f"ui {ui_state(h)}  loaded {level!r}  clock {i32(h, CLOCK)}"
          f"  released {i32(h, RELEASED)}", flush=True)
    rows, _g = hatch_y_for(level)
    if rows:
        print("hatch records from the .LVL (count, column, Y word, lands on row):")
        for count, x, y, row in rows:
            drop = None if row is None else y - row
            print(f"  ({count}, {x}, {y})  row {row}  drop {drop}")
    stat0 = statics(h)

    series: dict[tuple, list] = {}
    flags: dict[int, tuple] = {}
    gone: dict[int, float] = {}
    gen: dict[int, int] = {}
    departed: dict[tuple, float] = {}
    poked: set[int] = set()
    seen_for_poke: list[int] = []
    t0 = time.perf_counter()
    period = 1.0 / hz
    next_at = 0.0
    samples = 0
    while True:
        now = time.perf_counter() - t0
        if now > seconds:
            break
        if now < next_at:
            continue
        next_at = now + period
        live = set()
        for ptr in lemmings(h):
            lem = read_lem(h, ptr)
            if lem is None:
                continue
            live.add(ptr)
            samples += 1
            # A pointer freed when a lemming splats is handed to the next one
            # born, so a reappearance starts a fresh series under a
            # generation-suffixed key. Appending across the reuse would read
            # as a jump of seventy world units straight up.
            if ptr in gone:
                gen[ptr] = gen.get(ptr, 0) + 1
                gone.pop(ptr)
            s = series.setdefault((ptr, gen.get(ptr, 0)), [])
            # Dedup on (Y, X), not Y alone. A lemming walking a flat ledge
            # holds one Y for seconds, and a Y-only key then leaves an
            # eleven-second hole in the middle of a series that reads like a
            # tracker gap -- MM_H_002 produced exactly that.
            if not s or (s[-1][1], s[-1][2]) != (lem.y, lem.x):
                s.append((now, lem.y, lem.x, lem.direction,
                          lem.has_float, lem.float_anim))
            f = (lem.has_float, lem.float_anim, lem.kind)
            if flags.get(ptr) != f:
                print(f"  t={now:7.3f}  {ptr:#010x} flags +49={f[0]}"
                      f" +4b={f[1]} +4c={f[2]}  y={lem.y:.3f}", flush=True)
                flags[ptr] = f
            if (poke_below is not None and ptr not in poked
                    and poke_above < lem.y < poke_below and not lem.has_float):
                # Every Nth lemming only, when asked: the ones left alone are
                # the control, falling the same distance down the same column
                # in the same seconds, so the two fates are a differential and
                # not two separate runs.
                seen_for_poke.append(ptr)
                poked.add(ptr)
                if poke_every <= 1 or (len(seen_for_poke) - 1) % poke_every == 0:
                    ok = write_mem(h, ptr + OFF_HASFLOAT, b"\x01")
                    print(f"  t={now:7.3f}  POKE +49=1 on {ptr:#010x}"
                          f" at y={lem.y:.3f} -> {ok}", flush=True)
                else:
                    print(f"  t={now:7.3f}  control (no poke) {ptr:#010x}"
                          f" at y={lem.y:.3f}", flush=True)
        for ptr, generation in list(series):
            if (ptr not in live and ptr not in gone
                    and generation == gen.get(ptr, 0)):
                gone[ptr] = now
                departed[(ptr, generation)] = now
                last_y = series[(ptr, generation)][-1][1]
                print(f"  t={now:7.3f}  {ptr:#010x} left the actor list"
                      f" (last y {last_y:.3f})", flush=True)
    elapsed = max(1e-6, time.perf_counter() - t0)
    print(f"\n{samples} object reads in {elapsed:.1f} s"
          f" = {samples / elapsed:.0f} /s", flush=True)

    moved = {a: (v, stat0.get(a)) for a, v in statics(h).items()
             if stat0.get(a) != v}
    print("statics that moved in the window:")
    for a, (now_v, then) in sorted(moved.items()):
        print(f"  {a:#010x}  {then} -> {now_v}")

    report(series, departed, rows)
    return 0


def report(series: dict, gone: dict, hatch_rows) -> None:
    """Print each lemming's height series and the step between samples.

    Only consecutive samples of one lemming are differenced, and the gap is
    printed beside the step so a slow sample cannot be read as a slow fall.
    The key is (pointer, generation): a pointer freed on a splat is handed to
    the next lemming born, and appending across that would read as a jump of
    seventy units straight up.
    """
    starts = {y for _c, _x, y, _r in (hatch_rows or [])}
    for key, s in sorted(series.items()):
        if len(s) < 2:
            continue
        ptr, generation = key
        where = ("left the list at %.3f s" % gone[key]
                 if key in gone else "still listed")
        print("")
        print(f"--- {ptr:#010x}#{generation}  {len(s)} distinct Y ({where})")
        print("     t (s)        y      dy     dt(ms)  col     dir +49 +4b")
        prev = None
        for t, y, x, d, hf, fa in s:
            if prev is None:
                print(f"  {t:8.3f}  {y:8.3f}       -        -  {x:7.2f}  "
                      f"{d:3d} {hf:3d} {fa:3d}")
            else:
                print(f"  {t:8.3f}  {y:8.3f}  {y - prev[1]:6.3f}  "
                      f"{(t - prev[0]) * 1000:7.1f}  {x:7.2f}  "
                      f"{d:3d} {hf:3d} {fa:3d}")
            prev = (t, y)
        y_first, y_last = s[0][1], s[-1][1]
        print(f"  first {y_first:.3f}  last {y_last:.3f}"
              f"  span {y_first - y_last:.3f}")
        # Only quote the hatch Y as the start of the fall when the series
        # actually begins there. A lemming that landed, walked and then went
        # off an edge fell from the ledge, not from the hatch, and the
        # difference is the whole question for splatDistance.
        start = next((v for v in sorted(starts, reverse=True)
                      if 0.0 <= v - y_first <= 1.0), None)
        if start is not None:
            print(f"  series starts at the hatch Y word {start};"
                  f" distance fallen by the last sample"
                  f" {start - y_last:.3f}")
        else:
            print("  series does not start at a hatch Y; the fall this ends"
                  " in began at a ledge, so read its start off the series")


def dump_lemming(h, seconds: float = 1.5) -> int:
    """Dump one lemming's object and say which dwords change while it falls.

    This is how the fall *state* pointer is found: the stepper is a method on
    a state object that Round 2 could not locate by scanning the heap for its
    vtable, but the lemming must hold a pointer to it, and a pointer whose
    target's first dword is a 0x004Bxxxx vtable is that pointer.
    """
    ptrs = lemmings(h)
    print(f"{len(ptrs)} lemmings listed; loaded {loaded_level(h)!r}")
    if not ptrs:
        return 2
    ptr = ptrs[-1]
    first = read_mem(h, ptr, 0x100)
    t0 = time.perf_counter()
    changed: dict[int, set] = {}
    last = first
    while time.perf_counter() - t0 < seconds:
        buf = read_mem(h, ptr, 0x100)
        if not buf or len(buf) != 0x100:
            continue
        for off in range(0, 0x100, 4):
            a = struct.unpack_from("<I", last, off)[0]
            b = struct.unpack_from("<I", buf, off)[0]
            if a != b:
                changed.setdefault(off, set()).add(b)
        last = buf
    print(f"lemming {ptr:#010x}")
    for off in range(0, 0x100, 8):
        d = struct.unpack_from("<d", last, off)[0]
        w0, w1 = struct.unpack_from("<II", last, off)
        mark = "*" if off in changed or off + 4 in changed else " "
        print(f"  +{off:02x} {mark} {w0:#010x} {w1:#010x}"
              f"   double {d!r}")
    print("offsets that changed:", {f"+{o:02x}": len(v) for o, v in sorted(changed.items())})
    # Any dword that points at an object whose first dword is in .rdata is a
    # candidate state pointer.
    for off in range(0, 0x100, 4):
        p = struct.unpack_from("<I", last, off)[0]
        if p < 0x10000:
            continue
        vt = u32(h, p)
        if vt is not None and 0x004B0000 <= vt < 0x004C0000:
            acc = read_mem(h, p, 0x30)
            dbl = struct.unpack_from("<d", acc, 0x18)[0] if acc else None
            print(f"  +{off:02x} -> {p:#010x} vtable {vt:#010x}"
                  f"  its +0x18 = {dbl!r}")
    return 0


def find_state(h, seconds: float = 20.0) -> int:
    """Find the fall *state* object for a live lemming, by back-reference.

    The accumulator the binary tests -- `[state+0x18]`, the distance fallen --
    lives on the state object, not on the lemming: the lemming's own 0x80
    bytes hold no pointer to it (`--dump-lemming` shows everything past +0x80
    is zero and +0x78 is an embedded animator, vtable 0x004B14A8). Round 2
    scanned the heap for the faller vtable 0x004B0A20 and found nothing
    usable.

    The state holds the lemming instead, so the search runs the other way:
    scan the heap for a dword equal to a known lemming pointer, then walk
    back from each hit to the nearest dword that looks like an .rdata vtable.
    That is the object, and its +0x18 is the accumulator -- checked here by
    watching it climb while the lemming descends.
    """
    import struct as _struct

    from find_fallers import heap_regions

    t0 = time.perf_counter()
    target = None
    while time.perf_counter() - t0 < seconds:
        for p in lemmings(h):
            lem = read_lem(h, p)
            if lem and lem.y > 40.0:
                target = p
                break
        if target:
            break
    if target is None:
        print("no falling lemming within the wait")
        return 2
    print(f"following lemming {target:#010x}")
    needle = _struct.pack("<I", target)
    hits = []
    for base, size in heap_regions(h):
        d = read_mem(h, base, size)
        if not d:
            continue
        i = d.find(needle)
        while i >= 0:
            if i % 4 == 0:
                hits.append(base + i)
            i = d.find(needle, i + 4)
    print(f"{len(hits)} dwords in the heap equal that pointer")
    cands = []
    for a in hits:
        for back in range(0, 0x44, 4):
            vt = u32(h, a - back)
            if vt is not None and 0x004B0000 <= vt < 0x004C0000:
                cands.append((a - back, vt, back))
                break
    for obj, vt, back in cands:
        blk = read_mem(h, obj, 0x40)
        if not blk:
            continue
        print(f"  object {obj:#010x} vtable {vt:#010x}, lemming at +{back:02x}")
        print(f"    +18 double {_struct.unpack_from('<d', blk, 0x18)[0]!r}"
              f"  +20 {blk[0x20]}  +21 {blk[0x21]}")
    # Which candidate is the faller: the one whose +0x18 climbs.
    for obj, vt, _back in cands:
        a0 = _struct.unpack_from("<d", read_mem(h, obj, 0x20) or bytes(0x20), 0x18)[0]
        time.sleep(0.15)
        blk = read_mem(h, obj, 0x28)
        if not blk:
            continue
        a1 = _struct.unpack_from("<d", blk, 0x18)[0]
        lem = read_lem(h, target)
        print(f"  {obj:#010x} vt {vt:#010x}  +18 {a0} -> {a1}"
              f"  (lemming y {lem.y if lem else None})")
    return 0


def watch_state(h, seconds: float, state_obj: int, lemming: int,
                hz: float = 120.0) -> int:
    """Print `[state+0x18]`, `[state+0x20]` and the lemming's Y together.

    This is the direct read of `floatAfter`: the accumulator value on the
    sub-step where the `umbopen` latch `[state+0x20]` goes from 0 to 1.
    """
    import struct as _struct

    rows = []
    t0 = time.perf_counter()
    period = 1.0 / hz
    nxt = 0.0
    while True:
        now = time.perf_counter() - t0
        if now > seconds:
            break
        if now < nxt:
            continue
        nxt = now + period
        st = read_mem(h, state_obj, 0x28)
        lem = read_lem(h, lemming)
        if not st or lem is None:
            continue
        acc = _struct.unpack_from("<d", st, 0x18)[0]
        row = (now, lem.y, acc, st[0x20], st[0x21], lem.has_float,
               lem.float_anim, _struct.unpack_from("<I", st, 0)[0])
        if not rows or rows[-1][2:] != row[2:]:
            rows.append(row)
            print(f"  t={now:7.3f}  y={lem.y:9.3f}  acc={acc:8.3f}"
                  f"  +20={row[3]} +21={row[4]}  +49={row[5]} +4b={row[6]}"
                  f"  vt={row[7]:#010x}", flush=True)
    return 0


FALLER_VT = 0x004B0A20


_needle = struct.pack("<I", FALLER_VT)   # the faller vtable, little-endian

# The state the splat branch allocates. `0x0041664C` news 0x18 bytes and
# `0x0040B3A0` is the constructor: it writes this vtable at +0, 0x1A at +0x14
# and 0 at +0x11, and `0x00416688` then installs it on the lemming through
# `0x0042E696`. So an object with this vtable in the heap *is* a lemming
# that the landing branch decided to kill -- which is how a splat is seen
# directly rather than inferred from the actor list losing an entry.
SPLAT_VT = 0x004B0668
_splat_needle = struct.pack("<I", SPLAT_VT)


def _window_round(h, slots):
    """A readable (base, size) covering the found state objects.

    Clamped to the heap region that holds them: a read that straddles a
    region boundary fails for the whole range rather than partially, which
    `measure_dropoff.readable_block` records the hard way.
    """
    from find_fallers import heap_regions

    lo, hi = min(slots), max(slots) + 0x40
    for base, size in heap_regions(h):
        if base <= lo < base + size:
            lo = max(base, lo - 0x8000)
            hi = min(base + size, hi + 0x8000)
            break
    size = (hi - lo) & ~7
    while size > 0x1000 and read_mem(h, lo, size) is None:
        size -= 0x1000
    return (lo, size) if size > 0x1000 else None


def state_slots(h) -> list[int]:
    """Heap objects whose first dword is the faller vtable `0x004B0A20`.

    One per live fall. The scan is a plain `bytes.find` for the vtable over
    the heap regions, which is what Round 2 reported as having failed -- it
    works, but only while lemmings are actually falling: with none alive the
    only hits are the two image xrefs at `0x00415FB4` / `0x00415FF4`, which
    are instructions, not objects. Those are filtered out by address.
    """
    import struct as _struct

    from find_fallers import heap_regions

    needle = _struct.pack("<I", FALLER_VT)
    out = []
    for base, size in heap_regions(h):
        d = read_mem(h, base, size)
        if not d:
            continue
        i = d.find(needle)
        while i >= 0:
            if i % 4 == 0 and base + i > 0x00500000:
                out.append(base + i)
            i = d.find(needle, i + 4)
    return sorted(set(out))


def states(h, seconds: float, hz: float, set_acc: float | None,
           set_at: float | None, poke_below: float | None = None,
           poke_above: float = -1e9, poke_every: int = 1,
           clamp_acc: float | None = None) -> int:
    """Follow the fall-state accumulator `[state+0x18]` beside the lemming's Y.

    The state object is what the binary tests: `0x004162D6` compares
    `[state+0x18]` with 10.0 and `0x004165FC` compares it with 30.0. The
    accumulator is not on the lemming, so it has to be found by scanning for
    the vtable -- but the heap blocks are reused from fall to fall, so one
    scan at the start gives a small set of slots that every later fall lands
    in, and polling those is cheap enough for a 100 Hz series.

    `--set-acc V --set-at Y` writes V into the accumulator of every tracked
    state once its lemming passes Y on the way down. That is a differential
    probe on `splatDistance`, not an observation: it asks the original's own
    landing branch what it does with a given distance fallen, with everything
    else about the fall untouched.
    """
    import struct as _struct

    print(f"ui {ui_state(h)}  loaded {loaded_level(h)!r}  clock {i32(h, CLOCK)}",
          flush=True)
    # Nothing is falling in the first second of a level, and a scan then
    # finds no state object at all. Wait for one lemming to be airborne.
    t_wait = time.perf_counter()
    while time.perf_counter() - t_wait < 30.0:
        if any((read_lem(h, p) or Lem(0, bytes(LEM_BYTES))).y > 40.0
               for p in lemmings(h)):
            break
    t0 = time.perf_counter()
    slots = state_slots(h)
    print(f"{len(slots)} fall-state objects found in"
          f" {time.perf_counter() - t0:.1f} s: "
          + " ".join(f"{a:#010x}" for a in slots[:12]), flush=True)
    if not slots:
        print("no fall state is live; are any lemmings falling?")
        return 2
    # The state objects are pooled: three addresses served every fall on this
    # level, and a fall that starts later reuses one. But a later fall can
    # also land in a block the opening scan never saw, and a full re-scan
    # costs about two seconds -- far longer than the half second in which the
    # umbrella latch flips. So the window round the slots found is re-scanned
    # on every sample instead, which is one block read and a `find`.
    win = _window_round(h, slots)
    print(f"dense window {win[0]:#010x} + {win[1] // 1024} KB" if win
          else "no readable window round the slots", flush=True)
    rows, _g = hatch_y_for(loaded_level(h))
    hatch_ys = sorted({y for _c, _x, y, _r in (rows or [])}, reverse=True)
    print(f"hatch Y words {hatch_ys}", flush=True)

    last: dict[int, tuple] = {}
    done: set[int] = set()
    seen_lems: set[int] = set()
    splats: list[tuple[float, int]] = []
    splats_now: set[int] = set()
    left: list[tuple[float, int]] = []
    clamped = 0
    poked: set[int] = set()
    seen_for_poke: list[int] = []
    period = 1.0 / hz
    nxt = 0.0
    while True:
        now = time.perf_counter() - t0
        if now > seconds:
            break
        if now < nxt:
            continue
        nxt = now + period
        lems = {}
        for ptr in lemmings(h):
            lem = read_lem(h, ptr)
            if lem is not None:
                lems[ptr] = lem
                if (poke_below is not None and ptr not in poked
                        and poke_above < lem.y < poke_below
                        and not lem.has_float):
                    seen_for_poke.append(ptr)
                    poked.add(ptr)
                    if poke_every <= 1 or (len(seen_for_poke) - 1) % poke_every == 0:
                        write_mem(h, ptr + OFF_HASFLOAT, b"\x01")
                        print(f"  t={now:7.3f}  POKE +49=1 {ptr:#010x}"
                              f" y={lem.y:.3f}", flush=True)
        live_states = []
        if win:
            d = read_mem(h, win[0], win[1])
            if d:
                i = d.find(_needle)
                while i >= 0:
                    if i % 4 == 0 and i + 0x28 <= len(d):
                        live_states.append((win[0] + i, d[i:i + 0x28]))
                    i = d.find(_needle, i + 4)
        else:
            for a in slots:
                blk = read_mem(h, a, 0x28)
                if blk:
                    live_states.append((a, blk))
        if win:
            d = read_mem(h, win[0], win[1])
            if d:
                here = set()
                i = d.find(_splat_needle)
                while i >= 0:
                    if i % 4 == 0:
                        here.add(win[0] + i)
                    i = d.find(_splat_needle, i + 4)
                # The pool reuses addresses, so count 0->1 transitions rather
                # than distinct addresses: one transition is one splat.
                for a in here - splats_now:
                    splats.append((now, a))
                    print(f"  t={now:7.3f}  SPLAT state at {a:#010x}"
                          f" (vtable {SPLAT_VT:#010x})", flush=True)
                splats_now = here
        for ptr in list(seen_lems):
            if ptr not in lems:
                seen_lems.discard(ptr)
                left.append((now, ptr))
                print(f"  t={now:7.3f}  {ptr:#010x} left the actor list",
                      flush=True)
        seen_lems |= set(lems)
        for a, blk in live_states:
            if _struct.unpack_from("<I", blk, 0)[0] != FALLER_VT:
                continue
            acc = _struct.unpack_from("<d", blk, 0x18)[0]
            if not (0.0 <= acc <= 400.0):
                continue
            latch, step_flag = blk[0x20], blk[0x21]
            # Ownership-free probe on splatDistance: hold every live faller's
            # accumulator at `clamp_acc`, so each one reaches the landing
            # branch having "fallen" that far whatever the geometry did. The
            # accumulator still gains 1.0 on the next sub-step, so the value
            # the branch tests is clamp_acc or clamp_acc + 1 -- which is why
            # the probe is run at 29 and at 31 rather than at 30.
            if clamp_acc is not None and acc > clamp_acc:
                if write_mem(h, a + 0x18, _struct.pack("<d", clamp_acc)):
                    clamped += 1
                acc = clamp_acc
            # Which lemming is this state's? The one whose fall distance from
            # its own hatch Y matches the accumulator. Nothing stores the
            # link, so it is matched rather than read.
            owner, err = None, 9e9
            for ptr, lem in lems.items():
                for hy in hatch_ys or [0.0]:
                    e = abs((hy - lem.y) - acc)
                    if e < err:
                        owner, err = (ptr, lem, hy), e
            cur = (round(acc, 3), latch, step_flag,
                   None if owner is None else owner[1].float_anim,
                   None if owner is None else owner[1].has_float)
            if last.get(a) == cur:
                continue
            last[a] = cur
            if owner is None or err > 1.5:
                print(f"  t={now:7.3f}  {a:#010x} acc={acc:8.3f}"
                      f" +20={latch} +21={step_flag}  (no owner matched,"
                      f" best err {err:.3f})", flush=True)
                continue
            ptr, lem, hy = owner
            print(f"  t={now:7.3f}  {a:#010x} acc={acc:8.3f} +20={latch}"
                  f" +21={step_flag}  lemming {ptr:#010x} y={lem.y:8.3f}"
                  f" hatch {hy:.0f} fallen={hy - lem.y:7.3f}"
                  f" +49={lem.has_float} +4b={lem.float_anim}", flush=True)
            if (set_acc is not None and set_at is not None
                    and a not in done and lem.y < set_at):
                ok = write_mem(h, a + 0x18, _struct.pack("<d", set_acc))
                done.add(a)
                print(f"  t={now:7.3f}  SET acc := {set_acc} on {a:#010x}"
                      f" (lemming {ptr:#010x} y={lem.y:.3f}) -> {ok}",
                      flush=True)
    print("")
    print(f"{len(left)} lemmings left the actor list;"
          f" {len(seen_lems)} still listed", flush=True)
    print(f"{len(splats)} splat states entered"
          f" (vtable {SPLAT_VT:#010x} appearing in the window)", flush=True)
    if clamp_acc is not None:
        print(f"accumulator held at {clamp_acc} by {clamped} writes", flush=True)
    return 0


def enter(h, name: str) -> int:
    """Enter `name` with the record swap, restoring slot [0][0] afterwards.

    `goto_level.main` calls `focus(pid)` first and that crashes the process
    through dgVoodoo on this machine, so this calls `enter_level` directly.
    `focus` is never called from here, and `goto_level` is left alone.
    """
    from lrinput import LRInput
    from goto_level import (LEVEL_TABLE, loaded_level as gl_loaded,
                            select_level, write_mem as gl_write)
    from restart_level import ensure_playing

    saved = None
    with LRInput() as inp:
        inp.clear()
        inp.enable()
        try:
            saved = select_level(h, name)
            # `ensure_playing` is the route that also knows how to leave the
            # results screen (ui_state 5), which `goto_level.enter` does not:
            # on 5 it would hand `find_menu_index` a screen with no menu and
            # refuse to press anything. The record swap has to be in place
            # when the node is clicked, so it brackets this call.
            state = 0 if ensure_playing(h, inp) else ui_state(h)
        finally:
            inp.release()
            if saved is not None:
                print(f"  slot [0][0] restored: {gl_write(h, LEVEL_TABLE, saved)}")
    print(f"ui_state {state}  loaded {gl_loaded(h)!r}"
          f"  time {i32(h, TIME_LIMIT)}  save {i32(h, SAVE_TARGET)}")
    return 0 if state == 0 else 2


def find_parasol(h, clicks: int = 3) -> int:
    """Which panel button is the parasol, measured rather than assumed.

    `measure_dropoff` found index 0 to be the climber on MM_CLIMB_002 and
    took index 1 for the parasol from the strip order; this session read
    `0x004C9200` = 7 for a click at y = 67 on E10_KF_1, so the "one index per
    40 px from y = 27" mapping does not hold on every level and the strip is
    not a fixed ten buttons.

    So this walks the buttons the panel actually draws -- `panel_buttons`
    reads their sprite rectangles out of static memory -- clicks each one,
    records what `0x004C9200` becomes, then clicks lemmings and checks
    `[lemming+0x49]`. On a level whose only stock is the floater (MM_FLOAT
    ships floater 10 and nothing else) the button that sets that byte is the
    parasol, and no other button can do anything at all.
    """
    from lrinput import LRInput
    from measure_dropoff import (click_at, click_lemming, find_pointer,
                                 lemming_slots, panel_buttons, scan)

    cross = u32(h, CROSS_PTR)
    buttons = panel_buttons(h)
    print(f"{len(buttons)} panel buttons drawn: {buttons}", flush=True)
    t0 = time.perf_counter()
    slots, _bill = scan(h)
    print(f"{len(slots)} sprite records in {time.perf_counter() - t0:.1f} s",
          flush=True)
    with LRInput() as inp:
        inp.clear()
        inp.enable()
        ptr, before, after, scale = find_pointer(h, inp, cross)
        print(f"pointer {before} -> {after} = {scale} px/unit", flush=True)
        if scale is None or scale < 0.5:
            inp.release()
            return 2
        for bx, by in buttons:
            pos = click_at(h, inp, ptr, (bx, by))
            idx = i32(h, SELECTED)
            print(f"button ({bx}, {by}): click landed {pos},"
                  f" 0x004C9200 = {idx}", flush=True)
            fast = lemming_slots(h, slots)
            last = None
            for _ in range(clicks):
                last, landed = click_lemming(h, inp, ptr, fast, avoid=last)
                if last is None:
                    fast = lemming_slots(h, slots)
                    continue
                flagged = [(p, lem) for p, lem in
                           ((p, read_lem(h, p)) for p in lemmings(h))
                           if lem is not None and lem.has_float]
                print(f"    aimed {last} landed {landed};"
                      f" {len(flagged)} lemmings have +49 set", flush=True)
                if flagged:
                    for p, lem in flagged:
                        print(f"    -> {p:#010x} y={lem.y:.3f} x={lem.x:.3f}"
                              f" +49={lem.has_float} +4b={lem.float_anim}",
                              flush=True)
                    print(f"PARASOL is the button at ({bx}, {by}),"
                          f" 0x004C9200 = {idx}", flush=True)
                    inp.release()
                    return 0
            if (i32(h, CLOCK) or 0) < 25:
                print("  out of clock", flush=True)
                break
        inp.release()
    return 2


def click_float(h, rounds: int = 4, clicks: int = 8) -> int:
    """Select the parasol and click lemmings, reusing measure_dropoff's aim.

    The panel index static `0x004C9200` is read back after the panel click, so
    a click that was thrown away is visible rather than assumed. Writing the
    hit-test statics `0x004C36A0` / `0x004C36A4` does nothing in play -- Round
    1 proved that -- so this is the glide-and-click path.
    """
    from lrinput import LRInput
    from measure_dropoff import (click_at, click_lemming, find_pointer,
                                 lemming_slots, scan)

    cross = u32(h, CROSS_PTR)
    t0 = time.perf_counter()
    slots, _bill = scan(h)
    print(f"{len(slots)} sprite records in {time.perf_counter() - t0:.1f} s",
          flush=True)
    with LRInput() as inp:
        inp.clear()
        inp.enable()
        ptr, before, after, scale = find_pointer(h, inp, cross)
        print(f"pointer {before} -> {after} = {scale} px/unit", flush=True)
        if scale is None or scale < 0.5:
            inp.release()
            return 2
        for rnd in range(rounds):
            pos = click_at(h, inp, ptr,
                           (PANEL_X, PANEL_Y0 + PANEL_DY * PARASOL_INDEX))
            print(f"round {rnd}: panel click {pos}, 0x004C9200 ="
                  f" {i32(h, SELECTED)}", flush=True)
            fast = lemming_slots(h, slots)
            last = None
            for _ in range(clicks):
                last, landed = click_lemming(h, inp, ptr, fast, avoid=last)
                if last is None:
                    fast = lemming_slots(h, slots)
                    continue
                flagged = [p for p in lemmings(h)
                           if (read_lem(h, p) or Lem(0, bytes(LEM_BYTES))).has_float]
                print(f"    aimed {last} landed {landed};"
                      f" {len(flagged)} lemmings now have +49 set", flush=True)
                if flagged:
                    inp.release()
                    return 0
        inp.release()
    return 2


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--grid-search", action="store_true")
    ap.add_argument("--dir", default=CORPUS)
    ap.add_argument("--enter", metavar="LEVEL")
    ap.add_argument("--watch", type=float, metavar="SECONDS")
    ap.add_argument("--hz", type=float, default=60.0)
    ap.add_argument("--poke-float", type=float, metavar="Y",
                    help="write [lemming+0x49]=1 when a lemming's Y drops below this")
    ap.add_argument("--poke-above", type=float, default=-1e9, metavar="Y",
                    help="only poke while Y is above this, so the poke lands"
                         " at a chosen point in the fall")
    ap.add_argument("--poke-every", type=int, default=1, metavar="N",
                    help="poke only every Nth lemming; the rest are controls")
    ap.add_argument("--click-float", action="store_true")
    ap.add_argument("--find-parasol", action="store_true")
    ap.add_argument("--dump-lemming", action="store_true")
    ap.add_argument("--find-state", action="store_true")
    ap.add_argument("--states", type=float, metavar="SECONDS",
                    help="follow the fall-state accumulator [state+0x18]")
    ap.add_argument("--set-acc", type=float, metavar="V")
    ap.add_argument("--set-at", type=float, metavar="Y")
    ap.add_argument("--clamp-acc", type=float, metavar="V",
                    help="hold every live faller's [state+0x18] at V")
    ap.add_argument("--watch-state", nargs=2, metavar=("STATE", "LEMMING"),
                    help="hex addresses from --find-state")
    ap.add_argument("--snapshot", action="store_true")
    args = ap.parse_args()

    if args.grid_search:
        return grid_search(args.dir)

    need_write = bool(args.enter or args.poke_float is not None)
    h, pid = attach(write=need_write)
    if not h:
        print("the game is not running")
        return 1
    print(f"pid {pid}")
    try:
        if args.enter:
            rc = enter(h, args.enter)
            if rc:
                return rc
        if args.snapshot:
            print(f"ui {ui_state(h)}  loaded {loaded_level(h)!r}"
                  f"  clock {i32(h, CLOCK)}  released {i32(h, RELEASED)}")
            for p in lemmings(h):
                lem = read_lem(h, p)
                if lem:
                    print(f"  {p:#010x} y={lem.y:.3f} x={lem.x:.3f}"
                          f" dir={lem.direction} +49={lem.has_float}"
                          f" +4b={lem.float_anim} +4c={lem.kind}")
        if args.dump_lemming:
            return dump_lemming(h)
        if args.find_state:
            return find_state(h)
        if args.states:
            return states(h, args.states, args.hz, args.set_acc, args.set_at,
                          args.poke_float, args.poke_above, args.poke_every,
                          args.clamp_acc)
        if args.watch_state:
            return watch_state(h, args.watch or 10.0,
                               int(args.watch_state[0], 16),
                               int(args.watch_state[1], 16), args.hz)
        if args.find_parasol:
            rc = find_parasol(h)
            print(f"find_parasol rc {rc}")
            if args.watch is None and args.states is None:
                return rc
        if args.click_float:
            rc = click_float(h)
            print(f"click_float rc {rc}")
        if args.watch:
            return watch(h, args.watch, args.poke_float, args.hz,
                     args.poke_above, args.poke_every)
        return 0
    finally:
        k32.CloseHandle(h)


if __name__ == "__main__":
    raise SystemExit(main())
