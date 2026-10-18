"""Round 19: what does Fire do, and how is it detected?

Stock exe. Clean-negative audit: cue registered, never played; no fire.a3d.

    python tools/harness/measure_fire.py --audit

Predictions in docs/scratch/phase3-round19-fire.md.
No live writes; --audit is offline.
"""
from __future__ import annotations

import argparse
import hashlib
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "proto"))

from box import Box  # noqa: E402
from read_steppers import EXPECTED_SHA256, locate_exe  # noqa: E402

CUE_STR = 0x004C0D2C
CUE_OBJ = 0x004C6A08
SLOT_FIRE = 0x1C8FC
SLOT_BALLOON = 0x1C8F8
GLOBAL_FIRE = CUE_OBJ + SLOT_FIRE  # 0x004E3304
GLOBAL_BALLOON = CUE_OBJ + SLOT_BALLOON  # 0x004E3300
GLOBAL_ACTION = CUE_OBJ + 0x1C8F4  # 0x004E32FC
GLOBAL_SPEED = CUE_OBJ + 0x1C8F0  # 0x004E32F8

# Fire +0x1c8fc sites inside FUN_0045e51f / FUN_0045ed81 only.
REG_WRITE = 0x0045E71C
PRELOAD = 0x0045EB41
UNLOAD = 0x0045F186
EXPECTED_SLOT_SITES = {REG_WRITE, PRELOAD, UNLOAD}

PLAY_FUN = 0x0044535B

GAME = Path(r"C:\progs\Lemmings Revolution")

A3D_NEEDLES = ("fire", "flame", "burn", "torch", "lava")


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
    digest = hashlib.sha256(raw).hexdigest()
    if digest != EXPECTED_SHA256:
        print(f"FAIL unexpected exe sha256 {digest}")
        return 1

    text, text_va = pe_text(raw)

    if b"Balloon\x00Fire\x00" not in raw:
        print("FAIL Fire cue string missing after Balloon")
        return 1
    print(f"OK cue string Fire at lead VA {CUE_STR:#x}")

    str_refs = find_imm32(text, text_va, CUE_STR)
    # push imm at 0x45E708; imm32 payload at +1
    if set(str_refs) not in ({0x0045E709}, {0x0045E708}):
        print(f"FAIL Fire string imm refs {list(map(hex, str_refs))}")
        return 1
    print(f"OK string imm refs {[hex(x) for x in str_refs]}")

    slot_refs = find_imm32(text, text_va, SLOT_FIRE)
    if set(slot_refs) != EXPECTED_SLOT_SITES:
        print(f"FAIL +0x1c8fc sites {[hex(x) for x in slot_refs]} "
              f"expected {[hex(x) for x in sorted(EXPECTED_SLOT_SITES)]}")
        return 1
    print(f"OK +0x1c8fc only register/preload/unload "
          f"{[hex(x) for x in sorted(slot_refs)]}")

    g_refs = find_imm32(text, text_va, GLOBAL_FIRE)
    if g_refs:
        print(f"FAIL DAT_004E3304 play/read sites {[hex(x) for x in g_refs]}")
        return 1
    print(f"OK no absolute refs to Fire global {GLOBAL_FIRE:#x}")

    # Contrast: SpeedPad and Action globals are referenced (play).
    # Balloon global should also be unused (round 18 sibling).
    speed_refs = find_imm32(text, text_va, GLOBAL_SPEED)
    action_refs = find_imm32(text, text_va, GLOBAL_ACTION)
    balloon_refs = find_imm32(text, text_va, GLOBAL_BALLOON)
    if not speed_refs:
        print("FAIL SpeedPad global unexpectedly unreferenced (contrast broken)")
        return 1
    if not action_refs:
        print("FAIL Action global unexpectedly unreferenced (contrast broken)")
        return 1
    if balloon_refs:
        print(f"FAIL Balloon global unexpectedly referenced {list(map(hex, balloon_refs))}")
        return 1
    print(f"OK contrast SpeedPad refs={len(speed_refs)} Action refs={len(action_refs)} "
          f"Balloon refs=0")

    plays = call_sites(text, text_va, PLAY_FUN)
    fire_play = []
    for va in plays:
        off = va - text_va
        window = text[max(0, off - 120) : off]
        if struct.pack("<I", GLOBAL_FIRE) in window:
            fire_play.append(va)
        if struct.pack("<I", SLOT_FIRE) in window:
            fire_play.append(va)
    if fire_play:
        print(f"FAIL play callers load Fire {[hex(x) for x in fire_play]}")
        return 1
    print(f"OK none of {len(plays)} FUN_0044535b callers load Fire")

    a3d = []
    wav = []
    spr = []
    other = []
    for box_name in ("lemmings.box", "patch.box"):
        path = game_dir / box_name
        if not path.exists():
            print(f"FAIL missing {path}")
            return 1
        box = Box(str(path))
        for name in box.names:
            low = name.lower().replace("/", "\\")
            if not any(k in low for k in A3D_NEEDLES):
                continue
            if low.endswith(".a3d"):
                a3d.append(name)
            elif low.endswith(".wav"):
                wav.append(name)
            elif low.endswith(".spr"):
                spr.append(name)
            else:
                other.append(name)
    if a3d:
        print(f"FAIL unexpected fire/flame/burn a3d {a3d}")
        return 1
    if not any(w.lower().endswith("fire.wav") for w in wav):
        print(f"FAIL expected SOUNDS\\FIRE.WAV, got {wav}")
        return 1
    if not any(s.lower().endswith("fire.spr") for s in spr):
        print(f"FAIL expected SPRITES\\FIRE.SPR, got {spr}")
        return 1
    print(f"OK no fire a3d; wav={wav}; spr={spr}; other_fire_tagged={len(other)}")

    print("AUDIT PASS: Fire is cue-only unused (clean negative)")
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
