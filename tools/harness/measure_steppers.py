"""Round 10: the five stepper differences, measured off the actor list.

The round's five items all live in or next to the walk probe `0x0041BFFC` and
the walk stepper `0x0041BDD4`, so one sampler serves all of them. Everything
here reads the engine's own doubles out of the actor list
(`measure_spawn.actor_ptrs`, `0x004CBB18`); nothing projects a billboard, and
nothing writes to a lemming.

What is sampled, per lemming, per pass
--------------------------------------
    lemming + 0x008  double  X, unrolled round the 256-column tube
    lemming + 0x010  double  Y, world units, UP
    lemming + 0x040  int     facing, +1 / -1        (0x0041BE7C `fimul`)
    lemming + 0x044  int     vertical direction     (0x0041C48E `add`)
    lemming + 0x048  byte    the climber attribute  (0x0041C5C1 reads it)
    lemming + 0x049  byte    the floater attribute
    lemming + 0x04b  byte    the float-animation flag (0x00416038 halves on it)
    lemming + 0x340  ptr     the current state; its first dword is the vtable
    lemming + 0x128  ...     the animation controller (measure_dig.anim_block)

States this round cares about, from `read_steppers.py --states`:

    0x004B0D28  walk   step 0x0041BDD4
    0x004B0960  climb  step 0x004137CA
    0x004B0AF8  flip   step 0x00417CE6   <- the hoist
    0x004B0A20  fall   step 0x004160BD

Modes
-----
`--strips`     the whole animation strip table off a live controller: frames
               and divisor per index. No input, no writes.
`--watch S`    sample every lemming for S seconds and report every turn,
               climb, hoist and fall, with the X and Y the transition
               happened at. This is the series for items 1, 3 and 4.
`--trench`     take cells OUT of the live grid in front of the original's own
               walkers -- the inverse of `measure_stepup.carve`'s pillars --
               and watch what the step-down arm does with a drop of exactly
               N cells. Restores every cell it wrote. Item 2.
`--climber`    put the climber on a chosen lemming through the panel
               (`assign_skill.assign`, so the proof is the stock count going
               down) and then watch it, for items 1, 3 and 4.
`--compare`    round 1's per-tick comparison of one live lemming against
               `webo::sim::World`, through the same printer
               `build/cmake/dev-debug/compare_tick.exe` -- but off the actor
               list, where `compare_tick.py` used a billboard that round 10
               showed is not a lemming at all. `walker` or `climber`.

Cap the sample rate with `--hz` whenever only one lemming is tracked. An
uncapped loop drove about 300 000 `ReadProcessMemory` a second and the game
process was gone at the end of both long runs before `--hz 2000` was used;
2 kHz is ten samples per engine frame and nothing died after that.

Harness traps that cost other rounds a run, repeated here because they are
easy to get wrong: `lrstate.attach` returns `(handle, pid)`; `goto_level.focus`
kills the process through dgVoodoo and is never called; and this file must run
on the default Python, not with UCRT64 `bin` on PATH, or `capstone` and the
rest of the harness's imports disappear.
"""
from __future__ import annotations

import argparse
import os
import struct
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from lrstate import attach, i32, u32, ui_state  # noqa: E402
from measure_spawn import (  # noqa: E402
    LEMMING_VT,
    OTHER_VT,
    actor_ptrs,
    loaded_level,
)
from memdump import k32, read_mem  # noqa: E402
import measure_dig as D  # noqa: E402

CLOCK = 0x004CBAAC
RELEASED = 0x004CBA78

OFF_X, OFF_Y = 0x08, 0x10
OFF_FACING, OFF_VDIR = 0x40, 0x44
OFF_CLIMB, OFF_FLOAT, OFF_FLOATANIM = 0x48, 0x49, 0x4B
OFF_STATE = 0x340
LEM_READ = 0x344            # one read covers the position block and +0x340

VT_WALK = 0x004B0D28
VT_CLIMB = 0x004B0960
VT_FLIP = 0x004B0AF8        # the hoist; constructor 0x00417C80
VT_FALL = 0x004B0A20
VT_SPLAT = 0x004B0668
VT_NAMES = {
    VT_WALK: "walk", VT_CLIMB: "climb", VT_FLIP: "flip", VT_FALL: "fall",
    VT_SPLAT: "splat", 0x004B0810: "fall-810", 0x004B0680: "fall-680",
    0x004B0C60: "fall-c60", 0x004B0B28: "fall-b28", 0x004B0C48: "fall-c48",
    0x004B07B8: "block", 0x004B08DC: "build", 0x004B0C78: "shrug",
    0x004B06AC: "bash", 0x004B09F0: "dig", 0x004B0B68: "mine",
    0x004B0A58: "flapper", 0x004B0550: "blast", 0x004B0A38: "explode",
    0x004B0AF8: "flip",
}

