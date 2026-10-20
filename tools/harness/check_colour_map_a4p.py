"""A4p: static + read-only live check of the 0x4C6A10 position colour map."""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import struct
import subprocess
import sys
import time
import traceback
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT_DEFAULT = ROOT / "build/tmp/a4p-colour-map"
STOCK_EXE = Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")
STOCK_SHA256 = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
INSTALL = STOCK_EXE.parent
LEVEL = "MM_CLIMB_002"
WORLD = 0x004C6A10
MAP_PTR_VA = WORLD + 0x670  # 0x4C7080
PALETTE_VA = WORLD + 0x678  # 0x4C7088
MAP_BYTES = 12288
PALETTE_BYTES = 0x1800
ENTRY_BYTES = 0x18
LEMMING_VT = 0x004B10E8
OTHER_VT = 0x004B0F6C
SAMPLE_SEC = 10.0
SAMPLES_PER_WALKER = 10
TEXT_VA = 0x00401000
TEXT_FILE_OFF = 0x1000
FTOL_VA = 0x0049C5AC

sys.path.insert(0, str(ROOT / "tools" / "harness"))
from acquire_game_foreground import acquire  # noqa: E402
from goto_level import loaded_level  # noqa: E402
from lrinput import LRInput  # noqa: E402
from lrstate import attach, i32, ui_state, u32  # noqa: E402
from measure_spawn import actor_ptrs  # noqa: E402
from memdump import find_pids, k32, read_mem  # noqa: E402
from read_grid_siblings import game_dir, levels_from_box, walk_cylinder  # noqa: E402
from watch_actor_tint_a4i import f64, load_enter_fast, read_exact  # noqa: E402


def install_hashes() -> dict[str, str]:
    names = ("Lemmings Revolution.exe", "SavedGame.dat", "dgVoodoo.conf")
    return {n: hashlib.sha256((INSTALL / n).read_bytes()).hexdigest() for n in names}


def pe_file_bytes(addr: int, length: int) -> bytes:
    off = TEXT_FILE_OFF + (addr - TEXT_VA)
    return STOCK_EXE.read_bytes()[off : off + length]


def ftol(d: float) -> int:
    return int(math.trunc(d))


