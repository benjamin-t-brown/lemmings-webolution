"""When a builder lays a brick, and which cells it writes -- live.

Phase 3 round 6, the live half. `docs/scratch/phase3-round6.md` is the static
read this tests and does not repeat. The parts of it this script depends on,
each with the instruction:

The build state
---------------
    state vtable            0x004B08DC  (mov [edx], 0x4b08dc at 0x0040F93F)
    constructor             0x0040F8D1
    enter hook (vtable +8)  0x0040FAB6, `build` animation site 0x0040FB83
    step      (vtable +4)   0x00410302, tail `call 0x41b077` at 0x0041047B

so the builder is driven by the SAME staggered-action stepper as bash / dig /
mine (round 5): `0x0041B077` turns the animation's progress into an integer
percent and calls the record whose stored frame equals it. The constructor
registers exactly two records, through 0x0040C860 into the list at state+0x18:

    0040f9a8  c7 40 08 51 00 00 00  mov [eax+8], 0x51     ; frame 81
    0040f9b2  c7 41 0c 1c 01 41 00  mov [ecx+0xc], 0x41011c   ; the brick stamp
    0040fa06  c7 42 08 5a 00 00 00  mov [edx+8], 0x5a     ; frame 90
    0040fa10  c7 40 0c cc fb 40 00  mov [eax+0xc], 0x40fbcc   ; advance + count

and the brick counter at state+0x3C starts at 12:

    0040f948  c7 40 3c 0c 00 00 00  mov [eax+0x3c], 0xc

The stamp, 0x0041011C
---------------------
    00410141  call 0x42f22e          ; position: X + [+0x360], Y + [+0x368]
    00410150  call 0x49c5ac          ; trunc X
    00410166  call 0x40dec0          ; wrap  X into [0, 256)
    00410191  call 0x40df50          ; clamp Y into [0, height)
    00410199  mov  [ebp-0x10], 1     ; i = 1
    004101ab  cmp  [ebp-0x10], 3     ; while i < 3   -> i = 1 and 2, TWO cells
    004101b7  imul eax, [edx+0x40]   ; i * facing
    004101eb  call 0x40de60          ; the cell at (truncX + i*facing, truncY)
    004101f8  or   edx, 1            ; THE BRICK: or the solid bit
    004101fe  mov  dword ptr [eax], edx
    00410286  call 0x43b016          ; the surface-mesh update for those two
                                     ;   columns; 0x0043AE6C reaches no
                                     ;   0x0040DE60 and no `or`

So the predicted brick is two cells, `(x + facing, y)` and `(x + 2*facing, y)`,
on the lemming's own row, each cell ORed with 1.

`or edx, 0x403` at 0x004275C1 is NOT this. Its function 0x00427554 has exactly
three E8 callers -- 0x004271D5, 0x00427509 and 0x0043B1F8 (inside 0x0043B150,
which walks the object list at 0x004C8988 gated on byte [obj+0x58]) -- and the
builder's chain 0x0041011C -> 0x0043B016 -> 0x0043AE6C reaches none of them.
The live diff below is the measurement that settles it.

The advance, 0x0040FBCC (frame 90)
----------------------------------
    0040fbf6  fld  [eax+0x10]        ; Y
    0040fc01  add  eax, [ecx+0x44]   ; + the vertical direction (+1 upright)
    0040fc1b  call 0x40df50          ; clamped
    0040fc29  call 0x423275          ; stored back
    0040ff2a  fild [ecx+0x40]        ; then, on the continuing path, X + facing
    0040ff54  cmp  [edx+0x3c], 4     ; a sound while the stored count is < 4
    0040ff7e  mov  eax, [edx+0x3c]
    0040ff8a  sub  edx, 1
    0040ff93  mov  [ecx+0x3c], edx   ; the decrement
    0040ff96  cmp  eax, 1            ; the old value 1 -> 0x0041AB30, whose
                                     ;   vtable store is 0x004B0C78 = shrug

The clock
---------
Round 5: there are no tick intervals. One cycle is `frames * 0.06 s`, where
0.06 is the double at 0x004B1940 and the frame count is the first dword of the
animation strip, read off the controller at lemming+0x128 (strip index at
+0x1C8, table at +0x1F8, records 0x98 bytes apart). `--strips` prints the whole
table so the build strip's frame count is a read, not a fit.

What it measured, 2026-10-15
----------------------------
See docs/scratch/phase3-round6-live.md for the series. Headline: two cells per
brick at percent 81, `or 1` onto the row the builder stands on, Y +1 and X
+facing at percent 90, 12 bricks, then shrug.

Usage
-----
    # restart the process first; there is no route back to the map
    python tools/harness/measure_build.py --enter MM_BUILD --probe
    python tools/harness/measure_build.py --skill 4 --seconds 22 --rows 0
    python tools/harness/measure_build.py --skill 4 --seconds 22 --repeat 2

`--probe` and `--strips` send no input. `--enter` is the only thing that
writes to the game, and the only write is goto_level's level-record swap,
which is put back. No lemming byte and no grid cell is ever written: the skill
goes on through the panel, by assign_skill.py's primitives.
"""
from __future__ import annotations

