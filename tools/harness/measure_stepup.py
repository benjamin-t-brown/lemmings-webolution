"""Read `Rules::maxStepUp` and `Rules::lemmingHeight` off the original.

Phase 3 round 4. Both fields are still the classic-Lemmings numbers (3 and 4)
and `docs/scratch/phase3-round4.md` has a static read saying they should be 2
and 3. That note has no live series; this script is the series.

What the binary says, verified byte by byte with `read_steppers.py --at`
------------------------------------------------------------------------
The walk stepper is `0x0041BDD4`, slot 1 of the walk vtable `0x004B0D28`. It
loads only three `.rdata` doubles -- 2.0, 0.0 and 5.0 -- so **the step-up is
not a constant in the stepper**. Per fully-consumed slice it

    0041be70  fld [0x004b10b0]      ; 5.0, world units a second
    0041be7c  fimul [eax+0x40]      ; * facing
    0041be7f  fadd  [edx+8]         ; + X
    0041be95  fstp  [ecx+8]         ; -> X        (so X moves first)
    0041bece  call 0x49c5ac         ; truncate X
    0041bee6  call 0x41bffc         ; the mask probe

and `0x0041BFFC` is where the row changes. Its step-up arm is **unrolled, two
deep**, with the third solid cell being a wall:

    0041c47d  and edx, 1            ; cell(X, Y)      & 1
    0041c482  je  0x41c6ef          ;   clear -> the step-DOWN arm
    0041c48e  add ecx, [eax+0x44]   ; row += vertical direction  (= +1)
    0041c517  and ecx, 1            ; cell(X, Y+1)    & 1
    0041c51c  je  0x41c6ae          ;   clear -> adopt this row   (rise 1)
    0041c528  add eax, [edx+0x44]   ; row += direction again
    0041c5b1  and eax, 1            ; cell(X, Y+2)    & 1
    0041c5b6  je  0x41c6ae          ;   clear -> adopt this row   (rise 2)
    0041c5c1  mov dl, [ecx+0x48]    ; third solid cell: climber byte
    0041c5c6  je  0x41c676          ;   not a climber -> 0x42f66b, turn round

`0x0041C6AE` is the adopt path and passes the row to `0x0042EE74`, which
stores Y. There is **no third `add [+0x44]`** on that arm: the whole function
through its `ret 4` at `0x0041C872` has `add ..., [reg+0x44]` only at
`0x0041C48E`, `0x0041C528` and `0x0041C81B` (the last one is on the step-down
arm), and `sub` only at `0x0041C6FF` and `0x0041C741`. `[+0x44]` is the
integer 1, written at spawn by `0x0042DFE7`, and `0x0042F5E3` only ever
negates it. So **the greatest rise this probe can write is 2 world units**
and a stack of three solid cells in the forward column is a wall.

The step-down arm is one cell for the same reason: two `sub [+0x44]`, and two
empty cells take the fall transition at `0x0041C77D` (`push 0x28`).

The climb head test is `0x004139BF`, called only from `0x004138E8` inside the
climb stepper `0x004137CA` (slot 1 of `0x004B0960`). It loads no `.rdata`
double at all and has no immediate 3 or 4. It gets its position from
`0x0042F35F`, which returns `X + [+0x350]` and `Y + [+0x358]`, and `[+0x358]`
is the double **3.0** -- `mov dword [ecx+0x35c], 0x40080000` at `0x0042E0FC`
in the lemming constructor and again at `0x0042F616` whenever `0x0042F5E3`
leaves `[+0x44] == 1`. Two probes come off that biased position:

    00413a53  call 0x40de60         ; cell(trunc(X+[+0x350]) - [+0x40], Y+3)
    00413a5b  jne 0x413a70          ;   nonzero -> "Climber hit head" (0x4BF2FC)
    00413b22  sub eax, [edx+0x44]   ; row = (Y+3) - 1
    00413b50  call 0x40de60         ; cell(trunc(X+[+0x350]),          Y+2)
    00413b58  jne 0x413bfd          ;   nonzero -> keep climbing
                                    ;   zero    -> "Climber topping out"

So the head-room sample is **3 above the feet** and the top-out sample is
**2 above the feet**; the second is the first minus one direction unit, and
3.0 is the only constant. `World::stepClimber` uses `lemmingHeight` for both.

One more thing that arm needs, and that the static note missed: `0x0042F66B`,
the turn, writes the **X bias** as well as the facing --

    0042f684  cmp [eax+0x40], -1
    0042f68a  mov dword [ecx+0x354], 0xbff00000   ; facing -1 -> [+0x350] = -1.0
    0042f6ae  mov dword [edx+0x364], 0xbff00000   ; facing -1 -> [+0x360] = -1.0
    0042f6bd  mov dword [eax+0x350], 0            ; facing +1 -> both 0.0

and `[+0x360]` is the X bias the walk probe's own position helper
`0x0042F22E` adds. X has already moved by the facing when the probe runs, so
the column it reads is `X_old + 1` facing right and `X_old - 2` facing left.
That asymmetry is in `walk_sim` below; it is an observation off those four
stores, not a guess.

Modes
-----
    python measure_stepup.py --grid-search
        No game. For every level, walk the exe's own rules out of each hatch
        landing under a rise limit of 2 and again under 3, and print the
        levels where the two trajectories diverge soonest. That is the
        offline prediction, and it is made before the game is started.

    python measure_stepup.py --sites LEVEL
        The surface profile of one level and every adjacent rise on it, so a
        live series can be read against named columns.

    python measure_stepup.py --enter LEVEL
        Enter it through goto_level's record swap, slot [0][0] restored
        after. Never calls goto_level.focus -- that kills the process.

    python measure_stepup.py --enter LEVEL --walk 30 --hz 120
        Follow every lemming in the actor list, keep each one's distinct
        (X, Y, facing), then classify every column step against the stored
        grid: which rise it adopted, and at which rise it turned instead.
        That is the two-sided bound on maxStepUp. No input is sent and no
        memory is written.

    python measure_stepup.py --enter LEVEL --walk 30 --hz 120 --poke-climb
        The same, but write 1 into `[lemming+0x48]` -- the climber byte the
        wall arm reads at `0x0041C5C1` -- so walkers that meet a wall climb
        it instead of turning. Then the top-out height is the series on
        `lemmingHeight`. A probe, labelled as one, exactly like round 3b's
        `[+0x49]`: it writes the byte the original's own branch reads and
        asks what that branch does.
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
    LEMMING_VT,
    OTHER_VT,
    actor_ptrs,
    hatch_words,
    loaded_level,
)
from memdump import k32  # noqa: E402
from read_lvl_grid import GRID_WIDTH, find_grid  # noqa: E402

CORPUS = "build/lvl_dump/levels/COMPILED"
CLOCK = 0x004CBAAC
RELEASED = 0x004CBA78

# Offsets inside the lemming object. X, Y and direction are measure_spawn's,
# read off the fall stepper. +0x40 is the facing `0x0041BE7C` multiplies the
# walk speed by; +0x44 the vertical direction the probe adds to the row;
# +0x48 the climber byte `0x0041C5C1` reads.
OFF_X, OFF_Y, OFF_FACING, OFF_VDIR, OFF_CLIMB = 0x08, 0x10, 0x40, 0x44, 0x48
LEM_BYTES = 0x80

# The two cell tests are not the same test, and the difference is in the
# bytes: the step-up arm masks with 1 (`and edx, 1` at 0x0041C47D) and the
# step-down arm compares the whole dword with zero (`cmp dword [eax], 0` at
# 0x0041C732). read_lvl_grid.solid is a third one again -- the faller's
# landing probe, nonzero with bit 0x80 clear.


def step_solid(cell: int) -> bool:
    """The step-up arm's test: bit 0x1 only (0x0041C47D / C517 / C5B1)."""
    return bool(cell & 1)


