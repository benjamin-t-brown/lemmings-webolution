"""Drive the installed game into a level and collect hook-harness tallies.

The game reads all input through IDirectInputDevice::GetDeviceState and ignores
Windows input injection entirely, so SendInput and SendKeys cannot reach it.
That is why the hook harness's gameplay counters stayed at zero: only its
startup sweeps were ever exercised.

The sibling remake project solved the input problem with a `dinput.dll` proxy
that serves device state from a named shared-memory block, plus a frontend
driver that knows the quirks of getting from the title screen into a level.
Those quirks are not incidental -- the retail menus are German, the fifth main
menu item is "Beenden", so a blind RETURN quits the game, and RETURN is only
accepted while the highlight reads exactly zero. Reimplementing that here would
mean re-deriving all of it, so this script delegates frontend navigation to
`goto_level.py` across a process boundary and keeps the two repositories
otherwise separate.

Requires the stock 640x480 executable. Under the resolution-patched build the
level map's nodes are 3D-picked at the wrong scale and respond to no click.

    python tools/drive_hook_session.py --level MM_DIG_000 --seconds 60
    python tools/drive_hook_session.py --list-levels
"""

import argparse
import ctypes
import ctypes.wintypes as wt
import os
import struct
import subprocess
import sys
import time
from pathlib import Path

MAGIC = 0x4E49524C  # 'LRIN'
MAPPING_NAME = rb"Local\LemmingsRevolutionInput"

# Must equal sizeof(LRInputBlock) in the shim exactly. The section keeps the
# size it was created with, so a disagreement makes one side's MapViewOfFile
# fail and the shim silently serves nothing.
BLOCK_SIZE = 36 + 256 + 4 * 4
OFF_MAGIC = 0
OFF_CONSUME = 32
OFF_STATE_CALLS = 304

PAGE_READWRITE = 0x04
FILE_MAP_ALL_ACCESS = 0xF001F
INVALID_HANDLE_VALUE = wt.HANDLE(-1)

ROUTINES = ("collision_cell", "wrap_cylinder_x", "append_native_int")

k32 = ctypes.WinDLL("kernel32", use_last_error=True)
k32.CreateFileMappingA.restype = wt.HANDLE
k32.CreateFileMappingA.argtypes = [wt.HANDLE, ctypes.c_void_p, wt.DWORD,
                                   wt.DWORD, wt.DWORD, wt.LPCSTR]
k32.MapViewOfFile.restype = ctypes.c_void_p
k32.MapViewOfFile.argtypes = [wt.HANDLE, wt.DWORD, wt.DWORD, wt.DWORD,
                              ctypes.c_size_t]
k32.UnmapViewOfFile.argtypes = [ctypes.c_void_p]
k32.CloseHandle.argtypes = [wt.HANDLE]


class ShimProbe:
    """Read-only view of the shim's block, to confirm the game is polling it."""

    def __enter__(self):
        self.handle = k32.CreateFileMappingA(INVALID_HANDLE_VALUE, None,
                                             PAGE_READWRITE, 0, BLOCK_SIZE,
                                             MAPPING_NAME)
        if not self.handle:
            raise OSError("CreateFileMapping failed: %d" % ctypes.get_last_error())
        self.base = k32.MapViewOfFile(self.handle, FILE_MAP_ALL_ACCESS, 0, 0,
                                      BLOCK_SIZE)
        if not self.base:
            raise OSError("MapViewOfFile failed: %d" % ctypes.get_last_error())
        if self._u32(OFF_MAGIC) != MAGIC:
            ctypes.memset(self.base, 0, BLOCK_SIZE)
            self._set_u32(OFF_MAGIC, MAGIC)
            self._set_u32(OFF_CONSUME, 1)
        return self

    def __exit__(self, *exc):
        if self.base:
            k32.UnmapViewOfFile(ctypes.c_void_p(self.base))
        if self.handle:
            k32.CloseHandle(self.handle)

    def _u32(self, offset):
        return struct.unpack_from("<I", ctypes.string_at(self.base + offset, 4))[0]

    def _set_u32(self, offset, value):
        ctypes.memmove(self.base + offset, struct.pack("<I", value & 0xFFFFFFFF), 4)

    @property
    def state_calls(self):
        return self._u32(OFF_STATE_CALLS)

    def wait_until_polled(self, timeout=60.0):
        """True once the game has read device state at least twice."""
        first = self.state_calls
        deadline = time.time() + timeout
        while time.time() < deadline:
            if self.state_calls > first + 1:
                return True
            time.sleep(0.2)
        return False


