"""Peek the live level object and follow a few pointers."""
import struct
import sys
sys.path.insert(0, r"C:\progs\lemmings-webolution\tools\harness")
from memdump import find_pids, read_mem, read_u32, k32, PROCESS_VM_READ, PROCESS_QUERY_INFORMATION

LEVEL_PTR = 0x004CBF84

def main():
    procs = find_pids("Lemmings")
    if not procs:
        print("no process")
        return 1
    pid, exe = procs[0]
    print(f"{exe} pid={pid}")
    h = k32.OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, False, pid)
    level = read_u32(h, LEVEL_PTR)
    print(f"level={level:#x}")
    blob = read_mem(h, level, 0x200)
    print("first 128 u32:")
    words = struct.unpack_from("<" + "I" * 32, blob)
    for i, w in enumerate(words):
        mark = ""
        if 0x10000 < w < 0x7FFF0000:
            mark = " <- ptr?"
        print(f"  +{i*4:03x}: {w:#010x}{mark}")

    # Follow pointer-looking fields and print their heads
    for i, w in enumerate(words):
        if 0x01000000 < w < 0x7FFF0000 and w != level:
            head = read_mem(h, w, 64)
            if head:
                # printable?
                ascii = "".join(chr(b) if 32 <= b < 127 else "." for b in head[:32])
                print(f"\n*[{i}] {w:#x} ascii={ascii!r} hex={head[:32].hex(' ')}")
    k32.CloseHandle(h)
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