def any_solid(cell: int) -> bool:
    """The step-down arm's test: the whole dword (0x0041C732 / C774)."""
    return cell != 0


# --- offline ---------------------------------------------------------------


def grid_of(path: str):
    found = find_grid(open(path, "rb").read())
    if not found:
        return None
    _off, cells = found
    return cells, len(cells) // GRID_WIDTH


def surface(cells, height: int, x: int, top: int | None = None):
    """Highest row at or below `top` in column `x` that the step arm calls solid."""
    start = height - 1 if top is None else min(top, height - 1)
    for row in range(start, -1, -1):
        if step_solid(cells[row * GRID_WIDTH + x]):
            return row
    return None


def rise_at(cells, height: int, col: int, feet: int, limit: int):
    """What the probe does for feet row `feet` in column `col`.

    Returns ('up', k) for a rise of k adopted, ('flat', 0), ('down', 1),
    ('fall', 0), or ('wall', limit + 1). `limit` is how deep the unrolled
    arm goes -- 2 in the shipped bytes.
    """
    def cell(row):
        if not 0 <= row < height:
            return 0
        return cells[row * GRID_WIDTH + col]

    if step_solid(cell(feet)):
        for k in range(1, limit + 1):
            if not step_solid(cell(feet + k)):
                return "up", k
        return "wall", limit + 1
    if any_solid(cell(feet - 1)):
        return "flat", 0
    if any_solid(cell(feet - 2)):
        return "down", 1
    return "fall", 0


