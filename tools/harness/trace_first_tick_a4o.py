"""A4o: Stalker trace first walker update call at 0x425CCC for ctrl+0x40 flip."""
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
OUT_DEFAULT = ROOT / "build/tmp/a4o-first-tick-trace"
STOCK_EXE = Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")
STOCK_SHA256 = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
INSTALL = STOCK_EXE.parent
HOOKS = (
    (0x004409E0, 16),
    (0x0042E2BF, 16),
    (0x00403820, 16),
    (0x00403F40, 16),
)
PLAY_SEC = 15.0
TEXT_VA = 0x00401000
TEXT_FILE_OFF = 0x1000

sys.path.insert(0, str(ROOT / "tools" / "harness"))
from acquire_game_foreground import acquire  # noqa: E402
from goto_level import loaded_level  # noqa: E402
from lrinput import LRInput  # noqa: E402
from lrstate import attach, ui_state  # noqa: E402
from memdump import find_pids, k32, read_mem  # noqa: E402
from watch_actor_tint_a4i import load_enter_fast  # noqa: E402


def install_hashes() -> dict[str, str]:
    names = ("Lemmings Revolution.exe", "SavedGame.dat", "dgVoodoo.conf")
    return {n: hashlib.sha256((INSTALL / n).read_bytes()).hexdigest() for n in names}


def pe_file_bytes(addr: int, length: int) -> bytes:
    off = TEXT_FILE_OFF + (addr - TEXT_VA)
    return STOCK_EXE.read_bytes()[off : off + length]


def live_hook_bytes(handle, addr: int, length: int) -> bytes:
    data = read_mem(handle, addr, length)
    if not data or len(data) != length:
        raise RuntimeError(f"process READ failed {addr:#x}")
    return data


def load_enter(owned_pid: int, work: Path):
    enter = load_enter_fast(owned_pid)
    enter.WORK = work
    return enter


IMAGE_LO = 0x00401000
IMAGE_HI = 0x004C6000


def manifest_from_events(events: list[dict]) -> list[str]:
    addrs = {hex(a) for a, _ in HOOKS}
    for e in events:
        if e.get("k") == "HOOK_INSTALL":
            a = int(e["addr"], 16)
            if IMAGE_LO <= a < IMAGE_HI:
                addrs.add(hex(a))
    return sorted(addrs)


def cmd_prepare(out: Path) -> int:
    before = install_hashes()
    if before["Lemmings Revolution.exe"] != STOCK_SHA256:
        raise SystemExit("exe hash mismatch")
    (out / "before.json").write_text(json.dumps(before, indent=2) + "\n", encoding="utf-8")
    hooks = {hex(a): pe_file_bytes(a, n).hex() for a, n in HOOKS}
    (out / "hook-file-bytes.json").write_text(json.dumps(hooks, indent=2) + "\n", encoding="utf-8")
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
    (out / "enter.json").write_text(
        json.dumps({"enter_rc": rc, "loaded_level": lvl}, indent=2) + "\n",
        encoding="utf-8",
    )
    k32.CloseHandle(handle)
    print(json.dumps({"enter_rc": rc, "level": lvl}))
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
        raise SystemExit("attach failed")
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
    enters = [e for e in events if e.get("k") == "CALL_SITE_ENTER"]
    flip = next((e for e in events if e.get("k") == "STALKER_FLIP"), None)
    flip_sub = next((e for e in events if e.get("k") == "FLIP_SUBCALL"), None)
    stalk_end = next((e for e in events if e.get("k") == "STALKER_END"), None)
    first_enter = enters[0] if enters else None
    block_hex = None
    flip_store_va = 0x403877  # MOVSD.REP ctrl+0xa8 -> ctrl+0x40 (READ + subcall bisect)
    if flip and flip.get("block"):
        ba = int(flip["block"], 16)
        block_hex = pe_file_bytes(ba, 32).hex()
    elif flip_sub:
        block_hex = pe_file_bytes(flip_store_va, 32).hex()
    report = {
        "walker_spawns": len([e for e in events if e.get("k") == "4409E0_WALKER"]),
        "call_site_enters": len(enters),
        "first_call": first_enter,
        "stalker_flip": flip,
        "flip_subcall": flip_sub,
        "stalker_end": stalk_end,
        "flip_store_va": hex(flip_store_va) if flip_sub else None,
        "flip_block_file_bytes_32": block_hex,
    }
    if first_enter and first_enter.get("before") and first_enter.get("after", {}):
        pass
    leaves = [e for e in events if e.get("k") == "CALL_SITE_LEAVE"]
    if leaves:
        report["first_call_leave"] = leaves[0]
    (out / "analysis.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"target": first_enter.get("target_measured") if first_enter else None, "flip": flip.get("block") if flip else None}))
    return 0


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--output", type=Path, default=OUT_DEFAULT)
    ap.add_argument(
        "command",
        choices=("prepare", "start-menu", "run-level", "build-manifest", "verify-hooks", "shutdown", "analyze"),
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
