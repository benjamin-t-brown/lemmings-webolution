"""Fast scan for occupancy masks; compare to a reference .bin of 0/1 bytes."""
from __future__ import annotations

import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from memdump import (
    MEM_COMMIT,
    MEMORY_BASIC_INFORMATION,
    PAGE_GUARD,
    PAGE_NOACCESS,
    PROCESS_QUERY_INFORMATION,
    PROCESS_VM_READ,
    find_pids,
    k32,
    read_mem,
)

OUT = Path(r"C:\progs\lemmings-webolution\build\live_dump")


def score_byte_mask(data: bytes, ref: bytes | None) -> tuple[float, int, int]:
    n = len(data)
    ones = sum(1 for b in data if b != 0)
    weird = sum(1 for b in data if b not in (0, 1, 0xFF))
    if weird > n * 0.02:
        return -1.0, ones, weird
    frac = ones / n
    if frac < 0.02 or frac > 0.35:
        return -1.0, ones, weird
    corr = 0.0
    if ref and len(ref) == n:
        agree = sum(1 for a, b in zip(data, ref) if (a != 0) == (b != 0))
        corr = agree / n
    return frac + corr, ones, weird


def scan(h, sizes: list[int], ref: bytes | None):
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
            # Read whole region up to 16MB
            chunk = min(size, 16 * 1024 * 1024)
            data = read_mem(h, base, chunk)
            if data:
                for need in sizes:
                    if need > len(data):
                        continue
                    # stride by 16 within region; also check page starts
                    step = 16
                    limit = len(data) - need
                    off = 0
                    while off <= limit:
                        sc, ones, weird = score_byte_mask(data[off : off + need], ref if need == len(ref or b"") else None)
                        if sc > 0:
                            hits.append((sc, base + off, need, ones))
                            # skip ahead to avoid flooding same region
                            off += need
                            continue
                        off += step
                        # coarse: after first 256KB of region, jump by pages
                        if off > 256 * 1024:
                            step = 4096
        addr = base + size
        if addr <= base:
            break
    hits.sort(reverse=True)
    return hits[:50]


import ctypes  # noqa: E402


def main():
    ref_path = OUT / "ref_climb_mask.bin"
    ref = ref_path.read_bytes() if ref_path.exists() else None
    print("ref", len(ref) if ref else None, flush=True)
    sizes = [256 * 192, 320 * 200, 512 * 256, 256 * 160, 640 * 480]
    # bit-packed candidates
    bit_sizes = [256 * 192 // 8, 320 * 200 // 8]
    procs = find_pids("Lemmings")
    if not procs:
        print("no game")
        return 1
    h = k32.OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, False, procs[0][0])
    print("scan byte masks...", flush=True)
    hits = scan(h, sizes, ref)
    for sc, addr, need, ones in hits[:30]:
        print(f"  sc={sc:.4f} {addr:#x} n={need} ones={ones}", flush=True)
        if need == 256 * 192:
            blob = read_mem(h, addr, need)
            if blob:
                p = OUT / f"cand_{addr:08x}.bin"
                p.write_bytes(blob)
                if ref:
                    agree = sum(1 for a, b in zip(blob, ref) if (a != 0) == (b != 0))
                    print(f"    wrote {p.name} agree={agree/len(ref):.4f}", flush=True)
    print("scan bit-packed sizes (nonzero count only)...", flush=True)
    # For bit masks just look for size-matching alloc headers nearby — sample density
    mbi = MEMORY_BASIC_INFORMATION()
    addr = 0
    bit_hits = []
    while addr < 0x7FFF0000:
        got = k32.VirtualQueryEx(h, ctypes.c_void_p(addr), ctypes.byref(mbi), ctypes.sizeof(mbi))
        if got == 0:
            break
        base = mbi.BaseAddress or 0
        size = mbi.RegionSize or 0
        if size == 0:
            break
        if mbi.State == MEM_COMMIT and not (mbi.Protect & PAGE_GUARD) and mbi.Protect != PAGE_NOACCESS:
            data = read_mem(h, base, min(size, 4 * 1024 * 1024))
            if data:
                for need in bit_sizes:
                    if need > len(data):
                        continue
                    for off in range(0, min(len(data) - need, 64 * 1024), 8):
                        chunk = data[off : off + need]
                        ones = sum(bin(b).count("1") for b in chunk)
                        frac = ones / (need * 8)
                        if 0.05 < frac < 0.30:
                            bit_hits.append((frac, base + off, need, ones))
                            break
        addr = base + size
        if addr <= base:
            break
    bit_hits.sort(reverse=True)
    for frac, addr, need, ones in bit_hits[:20]:
        print(f"  bit frac={frac:.4f} {addr:#x} n={need} bits={ones}", flush=True)
    k32.CloseHandle(h)
    print("done", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
