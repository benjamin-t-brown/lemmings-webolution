#!/usr/bin/env python3
"""Offline generator for the per-cell tube mesh (A4c).

Reimplements, in Python, the exact rule the shipped engine uses to turn a
level's stored 256x96 collision grid into the per-cell `mesh0` / `mesh4` quad
arrays at `0x4ECB10`. It reads **only stored level data**:

  * the collision grid (`findCollisionGrid`'s walk, byte-identical to the grid
    the running engine collides against);
  * the `u32[49858]` "surface index" array on the same `Cylinder` walk, which
    this round decoded as the engine's `(x, y, layer) -> pool index` table at
    `world+0x42C` (key `layer + (x + y*0x101)*2`, `FUN_0043C930`);
  * the `R x 12` record array at `world+0x43C` (the `FUN_0043A1C6` get-or-create
    list; empty on MM_CLIMB_002).

Nothing else. The retained A1 live arena is used **only** by `--compare` as a
comparison oracle; it is never an input to the rule.

Provenance of every term (READ = decompiled/static bytes, MEASURED = compared
against the retained arena, inferred = neither):

  * `FUN_004615E9`  (0x4615E9)  face/side list builder          READ
  * `FUN_00461E1D`  (0x461E1D)  the eight 0x10-byte part lists   READ
  * `FUN_00463410`  (0x463410)  mesh0 / mesh4 assembly           READ
  * `FUN_00463100`  (0x463100)  the whole-block wall quad        READ
  * `FUN_0043A31D`  (0x43A31D)  get-or-create vertex (key table) READ
  * `FUN_0043A1C6`  (0x43A1C6)  get-or-create vertex (record list) READ
  * `FUN_0043C930`  (0x43C930)  key = layer + (x + y*0x101)*2    READ
  * `FUN_004136E0` / `FUN_00413710` (0x4136E0 / 0x413710) UV from grid READ
  * `FUN_00439D60`  (0x439EC1)  the UV table is `T[i] = i/64` for i in 0..64 READ
  * quad colour pairs at `entry+0xD0..+0xEC` set by `FUN_004608BB` (0x4608BB) READ

Run with normal Windows Python (do NOT prepend UCRT64 bin).
"""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

STOCK = Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")
STOCK_SHA256 = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
IMAGE_BASE = 0x00400000

DEFAULT_LVL = ROOT / "build/tmp/a1-camera/assets/COMPILED/MM_CLIMB_002.LVL"
LVL_FALLBACKS = [
    ROOT / "build/lvl_dump/levels/COMPILED/MM_CLIMB_002.LVL",
    ROOT / "build/tmp/MM_CLIMB_002.LVL",
]
ARENA_DIR = ROOT / "build/tmp/a1-live-arena"

# The extracted retail level this round was verified against.
LVL_SHA256 = "95f84f86fb53ab29ce2ce3aed794cf69a06e5754d42ef78657f3af1a5db4c567"

# ---------------------------------------------------------------------------
# static pins: the rule's anchors, re-read from the shipped executable
# ---------------------------------------------------------------------------

PINS = [
    (0x004615E9, 8, "558bec83ec4456", "FUN_004615E9 entry (face/side list builder)"),
    (0x00461E1D, 7, "558bec83ec3453", "FUN_00461E1D entry (part-list builder)"),
    (0x00461618, 5, "e800080000", "FUN_004615E9 -> FUN_00461E1D"),
    (0x004616E0, 5, "e87bc7faff", "FUN_004615E9 -> FUN_0040DE60 (collision cell)"),
    (0x0046226E, 5, "e876f3ffff", "FUN_00462264 -> FUN_004615E9"),
    (0x0046227E, 5, "e88d110000", "FUN_00462264 -> FUN_00463410"),
    (0x00463497, 5, "e8ebedffff", "mesh0 allocation"),
    (0x00463F07, 5, "e87be3ffff", "mesh4 allocation"),
    (0x004622ED, 5, "b910cb4e00", "mesh pool object literal 0x4ECB10"),
    (0x00464FA8, 7, "8b8800000a00", "FUN_00464F9C reads [pool+0xA0000]"),
    (0x00464FC0, 4, "8d541110", "FUN_00464F9C bumps by count*0x80 + 0x10"),
    (0x0040DE6A, 6, "25ff000000", "collision accessor: x & 0xff"),
    (0x0040DE78, 7, "3b9118040000", "collision accessor compares y with world+0x418"),
    (0x0043A31D, 8, "558bec83ec2456", "FUN_0043A31D entry (key-table vertex)"),
    (0x0043A337, 5, "e8f4250000", "FUN_0043A31D -> FUN_0043C930 (the key)"),
    (0x0043A1C6, 8, "558bec83ec2456", "FUN_0043A1C6 entry (record-list vertex)"),
    (0x0043C93A, 6, "69c001010000", "FUN_0043C930: y * 0x101"),
    (0x0043C948, 3, "8d044a", "FUN_0043C930: lea eax,[edx + ecx*2] (the *2)"),
    (0x00439EB5, 6, "dc3540094b00", "FUN_00439D60: T[i] = i / 64.0 (fdiv [0x4B0940])"),
    (0x00439EBE, 7, "dd1ccde05d4c00", "FUN_00439D60: store to [i*8 + 0x4C5DE0]"),
    (0x0043A07C, 6, "81c12c040000", "level load: the key table goes to world+0x42C"),
    (0x0043A082, 5, "e8b9150000", "level load: call FUN_0043B640 (key table loader)"),
    (0x0043A0A0, 6, "81c13c040000", "level load: the door records go to world+0x43C"),
    (0x0043A0A6, 5, "e815190000", "level load: call FUN_0043B9C0 (record loader)"),
]