import argparse
import struct
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from lrstate import attach, i32, u32, ui_state  # noqa: E402
from memdump import k32, read_mem  # noqa: E402

import assign_skill as A  # noqa: E402
import measure_dig as D  # noqa: E402

GRID_WIDTH = D.GRID_WIDTH
VT_BUILD = 0x004B08DC
VT_SHRUG = 0x004B0C78

# Teach round 5's sampler about the builder rather than editing its file:
# `measure_dig.sample` prints the record list and the animation block for any
# vtable in this map, and names the record functions from FN_NAMES.
D.SKILL_VT[VT_BUILD] = "build"
D.FN_NAMES[0x0041011C] = "brick-stamp"
D.FN_NAMES[0x0040FBCC] = "build-advance"

ST_COUNT = 0x3C          # the brick counter, 0x0040F948 / 0x0040FF93
ST_LEN = 0x14            # state+0x14 = 4 (0x0040F952); not a length, recorded
SEC_PER_FRAME = D.SEC_PER_FRAME


def cycle_s(a) -> float:
    """One whole animation cycle, in seconds: `frames * 0.06 / divisor`.

    0x00468B04 is `frames * 0.06` (`call 0x45b8f0`, `fmul [0x4b1940]`) divided
    by the strip's own divisor at `[strip+0x10]` (`fdiv [eax+0x10]` at
    0x00468B56). Round 5 noted that the divisor CANCELS in the percent, which
    it does, and measured three skills whose divisor is 1.0 -- so this is the
    first time it mattered. MEASURED here on the shrug: strip 10 is 8 frames
    with divisor 0.5, and the shrug ran 0.966 s, which is 8 * 0.06 / 0.5 and
    not 8 * 0.06. The divisor belongs in the duration.
    """
    if not a or not a["frames"]:
        return 0.0
    return a["frames"] * SEC_PER_FRAME / (a["div"] or 1.0)


def state_block(h, lem: int):
    """vt, cursor, percent and the brick counter, in one read of 0x40 bytes."""
    sp = u32(h, lem + D.OFF_STATE) or 0
    if not sp:
        return 0, 0, None, None, None
    d = read_mem(h, sp, 0x40)
    if not d or len(d) != 0x40:
        return sp, 0, None, None, None
    vt = struct.unpack_from("<I", d, 0)[0]
    cursor = struct.unpack_from("<i", d, D.ST_CURSOR)[0]
    pct = struct.unpack_from("<i", d, D.ST_PERCENT)[0]
    count = struct.unpack_from("<i", d, ST_COUNT)[0]
    return sp, vt, cursor, pct, count


def strips(h, lem: int, n: int = 20):
    """Every animation strip's frame count, off the live controller table.

    `0x00468D00` resolves a strip as `[[controller+0x1F8]+0x0C] + index*0x98`,
    with the frame count at `[strip+0x00]` and the strip's own divisor at
    `[strip+0x10]`. Printing the whole table makes the build strip's frame
    count -- and so its cycle -- a read rather than a fit to the series.
    """
    a = D.anim_block(h, lem)
    if not a or not a["data"]:
        return None, []
    out = []
    for i in range(n):
        sd = read_mem(h, a["data"] + i * 0x98, 0x18)
        if not sd or len(sd) != 0x18:
            break
        frames = struct.unpack_from("<i", sd, 0)[0]
        div = struct.unpack_from("<d", sd, 0x10)[0]
        if not (0 < frames < 1000):
            break
        out.append((i, frames, div))
    return a, out


