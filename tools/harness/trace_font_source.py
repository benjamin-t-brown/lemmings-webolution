"""Expose the source of the `^Font` texture read with ONE hardware execute breakpoint.

READ-only GDB trace, written by b1s-font-live-preflight and NOT yet run against a
process. It needs an explicit PID and a fresh output directory below build/tmp, sends
no input and writes no process memory.

The breakpoint is `0x0047E4AD`, the archive-hit arm of the file open routine
`0x0047E40D` (reached at `0x0045019E` with `push 0; push 1`). On that path:

    request path   (char*)(*(u32*)(ebp-0x128) + 0x14)     the name 0x47E395 stored
    matched entry  (char*)0x5D5734                         record filled by 0x47F759
    archive        manager 0x5D5608: list [0x5D5614][ [0x5D5730] ] + 0x38  (path)
    size / offset  rec+0x104 -> this+0x10 (0x47E4B6), rec+0x108 -> this+0x124 (0x47E4C8);
                   0x47E6D4 passes this+0x124 to 0x49DCF5 with origin 0 (seek), so offset

The record pointer is [ebp-0x18]. It fires only on an archive hit, so a loose
file or a miss for `Textures\\^Font*` shows up as NO hit, not as a different value.
It does not see a load that finished before attach. The `^Font` request is in the
startup chain (0x450A46), so the paused-level guard (default --ui-state 1) cannot be
combined with observing that load; round 2 must decide how to arm before it.

    python tools/harness/trace_font_source.py --pid N --output build/tmp/<new dir>
    python tools/harness/trace_font_source.py --launch --output build/tmp/<new dir>
    python tools/harness/trace_font_source.py --selftest        (synthetic data only)

`--pid` attaches to a running game and so only sees loads that happen later. `--launch`
is the opt-in arming mode for the startup load: GDB itself starts the stock exe
(`starti`, which stops before the first user instruction), the hardware breakpoint is
inserted at that stop, then the game continues. It spends ONE start with no retry, so
every guard runs before the exe is created. Run it from a session-1 worker (see
docs/agents/MOUV_SSH.md); it never runs in --selftest. Whether GDB on this Windows
build supports `starti`, and whether debug registers set at that stop survive into the
main thread, is UNPROVEN offline.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import struct
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
STOCK_EXE = Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")
STOCK_SHA256 = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"

BP_ADDRESS = 0x0047E4AD
# mov edx,[ebp-0x128]; mov eax,[ebp-0x18]
SITE_BYTES = bytes.fromhex("8b95d8feffff8b45e8")
MANAGER = 0x005D5608       # count at +0, capacity +4, field +8 (gate, = 0x5D5610), list data +0xC
GATE = 0x005D5610
ARCHIVE_ONLY_FLAG = 0x005D5604
RECORD = 0x005D5730        # +0 archive index, +4 entry name, +0x104 ordinal, +0x108 second field
ENTRY_NAME = RECORD + 4
ENTRY_PREFIX = b"TEXTURES\\^FONT"
INSTALL_DIR = STOCK_EXE.parent
INSTALL_SHA256 = {  # recorded 2026-10-23 on mouv (b1s hashes-before.txt / hashes-after.txt)
    "Lemmings Revolution.exe": "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f",
    "SavedGame.dat": "f7f8603928890fb13d022d828c8a602cf7a7619ab24de82c7de4ae86ec337e6a",
    "dgVoodoo.conf": "d639e220814d80b7956b96bad0ca708b768458bd348bfd607afbf1913598e72b",
    "dinput.dll": "9c610506ad29c6fe1e056992683f9ac7b4a231c3b2422e0b7cfd7499ba6861ac",
}
HIT_MARKER = "FONT_HIT"
STOP_MARKER = "FONT_PROBE_STOP"


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def prefix_condition(prefix: bytes = ENTRY_PREFIX, name_address: int = ENTRY_NAME) -> str:
    """GDB condition comparing the first bytes of the stored (upper-case) entry name.

    Integer compares only: `$_streq` and friends need GDB's Python and may be absent.
    """
    if len(prefix) != 14:
        raise ValueError("condition is built for a 14 byte prefix")
    a = struct.unpack_from("<I", prefix, 0)[0]
    b = struct.unpack_from("<I", prefix, 4)[0]
    c = struct.unpack_from("<H", prefix, 8)[0]
    d = struct.unpack_from("<I", prefix, 10)[0]
    return (f"*(unsigned*){name_address:#x} == {a:#x} && *(unsigned*){name_address + 4:#x} == {b:#x}"
            f" && *(unsigned short*){name_address + 8:#x} == {c:#x}"
            f" && *(unsigned*){name_address + 10:#x} == {d:#x}")


def cond_matches(memory: dict[int, int], prefix: bytes = ENTRY_PREFIX, name_address: int = ENTRY_NAME) -> bool:
    """Python mirror of prefix_condition over a synthetic {address: byte} map."""
    return all(memory.get(name_address + i) == prefix[i] for i in range(len(prefix)))


def breakpoint_actions(stop_after: int, dump_dir: str = ".") -> list[str]:
    """`dump_dir` must be absolute in launch mode: GDB's cwd there is the install directory."""
    return [
        "silent",
        "set $hits=$hits+1",
        "set $rec=*(unsigned*)($ebp-0x18)",
        "set $obj=*(unsigned*)($ebp-0x128)",
        "set $aidx=*(unsigned*)$rec",
        "set $arch=*(unsigned*)(*(unsigned*)(%#x)+$aidx*4)" % (MANAGER + 0xC),
        f'printf "{HIT_MARKER} n=%d this=%#x rec=%#x archidx=%u archobj=%#x size=%u offset=%u gate=%u req=%s entry=%s archive=%s\\n",'
        "$hits,$obj,$rec,$aidx,$arch,*(unsigned*)($rec+0x104),*(unsigned*)($rec+0x108),"
        f"*(unsigned*){GATE:#x},(char*)($obj+0x14),(char*)($rec+4),(char*)($arch+0x38)",
        f"dump binary memory {dump_dir}/record-last.bin $rec $rec+0x10c",
        f"if $hits >= {stop_after}",
        f'printf "{STOP_MARKER}\\n"',
        "else",
        "continue",
        "end",
    ]


