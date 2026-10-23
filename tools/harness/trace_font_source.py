"""Expose the source of the `^Font` texture read with TWO hardware execute breakpoints.

READ-only GDB trace. It needs an explicit PID (--pid) or launches the stock exe itself
(--launch), a fresh output directory below build/tmp, sends no input and writes no
process memory except the single int3 GDB itself plants and restores (see --entry-stop).

The file open routine is `0x0047E40D` (reached at `0x0045019E` with `push 0; push 1`).
Its two arms are armed together (B1s decision table, READ):

    0x0047E4AD  archive hit       request path  (char*)(*(u32*)(ebp-0x128) + 0x14)
                                  matched entry (char*)0x5D5734, record [ebp-0x18]
                                  archive       manager 0x5D5608: list [0x5D5614][ [0x5D5730] ] + 0x38
                                  size / offset rec+0x104 -> this+0x10, rec+0x108 -> this+0x124
    0x0047E463  loose / fallback  request path  (char*)(*(u32*)(ebp-0x128) + 0x14), the file is loose

Neither breakpoint carries a GDB condition. The name filter runs inside the command list,
so each breakpoint's own hit count is the number of ARRIVALS and a name mismatch (B1t cause 4)
is told apart from "never reached". The first FONT_SEEN names per arm are printed.

    python tools/harness/trace_font_source.py --pid N --output build/tmp/<new dir>     (archive arm only)
    python tools/harness/trace_font_source.py --launch --output build/tmp/<new dir>    (both arms)
    python tools/harness/trace_font_source.py --selftest        (synthetic data only)

`--launch` spends ONE start with no retry, so every guard runs before the exe is created.
GDB `starti` stops in SysWOW64 ntdll before any user code. A throwaway-exe test (b1u) showed
hardware breakpoints set at that stop do NOT reach the thread that is stopped there (the main
thread) though they do reach threads created later. So the launch flow runs to the exe entry
point `0x0049D518` on a one-shot SOFTWARE breakpoint (`--entry-stop soft`, default), arms both
hardware breakpoints in that main thread before any startup code, lists them (type
`hw breakpoint` required) and only then continues. Hit counts are read before `-break-delete`,
also on the timeout path. Run from a session-1 worker (docs/agents/MOUV_SSH.md); never in --selftest.
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

ARM_ARCHIVE = 0x0047E4AD   # archive-hit arm of 0x47E40D
ARM_LOOSE = 0x0047E463     # loose / fallback arm of 0x47E40D
ARMS = (ARM_ARCHIVE, ARM_LOOSE)
BP_ADDRESS = ARM_ARCHIVE
# mov edx,[ebp-0x128]; mov eax,[ebp-0x18]
SITE_BYTES = bytes.fromhex("8b95d8feffff8b45e8")
# mov eax,[ebp-0x128]; mov byte [eax+0x11d],0   (read 2026-10-23, b1u)
LOOSE_SITE_BYTES = bytes.fromhex("8b85d8feffffc6801d01000000")
SITE_BY_ARM = {ARM_ARCHIVE: SITE_BYTES, ARM_LOOSE: LOOSE_SITE_BYTES}
# Exe entry point (gdb `info files`, b1u) and its first bytes (read_steppers 0x49D518):
# push ebp; mov ebp,esp; push -1; push 0x4b3fe8
ENTRY_POINT = 0x0049D518
ENTRY_BYTES = bytes.fromhex("558bec6aff68e83f4b00")
ARCHIVE_ONLY_ROUTE = 0x0047E598   # taken when byte [0x5D5604] != 0; reaches NEITHER arm (b1u read)
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
SEEN_MARKER = "FONT_SEEN"
STOP_MARKER = "FONT_PROBE_STOP"
SEEN_CAP = 40              # FONT_SEEN lines printed per arm; arrivals past it are only counted
LIVE_CHECK_ADDRESS = 0x0049D51B   # entry+3, the instruction after `mov ebp,esp`; executes at once on resume
REQ_EXPR = "(char*)(*(unsigned*)($ebp-0x128)+0x14)"      # request name, `this+0x14`, both arms
REQ_BASE = "(*(unsigned*)($ebp-0x128)+0x14)"


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


def ci_condition(prefix: bytes = ENTRY_PREFIX, base: str = f"{ENTRY_NAME:#x}") -> str:
    """Case-insensitive variant of prefix_condition over an address EXPRESSION (a hex literal or a frame expression).

    Letter bytes are compared with bit 0x20 forced on both sides; `\\` and `^` are compared exactly.
    The request name keeps the caller's case, the archive entry name is upper-cased by 0x47F759.
    """
    if len(prefix) != 14:
        raise ValueError("condition is built for a 14 byte prefix")
    words = ((0, "unsigned", 4), (4, "unsigned", 4), (8, "unsigned short", 2), (10, "unsigned", 4))
    parts = []
    for off, ctype, size in words:
        want = int.from_bytes(prefix[off:off + size], "little")
        mask = 0
        for i in range(size):
            if chr(prefix[off + i]).isalpha():
                mask |= 0x20 << (8 * i)
        read = f"*({ctype}*)(({base})+{off})"
        parts.append(f"({read} | {mask:#x}) == {want | mask:#x}" if mask else f"{read} == {want:#x}")
    return " && ".join(parts)


def ci_matches(data: bytes, prefix: bytes = ENTRY_PREFIX) -> bool:
    """Python mirror of ci_condition over the bytes at the address."""
    if len(data) < len(prefix):
        return False
    return all((a | 0x20) == (b | 0x20) if chr(b).isalpha() else a == b for a, b in zip(data, prefix))


ARCHIVE_DETAIL = [
    "set $rec=*(unsigned*)($ebp-0x18)",
    "set $obj=*(unsigned*)($ebp-0x128)",
    "set $aidx=*(unsigned*)$rec",
    "set $arch=*(unsigned*)(*(unsigned*)(%#x)+$aidx*4)" % (MANAGER + 0xC),
    f'printf "{HIT_MARKER} n=%d arm=archive this=%#x rec=%#x archidx=%u archobj=%#x size=%u offset=%u gate=%u req=%s entry=%s archive=%s\\n",'
    "$hits,$obj,$rec,$aidx,$arch,*(unsigned*)($rec+0x104),*(unsigned*)($rec+0x108),"
    f"*(unsigned*){GATE:#x},{REQ_EXPR},(char*)($rec+4),(char*)($arch+0x38)",
    "dump binary memory {dump_dir}/record-last.bin $rec $rec+0x10c",
]
LOOSE_DETAIL = [
    "set $obj=*(unsigned*)($ebp-0x128)",
    f'printf "{HIT_MARKER} n=%d arm=loose this=%#x req=%s\\n",$hits,$obj,{REQ_EXPR}',
]
# arm address -> what is armed there. Tests substitute a throwaway exe's addresses here.
ARM_SPECS = {
    ARM_ARCHIVE: {"label": "archive", "cond": ci_condition(ENTRY_PREFIX, f"{ENTRY_NAME:#x}"), "detail": ARCHIVE_DETAIL},
    ARM_LOOSE: {"label": "loose", "cond": ci_condition(ENTRY_PREFIX, REQ_BASE), "detail": LOOSE_DETAIL},
}


def arm_actions(address: int, stop_after: int, dump_dir: str = ".", specs: dict | None = None) -> list[str]:
    """Command list for one arm. NO GDB condition: every arrival bumps `$arr_<label>`, the first
    SEEN_CAP are printed, and the name filter runs here, so a name mismatch (B1t cause 4) shows as
    arrivals without hits. `dump_dir` must be absolute in launch mode: GDB's cwd there is the install directory."""
    spec = (specs or ARM_SPECS)[address]
    arr = f"$arr_{spec['label']}"
    req = spec.get("req", REQ_EXPR)
    return [
        "silent",
        f"set {arr}={arr}+1",
        f"if {arr} <= {SEEN_CAP}",
        f'printf "{SEEN_MARKER} arm={spec["label"]} n=%d req=%s\\n",{arr},{req}',
        "end",
        f"if {spec['cond']}",
        "set $hits=$hits+1",
        *[line.replace("{dump_dir}", dump_dir) for line in spec["detail"]],
        f"if $hits >= {stop_after}",
        f'printf "{STOP_MARKER}\\n"',
        "else",
        "continue",
        "end",
        "else",
        "continue",
        "end",
    ]


