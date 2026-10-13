"""Attach to a running Lemmings Revolution process and dump candidate terrain.

The exe loads at a fixed image base 0x00400000 (PLAN.md). The level object
pointer at 0x004CBF84 is a known static. This tool:

  1. Opens the process by name substring
  2. Reads [004CBF84] and a few neighbour statics
  3. Optionally scans committed memory for byte patterns that look like a
     wraparound occupancy mask (high runs of 0/1, dimensions near 256x192)

Usage:
  python memdump.py                  # list process + statics
  python memdump.py --scan           # also scan for mask-like regions
  python memdump.py --dump out.bin --addr 0x........ --size N
"""

from __future__ import annotations

import argparse
import ctypes
import ctypes.wintypes as wt
import struct
import sys

PROCESS_VM_READ = 0x0010
PROCESS_QUERY_INFORMATION = 0x0400
MEM_COMMIT = 0x1000
PAGE_NOACCESS = 0x01
PAGE_GUARD = 0x100

k32 = ctypes.WinDLL("kernel32", use_last_error=True)


class MEMORY_BASIC_INFORMATION(ctypes.Structure):
    _fields_ = [
        ("BaseAddress", ctypes.c_void_p),
        ("AllocationBase", ctypes.c_void_p),
        ("AllocationProtect", wt.DWORD),
        ("RegionSize", ctypes.c_size_t),
        ("State", wt.DWORD),
        ("Protect", wt.DWORD),
        ("Type", wt.DWORD),
    ]


k32.OpenProcess.argtypes = [wt.DWORD, wt.BOOL, wt.DWORD]
k32.OpenProcess.restype = wt.HANDLE
k32.ReadProcessMemory.argtypes = [
    wt.HANDLE,
    ctypes.c_void_p,
    ctypes.c_void_p,
    ctypes.c_size_t,
    ctypes.POINTER(ctypes.c_size_t),
]
k32.ReadProcessMemory.restype = wt.BOOL
k32.VirtualQueryEx.argtypes = [
    wt.HANDLE,
    ctypes.c_void_p,
    ctypes.POINTER(MEMORY_BASIC_INFORMATION),
    ctypes.c_size_t,
]
k32.VirtualQueryEx.restype = ctypes.c_size_t
k32.CloseHandle.argtypes = [wt.HANDLE]

# Toolhelp for process list
TH32CS_SNAPPROCESS = 0x00000002


class PROCESSENTRY32(ctypes.Structure):
    _fields_ = [
        ("dwSize", wt.DWORD),
        ("cntUsage", wt.DWORD),
        ("th32ProcessID", wt.DWORD),
        ("th32DefaultHeapID", ctypes.POINTER(ctypes.c_ulong)),
        ("th32ModuleID", wt.DWORD),
        ("cntThreads", wt.DWORD),
        ("th32ParentProcessID", wt.DWORD),
        ("pcPriClassBase", ctypes.c_long),
        ("dwFlags", wt.DWORD),
        ("szExeFile", ctypes.c_char * 260),
    ]


k32.CreateToolhelp32Snapshot.argtypes = [wt.DWORD, wt.DWORD]
k32.CreateToolhelp32Snapshot.restype = wt.HANDLE
k32.Process32First.argtypes = [wt.HANDLE, ctypes.POINTER(PROCESSENTRY32)]
k32.Process32First.restype = wt.BOOL
k32.Process32Next.argtypes = [wt.HANDLE, ctypes.POINTER(PROCESSENTRY32)]
k32.Process32Next.restype = wt.BOOL

STATICS = {
    "level_ptr": 0x004CBF84,
    "font_a": 0x004CBF78,
    "font_b": 0x004CBF7C,
}


def find_pids(substr: str) -> list[tuple[int, str]]:
    snap = k32.CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)
    if snap == wt.HANDLE(-1).value:
        raise OSError("CreateToolhelp32Snapshot failed")
    try:
        pe = PROCESSENTRY32()
        pe.dwSize = ctypes.sizeof(PROCESSENTRY32)
        out = []
        ok = k32.Process32First(snap, ctypes.byref(pe))
        while ok:
            name = pe.szExeFile.decode("ascii", "replace")
            if substr.lower() in name.lower():
                out.append((pe.th32ProcessID, name))
            ok = k32.Process32Next(snap, ctypes.byref(pe))
        return out
    finally:
        k32.CloseHandle(snap)


def read_mem(h, addr: int, size: int) -> bytes | None:
    buf = (ctypes.c_ubyte * size)()
    got = ctypes.c_size_t(0)
    if not k32.ReadProcessMemory(h, ctypes.c_void_p(addr), buf, size, ctypes.byref(got)):
        return None
    return bytes(buf[: got.value])


