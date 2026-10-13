"""Dump sized buffers hanging off the live level object."""
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from memdump import find_pids, read_mem, read_u32, k32, PROCESS_VM_READ, PROCESS_QUERY_INFORMATION

LEVEL_PTR = 0x004CBF84
OUT = Path(r"C:\progs\lemmings-webolution\build\live_dump")


def try_desc(h, addr, label):
    d = read_mem(h, addr, 64)
    if not d:
        print(label, "unreadable")
        return
    words = struct.unpack_from("<16I", d)
    print(f"{label} @{addr:#x}:", [hex(w) for w in words[:8]])
    # Pattern seen: size, size, 1, ptr
    if words[0] == words[1] and 16 <= words[0] <= 2_000_000 and 0x10000 < words[3] < 0x7FFF0000:
        n, ptr = words[0], words[3]
        body = read_mem(h, ptr, min(n, 256 * 192 * 4))
        if body:
            path = OUT / f"{label}_{n}.bin"
            path.write_bytes(body[:n] if len(body) >= n else body)
            ones = sum(1 for b in body[:n] if b not in (0,))
            uniq = len(set(body[: min(n, 4096)]))
            print(f"  wrote {path.name} n={n} nonzero={ones} uniq4k={uniq}")


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    procs = find_pids("Lemmings")
    if not procs:
        print("no game")
        return 1
    h = k32.OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, False, procs[0][0])
    level = read_u32(h, LEVEL_PTR)
    print("level", hex(level or 0))
    if not level:
        return 1
    blob = read_mem(h, level, 0x100)
    words = struct.unpack_from("<64I", blob)
    for i, w in enumerate(words):
        if 0x01000000 < w < 0x7FFF0000:
            try_desc(h, w, f"p{i}")
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