def predict_stair(h, core, bricks: int = 12):
    """The cells the static read says `bricks` bricks will write, in order.

    The stamp's own column is `trunc(X + [+0x360])` (0x0042F22E then
    0x0049C5AC), and the enter hook has already snapped X: facing left it adds
    1 to the truncation (`cmp [edx+0x40], -1` at 0x0040FB01, `add ecx, eax` at
    0x0040FB0B) and the bias `[+0x360]` is -1.0, so the two cancel and the
    stamp column is `trunc(X at the click)` in BOTH facings -- the same
    cancellation round 5 found in the bash bite. Brick k is then
    `(p + facing, row)` and `(p + 2*facing, row)`; frame 90 raises the row by
    `[+0x44]` and moves p by `[+0x40]`.

    Live-grid coordinates, Y-up, exactly as read_grid prints them.
    """
    x = D.trunc(core["x"])
    y = D.trunc(core["y"] + core["ybias"])
    dx, dy = core["dir"], core["vdir"]
    out = []
    for _k in range(bricks):
        out.append(((x + dx) & 0xFF, y))
        out.append(((x + 2 * dx) & 0xFF, y))
        y += dy
        x += dx
    return out


def probe_cell(h, core):
    """The cell `0x0040FBCC` tests before it advances, and its word.

    The advance raises Y first (`fld [eax+0x10]`, `add eax, [ecx+0x44]`,
    0x0040FC1B, 0x0040FC29) and then probes `(p + facing, the new row)`. A
    non-zero word there sends it to the turn 0x0042F66B and out of the build
    state, with the brick counter NOT decremented.
    """
    rows, base = D.grid_shape(h)
    p = D.trunc(core["x"])
    r = D.trunc(core["y"] + core["ybias"]) + core["vdir"]
    if not (0 <= r < rows):
        return (p + core["dir"]) & 0xFF, r, None
    w = read_mem(h, base + (r * GRID_WIDTH + ((p + core["dir"]) & 0xFF)) * 4, 4)
    word = struct.unpack("<I", w)[0] if w and len(w) == 4 else None
    return (p + core["dir"]) & 0xFF, r, word


def clear_prefix(h, core, bricks: int = 12) -> int:
    """How many bricks from the first one have BOTH their cells empty now.

    The stamp is an `or`, so a brick laid into terrain that is already solid
    writes nothing a grid diff can see. This is the filter that matters when
    the point is to watch cells change: `clear_ahead` counts clear cells
    anywhere in the staircase, which a run can satisfy while its first brick
    is buried in a wall.
    """
    rows, base = D.grid_shape(h)
    cells = predict_stair(h, core, bricks)
    for k in range(bricks):
        ok = True
        for (c, r) in cells[2 * k:2 * k + 2]:
            if not (0 <= r < rows):
                ok = False
                break
            w = read_mem(h, base + (r * GRID_WIDTH + c) * 4, 4)
            if not (w and len(w) == 4 and struct.unpack("<I", w)[0] == 0):
                ok = False
                break
        if not ok:
            return k
    return bricks


def clear_ahead(h, core, bricks: int = 12) -> int:
    """How many of the predicted staircase cells are empty right now.

    Used only to pick a builder with room to work: a staircase that runs into
    a wall or a ceiling stops early, and this round wants one that reaches
    the twelfth brick.
    """
    rows, base = D.grid_shape(h)
    n = 0
    for (c, r) in predict_stair(h, core, bricks):
        if not (0 <= r < rows):
            continue
        w = read_mem(h, base + (r * GRID_WIDTH + c) * 4, 4)
        if w and len(w) == 4 and struct.unpack("<I", w)[0] == 0:
            n += 1
    return n


