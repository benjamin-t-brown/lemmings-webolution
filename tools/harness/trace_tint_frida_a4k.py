"""A4k: Frida interceptors + optional MAM for walk/block controller tint 152."""
from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import struct
import subprocess
import sys
import time
import traceback
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT_DEFAULT = ROOT / "build/tmp/a4k-frida-tint"
STOCK_EXE = Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")
STOCK_SHA256 = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
INSTALL = STOCK_EXE.parent
HOOKS = (
    (0x004409E0, 16),
    (0x00406260, 16),
    (0x00455BA0, 16),
)
ACTOR_LIST = 0x004CBB18
LEMMING_VT = 0x004B10E8
OTHER_VT = 0x004B0F6C
PLAY_SEC = 15.0

sys.path.insert(0, str(ROOT / "tools" / "harness"))
from acquire_game_foreground import acquire  # noqa: E402
from goto_level import loaded_level  # noqa: E402
from lrinput import LRInput  # noqa: E402
from lrstate import attach, i32, ui_state, u32  # noqa: E402
from memdump import find_pids, k32, read_mem  # noqa: E402
from measure_spawn import actor_ptrs  # noqa: E402
from watch_actor_tint_a4i import advancement_snapshot, f64, gameplay_advanced, load_enter_fast, read_exact  # noqa: E402


def install_hashes() -> dict[str, str]:
    names = ("Lemmings Revolution.exe", "SavedGame.dat", "dgVoodoo.conf")
    return {n: hashlib.sha256((INSTALL / n).read_bytes()).hexdigest() for n in names}


def file_hook_bytes() -> dict[str, str]:
    out = {}
    script = ROOT / "tools/harness/read_steppers.py"
    for addr, length in HOOKS:
        proc = subprocess.run(
            [sys.executable, str(script), "--at", hex(addr), "--len", hex(length)],
            capture_output=True,
            text=True,
            cwd=ROOT,
            timeout=60,
        )
        out[hex(addr)] = {
            "returncode": proc.returncode,
            "stdout_tail": (proc.stdout or "")[-2000:],
            "file_hex": _extract_hex_from_stepper(proc.stdout or ""),
        }
    return out


def _extract_hex_from_stepper(text: str) -> str | None:
    for line in text.splitlines():
        if "file offset" in line.lower() or "bytes:" in line.lower():
            continue
        parts = line.split()
        for p in parts:
            if len(p) >= 32 and all(c in "0123456789abcdefABCDEF" for c in p):
                return p.lower()
    return None


def live_hook_bytes(handle, addr: int, length: int) -> bytes:
    data = read_mem(handle, addr, length)
    if not data or len(data) != length:
        raise RuntimeError(f"hook read failed {addr:#x}")
    return data


def load_enter(owned_pid: int, work: Path):
    enter = load_enter_fast(owned_pid)
    enter.WORK = work
    return enter


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
                "temporal": i32(handle, c + 0x1CC),
                "direction": i32(handle, c + 0x1B8),
                "tint_r_0x40": f64(handle, c + 0x40),
                "draw_position": list(struct.unpack("<3d", read_exact(handle, actor + 0x108, 24))),
            }
        )
    return rows


def parse_frida_messages(raw: dict) -> list[dict]:
    events = []
    for msg in raw.get("messages") or raw.get("script_messages") or []:
        payload = msg.get("payload") if isinstance(msg, dict) else msg
        if isinstance(payload, str):
            try:
                payload = json.loads(payload)
            except json.JSONDecodeError:
                payload = {"k": "raw", "text": payload}
        if isinstance(payload, dict):
            events.append(payload)
    return events


def map_events_to_actors(events: list[dict], actors: list[dict]) -> dict:
    ctrl_set = {a["controller"] for a in actors}
    by_ctrl = {a["controller"]: a for a in actors}
    mapped = {}
    for a in actors:
        c = a["controller"]
        mapped[c] = {
            "actor_row": a,
            "events": [],
        }
    for ev in events:
        k = ev.get("k", "")
        ecx = ev.get("ecx")
        dst = ev.get("dst")
        for target in (ecx, dst):
            if not target:
                continue
            t = target.lower()
            if not t.startswith("0x"):
                t = "0x" + t
            if t in mapped:
                mapped[t]["events"].append(ev)
            elif t in ctrl_set:
                mapped[t]["events"].append(ev)
    return mapped