def walk_sim(cells, height: int, x: int, feet: int, facing: int,
             limit: int, steps: int):
    """The walk arm, as the bytes write it, for `steps` ticks.

    X moves by the facing first (`0x0041BE7F`, then truncated at
    `0x0041BECE`), and only then is the probe column `X + [+0x360]` --
    0.0 facing right, -1.0 facing left (`0x0042F6BD` / `0x0042F6AE`).
    """
    out = []
    for _ in range(steps):
        x = (x + facing) % GRID_WIDTH
        col = (x + (0 if facing > 0 else -1)) % GRID_WIDTH
        kind, k = rise_at(cells, height, col, feet, limit)
        if kind == "up":
            feet += k
        elif kind == "down":
            feet -= 1
        elif kind == "fall":
            row = surface(cells, height, col, feet - 1)
            out.append((x, feet, facing, "fall", k))
            if row is None:
                break
            feet = row + 1
            out.append((x, feet, facing, "land", 0))
            continue
        elif kind == "wall":
            out.append((x, feet, facing, "wall", k))
            facing = -facing
            continue
        out.append((x, feet, facing, kind, k))
    return out


def divergence(cells, height, x, feet, facing, steps=400):
    """First tick at which rise-limit 2 and rise-limit 3 part company."""
    a = walk_sim(cells, height, x, feet, facing, 2, steps)
    b = walk_sim(cells, height, x, feet, facing, 3, steps)
    for i, (pa, pb) in enumerate(zip(a, b)):
        if pa[:2] != pb[:2] or pa[3] != pb[3]:
            return i, pa, pb
    return None, None, None


def stack(cells, height, col, feet, cap=12):
    """How many consecutive step-solid cells the probe would see at `feet`."""
    n = 0
    while n < cap and 0 <= feet + n < height \
            and step_solid(cells[(feet + n) * GRID_WIDTH + col]):
        n += 1
    return n


def reachable(cells, height, starts, limit=2, cap=200000):
    """Every (col, feet) a walker under `limit` can stand on, from `starts`.

    Breadth first, so the distance is in ticks. Both facings are explored
    from every state because a turn is one tick and the X bias makes the two
    directions read different columns.
    """
    from collections import deque

    seen = {}
    q = deque()
    for st in starts:
        if st not in seen:
            seen[st] = 0
            q.append(st)
    while q and len(seen) < cap:
        col, feet = q.popleft()
        d = seen[(col, feet)]
        for facing in (1, -1):
            x2 = (col + facing) % GRID_WIDTH
            probe = (x2 + (0 if facing > 0 else -1)) % GRID_WIDTH
            kind, k = rise_at(cells, height, probe, feet, limit)
            if kind == "wall":
                continue
            nf = feet + k if kind == "up" else feet - 1 if kind == "down" else feet
            if kind == "fall":
                row = surface(cells, height, probe, feet - 1)
                if row is None or feet - (row + 1) > 30:
                    continue
                nf = row + 1
            nxt = (x2, nf)
            if nxt not in seen:
                seen[nxt] = d + 1
                q.append(nxt)
    return seen


