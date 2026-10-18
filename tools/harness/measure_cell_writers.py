"""Round 31: who paints/clears cell bits 0x100 and 0x400?

Stock exe. Static + offline. Predictions in
docs/scratch/phase3-round31-cell-writers.md.

    python tools/harness/measure_cell_writers.py --audit

0x100: reader at 0x00415552 sets lem[+0x5f] (round 15). No runtime
or/and-imm cell writer in .text — authored-only in 3/102 grids.

0x400: painted with solid|indestructible as the word 0x403 by door/gate
vtable 0x004B0F00 (round 26 Rumble host). Mesh rebuild scan 0x004615E9
reads it. Does not change the round-15 0x100 reader story.
"""
from __future__ import annotations

import argparse
import hashlib
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "proto"))

from box import Box  # noqa: E402
from read_lvl_grid import find_grid  # noqa: E402
from read_steppers import EXPECTED_SHA256, locate_exe  # noqa: E402

BIT100 = 0x100
BIT400 = 0x400
WORD403 = 0x403
CLEAR403 = 0xFBFC  # and-mask clears exactly 0x403

PAINT_OR = 0x004275C1  # or edx, 0x403
ERASE_AND = 0x00427641  # and edx, 0xfbfc
PAINT_FN = 0x00427554
ERASE_FN = 0x004275D4
ANIM_STEP = 0x004270DC  # pose anim → paint/erase
INIT_FN = 0x0042727C  # init paint/erase from +0x58
OBJECT_STEP = 0x00423749  # vtable 0x4b0f00 +0x14
OBJECT_INIT = 0x0042421D  # vtable +0x1c
VT_DOOR = 0x004B0F00
VT_INSTALL = 0x00423505  # mov [reg], 0x4b0f00
WORLD_PASS = 0x0043B150  # walks 0x4c8968/8988, calls paint when +0x58
SCAN_FN = 0x004615E9
SCAN_READ_400 = 0x00461704
SCAN_READ_401 = 0x00461843
SCAN_WRAP = 0x00462264
CELL_PROBE = 0x0040DE60
CELL_STAMP = 0x004311D0
READER_100 = 0x00415552

GAME = Path(r"C:\progs\Lemmings Revolution")


def pe_text(data: bytes):
    e_lfanew = struct.unpack_from("<I", data, 0x3C)[0]
    num = struct.unpack_from("<H", data, e_lfanew + 6)[0]
    opt = struct.unpack_from("<H", data, e_lfanew + 20)[0]
    sec = e_lfanew + 24 + opt
    image_base = struct.unpack_from("<I", data, e_lfanew + 24 + 28)[0]
    for i in range(num):
        o = sec + i * 40
        name = data[o : o + 8].split(b"\x00")[0]
        vsize, va, rawsize, rawptr = struct.unpack_from("<IIII", data, o + 8)
        if name == b".text":
            return data[rawptr : rawptr + rawsize], image_base + va
    raise SystemExit(".text missing")


def find_bytes(text: bytes, text_va: int, pattern: bytes) -> list[int]:
    out = []
    idx = 0
    while True:
        i = text.find(pattern, idx)
        if i < 0:
            break
        out.append(text_va + i)
        idx = i + 1
    return out


def call_sites(text: bytes, text_va: int, target: int) -> list[int]:
    hits = []
    for i in range(len(text) - 5):
        if text[i] != 0xE8:
            continue
        rel = struct.unpack_from("<i", text, i + 1)[0]
        if text_va + i + 5 + rel == target:
            hits.append(text_va + i)
    return hits


def levels_from_box(game: Path) -> dict[str, bytes]:
    """COMPILED\\*.LVL. A later box overrides the same stem."""
    out: dict[str, bytes] = {}
    for box_name in ("lemmings.box", "patch.box"):
        bp = game / box_name
        if not bp.is_file():
            continue
        box = Box(str(bp))
        for i, name in enumerate(box.names):
            upper = name.upper().replace("/", "\\")
            if upper.startswith("COMPILED\\") and upper.endswith(".LVL"):
                out[Path(name).stem.upper()] = box.read(i)
    return out


