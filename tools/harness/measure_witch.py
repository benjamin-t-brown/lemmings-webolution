"""Round 20: what does Witch do, and how is it detected?

Stock exe. Cue is registered and played as an e10_kf_1 pause easter egg;
no lemming gadget / no witch.a3d.

    python tools/harness/measure_witch.py --audit

Predictions in docs/scratch/phase3-round20-witch.md.
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

CUE_STR = 0x004C0D34
CUE_OBJ = 0x004C6A08
SLOT_WITCH = 0x1C8A8
SLOT_FIRE = 0x1C8FC
GLOBAL_WITCH = CUE_OBJ + SLOT_WITCH  # 0x004E32B0
GLOBAL_FIRE = CUE_OBJ + SLOT_FIRE  # 0x004E3304
GLOBAL_ACTION = CUE_OBJ + 0x1C8F4  # 0x004E32FC
GLOBAL_SPEED = CUE_OBJ + 0x1C8F0  # 0x004E32F8
GLOBAL_BALLOON = CUE_OBJ + 0x1C8F8  # 0x004E3300

# Witch +0x1c8a8 sites inside FUN_0045e51f / FUN_0045ed81 only.
REG_WRITE = 0x0045E734
PRELOAD = 0x0045EB57
UNLOAD = 0x0045F19A
EXPECTED_SLOT_SITES = {REG_WRITE, PRELOAD, UNLOAD}

PLAY_LOAD = 0x0045D200  # mov ecx, [DAT_004E32B0]; imm32 at +2
PLAY_HELPER = 0x0044521B
E10_PATH_STR = 0x004C0BD0
E10_PATH = b"compiled\\e10_kf_1.lvl"

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
    digest = hashlib.sha256(raw).hexdigest()
    if digest != EXPECTED_SHA256:
        print(f"FAIL unexpected exe sha256 {digest}")
        return 1

    text, text_va = pe_text(raw)

    if b"Fire\x00\x00\x00\x00Witch\x00" not in raw and b"Witch\x00" not in raw[CUE_STR - 0x400000 :]:
        # Prefer exact VA content
        pass
    off = CUE_STR - 0x400000
    if raw[off : off + 6] != b"Witch\x00":
        print(f"FAIL Witch cue string missing at {CUE_STR:#x}")
        return 1
    print(f"OK cue string Witch at {CUE_STR:#x}")

    str_refs = find_imm32(text, text_va, CUE_STR)
    if set(str_refs) not in ({0x0045E721}, {0x0045E720}):
        print(f"FAIL Witch string imm refs {list(map(hex, str_refs))}")
        return 1
    print(f"OK string imm refs {[hex(x) for x in str_refs]}")

    slot_refs = find_imm32(text, text_va, SLOT_WITCH)
    if set(slot_refs) != EXPECTED_SLOT_SITES:
        print(f"FAIL +0x1c8a8 sites {[hex(x) for x in slot_refs]} "
              f"expected {[hex(x) for x in sorted(EXPECTED_SLOT_SITES)]}")
        return 1
    print(f"OK +0x1c8a8 only register/preload/unload "
          f"{[hex(x) for x in sorted(slot_refs)]}")

    # Not the sequential post-Fire slot.
    if SLOT_WITCH == 0x1C900:
        print("FAIL Witch unexpectedly used sequential +0x1c900")
        return 1
    seq_refs = find_imm32(text, text_va, 0x1C900)
    print(f"OK Witch slot is +0x1c8a8 (not +0x1c900); "
          f"+0x1c900 imm count={len(seq_refs)}")

    g_refs = find_imm32(text, text_va, GLOBAL_WITCH)
    if set(g_refs) != {PLAY_LOAD + 2}:
        print(f"FAIL DAT_004E32B0 sites {[hex(x) for x in g_refs]} "
              f"expected [{PLAY_LOAD + 2:#x}]")
        return 1
    print(f"OK sole absolute Witch global ref at play load {PLAY_LOAD:#x}")

    # Bytes at play site: mov ecx,[DAT]; push; mov ecx, cueobj; call helper
    play_off = PLAY_LOAD - text_va
    play_bytes = text[play_off : play_off + 18]
    if play_bytes[:6] != bytes.fromhex("8b0db0324e00"):
        print(f"FAIL play load bytes {play_bytes[:6].hex()}")
        return 1
    plays = call_sites(text, text_va, PLAY_HELPER)
    if PLAY_LOAD + 0xC not in plays:  # call at 0x45d20c
        # accept either exact 0x45d20c or nearby call after the mov
        witch_calls = [va for va in plays if PLAY_LOAD <= va <= PLAY_LOAD + 0x20]
        if not witch_calls:
            print(f"FAIL no FUN_0044521b call near {PLAY_LOAD:#x}; "
                  f"plays={[hex(x) for x in plays]}")
            return 1
        print(f"OK FUN_0044521b call near play site {[hex(x) for x in witch_calls]}")
    else:
        print(f"OK FUN_0044521b call at {PLAY_LOAD + 0xC:#x}")

    e10_off = E10_PATH_STR - 0x400000
    if raw[e10_off : e10_off + len(E10_PATH) + 1] != E10_PATH + b"\x00":
        print(f"FAIL e10 path string missing at {E10_PATH_STR:#x}")
        return 1
    e10_refs = find_imm32(text, text_va, E10_PATH_STR)
    if 0x0045D1E9 not in e10_refs and 0x0045D1E8 not in e10_refs:
        print(f"FAIL e10 path imm refs {list(map(hex, e10_refs))}")
        return 1
    print(f"OK e10 path string + imm near pause/Witch branch "
          f"{[hex(x) for x in e10_refs]}")

    # Contrast: Balloon/Fire unused; SpeedPad/Action referenced.
    speed_refs = find_imm32(text, text_va, GLOBAL_SPEED)
    action_refs = find_imm32(text, text_va, GLOBAL_ACTION)
    balloon_refs = find_imm32(text, text_va, GLOBAL_BALLOON)
    fire_refs = find_imm32(text, text_va, GLOBAL_FIRE)
    if not speed_refs:
        print("FAIL SpeedPad global unexpectedly unreferenced (contrast broken)")
        return 1
    if not action_refs:
        print("FAIL Action global unexpectedly unreferenced (contrast broken)")
        return 1
    if balloon_refs:
        print(f"FAIL Balloon global unexpectedly referenced {list(map(hex, balloon_refs))}")
        return 1
    if fire_refs:
        print(f"FAIL Fire global unexpectedly referenced {list(map(hex, fire_refs))}")
        return 1
    print(f"OK contrast SpeedPad refs={len(speed_refs)} Action refs={len(action_refs)} "
          f"Balloon=0 Fire=0 Witch=1")

    a3d = []
    wav = []
    tex = []
    for box_name in ("lemmings.box", "patch.box"):
        path = game_dir / box_name
        if not path.exists():
            print(f"FAIL missing {path}")
            return 1
        box = Box(str(path))
        for name in box.names:
            low = name.lower().replace("/", "\\")
            if "witch" not in low:
                continue
            if low.endswith(".a3d"):
                a3d.append(name)
            elif low.endswith(".wav"):
                wav.append(name)
            elif "textures\\models\\witch\\" in low:
                tex.append(name)
    if a3d:
        print(f"FAIL unexpected witch a3d {a3d}")
        return 1
    if not any(w.lower().endswith("witch.wav") for w in wav):
        print(f"FAIL expected SOUNDS\\WITCH.WAV, got {wav}")
        return 1
    if not tex:
        print("FAIL expected TEXTURES\\MODELS\\WITCH\\* orphans")
        return 1
    # No a3d should embed those texture paths.
    consumers = []
    for box_name in ("lemmings.box", "patch.box"):
        box = Box(str(game_dir / box_name))
        for name in box.names:
            if not name.lower().endswith(".a3d"):
                continue
            blob = box.read(name)
            if b"WITCHFROCK" in blob or b"WITCHGOB" in blob or b"MODELS\\WITCH" in blob.upper():
                consumers.append(name)
    if consumers:
        print(f"FAIL a3d consumers of witch textures {consumers}")
        return 1
    print(f"OK no witch a3d; wav={wav}; orphan_tex={len(tex)}; a3d_consumers=0")

    print("AUDIT PASS: Witch is UI easter-egg cue only (no lemming gadget)")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--audit", action="store_true", help="offline Witch audit")
    ap.add_argument("--game", type=Path, default=GAME)
    ap.add_argument("--exe", type=Path, default=None,
                    help="stock Lemmings Revolution.exe (default: locate)")
    args = ap.parse_args()
    if not args.audit:
        ap.error("use --audit (live watch not required for this settlement)")
    exe = args.exe if args.exe else locate_exe(None)
    return audit(Path(exe), args.game)


if __name__ == "__main__":
    raise SystemExit(main())