# The world object the cell probe reads, and its grid: 256-wide, row-major u32
# flag cells, the row count at WORLD+0x418 and the data pointer at
# WORLD+0x41C+0x0C. Taken off `0x0040DE60` by read_grid.py.
WORLD = 0x004C6A10
ROW_COUNT = WORLD + 0x418
GRID_DATA_PTR = WORLD + 0x41C + 0x0C


def write_mem(h, addr: int, data: bytes) -> bool:
    import ctypes

    n = ctypes.c_size_t(0)
    ok = k32.WriteProcessMemory(h, ctypes.c_void_p(addr), data, len(data),
                                ctypes.byref(n))
    return bool(ok) and n.value == len(data)


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


# --- sampling ---------------------------------------------------------------


class Sample:
    __slots__ = ("t", "x", "y", "facing", "vdir", "climb", "flt", "fltanim",
                 "state", "vt", "strip", "aframe")

    def __init__(self, t, buf, vt):
        self.t = t
        self.x = struct.unpack_from("<d", buf, OFF_X)[0]
        self.y = struct.unpack_from("<d", buf, OFF_Y)[0]
        self.facing = struct.unpack_from("<i", buf, OFF_FACING)[0]
        self.vdir = struct.unpack_from("<i", buf, OFF_VDIR)[0]
        self.climb = buf[OFF_CLIMB]
        self.flt = buf[OFF_FLOAT]
        self.fltanim = buf[OFF_FLOATANIM]
        self.state = struct.unpack_from("<I", buf, OFF_STATE)[0]
        self.vt = vt
        # The animation controller is at lemming+0x128 and its strip index at
        # controller+0x1C8 (measure_dig.AN_STRIP_IX), i.e. lemming+0x2F0 --
        # inside the one 0x344-byte read, so the playing strip costs nothing.
        self.strip = struct.unpack_from("<i", buf, 0x128 + 0x1C8)[0]
        self.aframe = struct.unpack_from("<i", buf, 0x128 + 0x1CC)[0]

    @property
    def name(self):
        return VT_NAMES.get(self.vt, f"{self.vt:#010x}")


def sample(h, ptr: int, t: float):
    buf = read_mem(h, ptr, LEM_READ)
    if not buf or len(buf) != LEM_READ:
        return None
    if struct.unpack_from("<I", buf, 0)[0] not in (LEMMING_VT, OTHER_VT):
        return None
    sp = struct.unpack_from("<I", buf, OFF_STATE)[0]
    vt = (u32(h, sp) or 0) if sp else 0
    return Sample(t, buf, vt)


def lemmings(h) -> list[int]:
    return [p for p in actor_ptrs(h) if u32(h, p) in (LEMMING_VT, OTHER_VT)]


def strips(h, lem: int, n: int = 24):
    """Every strip's frame count and divisor, off the live controller table.

    `0x00468D00` resolves a strip as `[[controller+0x1F8]+0x0C] + index*0x98`,
    frame count at `[strip+0]`, divisor at `[strip+0x10]`. Round 6's
    `measure_build.strips` is the same read; this copy also prints the strip's
    first few dwords so a name pointer, if there is one, is visible.
    """
    a = D.anim_block(h, lem)
    if not a or not a["data"]:
        return None, []
    out = []
    for i in range(n):
        sd = read_mem(h, a["data"] + i * 0x98, 0x98)
        if not sd or len(sd) != 0x98:
            break
        frames = struct.unpack_from("<i", sd, 0)[0]
        div = struct.unpack_from("<d", sd, 0x10)[0]
        if not (0 < frames < 1000):
            break
        out.append((i, frames, div, sd))
    return a, out


def strip_name(h, sd: bytes) -> str:
    """Any ASCII string a strip record points at, so index -> name is a read."""
    for off in range(0, 0x98 - 3, 4):
        p = struct.unpack_from("<I", sd, off)[0]
        if not 0x10000 < p < 0x7FFF0000:
            continue
        raw = read_mem(h, p, 32)
        if not raw:
            continue
        s = raw.split(b"\0", 1)[0]
        if 2 <= len(s) <= 24 and all(32 <= c < 127 for c in s):
            return f"+{off:#04x} -> {s.decode()!r}"
    return ""


