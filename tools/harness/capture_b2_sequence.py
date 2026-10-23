"""B2 frontend sequence capture: per scripted step, the construction counter and screen list chain.

WRITTEN, NOT RUN LIVE (b2d-capture-preflight, offline). `--selftest` and `--plan` touch no process.
`--run` needs the live-owner flag and one stock start granted by the facilitator.

Per step it records `[0x004C416C]` (a construction counter, not an enum; 3 menu, 4 confirm, 5 map by
PLAN recon), then walks the screen list: head `[0x005D59E4]`, next at `+4`, prev at `+8`, first dword of
each node = vtable (the menu is vtable `0x004B2528`, chrome `0x004B1E88`). Every address is static
(the exe has no relocations and no ASLR) and only the heap node pointers are read from the live head,
never reused from an earlier process. The script writes nothing to the game: no memory writes,
so there is nothing to restore; it only sends keys and one mouse click through tools/dinput-shim.

Guards, all fail-closed: stock exe hash; one stock process; foreground acquired through
`acquire_game_foreground.acquire` (never `goto_level.focus`); RETURN only after the highlight reads
exactly 0 (quirk: RETURN only at highlight 0); the loaded path is read from `0x004CBBF8` after level
entry; `ui_state` (`0x004CC134`, 1 = paused) is read around the pause steps; ESC steps are opt-in
(`--allow-esc`) because ESC has quit the process; install-file hashes before and after.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
import sys
import time
from pathlib import Path

COUNTER = 0x004C416C
LIST_HEAD = 0x005D59E4
NEXT_OFF = 4
PREV_OFF = 8
MENU_VTABLE = 0x004B2528
CHROME_VTABLE = 0x004B1E88
UI_STATE = 0x004CC134
PATH_BUFFER = 0x004CBBF8
MAX_NODES = 32
EXE_SHA256 = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
GAME_DIR = Path(r"C:\progs\Lemmings Revolution")
INSTALL_FILES = ("Lemmings Revolution.exe", "SavedGame.dat", "dgVoodoo.conf", "dinput.dll")
NODE_PIXEL = (206, 204)   # first gold node, render pixels (goto_level.NODE)


class CaptureError(RuntimeError):
    pass


def u32(read, address):
    data = read(address, 4)
    if not isinstance(data, bytes) or len(data) != 4:
        raise CaptureError(f"unreadable dword at {address:#x}")
    return struct.unpack("<I", data)[0]


def node_ok(p):
    return p % 4 == 0 and 0x10000 <= p < 0x80000000


def read_chain(read, next_off=None, prev_off=None):
    """Bracketed read of counter and list chain. Fails closed on a bad pointer, a cycle or a change."""
    next_off = NEXT_OFF if next_off is None else next_off
    prev_off = PREV_OFF if prev_off is None else prev_off
    counter = u32(read, COUNTER)
    head = u32(read, LIST_HEAD)
    nodes, seen, p = [], set(), head
    while p:
        if not node_ok(p):
            raise CaptureError(f"bad node pointer {p:#x}")
        if p in seen:
            raise CaptureError(f"cycle at {p:#x}")
        if len(nodes) >= MAX_NODES:
            raise CaptureError("chain longer than MAX_NODES")
        seen.add(p)
        nxt = u32(read, p + next_off)
        nodes.append(dict(addr=p, vtable=u32(read, p), next=nxt, prev=u32(read, p + prev_off)))
        p = nxt
    if u32(read, COUNTER) != counter or u32(read, LIST_HEAD) != head:
        raise CaptureError("counter or head changed while sampled")
    for a, b in zip(nodes, nodes[1:]):
        if b["prev"] != a["addr"]:
            raise CaptureError(f"prev link of {b['addr']:#x} is {b['prev']:#x}, not {a['addr']:#x}")
    if nodes and nodes[0]["prev"] != 0:
        raise CaptureError("head node has a nonzero prev")
    return dict(counter=counter, head=head, nodes=nodes,
                menus=sum(n["vtable"] == MENU_VTABLE for n in nodes),
                vtables=[hex(n["vtable"]) for n in nodes])


def observe(read, tries=3):
    for i in range(tries):
        try:
            return read_chain(read)
        except CaptureError as exc:
            if i == tries - 1 or "changed while sampled" not in str(exc):
                raise
            time.sleep(0.05)


# Scripted steps. kind: observe | highlight0 | key | node_click. `expect_counter` is recorded beside
# the observation, never asserted: the construction counter is exactly what is being measured.
PLAN = (
    dict(name="baseline", kind="observe", expect_counter=3),
    dict(name="menu-highlight-0", kind="highlight0"),
    dict(name="menu-RETURN", kind="key", key="RETURN", hold=20, needs_highlight0=True, expect_counter=4),
    dict(name="confirm-RETURN", kind="key", key="RETURN", hold=20, expect_counter=5),
    dict(name="map-node-click", kind="node_click"),
    dict(name="pause-P", kind="key", key="P", hold=20, expect_ui_state=1),
    dict(name="pause-short-ESC", kind="key", key="ESCAPE", hold=25, needs_esc=True, expect_ui_state=1),
    dict(name="pause-long-ESC", kind="key", key="ESCAPE", hold=40, needs_esc=True),
)


def run_plan(read, io, allow_esc=False, log=None, on_step=None):
    """io: key(name, hold), highlight() -> int | None, drive_highlight_to_zero(), click_node(),
    ui_state() -> int, loaded_path() -> str. Returns the list of per-step records."""
    out = []

    def rec(step, **extra):
        r = dict(step=step["name"], expect_counter=step.get("expect_counter"),
                 expect_ui_state=step.get("expect_ui_state"), ui_state=io.ui_state(), **extra)
        try:
            r.update(observe(read))
        except CaptureError as exc:
            r["chain_error"] = str(exc)
            out.append(r)
            if log:
                log(r)
            if on_step:
                on_step(r)
            raise
        out.append(r)
        if log:
            log(r)
        if on_step:
            on_step(r)

    for step in PLAN:
        kind = step["kind"]
        if step.get("needs_esc") and not allow_esc:
            out.append(dict(step=step["name"], skipped="ESC is opt-in (--allow-esc)"))
            continue
        if kind == "observe":
            rec(step)
        elif kind == "highlight0":
            io.drive_highlight_to_zero()
            rec(step, highlight=io.highlight())
        elif kind == "key":
            if step["key"] in ("P", "ESCAPE") and io.ui_state() not in (0, 1):
                raise CaptureError(f"refusing {step['key']}: ui_state {io.ui_state()} is not a level (0) or pause (1)")
            if step["key"] == "ESCAPE" and io.ui_state() != 1:
                raise CaptureError(f"refusing ESCAPE: ui_state {io.ui_state()} is not pause (1)")
            if step.get("needs_highlight0") and io.highlight() != 0:
                raise CaptureError(f"refusing RETURN: highlight is {io.highlight()!r}, not 0")
            io.key(step["key"], step["hold"])
            rec(step)
        elif kind == "node_click":
            io.click_node()
            rec(step, loaded_path=io.loaded_path())
    return out


def sha256_file(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def install_hashes(game_dir=GAME_DIR):
    return {n: sha256_file(Path(game_dir) / n) for n in INSTALL_FILES}


def live(args):  # pragma: no cover - Windows, live game only
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    from acquire_game_foreground import acquire, foreground_pid
    from drive import aim_at
    from goto_level import find_menu_index
    from lrinput import DIK, LRInput
    from lrstate import attach, i32, ui_state
    from memdump import k32, read_mem

    if not args.live_owner:
        raise CaptureError("--live-owner is required: one live owner, one granted stock start")
    before = install_hashes()
    if before["Lemmings Revolution.exe"] != EXE_SHA256:
        raise CaptureError("stock exe hash mismatch")
    h, pid = attach()
    if not h:
        raise CaptureError("the stock game is not running")
    rawlog = []

    def read(a, n):
        d = read_mem(h, a, n)
        rawlog.append(dict(addr=hex(a), size=n, hex=d.hex() if isinstance(d, bytes) else None))
        return d

    dump_dir = Path(args.dump_dir) if args.dump_dir else None
    if dump_dir:
        dump_dir.mkdir(parents=True, exist_ok=True)
    done = []

    def on_step(r):
        # raw reads behind this record, then the first 0x40 bytes of every node (after the bracket)
        for n in r.get("nodes", []):
            read(n["addr"], 0x40)
        if dump_dir:
            name = f"{len(done):02d}-{r['step']}.json"
            (dump_dir / name).write_text(json.dumps(dict(step=r["step"], reads=rawlog), indent=1) + "\n", encoding="utf-8")
        rawlog.clear()
        done.append(r)

    report = dict(pid=pid, install_before=before, allow_esc=args.allow_esc, writes_to_game=0, steps=[])
    with LRInput() as inp:
        class IO:
            sel = None

            def key(self, name, hold):
                report.setdefault("foreground", []).append(acquire(pid))
                if foreground_pid() != pid:
                    raise CaptureError("game is not foreground; no input sent")
                inp.tap(DIK[name], hold_frames=hold)
                time.sleep(1.5)

            def drive_highlight_to_zero(self):
                report.setdefault("foreground", []).append(acquire(pid))
                self.sel = find_menu_index(h, inp)
                if self.sel is None:
                    raise CaptureError("menu highlight not found; refusing RETURN")
                for _ in range(8):
                    if i32(h, self.sel) == 0:
                        return
                    inp.tap(DIK["DOWN"], hold_frames=20)
                    time.sleep(0.5)

            def highlight(self):
                return i32(h, self.sel) if self.sel else None

            def click_node(self):
                report.setdefault("foreground", []).append(acquire(pid))
                aim_at(h, inp, NODE_PIXEL)
                inp.button(0, True)
                inp.wait_frames(8)
                inp.button(0, False)
                inp.wait_frames(8)
                time.sleep(2.5)

            def ui_state(self):
                return ui_state(h)

            def loaded_path(self):
                raw = read(PATH_BUFFER, 64) or b""
                return raw.split(b"\0")[0].decode("latin1")

        inp.enable()
        try:
            report["steps"] = run_plan(read, IO(), args.allow_esc,
                                       log=lambda r: print(json.dumps(r), flush=True), on_step=on_step)
        except Exception as exc:
            report["steps"] = done
            report["error"] = f"{type(exc).__name__}: {exc}"
            raise
        finally:
            inp.release()
            k32.CloseHandle(h)
            report["install_after"] = install_hashes()
            report["install_unchanged"] = report["install_after"] == before
            Path(args.out).write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    return 0


class FakeMem:
    """Synthetic memory image: dword map, unmapped reads return None."""

    def __init__(self, words):
        self.words = dict(words)

    def __call__(self, address, size):
        return struct.pack("<I", self.words[address]) if address in self.words and size == 4 else None


def synth(counter=3, menu_vt=MENU_VTABLE):
    a, b = 0x01000100, 0x01000200   # newest first: a (menu), b (older screen)
    return FakeMem({COUNTER: counter, LIST_HEAD: a,
                    a: menu_vt, a + 4: b, a + 8: 0,
                    b: 0x004B1111, b + 4: 0, b + 8: a})


def selftest():
    import copy
    ok = []

    def check(name, cond):
        ok.append((name, bool(cond)))
        print(("PASS " if cond else "FAIL ") + name)

    def raises(fn):
        try:
            fn()
        except CaptureError:
            return True
        return False

    m = synth()
    c = read_chain(m)
    check("chain: counter 3, two nodes, one menu", c["counter"] == 3 and len(c["nodes"]) == 2 and c["menus"] == 1)
    check("chain: vtables in list order", c["vtables"] == [hex(MENU_VTABLE), "0x4b1111"])
    cyc = synth(); cyc.words[0x01000200 + 4] = 0x01000100
    check("cycle fails closed", raises(lambda: read_chain(cyc)))
    mis = synth(); mis.words[0x01000100 + 4] = 0x01000102
    check("unaligned next fails closed", raises(lambda: read_chain(mis)))
    unm = synth(); unm.words[0x01000100 + 4] = 0x01000300
    check("unmapped next fails closed", raises(lambda: read_chain(unm)))
    badprev = synth(); badprev.words[0x01000200 + 8] = 0x01000900
    check("inconsistent prev fails closed", raises(lambda: read_chain(badprev)))
    check("empty list ok", read_chain(FakeMem({COUNTER: 1, LIST_HEAD: 0}))["nodes"] == [])

    class FakeIO:
        """Fake original: counter 3 -> 4 -> 5 on RETURN, highlight starts at 2."""

        def __init__(self, mem, hl=2, stuck=False):
            self.mem, self.hl, self.stuck, self.keys, self.ui = mem, hl, stuck, [], 3

        def key(self, name, hold):
            self.keys.append((name, hold))
            if name == "RETURN":
                self.mem.words[COUNTER] += 1
            if name == "P":
                self.ui = 1

        def drive_highlight_to_zero(self):
            self.hl = 2 if self.stuck else 0

        def highlight(self):
            return self.hl

        def click_node(self):
            self.ui = 0

        def ui_state(self):
            return self.ui

        def loaded_path(self):
            return "Compiled\\FAKE.lvl"

    io = FakeIO(synth())
    steps = run_plan(io.mem, io)
    counters = [s["counter"] for s in steps if "counter" in s]
    check("plan: counters 3,3,4,5,5,5", counters == [3, 3, 4, 5, 5, 5])
    check("plan: two RETURN taps, no ESC without opt-in",
          [k for k, _ in io.keys].count("RETURN") == 2 and "ESCAPE" not in [k for k, _ in io.keys])
    check("plan: ESC steps skipped", sum("skipped" in s for s in steps) == 2)
    io2 = FakeIO(synth())
    run_plan(io2.mem, io2, allow_esc=True)
    check("plan: --allow-esc sends short 25 then long 40", [x for x in io2.keys if x[0] == "ESCAPE"] == [("ESCAPE", 25), ("ESCAPE", 40)])
    io3 = FakeIO(synth(), stuck=True)
    check("plan: RETURN refused when highlight != 0", raises(lambda: run_plan(io3.mem, io3)) and not io3.keys)
    io5 = FakeIO(synth())
    io5.click_node = lambda: None   # click never entered a level: ui_state stays 3
    check("plan: P/ESC refused outside a level", raises(lambda: run_plan(io5.mem, io5, allow_esc=True))
          and not [k for k, _ in io5.keys if k in ("P", "ESCAPE")])
    io6 = FakeIO(synth())
    io6.key = lambda name, hold: io6.keys.append((name, hold))   # P never pauses: ui_state stays 0
    check("plan: ESC refused when ui_state is not pause", raises(lambda: run_plan(io6.mem, io6, allow_esc=True))
          and not [k for k, _ in io6.keys if k == "ESCAPE"])
    bad = synth(); bad.words[0x01000200 + 4] = 0x01000100
    io4 = FakeIO(bad)
    check("plan: bad chain stops before any key", raises(lambda: run_plan(io4.mem, io4)) and not io4.keys)
    failed = [n for n, v in ok if not v]
    print(f"{len(ok) - len(failed)}/{len(ok)} passed")
    return 1 if failed else 0


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--selftest", action="store_true")
    ap.add_argument("--plan", action="store_true", help="print the scripted steps; touches nothing")
    ap.add_argument("--run", action="store_true", help="LIVE capture; not run in the preflight round")
    ap.add_argument("--live-owner", action="store_true")
    ap.add_argument("--allow-esc", action="store_true")
    ap.add_argument("--mutate-next-off", action="store_true",
                    help="selftest control: set the list next offset to 8; the selftest must then FAIL")
    ap.add_argument("--out")
    ap.add_argument("--dump-dir", help="write the raw reads behind each step here")
    args = ap.parse_args()
    if args.selftest:
        if args.mutate_next_off:
            global NEXT_OFF
            NEXT_OFF = 8
        return selftest()
    if args.plan:
        print(json.dumps(PLAN, indent=1))
        return 0
    if args.run:
        if not args.out:
            ap.error("--run needs --out")
        try:
            return live(args)
        except CaptureError as exc:
            print(f"FAIL_CLOSED: {exc}", file=sys.stderr)
            return 2
    ap.print_help()
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