def sample(h, lem: int, seconds: float, r0: int, nrows: int):
    """Per-sample state / position / brick-count / grid-window series.

    Five `ReadProcessMemory` calls an iteration: the state pointer, the state
    block, the lemming's own 0x40 bytes from +8, and the grid window. Nothing
    is written. A row is printed whenever the state, the animation percent,
    the brick count, the integer position, or any cell in the window changes,
    so a stamp inside one 200 ms tick still gets its own timestamp, percent
    and count.
    """
    rows, base = D.grid_shape(h)
    r0 = max(0, min(rows - 1, r0))
    nrows = max(1, min(rows - r0, nrows))
    prev = D.read_rows(h, base, r0, nrows)
    print(f"  watching rows {r0}..{r0 + nrows - 1} of {rows}"
          f" ({len(prev)} bytes)", flush=True)
    series, diffs = [], []
    t0 = time.perf_counter()
    n = 0
    last = None
    shown = False
    shrug_shown = False
    while (t := time.perf_counter() - t0) < seconds:
        n += 1
        sp, vt, cursor, pct, count = state_block(h, lem)
        core = D.lem_core(h, lem)
        g = D.read_rows(h, base, r0, nrows)
        changed = []
        if g and prev and g != prev:
            for i in range(0, len(g), 4):
                if g[i:i + 4] != prev[i:i + 4]:
                    old = struct.unpack_from("<I", prev, i)[0]
                    new = struct.unpack_from("<I", g, i)[0]
                    cell = i // 4
                    changed.append((r0 + cell // GRID_WIDTH,
                                    cell % GRID_WIDTH, old, new))
            prev = g
        if vt == VT_SHRUG and not shrug_shown:
            # Which strip the shrug plays, so its length is a read off the
            # same table rather than a fit to the one gap measured.
            a = D.anim_block(h, lem)
            if a:
                print(f"    SHRUG at t {t:.3f}s: strip {a['strip']}"
                      f" frames {a['frames']} divisor {a['div']}"
                      f" rate {a['rate']}"
                      f"  => frames * 0.06 / divisor ="
                      f" {cycle_s(a):.4f} s",
                      flush=True)
            shrug_shown = True
        if vt == VT_BUILD and not shown:
            D.print_records(h, sp, f"at t {t:.3f}s")
            a = D.anim_block(h, lem)
            if a:
                cyc = cycle_s(a)
                print(f"    animation strip {a['strip']} frames {a['frames']}"
                      f" divisor {a['div']} rate {a['rate']}"
                      f"  => frames * 0.06 / divisor = {cyc:.4f} s"
                      f" = {cyc / 0.2:.3f} ticks"
                      f"  (frame {a['frame']} frac {a['frac']:.4f})",
                      flush=True)
            print(f"    state+0x3c (bricks) {count}  state+0x14 {i32(h, sp + ST_LEN)}",
                  flush=True)
            shown = True
        if core is None:
            continue
        key = (vt, cursor, pct, count, D.trunc(core["y"]), D.trunc(core["x"]))
        if changed or key != last:
            series.append(dict(t=t, state=sp, vt=vt, cursor=cursor, pct=pct,
                               count=count, x=core["x"], y=core["y"],
                               dir=core["dir"], vdir=core["vdir"],
                               changed=changed))
            diffs.extend((t, pct, count) + c for c in changed)
            name = D.SKILL_VT.get(vt, A.STATE_NAMES.get(vt, f"{vt:#010x}"))
            cells = "".join(
                f"\n          cell (col {c[1]:3d}, row {c[0]:3d})"
                f"  {c[2]:#010x} -> {c[3]:#010x}"
                f"   d(col)={((c[1] - D.trunc(core['x']) + 128) % 256) - 128:+d}"
                f" d(row)={c[0] - D.trunc(core['y']):+d}"
                for c in changed)
            print(f"  {t:6.3f}s {name:<6} pct {pct if pct is not None else '?':>4}"
                  f" cur {cursor if cursor is not None else '?':>2}"
                  f" bricks {count if count is not None else '?':>3}"
                  f"  x {core['x']:8.3f} y {core['y']:7.3f}"
                  f" dir {core['dir']:+d} vdir {core['vdir']:+d}{cells}",
                  flush=True)
            last = key
    print(f"  {n} samples in {seconds:.1f}s ({n / seconds:.0f} Hz),"
          f" {len(diffs)} cell changes", flush=True)
    return series, diffs


def walkers(h):
    """On-screen walkers, with the predicted staircase's clearance."""
    out = []
    for p in A.lemmings(h):
        core = D.lem_core(h, p)
        if core is None:
            continue
        box = A.lemming_box(h, p)
        on_screen = bool(box and 0 <= (box[0] + box[2]) / 2 <= 640
                         and 0 <= (box[1] + box[3]) / 2 <= 480)
        _vt, name = A.state_of(h, p)
        out.append(dict(ptr=p, core=core, state=name, on_screen=on_screen,
                        clear=clear_ahead(h, core),
                        prefix=clear_prefix(h, core),
                        probe=probe_cell(h, core)))
    # Most room first, and within a tie the lemming with the most floor
    # behind it -- a walker that has just turned at a wall has the whole pen
    # ahead of it, and twelve bricks need eleven columns of it.
    out.sort(key=lambda c: (-c["prefix"], -c["clear"],
                           c["core"]["x"] * c["core"]["dir"]))
    return out