def show_strips(h) -> int:
    lems = lemmings(h)
    if not lems:
        print("no lemming in the actor list")
        return 1
    a, table = strips(h, lems[0])
    if not table:
        print("no strip table")
        return 1
    print(f"controller table {a['table']:#010x} data {a['data']:#010x}"
          f"  current strip {a['strip']} frame {a['frame']} rate {a['rate']}")
    print("  ix  frames   div    frames*0.06/div   name-ish")
    for i, frames, div, sd in table:
        cyc = frames * D.SEC_PER_FRAME / (div or 1.0)
        print(f"  {i:>3}  {frames:>6}  {div:>5.3g}   {cyc:>8.3f} s"
              f"        {strip_name(h, sd)}")
    return 0


# --- the watcher ------------------------------------------------------------


def pick(h, want, seconds: float = 6.0, hz: float = 30.0):
    """Lemming pointers whose latest sample satisfies `want(s)`."""
    t0 = time.perf_counter()
    while time.perf_counter() - t0 < seconds:
        hits = []
        for p in lemmings(h):
            s = sample(h, p, time.perf_counter() - t0)
            if s and want(s):
                hits.append(p)
        if hits:
            return hits
        time.sleep(1.0 / hz)
    return []


def watch(h, seconds: float, ptrs=None, quiet: bool = False,
          hz: float = 0.0):
    """Sample lemmings and return {ptr: [Sample, ...]}, newest last.

    One pass is one `ReadProcessMemory` of 0x344 bytes per lemming plus one of
    four bytes for its state's vtable, so the rate scales with how many
    lemmings are tracked. `ptrs=None` tracks every lemming the actor list
    holds and re-enumerates once a second.
    """
    series: dict[int, list] = {}
    t0 = time.perf_counter()
    last_enum = -1.0
    live = list(ptrs) if ptrs else []
    passes = 0
    gap = 1.0 / hz if hz > 0 else 0.0
    while True:
        t = time.perf_counter() - t0
        if t >= seconds:
            break
        if ptrs is None and t - last_enum > 1.0:
            live = lemmings(h)
            last_enum = t
        for p in live:
            s = sample(h, p, t)
            if s is None:
                continue
            series.setdefault(p, []).append(s)
        passes += 1
        if gap:
            # Capping the rate matters: an uncapped single-lemming loop drove
            # 300k ReadProcessMemory a second and the process died twice at
            # the end of a 40-70 s run. 2 kHz is ten samples per engine frame.
            nxt = t0 + passes * gap
            while time.perf_counter() < nxt:
                pass
    if not quiet:
        n = sum(len(v) for v in series.values())
        print(f"  {passes} passes, {n} object samples over {seconds:.1f} s"
              f"  ({n / max(seconds, 1e-9):.0f} reads/s,"
              f" {len(series)} lemmings)", flush=True)
    return series


def transitions(series):
    """Every state / facing change in a series, as plain tuples."""
    out = []
    for ptr, ss in series.items():
        for a, b in zip(ss, ss[1:]):
            if a.vt != b.vt:
                out.append(("state", ptr, a, b))
            elif a.facing != b.facing:
                out.append(("turn", ptr, a, b))
    out.sort(key=lambda e: e[2].t)
    return out


def report_fall(series) -> None:
    """Item 5: the fall, the umbrella animation and the halving flag.

    `[+0x4b]` is the byte `0x00416038` branches on to divide the 20.0 at
    `0x004B0A18` by the 2.0 at `0x004B03F8`; `0x004162C2` sets it when the
    `umbopen` animation finishes. The strip index is the animation, so this
    prints the strip alongside it.
    """
    for ptr, ss in series.items():
        falls = [s for s in ss if s.vt == VT_FALL]
        if not falls:
            continue
        print(f"\n  {ptr:#010x}  fall, strip and the halving flag")
        y0 = falls[0].y
        last = None
        for s in falls:
            key = (s.strip, s.fltanim, s.flt)
            if key == last:
                continue
            last = key
            print(f"    t {s.t:>8.4f}  y {s.y:>8.3f}  fallen {y0 - s.y:>7.3f}"
                  f"  strip {s.strip:>2} frame {s.aframe:>2}"
                  f"  [+0x49] {s.flt}  [+0x4b] {s.fltanim}")
        # The speed either side of the flip, sampled over 0.2 s windows.
        flip = next((s for s in falls if s.fltanim), None)
        if flip:
            before = [s for s in falls if s.t < flip.t - 0.02]
            after = [s for s in falls if s.t > flip.t + 0.02]
            for label, grp in (("before", before), ("after", after)):
                if len(grp) > 2 and grp[-1].t > grp[0].t:
                    rate = (grp[0].y - grp[-1].y) / (grp[-1].t - grp[0].t)
                    print(f"    descent rate {label} the flag:"
                          f" {rate:.3f} world units / s"
                          f"  over {grp[-1].t - grp[0].t:.3f} s")