def read_u32(h, addr: int) -> int | None:
    b = read_mem(h, addr, 4)
    if not b or len(b) < 4:
        return None
    return struct.unpack_from("<I", b)[0]


def mask_score(blob: bytes, width: int, height: int) -> float:
    """How much a buffer looks like a 0/1 occupancy mask of given size."""
    n = width * height
    if len(blob) < n:
        return -1
    data = blob[:n]
    ones = sum(1 for b in data if b != 0)
    zeros = n - ones
    # Prefer binary-ish values
    weird = sum(1 for b in data if b not in (0, 1, 0xFF))
    if weird > n * 0.05:
        return -1
    frac = ones / n
    # Our surface masks are sparse (~1-25%)
    if frac < 0.005 or frac > 0.4:
        return -1
    # Reward horizontal wrap structure: some rows with runs
    return frac * (1.0 - weird / n)


def scan_masks(h, widths=(256, 320, 512, 640), heights=(160, 192, 200, 240, 480)):
    mbi = MEMORY_BASIC_INFORMATION()
    addr = 0
    hits = []
    while addr < 0x7FFF0000:
        got = k32.VirtualQueryEx(h, ctypes.c_void_p(addr), ctypes.byref(mbi), ctypes.sizeof(mbi))
        if got == 0:
            break
        base = mbi.BaseAddress or 0
        size = mbi.RegionSize or 0
        if size == 0:
            break
        if mbi.State == MEM_COMMIT and not (mbi.Protect & PAGE_GUARD) and mbi.Protect != PAGE_NOACCESS:
            # Cap read size
            chunk = min(size, 8 * 1024 * 1024)
            data = read_mem(h, base, chunk)
            if data:
                for w in widths:
                    for ht in heights:
                        need = w * ht
                        if need > len(data):
                            continue
                        # sample a few offsets
                        for off in range(0, min(len(data) - need, 0x10000), 4):
                            sc = mask_score(data[off : off + need], w, ht)
                            if sc > 0:
                                hits.append((sc, base + off, w, ht, int(sc * w * ht)))
                                break  # one hit per region/size
        addr = base + size
        if addr <= base:
            break
    hits.sort(reverse=True)
    return hits[:40]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--name", default="SkyIslands", help="process name substring")
    ap.add_argument("--scan", action="store_true")
    ap.add_argument("--dump")
    ap.add_argument("--addr", type=lambda x: int(x, 0))
    ap.add_argument("--size", type=int, default=256 * 192)
    args = ap.parse_args()

    procs = find_pids(args.name)
    if not procs:
        # broader search
        for cand in ("lemming", "revolution", "Sky", "Take2"):
            procs = find_pids(cand)
            if procs:
                break
    if not procs:
        print("no matching process; start the game first", file=sys.stderr)
        # still list a few running exes for convenience
        snap = k32.CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)
        pe = PROCESSENTRY32()
        pe.dwSize = ctypes.sizeof(PROCESSENTRY32)
        print("running processes (sample):")
        n = 0
        ok = k32.Process32First(snap, ctypes.byref(pe))
        while ok and n < 30:
            print(" ", pe.szExeFile.decode("ascii", "replace"), pe.th32ProcessID)
            ok = k32.Process32Next(snap, ctypes.byref(pe))
            n += 1
        k32.CloseHandle(snap)
        return 1

    pid, exe = procs[0]
    print(f"attach {exe} pid={pid}")
    h = k32.OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, False, pid)
    if not h:
        print("OpenProcess failed:", ctypes.get_last_error(), file=sys.stderr)
        return 1
    try:
        for label, addr in STATICS.items():
            v = read_u32(h, addr)
            print(f"  [{label} {addr:#010x}] = {v:#010x}" if v is not None else f"  [{label}] unreadable")
            if v and label == "level_ptr" and 0x10000 < v < 0x7FFF0000:
                # dump first 64 bytes of level object
                head = read_mem(h, v, 64)
                if head:
                    print("    level object head:", head.hex(" "))
                    words = struct.unpack_from("<16I", head)
                    print("    as u32:", [hex(x) for x in words])

        if args.dump and args.addr is not None:
            blob = read_mem(h, args.addr, args.size)
            if not blob:
                print("dump read failed", file=sys.stderr)
                return 1
            open(args.dump, "wb").write(blob)
            print(f"wrote {len(blob)} bytes to {args.dump}")

        if args.scan:
            print("scanning for mask-like regions (this can take a bit)...")
            hits = scan_masks(h)
            for sc, addr, w, ht, ones in hits:
                print(f"  score={sc:.4f} {addr:#010x} {w}x{ht} ones≈{ones}")
    finally:
        k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    sys.exit(main())