HIT_RE = re.compile(
    rf"{HIT_MARKER} n=(\d+) arm=archive this=(0x[0-9a-f]+) rec=(0x[0-9a-f]+) archidx=(\d+) archobj=(0x[0-9a-f]+) "
    r"size=(\d+) offset=(\d+) gate=(\d+) req=(.*?) entry=(.*?) archive=(.*)$")
LOOSE_RE = re.compile(rf"{HIT_MARKER} n=(\d+) arm=loose this=(0x[0-9a-f]+) req=(.*)$")
SEEN_RE = re.compile(rf"{SEEN_MARKER} arm=(\w+) n=(\d+) req=(.*)$")


def parse_hit(line: str) -> dict | None:
    m = HIT_RE.search(line)
    if m:
        n, this, rec, aidx, aobj, ord_, f108, gate, req, entry, archive = m.groups()
        return {"n": int(n), "arm": "archive", "this": this, "record": rec, "archive_index": int(aidx),
                "archive_object": aobj, "size_field_0x104": int(ord_), "offset_field_0x108": int(f108),
                "gate": int(gate), "request_path": req, "matched_entry": entry, "archive_path": archive}
    m = LOOSE_RE.search(line)
    if m:
        n, this, req = m.groups()
        return {"n": int(n), "arm": "loose", "this": this, "request_path": req}
    return None


def parse_seen(line: str) -> dict | None:
    m = SEEN_RE.search(line)
    return {"arm": m.group(1), "n": int(m.group(2)), "request_path": m.group(3)} if m else None


def parse_break_list(text: str) -> list[dict]:
    """Breakpoints from a `-break-list` result: number, type, enabled, address and hit count (`times`)."""
    out = []
    for chunk in re.findall(r"bkpt=\{(.*?)\}(?=,bkpt=|\]|$)", text):
        def field(name):
            m = re.search(rf'(?:^|,){name}="([^"]*)"', chunk)
            return m.group(1) if m else None
        addr = field("addr")
        times = field("times")
        out.append({"number": field("number"), "type": field("type"), "enabled": field("enabled"),
                    "address": int(addr, 16) if addr and addr.startswith("0x") else None,
                    "times": int(times) if times and times.isdigit() else None})
    return out


def live_problems(bkpts: list[dict], arms=None) -> list[str]:
    """Empty when exactly the arms are present as enabled hardware breakpoints that have not fired yet."""
    arms = list(ARMS if arms is None else arms)
    problems = []
    for a in arms:
        match = [b for b in bkpts if b["address"] == a]
        if len(match) != 1:
            problems.append(f"{a:#x}: {len(match)} breakpoints listed")
            continue
        b = match[0]
        if b["type"] != "hw breakpoint":
            problems.append(f"{a:#x}: type {b['type']!r}, not 'hw breakpoint'")
        if b["enabled"] != "y":
            problems.append(f"{a:#x}: not enabled")
        if b["times"] not in (0, None):
            problems.append(f"{a:#x}: already hit {b['times']} times before the game resumed")
    stray = [b for b in bkpts if b["address"] not in arms]
    if stray:
        problems.append(f"{len(stray)} unexpected breakpoint(s) listed: {[hex(b['address'] or 0) for b in stray]}")
    return problems


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


def check_site(site: bytes, address: int = ARM_ARCHIVE, want: bytes | None = None) -> None:
    want = SITE_BY_ARM[address] if want is None else want
    if site != want:
        raise RuntimeError(f"bytes at {address:#x} are {site.hex()}, expected {want.hex()}")


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
    if args.launch and not 0 < args.live_timeout <= 120:
        raise SystemExit("--live-timeout must be in (0, 120]")


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
    bad = [f"{a:#x} is {c['site_bytes'].get(a, b'').hex()}" for a, want in c["want_sites"].items()
           if c["site_bytes"].get(a) != want]
    return f"arm site bytes differ: {bad}" if bad else None


def _g_entry(c):
    return None if c["entry_bytes"] == c["want_entry"] else f"entry bytes are {c['entry_bytes'].hex()}"