def discriminating(directory: str) -> int:
    """Reachable places where a rise of exactly 3 decides the question.

    A wall of four or more is refused whether the limit is 2 or 3, so it
    says nothing. Only a stack of exactly three solid cells in the probe
    column separates the two, and this is where a walker can reach one.
    """
    files = sorted(glob.glob(os.path.join(directory, "*.LVL")))
    rows = []
    for path in files:
        g = grid_of(path)
        if not g:
            continue
        cells, height = g
        name = os.path.basename(path)[:-4]
        data = open(path, "rb").read()
        starts = []
        for hw in hatch_words(data):
            x = hw["x"] % GRID_WIDTH
            row = surface(cells, height, x, min(hw["y"], height - 1))
            if row is None or hw["y"] - row > 30:
                continue
            starts.append((x, row + 1))
        if not starts:
            continue
        seen = reachable(cells, height, starts)
        best = None
        rises = {}
        for (col, feet), d in seen.items():
            for facing in (1, -1):
                x2 = (col + facing) % GRID_WIDTH
                probe = (x2 + (0 if facing > 0 else -1)) % GRID_WIDTH
                n = stack(cells, height, probe, feet)
                if n:
                    rises[n] = rises.get(n, 0) + 1
                if n == 3 and (best is None or d < best[0]):
                    best = (d, col, feet, facing, probe)
        rows.append((best[0] if best else 10 ** 9, name, len(starts),
                     len(seen), best, dict(sorted(rises.items()))))
    rows.sort()
    print("# levels where a walker can reach a stack of exactly 3, and how far")
    print("#  ticks  level             hatches  states   nearest (col,feet,dir,probe)"
          "   stacks seen in the probe column")
    for d, name, nh, ns, best, rises in rows:
        where = "-" if best is None else f"({best[1]:3d},{best[2]:3d},{best[3]:+d},{best[4]:3d})"
        dd = "   -" if best is None else f"{d:4d}"
        print(f"  {dd}  {name:16s}  {nh:7d}  {ns:6d}   {where:22s}  {rises}")
    reach3 = [r for r in rows if r[4] is not None]
    print(f"\n# {len(reach3)} of {len(rows)} levels have one reachable;"
          f" nearest overall {reach3[0][1] if reach3 else '-'}"
          f" at {reach3[0][0] if reach3 else '-'} ticks")
    return 0


def grid_search(directory: str) -> int:
    files = sorted(glob.glob(os.path.join(directory, "*.LVL")))
    if not files:
        print(f"no .LVL files under {directory}", file=sys.stderr)
        return 1
    found = []
    for path in files:
        g = grid_of(path)
        if not g:
            continue
        cells, height = g
        name = os.path.basename(path)[:-4]
        data = open(path, "rb").read()
        for hw in hatch_words(data):
            x = hw["x"] % GRID_WIDTH
            row = surface(cells, height, x, min(hw["y"], height - 1))
            if row is None:
                continue
            feet = row + 1
            # splatDistance is 30 (round 3b, measured). A hatch that drops
            # further than that lands nobody, so its walk never happens --
            # E10_KF_1 is the whole corpus's long-drop level and every
            # lemming on it splats.
            drop = hw["y"] - row
            for facing in (1, -1):
                i, pa, pb = divergence(cells, height, x, feet, facing)
                if i is None:
                    continue
                rises = {}
                for p in walk_sim(cells, height, x, feet, facing, 2, i + 1):
                    if p[3] == "up":
                        rises[p[4]] = rises.get(p[4], 0) + 1
                found.append((drop > 30, i, name, hw["count"], x, feet,
                              facing, drop, pa, pb, rises))
    found.sort()
    print("# where a rise limit of 2 and a rise limit of 3 part company")
    print("# (walking the exe's own arm out of a hatch landing)")
    print("# splat = the hatch drop exceeds splatDistance 30, so nobody")
    print("#         survives to walk it")
    print("#  tick  level             count  hatch        drop  dir"
          "   limit2 -> (col,feet,what)  limit3 -> (col,feet,what)  rises taken")
    for splat, i, name, count, x, feet, facing, drop, pa, pb, rises in found[:30]:
        mark = "SPLAT" if splat else "     "
        print(f" {mark} {i:5d}  {name:16s}  {count:5d}  ({x:3d},feet{feet:3d})"
              f" {drop:4d}  {facing:+d}    ({pa[0]:3d},{pa[1]:3d},{pa[3]:5s})"
              f"        ({pb[0]:3d},{pb[1]:3d},{pb[3]:5s})"
              f"      {dict(sorted(rises.items()))}")
    print(f"\n# {len(found)} hatch/direction pairs diverge inside 400 ticks,"
          f" {sum(1 for f in found if not f[0])} of them on a survivable drop")
    return 0


