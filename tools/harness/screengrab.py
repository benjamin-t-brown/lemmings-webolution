"""Grab the screen the way a human sees it, not through PrintWindow.

`PrintWindow` on this game returns an all-black bitmap -- dgVoodoo presents
through a D3D11 swapchain and nothing of it lands in the window DC. A BitBlt
from the *desktop* DC does capture composited swapchain output, so this is
the capture path that actually works here. It needs the game foreground and
unoccluded, which `goto_level.focus` arranges.
"""
from __future__ import annotations

import ctypes
import ctypes.wintypes as wt
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

user32 = ctypes.WinDLL("user32")
gdi32 = ctypes.WinDLL("gdi32")
SRCCOPY = 0x00CC0020


class BIH(ctypes.Structure):
    _fields_ = [
        ("biSize", wt.DWORD), ("biWidth", wt.LONG), ("biHeight", wt.LONG),
        ("biPlanes", wt.WORD), ("biBitCount", wt.WORD),
        ("biCompression", wt.DWORD), ("biSizeImage", wt.DWORD),
        ("biXPelsPerMeter", wt.LONG), ("biYPelsPerMeter", wt.LONG),
        ("biClrUsed", wt.DWORD), ("biClrImportant", wt.DWORD),
    ]


def desktop_shot():
    from PIL import Image

    w = user32.GetSystemMetrics(0)
    h = user32.GetSystemMetrics(1)
    src = user32.GetDC(0)
    mem = gdi32.CreateCompatibleDC(src)
    bmp = gdi32.CreateCompatibleBitmap(src, w, h)
    old = gdi32.SelectObject(mem, bmp)
    gdi32.BitBlt(mem, 0, 0, w, h, src, 0, 0, SRCCOPY)
    bih = BIH(biSize=40, biWidth=w, biHeight=-h, biPlanes=1, biBitCount=24)
    row = (w * 3 + 3) & ~3
    buf = (ctypes.c_ubyte * (row * h))()
    gdi32.GetDIBits(mem, bmp, 0, h, buf, ctypes.byref(bih), 0)
    gdi32.SelectObject(mem, old)
    gdi32.DeleteObject(bmp)
    gdi32.DeleteDC(mem)
    user32.ReleaseDC(0, src)
    return Image.frombytes("RGB", (w, h), bytes(buf), "raw", "BGR", row, 1)


def main() -> int:
    out = Path(sys.argv[1] if len(sys.argv) > 1
               else r"C:\progs\lemmings-webolution\build\live_dump\screen.png")
    out.parent.mkdir(parents=True, exist_ok=True)
    img = desktop_shot()
    img.save(out)
    small = img.resize((64, 36))
    mean = sum(sum(p) for p in small.getdata()) / (64 * 36 * 3)
    print(f"{img.size} mean={mean:.1f} -> {out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