LAUNCH_GUARDS = {"install": _g_install, "no_game": _g_no_game, "first_start": _g_first_start}
ENTRY_STOP_GUARDS = {"site": _g_site, "entry": _g_entry}


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
                   help="launch mode: GDB starts the stock exe and arms both breakpoints in the main thread before any exe code (ONE start, no retry)")
    p.add_argument("--boot-timeout", type=float, default=90.0,
                   help="launch mode: seconds to wait for the entry stop and for the first hit")
    p.add_argument("--output", type=Path, help="new directory under build/tmp")
    p.add_argument("--stop-after", type=int, default=None,
                   help="stop after this many matching hits over both arms (default: 2 with --launch, the expected ^FONT.BMP + .ALF pair; 4 with --pid)")
    p.add_argument("--timeout", type=float, default=120.0, help="seconds to wait for the stop marker")
    p.add_argument("--ui-state", type=int, default=1,
                   help="required lrstate.ui_state (1 = paused level, the precedent guard); -1 skips the guard")
    p.add_argument("--entry-stop", choices=("soft", "starti"), default="soft",
                   help="launch mode: where the hardware breakpoints are armed. soft (default) = main thread at the exe entry point "
                        "0x49D518 on a one-shot software breakpoint; starti = the ntdll starti stop (B1t; unproven to reach the main thread)")
    p.add_argument("--liveness", choices=("check", "off"), default="check",
                   help="launch mode, soft entry stop: fire a throwaway hardware breakpoint at entry+3 in the main thread before arming")
    p.add_argument("--live-timeout", type=float, default=15.0, help="launch mode: seconds the liveness breakpoint may take to fire")
    p.add_argument("--gdb", type=Path, default=Path(r"C:\progs\msys2\mingw32\bin\gdb.exe"))
    p.add_argument("--selftest", action="store_true", help="exercise argument/guard logic on synthetic data only")
    args = p.parse_args(argv)
    if args.stop_after is None:
        args.stop_after = 2 if args.launch else 4
    validate_args(args)
    return args


def stock_target() -> dict:
    """Everything a launch needs to know about the program it starts. Tests substitute a throwaway exe."""
    return {"exe": STOCK_EXE, "cwd": INSTALL_DIR, "arms": ARMS, "sites": SITE_BY_ARM, "entry": ENTRY_POINT,
            "entry_bytes": ENTRY_BYTES, "live": LIVE_CHECK_ADDRESS, "specs": ARM_SPECS, "count_tail": COUNT_TAIL_STOCK}


def stop_address(record: str) -> int | None:
    m = re.search(r'frame=\{[^}]*?addr="(0x[0-9a-f]+)"', record)
    return int(m.group(1), 16) if m else None


def stop_thread(record: str) -> str | None:
    m = re.search(r'thread-id="(\w+)"', record)
    return m.group(1) if m else None


def with_counts(base):
    """Adds hit-count reads that happen BEFORE any breakpoint is deleted, also when the run is cut short."""

    class CountingMI(base):
        count_tail = None   # set by the launcher: (printf format tail, printf argument tail) for the target

        def read_breakpoints(self) -> list[dict]:
            return parse_break_list(self.command("-break-list", timeout=10))

        def cleanup(self, report: dict) -> None:
            if self.attached and any(x.startswith("=thread-group-exited") or '*stopped,reason="exited' in x for x in self.lines):
                self.attached = False        # nothing left to interrupt, delete from or detach
                report["inferior_exited_before_cleanup"] = True
            if self.attached and self.running:
                try:
                    self.command("-exec-interrupt --all", timeout=10)
                    report["cleanup_stop_record"] = self.wait_any_stop(timeout=10)
                except Exception as exc:
                    report["cleanup_interrupt_error"] = repr(exc)
            if self.attached and not self.running and "armed_list" in report and not report.get("counts_read_before_delete"):
                try:
                    read_counts(self, report, "cleanup (run cut short)")
                except Exception as exc:
                    report["counts_error"] = repr(exc)
            super().cleanup(report)

    return CountingMI


def collect_hits(mi, report: dict) -> None:
    """Copy every hit already printed into the report; safe to call after a failed wait."""
    lines = [json.loads(x[1:]).strip() for x in mi.lines if x.startswith("~")]
    report["console"] = [x for x in lines if HIT_MARKER in x or STOP_MARKER in x]
    report["hits"] = [h for h in map(parse_hit, lines) if h]
    report["seen"] = [h for h in map(parse_seen, lines) if h]
    counters = [x for x in lines if x.startswith("FONT_COUNTS")]
    if counters:
        report["gdb_counters"] = counters[-1]


def arm_both(mi, args, report: dict, dump_dir: str, target: dict) -> None:
    """Insert one unconditioned hardware breakpoint per arm and prove both are listed live, before any resume."""
    mi.console("set $hits=0")
    for label in {s["label"] for s in target["specs"].values()}:
        mi.console(f"set $arr_{label}=0")
    for address in target["arms"]:
        # No GDB condition: the name filter is in the command list so arrivals are counted either way.
        mi.breakpoint(address, None, arm_actions(address, args.stop_after, dump_dir, target["specs"]))
    report["breakpoints_armed"] = [hex(a) for a in target["arms"]]
    listed = mi.read_breakpoints()
    report["armed_list"] = listed
    report["armed_problems"] = live_problems(listed, target["arms"])
    if report["armed_problems"]:
        raise RuntimeError("breakpoints not live: " + "; ".join(report["armed_problems"]))


COUNT_TAIL_STOCK = (" flag5604=%d gate5610=%u", f",*(unsigned char*){ARCHIVE_ONLY_FLAG:#x},*(unsigned*){GATE:#x}")


def read_counts(mi, report: dict, where: str) -> None:
    """Per-arm arrival counts (GDB convenience variables) and each breakpoint's own hit count, while stopped."""
    fmt_tail, arg_tail = getattr(mi, "count_tail", None) or COUNT_TAIL_STOCK
    mi.console(f'printf "FONT_COUNTS archive=%d loose=%d hits=%d{fmt_tail}\\n",$arr_archive,$arr_loose,$hits{arg_tail}')
    collect_hits(mi, report)
    report["breakpoint_counts"] = mi.read_breakpoints()
    report["counts_read_before_delete"] = True
    report["counts_read_at"] = where


def read_counts_and_delete(mi, report: dict) -> None:
    """Hit counts first, then delete while stopped, then prove the debug registers are empty before detach."""
    read_counts(mi, report, "stop marker")
    mi.command("-break-delete", timeout=10)
    # GDB 17.2 words the empty list "No breakpoints, watchpoints, tracepoints, or catchpoints."; the
    # old "No breakpoints or watchpoints" check would have failed a clean run (found by the b1u throwaway test).
    report["breakpoints_after_delete"] = mi.read_breakpoints()
    report["breakpoints_deleted_before_detach"] = report["breakpoints_after_delete"] == []
    if not report["breakpoints_deleted_before_detach"]:
        raise RuntimeError("breakpoints still listed before detach")