HIT_RE = re.compile(
    rf"{HIT_MARKER} n=(\d+) this=(0x[0-9a-f]+) rec=(0x[0-9a-f]+) archidx=(\d+) archobj=(0x[0-9a-f]+) "
    r"size=(\d+) offset=(\d+) gate=(\d+) req=(.*?) entry=(.*?) archive=(.*)$")


def parse_hit(line: str) -> dict | None:
    m = HIT_RE.search(line)
    if not m:
        return None
    n, this, rec, aidx, aobj, ord_, f108, gate, req, entry, archive = m.groups()
    return {"n": int(n), "this": this, "record": rec, "archive_index": int(aidx), "archive_object": aobj,
            "size_field_0x104": int(ord_), "offset_field_0x108": int(f108), "gate": int(gate),
            "request_path": req, "matched_entry": entry, "archive_path": archive}


def check_output_dir(output: Path, tmp: Path) -> Path:
    output = output.resolve()
    try:
        output.relative_to(tmp.resolve())
    except ValueError:
        raise SystemExit(f"--output must be below {tmp}")
    if output.exists():
        raise SystemExit(f"--output must be new: {output}")
    return output


def check_stock(exe_bytes: bytes) -> str:
    digest = sha(exe_bytes)
    if digest != STOCK_SHA256:
        raise RuntimeError("installed executable differs from the stock authority")
    return digest


def check_site(site: bytes) -> None:
    if site != SITE_BYTES:
        raise RuntimeError(f"bytes at {BP_ADDRESS:#x} are {site.hex()}, expected {SITE_BYTES.hex()}")