def audit(exe_path: Path, game_dir: Path) -> int:
    raw = exe_path.read_bytes()
    digest = hashlib.sha256(raw).hexdigest()
    if digest != EXPECTED_SHA256:
        print(f"FAIL unexpected exe sha256 {digest}")
        return 1
    text, text_va = pe_text(raw)
    fail = 0

    # --- 0x100: no cell or/and-imm writer ---
    # or r/m32, imm32 with imm = 0x100: 81 /1 with ModRM selecting edx etc.
    or_edx_100 = find_bytes(text, text_va, bytes.fromhex("81 ca 00 01 00 00"))
    or_eax_100 = find_bytes(text, text_va, bytes.fromhex("0d 00 01 00 00"))
    # and-clear 0xfffffeff as imm32 little-endian ff fe ff ff
    clear_100 = find_bytes(text, text_va, bytes.fromhex("ff fe ff ff"))
    # push 0x100 that could feed CELL_STAMP — report count; stamp uses
    # [ebp+8], so a push immediately before call 0x4311d0 would matter.
    push_100 = find_bytes(text, text_va, bytes.fromhex("68 00 01 00 00"))
    stamp_calls = call_sites(text, text_va, CELL_STAMP)
    stamp_fed_100 = []
    for site in stamp_calls:
        off = site - text_va
        window = text[max(0, off - 24) : off]
        if bytes.fromhex("68 00 01 00 00") in window:
            stamp_fed_100.append(site)

    if or_edx_100 or or_eax_100:
        print(f"FAIL unexpected or-imm 0x100: edx={list(map(hex, or_edx_100))} "
              f"eax={list(map(hex, or_eax_100))}")
        fail = 1
    else:
        print("OK no or edx/eax, 0x100 in .text")

    # Clear pattern appears in many non-cell contexts; require it within
    # 40 bytes of a call to the cell probe — none expected.
    probe_calls = call_sites(text, text_va, CELL_PROBE)
    clear_near_probe = [
        (va, cs) for va in clear_100 for cs in probe_calls if abs(cs - va) < 40
    ]
    if clear_near_probe:
        print(f"FAIL and 0xfffffeff near cell probe: {clear_near_probe}")
        fail = 1
    else:
        print("OK no and-clear 0x100 within 40 bytes of call 0x40de60 "
              f"({len(clear_100)} bare ff fe ff ff elsewhere, rejected)")

    if stamp_fed_100:
        print(f"FAIL push 0x100 feeds CELL_STAMP at {list(map(hex, stamp_fed_100))}")
        fail = 1
    else:
        print(f"OK no push 0x100 within 24 bytes of call CELL_STAMP "
              f"({len(push_100)} push 0x100 total, none stamp-fed)")

    r100_off = READER_100 - text_va
    if text[r100_off : r100_off + 6] != bytes.fromhex("81 e2 00 01 00 00"):
        print(f"FAIL reader 0x100 bytes at {READER_100:#x}")
        fail = 1
    else:
        print(f"OK reader and edx, 0x100 still at {READER_100:#x} (round 15)")

    # --- 0x400 / 0x403 door paint ---
    paint_off = PAINT_OR - text_va
    if text[paint_off : paint_off + 6] != bytes.fromhex("81 ca 03 04 00 00"):
        print(f"FAIL paint or 0x403 at {PAINT_OR:#x}")
        fail = 1
    else:
        print(f"OK paint or edx, 0x403 at {PAINT_OR:#x}")

    erase_off = ERASE_AND - text_va
    if text[erase_off : erase_off + 6] != bytes.fromhex("81 e2 fc fb 00 00"):
        print(f"FAIL erase and 0xfbfc at {ERASE_AND:#x}")
        fail = 1
    else:
        print(f"OK erase and edx, 0xfbfc at {ERASE_AND:#x}")

    paint_callers = call_sites(text, text_va, PAINT_FN)
    erase_callers = call_sites(text, text_va, ERASE_FN)
    # Expected: anim step, init, and world pass (interior call sites).
    expected_paint_fns = {ANIM_STEP, INIT_FN, WORLD_PASS}
    paint_owner = set()
    for cs in paint_callers:
        for owner in (ANIM_STEP, INIT_FN, WORLD_PASS):
            # call site lies inside owner body windows (known extents)
            if owner == ANIM_STEP and 0x004270DC <= cs < 0x0042727C:
                paint_owner.add(owner)
            elif owner == INIT_FN and 0x0042727C <= cs < 0x00427554:
                paint_owner.add(owner)
            elif owner == WORLD_PASS and 0x0043B150 <= cs < 0x0043B2EE:
                paint_owner.add(owner)
    if paint_owner != expected_paint_fns:
        print(f"FAIL paint callers owners {list(map(hex, sorted(paint_owner)))} "
              f"sites {[hex(x) for x in paint_callers]}")
        fail = 1
    else:
        print(f"OK paint FN callers inside anim/init/world-pass "
              f"{[hex(x) for x in paint_callers]}")

    erase_owner = set()
    for cs in erase_callers:
        if 0x004270DC <= cs < 0x0042727C:
            erase_owner.add(ANIM_STEP)
        elif 0x0042727C <= cs < 0x00427554:
            erase_owner.add(INIT_FN)
        elif 0x0043B150 <= cs < 0x0043B2EE:
            erase_owner.add(WORLD_PASS)
    if erase_owner != expected_paint_fns:
        print(f"FAIL erase callers owners {list(map(hex, sorted(erase_owner)))} "
              f"sites {[hex(x) for x in erase_callers]}")
        fail = 1
    else:
        print(f"OK erase FN callers inside anim/init/world-pass "
              f"{[hex(x) for x in erase_callers]}")

    anim_from_obj = [
        cs for cs in call_sites(text, text_va, ANIM_STEP)
        if OBJECT_STEP <= cs < OBJECT_STEP + 0x110
    ]
    if not anim_from_obj:
        print(f"FAIL object step {OBJECT_STEP:#x} does not call anim {ANIM_STEP:#x}")
        fail = 1
    else:
        print(f"OK object step {OBJECT_STEP:#x} calls anim {ANIM_STEP:#x} "
              f"at {[hex(x) for x in anim_from_obj]}")

    vt_off = VT_DOOR - 0x400000
    slot14 = struct.unpack_from("<I", raw, vt_off + 0x14)[0]
    slot1c = struct.unpack_from("<I", raw, vt_off + 0x1C)[0]
    if slot14 != OBJECT_STEP or slot1c != OBJECT_INIT:
        print(f"FAIL vtable {VT_DOOR:#x} +0x14={slot14:#x} +0x1c={slot1c:#x}")
        fail = 1
    else:
        print(f"OK door/gate vtable {VT_DOOR:#x}: +0x14=step +0x1c=init")

    install = find_bytes(text, text_va, bytes.fromhex("00 0f 4b 00"))
    if VT_INSTALL not in install:
        print(f"FAIL missing mov-imm 0x4b0f00 at {VT_INSTALL:#x}; "
              f"hits={[hex(x) for x in install]}")
        fail = 1
    else:
        print(f"OK ctor stores vtable at {VT_INSTALL:#x} "
              f"(and {[hex(x) for x in install]})")

    for va, pat, label in (
            (SCAN_READ_400, bytes.fromhex("25 00 04 00 00"), "and eax, 0x400"),
            (SCAN_READ_401, bytes.fromhex("25 01 04 00 00"), "and eax, 0x401"),
    ):
        off = va - text_va
        if text[off : off + 5] != pat:
            print(f"FAIL scan {label} at {va:#x}")
            fail = 1
        else:
            print(f"OK scan {label} at {va:#x}")

    wrap_calls = [
        cs for cs in call_sites(text, text_va, SCAN_FN)
        if SCAN_WRAP <= cs < SCAN_WRAP + 0x20
    ]
    if not wrap_calls:
        print(f"FAIL scan wrap {SCAN_WRAP:#x} missing call to {SCAN_FN:#x}")
        fail = 1
    else:
        print(f"OK mesh rebuild wrap {SCAN_WRAP:#x} calls scan {SCAN_FN:#x} "
              f"at {[hex(x) for x in wrap_calls]}")

    # Offline grids
    lvls = levels_from_box(game_dir)
    b100, b400, w403 = set(), set(), set()
    n403_cells = 0
    for name, data in lvls.items():
        found = find_grid(data)
        if not found:
            continue
        _off, cells = found
        if any(c & BIT100 for c in cells):
            b100.add(name)
        if any(c & BIT400 for c in cells):
            b400.add(name)
        for c in cells:
            if c == WORD403:
                n403_cells += 1
                w403.add(name)
    print(f"levels {len(lvls)}")
    print(f"any cell & 0x100: {len(b100)} -> {sorted(b100)}")
    print(f"any cell & 0x400: {len(b400)}")
    print(f"exact word 0x403: {n403_cells} cells across {len(w403)} levels")
    if sorted(b100) != ["H08_MT_1", "H22_MT_1", "VH06_MT"]:
        # stems from box may vary case
        norm = sorted(x.upper() for x in b100)
        if norm != ["H08_MT_1", "H22_MT_1", "VH06_MT"]:
            print(f"FAIL unexpected 0x100 levels {sorted(b100)}")
            fail = 1
        else:
            print("OK 0x100 authored in the three known levels")
    else:
        print("OK 0x100 authored in the three known levels")

    print("NOTE: 0x400 is the door/gate solid stamp marker (with 0x1|0x2); "
          "mesh scan 0x4615e9 stops/continues on it. Round 15 0x100 story "
          "unchanged (flag5f, not death).")
    return fail


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--audit", action="store_true")
    ap.add_argument("--exe", type=Path, default=None)
    ap.add_argument("--game", type=Path, default=GAME)
    args = ap.parse_args()
    if not args.audit:
        ap.error("use --audit")
    exe = args.exe if args.exe else locate_exe(None)
    return audit(Path(exe), args.game)


if __name__ == "__main__":
    raise SystemExit(main())