# instruction-level pins inside FUN_00463410 / FUN_004615E9 that carry the rule
RULE_PINS = [
    (0x00463FF5, 4, "0fbf7106", "mesh4 sort key 1: MOVSX ESI,[ECX+6] (x1)"),
    (0x0046404D, 4, "0fbf7202", "mesh4 sort key 2: MOVSX ESI,[EDX+2] (yStart)"),
    (0x0046400F, 2, "7f58", "mesh4 sort: JG swap when x1[b] > x1[a]"),
    (0x00464067, 2, "7d48", "mesh4 sort: JGE no-swap when yStart[b] >= yStart[a]"),
    (0x00463833, 4, "0fbf4804", "mesh0 right-side v/corner y uses record+4 (gap start)"),
    (0x00463846, 4, "0fbf4202", "mesh0 right-side v uses record+2 (gap end)"),
]


def file_offset(va: int) -> int:
    return va - IMAGE_BASE


def read_pins(exe: Path) -> tuple[list[str], bytes]:
    data = exe.read_bytes()
    problems: list[str] = []
    digest = hashlib.sha256(data).hexdigest()
    if digest != STOCK_SHA256:
        problems.append(f"exe sha256 {digest} != {STOCK_SHA256}")
    for va, n, want, label in PINS + RULE_PINS:
        got = data[file_offset(va):file_offset(va) + n].hex()
        if not got.startswith(want):
            problems.append(f"{va:#010x} {got!r} does not start with {want!r} ({label})")
    return problems, data


# ---------------------------------------------------------------------------
# stored level data
# ---------------------------------------------------------------------------

def parse_level(path: Path):
    """Walk the Cylinder chunk the way CollisionGrid.cpp does.

    Returns (height, key_table, record_list, collision_grid).
    """
    d = path.read_bytes()
    at = d.find(struct.pack("<I", 8) + b"Cylinder")
    if at < 0:
        raise ValueError("no Cylinder chunk")
    p = at + 4 + 8 + 20                       # 5 zero words, height, f64
    height = struct.unpack_from("<I", d, p)[0]
    p += 4 + 8
    n = struct.unpack_from("<I", d, p)[0]
    p += 4
    key_table = list(struct.unpack_from("<%dI" % n, d, p))
    p += 4 * n
    r = struct.unpack_from("<I", d, p)[0]
    p += 4
    records = [struct.unpack_from("<3i", d, p + 12 * i) for i in range(r)]
    p += 12 * r
    c = struct.unpack_from("<I", d, p)[0]
    p += 4
    if c != 256 * height:
        raise ValueError(f"grid count {c} != 256*{height}")
    grid = list(struct.unpack_from("<%dI" % c, d, p))
    return height, key_table, records, grid


# ---------------------------------------------------------------------------
# the rule
# ---------------------------------------------------------------------------

def u_of(x: int) -> float:
    """FUN_004136E0: U = T[2*(x & 31)] = (x & 31)/32."""
    return (x & 31) / 32.0


def v_of(y: int) -> float:
    """FUN_00413710: V = T[64 - 2*(y & 31)] = 1 - (y & 31)/32."""
    return 1.0 - (y & 31) / 32.0