def print_walkers(h, cands) -> None:
    for c in cands:
        st = predict_stair(h, c["core"], 2)
        print(f"  {c['ptr']:#010x} x {c['core']['x']:8.3f} y {c['core']['y']:7.3f}"
              f" dir {c['core']['dir']:+d} vdir {c['core']['vdir']:+d}"
              f" bias ({c['core']['xbias']:+.1f},{c['core']['ybias']:+.1f})"
              f"  {c['state']:<6}"
              f" {'on-screen' if c['on_screen'] else 'off-screen'}"
              f"  first brick {st[0]} {st[1]}"
              f"  staircase clear {c['clear']}/24"
              f"  first {c['prefix']} brick(s) land on air"
              f"  step-into cell ({c['probe'][0]},{c['probe'][1]})"
              f" {'?' if c['probe'][2] is None else hex(c['probe'][2])}",
              flush=True)


def sweep_pick(h, inp, scr, cands, facing: int, min_clear: int,
               min_bricks: int, blocked: bool = False,
               max_bricks: int = 0, step: float = 3.0):
    """Walk the cursor across the lemmings' band and take the first hover
    that passes the filters.

    Aiming at one chosen lemming does not work on MM_BUILD: twenty lemmings
    share one twenty-column pen at row 23, their stored screen rectangles
    (`lemming+0x128`) overlap, and `[screen+0x16D8]` names whichever of them
    the engine's own AABB pass settles on -- in 24 aimed attempts this
    session it named a left-facing neighbour every time while six clean
    right-facing candidates were on screen. So this sweeps the cursor
    instead and lets the engine choose, filtering on what it chose. Nothing
    is written: the only thing that moves is the cursor, through the shim.
    """
    boxes = [A.lemming_box(h, c["ptr"]) for c in cands]
    boxes = [b for b in boxes if b]
    if not boxes:
        return 0
    x0 = min(b[0] for b in boxes) - 8
    x1 = max(b[2] for b in boxes) + 8
    y = sum((b[1] + b[3]) / 2 for b in boxes) / len(boxes)
    x = x0
    while x <= x1:
        A.aim_at(h, inp, x, y, tol=2.0, tries=6)
        hp = A.hovered(h, scr)
        if hp:
            hc = D.lem_core(h, hp)
            if (hc is not None
                    and (not facing or hc["dir"] == facing)
                    and A.state_of(h, hp)[1] == "walk"
                    and clear_ahead(h, hc) >= min_clear
                    and clear_prefix(h, hc) >= min_bricks
                    and (not blocked or bool(probe_cell(h, hc)[2]))
                    and (max_bricks <= 0
                         or clear_prefix(h, hc) <= max_bricks)):
                print(f"  sweep found {hp:#010x} at cursor ({x:.0f},{y:.0f}):"
                      f" dir {hc['dir']:+d} x {hc['x']:.3f} y {hc['y']:.3f}",
                      flush=True)
                return hp
        x += step
    return 0