def sites(level: str) -> int:
    path = os.path.join(CORPUS, level.upper() + ".LVL")
    g = grid_of(path)
    if not g:
        print(f"no grid in {path}", file=sys.stderr)
        return 1
    cells, height = g
    data = open(path, "rb").read()
    prof = [surface(cells, height, c) for c in range(GRID_WIDTH)]
    print(f"# {level}  {GRID_WIDTH} x {height}")
    print("hatch records (count, column, Y word) -> landing row, feet:")
    for hw in hatch_words(data):
        x = hw["x"] % GRID_WIDTH
        row = surface(cells, height, x, min(hw["y"], height - 1))
        print(f"  ({hw['count']}, {x}, {hw['y']})  row {row}"
              f"  feet {None if row is None else row + 1}")
    print("\nadjacent rises (col -> col+1), grouped by size:")
    by = {}
    for c in range(GRID_WIDTH):
        d = (c + 1) % GRID_WIDTH
        if prof[c] is None or prof[d] is None:
            continue
        by.setdefault(prof[d] - prof[c], []).append(c)
    for k in sorted(by):
        if k == 0:
            continue
        print(f"  {k:+3d}: {len(by[k]):3d} places  {by[k][:24]}")
    print("\nsurface profile, 32 columns a line:")
    for base in range(0, GRID_WIDTH, 32):
        row = " ".join("--" if prof[c] is None else f"{prof[c]:2d}"
                       for c in range(base, base + 32))
        print(f"  {base:3d}: {row}")
    for facing in (1, -1):
        for hw in hatch_words(data):
            x = hw["x"] % GRID_WIDTH
            row = surface(cells, height, x, min(hw["y"], height - 1))
            if row is None:
                continue
            print(f"\nwalk from hatch column {x} feet {row + 1} facing"
                  f" {facing:+d}, rise limit 2, 40 ticks:")
            for p in walk_sim(cells, height, x, row + 1, facing, 2, 40):
                print(f"    col {p[0]:3d} feet {p[1]:3d} dir {p[2]:+d}"
                      f"  {p[3]} {p[4]}")
    return 0


# --- live ------------------------------------------------------------------


def write_mem(h, addr: int, data: bytes) -> bool:
    import ctypes

    n = ctypes.c_size_t(0)
    ok = k32.WriteProcessMemory(h, ctypes.c_void_p(addr), data, len(data),
                                ctypes.byref(n))
    return bool(ok) and n.value == len(data)


# The world object the cell probe reads, and its grid. Addresses from
# read_grid.py, which took them off `0x0040DE60`: 256-wide, row-major u32
# flag cells, the row count at WORLD+0x418 and the data pointer at
# WORLD+0x41C+0x0C.
WORLD = 0x004C6A10
ROW_COUNT = WORLD + 0x418
GRID_DATA_PTR = WORLD + 0x41C + 0x0C


def grid_addr(h, col: int, row: int):
    base = u32(h, GRID_DATA_PTR)
    rows = i32(h, ROW_COUNT)
    if not base or rows is None or not 0 <= row < rows:
        return None
    return base + ((row << 8) + (col & 0xFF)) * 4


def read_cell(h, col: int, row: int):
    a = grid_addr(h, col, row)
    if a is None:
        return None
    raw = read_mem(h, a, 4)
    return struct.unpack("<I", raw)[0] if raw and len(raw) == 4 else None