def _dbl(lo: int, hi: int) -> float:
    return struct.unpack("<d", struct.pack("<II", lo, hi))[0]


# entry+0xD8/0xDC (top faces, both banks), +0xE8/0xEC (mesh0 right side),
# +0xE0/0xE4 (mesh4 left side), +0xD0/0xD4 (horizontal faces). FUN_004608BB.
C_TOP = _dbl(0, 0x406FE000)
C_RIGHT = _dbl(0, 0x4064B800)
C_LEFT = _dbl(0, 0x406B1800)
C_HORIZ = _dbl(0, 0x406FE000)

FIX = 1.0 / 512.0     # 0.001953125
FIX128 = 1.0 / 128.0  # 0.0078125


class Level:
    def __init__(self, height, key_table, records, grid):
        self.height = height
        self.grid = grid
        self.table = {k: v for k, v in enumerate(key_table) if v != 0xFFFFFFFF}
        self.records = {(x, y): i for (x, y, i) in records}
        self.keys_touched: set[int] = set()
        self.missing: list[tuple] = []
        self.mesh0_quads = 0
        self.mesh4_quads = 0

    # -- engine primitives --------------------------------------------------

    def cell(self, x, y):
        x &= 0xFF
        if y >= self.height:
            y = self.height - 1
        if y < 0:
            y = 0
        return self.grid[y * 256 + x]

    def solid_nodoor(self, x, y):
        c = self.cell(x, y)
        return (c & 1) != 0 and (c & 0x400) == 0

    def key(self, x, y, layer):
        return layer + (x + y * 0x101) * 2

    def vertex(self, x, y, layer):
        """FUN_0043A31D: table lookup; a miss means the rule is wrong."""
        k = self.key(x, y, layer)
        self.keys_touched.add(k)
        v = self.table.get(k)
        if v is None:
            self.missing.append(("a31d", x, y, layer, k))
            return -1
        return v

    def vertex_door(self, x, y):
        """FUN_0043A1C6: the world+0x43C record list."""
        v = self.records.get((x, y))
        if v is None:
            self.missing.append(("a1c6", x, y))
            return -1
        return v

    # -- FUN_00461E1D -------------------------------------------------------

    def part_lists(self, x0, y0):
        """The eight 0x10-byte part lists, then the merged 0x4ECA80 records."""
        lists = [[] for _ in range(8)]
        for r in range(8):
            c = 0
            while c < 0x10:
                while c < 0x10 and (self.cell(x0 + r, y0 + c) & 0x401) == 0:
                    c += 1
                if c < 0x10:
                    y_start = y0 + c
                    is_door = (self.cell(x0 + r, y0 + c) & 0x400) != 0
                    if not is_door:
                        while True:
                            c += 1
                            if c > 0xF:
                                break
                            cc = self.cell(x0 + r, y0 + c)
                            if (cc & 1) == 0 or (cc & 0x400) != 0:
                                break
                    else:
                        while True:
                            c += 1
                            if c > 0xF:
                                break
                            if (self.cell(x0 + r, y0 + c) & 0x400) == 0:
                                break
                    lists[r].append({"y_start": y_start, "y_end": y0 - 1 + c,
                                     "merged": 0, "door": is_door})
        recs = []
        for r in range(8):
            for i in range(len(lists[r])):
                rec = lists[r][i]
                r2 = r
                if rec["merged"] == 0:
                    if r + 1 < 8:
                        j = 0
                        while True:
                            r2 += 1
                            if r2 < 8:
                                j = 0
                                later = lists[r2]
                                while j < len(later) and not (
                                        later[j]["y_start"] == rec["y_start"]
                                        and later[j]["y_end"] == rec["y_end"]
                                        and later[j]["door"] == rec["door"]):
                                    j += 1
                                if j < len(later):
                                    later[j]["merged"] = 1
                            cnt = len(lists[r2]) if r2 < 8 else 0
                            if not (j < cnt and r2 < 8):
                                break
                        r2 -= 1
                    recs.append({"y_start": rec["y_start"], "y_end": rec["y_end"],
                                 "x0": x0 + r, "x1": x0 + r2, "door": rec["door"],
                                 "r4": 0, "r5": 0, "r6": 0, "r7": 0, "r8": 0, "r9": 0})
        return recs

    # -- FUN_004615E9 -------------------------------------------------------

    def annotate(self, recs):
        l68, l58, l48 = [], [], []
        for rec in recs:
            rec["r4"] = len(l68)
            if rec["x0"] >= 0:
                self._side(rec, l68, rec["x0"], (rec["x0"] - 1) & 0xFF)
            rec["r5"] = len(l68)
            rec["r6"] = len(l58)
            if rec["x1"] < 0x100:
                self._side(rec, l58, rec["x1"], (rec["x1"] + 1) & 0xFF)
            rec["r7"] = len(l58)
            rec["r8"] = len(l48)
            if rec["y_end"] < self.height - 1:
                self._horiz(rec, l48)
            rec["r9"] = len(l48)
        return l68, l58, l48

    def _side(self, rec, into, x_rec, x):
        """Gaps in the column just outside the record, over [y_start, y_end]."""
        if not rec["door"]:
            y = rec["y_start"]
            while True:
                while y <= rec["y_end"] and self.solid_nodoor(x, y):
                    y += 1
                if y <= rec["y_end"]:
                    gap0 = y
                    s = y
                    while True:
                        y = s
                        s = y + 1
                        if rec["y_end"] < s:
                            break
                        if not (not self.solid_nodoor(x, s) or (self.cell(x, s) & 0x400) != 0):
                            break
                    into.append({"x": x_rec, "gap0": gap0, "gap1": y, "side": 0})
                    y = s
                if not (y <= rec["y_end"]):
                    break
        else:
            y = rec["y_start"]
            while y <= rec["y_end"] and (self.cell(x, y) & 0x401) == 0x401:
                y += 1
            if y <= rec["y_end"]:
                into.append({"x": x_rec, "gap0": rec["y_start"],
                             "gap1": rec["y_end"], "side": 1})

    def _horiz(self, rec, into):
        y = rec["y_end"] + 1
        if not rec["door"]:
            x = rec["x0"]
            while True:
                while x <= rec["x1"] and self.solid_nodoor(x, y):
                    x += 1
                if x <= rec["x1"]:
                    gap0 = x
                    s = x
                    while True:
                        x = s
                        s = x + 1
                        if rec["x1"] < s:
                            break
                        if not (not self.solid_nodoor(s, y) or (self.cell(s, y) & 0x400) != 0):
                            break
                    into.append({"y": rec["y_end"], "gap0": gap0, "gap1": x, "side": 0})
                    x = s
                if not (x <= rec["x1"]):
                    break
        else:
            x = rec["x0"]
            while x <= rec["x1"] and (self.cell(x, y) & 0x401) == 0x401:
                x += 1
            if x <= rec["x1"]:
                into.append({"y": rec["y_end"], "gap0": rec["x0"], "gap1": rec["x1"], "side": 1})

    # -- FUN_00463410 -------------------------------------------------------

    def _top(self, rec):
        u0 = u_of(rec["x0"])
        u1 = u_of(rec["x1"] + 1) or 1.0
        v0 = v_of(rec["y_start"])
        v1 = v_of(rec["y_end"] + 1)
        if v1 == 1.0:
            v1 = 0.0
        if not rec["door"]:
            idx = [self.vertex(rec["x0"], rec["y_start"], 1),
                   self.vertex(rec["x0"], rec["y_end"] + 1, 1),
                   self.vertex(rec["x1"] + 1, rec["y_end"] + 1, 1),
                   self.vertex(rec["x1"] + 1, rec["y_start"], 1)]
        else:
            idx = [self.vertex_door(rec["x0"], rec["y_start"]),
                   self.vertex_door(rec["x0"], rec["y_end"] + 1),
                   self.vertex_door(rec["x1"] + 1, rec["y_end"] + 1),
                   self.vertex_door(rec["x1"] + 1, rec["y_start"])]
        return idx, [(u0, v0), (u0, v1), (u1, v1), (u1, v0)]

    def mesh0(self, recs, l58, l48):
        quads = []
        for rec in recs:
            idx, uv = self._top(rec)
            quads.append({"idx": idx, "uv": uv, "colour": C_TOP, "coord": rec["x0"]})
            for k in range(rec["r6"], rec["r7"]):
                g = l58[k]
                u = u_of(g["x"] + 1) or 1.0
                v0 = v_of(g["gap0"])
                v1 = v_of(g["gap1"] + 1)
                if v1 == 1.0:
                    v1 = 0.0
                if g["side"] == 0:
                    c0 = self.vertex(g["x"] + 1, g["gap0"], 1)
                    c1 = self.vertex(g["x"] + 1, g["gap1"] + 1, 1)
                else:
                    c0 = self.vertex_door(g["x"] + 1, g["gap0"])
                    c1 = self.vertex_door(g["x"] + 1, g["gap1"] + 1)
                c2 = self.vertex(g["x"] + 1, g["gap1"] + 1, 0)
                c3 = self.vertex(g["x"] + 1, g["gap0"], 0)
                # u0..u3 -= 1/512; v0 -= 1/512; v1 += 1/512; v2 += 1/512; v3 -= 1/512
                uv = [(u - FIX, v0 - FIX), (u - FIX, v1 + FIX),
                      (u - FIX, v1 + FIX), (u - FIX, v0 - FIX)]
                quads.append({"idx": [c0, c1, c2, c3], "uv": uv, "colour": C_RIGHT,
                              "coord": rec["x0"]})
            for k in range(rec["r8"], rec["r9"]):
                g = l48[k]
                u0 = u_of(g["gap0"])
                u1 = u_of(g["gap1"] + 1) or 1.0
                v = v_of(g["y"]) - FIX128
                if v == 1.0:
                    v = 0.0
                if g["side"] == 0:
                    c0 = self.vertex(g["gap0"], g["y"] + 1, 1)
                    c3 = self.vertex(g["gap1"] + 1, g["y"] + 1, 1)
                else:
                    c0 = self.vertex_door(g["gap0"], g["y"] + 1)
                    c3 = self.vertex_door(g["gap1"] + 1, g["y"] + 1)
                c1 = self.vertex(g["gap0"], g["y"] + 1, 0)
                c2 = self.vertex(g["gap1"] + 1, g["y"] + 1, 0)
                uv = [(u0 + FIX, v - FIX), (u0 + FIX, v - FIX),
                      (u1 - FIX, v - FIX), (u1 - FIX, v - FIX)]
                quads.append({"idx": [c0, c1, c2, c3], "uv": uv, "colour": C_HORIZ,
                              "coord": rec["x0"]})
        return quads

    def mesh4(self, recs, l68, l48):
        n = len(recs)
        order = [n - 1 - i for i in range(n)]
        for a in range(n - 1):
            b = a + 1
            while b < n:
                ra, rb = recs[order[a]], recs[order[b]]
                if rb["x1"] > ra["x1"] or (rb["x1"] == ra["x1"] and rb["y_start"] < ra["y_start"]):
                    order[a], order[b] = order[b], order[a]
                b += 1
        quads = []
        for oi in order:
            rec = recs[oi]
            idx, uv = self._top(rec)
            quads.append({"idx": idx, "uv": uv, "colour": C_TOP, "coord": rec["x0"]})
            for k in range(rec["r4"], rec["r5"]):
                g = l68[k]
                u = u_of(g["x"])
                v0 = v_of(g["gap0"])
                v1 = v_of(g["gap1"] + 1)
                if v1 == 1.0:
                    v1 = 0.0
                if g["side"] == 0:
                    c2 = self.vertex(g["x"], g["gap1"] + 1, 1)
                    c3 = self.vertex(g["x"], g["gap0"], 1)
                else:
                    c2 = self.vertex_door(g["x"], g["gap1"] + 1)
                    c3 = self.vertex_door(g["x"], g["gap0"])
                c0 = self.vertex(g["x"], g["gap0"], 0)
                c1 = self.vertex(g["x"], g["gap1"] + 1, 0)
                # u0..u3 += 1/512; v0 -= 1/512; v1 += 1/512; v2 += 1/512; v3 -= 1/512
                uv = [(u + FIX, v0 - FIX), (u + FIX, v1 + FIX),
                      (u + FIX, v1 + FIX), (u + FIX, v0 - FIX)]
                quads.append({"idx": [c0, c1, c2, c3], "uv": uv, "colour": C_LEFT,
                              "coord": rec["x0"]})
            for k in range(rec["r8"], rec["r9"]):
                g = l48[k]
                u0 = u_of(g["gap0"])
                u1 = u_of(g["gap1"] + 1) or 1.0
                v = v_of(g["y"]) - FIX128
                if v == 1.0:
                    v = 0.0
                if g["side"] == 0:
                    c0 = self.vertex(g["gap0"], g["y"] + 1, 1)
                    c3 = self.vertex(g["gap1"] + 1, g["y"] + 1, 1)
                else:
                    c0 = self.vertex_door(g["gap0"], g["y"] + 1)
                    c3 = self.vertex_door(g["gap1"] + 1, g["y"] + 1)
                c1 = self.vertex(g["gap0"], g["y"] + 1, 0)
                c2 = self.vertex(g["gap1"] + 1, g["y"] + 1, 0)
                uv = [(u0 + FIX, v - FIX), (u0 + FIX, v - FIX),
                      (u1 - FIX, v - FIX), (u1 - FIX, v - FIX)]
                quads.append({"idx": [c0, c1, c2, c3], "uv": uv, "colour": C_HORIZ,
                              "coord": rec["x0"]})
        return quads

    # -- FUN_00463100 -------------------------------------------------------

    def wall_flag(self, recs, x0, y0):
        """False when the whole 8x16 block is one solid rectangle."""
        if (len(recs) == 1 and recs[0]["y_end"] == y0 + 0xF
                and recs[0]["y_start"] == y0 and recs[0]["x0"] == x0
                and recs[0]["x1"] == x0 + 7):
            return False
        return True