def one_run(h, inp, skill: int, seconds: float, nrows: int,
            min_clear: int = 0, facing: int = 0,
            min_bricks: int = 0, blocked: bool = False,
            max_bricks: int = 0) -> dict:
    scr = A.screen(h)
    if not A.select_skill(h, inp, skill):
        return dict(ok=False, reason=f"panel {skill} not selected")

    def pick():
        return [c for c in walkers(h)
                if c["on_screen"] and c["state"] == "walk"
                and c["clear"] >= min_clear
                and c["prefix"] >= min_bricks
                and (max_bricks <= 0 or c["prefix"] <= max_bricks)
                and (not blocked or bool(c["probe"][2]))
                and (not facing or c["core"]["dir"] == facing)]

    # The engine's own direction filter, reached by input rather than by a
    # write: holding RIGHT sets `[screen+0x16B1]` and holding LEFT sets
    # `[screen+0x16B2]` (the arrow-key filters `0x00454D6C`'s hover pass
    # tests against `[lemming+0x40]`), so while the key is down only
    # lemmings facing that way are pickable at all. MEASURED this session:
    # RIGHT down -> 0x16B1 = 1, LEFT down -> 0x16B2 = 1, both back to 0 on
    # release. On MM_BUILD, where twenty lemmings share one pen and aiming
    # at a chosen one never works, this is what makes the mirrored facing
    # reachable. Released before the sample so nothing holds a key while the
    # series is recorded.
    from lrinput import DIK
    dir_key = DIK["RIGHT"] if facing > 0 else DIK["LEFT"] if facing < 0 else 0
    if dir_key:
        inp.key(dir_key, True)
        inp.wait_frames(2)

    t_wait = time.perf_counter()
    cands = pick()
    while not cands and time.perf_counter() - t_wait < 30.0:
        time.sleep(0.2)
        cands = pick()
    if not cands:
        if dir_key:
            inp.key(dir_key, False)
        return dict(ok=False, reason="no on-screen walker with room")
    print(f"  {len(cands)} candidate(s):", flush=True)
    print_walkers(h, cands[:6])

    # Aim at a candidate, then take whichever lemming the engine actually
    # names at `[screen+0x16D8]` -- as long as it is a walker facing the way
    # this run wants. With twenty lemmings packed into the same twenty
    # columns the engine very often names a neighbour, and round 5 lost four
    # runs to insisting on the one it had picked. The footprint is read off
    # the lemming's own live position afterwards, so any walker will do.
    p = 0
    for _pass in range(6):
      if p:
          break
      cands = pick() or cands
      for c in cands[:8]:
          b = A.lemming_box(h, c["ptr"])
          if not b:
              continue
          A.aim_at(h, inp, (b[0] + b[2]) / 2, (b[1] + b[3]) / 2)
          b = A.lemming_box(h, c["ptr"])
          if b:
              A.aim_at(h, inp, (b[0] + b[2]) / 2, (b[1] + b[3]) / 2,
                       tol=1.0, tries=6)
          hp = A.hovered(h, scr)
          if not hp:
              continue
          hc = D.lem_core(h, hp)
          if hc is None:
              continue
          why = ""
          if facing and hc["dir"] != facing:
              why = f"dir {hc['dir']:+d}"
          elif A.state_of(h, hp)[1] != "walk":
              why = A.state_of(h, hp)[1]
          elif clear_ahead(h, hc) < min_clear:
              why = f"clear {clear_ahead(h, hc)}/24"
          elif clear_prefix(h, hc) < min_bricks:
              why = f"only {clear_prefix(h, hc)} brick(s) on air"
          elif blocked and not probe_cell(h, hc)[2]:
              why = "step-into cell is empty, not a refusal case"
          elif max_bricks > 0 and clear_prefix(h, hc) > max_bricks:
              why = (f"{clear_prefix(h, hc)} brick(s) on air, wanted"
                     f" at most {max_bricks}")
          if why:
              print(f"    hovered {hp:#010x} rejected: {why}", flush=True)
              continue
          # The clearance re-check above is on the lemming the engine
          # actually named, at the moment of the click. A walker turns and
          # walks a column a tick, so the scan can be several columns stale
          # -- and a staircase that starts inside solid terrain writes
          # nothing a grid diff can see, because the stamp is an OR
          # (0x004101F8). Run 2 of this session was exactly that.
          p = hp
          if hp != c["ptr"]:
              print(f"  aimed at {c['ptr']:#010x}, the engine names"
                    f" {hp:#010x} -- taking that one", flush=True)
          break
      if not p:
          p = sweep_pick(h, inp, scr, cands, facing, min_clear,
                         min_bricks, blocked, max_bricks)
      if not p:
          time.sleep(0.4)
    if not p:
        if dir_key:
            inp.key(dir_key, False)
        return dict(ok=False, reason="none of the candidates became hoverable")

    before = D.lem_core(h, p)
    stair = predict_stair(h, before)
    cb = A.counts(h)
    rows_pre, pre = D.full_grid(h)
    print(f"  clicking {p:#010x}: predicted brick 1 cells"
          f" {stair[0]} {stair[1]}, brick 12 {stair[22]} {stair[23]},"
          f" stock[{skill}] {cb[skill]}", flush=True)
    nrows_live = rows_pre if nrows <= 0 else nrows
    r0 = 0 if nrows <= 0 else D.trunc(before["y"]) - 2
    inp.button(0, True)
    inp.wait_frames(4)
    inp.button(0, False)
    if dir_key:
        inp.key(dir_key, False)
        inp.wait_frames(1)
    series, diffs = sample(h, p, seconds, r0, nrows_live)
    window = (r0, r0 + nrows_live)
    ca = A.counts(h)
    _rows_post, post = D.full_grid(h)
    spent = bool(cb and ca and ca[skill] == cb[skill] - 1)

    outside = []
    if pre and post and len(pre) == len(post):
        for i in range(0, len(pre), 4):
            if pre[i:i + 4] != post[i:i + 4]:
                cell = i // 4
                r, cc = cell // GRID_WIDTH, cell % GRID_WIDTH
                if not (window[0] <= r < window[1]):
                    outside.append((r, cc,
                                    struct.unpack_from("<I", pre, i)[0],
                                    struct.unpack_from("<I", post, i)[0]))
    print(f"  stock[{skill}] {cb[skill] if cb else '?'} ->"
          f" {ca[skill] if ca else '?'}  spent={spent}", flush=True)
    if outside:
        print(f"  {len(outside)} whole-grid changes OUTSIDE the window:"
              f" {outside[:12]}", flush=True)
    else:
        print("  whole-grid diff: no change outside the window", flush=True)
    time.sleep(0.6)
    return dict(ok=spent, reason="", lemming=p, series=series, diffs=diffs,
                outside=outside, before=before, stair=stair)


