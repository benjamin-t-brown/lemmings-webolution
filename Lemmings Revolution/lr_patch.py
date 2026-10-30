#!/usr/bin/env python3
"""
Resolution patcher for Lemmings Revolution (lrpc, 2000, MSVC x86).

The game picks its display mode in exactly one place:

    .text:0045F1FE   push 1E0h            ; height = 480
    .text:0045F203   push 280h            ; width  = 640
    .text:0045F208   mov  ecx, 58CD80h    ; renderer singleton
    .text:0045F20D   call 0046EA17h       ; Renderer::Init(w, h, bpp, bWindowed)

Everything downstream is derived from those two numbers:
  * IDirectDraw4::EnumDisplayModes builds a full mode list with no filtering,
    and FindMode(w, h, bpp) looks up the requested one (0047BF16).
  * IDirectDraw4::SetDisplayMode(w, h, 16, refresh, 0) at 0046F850.
  * Primary / back / z surfaces are created at w x h (0046F507).
  * D3DVIEWPORT2 is dwWidth=w, dwHeight=h with a normalised [-1,1] clip
    volume (004702A7), so the 3D scene renders natively at any resolution
    with an unchanged field of view.

Two further sites set the mouse-cursor aspect ratio as width/height
(Cursor::SetAspect, 0047B178). Originally that equalled renderW/renderH,
so we keep the relationship and patch them to match.

Nothing else in .text references 640 or 480 for the render path, so the
resolution is genuinely just these constants.

=============================================================================
IMPORTANT: PATCH TO YOUR DESKTOP RESOLUTION, NOTHING ELSE
=============================================================================
The game maps its own pixel space 1:1 onto the Win32 window starting at
(0, 0). It has no notion of a scale factor or a letterbox margin:

  * 0058CE0C / 0058CE10 hold the window origin, and are 0,0 in fullscreen.
  * 0058CB40 / 0058CB44 hold the cursor bounds, and equal renderW/renderH.
  * 0058D294 / 0058D298 hold the cursor centre, and equal renderW/2, renderH/2.

dgVoodoo's "fake" fullscreen always creates a *desktop-sized* window and
scales the rendered image into it, centred. So whenever renderW/H differs
from the desktop, every click is off by the letterbox margin.

Measured on a 2560x1440 desktop:

  render 1920x1440 -> image pillarboxed, drawn menu centre at screen x=1287
                      but the game hit-tests at its own centre x=960.
                      Every click is 320 px = (2560-1920)/2 out.
  render 2560x1440 -> drawn menu centre x=1285, game centre x=1280. Correct.

So: set width/height to your actual desktop resolution. (If you want a
non-native resolution, you must stop dgVoodoo scaling instead -- clear
FullscreenAttributes in dgVoodoo.conf so it performs a real display-mode
switch and the window matches the render size again. That resizes your
desktop while the game runs.)

Also note the 2D layer (menus, HUD, text) does NOT scale with resolution.
Vertical positions are absolute 640x480-era pixel values and horizontal
positions are centred on renderW, so at 1440p the interface is a small
island near the top-middle. The 3D scene does scale correctly and keeps
its aspect, so it renders natively and sharp at any resolution.
"""

import argparse
import ctypes
import shutil
import struct
import sys
from pathlib import Path


def desktop_size():
    user32 = ctypes.windll.user32
    user32.SetProcessDPIAware()
    return user32.GetSystemMetrics(0), user32.GetSystemMetrics(1)

HERE = Path(__file__).resolve().parent
DEFAULT_SRC = HERE / "Lemmings Revolution.exe"

# (file offset, label). Each site is `push <height>; push <width>` =
# 68 e0 01 00 00 68 80 02 00 00
SITES = [
    (0x0005F1FE, "Renderer::Init display mode"),
    (0x0005C910, "cursor aspect (in-game)"),
    (0x0004E619, "cursor aspect (frontend)"),
]
EXPECT = bytes.fromhex("68 e0 01 00 00 68 80 02 00 00".replace(" ", ""))


def patch(src: Path, dst: Path, width: int, height: int, keep_aspect: bool) -> None:
    buf = bytearray(src.read_bytes())

    for off, label in SITES:
        found = bytes(buf[off:off + len(EXPECT)])
        if found != EXPECT:
            sys.exit(
                "unexpected bytes at 0x%08X (%s):\n  want %s\n  got  %s\n"
                "This is not the expected 851968-byte build; aborting."
                % (off, label, EXPECT.hex(" "), found.hex(" "))
            )

    w, h = width, height
    for off, label in SITES:
        aw, ah = (640, 480) if (keep_aspect and "aspect" in label) else (w, h)
        struct.pack_into("<I", buf, off + 1, ah)
        struct.pack_into("<I", buf, off + 6, aw)
        print("  0x%08X  %-28s -> %dx%d" % (off, label, aw, ah))

    dst.write_bytes(buf)


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__.strip().splitlines()[0])
    ap.add_argument("width", type=int, nargs="?",
                    help="render width (default: your desktop width)")
    ap.add_argument("height", type=int, nargs="?",
                    help="render height (default: your desktop height)")
    ap.add_argument("-i", "--input", type=Path, default=DEFAULT_SRC,
                    help="clean source exe (default: %(default)s)")
    ap.add_argument("-o", "--output", type=Path,
                    help="output exe (default: 'Lemmings Revolution <WxH>.exe')")
    ap.add_argument("--keep-cursor-aspect", action="store_true",
                    help="leave the mouse aspect ratio at 4:3 instead of "
                         "matching the new resolution")
    args = ap.parse_args()

    dw, dh = desktop_size()
    if args.width is None or args.height is None:
        args.width, args.height = dw, dh
        print("no resolution given; using desktop %dx%d" % (dw, dh))
    elif (args.width, args.height) != (dw, dh):
        print("WARNING: %dx%d is not your desktop resolution (%dx%d)."
              % (args.width, args.height, dw, dh))
        print("         dgVoodoo will scale and centre the image, but the game")
        print("         hit-tests in unscaled pixels anchored at (0,0), so clicks")
        print("         will be off by %d px horizontally and %d px vertically."
              % (abs(dw - args.width) // 2, abs(dh - args.height) // 2))
        print("         See the module docstring for the only way around this.")

    src = args.input
    if not src.is_file():
        sys.exit("no such file: %s" % src)

    dst = args.output or (HERE / ("Lemmings Revolution %dx%d.exe" % (args.width, args.height)))

    backup = HERE / "Troubleshooting Backup" / "Lemmings Revolution.exe.orig"
    if src == DEFAULT_SRC and not backup.exists():
        backup.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(src, backup)
        print("backed up original -> %s" % backup)

    print("patching %s -> %s" % (src.name, dst.name))
    patch(src, dst, args.width, args.height, args.keep_cursor_aspect)
    print("done")


if __name__ == "__main__":
    main()