def carve(h, pillars, feet: int):
    """Build single-column pillars of a chosen height in the live grid.

    A probe, labelled as one, in the same shape as round 3b's `[+0x49]`
    write: it changes nothing in the code and asks the original's own
    step-up arm what it does with a stack of exactly N solid cells. Bit
    0x1 is the bit that arm tests (`and edx, 1` at 0x0041C47D), so that is
    the bit written.

    A one-column pillar resets itself: standing on top of a pillar of 1 the
    next column is a one-cell step down, and of 2 or 3 it is a fall of 2 or
    3 world units -- well inside splatDistance 30.

    Returns the originals so they can be put back.
    """
    saved = []
    for col, n in pillars:
        for k in range(n):
            row = feet + k
            a = grid_addr(h, col, row)
            if a is None:
                continue
            old = read_cell(h, col, row)
            saved.append((a, old))
            write_mem(h, a, struct.pack("<I", (old or 0) | 1))
    print(f"carved {pillars} at feet row {feet}: "
          f"{len(saved)} cells written", flush=True)
    return saved


def uncarve(h, saved) -> None:
    for a, old in reversed(saved):
        write_mem(h, a, struct.pack("<I", old or 0))
    print(f"restored {len(saved)} grid cells", flush=True)


class Lem:
    __slots__ = ("ptr", "vt", "x", "y", "facing", "vdir", "climb")

    def __init__(self, ptr: int, buf: bytes):
        self.ptr = ptr
        self.vt = struct.unpack_from("<I", buf, 0)[0]
        self.x = struct.unpack_from("<d", buf, OFF_X)[0]
        self.y = struct.unpack_from("<d", buf, OFF_Y)[0]
        self.facing = struct.unpack_from("<i", buf, OFF_FACING)[0]
        self.vdir = struct.unpack_from("<i", buf, OFF_VDIR)[0]
        self.climb = buf[OFF_CLIMB]


def read_lem(h, ptr: int):
    buf = read_mem(h, ptr, LEM_BYTES)
    if not buf or len(buf) != LEM_BYTES:
        return None
    lem = Lem(ptr, buf)
    return lem if lem.vt in (LEMMING_VT, OTHER_VT) else None


def lemmings(h) -> list[int]:
    out = []
    for p in actor_ptrs(h):
        if u32(h, p) in (LEMMING_VT, OTHER_VT):
            out.append(p)
    return out


def level_grid(h):
    stem = loaded_level(h).replace("/", "\\").split("\\")[-1]
    if stem.lower().endswith(".lvl"):
        stem = stem[:-4]
    path = os.path.join(CORPUS, stem + ".LVL")
    return (stem, grid_of(path)) if os.path.exists(path) else (stem, None)


def walk(h, seconds: float, hz: float, poke_climb: bool,
         pillars=None, feet_row: int | None = None, cells_arg=None) -> int:
    stem, g = level_grid(h)
    print(f"ui {ui_state(h)}  loaded {stem!r}  clock {i32(h, CLOCK)}"
          f"  released {i32(h, RELEASED)}  grid {'yes' if g else 'NO'}",
          flush=True)
    saved = []
    if cells_arg:
        # Single cells at named rows, for the climb: a ceiling over the open
        # column behind the wall is what the head test at 0x004139BF reads.
        for col, row in cells_arg:
            a = grid_addr(h, col, row)
            if a is None:
                continue
            old = read_cell(h, col, row)
            saved.append((a, old))
            write_mem(h, a, struct.pack("<I", (old or 0) | 1))
            print(f"ceiling cell ({col}, {row}) = "
                  f"{hex(read_cell(h, col, row))}", flush=True)
    if pillars and feet_row is not None:
        # `+=`, not `=`: a `--cells` ceiling was appended above and an
        # assignment here would drop it from the restore list.
        saved += carve(h, pillars, feet_row)
        for col, n in pillars:
            got = [read_cell(h, col, feet_row + k) for k in range(n + 1)]
            print(f"  col {col}: cells from row {feet_row} = "
                  f"{[None if c is None else hex(c) for c in got]}", flush=True)
    series: dict[tuple, list] = {}
    gen: dict[int, int] = {}
    gone: dict[int, float] = {}
    poked: set[int] = set()
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
            if ptr in gone:
                gen[ptr] = gen.get(ptr, 0) + 1
                gone.pop(ptr)
                poked.discard(ptr)
            if poke_climb and ptr not in poked and not lem.climb:
                ok = write_mem(h, ptr + OFF_CLIMB, b"\x01")
                poked.add(ptr)
                print(f"  t={now:7.3f}  POKE +48=1 on {ptr:#010x}"
                      f" at ({lem.x:.3f}, {lem.y:.3f}) -> {ok}", flush=True)
            key = (ptr, gen.get(ptr, 0))
            s = series.setdefault(key, [])
            cur = (lem.y, lem.x, lem.facing, lem.vdir, lem.climb)
            if not s or s[-1][1:] != cur:
                s.append((now,) + cur)
        for ptr, generation in list(series):
            if (ptr not in live and ptr not in gone
                    and generation == gen.get(ptr, 0)):
                gone[ptr] = now
                print(f"  t={now:7.3f}  {ptr:#010x} left the actor list",
                      flush=True)
    elapsed = max(1e-6, time.perf_counter() - t0)
    print(f"\n{samples} object reads in {elapsed:.1f} s"
          f" = {samples / elapsed:.0f} /s", flush=True)
    if saved:
        uncarve(h, saved)
    classify(series, g, pillars, feet_row)
    return 0