# ---------------------------------------------------------------------------


def generate(height, key_table, records, grid, bands=6, buckets=32):
    lvl = Level(height, key_table, records, grid)
    cells = {}
    for bucket in range(buckets):
        x0 = bucket * 8
        for band in range(bands):
            y0 = band * 16
            recs = lvl.part_lists(x0, y0)
            l68, l58, l48 = lvl.annotate(recs)
            m0 = lvl.mesh0(recs, l58, l48)
            m4 = lvl.mesh4(recs, l68, l48)
            bands_total = height // 16
            material = (bucket >> 2) + ((bands_total - 1 - band) // 2) * 8
            wall = lvl.wall_flag(recs, x0, y0)
            if wall:
                # FUN_00463100 draws the whole-block quad at entry+8 from four
                # layer-0 vertices; they share the key table with the meshes.
                lvl.vertex(x0, y0, 0)
                lvl.vertex(x0, y0 + 16, 0)
                lvl.vertex(x0 + 8, y0 + 16, 0)
                lvl.vertex(x0 + 8, y0, 0)
            cells[(bucket, band)] = {
                "bucket": bucket, "band": band, "x0": x0, "y0": y0,
                "material": material,
                "wall": wall,
                "mesh0": m0, "mesh4": m4,
            }
    lvl.mesh0_quads = sum(len(c["mesh0"]) for c in cells.values())
    lvl.mesh4_quads = sum(len(c["mesh4"]) for c in cells.values())
    return lvl, cells


# ---------------------------------------------------------------------------
# comparison against the retained arena (oracle only)
# ---------------------------------------------------------------------------

def arena_quads(arena: bytes, route: dict):
    out = []
    if not route.get("count"):
        return out
    off = route["array_offset"]
    for q in range(route["count"]):
        raw = arena[off + 0x80 * q:off + 0x80 * (q + 1)]
        idx = tuple(struct.unpack_from("<I", raw, 0x20 + 0x18 * c)[0] for c in range(4))
        uv = tuple(struct.unpack_from("<2d", raw, 0x20 + 0x18 * c + 8) for c in range(4))
        col = struct.unpack_from("<3d", raw, 0)
        out.append({"idx": idx, "uv": uv, "colour": col[0]})
    return out


def compare(cells, arena_dir: Path) -> int:
    man = json.loads((arena_dir / "live-arena-manifest.json").read_text(encoding="utf-8-sig"))
    arena = (arena_dir / "arena.bin").read_bytes()
    bad = 0
    checked = Counter()
    matched = Counter()
    bad_cells = Counter()
    details = []
    for e in man["entries"]:
        key = (e["group"], e["entry"])
        cell = cells.get(key)
        if cell is None:
            details.append((key, "no generated cell"))
            bad += 1
            continue
        for bank in ("mesh0", "mesh4"):
            want = arena_quads(arena, e[bank])
            got = cell[bank]
            checked[bank] += len(want)
            if len(got) != len(want):
                bad += 1
                bad_cells[(bank, key)] += 1
                details.append((key, bank, "count", len(got), len(want)))
                continue
            for q, (g, w) in enumerate(zip(got, want)):
                gi = tuple(g["idx"])
                gu = tuple((round(a, 12), round(b, 12)) for a, b in g["uv"])
                wu = tuple((round(a, 12), round(b, 12)) for a, b in w["uv"])
                if gi != w["idx"] or gu != wu or round(g["colour"], 9) != round(w["colour"], 9):
                    bad += 1
                    bad_cells[(bank, key)] += 1
                    if len(details) < 12:
                        details.append((key, bank, q, "idx", gi, w["idx"], "uv", gu, wu))
                else:
                    matched[bank] += 1
    for bank in ("mesh0", "mesh4"):
        n_bad = sum(v for (b, _), v in bad_cells.items() if b == bank)
        n_cells_bad = sum(1 for (b, _) in bad_cells if b == bank)
        print(f"  {bank}: {matched[bank]}/{checked[bank]} quads match, "
              f"{n_bad} bad in {n_cells_bad} cells")
    if details:
        for d in details[:12]:
            print("  ", d)
    return bad


# ---------------------------------------------------------------------------

def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--lvl", type=Path, default=None)
    ap.add_argument("--exe", type=Path, default=STOCK)
    ap.add_argument("--compare", action="store_true",
                    help="compare against the retained A1 live arena (oracle only)")
    ap.add_argument("--arena-dir", type=Path, default=ARENA_DIR)
    ap.add_argument("--json", type=Path, default=None)
    ap.add_argument("--no-pins", action="store_true")
    args = ap.parse_args()

    problems: list[str] = []
    if not args.no_pins:
        problems, _ = read_pins(args.exe)
        print("-- static pins --")
        for p in problems:
            print("  BAD", p)
        if not problems:
            print(f"  ok  {len(PINS) + len(RULE_PINS)} pinned windows")

    lvl_path = args.lvl
    if lvl_path is None:
        for cand in [DEFAULT_LVL, *LVL_FALLBACKS]:
            if cand.exists():
                lvl_path = cand
                break
    if lvl_path is None or not lvl_path.exists():
        print("no level file; pass --lvl", file=sys.stderr)
        return 2

    digest = hashlib.sha256(lvl_path.read_bytes()).hexdigest()
    print(f"-- level --\n  {lvl_path}\n  sha256 {digest}")
    if digest != LVL_SHA256:
        print(f"  note: not the recorded MM_CLIMB_002 extract ({LVL_SHA256})")

    height, key_table, records, grid = parse_level(lvl_path)
    lvl, cells = generate(height, key_table, records, grid)
    populated = len(lvl.table)
    print("-- generated --")
    print(f"  height {height}  key table slots {len(key_table)}  populated {populated}")
    print(f"  mesh0 quads {lvl.mesh0_quads}  mesh4 quads {lvl.mesh4_quads}")
    print(f"  distinct keys touched {len(lvl.keys_touched)}")
    print(f"  unresolved lookups {len(lvl.missing)}")
    if lvl.missing:
        for m in lvl.missing[:5]:
            print("   ", m)
    keys_ok = lvl.keys_touched == set(lvl.table)
    print(f"  key support == stored table support: {keys_ok}")
    dist = Counter(len(c["mesh0"]) for c in cells.values())
    print(f"  mesh0 quads/cell histogram {dict(sorted(dist.items()))}")

    rc = 0
    if problems:
        rc = 1
    if not keys_ok or lvl.missing:
        rc = 1
    if args.compare:
        print("-- compare against retained arena (oracle) --")
        if compare(cells, args.arena_dir):
            rc = 1
        else:
            print("  ALL QUADS MATCH (indices, UVs, colours, order, both banks)")

    if args.json:
        out = {
            "level": str(lvl_path), "level_sha256": digest,
            "height": height, "key_slots": len(key_table), "key_populated": populated,
            "keys_touched": sorted(lvl.keys_touched), "missing": lvl.missing,
            "mesh0_quads": lvl.mesh0_quads, "mesh4_quads": lvl.mesh4_quads,
            "cells": [{"bucket": b, "band": n, "x0": c["x0"], "y0": c["y0"],
                       "material": c["material"], "wall": c["wall"],
                       "mesh0_count": len(c["mesh0"]), "mesh4_count": len(c["mesh4"]),
                       "mesh0": c["mesh0"], "mesh4": c["mesh4"]}
                      for (b, n), c in sorted(cells.items())],
        }
        args.json.write_text(json.dumps(out, indent=1), encoding="utf-8")
        print(f"wrote {args.json}")

    return rc


if __name__ == "__main__":
    sys.exit(main())