def positive_control_ok(events: list[dict]) -> dict:
    e4409 = [e for e in events if e.get("k") == "4409E0"]
    e406 = [e for e in events if e.get("k") == "406260"]
    e255 = [e for e in e406 if e.get("r") == 255.0 and e.get("g") == 255.0 and e.get("b") == 255.0]
    return {
        "count_4409E0": len(e4409),
        "count_406260": len(e406),
        "count_406260_255": len(e255),
        "passed": len(e4409) > 0 and len(e255) > 0,
    }


def cmd_prepare(out: Path) -> int:
    before = install_hashes()
    if before["Lemmings Revolution.exe"] != STOCK_SHA256:
        raise SystemExit("exe hash mismatch")
    (out / "before.json").write_text(json.dumps(before, indent=2) + "\n", encoding="utf-8")
    hooks = file_hook_bytes()
    (out / "hook-file-bytes.json").write_text(json.dumps(hooks, indent=2) + "\n", encoding="utf-8")
    return 0


def cmd_start_menu(out: Path) -> int:
    if find_pids("Lemmings"):
        raise SystemExit("exclusive ownership: game already running")
    conf_backup = (INSTALL / "dgVoodoo.conf").read_bytes()
    (out / "dgVoodoo.backup").write_bytes(conf_backup)
    windowed = ROOT / "build/tmp/a1-zoom-elevation/dgVoodoo.windowed.conf"
    (INSTALL / "dgVoodoo.conf").write_bytes(windowed.read_bytes())
    owned = subprocess.Popen([str(STOCK_EXE)], cwd=INSTALL)
    time.sleep(10)
    handle, pid = attach()
    if not handle or pid != owned.pid:
        raise SystemExit("attach failed")
    state = {
        "pid": owned.pid,
        "ui_state": ui_state(handle),
        "loaded_level": loaded_level(handle),
        "game_starts": 1,
    }
    (out / "pid.json").write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
    k32.CloseHandle(handle)
    print(json.dumps(state), flush=True)
    return 0