def arm_and_collect(mi, args, report: dict, timeout: float, dump_dir: str = ".", target: dict | None = None) -> None:
    """Arm both arms, run to the stop marker, read the counts, delete them while stopped."""
    target = target or stock_target()
    arm_both(mi, args, report, dump_dir, target)
    while not mi.messages.empty():     # a stale stop record must not be read as an unexpected stop
        mi.messages.get_nowait()
    mi.resume()
    try:
        mi.wait_marker(STOP_MARKER, timeout=timeout)
    except BaseException as exc:
        # A timeout or an unexpected stop must still report the hits that fired (gdb-mi.log has them too).
        report["wait_error"] = repr(exc)
        collect_hits(mi, report)
        raise
    read_counts_and_delete(mi, report)
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
    report: dict = {"pid": args.pid, "breakpoints": [hex(a) for a in ARMS], "completed": False,
                    "scope": "READ-only; two hardware execute breakpoints, the arms of 0x47E40D"}
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
        sites = {a: read_mem(handle, a, len(SITE_BY_ARM[a])) or b"" for a in ARMS}
        failed = run_guards({"site": _g_site}, {"site_bytes": sites, "want_sites": SITE_BY_ARM})
        if failed:
            raise RuntimeError("; ".join(failed))
        report["pre_arm"] = {"site_bytes": {hex(a): b.hex() for a, b in sites.items()},
                             "gate_0x5d5610": u32(handle, GATE),
                             "manager_count_0x5d5608": u32(handle, MANAGER),
                             "archive_only_flag_0x5d5604": read_mem(handle, ARCHIVE_ONLY_FLAG, 1)[0]}
        mi = with_counts(MI)(args.gdb, args.pid, output)
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


def launch_mi_class(base, target: dict | None = None):
    """GDB/MI session class that starts the target exe instead of attaching. `base` is MI (or a recorder)."""
    target = target or stock_target()

    class LaunchMI(with_counts(base)):
        count_tail = target["count_tail"]

        def start(self) -> None:
            for setting in ("pagination off", "confirm off", "auto-solib-add off", "mi-async on"):
                self.command("-gdb-set " + setting)
            self.command("-environment-cd " + json.dumps(Path(target["cwd"]).as_posix()))  # the game expects its own directory
            self.command("-file-exec-and-symbols " + json.dumps(Path(target["exe"]).as_posix()))

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


def make_launch_mi(gdb: Path, output: Path, target: dict | None = None):
    sys.path.insert(0, str(ROOT / "tools" / "harness"))
    from trace_cell_emission import MI
    return launch_mi_class(MI, target)(gdb, 0, output)


def run_to_stop(mi, command: str, timeout: float) -> str:
    """Issue a resume-type command and return the first `*stopped` record that follows it."""
    mark = len(mi.lines)
    mi.command(command, timeout=10)
    mi.running = True
    return mi.wait_any_stop(timeout=timeout, since=mark)


def enter_main_thread(mi, args, report: dict, target: dict) -> None:
    """Stop the MAIN thread at the exe entry point on a one-shot software breakpoint, so the
    hardware breakpoints armed next are applied to it (a throwaway test, b1u: hardware
    breakpoints set at the `starti` stop never reached the main thread; threads created
    later got them). Then prove the debug registers work in that thread, before arming."""
    entry, live = target["entry"], target["live"]
    mi.command(f"-break-insert -t *{entry:#x}")
    record = run_to_stop(mi, "-exec-continue", args.boot_timeout)
    report["entry_main_stop"] = {"record": record, "address": stop_address(record), "thread": stop_thread(record)}
    if stop_address(record) != entry or 'reason="breakpoint-hit"' not in record:
        raise RuntimeError("did not stop at the exe entry point: " + record[:200])
    if args.liveness == "check":
        number = re.search(r'number="(\d+)"', mi.command(f"-break-insert -h *{live:#x}")).group(1)
        try:
            record = run_to_stop(mi, "-exec-continue", args.live_timeout)
        except TimeoutError:
            report["liveness"] = {"address": hex(live), "hit": False}
            raise RuntimeError(f"hardware breakpoint at {live:#x} did not fire within {args.live_timeout}s: "
                               "debug registers are not live in the main thread; arms NOT inserted")
        listed = [b for b in mi.read_breakpoints() if b["address"] == live]
        report["liveness"] = {"address": hex(live), "hit": stop_address(record) == live,
                              "thread": stop_thread(record), "type": listed[0]["type"] if listed else None,
                              "times": listed[0]["times"] if listed else None, "record": record}
        if stop_address(record) != live or not listed or listed[0]["type"] != "hw breakpoint" or listed[0]["times"] != 1:
            raise RuntimeError("liveness check did not stop at the hardware breakpoint: " + record[:200])
        mi.command(f"-break-delete {number}", timeout=10)
    else:
        report["liveness"] = {"skipped": True}


