"""A4m: backtrace + frame/call onLeave bisect for controller+0x40 (152 writer)."""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
import subprocess
import sys
import time
import traceback
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT_DEFAULT = ROOT / "build/tmp/a4m-tint-bisect"
STOCK_EXE = Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")
STOCK_SHA256 = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
INSTALL = STOCK_EXE.parent
BASE_HOOKS = (0x004409E0, 16)
# Anticipated from static read (verified after run via manifest)
STATIC_HOOKS = (
    (0x004680F0, 16),
    (0x00401B80, 16),
    (0x004017F0, 16),
)
ACTOR_LIST = 0x004CBB18
LEMMING_VT = 0x004B10E8
OTHER_VT = 0x004B0F6C
PLAY_SEC = 15.0
TEXT_VA = 0x00401000
TEXT_FILE_OFF = 0x1000

sys.path.insert(0, str(ROOT / "tools" / "harness"))
from acquire_game_foreground import acquire  # noqa: E402
from goto_level import loaded_level  # noqa: E402
from lrinput import LRInput  # noqa: E402
from lrstate import attach, i32, ui_state, u32  # noqa: E402
from memdump import find_pids, k32, read_mem  # noqa: E402
from measure_spawn import actor_ptrs  # noqa: E402
from watch_actor_tint_a4i import f64, load_enter_fast, read_exact  # noqa: E402


def install_hashes() -> dict[str, str]:
    names = ("Lemmings Revolution.exe", "SavedGame.dat", "dgVoodoo.conf")
    return {n: hashlib.sha256((INSTALL / n).read_bytes()).hexdigest() for n in names}


def pe_file_bytes(addr: int, length: int) -> bytes:
    off = TEXT_FILE_OFF + (addr - TEXT_VA)
    data = STOCK_EXE.read_bytes()
    return data[off : off + length]


def file_hook_bytes(addrs: list[tuple[int, int]]) -> dict[str, str]:
    out = {}
    for addr, length in addrs:
        out[hex(addr)] = pe_file_bytes(addr, length).hex()
    return out


def live_hook_bytes(handle, addr: int, length: int) -> bytes:
    data = read_mem(handle, addr, length)
    if not data or len(data) != length:
        raise RuntimeError(f"process READ failed {addr:#x}")
    return data


def walk_block_actors(handle) -> list[dict]:
    rows = []
    for actor in actor_ptrs(handle):
        if not actor or u32(handle, actor) not in (LEMMING_VT, OTHER_VT):
            continue
        c = actor + 0x128
        action = i32(handle, c + 0x1C8)
        if action not in (1, 13):
            continue
        rows.append(
            {
                "actor": hex(actor),
                "controller": hex(c),
                "action": action,
                "tint_r_0x40": f64(handle, c + 0x40),
                "draw_position": list(struct.unpack("<3d", read_exact(handle, actor + 0x108, 24))),
            }
        )
    return rows


def walker_positions(handle) -> list[dict]:
    return [
        {
            "actor": a["actor"],
            "controller": a["controller"],
            "draw_position": a["draw_position"],
            "tint": a["tint_r_0x40"],
        }
        for a in walk_block_actors(handle)
    ]


def walkers_advanced(before: list[dict], after: list[dict]) -> bool:
    if not before or not after:
        return len(after) > len(before)
    by_actor = {b["actor"]: b for b in before}
    for a in after:
        b = by_actor.get(a["actor"])
        if b and b.get("draw_position") != a.get("draw_position"):
            return True
    return len(after) > len(before)


def load_enter(owned_pid: int, work: Path):
    enter = load_enter_fast(owned_pid)
    enter.WORK = work
    return enter


def find_frida_messages(raw: dict) -> list:
    def walk(o):
        if isinstance(o, dict):
            if "messages" in o and isinstance(o["messages"], list) and o["messages"]:
                m0 = o["messages"][0]
                if isinstance(m0, dict) and "payload" in m0:
                    return o["messages"]
            for v in o.values():
                r = walk(v)
                if r:
                    return r
        return None

    return walk(raw) or []


IMAGE_LO = 0x00401000
IMAGE_HI = 0x004C6000


def in_image(addr: int) -> bool:
    return IMAGE_LO <= addr < IMAGE_HI


def manifest_from_events(events: list[dict]) -> list[str]:
    addrs = {hex(BASE_HOOKS[0])}
    for e in events:
        if e.get("k") == "HOOK_INSTALL":
            a = int(e["addr"], 16)
            if in_image(a):
                addrs.add(hex(a))
    return sorted(addrs)


def cmd_prepare(out: Path) -> int:
    before = install_hashes()
    if before["Lemmings Revolution.exe"] != STOCK_SHA256:
        raise SystemExit("exe hash mismatch")
    hooks = list({BASE_HOOKS, *STATIC_HOOKS})
    seen = set()
    uniq = []
    for item in [BASE_HOOKS, *STATIC_HOOKS]:
        if item[0] not in seen:
            seen.add(item[0])
            uniq.append(item)
    (out / "before.json").write_text(json.dumps(before, indent=2) + "\n", encoding="utf-8")
    (out / "hook-file-bytes.json").write_text(json.dumps(file_hook_bytes(uniq), indent=2) + "\n", encoding="utf-8")
    return 0


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