def classify(series: dict, g, pillars=None, feet_row: int | None = None) -> None:
    """Every column step a tracked lemming took, against the stored grid.

    The question is two-sided. A rise adopted is a lower bound on the limit;
    a rise refused -- the facing flipping with that many solid cells in the
    probe column -- is an upper bound. The probe column is the one the bytes
    read: X after the move, minus one more when facing left.
    """
    cells = height = None
    if g:
        cells, height = g
        if pillars and feet_row is not None:
            # The stored grid is the geometry the probe saw everywhere except
            # the pillars this run wrote, so overlay them before reading a
            # turn against it. Otherwise a carved wall reports as empty.
            cells = list(cells)
            for col, n in pillars:
                for k in range(n):
                    cells[(feet_row + k) * GRID_WIDTH + (col % GRID_WIDTH)] |= 1
    adopted: dict[int, int] = {}
    refused: dict[int, int] = {}
    climbs: list = []
    events: list = []
    print("\n--- per lemming ---")
    for (ptr, generation), s in sorted(series.items()):
        if len(s) < 3:
            continue
        print(f"\n{ptr:#010x} gen {generation}, {len(s)} distinct samples")
        for i, (t, y, x, facing, vdir, climb) in enumerate(s):
            tag = ""
            if i:
                pt, py, px, pf, pv, pc = s[i - 1]
                dy, dx = y - py, x - px
                tag = f"  dy {dy:+.3f} dx {dx:+.3f}"
                if pf != facing:
                    tag += f"  TURN {pf:+d}->{facing:+d}"
                    if cells is not None:
                        feet = int(round(py))
                        # X moved by the facing before the probe ran, and the
                        # facing-left bias takes one more column off, so the
                        # column read is trunc(X)+1 going right and
                        # trunc(X)-2 going left. The sample is a frame early,
                        # so both candidates are printed rather than guessed.
                        cands = ([int(px) + 1, int(px) + 2] if pf > 0
                                 else [int(px) - 2, int(px) - 1])
                        seen_n = []
                        for col in cands:
                            col %= GRID_WIDTH
                            n = 0
                            while (n < 12 and 0 <= feet + n < height
                                   and step_solid(
                                       cells[(feet + n) * GRID_WIDTH + col])):
                                n += 1
                            seen_n.append((col, n))
                        tag += f" feet {feet} probe candidates {seen_n}"
                        best = max((n for _c, n in seen_n), default=0)
                        if best >= 1:
                            refused[best] = refused.get(best, 0) + 1
                    events.append(("turn", px, 0))
                elif dy > 0.5 and pc == 0:
                    k = int(round(dy))
                    adopted[k] = adopted.get(k, 0) + 1
                    events.append(("rose", x, k))
                    tag += f"  ROSE {k}"
                elif pc and dy > 0.5 and abs(dx) < 0.5:
                    climbs.append((ptr, t, py, y, x))
            print(f"  t={t:7.3f}  y {y:8.3f}  x {x:8.3f}"
                  f"  facing {facing:+d} vdir {vdir:+d} climb {climb}{tag}")
    print("\n--- SUMMARY ---")
    print(f"rises adopted while walking (dy on a column step): {dict(sorted(adopted.items()))}")
    print(f"solid cells in the probe column at a turn:          {dict(sorted(refused.items()))}")
    if adopted:
        print(f"largest rise surmounted = {max(adopted)}"
              f"   -> maxStepUp >= {max(adopted)}")
    if refused:
        print(f"smallest wall refused   = {min(refused)} solid cells"
              f"   -> maxStepUp <= {min(refused) - 1}")
    if climbs:
        print(f"{len(climbs)} climbing steps seen (climb byte set)")
    if pillars:
        print("\n--- the carved pillars ---")
        print("# a pillar of N is a stack of exactly N solid cells above the"
              " feet row;\n# 'rose' means the step-up arm adopted it,"
              " 'turned' means it was a wall")
        for col, n in pillars:
            rose = [e for e in events
                    if e[0] == "rose" and abs(e[1] - col) < 1.5]
            turned = [e for e in events
                      if e[0] == "turn" and abs(e[1] - col) < 2.5]
            print(f"  col {col:3d} pillar {n}:  rose {len(rose)}"
                  f" {sorted({e[2] for e in rose})}"
                  f"   turned {len(turned)}")