def validate_args(args: argparse.Namespace) -> None:
    """Mode and range checks; raises SystemExit. Nothing here touches the machine."""
    if args.selftest:
        return
    if args.launch == (args.pid is not None):
        raise SystemExit("choose exactly one of --pid (attach) or --launch")
    if args.output is None:
        raise SystemExit("--output is required")
    if args.stop_after < 1:
        raise SystemExit("--stop-after must be >= 1")
    if args.launch and not 0 < args.boot_timeout <= 600:
        raise SystemExit("--boot-timeout must be in (0, 600]")


def game_processes(tasklist_text: str) -> list[str]:
    """Lines of a `tasklist` listing that name a Lemmings process."""
    return [ln for ln in tasklist_text.splitlines() if "lemming" in ln.lower()]


def _g_install(c):
    bad = [k for k, v in INSTALL_SHA256.items() if c["install_sha256"].get(k) != v]
    return f"install files differ from the recorded hashes: {bad}" if bad else None


def _g_no_game(c):
    found = game_processes(c["tasklist"])
    return f"a Lemmings process already exists: {found[0].strip()}" if found else None


def _g_first_start(c):
    return "a launch was already attempted in this run; no retry" if c["launch_attempted"] else None


def _g_site(c):
    return None if c["site_bytes"] == SITE_BYTES else f"bytes at {BP_ADDRESS:#x} are {c['site_bytes'].hex()}"


LAUNCH_GUARDS = {"install": _g_install, "no_game": _g_no_game, "first_start": _g_first_start}
ENTRY_STOP_GUARDS = {"site": _g_site}


def run_guards(guards: dict, ctx: dict) -> list[str]:
    return [f"{name}: {msg}" for name, g in guards.items() if (msg := g(ctx))]


def parse_memory_bytes(mi_line: str) -> bytes:
    """Bytes from a `-data-read-memory-bytes` result line."""
    m = re.search(r'contents="([0-9a-f]+)"', mi_line)
    if not m:
        raise RuntimeError("no contents in memory read: " + mi_line[:200])
    return bytes.fromhex(m.group(1))


def parse_args(argv=None) -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--pid", type=int, help="attach mode: PID of the already owned stock original")
    p.add_argument("--launch", action="store_true",
                   help="launch mode: GDB starts the stock exe and arms the breakpoint before the load (ONE start, no retry)")
    p.add_argument("--boot-timeout", type=float, default=90.0,
                   help="launch mode: seconds to wait for the entry stop and for the first hit")
    p.add_argument("--output", type=Path, help="new directory under build/tmp")
    p.add_argument("--stop-after", type=int, default=None,
                   help="stop after this many matching hits (default: 2 with --launch, the expected ^FONT.BMP + .ALF pair; 4 with --pid)")
    p.add_argument("--timeout", type=float, default=120.0, help="seconds to wait for the stop marker")
    p.add_argument("--ui-state", type=int, default=1,
                   help="required lrstate.ui_state (1 = paused level, the precedent guard); -1 skips the guard")
    p.add_argument("--gdb", type=Path, default=Path(r"C:\progs\msys2\mingw32\bin\gdb.exe"))
    p.add_argument("--selftest", action="store_true", help="exercise argument/guard logic on synthetic data only")
    args = p.parse_args(argv)
    if args.stop_after is None:
        args.stop_after = 2 if args.launch else 4
    validate_args(args)
    return args


def collect_hits(mi, report: dict) -> None:
    """Copy every hit already printed into the report; safe to call after a failed wait."""
    lines = [json.loads(x[1:]).strip() for x in mi.lines if x.startswith("~")]
    report["console"] = [x for x in lines if HIT_MARKER in x or STOP_MARKER in x]
    report["hits"] = [h for h in map(parse_hit, lines) if h]


