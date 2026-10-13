"""Scan the live game (must be in a level) for mask-like buffers."""
from __future__ import annotations

import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from dump_buffers import try_desc
from enter_play import level_info
from memdump import (
    PROCESS_QUERY_INFORMATION,
    PROCESS_VM_READ,
    find_pids,
    k32,
    read_mem,
    read_u32,
    scan_masks,
)

OUT = Path(r"C:\progs\lemmings-webolution\build\live_dump")
LEVEL_PTR = 0x004CBF84


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    procs = find_pids("Lemmings")
    if not procs:
        print("no game")
        return 1
    h = k32.OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, False, procs[0][0])
    print("level info", level_info(h), flush=True)
    print("level_ptr", hex(read_u32(h, LEVEL_PTR) or 0), flush=True)
    print("scanning...", flush=True)
    hits = scan_masks(h)
    print("hits", len(hits), flush=True)
    for sc, addr, ww, ht, ones in hits[:40]:
        print(f"  {sc:.4f} {addr:#x} {ww}x{ht} ones~{ones}", flush=True)
    if hits:
        sc, addr, ww, ht, _ = hits[0]
        blob = read_mem(h, addr, ww * ht)
        if blob:
            p = OUT / f"live_mask_{ww}x{ht}_{addr:08x}.bin"
            p.write_bytes(blob)
            print("wrote", p, "nz", sum(1 for b in blob if b), flush=True)
    level = read_u32(h, LEVEL_PTR)
    blob = read_mem(h, level, 0x200)
    words = struct.unpack_from("<128I", blob)
    for i, w in enumerate(words):
        if 0x01000000 < w < 0x7FFF0000:
            try_desc(h, w, f"L{i}")
    k32.CloseHandle(h)
    print("done", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