def summarise(r: dict) -> None:
    """Bricks, their cells, their percent, and the gaps between them."""
    series = r.get("series") or []
    diffs = r.get("diffs") or []
    before = r["before"]
    print(f"  lemming {r['lemming']:#010x}"
          f" start ({before['x']:.3f},{before['y']:.3f})"
          f" dir {before['dir']:+d} vdir {before['vdir']:+d}"
          f"  {len(diffs)} cells written, stock spent={r['ok']}", flush=True)

    # Group the writes into bricks by the brick counter they were seen with.
    bricks: list[list] = []
    for d in diffs:
        t, pct, count, row, col, old, new = d
        if bricks and bricks[-1][0] == count and t - bricks[-1][1] < 0.3:
            bricks[-1][3].append((col, row, old, new, pct))
        else:
            bricks.append([count, t, pct, [(col, row, old, new, pct)]])
    print(f"  {len(bricks)} brick(s):", flush=True)
    prev_t = None
    for k, (count, t, pct, cells) in enumerate(bricks):
        gap = "" if prev_t is None else f"  gap {t - prev_t:.3f}s"
        prev_t = t
        cs = " ".join(f"({c},{rr}) {o:#x}->{nn:#x} @{p}"
                      for c, rr, o, nn, p in cells)
        print(f"    brick {k + 1:2d}  t {t:6.3f}s  count-at-write {count:>3}"
              f"  {len(cells)} cell(s)  {cs}{gap}", flush=True)

    # The brick counter and the Y, as the state's own fields reported them.
    print("  counter and position, every change:", flush=True)
    seen = None
    for s in series:
        key = (s["vt"], s["count"], D.trunc(s["y"]), D.trunc(s["x"]))
        if key == seen:
            continue
        seen = key
        name = D.SKILL_VT.get(s["vt"], A.STATE_NAMES.get(s["vt"],
                                                         f"{s['vt']:#010x}"))
        print(f"    {s['t']:6.3f}s {name:<6} pct {s['pct']:>4}"
              f" bricks {s['count']:>3} x {s['x']:8.3f} y {s['y']:7.3f}",
              flush=True)