def enter(h, name: str) -> int:
    """Enter `name` with the record swap, restoring slot [0][0] afterwards.

    Lifted verbatim in shape from measure_float_splat.enter: never calls
    goto_level.focus, which kills the process through dgVoodoo, and uses
    restart_level.ensure_playing because that one knows how to leave the
    results screen.
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
            state = 0 if ensure_playing(h, inp) else ui_state(h)
        finally:
            inp.release()
            if saved is not None:
                print(f"  slot [0][0] restored: {gl_write(h, LEVEL_TABLE, saved)}")
    print(f"ui_state {state}  loaded {gl_loaded(h)!r}")
    return 0 if state == 0 else 2


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--grid-search", action="store_true")
    ap.add_argument("--discriminating", action="store_true")
    ap.add_argument("--sites", metavar="LEVEL")
    ap.add_argument("--corpus", default=CORPUS)
    ap.add_argument("--enter", metavar="LEVEL")
    ap.add_argument("--walk", type=float, metavar="SECONDS")
    ap.add_argument("--hz", type=float, default=120.0)
    ap.add_argument("--poke-climb", action="store_true")
    ap.add_argument("--carve", metavar="COL:N,COL:N",
                    help="write single-column pillars of N solid cells into"
                         " the live grid, starting at --carve-row")
    ap.add_argument("--carve-row", type=int, metavar="FEET",
                    help="the feet row the pillars start on")
    ap.add_argument("--cells", metavar="COL:ROW,COL:ROW",
                    help="set bit 0x1 on single cells at named rows")
    args = ap.parse_args()

    if args.grid_search:
        return grid_search(args.corpus)
    if args.discriminating:
        return discriminating(args.corpus)
    if args.sites:
        return sites(args.sites)
    if not (args.enter or args.walk is not None):
        ap.print_help()
        return 0

    h, pid = attach(write=True)
    if h is None:
        print("no Lemmings Revolution process", file=sys.stderr)
        return 1
    print(f"attached pid {pid}", flush=True)
    if args.enter:
        rc = enter(h, args.enter)
        if rc:
            return rc
    if args.walk is not None:
        pillars = None
        if args.carve:
            pillars = [(int(p.split(":")[0]), int(p.split(":")[1]))
                       for p in args.carve.split(",")]
            if args.carve_row is None:
                print("--carve needs --carve-row", file=sys.stderr)
                return 1
        cells_arg = None
        if args.cells:
            cells_arg = [(int(p.split(":")[0]), int(p.split(":")[1]))
                         for p in args.cells.split(",")]
        return walk(h, args.walk, args.hz, args.poke_climb,
                    pillars, args.carve_row, cells_arg)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
