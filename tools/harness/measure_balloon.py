"""Round 18: what does a Balloon do, and how is it detected?

Stock exe. Clean-negative audit: cue registered, never played; no balloon.a3d.

    python tools/harness/measure_balloon.py --audit

Predictions in docs/scratch/phase3-round18-balloon.md.
No live writes; --audit is offline.
"""
from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "proto"))

from box import Box  # noqa: E402
from read_steppers import EXPECTED_SHA256, locate_exe  # noqa: E402

CUE_STR = 0x004C0D24
CUE_OBJ = 0x004C6A08
SLOT_BALLOON = 0x1C8F8
SLOT_ACTION = 0x1C8F4
GLOBAL_BALLOON = CUE_OBJ + SLOT_BALLOON  # 0x004E3300
GLOBAL_ACTION = CUE_OBJ + SLOT_ACTION    # 0x004E32FC
GLOBAL_SPEED = CUE_OBJ + 0x1C8F0         # 0x004E32F8

# Balloon +0x1c8f8 sites inside FUN_0045e51f / FUN_0045ed81 only.
REG_WRITE = 0x0045E704
PRELOAD = 0x0045EB2B
UNLOAD = 0x0045F172
EXPECTED_SLOT_SITES = {REG_WRITE, PRELOAD, UNLOAD}

PLAY_FUN = 0x0044535B

GAME = Path(r"C:\progs\Lemmings Revolution")


def pe_text(data: bytes):
    e_lfanew = struct.unpack_from("<I", data, 0x3C)[0]
    num = struct.unpack_from("<H", data, e_lfanew + 6)[0]
    opt = struct.unpack_from("<H", data, e_lfanew + 20)[0]
    sec = e_lfanew + 24 + opt
    image_base = struct.unpack_from("<I", data, e_lfanew + 24 + 28)[0]
    for i in range(num):
        o = sec + i * 40
        name = data[o : o + 8].split(b"\x00")[0]
        vsize, va, rawsize, rawptr = struct.unpack_from("<IIII", data, o + 8)
        if name == b".text":
            return data[rawptr : rawptr + rawsize], image_base + va
    raise SystemExit(".text missing")


def find_imm32(text: bytes, text_va: int, value: int) -> list[int]:
    needle = struct.pack("<I", value)
    out = []
    idx = 0
    while True:
        i = text.find(needle, idx)
        if i < 0:
            break
        out.append(text_va + i)
        idx = i + 1
    return out


def call_sites(text: bytes, text_va: int, target: int) -> list[int]:
    hits = []
    for i in range(len(text) - 5):
        if text[i] != 0xE8:
            continue
        rel = struct.unpack_from("<i", text, i + 1)[0]
        if text_va + i + 5 + rel == target:
            hits.append(text_va + i)
    return hits