def report_raw(series, window: float = 0.05, limit: int = 6) -> None:
    """Every distinct sample in a window either side of each transition.

    X moves once per rendered frame, not once per 200 ms tick, so the engine
    writes it in steps of about 0.03 world units. Printing the raw neighbours
    of a turn is the only way to see which side of the integer the facing
    flipped on.
    """
    tr = transitions(series)
    print(f"\n  raw samples +/- {window * 1000:.0f} ms around the first"
          f" {min(limit, len(tr))} transitions:")
    for kind, ptr, a, b in tr[:limit]:
        ss = series[ptr]
        print(f"    --- {kind} at t={a.t:.4f}  {ptr:#010x} ---")
        last = None
        for s in ss:
            if not (a.t - window <= s.t <= b.t + window):
                continue
            key = (round(s.x, 4), round(s.y, 4), s.facing, s.vt, s.strip,
                   s.aframe)
            if key == last:
                continue
            last = key
            mark = " <<<" if a.t < s.t <= b.t else ""
            print(f"      {s.t:>8.4f}  x {s.x:>9.4f}  y {s.y:>8.4f}"
                  f"  f {s.facing:+d}  {s.name:<8}"
                  f"  strip {s.strip:>2} frame {s.aframe:>2}{mark}")


def report_walls(series, lo: int, hi: int) -> None:
    """Items 1 and 3: where turns and climbs happen, to three decimals."""
    tr = transitions(series)
    print(f"\n  turns and state changes ({len(tr)}):")
    for kind, ptr, a, b in tr:
        if kind == "turn":
            print(f"    {a.t:>7.3f}  {ptr:#010x}  TURN  facing"
                  f" {a.facing:+d} -> {b.facing:+d}   x {a.x:.3f} -> {b.x:.3f}"
                  f"   y {a.y:.3f}   state {a.name}")
        else:
            print(f"    {a.t:>7.3f}  {ptr:#010x}  {a.name} -> {b.name}"
                  f"   x {a.x:.3f} -> {b.x:.3f}   y {a.y:.3f} -> {b.y:.3f}"
                  f"   facing {b.facing:+d}  climb {b.climb}")

    # The X a walker was last seen at inside the band, per lemming and facing.
    print(f"\n  extreme walker X inside columns {lo}..{hi}:")
    for ptr, ss in series.items():
        walk = [s for s in ss if s.vt == VT_WALK and lo - 2 <= s.x <= hi + 2]
        if not walk:
            continue
        print(f"    {ptr:#010x}  min x {min(s.x for s in walk):.3f}"
              f"  max x {max(s.x for s in walk):.3f}"
              f"  y {walk[-1].y:.3f}  n {len(walk)}")


