"""Read the original's own UI state out of its memory, instead of guessing it.

Why this exists
---------------
Every earlier attempt at driving the original judged "where am I" from a
screenshot, and `PrintWindow` on this Direct3D window returns an all-black
bitmap whenever the window is not foreground -- silently, so brightness
checks then report nonsense. That cost at least two wrong conclusions.

The engine makes the question answerable directly. The main loop at
`.text:0045CFF9` is a method on a single static game object:

    .text:00450A4E   mov ecx, 4C6A08h        ; the game object
    .text:00450A53   call 0045CFF9           ; GameLoop::Step()

Inside it the screen is a plain switch on `this + 0x572C` with six arms
(jump table at `0045E487`), and the input manager hangs off `this + 0x55B8`:

    .text:0045D18D   mov edx, [ecx + 572Ch]  ; UI state 0..5
    .text:0045D15E   mov ecx, [ecx + 55B8h]  ; InputManager
    .text:0045D164   call 0048316B           ; InputManager::Update(now)

`InputManager + 0x08` is the keyboard device wrapper and `+ 0x0C` the mouse
one. The mouse wrapper's own layout falls straight out of the poll at
`.text:00483B5D`, which copies DIMOUSESTATE into it:

    +0x00  int   button0 down          (rgbButtons[0] & 0x80) != 0
    +0x04  int   button1 down
    +0x08  int   button0 down, previous frame
    +0x0C  int   button1 down, previous frame
    +0x10  byte  button0 press edge    down now && !down last frame
    +0x11  byte  button1 press edge
    +0x14  int   timestamp of the last button0 release
    +0x18  int   timestamp of the last button1 release
    +0x1C  byte  button0 "click"       set when two releases land < 500 ms apart
    +0x1D  byte  button1 "click"
    +0x20  int   dx, this frame
    +0x24  int   dy, this frame

That last pair of fields is the useful part for the harness: the shim's
deltas and button bytes can be confirmed to have *arrived*, separately from
whether the game acted on them, which is exactly the ambiguity that blocked
the live terrain dump.

`+0x1C` is worth calling out. It is not a single-click flag: nothing sets
`+0x14` on a press, only on a release, so the comparison at `.text:00483CD9`
measures release-to-release and the flag is a double-click.
"""
from __future__ import annotations

import ctypes
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from memdump import (  # noqa: E402
    PROCESS_QUERY_INFORMATION,
    PROCESS_VM_READ,
    find_pids,
    k32,
    read_mem,
)

GAME_OBJ = 0x004C6A08
UI_STATE = GAME_OBJ + 0x572C          # 0x004CC134
INPUT_MGR_PTR = GAME_OBJ + 0x55B8     # 0x004CBFC0
CURSOR_OBJ = 0x0058CB40               # ecx at .text:0045C980; bounds live here
LEVEL_PTR = 0x004CBF84

PROCESS_VM_WRITE = 0x0020
PROCESS_VM_OPERATION = 0x0008


def attach(write: bool = False):
    procs = find_pids("Lemmings")
    if not procs:
        return None, None
    pid, _exe = procs[0]
    access = PROCESS_VM_READ | PROCESS_QUERY_INFORMATION
    if write:
        access |= PROCESS_VM_WRITE | PROCESS_VM_OPERATION
    h = k32.OpenProcess(access, False, pid)
    return (h or None), pid


def u32(h, addr):
    b = read_mem(h, addr, 4)
    return struct.unpack("<I", b)[0] if b and len(b) == 4 else None


def i32(h, addr):
    b = read_mem(h, addr, 4)
    return struct.unpack("<i", b)[0] if b and len(b) == 4 else None


def ui_state(h):
    return u32(h, UI_STATE)


def mouse_obj(h):
    im = u32(h, INPUT_MGR_PTR)
    if not im:
        return None
    return u32(h, im + 0x0C)


def mouse(h):
    """The engine's own view of the mouse, as the poll left it."""
    m = mouse_obj(h)
    if not m:
        return None
    b = read_mem(h, m, 0x28)
    if not b or len(b) < 0x28:
        return None
    btn0, btn1, prev0, prev1 = struct.unpack_from("<4i", b, 0)
    edge0, edge1 = b[0x10], b[0x11]
    t0, t1 = struct.unpack_from("<2i", b, 0x14)
    click0, click1 = b[0x1C], b[0x1D]
    dx, dy = struct.unpack_from("<2i", b, 0x20)
    return dict(
        addr=m, btn0=btn0, btn1=btn1, prev0=prev0, prev1=prev1,
        edge0=edge0, edge1=edge1, t0=t0, t1=t1,
        click0=click0, click1=click1, dx=dx, dy=dy,
    )


def cursor_block(h, size: int = 0x40):
    return read_mem(h, CURSOR_OBJ, size)


def main() -> int:
    h, pid = attach()
    if not h:
        print("the game is not running")
        return 1
    print(f"pid {pid}")
    print(f"ui_state       {ui_state(h)}")
    print(f"level ptr      {u32(h, LEVEL_PTR):#010x}")
    m = mouse(h)
    print(f"mouse object   {m['addr']:#010x}" if m else "mouse object   unavailable")
    if m:
        for k in ("btn0", "prev0", "edge0", "click0", "dx", "dy", "t0"):
            print(f"  {k:8s} {m[k]}")
    blk = cursor_block(h)
    if blk:
        print("cursor block @ %#010x" % CURSOR_OBJ)
        for o in range(0, len(blk), 4):
            iv = struct.unpack_from("<i", blk, o)[0]
            fv = struct.unpack_from("<f", blk, o)[0]
            print(f"  +{o:02x}  {iv:>12d}  {fv:>14.4f}")
    k32.CloseHandle(h)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
