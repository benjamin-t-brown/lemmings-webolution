"""One-shot: poke a walker onto the H10 SpeedPad AABB and watch FASTER."""
from __future__ import annotations

import ctypes
import struct
import sys
import time

sys.path.insert(0, ".")
from goto_level import focus  # noqa: E402
from lrstate import attach, u32, ui_state  # noqa: E402
from measure_spawn import LEMMING_VT, OTHER_VT, actor_ptrs  # noqa: E402
from memdump import k32, read_mem  # noqa: E402

VT_FASTER = 0x004B0A38
OBJ = 0x02697780  # H10_MT_1 live pad (re-resolve if needed)


def write_mem(h, addr: int, data: bytes) -> bool:
    n = ctypes.c_size_t(0)
    return bool(
        k32.WriteProcessMemory(
            h, ctypes.c_void_p(addr), data, len(data), ctypes.byref(n)
        )
    )


def find_pad(h) -> int | None:
    needle = struct.pack("<I", 0x004B1178)
    from diffscan import regions

    for base, size in regions(h):
        blob = read_mem(h, base, size)
        if not blob:
            continue
        i = 0
        while True:
            j = blob.find(needle, i)
            if j < 0:
                break
            if j % 4 == 0:
                return base + j
            i = j + 4
    return None


def main() -> int:
    h, pid = attach(write=True)
    if not h:
        print("no game")
        return 1
    print("ui", ui_state(h), "pid", pid, "focus", focus(pid))
    obj = find_pad(h) or OBJ
    buf = read_mem(h, obj, 0x100)
    print("pad", hex(obj), "vt", hex(struct.unpack_from("<I", buf, 0)[0]))
    print("+0xa0", buf[0xA0], "+0xb8", struct.unpack_from("<i", buf, 0xB8)[0],
          "+0xbc", struct.unpack_from("<i", buf, 0xBC)[0])
    print("msg +0xc0..", [struct.unpack_from("<i", buf, 0xC0 + i)[0]
                          for i in range(0, 0x18, 4)])
    cue = u32(h, 0x004E32F8)
    print("DAT_004E32F8", hex(cue) if cue else None)

    target = None
    orig_x = None
    orig_face = None
    for p in actor_ptrs(h):
        if u32(h, p) not in (LEMMING_VT, OTHER_VT):
            continue
        b = read_mem(h, p, 0x60)
        x, y = struct.unpack_from("<dd", b, 8)
        mode = struct.unpack_from("<i", b, 0x50)[0]
        face = struct.unpack_from("<i", b, 0x40)[0]
        vdir = struct.unpack_from("<i", b, 0x44)[0]
        if abs(y - 29.0) < 0.5 and mode == 1:
            target, orig_x, orig_face = p, x, face
            print(f"pick {p:#010x} x={x:.3f} face={face:+d} vdir={vdir:+d}")
            break
    if not target:
        print("no walker at y=29")
        k32.CloseHandle(h)
        return 1

    # Pad AABB X[208..210]; facing filter uses object+0xbc (0 = either).
    assert write_mem(h, target + 8, struct.pack("<d", 209.0))
    print("wrote X=209.0")
    t0 = time.perf_counter()
    seen = False
    while time.perf_counter() - t0 < 12:
        b = read_mem(h, target, 0x60)
        x, y = struct.unpack_from("<dd", b, 8)
        mode = struct.unpack_from("<i", b, 0x50)[0]
        ob = read_mem(h, obj, 0xC0)
        ost = struct.unpack_from("<i", ob, 0xB8)[0]
        head = u32(h, target + 0x374) or 0
        pars = []
        node = head
        for _ in range(8):
            if not node:
                break
            s = u32(h, node) or 0
            if s:
                pars.append(u32(h, s) or 0)
            node = u32(h, node + 4) or 0
        if mode == 2 or VT_FASTER in pars or ost == 1:
            seen = True
            print(
                f"HIT t={time.perf_counter()-t0:.3f} x={x:.3f} y={y:.3f} "
                f"mode={mode} obj+0xb8={ost} par={[hex(v) for v in pars]}"
            )
            x0 = x
            time.sleep(1.0)
            b = read_mem(h, target, 0x60)
            x1 = struct.unpack_from("<d", b, 8)[0]
            mode1 = struct.unpack_from("<i", b, 0x50)[0]
            print(f"rate |dx|/s={abs(x1 - x0):.3f} mode_now={mode1}")
            break
        time.sleep(0.05)

    b = read_mem(h, target, 0x60)
    x, y = struct.unpack_from("<dd", b, 8)
    mode = struct.unpack_from("<i", b, 0x50)[0]
    ob = read_mem(h, obj, 0xC0)
    print(
        f"final x={x:.3f} mode={mode} "
        f"obj+0xb8={struct.unpack_from('<i', ob, 0xB8)[0]} seen={seen}"
    )
    write_mem(h, target + 8, struct.pack("<d", orig_x))
    print(f"restored X={orig_x:.3f}")
    k32.CloseHandle(h)
    return 0 if seen else 2


if __name__ == "__main__":
    raise SystemExit(main())