def live(h, skill: int | None, repeat: int, seconds: float, nrows: int,
         min_clear: int, facing: int = 0, min_bricks: int = 0,
         blocked: bool = False, max_bricks: int = 0) -> int:
    from lrinput import LRInput

    if ui_state(h) != 0:
        print(f"ui_state {ui_state(h)}, not in a level")
        return 2
    n = A.wait_for_lemmings(h, 25.0)
    print(f"{n} lemmings in the actor list; stock {A.counts(h)}"
          f"  selected {A.selected(h)}", flush=True)
    if skill is None:
        st = A.counts(h)
        pick = [k for k in range(9) if st[k] > 0]
        if len(pick) != 1:
            print(f"cannot tell which skill this level ships: {st}")
            return 2
        skill = pick[0]
        print(f"the level's only stocked skill is index {skill}"
              f" ({A.PANEL_HINT.get(skill, 'unidentified')})", flush=True)
    rows, base = D.grid_shape(h)
    print(f"grid {GRID_WIDTH} x {rows} at {base:#010x}", flush=True)
    cands = walkers(h)
    if cands:
        a, tbl = strips(h, cands[0]["ptr"])
        if tbl:
            print("animation strips, live (frames, divisor):", flush=True)
            print("   " + "  ".join(f"[{i}] {f}f/{d:g}" for i, f, d in tbl),
                  flush=True)
    print("live lemmings, with the staircase the static read predicts:",
          flush=True)
    print_walkers(h, cands)

    results = []
    with LRInput() as inp:
        inp.clear()
        inp.enable()
        try:
            for k in range(repeat):
                print(f"\n-- run {k}: skill {skill}", flush=True)
                r = one_run(h, inp, skill, seconds, nrows, min_clear,
                            facing, min_bricks, blocked, max_bricks)
                if not r["ok"]:
                    print(f"   -> {r['reason']}", flush=True)
                results.append(r)
        finally:
            inp.release()

    print("\n== summary", flush=True)
    for k, r in enumerate(results):
        if not r.get("series"):
            print(f"  run {k}: {r['reason']}")
            continue
        print(f"  run {k}:", flush=True)
        summarise(r)
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--enter", metavar="LEVEL", help="enter this level first")
    ap.add_argument("--probe", action="store_true",
                    help="read and print, send no input")
    ap.add_argument("--strips", action="store_true",
                    help="print the live animation strip table and stop")
    ap.add_argument("--skill", type=int, nargs="?", const=-1, default=None,
                    metavar="INDEX",
                    help="panel index to assign; bare --skill picks the"
                         " level's only stocked one. The builder is 4.")
    ap.add_argument("--repeat", type=int, default=1)
    ap.add_argument("--seconds", type=float, default=22.0,
                    help="how long to sample after the click; twelve bricks"
                         " need the whole cycle twelve times over")
    ap.add_argument("--rows", type=int, default=0,
                    help="grid rows to watch; 0 watches the whole grid and"
                         " proves nothing moved outside it")
    ap.add_argument("--max-bricks", type=int, default=0,
                    help="only take a lemming at most this many of whose"
                         " bricks land on air -- the staircase then runs"
                         " into something, which is the refusal case and is"
                         " predicted before the click")
    ap.add_argument("--blocked", action="store_true",
                    help="aim at a lemming whose step-into cell is already"
                         " solid, which the read says turns it round at"
                         " percent 90 without spending a brick")
    ap.add_argument("--min-bricks", type=int, default=0,
                    help="only take a lemming whose first N bricks both land"
                         " on empty cells, so the grid diff can see them")
    ap.add_argument("--facing", type=int, default=0, choices=(-1, 0, 1),
                    help="only take a lemming facing this way, so the"
                         " mirrored footprint is a measurement and not a"
                         " reflection of the left-facing one")
    ap.add_argument("--min-clear", type=int, default=0,
                    help="only take a lemming this many of whose 24 predicted"
                         " staircase cells are empty now")
    args = ap.parse_args()

    h, pid = attach(write=bool(args.enter))
    if not h:
        print("the game is not running")
        return 1
    print(f"pid {pid}")
    try:
        if args.enter:
            from measure_float_splat import enter
            rc = enter(h, args.enter)
            if rc:
                return rc
        if args.probe or args.strips or args.skill is None:
            rows, base = D.grid_shape(h)
            print(f"ui_state {ui_state(h)}  grid {GRID_WIDTH} x {rows}"
                  f" at {base and hex(base)}  stock {A.counts(h)}"
                  f"  selected {A.selected(h)}")
            A.wait_for_lemmings(h, 25.0)
            cands = walkers(h)
            if cands:
                a, tbl = strips(h, cands[0]["ptr"])
                if a:
                    print(f"controller table {a['table']:#010x}"
                          f" data {a['data'] and hex(a['data'])}"
                          f"  this lemming: strip {a['strip']} frames"
                          f" {a['frames']} divisor {a['div']} rate {a['rate']}")
                for i, f, d in tbl:
                    c = f * SEC_PER_FRAME / (d or 1.0)
                    print(f"  strip [{i:2d}] frames {f:3d} divisor {d:g}"
                          f"  cycle {c:.4f} s = {c / 0.2:.3f} ticks")
            print_walkers(h, cands)
            for p in A.lemmings(h):
                sp, vt, cur, pct, count = state_block(h, p)
                if vt == VT_BUILD:
                    D.print_records(h, sp, f"(lemming {p:#010x})")
            return 0
        skill = None if args.skill == -1 else args.skill
        return live(h, skill, args.repeat, args.seconds, args.rows,
                    args.min_clear, args.facing, args.min_bricks,
                    args.blocked, args.max_bricks)
    finally:
        k32.CloseHandle(h)


if __name__ == "__main__":
    raise SystemExit(main())