def cmd_play_only(out: Path) -> int:
    state = json.loads((out / "pid.json").read_text(encoding="utf-8"))
    pid = state["pid"]
    handle, apid = attach()
    if not handle or apid != pid:
        raise SystemExit("attach mismatch")
    before = advancement_snapshot(handle, actor_ptrs(handle)[0]) if actor_ptrs(handle) else {}
    acquire(pid)
    with LRInput() as inp:
        inp.enable()
        if ui_state(handle) == 1:
            inp.tap("P", 2)
        inp.wait_frames(2)
        time.sleep(PLAY_SEC)
        acquire(pid)
        if ui_state(handle) == 0:
            inp.tap("P", 2)
        inp.release()
    after = advancement_snapshot(handle, actor_ptrs(handle)[0]) if actor_ptrs(handle) else {}
    report = {
        "before_play": before,
        "after_play": after,
        "gameplay_advanced": gameplay_advanced(before, after) if before and after else False,
        "walk_block_actors": walk_block_actors(handle),
        "actor_count_before": i32(handle, ACTOR_LIST),
    }
    (out / "live-snapshot-mam.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    k32.CloseHandle(handle)
    print(json.dumps(report, indent=2))
    return 0


def cmd_run_level(out: Path) -> int:
    state = json.loads((out / "pid.json").read_text(encoding="utf-8"))
    pid = state["pid"]
    if not find_pids("Lemmings"):
        raise SystemExit("game not running")
    handle, apid = attach()
    if not handle or apid != pid:
        raise SystemExit("attach mismatch")
    before = advancement_snapshot(handle, actor_ptrs(handle)[0]) if actor_ptrs(handle) else {}
    enter = load_enter(pid, out)
    rc = enter.main()
    lvl = loaded_level(handle)
    report = {
        "enter_rc": rc,
        "loaded_level": lvl,
        "before_play": {
            "ui_state": ui_state(handle),
            "actor_count": i32(handle, ACTOR_LIST),
            "ref_snapshot": before,
        },
    }
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
    after = advancement_snapshot(handle, actor_ptrs(handle)[0]) if actor_ptrs(handle) else {}
    report["after_play"] = {
        "ui_state": ui_state(handle),
        "actor_count": i32(handle, ACTOR_LIST),
        "ref_snapshot": after,
        "gameplay_advanced": gameplay_advanced(before, after) if before and after else False,
    }
    report["walk_block_actors"] = walk_block_actors(handle)
    (out / "live-snapshot.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    k32.CloseHandle(handle)
    print(json.dumps(report, indent=2), flush=True)
    return 0 if rc == 0 else 1


def cmd_verify_hooks(out: Path) -> int:
    state = json.loads((out / "pid.json").read_text(encoding="utf-8"))
    pid = state["pid"]
    handle, apid = attach()
    if not handle or apid != pid:
        raise SystemExit("attach failed for verify")
    file_meta = json.loads((out / "hook-file-bytes.json").read_text(encoding="utf-8"))
    checks = {}
    for addr, length in HOOKS:
        live = live_hook_bytes(handle, addr, length).hex()
        checks[hex(addr)] = {"live_hex": live, "file_meta": file_meta.get(hex(addr))}
    (out / "hook-live-bytes.json").write_text(json.dumps(checks, indent=2) + "\n", encoding="utf-8")
    k32.CloseHandle(handle)
    return 0


def cmd_shutdown(out: Path) -> int:
    state = json.loads((out / "pid.json").read_text(encoding="utf-8"))
    pid = state["pid"]
    procs = find_pids("Lemmings")
    for p, _ in procs:
        if p == pid:
            subprocess.run(
                ["taskkill", "/PID", str(pid), "/F"],
                capture_output=True,
                check=False,
            )
            break
    if (out / "dgVoodoo.backup").exists():
        (INSTALL / "dgVoodoo.conf").write_bytes((out / "dgVoodoo.backup").read_bytes())
    after = install_hashes()
    before = json.loads((out / "before.json").read_text(encoding="utf-8"))
    (out / "after.json").write_text(json.dumps(after, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"install_match": after == before, "process_stopped": not find_pids("Lemmings")}))
    return 0


def cmd_analyze(out: Path, events_path: Path) -> int:
    raw = json.loads(events_path.read_text(encoding="utf-8"))
    events = parse_frida_messages(raw)
    snap = json.loads((out / "live-snapshot.json").read_text(encoding="utf-8"))
    actors = snap.get("walk_block_actors") or []
    pc = positive_control_ok(events)
    mapped = map_events_to_actors(events, actors)
    need_mam = False
    for a in actors:
        if abs(a.get("tint_r_0x40", 0) - 152.0) < 0.01:
            evs = mapped.get(a["controller"], {}).get("events", [])
            explains = any(
                e.get("k") == "406260"
                and abs((e.get("r") or 0) - 152.0) < 0.01
                or e.get("k") == "455BA0"
                for e in evs
            )
            if not explains:
                need_mam = True
    report = {
        "event_count": len(events),
        "positive_control": pc,
        "need_mam": need_mam,
        "mapped": mapped,
        "events_sample": events[:30],
    }
    (out / "analysis.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"positive_control": pc, "need_mam": need_mam, "events": len(events)}))
    return 0


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--output", type=Path, default=OUT_DEFAULT)
    ap.add_argument(
        "command",
        choices=("prepare", "start-menu", "run-level", "play-only", "verify-hooks", "shutdown", "analyze"),
    )
    ap.add_argument("--events", type=Path, help="frida messages json for analyze")
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
        if args.command == "play-only":
            return cmd_play_only(out)
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