def audit(exe_path: Path, game_dir: Path) -> int:
    raw = exe_path.read_bytes()
    import hashlib

    digest = hashlib.sha256(raw).hexdigest()
    if digest != EXPECTED_SHA256:
        print(f"FAIL unexpected exe sha256 {digest}")
        return 1

    text, text_va = pe_text(raw)

    # Cue string bytes (inside .data rawsize 0x7000 window — VA check only).
    # File offset for 0x4C0D24: .data va=0xbf000 rawptr=0xbf000 → off 0xC0D24-0x400000
    # = 0x80D24 relative to image; simpler: search ASCII.
    if b"Balloon\x00Fire" not in raw:
        print("FAIL Balloon cue string missing")
        return 1
    print(f"OK cue string Balloon at lead VA {CUE_STR:#x}")

    str_refs = find_imm32(text, text_va, CUE_STR)
    if str_refs != [0x0045E6F1]:
        # push imm sits at 0x45E6F0; imm32 payload at +1
        if str_refs != [0x0045E6F1] and str_refs != [0x0045E6F0]:
            print(f"FAIL Balloon string imm refs {list(map(hex, str_refs))}")
            return 1
    print(f"OK string imm refs {[hex(x) for x in str_refs]}")

    slot_refs = find_imm32(text, text_va, SLOT_BALLOON)
    if set(slot_refs) != EXPECTED_SLOT_SITES:
        print(f"FAIL +0x1c8f8 sites {[hex(x) for x in slot_refs]} "
              f"expected {[hex(x) for x in sorted(EXPECTED_SLOT_SITES)]}")
        return 1
    print(f"OK +0x1c8f8 only register/preload/unload "
          f"{[hex(x) for x in sorted(slot_refs)]}")

    g_refs = find_imm32(text, text_va, GLOBAL_BALLOON)
    if g_refs:
        print(f"FAIL DAT_004E3300 play/read sites {[hex(x) for x in g_refs]}")
        return 1
    print(f"OK no absolute refs to Balloon global {GLOBAL_BALLOON:#x}")

    # Contrast: SpeedPad and Action globals are referenced (play).
    speed_refs = find_imm32(text, text_va, GLOBAL_SPEED)
    action_refs = find_imm32(text, text_va, GLOBAL_ACTION)
    if not speed_refs:
        print("FAIL SpeedPad global unexpectedly unreferenced (contrast broken)")
        return 1
    if not action_refs:
        print("FAIL Action global unexpectedly unreferenced (contrast broken)")
        return 1
    print(f"OK contrast SpeedPad refs={len(speed_refs)} Action refs={len(action_refs)}")

    plays = call_sites(text, text_va, PLAY_FUN)
    balloon_play = []
    for va in plays:
        off = va - text_va
        window = text[max(0, off - 120) : off]
        if struct.pack("<I", GLOBAL_BALLOON) in window:
            balloon_play.append(va)
        if struct.pack("<I", SLOT_BALLOON) in window:
            balloon_play.append(va)
    if balloon_play:
        print(f"FAIL play callers load Balloon {[hex(x) for x in balloon_play]}")
        return 1
    print(f"OK none of {len(plays)} FUN_0044535b callers load Balloon")

    # Boxes: no balloon.a3d; WAV + exit decorations only.
    a3d = []
    wav = []
    exit_mesh = []
    for box_name in ("lemmings.box", "patch.box"):
        path = game_dir / box_name
        if not path.exists():
            print(f"FAIL missing {path}")
            return 1
        box = Box(str(path))
        for name in box.names:
            low = name.lower()
            if "balloon" not in low:
                continue
            if low.endswith(".a3d"):
                a3d.append(name)
            elif low.endswith(".wav"):
                wav.append(name)
            elif "models\\exit" in low.replace("/", "\\") or low.endswith(".bmp"):
                exit_mesh.append(name)
            else:
                # leftover balloon-tagged asset
                exit_mesh.append(name)
    if a3d:
        print(f"FAIL unexpected balloon a3d {a3d}")
        return 1
    if "SOUNDS\\BALLOON.WAV" not in [w.upper() for w in wav] and \
       "sounds\\balloon.wav" not in [w.lower() for w in wav]:
        # accept any case
        if not any(w.lower().endswith("balloon.wav") for w in wav):
            print(f"FAIL expected SOUNDS\\BALLOON.WAV, got {wav}")
            return 1
    print(f"OK no balloon.a3d; wav={wav}; exit/texture decorations={len(exit_mesh)}")

    print("AUDIT PASS: Balloon is cue-only unused (clean negative)")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--audit", action="store_true", help="offline clean-negative audit")
    ap.add_argument("--game", type=Path, default=GAME)
    ap.add_argument("--exe", type=Path, default=None,
                    help="stock Lemmings Revolution.exe (default: locate)")
    args = ap.parse_args()
    if not args.audit:
        ap.error("use --audit (live watch not applicable for this clean negative)")
    exe = args.exe if args.exe else locate_exe(None)
    return audit(Path(exe), args.game)


if __name__ == "__main__":
    raise SystemExit(main())