def arm_and_collect(mi, args, report: dict, timeout: float, dump_dir: str = ".") -> None:
    """Insert the one breakpoint, run to the stop marker, delete it while stopped."""
    mi.console("set $hits=0")
    # One fixed dump file, overwritten per hit: GDB cannot interpolate $hits into a name.
    mi.breakpoint(BP_ADDRESS, prefix_condition(), breakpoint_actions(args.stop_after, dump_dir))
    report["breakpoint_armed"] = True
    mi.resume()
    try:
        mi.wait_marker(STOP_MARKER, timeout=timeout)
    except BaseException as exc:
        # A timeout or an unexpected stop must still report the hits that fired (gdb-mi.log has them too).
        report["wait_error"] = repr(exc)
        collect_hits(mi, report)
        raise
    collect_hits(mi, report)
    # Delete while stopped, then prove the debug registers are empty before detach.
    mi.command("-break-delete", timeout=10)
    mi.console("info breakpoints")
    info = "\n".join(json.loads(x[1:]) for x in mi.lines if x.startswith("~"))
    report["breakpoints_deleted_before_detach"] = "No breakpoints or watchpoints" in info
    if not report["breakpoints_deleted_before_detach"]:
        raise RuntimeError("breakpoints still listed before detach")
    report["completed"] = bool(report["hits"])