def report_climbs(h, series) -> None:
    """Items 1, 3 and 4: the whole climb -> flip -> walk sequence."""
    for ptr, ss in series.items():
        vts = [s.vt for s in ss]
        if VT_CLIMB not in vts and VT_FLIP not in vts:
            continue
        print(f"\n  {ptr:#010x}  climb sequence")
        runs = []
        for s in ss:
            if runs and runs[-1][0] == s.vt:
                runs[-1][2] = s
                runs[-1][3] += 1
            else:
                runs.append([s.vt, s, s, 1])
        for vt, first, last, n in runs:
            nm = VT_NAMES.get(vt, f"{vt:#010x}")
            print(f"    {nm:<8} {first.t:>7.3f} .. {last.t:>7.3f}"
                  f"  ({last.t - first.t:>6.3f} s, {n} samples)"
                  f"  x {first.x:.3f} -> {last.x:.3f}"
                  f"  y {first.y:.3f} -> {last.y:.3f}"
                  f"  strip {first.strip}->{last.strip}"
                  f" frame {first.aframe}->{last.aframe}")
        climbing = [s for s in ss if s.vt == VT_CLIMB]
        if climbing:
            print(f"    last climbing y {max(s.y for s in climbing):.3f}"
                  f"   x held {min(s.x for s in climbing):.3f}"
                  f" .. {max(s.x for s in climbing):.3f}")
        flips = [s for s in ss if s.vt == VT_FLIP]
        if flips:
            print(f"    flip held x {min(s.x for s in flips):.3f}"
                  f" .. {max(s.x for s in flips):.3f}"
                  f"   y {min(s.y for s in flips):.3f}"
                  f" .. {max(s.y for s in flips):.3f}")
            # The first sample after the flip is the placement.
            ix = ss.index(flips[-1])
            if ix + 1 < len(ss):
                nxt = ss[ix + 1]
                print(f"    placed  x {nxt.x:.3f}  y {nxt.y:.3f}"
                      f"  state {nxt.name}   (dy {nxt.y - flips[-1].y:+.3f},"
                      f" dx {nxt.x - flips[-1].x:+.3f})")


# --- item 2: trenches -------------------------------------------------------


def dig_trench(h, trenches, floor_row: int):
    """Clear single-column holes of N cells downward from `floor_row`.

    The inverse of `measure_stepup.carve`. A walker whose feet are at
    `floor_row + 1` meets a drop of exactly N when it steps onto the column.
    The step-down arm compares the WHOLE dword (`cmp dword [eax], 0` at
    0x0041C732 / 0x0041C774), so the cell is zeroed rather than masked -- the
    same whole-dword write the bites use (`mov dword ptr [edx], 0` at
    0x00415B84).

    Single-column features reset themselves: after a step down of 1 the next
    column is a step up of 1, and a fall of 2 or 3 is far inside
    splatDistance 30.
    """
    saved = []
    for col, n in trenches:
        for k in range(n):
            row = floor_row - k
            a = grid_addr(h, col, row)
            if a is None:
                continue
            saved.append((a, read_cell(h, col, row)))
            write_mem(h, a, b"\0\0\0\0")
    print(f"  trenches {trenches} down from row {floor_row}:"
          f" {len(saved)} cells zeroed", flush=True)
    return saved


def fill_trench(h, saved) -> None:
    for a, old in reversed(saved):
        write_mem(h, a, struct.pack("<I", old or 0))
    print(f"  restored {len(saved)} grid cells", flush=True)


def report_trenches(series, trenches, floor_row: int) -> None:
    cols = {c for c, _n in trenches}
    depth = dict(trenches)
    print(f"\n  what happened over each trench (floor row {floor_row}):")
    for ptr, ss in series.items():
        events = []
        for a, b in zip(ss, ss[1:]):
            col = int(b.x) if b.x >= 0 else int(b.x) - 1
            near = {c for c in cols if abs(col - c) <= 2}
            if not near:
                continue
            if a.vt != b.vt:
                events.append((b.t, f"{a.name}->{b.name}", a, b))
            elif abs(b.y - a.y) > 0.5:
                events.append((b.t, f"dy {b.y - a.y:+.3f}", a, b))
        for t, what, a, b in events:
            col = int(b.x)
            print(f"    {t:>7.3f}  {ptr:#010x}  col~{col}"
                  f" (trench {depth.get(col, depth.get(col + 1, depth.get(col - 1, '?')))})"
                  f"  {what:<14}  x {a.x:.3f} -> {b.x:.3f}"
                  f"  y {a.y:.3f} -> {b.y:.3f}  facing {b.facing:+d}")


# --- entry ------------------------------------------------------------------


