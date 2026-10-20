"""Static scan for A4h: callers of 0x4682a6 and copy helper 0x455ba0."""
from __future__ import annotations

import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from read_steppers import Image, _raw_e8, _text_bytes  # noqa: E402

TARGET = 0x004682A6
COPY_FN = 0x00455BA0


def main() -> int:
    img = Image(Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe"))
    code_info, _ = _text_bytes(img)
    base, _vsize, _rsize, code, _chars = code_info
    callers = _raw_e8(code, base, TARGET)
    print(f"call 0x{TARGET:08x}: {len(callers)} direct site(s)")
    for site in callers:
        print(f"  {site:#010x}")
    if not callers:
        return 1
    # Disassemble 0x455ba0 prologue (fixed size copy)
    off = COPY_FN - base
    chunk = code[off : off + 32]
    print(f"\n0x{COPY_FN:08x} bytes: {chunk.hex()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