def map_index(x: float, y: float) -> int:
    a = (ftol(x) & 0xFF) // 2
    b = ftol(y)
    return ((b << 8) // 2) + a


def palette_triple(data: bytes, byte_index: int) -> tuple[float, float, float] | None:
    off = byte_index * ENTRY_BYTES
    if off + 24 > len(data):
        return None
    return struct.unpack_from("<ddd", data, off)


def offline_level_path() -> Path | None:
    p = ROOT / "build/lvl_dump/levels/COMPILED/MM_CLIMB_002.LVL"
    return p if p.is_file() else None


def load_lvl_bytes() -> bytes:
    p = offline_level_path()
    if p:
        return p.read_bytes()
    lvls = levels_from_box(game_dir())
    if LEVEL not in lvls:
        raise SystemExit(f"{LEVEL} not in game box")
    return lvls[LEVEL]


def cmd_static(out: Path) -> int:
    ftol_bytes = pe_file_bytes(FTOL_VA, 16).hex()
    producer = {
        "loader": "0x439FD3",
        "loader_caller_site": "0x443A0A",
        "loader_caller_fn": "0x4432C1",
        "world_global": hex(WORLD),
        "byte_map_loader": "0x43B700 at world+0x664",
        "palette_loader": "0x43B5B0 reads 0x1800 bytes into world+0x678",
        "lvl_source": "Cylinder chunk after collision grid (read_grid_siblings.walk_cylinder)",
        "map_ptr_field": hex(MAP_PTR_VA),
        "palette_base": hex(PALETTE_VA),
        "bss_note": "past 0x4C6000 file image; live only",
        "ftol_fn": hex(FTOL_VA),
        "ftol_file_bytes_16": ftol_bytes,
        "actor_x": "lemming+0x08 double world X (measure_steppers.py)",
        "actor_y": "lemming+0x10 double world Y up (measure_steppers.py)",
        "update_colour_site": "0x42E62F..0x42E661 in 0x42E2BF",
        "update_guard": "0x42E62B TEST after CALL 0x49C870 (_strcmp); JZ 0x42E666 skips copy",
        "reapply": "each lemming update tick when guard falls through (not once)",
    }
    info = walk_cylinder(load_lvl_bytes())
    payload = info["payload"]
    table = info["table"]
    spawn_x, spawn_y = 32, 42
    si = map_index(float(spawn_x), float(spawn_y))
    sb = payload[si]
    srgb = palette_triple(table, sb)
    predict = {
        "level": LEVEL,
        "spawn_words_x_y": [spawn_x, spawn_y],
        "map_index": si,
        "map_byte": sb,
        "predicted_rgb": srgb,
        "palette_record0_rgb": palette_triple(table, 0),
        "map_byte_max": max(payload) if payload else None,
        "palette_slots_used_max_byte": max(payload) if payload else None,
        "palette_capacity_entries": PALETTE_BYTES // ENTRY_BYTES,
    }
    (out / "static-producer.json").write_text(json.dumps(producer, indent=2) + "\n", encoding="utf-8")
    (out / "offline-predict.json").write_text(json.dumps(predict, indent=2) + "\n", encoding="utf-8")
    (out / "offline-map.bin").write_bytes(payload)
    (out / "offline-palette.bin").write_bytes(table)
    print(json.dumps({"predicted_rgb": srgb, "map_byte": sb}))
    return 0


def walk_block_actors(handle) -> list[dict]:
    rows = []
    for actor in actor_ptrs(handle):
        if not actor or u32(handle, actor) not in (LEMMING_VT, OTHER_VT):
            continue
        c = actor + 0x128
        action = i32(handle, c + 0x1C8)
        if action not in (1, 13):
            continue
        rows.append({"actor": actor, "controller": c, "action": action})
    return rows


def read_lookup(handle, x: float, y: float, pal_blob: bytes) -> dict:
    idx = map_index(x, y)
    map_ptr = u32(handle, MAP_PTR_VA)
    map_byte = None
    if map_ptr and idx >= 0:
        raw = read_mem(handle, map_ptr + idx, 1)
        if raw and len(raw) == 1:
            map_byte = raw[0]
    pred = None
    if map_byte is not None:
        pred = palette_triple(pal_blob, map_byte)
    return {"idx": idx, "map_ptr": map_ptr, "map_byte": map_byte, "palette_rgb": pred}


def cmd_prepare(out: Path) -> int:
    before = install_hashes()
    if before["Lemmings Revolution.exe"] != STOCK_SHA256:
        raise SystemExit("exe hash mismatch")
    (out / "before.json").write_text(json.dumps(before, indent=2) + "\n", encoding="utf-8")
    out.mkdir(parents=True, exist_ok=True)
    return cmd_static(out)


def cmd_start_menu(out: Path) -> int:
    if find_pids("Lemmings"):
        raise SystemExit("game already running")
    (out / "dgVoodoo.backup").write_bytes((INSTALL / "dgVoodoo.conf").read_bytes())
    windowed = ROOT / "build/tmp/a1-zoom-elevation/dgVoodoo.windowed.conf"
    (INSTALL / "dgVoodoo.conf").write_bytes(windowed.read_bytes())
    owned = subprocess.Popen([str(STOCK_EXE)], cwd=INSTALL)
    time.sleep(10)
    handle, pid = attach()
    if not handle or pid != owned.pid:
        raise SystemExit("attach failed")
    state = {"pid": owned.pid, "ui_state": ui_state(handle), "game_starts": 1}
    (out / "pid.json").write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
    k32.CloseHandle(handle)
    print(json.dumps(state), flush=True)
    return 0


def cmd_sample(out: Path) -> int:
    pid = json.loads((out / "pid.json").read_text())["pid"]
    handle, apid = attach()
    if not handle or apid != pid:
        raise SystemExit("attach mismatch")
    enter = load_enter_fast(pid)
    enter.WORK = out
    rc = enter.main()
    lvl = loaded_level(handle)
    acquire(pid)
    with LRInput() as inp:
        inp.enable()
        inp.tap("P", 2)
        if ui_state(handle) != 0:
            k32.CloseHandle(handle)
            raise SystemExit(f"expected playing ui 0, got {ui_state(handle)} enter_rc={rc}")
        inp.release()

    pal_live = read_mem(handle, PALETTE_VA, PALETTE_BYTES) or b""
    map_ptr = u32(handle, MAP_PTR_VA)
    map_live = b""
    if map_ptr:
        map_live = read_mem(handle, map_ptr, MAP_BYTES) or b""
    (out / "live-palette.bin").write_bytes(pal_live)
    (out / "live-map.bin").write_bytes(map_live)
    (out / "live-map-meta.json").write_text(
        json.dumps({"map_ptr": hex(map_ptr) if map_ptr else None, "map_ptr_va": hex(MAP_PTR_VA)}, indent=2)
        + "\n",
        encoding="utf-8",
    )

    walkers: dict[int, dict] = {}
    samples: list[dict] = []
    per_actor_count: dict[int, int] = {}
    t0 = time.time()
    interval = SAMPLE_SEC / SAMPLES_PER_WALKER
    while time.time() - t0 < SAMPLE_SEC:
        handle, apid = attach()
        if not handle or apid != pid:
            raise SystemExit("attach lost")
        for row in walk_block_actors(handle):
            walkers[row["actor"]] = row
        now = time.time() - t0
        for actor, meta in list(walkers.items()):
            n = per_actor_count.get(actor, 0)
            if n >= SAMPLES_PER_WALKER:
                continue
            if n > 0 and now < n * interval:
                continue
            buf = read_exact(handle, actor, 0x344)
            x = struct.unpack_from("<d", buf, 0x08)[0]
            y = struct.unpack_from("<d", buf, 0x10)[0]
            ctrl = meta["controller"]
            cr = f64(handle, ctrl + 0x40)
            cg = f64(handle, ctrl + 0x48)
            cb = f64(handle, ctrl + 0x50)
            lk = read_lookup(handle, x, y, pal_live)
            pr = lk["palette_rgb"]
            match = (
                pr is not None
                and cr is not None
                and cg is not None
                and cb is not None
                and abs(pr[0] - cr) < 0.01
                and abs(pr[1] - cg) < 0.01
                and abs(pr[2] - cb) < 0.01
            )
            samples.append(
                {
                    "t": round(now, 3),
                    "actor": hex(actor),
                    "controller": hex(ctrl),
                    "x": x,
                    "y": y,
                    "ctrl_rgb": [cr, cg, cb],
                    "lookup": lk,
                    "match": match,
                }
            )
            per_actor_count[actor] = n + 1
        k32.CloseHandle(handle)
        time.sleep(0.15)

    mismatches = [s for s in samples if not s["match"]]
    rgb_changed = {}
    for actor_hex in {s["actor"] for s in samples}:
        rgbs = [tuple(s["ctrl_rgb"]) for s in samples if s["actor"] == actor_hex]
        rgb_changed[actor_hex] = len(set(rgbs)) > 1

    report = {
        "enter_rc": rc,
        "level": lvl,
        "walker_count": len(walkers),
        "sample_count": len(samples),
        "mismatch_count": len(mismatches),
        "mismatches": mismatches[:20],
        "rgb_changed_per_actor": rgb_changed,
        "all_match": len(mismatches) == 0 and len(samples) > 0,
        "palette_bytes": len(pal_live),
        "map_bytes": len(map_live),
        "palette_capacity_entries": PALETTE_BYTES // ENTRY_BYTES,
    }
    (out / "samples.json").write_text(json.dumps({"samples": samples, "report": report}, indent=2) + "\n", encoding="utf-8")
    (out / "enter.json").write_text(json.dumps({"enter_rc": rc, "loaded_level": lvl}, indent=2) + "\n", encoding="utf-8")
    k32.CloseHandle(handle)
    print(json.dumps({"samples": len(samples), "all_match": report["all_match"], "mismatches": len(mismatches)}))
    return 0


def cmd_shutdown(out: Path) -> int:
    pid = json.loads((out / "pid.json").read_text())["pid"]
    for p, _ in find_pids("Lemmings"):
        if p == pid:
            subprocess.run(["taskkill", "/PID", str(pid), "/F"], capture_output=True)
    if (out / "dgVoodoo.backup").exists():
        (INSTALL / "dgVoodoo.conf").write_bytes((out / "dgVoodoo.backup").read_bytes())
    after = install_hashes()
    before = json.loads((out / "before.json").read_text())
    (out / "after.json").write_text(json.dumps(after, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"install_match": after == before, "lemmings_running": bool(find_pids("Lemmings"))}))
    return 0


def cmd_live(out: Path) -> int:
    cmd_prepare(out)
    cmd_start_menu(out)
    try:
        rc = cmd_sample(out)
    finally:
        cmd_shutdown(out)
    return rc


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--output", type=Path, default=OUT_DEFAULT)
    ap.add_argument(
        "command",
        choices=("prepare", "static", "start-menu", "sample", "shutdown", "live"),
    )
    args = ap.parse_args()
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=True)
    try:
        if args.command == "prepare":
            return cmd_prepare(out)
        if args.command == "static":
            return cmd_static(out)
        if args.command == "start-menu":
            return cmd_start_menu(out)
        if args.command == "sample":
            return cmd_sample(out)
        if args.command == "shutdown":
            return cmd_shutdown(out)
        if args.command == "live":
            return cmd_live(out)
    except Exception:
        traceback.print_exc()
        return 1
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