def enter(h, name: str) -> int:
    """Enter `name` with the record swap, restoring slot [0][0] afterwards.

    Same shape as `measure_stepup.enter` / `measure_float_splat.enter`: never
    calls `goto_level.focus`, and uses `restart_level.ensure_playing` because
    that is the one that knows how to leave the results screen.
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
    print(f"ui_state {state}  loaded {gl_loaded(h)!r}  clock {i32(h, CLOCK)}")
    return 0 if state == 0 else 2


def assign_climber(h, lo: int, hi: int, seconds: float, facing: int = 0,
                   skill: int = 0, poke_float: bool = False,
                   hz: float = 0.0):
    """Panel-assign `skill` to a walker in the band, then watch that lemming.

    `facing` +1 / -1 restricts the choice to a lemming already walking that
    way, so a left-facing climb can be reached without poking anything;
    0 takes whichever is nearest the low end of the band.
    """
    from lrinput import LRInput
    import assign_skill as A

    def on_ledge(hh, ptrs):
        best = None
        for p in ptrs:
            s = sample(hh, p, 0.0)
            if not (s and s.vt == VT_WALK and lo <= s.x <= hi):
                continue
            if facing and s.facing != facing:
                continue
            key = s.x if facing >= 0 else -s.x
            if best is None or key < best[1]:
                best = (p, key)
        return best[0] if best else (ptrs[0] if ptrs else 0)

    with LRInput() as inp:
        inp.clear()
        inp.enable()
        try:
            res = A.assign(h, inp, skill, choose=on_ledge)
        finally:
            inp.release()
    print(f"  assign skill {skill}: ok={res['ok']} lemming={res.get('lemming', 0):#010x}"
          f" stock {res.get('count_before')} -> {res.get('count_after')}"
          f" +48 {res.get('climber_before')} -> {res.get('climber_after')}"
          f"  reason {res['reason']!r}", flush=True)
    ptr = res.get("lemming") or None
    if ptr and poke_float:
        # A PROBE, labelled as one: MM_CLIMB_002 stocks climber 10 and nothing
        # else (0x004C91D8 reads [10,0,0,0,0,0,0,0,0,1]), so there is no
        # parasol to click. Round 3b did the same write for the same reason.
        ok = write_mem(h, ptr + OFF_FLOAT, b"")
        print(f"  PROBE: wrote [{ptr:#010x}+0x49] = 1 -> {ok}", flush=True)
    return res, watch(h, seconds, ptrs=[ptr] if ptr else None, hz=hz)




# --- round 1's gate: one live lemming against World, off the actor list -----

GAME_DIR = r"C:\progs\Lemmings Revolution"
PRINTER = os.path.join(os.path.dirname(os.path.dirname(
    os.path.dirname(os.path.abspath(__file__)))),
    "build", "cmake", "dev-debug", "compare_tick.exe")
UCRT_BIN = r"C:\progs\msys2\ucrt64\bin"


def _cell(v: float) -> int:
    """The engine's own truncation, tolerant of a double that reads 63.99997.

    At a probe instant X and Y are exact integers (`0x0041BECE` snaps X,
    `0x0042EE74` stores an integer row), so a value within 0.02 of an integer
    IS that integer; anything else is truncated the way `0x0049C5AC` does.
    """
    import math

    r = round(v)
    return int(r) if abs(v - r) < 0.02 else int(math.floor(v))


def live_cell(s, height: int = 96, snap: bool = False):
    """(column, row) in OUR grid for a live sample.

    A world Y becomes a file row by truncation and `Terrain::fromCollisionGrid`
    flips the file rows, so our row is `height - 1 - trunc(Y)`. See the Phase 3
    handoff, "Units".

    `snap` is for a sample `tick_series` picked as a probe instant. At such an
    instant both coordinates ARE whole numbers in the engine (`0x0041BECE`
    snaps X, `0x0042EE74` stores an integer row), so the nearest integer is the
    engine's own value and any residual is our sampler being up to a frame or
    two late -- the engine writes X once per rendered frame, in steps of about
    0.03 world units. `compare` prints the raw doubles and the largest residual
    it used, so the snap is visible rather than hidden.
    """
    if snap:
        return int(round(s.x)), height - 1 - int(round(s.y))
    return _cell(s.x), height - 1 - _cell(s.y)


def printer(level: str, which: str, x: int, y: int, facing: str, ticks: int):
    import subprocess

    env = dict(os.environ)
    env["PATH"] = UCRT_BIN + os.pathsep + env.get("PATH", "")
    out = subprocess.run([PRINTER, GAME_DIR, level, which, str(x), str(y),
                          facing, str(ticks)],
                         capture_output=True, text=True, env=env)
    print("  printer:", out.stderr.strip(), flush=True)
    rows = []
    for line in out.stdout.splitlines():
        parts = line.split()
        if len(parts) == 4:
            rows.append((int(parts[1]), int(parts[2]), parts[3]))
    return rows


def tick_series(ss, ticks: int, want_vt: int):
    """The engine's own probe instants, which are its ticks.

    There is no tick in the walk stepper: X moves continuously at 5.0 world
    units a second and `0x0041BEE6` calls the probe when the slice reaches a
    column boundary, which `0x0041BF61` puts on an integer -- so a probe fires
    every 200 ms and X is an exact integer at each one. `0x0041BECE` then snaps
    it, and `0x0042EE74` stores an integer row, so the same is true of Y for
    the climb at the same 5.0 a second.

    Sampling on a fixed 0.2 s grid off our own wall clock drifts against that
    (measured: the grid landed on 49.975 and 53.965 where the engine's instants
    were 50.000 and 54.000), so this finds the instants instead: a local
    minimum of the distance from the moving coordinate to the nearest integer.
    """
    # The FIRST CONTIGUOUS run of the state, not every sample in it. A
    # climber on MM_CLIMB_002 tops out, hoists, walks on and climbs the next
    # wall, and taking all its climb samples stitched two different walls into
    # one series -- which reads as a divergence at the join and is not one.
    run = []
    seen = False
    for s in ss:
        if s.vt == want_vt:
            run.append(s)
            seen = True
        elif seen:
            break
    if len(run) < 3:
        return []
    key = (lambda s: s.x) if want_vt == VT_WALK else (lambda s: s.y)
    # One group per integer the moving coordinate is nearest to, in time order,
    # and the sample closest to that integer inside each group is the instant
    # the probe fired. A group spans the approach AND the retreat, so a
    # right-facing walker that turns at a wall gives ... 62, 63, 64, 63, 62.
    out = []
    cur = None
    best = None
    best_d = 1e9
    for s in run:
        v = key(s)
        r = round(v)
        if cur is None or r != cur:
            if best is not None:
                out.append(best)
                if len(out) > ticks:
                    return out
            cur, best, best_d = r, s, abs(v - r)
            continue
        d = abs(v - r)
        if d < best_d:
            best, best_d = s, d
    if best is not None and len(out) <= ticks:
        out.append(best)
    return out


def compare(h, which: str, ticks: int, lo: int, hi: int, seconds: float,
            facing: int = 0):
    """Round 1's comparison, with the actor list where it used a billboard."""
    level = loaded_level(h).replace("/", "\\").split("\\")[-1]
    if level.lower().endswith(".lvl"):
        level = level[:-4]
    rows = i32(h, ROW_COUNT) or 96

    if which == "walker":
        chosen = pick(h, lambda s: (s.vt == VT_WALK and lo <= s.x <= hi), 20.0)
        if not chosen:
            print(f"  no walker inside columns {lo}..{hi}")
            return 2
        series = watch(h, seconds, ptrs=chosen[:1], hz=2000)
        want = VT_WALK
    else:
        res, series = assign_climber(h, lo, hi, seconds, facing=facing,
                                     hz=2000)
        if not res["ok"]:
            print(f"  no climber: {res['reason']}")
            return 2
        want = VT_CLIMB

    ptr, ss = next(iter(series.items()))
    live = tick_series(ss, ticks, want)
    if len(live) < 2:
        print(f"  could not assemble a tick series ({len(live)} samples)")
        return 2
    first = live[0]
    face = "right" if first.facing == 1 else "left"
    col, row = live_cell(first, rows, snap=True)
    worst = max(max(abs(s.x - round(s.x)), abs(s.y - round(s.y)))
                for s in live)
    print(f"  {ptr:#010x}  {which} on {level}, start cell ({col}, {row})"
          f" facing {face}, live x {first.x:.3f} y {first.y:.3f};"
          f" largest probe-instant residual {worst:.4f} world units"
          f" ({len(live)} instants)", flush=True)
    ours = printer(level, which, col, row, face, ticks)

    print("\n  | tick | live cell | ours | ours state | live x | live y |")
    print("  | --- | --- | --- | --- | --- | --- |")
    diverge = None
    for n, s in enumerate(live):
        lc = live_cell(s, rows, snap=True)
        oc = (ours[n][0], ours[n][1]) if n < len(ours) else (None, None)
        mark = "   <-- DIVERGE" if lc != oc else ""
        print(f"  | {n} | {lc[0]},{lc[1]} | {oc[0]},{oc[1]} |"
              f" {ours[n][2] if n < len(ours) else '?'} |"
              f" {s.x:.3f} | {s.y:.3f} |{mark}")
        if lc != oc and diverge is None:
            diverge = n
    if diverge is None:
        print(f"\n  SUMMARY {which} MATCH {len(live) - 1} ticks on {level}")
    else:
        print(f"\n  SUMMARY {which} diverge at tick {diverge} on {level}")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--enter", metavar="LEVEL")
    ap.add_argument("--strips", action="store_true")
    ap.add_argument("--watch", type=float, metavar="SECONDS")
    ap.add_argument("--climber", type=float, metavar="SECONDS",
                    help="assign the climber through the panel, then watch")
    ap.add_argument("--band", default="160:171",
                    help="LO:HI, the walkable band to pick from / report on")
    ap.add_argument("--trench", metavar="COL:N,COL:N")
    ap.add_argument("--trench-row", type=int, metavar="ROW",
                    help="the top solid row to clear downward from")
    ap.add_argument("--cell", metavar="COL:ROW", action="append", default=[],
                    help="print one live cell and exit")
    ap.add_argument("--poke-float", action="store_true",
                    help="write [lemming+0x49] = 1 on the chosen lemming -- a"
                         " labelled PROBE, in round 3b's shape, because this"
                         " level stocks no parasol")
    ap.add_argument("--hz", type=float, default=0.0,
                    help="cap the sample rate (0 = flat out)")
    ap.add_argument("--facing", type=int, default=0, choices=(-1, 0, 1),
                    help="only assign to a lemming already walking this way")
    ap.add_argument("--skill", type=int, default=0,
                    help="panel index to assign (0 climber)")
    ap.add_argument("--carve-clear", metavar="COL:N,COL:N",
                    help="zero N cells downward from --trench-row, kept for"
                         " the whole watch and restored after")
    ap.add_argument("--compare", choices=("walker", "climber"),
                    help="round 1's per-tick comparison against World, off"
                         " the actor list instead of a billboard")
    ap.add_argument("--ticks", type=int, default=12)
    ap.add_argument("--seconds", type=float, default=20.0)
    ap.add_argument("--raw", type=int, default=0, metavar="N",
                    help="also print the raw samples around the first N"
                         " transitions")
    args = ap.parse_args()

    h, pid = attach(write=True)
    if h is None:
        print("no Lemmings Revolution process", file=sys.stderr)
        return 1
    lo, hi = (int(v) for v in args.band.split(":"))
    print(f"attached pid {pid}  ui {ui_state(h)}  loaded {loaded_level(h)!r}",
          flush=True)

    if args.enter:
        rc = enter(h, args.enter)
        if rc:
            return rc

    for spec in args.cell:
        c, r = (int(v) for v in spec.split(":"))
        print(f"  cell({c},{r}) = {read_cell(h, c, r)}")

    if args.strips:
        return show_strips(h)

    if args.compare:
        return compare(h, args.compare, args.ticks, lo, hi, args.seconds,
                       facing=args.facing)

    if args.climber is not None:
        saved = None
        if args.carve_clear:
            spec = [(int(p.split(":")[0]), int(p.split(":")[1]))
                    for p in args.carve_clear.split(",")]
            if args.trench_row is None:
                print("--carve-clear needs --trench-row", file=sys.stderr)
                return 2
            saved = dig_trench(h, spec, args.trench_row)
        try:
            res, series = assign_climber(h, lo, hi, args.climber,
                                         facing=args.facing, skill=args.skill,
                                         poke_float=args.poke_float,
                                         hz=args.hz)
        finally:
            if saved:
                fill_trench(h, saved)
        report_climbs(h, series)
        report_fall(series)
        report_walls(series, lo, hi)
        if args.raw:
            report_raw(series, limit=args.raw)
        return 0

    if args.trench:
        trenches = [(int(p.split(":")[0]), int(p.split(":")[1]))
                    for p in args.trench.split(",")]
        if args.trench_row is None:
            print("--trench needs --trench-row", file=sys.stderr)
            return 2
        saved = dig_trench(h, trenches, args.trench_row)
        try:
            series = watch(h, args.watch or 20.0)
        finally:
            fill_trench(h, saved)
        report_trenches(series, trenches, args.trench_row)
        report_walls(series, lo, hi)
        if args.raw:
            report_raw(series, limit=args.raw)
        return 0

    if args.watch is not None:
        series = watch(h, args.watch)
        report_walls(series, lo, hi)
        report_climbs(h, series)
        if args.raw:
            report_raw(series, limit=args.raw)
        return 0

    ap.print_help()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