def run_launch(args: argparse.Namespace, preflight=launch_preflight, mi_factory=make_launch_mi,
               target: dict | None = None) -> int:
    """Opt-in arming mode: GDB creates the stock process and stops it, both breakpoints are
    inserted in its MAIN thread before the exe's first instruction, then the game runs. ONE start, no retry.

    The `--entry-stop soft` option plants GDB's one-shot int3 at the exe entry point; GDB removes
    it when that stop is reported. It is a debugger probe beyond the two hardware breakpoints and
    must be authorised by whoever dispatches the start.
    """
    target = target or stock_target()
    output = check_output_dir(args.output, ROOT / "build" / "tmp")
    output.mkdir(parents=True, exist_ok=False)
    report: dict = {"mode": "launch", "breakpoints": [hex(a) for a in target["arms"]], "completed": False,
                    "launch_attempted": False, "entry_stop_mode": args.entry_stop, "liveness_mode": args.liveness,
                    "scope": "one GDB-created stock process; two hardware execute breakpoints; no input, no memory writes"
                             + ("; one one-shot software breakpoint at the exe entry point" if args.entry_stop == "soft" else "")}
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
        mi = mi_factory(args.gdb, output) if mi_factory is not make_launch_mi else make_launch_mi(args.gdb, output, target)
        mi.start()
        report["launch_attempted"] = True   # recorded before the start; a failure below is final
        mark = len(mi.lines)
        mi.console("starti")
        mi.attached = True                  # from here cleanup must delete the breakpoints and detach
        report["debugger_attached"] = True
        report["entry_stop"] = mi.wait_any_stop(timeout=args.boot_timeout, since=mark)
        sites = {}
        for a in target["arms"]:
            sites[a] = parse_memory_bytes(mi.command("-data-read-memory-bytes %#x %d" % (a, len(target["sites"][a]))))
        entry_bytes = parse_memory_bytes(mi.command("-data-read-memory-bytes %#x %d" % (target["entry"], len(target["entry_bytes"]))))
        failed = run_guards(ENTRY_STOP_GUARDS, {"site_bytes": sites, "want_sites": target["sites"],
                                                "entry_bytes": entry_bytes, "want_entry": target["entry_bytes"]})
        if failed:
            raise RuntimeError("; ".join(failed))
        report["entry_stop_site_bytes"] = {hex(a): b.hex() for a, b in sites.items()}
        report["entry_point_bytes"] = entry_bytes.hex()
        while not mi.messages.empty():     # a stale entry-stop record must not be read as an unexpected stop
            mi.messages.get_nowait()
        pids = re.findall(r'pid="(\d+)"', mi.command("-list-thread-groups"))
        report["pid"] = int(pids[0]) if pids else None
        if args.entry_stop == "soft":
            enter_main_thread(mi, args, report, target)
        arm_and_collect(mi, args, report, timeout=args.boot_timeout, dump_dir=output.as_posix(), target=target)
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
    # site-bytes guard (attach mode), both arms
    for arm in ARMS:
        check_site(SITE_BY_ARM[arm], arm)
        try:
            check_site(bytes([SITE_BY_ARM[arm][0] ^ 1]) + SITE_BY_ARM[arm][1:], arm)
            check(f"site guard rejects changed byte at {arm:#x}", False)
        except RuntimeError:
            check(f"site guard rejects changed byte at {arm:#x}", True)
    # name filter: generated GDB text vs Python mirror, case-insensitive, letters only
    ci = ci_condition(ENTRY_PREFIX, f"{ENTRY_NAME:#x}")
    check("ci condition has OR-0x20 constants", "0x74786574" in ci and "0x20202020" in ci and "0x5e5c" in ci)

    def eval_cond(cond, memory):
        """Evaluate a hex-literal-based condition over {address: byte}. Synthetic data only."""
        def rd(m):
            kind, base, off = m.groups()
            addr, n = int(base, 16) + int(off), 4 if kind == "unsigned" else 2
            return str(int.from_bytes(bytes(memory.get(addr + i, 0) for i in range(n)), "little"))
        return eval(re.sub(r"\*\((unsigned short|unsigned)\*\)\(\((0x[0-9a-f]+)\)\+(\d+)\)", rd, cond).replace("&&", " and "))

    for name, want in ((b"TEXTURES\\^FONT.BMP\0", True), (b"Textures\\^Font.bmp\0", True), (b"textures\\^font.alf\0", True),
                       (b"TEXTURES\\^FONG.BMP\0", False), (b"TEXTURES/^FONT.BMP\0", False), (b"TEXTURES\\FONT.BMP\0", False)):
        memory = {ENTRY_NAME + i: b for i, b in enumerate(name)}
        check(f"ci condition text == mirror for {name[:19]!r}", eval_cond(ci, memory) == ci_matches(name) == want)
    # hit and seen line parsers
    line = (f"{HIT_MARKER} n=1 arm=archive this=0x19f9a4 rec=0x5d5730 archidx=1 archobj=0x2a3b40 size=1234 offset=5678 gate=1 "
            "req=Textures\\^Font.bmp entry=TEXTURES\\^FONT.BMP archive=C:\\progs\\Lemmings Revolution\\lemmings.box")
    h = parse_hit(line)
    check("parse_hit archive fields", bool(h) and h["arm"] == "archive" and h["size_field_0x104"] == 1234 and h["offset_field_0x108"] == 5678
          and h["archive_path"].endswith("lemmings.box") and h["matched_entry"] == "TEXTURES\\^FONT.BMP" and h["gate"] == 1)
    h = parse_hit(f"{HIT_MARKER} n=2 arm=loose this=0x19f9a4 req=Textures\\^Font.alf")
    check("parse_hit loose fields", bool(h) and h["arm"] == "loose" and h["request_path"] == "Textures\\^Font.alf")
    check("parse_hit rejects noise", parse_hit("TARGET_ENTER quad=0x1") is None)
    s = parse_seen(f"{SEEN_MARKER} arm=loose n=7 req=Data\\x.bin")
    check("parse_seen", bool(s) and s["arm"] == "loose" and s["n"] == 7 and s["request_path"] == "Data\\x.bin")
    # per-arm action lists: no GDB condition anywhere, arrivals counted before the filter
    for arm in ARMS:
        acts = arm_actions(arm, 2, "C:/x/out")
        label = ARM_SPECS[arm]["label"]
        check(f"arm {arm:#x} actions count arrivals before the filter",
              acts.index(f"set $arr_{label}=$arr_{label}+1") < acts.index(f"if {ARM_SPECS[arm]['cond']}"))
        check(f"arm {arm:#x} actions carry stop marker and continue", any(STOP_MARKER in x for x in acts) and acts.count("continue") == 2)
    check("dump path carries the output dir", any(x.startswith("dump binary memory C:/x/out/record-last.bin") for x in arm_actions(ARM_ARCHIVE, 2, "C:/x/out")))
    check("loose arm writes no dump", not any(x.startswith("dump") for x in arm_actions(ARM_LOOSE, 2, "C:/x/out")))
    # -break-list parser and the liveness rules
    bl = ('1^done,BreakpointTable={nr_rows="2",nr_cols="6",hdr=[{width="7",alignment="-1",col_name="number",colhdr="Num"}],body=['
          'bkpt={number="2",type="hw breakpoint",disp="keep",enabled="y",addr="0x0047e4ad",times="0",original-location="*0x47e4ad"},'
          'bkpt={number="3",type="hw breakpoint",disp="keep",enabled="y",addr="0x0047e463",times="3",original-location="*0x47e463"}]}')
    parsed = parse_break_list(bl)
    check("parse_break_list reads type, address and times",
          [(b["number"], b["type"], b["address"], b["times"]) for b in parsed] == [("2", "hw breakpoint", 0x47E4AD, 0), ("3", "hw breakpoint", 0x47E463, 3)])
    lit = (0x47E4AD, 0x47E463)   # literals on purpose: fixtures must not move with the ARM_* constants
    ok_list = [dict(b, times=0) for b in parsed]
    check("live_problems accepts two fresh hw breakpoints", live_problems(ok_list, lit) == [])
    check("live_problems rejects a software breakpoint", len(live_problems([dict(ok_list[0], type="breakpoint"), ok_list[1]], lit)) == 1)
    check("live_problems rejects a missing arm", len(live_problems(ok_list[:1], lit)) == 1)
    check("live_problems rejects a third breakpoint", len(live_problems(ok_list + [dict(ok_list[0], number="9", address=0x1000)], lit)) == 1)
    check("live_problems rejects a pre-fired arm", len(live_problems([ok_list[0], dict(ok_list[1], times=1)], lit)) == 1)

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
    check("live timeout range rejected", rejects(["--launch", "--output", "x", "--live-timeout", "0"]))
    check("selftest never needs a mode", not rejects(["--selftest"]))
    la = parse_args(["--launch", "--output", "x"])
    check("launch defaults: soft entry stop, liveness check on", la.entry_stop == "soft" and la.liveness == "check")
    good = {"install_sha256": dict(INSTALL_SHA256), "tasklist": "System Idle Process  0\nexplorer.exe  4\n",
            "launch_attempted": False}
    check("clean preflight passes every launch guard", run_guards(LAUNCH_GUARDS, good) == [])
    entry_ctx = {"site_bytes": dict(SITE_BY_ARM), "want_sites": SITE_BY_ARM, "entry_bytes": ENTRY_BYTES, "want_entry": ENTRY_BYTES}
    check("clean entry stop passes site and entry guards", run_guards(ENTRY_STOP_GUARDS, entry_ctx) == [])
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
    for arm in ARMS:
        bad_sites = {**SITE_BY_ARM, arm: bytes([SITE_BY_ARM[arm][0] ^ 1]) + SITE_BY_ARM[arm][1:]}
        got = run_guards(ENTRY_STOP_GUARDS, dict(entry_ctx, site_bytes=bad_sites))
        check(f"guard site rejects changed bytes at {arm:#x}", len(got) == 1 and got[0].startswith("site"))
    got = run_guards(ENTRY_STOP_GUARDS, dict(entry_ctx, entry_bytes=bytes([ENTRY_BYTES[0] ^ 1]) + ENTRY_BYTES[1:]))
    check("guard entry rejects changed entry-point bytes", len(got) == 1 and got[0].startswith("entry"))
    check("MUTATION without guards site/entry the changed bytes are accepted",
          run_guards({}, dict(entry_ctx, site_bytes={}, entry_bytes=b"x")) == [])
    check("parse_break_list: GDB 17.2 empty table is empty", parse_break_list('28^done,BreakpointTable={nr_rows="0",nr_cols="6",hdr=[{width="7"}],body=[]}') == [])
    check("parse_memory_bytes", parse_memory_bytes('1^done,memory=[{begin="0x0047e4ad",contents="8b95d8feffff8b45e8"}]') == SITE_BYTES)
    check("stop_address / stop_thread", stop_address('*stopped,reason="breakpoint-hit",frame={addr="0x0049d518",func="x"},thread-id="1"') == 0x49D518
          and stop_thread('*stopped,thread-id="3"') == "3")
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

        class Rec(MI):
            """Recording stand-in for the GDB/MI session. Class attributes inject faults."""
            target: dict = {}
            script_hits = 2            # FONT_HIT lines the run phase prints, alternating archive / loose
            marker = True              # print the stop marker at the end of the run phase
            seen_only = False          # name mismatch: arrivals but no hit
            live_fires = True          # the liveness breakpoint fires
            arm_type = "hw breakpoint"
            entry_addr = None          # override the address reported by the entry stop
            sequence: list = []

            def __init__(self, gdb, pid, output):
                self.messages = queue.Queue(); self.lines = []; self.token = 0; self.attached = False
                self.running = False; self.bp_numbers = []; self.output = output; self.pid = pid
                self.proc = type("P", (), {"poll": lambda s: None if not self.exited else 0,
                                           "wait": lambda s, timeout=None: 0})()
                self.exited = False
                self.bps: list = []
                self.next_number = 1
                self.arrivals = {"archive": 0, "loose": 0}
                self.hits = 0
                Rec.sequence = []

            def emit(self, line):
                self.lines.append(line); self.messages.put(line)

            def bp_by_addr(self, addr):
                return [b for b in self.bps if b["addr"] == addr]

            def command(self, text, timeout=20):
                Rec.sequence.append(text)
                t = Rec.target
                if text == "-gdb-exit":
                    self.exited = True
                elif text.startswith("-interpreter-exec console"):
                    inner = json.loads(text.split(" ", 2)[2])
                    if inner == "starti":
                        self.emit('*stopped,reason="signal-received",signal-name="SIGTRAP"')
                    elif inner == "info breakpoints":
                        self.emit("~" + json.dumps("No breakpoints, watchpoints, tracepoints, or catchpoints.\n" if not self.bps else "Num Type\n1 hw breakpoint\n"))
                    elif inner.startswith('printf "FONT_COUNTS'):
                        self.emit("~" + json.dumps(f"FONT_COUNTS archive={self.arrivals['archive']} loose={self.arrivals['loose']} hits={self.hits} flag5604=0 gate5610=1\n"))
                elif text.startswith("-break-insert"):
                    addr = int(text.rsplit("*", 1)[1], 16)
                    temp = text.startswith("-break-insert -t")
                    kind = "breakpoint" if temp else (Rec.arm_type if addr in t["arms"] else "hw breakpoint")
                    self.bps.append({"number": str(self.next_number), "type": kind, "disp": "del" if temp else "keep",
                                     "addr": addr, "times": 0})
                    self.next_number += 1
                    return '1^done,bkpt={number="%s"}' % self.bps[-1]["number"]
                elif text == "-break-list":
                    body = ",".join('bkpt={number="%s",type="%s",disp="%s",enabled="y",addr="0x%08x",times="%d"}'
                                    % (b["number"], b["type"], b["disp"], b["addr"], b["times"]) for b in self.bps)
                    if not self.bps:
                        return '1^done,BreakpointTable={nr_rows="0",nr_cols="6",hdr=[],body=[]}'   # GDB 17.2 shape
                    return '1^done,BreakpointTable={nr_rows="%d",nr_cols="6",hdr=[],body=[%s]}' % (len(self.bps), body)
                elif text == "-break-delete":
                    self.bps = []
                elif text.startswith("-break-delete "):
                    self.bps = [b for b in self.bps if b["number"] != text.split()[1]]
                elif text.startswith("-exec-interrupt"):
                    self.emit('*stopped,reason="signal-received",signal-name="SIGINT"')
                elif text.startswith("-exec-continue"):
                    self.emit('*running,thread-id="all"')
                    temp = [b for b in self.bps if b["disp"] == "del"]
                    live = self.bp_by_addr(t["live"])
                    if temp:
                        addr = Rec.entry_addr if Rec.entry_addr is not None else temp[0]["addr"]
                        self.bps = [b for b in self.bps if b["disp"] != "del"]
                        self.emit('*stopped,reason="breakpoint-hit",disp="del",frame={addr="0x%08x"},thread-id="1"' % addr)
                    elif live:
                        if Rec.live_fires:
                            live[0]["times"] += 1
                            self.emit('*stopped,reason="breakpoint-hit",disp="keep",frame={addr="0x%08x"},thread-id="1"' % t["live"])
                    else:
                        labels = [t["specs"][a]["label"] for a in t["arms"]]
                        for i in range(1, self.script_hits + 1):
                            label = labels[(i - 1) % 2]
                            arm = t["arms"][(i - 1) % 2]
                            self.arrivals[label] += 1
                            self.bp_by_addr(arm)[0]["times"] += 1
                            self.emit("~" + json.dumps(f"{SEEN_MARKER} arm={label} n={self.arrivals[label]} req=Textures\\^Font.x\n"))
                            if not Rec.seen_only:
                                self.hits += 1
                                if label == "archive":
                                    self.emit("~" + json.dumps(f"{HIT_MARKER} n={self.hits} arm=archive this=0x19f9a4 rec=0x5d5730 archidx=1 archobj=0x2a3b40 size=10 offset=20 gate=1 "
                                                               "req=Textures\\^Font.x entry=TEXTURES\\^FONT.BMP archive=C:\\x\\lemmings.box\n"))
                                else:
                                    self.emit("~" + json.dumps(f"{HIT_MARKER} n={self.hits} arm=loose this=0x19f9a4 req=Textures\\^Font.x\n"))
                        if self.marker and not Rec.seen_only:
                            self.emit("~" + json.dumps(STOP_MARKER + "\n"))
                            self.emit('*stopped,reason="breakpoint-hit"')
                elif text.startswith("-data-read-memory-bytes"):
                    addr = int(text.split()[1], 16)
                    data = t["entry_bytes"] if addr == t["entry"] else t["sites"][addr]
                    return '1^done,memory=[{begin="0x%08x",contents="%s"}]' % (addr, data.hex())
                elif text.startswith("-list-thread-groups"):
                    return '1^done,groups=[{id="i1",type="process",pid="4242"}]'
                return "1^done"

        good_pre = lambda: {"install_sha256": dict(INSTALL_SHA256), "tasklist": "explorer.exe 4\n"}
        tmp_root = ROOT / "build" / "tmp"
        tmp_root.mkdir(parents=True, exist_ok=True)
        EXPECTED_ARMS = {0x47E4AD, 0x47E463}   # literals on purpose: the mutation below changes the constant, not this

        def launch_case(target=None, argv=(), **attrs):
            target = target or stock_target()
            work = Path(tempfile.mkdtemp(dir=tmp_root, prefix="selftest-"))
            saved = {k: getattr(Rec, k) for k in ("script_hits", "marker", "seen_only", "live_fires", "arm_type", "entry_addr")}
            try:
                Rec.target = target
                for k, v in attrs.items():
                    setattr(Rec, k, v)
                a = parse_args(["--launch", "--output", str(work / "out"), "--boot-timeout", "1", "--live-timeout", "1", *argv])
                a.gdb = Path(sys.executable)   # exists; never executed (stub MI)
                import contextlib, io
                with contextlib.redirect_stdout(io.StringIO()):
                    rc = run_launch(a, preflight=good_pre, mi_factory=lambda g, o: launch_mi_class(Rec, target)(g, 0, o), target=target)
                return {"rc": rc, "rep": json.loads((work / "out" / "trace-result.json").read_text()),
                        "seq": list(Rec.sequence), "args": a, "target": target}
            finally:
                shutil.rmtree(work, ignore_errors=True)
                for k, v in saved.items():
                    setattr(Rec, k, v)

        def inserts(seq):
            return [(t, int(t.rsplit("*", 1)[1], 16)) for t in seq if t.startswith("-break-insert")]

        def counts_before_delete(seq):
            """The counter line and a -break-list both precede the FIRST bare -break-delete."""
            first_del = next((i for i, t in enumerate(seq) if t == "-break-delete"), None)
            cnt = next((i for i, t in enumerate(seq) if "FONT_COUNTS" in t), None)
            if first_del is None or cnt is None or cnt > first_del:
                return False
            return any(t == "-break-list" for t in seq[cnt:first_del])

        def launch_assertions(res):
            """Every acceptance assertion for one stub launch. Returns [(name, ok)]."""
            rep, seq, target = res["rep"], res["seq"], res["target"]
            ins = inserts(seq)
            arm_ins = [x for x in ins if "-break-insert -h" in x[0] and x[1] != target["live"]]
            last_cont = max(i for i, t in enumerate(seq) if t.startswith("-exec-continue"))
            arm_idx = [i for i, t in enumerate(seq) if t.startswith("-break-insert -h") and int(t.rsplit("*", 1)[1], 16) in target["arms"]]
            live_stop = [i for i, t in enumerate(seq) if t.startswith("-exec-continue")][1]
            counts = {b["address"]: b["times"] for b in rep.get("breakpoint_counts", [])}
            return [
                ("(a) exactly two hardware breakpoints, at 0x47E4AD and 0x47E463",
                 len(arm_ins) == 2 and {a for _, a in arm_ins} == EXPECTED_ARMS),
                ("(a) no GDB condition on either arm", not any(t.startswith("-break-condition") for t in seq)),
                ("(a) one command list per arm", sum(1 for t in seq if t.startswith("-break-commands")) == 2),
                ("(b) liveness recorded: main thread fired a hw breakpoint before arming",
                 rep.get("liveness", {}).get("hit") is True and rep["liveness"]["type"] == "hw breakpoint" and rep["liveness"]["times"] == 1),
                ("(b) liveness stop precedes the arm inserts, which precede the run", live_stop < min(arm_idx) < max(arm_idx) < last_cont),
                ("(b) armed list read and clean before the run",
                 rep.get("armed_problems") == [] and len(rep.get("armed_list", [])) == 2
                 and any(t == "-break-list" for t in seq[max(arm_idx):last_cont])),
                ("(c) entry stop in the main thread at the entry point before the liveness check",
                 rep.get("entry_main_stop", {}).get("address") == target["entry"] and rep["entry_main_stop"]["thread"] == "1"),
                ("(d) counts read before -break-delete", counts_before_delete(seq) and rep.get("counts_read_before_delete") is True),
                ("(d) per-breakpoint hit counts in the report", set(counts) == set(target["arms"]) and all(isinstance(v, int) for v in counts.values())),
                ("completed, deleted before detach, clean cleanup",
                 res["rc"] == 0 and rep["completed"] and rep["breakpoints_deleted_before_detach"] and rep["debugger_attached_after_cleanup"] is False),
            ]

        res = launch_case()
        base = launch_assertions(res)
        for name, ok in base:
            check("launch stub " + name, ok)
        rep, seq = res["rep"], res["seq"]
        check("launch default --stop-after is 2", res["args"].stop_after == 2)
        check("attach default --stop-after stays 4", parse_args(["--pid", "1", "--output", "x"]).stop_after == 4)
        check("launch stub: one hit per arm in the report", sorted(h["arm"] for h in rep["hits"]) == ["archive", "loose"])
        check("launch stub: FONT_SEEN arrivals and the gdb counter line reported",
              len(rep["seen"]) == 2 and rep["gdb_counters"].startswith("FONT_COUNTS archive=1 loose=1 hits=2"))
        check("launch stub: bp counts equal arrivals (1 each)", all(b["times"] == 1 for b in rep["breakpoint_counts"]))
        order = [[i for i, t in enumerate(seq) if t.startswith("-file-exec-and-symbols")], [i for i, t in enumerate(seq) if "starti" in t],
                 [i for i, t in enumerate(seq) if t.startswith("-break-insert -t")], [i for i, t in enumerate(seq) if t == "-break-delete"],
                 [i for i, t in enumerate(seq) if t == "-target-detach"]]
        flat = [x[0] for x in order if x]
        check("launch stub: file, starti, entry bp, delete, detach in order, each once", len(flat) == 5 and flat == sorted(flat))
        # ---- isolated mutation: the arm address alone
        mut = stock_target()
        moved = ARM_LOOSE + 1
        mut["arms"] = (ARM_ARCHIVE, moved)
        mut["sites"] = {ARM_ARCHIVE: SITE_BYTES, moved: LOOSE_SITE_BYTES}
        mut["specs"] = {ARM_ARCHIVE: ARM_SPECS[ARM_ARCHIVE], moved: ARM_SPECS[ARM_LOOSE]}
        failed_mut = [n for n, ok in launch_assertions(launch_case(target=mut)) if not ok]
        check("MUTATION arm 0x47E463 -> 0x47E464 alone fails exactly one assertion, the arm-address one",
              failed_mut == ["(a) exactly two hardware breakpoints, at 0x47E4AD and 0x47E463"])
        # ---- isolated mutation: delete before reading the counts
        swapped = list(seq)
        i_cnt = next(i for i, t in enumerate(swapped) if "FONT_COUNTS" in t)
        i_del = next(i for i, t in enumerate(swapped) if t == "-break-delete")
        swapped[i_cnt], swapped[i_del] = swapped[i_del], swapped[i_cnt]
        check("MUTATION delete before counts is caught by the order check", counts_before_delete(seq) and not counts_before_delete(swapped))
        # ---- faults
        res = launch_case(live_fires=False)
        seq = res["seq"]
        check("liveness never fires: arms not inserted and the run never resumed past the liveness stop",
              res["rc"] == 1 and "arms NOT inserted" in res["rep"]["error"] and res["rep"]["liveness"]["hit"] is False
              and not [x for x in inserts(seq) if x[1] in ARMS] and res["rep"].get("completed") is False)
        check("liveness never fires: nothing left behind (cleanup ran, detached)", res["rep"]["debugger_attached_after_cleanup"] is False)
        res = launch_case(arm_type="breakpoint")
        check("armed breakpoint not hardware: refuses before the run",
              res["rc"] == 1 and "not live" in res["rep"]["error"] and len([t for t in res["seq"] if t.startswith("-exec-continue")]) == 2)
        res = launch_case(entry_addr=0x401000)
        check("wrong entry stop address: refuses, arms not inserted",
              res["rc"] == 1 and "entry point" in res["rep"]["error"] and not [x for x in inserts(res["seq"]) if x[1] in ARMS])
        res = launch_case(argv=("--entry-stop", "starti"))
        check("--entry-stop starti: no software breakpoint, no liveness step, arms inserted at the starti stop",
              not [t for t in res["seq"] if t.startswith("-break-insert -t")] and "liveness" not in res["rep"]
              and len([x for x in inserts(res["seq"]) if x[1] in ARMS]) == 2 and res["rc"] == 0)
        res = launch_case(argv=("--liveness", "off"))
        check("--liveness off: skip recorded, still two arms", res["rep"]["liveness"] == {"skipped": True} and res["rc"] == 0)
        res = launch_case(script_hits=1, marker=False)
        check("timeout: nonzero rc, error recorded, the fired hit still in the report",
              res["rc"] == 1 and "wait_error" in res["rep"] and len(res["rep"].get("hits", [])) == 1 and res["rep"]["completed"] is False)
        check("timeout: counts read before the cleanup delete, breakpoints deleted, detached",
              res["rep"].get("counts_read_before_delete") is True and res["rep"]["counts_read_at"].startswith("cleanup")
              and counts_before_delete(res["seq"]) and res["rep"]["breakpoints_deleted_before_detach"] is True
              and res["rep"]["debugger_attached_after_cleanup"] is False)
        res = launch_case(seen_only=True, script_hits=4)
        check("name mismatch (cause 4): arrivals are seen and counted with zero hits",
              res["rep"].get("hits") == [] and len(res["rep"]["seen"]) == 4 and all(b["times"] == 2 for b in res["rep"]["breakpoint_counts"])
              and res["rep"]["gdb_counters"].startswith("FONT_COUNTS archive=2 loose=2 hits=0"))
        print("RECORDED_SEQUENCE_BEGIN")
        for t in launch_case()["seq"]:
            print("  " + t[:230])
        print("RECORDED_SEQUENCE_END")
    print("selftest:", "FAILED " + ", ".join(failures) if failures else "all passed (synthetic data only)")
    return 1 if failures else 0


if __name__ == "__main__":
    ns = parse_args()
    raise SystemExit(selftest() if ns.selftest else run_live(ns))