def cmd_run_level(out: Path) -> int:
    pid = json.loads((out / "pid.json").read_text())["pid"]
    handle, apid = attach()
    if not handle or apid != pid:
        raise SystemExit("attach mismatch")
    walkers_before = walker_positions(handle)
    enter = load_enter(pid, out)
    rc = enter.main()
    lvl = loaded_level(handle)
    acquire(pid)
    with LRInput() as inp:
        inp.enable()
        inp.tap("P", 2)
        assert ui_state(handle) == 0
        time.sleep(PLAY_SEC)
        acquire(pid)
        inp.tap("P", 2)
        inp.wait_frames(3)
        inp.release()
    walkers_after = walker_positions(handle)
    report = {
        "enter_rc": rc,
        "loaded_level": lvl,
        "walkers_before_play": walkers_before,
        "walkers_after_play": walkers_after,
        "walkers_advanced": walkers_advanced(walkers_before, walkers_after),
        "walk_block_actors": walk_block_actors(handle),
    }
    (out / "live-snapshot.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    k32.CloseHandle(handle)
    print(json.dumps(report, indent=2), flush=True)
    return 0 if rc == 0 else 1


def cmd_build_manifest(out: Path, events_path: Path) -> int:
    events = json.loads(events_path.read_text()).get("events", [])
    manifest = manifest_from_events(events)
    file_hex = json.loads((out / "hook-file-bytes.json").read_text())
    for addr in manifest:
        if addr not in file_hex:
            a = int(addr, 16)
            file_hex[addr] = pe_file_bytes(a, 16).hex()
    (out / "hook-manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    (out / "hook-file-bytes.json").write_text(json.dumps(file_hex, indent=2) + "\n", encoding="utf-8")
    return 0


def cmd_verify_hooks(out: Path) -> int:
    pid = json.loads((out / "pid.json").read_text())["pid"]
    handle, apid = attach()
    if not handle or apid != pid:
        raise SystemExit("attach failed for verify")
    file_hex = json.loads((out / "hook-file-bytes.json").read_text())
    manifest = json.loads((out / "hook-manifest.json").read_text())
    checks = {}
    all_match = True
    for addr in manifest:
        a = int(addr, 16)
        live = live_hook_bytes(handle, a, 16).hex()
        expected = file_hex[addr]
        match = live == expected
        all_match = all_match and match
        checks[addr] = {"live_hex": live, "file_hex": expected, "match": match}
    checks["all_match"] = all_match
    (out / "hook-process-bytes.json").write_text(json.dumps(checks, indent=2) + "\n", encoding="utf-8")
    k32.CloseHandle(handle)
    print(json.dumps({"all_match": all_match, "count": len(manifest)}))
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
    print(json.dumps({"install_match": after == before}))
    return 0


def cmd_analyze(out: Path, events_path: Path) -> int:
    events = json.loads(events_path.read_text()).get("events", [])
    bt = next((e for e in events if e.get("k") == "BACKTRACE"), None)
    flips = [e for e in events if e.get("k") == "FLIP"]
    writer = next((e for e in events if e.get("k") == "WRITER_FUNC"), None)
    call_leaves = [e for e in events if e.get("k") == "CALL_LEAVE"]
    leaves255 = [e for e in events if e.get("k") == "4409E0_LEAVE" and e.get("tint_0x40") == 255.0]
    report = {
        "positive_control": {
            "4409E0_leave_255": len(leaves255),
            "passed": len(leaves255) > 0,
        },
        "first_backtrace": bt,
        "flip_events": flips,
        "writer_func": writer,
        "call_leave_flips": [
            e
            for e in call_leaves
            if e.get("tint_in") == 255.0 and e.get("tint_0x40") == 152.0
        ],
        "hook_installs": [e for e in events if e.get("k") == "HOOK_INSTALL"],
    }
    (out / "analysis.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"passed": report["positive_control"]["passed"], "writer": writer}))
    return 0


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--output", type=Path, default=OUT_DEFAULT)
    ap.add_argument(
        "command",
        choices=(
            "prepare",
            "start-menu",
            "run-level",
            "build-manifest",
            "verify-hooks",
            "shutdown",
            "analyze",
        ),
    )
    ap.add_argument("--events", type=Path)
    args = ap.parse_args()
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=True)
    try:
        if args.command == "prepare":
            return cmd_prepare(out)
        if args.command == "start-menu":
            return cmd_start_menu(out)
        if args.command == "run-level":
            return cmd_run_level(out)
        if args.command == "build-manifest":
            if not args.events:
                raise SystemExit("--events required")
            return cmd_build_manifest(out, args.events)
        if args.command == "verify-hooks":
            return cmd_verify_hooks(out)
        if args.command == "shutdown":
            return cmd_shutdown(out)
        if args.command == "analyze":
            if not args.events:
                raise SystemExit("--events required")
            return cmd_analyze(out, args.events)
    except Exception:
        traceback.print_exc()
        return 1
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