def tallies(log_path):
    """Latest per-routine counters the harness has written."""
    if not log_path.exists():
        return {}
    found = {}
    for line in log_path.read_text(errors="replace").splitlines():
        for routine in ROUTINES:
            marker = routine + ": "
            if line.startswith(marker) and " calls" in line:
                found[routine] = line[len(marker):].strip()
    return found


def run_goto_level(harness, level, list_only=False):
    script = harness / "goto_level.py"
    if not script.exists():
        print(f"frontend driver not found: {script}", file=sys.stderr)
        return None
    command = [sys.executable, str(script)]
    command.append("--list" if list_only else level)
    done = subprocess.run(command, cwd=str(harness), capture_output=True, text=True)
    sys.stdout.write(done.stdout)
    if done.stderr.strip():
        sys.stderr.write(done.stderr)
    return done


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--install-dir", type=Path,
                        default=Path(r"C:\progs\Lemmings Revolution"))
    parser.add_argument("--harness", type=Path,
                        default=Path(r"C:\progs\lemmings-webolution\tools\harness"),
                        help="the remake project's harness, for goto_level.py")
    parser.add_argument("--level", default="MM_DIG_000",
                        help="level id to enter, as the live table names it")
    parser.add_argument("--seconds", type=float, default=60.0,
                        help="how long to stay in the level")
    parser.add_argument("--list-levels", action="store_true",
                        help="print the live level table and exit")
    parser.add_argument("--coverage", type=Path,
                        help="trace executed functions instead of running the "
                             "routine detours; takes the target list from "
                             "tools/classify_inventory.py --addresses")
    parser.add_argument("--frontend-only", action="store_true",
                        help="do not enter a level; trace the frontend alone")
    args = parser.parse_args()

    exe = args.install_dir / "Lemmings Revolution.exe"
    log = args.install_dir / "lemmings_hooks.log"
    shim = args.install_dir / "dinput.dll"

    if not shim.exists():
        print(f"no dinput shim at {shim}; input cannot reach the game",
              file=sys.stderr)
        return 1

    trace = args.install_dir / "lemmings_coverage.txt"
    environment = None
    if args.coverage:
        if not args.coverage.exists():
            print(f"coverage target list not found: {args.coverage}", file=sys.stderr)
            return 1
        environment = dict(os.environ)
        environment["LEMMINGS_HOOK_COVERAGE"] = str(args.coverage.resolve())
        if trace.exists():
            trace.unlink()

    if log.exists():
        log.unlink()
    proc = subprocess.Popen([str(exe)], cwd=str(args.install_dir), env=environment)
    print(f"launched pid {proc.pid}")

    try:
        with ShimProbe() as probe:
            if not probe.wait_until_polled():
                print("the game never polled the shim", file=sys.stderr)
                return 2
            print(f"shim is live (state_calls={probe.state_calls})")

            if args.list_levels:
                run_goto_level(args.harness, None, list_only=True)
                return 0

            if args.frontend_only:
                print(f"frontend only; holding {args.seconds:.0f}s")
            else:
                print(f"entering {args.level} via the frontend driver")
                done = run_goto_level(args.harness, args.level)
                if done is None:
                    return 1
                if done.returncode != 0:
                    print(f"frontend driver returned {done.returncode}; "
                          f"not in a level", file=sys.stderr)
                    return 3
                print(f"in a level; holding {args.seconds:.0f}s")

            end = time.time() + args.seconds
            while time.time() < end:
                if proc.poll() is not None:
                    print("the game exited early", file=sys.stderr)
                    break
                time.sleep(3.0)
                if args.coverage:
                    if trace.exists():
                        hits = sum(1 for line in trace.read_text(errors="replace")
                                   .splitlines() if line and not line.startswith("#"))
                        print(f"  {hits} functions executed so far")
                else:
                    current = tallies(log)
                    if current:
                        print(f"  {current}")
    finally:
        if proc.poll() is None:
            # A clean quit would let DLL_PROCESS_DETACH write final totals, but
            # the frontend quit path is not scripted, so the harness's
            # power-of-two milestones are what survive a terminate.
            proc.terminate()
            try:
                proc.wait(timeout=10)
            except subprocess.TimeoutExpired:
                proc.kill()
            print("game stopped")

    time.sleep(1)
    print()
    if args.coverage:
        print("=== coverage ===")
        if not trace.exists():
            print("no trace written")
            return 4
        rows = [l for l in trace.read_text(errors="replace").splitlines()
                if l and not l.startswith("#")]
        print(f"  {len(rows)} functions executed, trace at {trace}")
        return 0
    print("=== harness tallies ===")
    final = tallies(log)
    if not final:
        print("no per-routine tallies; the hooks saw no gameplay calls")
        return 4
    for routine, line in sorted(final.items()):
        print(f"  {routine}: {line}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