def run_live(args: argparse.Namespace) -> int:
    # Windows-only imports stay here so --selftest runs anywhere.
    sys.path.insert(0, str(ROOT / "tools" / "harness"))
    from acquire_game_foreground import acquire
    from lrstate import attach, ui_state, u32
    from memdump import k32, read_mem
    from trace_cell_emission import MI

    if args.launch:
        return run_launch(args)
    output = check_output_dir(args.output, ROOT / "build" / "tmp")
    output.mkdir(parents=True, exist_ok=False)
    report: dict = {"pid": args.pid, "breakpoint": hex(BP_ADDRESS), "completed": False,
                    "scope": "READ-only; one hardware execute breakpoint at the archive-hit arm of 0x47E40D"}
    handle = None
    mi = None
    try:
        report["authority_sha256"] = check_stock(STOCK_EXE.read_bytes())
        report["foreground"] = acquire(args.pid)
        handle, attached_pid = attach()
        if not handle or attached_pid != args.pid:
            raise RuntimeError(f"process attach PID mismatch: {attached_pid}")
        if args.ui_state != -1 and ui_state(handle) != args.ui_state:
            raise RuntimeError(f"ui_state guard failed (wanted {args.ui_state})")
        site = read_mem(handle, BP_ADDRESS, len(SITE_BYTES))
        check_site(site or b"")
        report["pre_arm"] = {"site_bytes": site.hex(), "gate_0x5d5610": u32(handle, GATE),
                             "manager_count_0x5d5608": u32(handle, MANAGER),
                             "archive_only_flag_0x5d5604": read_mem(handle, ARCHIVE_ONLY_FLAG, 1)[0]}
        mi = MI(args.gdb, args.pid, output)
        mi.start()
        report["debugger_attached"] = True
        arm_and_collect(mi, args, report, timeout=args.timeout)
    except Exception as exc:
        report["error"] = repr(exc)
    finally:
        if mi:
            try:
                mi.cleanup(report)
            except Exception as exc:
                report["gdb_cleanup_error"] = repr(exc)
        if handle:
            try:
                k32.CloseHandle(handle)
            except Exception:
                pass
        (output / "trace-result.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
        print(json.dumps(report, indent=2), flush=True)
    clean = (not report.get("debugger_attached_after_cleanup", True)
             and report.get("gdb_exit_code") == 0
             and not report.get("gdb_cleanup_error") and not report.get("gdb_cleanup_errors"))
    return 0 if report.get("completed") and clean else 1


def launch_preflight() -> dict:
    """Machine state read before the exe exists (tasklist, install hashes). Replaced in --selftest."""
    import subprocess
    from hashlib import sha256
    tasklist = subprocess.run(["tasklist"], capture_output=True, text=True, timeout=30).stdout
    return {"install_sha256": {k: sha256((INSTALL_DIR / k).read_bytes()).hexdigest() for k in INSTALL_SHA256},
            "tasklist": tasklist}


def launch_mi_class(base):
    """GDB/MI session class that starts the stock exe instead of attaching. `base` is MI (or a recorder)."""

    class LaunchMI(base):
        def start(self) -> None:
            for setting in ("pagination off", "confirm off", "auto-solib-add off", "mi-async on"):
                self.command("-gdb-set " + setting)
            self.command("-environment-cd " + json.dumps(INSTALL_DIR.as_posix()))  # the game expects its own directory
            self.command("-file-exec-and-symbols " + json.dumps(STOCK_EXE.as_posix()))

        def wait_any_stop(self, timeout: float = 10, since: int | None = None) -> str:
            if since is None:
                return super().wait_any_stop(timeout)
            # command() drains the queue while waiting for its result token, so a `*stopped`
            # record that arrives before `^done` is gone from the queue but stays in self.lines.
            deadline = time.monotonic() + timeout
            while time.monotonic() < deadline:
                for line in self.lines[since:]:
                    if line.startswith("*stopped"):
                        self.running = False
                        return line
                time.sleep(0.1)
            raise TimeoutError("inferior stop")

    return LaunchMI


def make_launch_mi(gdb: Path, output: Path):
    sys.path.insert(0, str(ROOT / "tools" / "harness"))
    from trace_cell_emission import MI
    return launch_mi_class(MI)(gdb, 0, output)


def run_launch(args: argparse.Namespace, preflight=launch_preflight, mi_factory=make_launch_mi) -> int:
    """Opt-in arming mode: GDB creates the stock process, stops at its first instruction,
    the breakpoint is inserted there, then the game runs. ONE start, no retry.

    UNPROVEN offline: GDB `starti` on this Windows build, and that a debug register set at
    that stop is applied to the thread that later runs the startup chain.
    """
    output = check_output_dir(args.output, ROOT / "build" / "tmp")
    output.mkdir(parents=True, exist_ok=False)
    report: dict = {"mode": "launch", "breakpoint": hex(BP_ADDRESS), "completed": False, "launch_attempted": False,
                    "scope": "one GDB-created stock process; one hardware execute breakpoint; no input, no memory writes"}
    mi = None
    try:
        # Every guard that can be checked before the exe exists runs first; the start is spent after.
        ctx = dict(preflight(), launch_attempted=report["launch_attempted"])
        failed = run_guards(LAUNCH_GUARDS, ctx)
        if failed:
            raise RuntimeError("; ".join(failed))
        report["install_sha256"] = ctx["install_sha256"]
        if not args.gdb.exists():
            raise RuntimeError(f"gdb not found: {args.gdb}")
        mi = mi_factory(args.gdb, output)
        mi.start()
        report["launch_attempted"] = True   # recorded before the start; a failure below is final
        mark = len(mi.lines)
        mi.console("starti")
        mi.attached = True                  # from here cleanup must delete the breakpoint and detach
        report["debugger_attached"] = True
        report["entry_stop"] = mi.wait_any_stop(timeout=args.boot_timeout, since=mark)
        mem = mi.command("-data-read-memory-bytes %#x %d" % (BP_ADDRESS, len(SITE_BYTES)))
        site = parse_memory_bytes(mem)
        failed = run_guards(ENTRY_STOP_GUARDS, {"site_bytes": site})
        if failed:
            raise RuntimeError("; ".join(failed))
        report["entry_stop_site_bytes"] = site.hex()
        while not mi.messages.empty():     # a stale entry-stop record must not be read as an unexpected stop
            mi.messages.get_nowait()
        pids = re.findall(r'pid="(\d+)"', mi.command("-list-thread-groups"))
        report["pid"] = int(pids[0]) if pids else None
        arm_and_collect(mi, args, report, timeout=args.boot_timeout, dump_dir=output.as_posix())
    except Exception as exc:
        report["error"] = repr(exc)
    finally:
        if mi:
            try:
                mi.cleanup(report)
            except Exception as exc:
                report["gdb_cleanup_error"] = repr(exc)
        report["game_left_running_by_this_script"] = report.get("pid")
        (output / "trace-result.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
        print(json.dumps(report, indent=2), flush=True)
    clean = (not report.get("debugger_attached_after_cleanup", True)
             and report.get("gdb_exit_code") == 0
             and not report.get("gdb_cleanup_error") and not report.get("gdb_cleanup_errors"))
    return 0 if report.get("completed") and clean else 1


def selftest() -> int:
    failures = []

    def check(name, ok):
        print(("PASS " if ok else "FAIL ") + name)
        if not ok:
            failures.append(name)

    # argument logic
    a = parse_args(["--pid", "123", "--output", "build/tmp/x"])
    check("defaults", a.stop_after == 4 and a.ui_state == 1 and not a.selftest)
    # output-dir guard
    import tempfile
    with tempfile.TemporaryDirectory() as t:
        tmp = Path(t) / "build" / "tmp"
        tmp.mkdir(parents=True)
        check("output below tmp accepted", check_output_dir(tmp / "new", tmp) == (tmp / "new").resolve())
        for bad, why in ((Path(t) / "elsewhere", "outside tmp rejected"), (tmp, "existing dir rejected")):
            try:
                check_output_dir(bad, tmp)
                check(why, False)
            except SystemExit:
                check(why, True)
    # stock hash guard
    try:
        check_stock(b"not the exe")
        check("hash guard rejects wrong bytes", False)
    except RuntimeError:
        check("hash guard rejects wrong bytes", True)
    # site-bytes guard
    check_site(SITE_BYTES)
    try:
        check_site(SITE_BYTES[:-1] + b"\x00")
        check("site guard rejects changed byte", False)
    except RuntimeError:
        check("site guard rejects changed byte", True)
    # condition: constants derived from the prefix match the hand-computed words
    cond = prefix_condition()
    check("condition constants", "0x54584554" in cond and "0x53455255" in cond and "0x5e5c" in cond and "0x544e4f46" in cond)
    check("condition addresses", f"{ENTRY_NAME:#x}" in cond and f"{ENTRY_NAME + 10:#x}" in cond)
    # python mirror on synthetic memory
    mem = {ENTRY_NAME + i: b for i, b in enumerate(b"TEXTURES\\^FONT.BMP\0")}
    check("mirror accepts ^FONT.BMP", cond_matches(mem))
    # isolated mutation: change one byte of the prefix, nothing else
    mutated = dict(mem)
    mutated[ENTRY_NAME + 10] = ord("G")
    check("MUTATION F->G in prefix is rejected", not cond_matches(mutated))
    # hit-line parser
    line = (f"{HIT_MARKER} n=1 this=0x19f9a4 rec=0x5d5730 archidx=1 archobj=0x2a3b40 size=1234 offset=5678 gate=1 "
            "req=Textures\\^Font.bmp entry=TEXTURES\\^FONT.BMP archive=C:\\progs\\Lemmings Revolution\\lemmings.box")
    h = parse_hit(line)
    check("parse_hit fields", bool(h) and h["size_field_0x104"] == 1234 and h["offset_field_0x108"] == 5678 and h["archive_path"].endswith("lemmings.box")
          and h["matched_entry"] == "TEXTURES\\^FONT.BMP" and h["gate"] == 1)
    check("parse_hit rejects noise", parse_hit("TARGET_ENTER quad=0x1") is None)
    # action list sanity: one breakpoint, ends the run via the marker
    acts = breakpoint_actions(4)
    check("actions carry stop marker and continue", any(STOP_MARKER in x for x in acts) and "continue" in acts)
    # ---- launch / arm mode: argument and guard logic, synthetic data only
    def rejects(argv):
        try:
            parse_args(argv)
            return False
        except SystemExit:
            return True
    check("launch accepted", not rejects(["--launch", "--output", "build/tmp/x"]))
    check("launch with --pid rejected", rejects(["--launch", "--pid", "5", "--output", "build/tmp/x"]))
    check("neither mode rejected", rejects(["--output", "build/tmp/x"]))
    check("missing --output rejected", rejects(["--launch"]))
    check("boot timeout range rejected", rejects(["--launch", "--output", "x", "--boot-timeout", "0"]))
    check("selftest never needs a mode", not rejects(["--selftest"]))
    good = {"install_sha256": dict(INSTALL_SHA256), "tasklist": "System Idle Process  0\nexplorer.exe  4\n",
            "launch_attempted": False}
    check("clean preflight passes every launch guard", run_guards(LAUNCH_GUARDS, good) == [])
    check("clean entry stop passes site guard", run_guards(ENTRY_STOP_GUARDS, {"site_bytes": SITE_BYTES}) == [])
    bad_cases = {
        "install": dict(good, install_sha256=dict(INSTALL_SHA256, **{"dinput.dll": "0" * 64})),
        "no_game": dict(good, tasklist="Lemmings Revolution.exe  1234\n"),
        "first_start": dict(good, launch_attempted=True),
    }
    for name, ctx in bad_cases.items():
        got = run_guards(LAUNCH_GUARDS, ctx)
        check(f"guard {name} rejects its bad input and only that one", len(got) == 1 and got[0].startswith(name))
        # isolated mutation: remove ONLY this guard; the bad input must now pass, proving the guard did the work
        without = {k: v for k, v in LAUNCH_GUARDS.items() if k != name}
        check(f"MUTATION without guard {name} the bad input is accepted", run_guards(without, ctx) == [])
    got = run_guards(ENTRY_STOP_GUARDS, {"site_bytes": SITE_BYTES[:-1] + b"\x00"})
    check("guard site rejects changed entry-stop bytes", len(got) == 1 and got[0].startswith("site"))
    check("MUTATION without guard site the changed bytes are accepted", run_guards({}, {"site_bytes": b"x"}) == [])
    check("parse_memory_bytes", parse_memory_bytes('1^done,memory=[{begin="0x0047e4ad",contents="8b95d8feffff8b45e8"}]') == SITE_BYTES)
    check("game_processes ignores unrelated rows", game_processes("cmd.exe 1\nLemmings Revolution.exe 2\n") == ["Lemmings Revolution.exe 2"])
    # ---- launch flow end to end against a recording stub (no GDB, no process)
    try:
        from trace_cell_emission import MI
    except Exception as exc:  # Windows-only imports; run the selftest on mouv for these checks
        print("SKIP launch stub checks (MI import failed: %s)" % type(exc).__name__)
        MI = None
    if MI is not None:
        import queue
        import shutil
        import tempfile

        def hit_line(n, entry):
            return (f"{HIT_MARKER} n={n} this=0x19f9a4 rec=0x5d5730 archidx=1 archobj=0x2a3b40 size=10 offset=20 gate=1 "
                    f"req=Textures\\^Font.x entry={entry} archive=C:\\x\\lemmings.box")

        class Rec(MI):
            script_hits = 2
            marker = True
            sequence: list = []

            def __init__(self, gdb, pid, output):
                self.messages = queue.Queue(); self.lines = []; self.token = 0; self.attached = False
                self.running = False; self.bp_numbers = []; self.output = output; self.pid = pid
                self.proc = type("P", (), {"poll": lambda s: None if not self.exited else 0,
                                           "wait": lambda s, timeout=None: 0})()
                self.exited = False
                Rec.sequence = []

            def emit(self, line):
                self.lines.append(line); self.messages.put(line)

            def command(self, text, timeout=20):
                Rec.sequence.append(text)
                if text == "-gdb-exit":
                    self.exited = True
                elif text.startswith("-interpreter-exec console"):
                    inner = json.loads(text.split(" ", 2)[2])
                    if inner == "starti":
                        self.emit('*stopped,reason="signal-received",signal-name="SIGTRAP"')
                    elif inner == "info breakpoints":
                        self.emit("~" + json.dumps("No breakpoints or watchpoints.\n"))
                elif text.startswith("-exec-continue"):
                    self.emit('*running,thread-id="all"')
                    for i in range(1, self.script_hits + 1):
                        self.emit("~" + json.dumps(hit_line(i, "TEXTURES\\^FONT.%s" % ("BMP", "ALF")[i % 2 == 0]) + "\n"))
                    if self.marker:
                        self.emit("~" + json.dumps(STOP_MARKER + "\n"))
                        self.emit('*stopped,reason="breakpoint-hit"')
                elif text.startswith("-data-read-memory-bytes"):
                    return '1^done,memory=[{begin="0x0047e4ad",contents="%s"}]' % SITE_BYTES.hex()
                elif text.startswith("-list-thread-groups"):
                    return '1^done,groups=[{id="i1",type="process",pid="4242"}]'
                elif text.startswith("-break-insert"):
                    return '1^done,bkpt={number="1"}'
                return "1^done"

        good_pre = lambda: {"install_sha256": dict(INSTALL_SHA256), "tasklist": "explorer.exe 4\n"}
        tmp_root = ROOT / "build" / "tmp"
        tmp_root.mkdir(parents=True, exist_ok=True)

        def launch_case(**attrs):
            work = Path(tempfile.mkdtemp(dir=tmp_root, prefix="selftest-"))
            try:
                for k, v in attrs.items():
                    setattr(Rec, k, v)
                a = parse_args(["--launch", "--output", str(work / "out"), "--boot-timeout", "1"])
                a.gdb = Path(sys.executable)   # exists; never executed (stub MI)
                import contextlib, io
                with contextlib.redirect_stdout(io.StringIO()):
                    rc = run_launch(a, preflight=good_pre, mi_factory=lambda g, o: launch_mi_class(Rec)(g, 0, o))
                return rc, json.loads((work / "out" / "trace-result.json").read_text()), list(Rec.sequence), a
            finally:
                shutil.rmtree(work, ignore_errors=True)
                Rec.script_hits, Rec.marker = 2, True

        rc, rep, seq, a = launch_case()
        check("launch default --stop-after is 2", a.stop_after == 2)
        check("attach default --stop-after stays 4", parse_args(["--pid", "1", "--output", "x"]).stop_after == 4)
        check("launch stub: two hits reported", [h["matched_entry"] for h in rep.get("hits", [])] == ["TEXTURES\\^FONT.BMP", "TEXTURES\\^FONT.ALF"])
        check("launch stub: completed, deleted before detach, clean cleanup",
              rc == 0 and rep["completed"] and rep["breakpoints_deleted_before_detach"] and rep["debugger_attached_after_cleanup"] is False)
        order = [i for i, t in enumerate(seq) if t.startswith("-file-exec-and-symbols")], \
                [i for i, t in enumerate(seq) if "starti" in t], \
                [i for i, t in enumerate(seq) if t.startswith("-break-insert")], \
                [i for i, t in enumerate(seq) if t.startswith("-exec-continue")], \
                [i for i, t in enumerate(seq) if t == "-break-delete"], \
                [i for i, t in enumerate(seq) if t == "-target-detach"]
        flat = [x[0] for x in order if x]
        check("launch stub: every step present once and in order", len(flat) == 6 and flat == sorted(flat))
        acts = breakpoint_actions(2, "C:/x/out")
        check("dump path carries the output dir", any(x.startswith("dump binary memory C:/x/out/record-last.bin") for x in acts))
        rc, rep, seq, a = launch_case(script_hits=1, marker=False)
        check("timeout: nonzero rc, error recorded, the fired hit still in the report",
              rc == 1 and "wait_error" in rep and len(rep.get("hits", [])) == 1 and rep["completed"] is False)
        check("timeout: breakpoint still deleted before detach by cleanup",
              rep.get("breakpoints_deleted_before_detach") is True and rep["debugger_attached_after_cleanup"] is False)
        print("RECORDED_SEQUENCE_BEGIN")
        rc, rep, seq, a = launch_case()
        for t in seq:
            print("  " + t[:230])
        print("RECORDED_SEQUENCE_END")
    print("selftest:", "FAILED " + ", ".join(failures) if failures else "all passed (synthetic data only)")
    return 1 if failures else 0


if __name__ == "__main__":
    ns = parse_args()
    raise SystemExit(selftest() if ns.selftest else run_live(ns))
