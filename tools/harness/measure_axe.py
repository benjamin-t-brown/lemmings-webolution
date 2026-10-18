"""Round 23: what does Axe do, and how is it detected?

Stock exe. Axe is a HUD sprite sheet (SPRITES\\AXE.SPR), registered via
the sprite-sheet path; sole absolute global ref is a UI bind, not an SFX
play and not a lemming gadget.

    python tools/harness/measure_axe.py --audit

Predictions in docs/scratch/phase3-round23-axe.md.
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

CUE_STR = 0x004C0D6C
CUE_OBJ = 0x004C6A08
SLOT_AXE = 0x1C914
GLOBAL_AXE = CUE_OBJ + SLOT_AXE  # 0x004E331C
GLOBAL_FIRE = CUE_OBJ + 0x1C8FC  # 0x004E3304
GLOBAL_BALLOON = CUE_OBJ + 0x1C8F8  # 0x004E3300
GLOBAL_WITCH = CUE_OBJ + 0x1C8A8  # 0x004E32B0
GLOBAL_SPEED = CUE_OBJ + 0x1C8F0  # 0x004E32F8
GLOBAL_ACTION = CUE_OBJ + 0x1C8F4  # 0x004E32FC

# Axe +0x1c914 sites: register / preload / two unload-family ops.
REG_WRITE = 0x0045E7D7
PRELOAD = 0x0045EBEB
UNLOAD_A = 0x0045EEDC
UNLOAD_B = 0x0045EEF7
EXPECTED_SLOT_SITES = {REG_WRITE, PRELOAD, UNLOAD_A, UNLOAD_B}

SPRITE_REG = 0x00446040
BIND_FN = 0x00496E39
BIND_LOAD = 0x00496E40  # mov eax, [DAT_004E331C]
BIND_HELPER = 0x00496D5B
SFX_PLAY = 0x0044535B
SFX_PLAY_ALT = 0x0044521B
UI_CALLER = 0x004194F0

# Neighbor sheet / cursor names in the same string block.
BITS_STR = 0x004C0D3C
GREY_STR = 0x004C0D44
BLACK_STR = 0x004C0D50
ADD_STR = 0x004C0D5C
SMOKE_STR = 0x004C0D70

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

    off = CUE_STR - 0x400000
    if raw[off : off + 4] != b"Axe\x00":
        print(f"FAIL Axe string missing at {CUE_STR:#x}")
        return 1
    # Neighborhood: AdditiveIcons ... Axe ... Smoke
    if b"AdditiveIcons\x00\x00\x00Axe\x00Smoke\x00" not in raw[off - 32 : off + 16]:
        print("FAIL Axe not beside AdditiveIcons/Smoke sheet names")
        return 1
    print(f"OK cue/sheet string Axe at {CUE_STR:#x} (HUD sheet neighborhood)")

    str_refs = find_imm32(text, text_va, CUE_STR)
    if set(str_refs) != {0x0045E7C4}:
        print(f"FAIL Axe string imm refs {list(map(hex, str_refs))}")
        return 1
    print(f"OK string imm refs {[hex(x) for x in str_refs]}")

    # Register path: push Axe; mov ecx, 0x58d23c; call SPRITE_REG; store +0x1c914
    reg_off = 0x0045E7C3 - text_va
    reg_bytes = text[reg_off : reg_off + 20]
    if reg_bytes[:5] != bytes.fromhex("686c0d4c00"):
        print(f"FAIL register push bytes {reg_bytes[:5].hex()}")
        return 1
    if struct.unpack_from("<I", reg_bytes, 6)[0] != 0x0058D23C:
        print("FAIL register ecx immediate not 0x58d23c")
        return 1
    axe_reg_calls = [
        va for va in call_sites(text, text_va, SPRITE_REG)
        if 0x0045E7C3 <= va <= 0x0045E7D5
    ]
    if not axe_reg_calls:
        print("FAIL no FUN_00446040 call after Axe push")
        return 1
    print(f"OK sprite registrar FUN_00446040 at {[hex(x) for x in axe_reg_calls]}")

    slot_refs = find_imm32(text, text_va, SLOT_AXE)
    if set(slot_refs) != EXPECTED_SLOT_SITES:
        print(f"FAIL +0x1c914 sites {[hex(x) for x in slot_refs]} "
              f"expected {[hex(x) for x in sorted(EXPECTED_SLOT_SITES)]}")
        return 1
    print(f"OK +0x1c914 register/preload/unload-family "
          f"{[hex(x) for x in sorted(slot_refs)]}")

    g_refs = find_imm32(text, text_va, GLOBAL_AXE)
    if set(g_refs) != {BIND_LOAD + 1}:
        print(f"FAIL DAT_004E331C sites {[hex(x) for x in g_refs]} "
              f"expected [{BIND_LOAD + 1:#x}]")
        return 1
    print(f"OK sole absolute Axe global ref at bind load {BIND_LOAD:#x}")

    play_off = BIND_LOAD - text_va
    play_bytes = text[play_off : play_off + 15]
    if play_bytes[:5] != bytes.fromhex("a11c334e00"):
        print(f"FAIL bind load bytes {play_bytes[:5].hex()}")
        return 1
    bind_calls = call_sites(text, text_va, BIND_HELPER)
    if BIND_LOAD + 9 not in bind_calls:  # call at 0x496e49
        near = [va for va in bind_calls if BIND_FN <= va <= BIND_FN + 0x20]
        if not near:
            print(f"FAIL no FUN_00496d5b call near {BIND_LOAD:#x}")
            return 1
        print(f"OK FUN_00496d5b near bind {[hex(x) for x in near]}")
    else:
        print(f"OK FUN_00496d5b call at {BIND_LOAD + 9:#x}")

    callers = call_sites(text, text_va, BIND_FN)
    if set(callers) != {0x00419506}:
        print(f"FAIL FUN_00496e39 callers {list(map(hex, callers))}")
        return 1
    ui_callers = call_sites(text, text_va, UI_CALLER)
    if 0x00418E5B not in ui_callers:
        print(f"FAIL FUN_004194f0 not called from UI init; "
              f"got {list(map(hex, ui_callers))}")
        return 1
    print("OK bind chain FUN_004188f6 -> FUN_004194f0 -> FUN_00496e39")

    # Not SFX play helpers.
    for helper, label in ((SFX_PLAY, "FUN_0044535b"),
                          (SFX_PLAY_ALT, "FUN_0044521b")):
        bad = []
        for va in call_sites(text, text_va, helper):
            window = text[max(0, va - text_va - 80) : va - text_va]
            if struct.pack("<I", GLOBAL_AXE) in window:
                bad.append(va)
        if bad:
            print(f"FAIL {label} loads Axe at {list(map(hex, bad))}")
            return 1
    print("OK no SFX play helper loads DAT_004E331C")

    # Contrast: Balloon/Fire unused; Witch/Speed/Action referenced.
    speed_refs = find_imm32(text, text_va, GLOBAL_SPEED)
    action_refs = find_imm32(text, text_va, GLOBAL_ACTION)
    balloon_refs = find_imm32(text, text_va, GLOBAL_BALLOON)
    fire_refs = find_imm32(text, text_va, GLOBAL_FIRE)
    witch_refs = find_imm32(text, text_va, GLOBAL_WITCH)
    if not speed_refs or not action_refs or not witch_refs:
        print("FAIL contrast globals unexpectedly empty")
        return 1
    if balloon_refs or fire_refs:
        print(f"FAIL Balloon/Fire unexpectedly referenced "
              f"balloon={list(map(hex, balloon_refs))} "
              f"fire={list(map(hex, fire_refs))}")
        return 1
    print(f"OK contrast SpeedPad={len(speed_refs)} Action={len(action_refs)} "
          f"Witch={len(witch_refs)} Balloon=0 Fire=0 Axe=1")

    # Sibling sheet strings also go through the sprite registrar batch.
    for label, sva in (("Bits", BITS_STR), ("GreyIcons", GREY_STR),
                       ("BlackIcons", BLACK_STR), ("AdditiveIcons", ADD_STR),
                       ("Smoke", SMOKE_STR)):
        refs = find_imm32(text, text_va, sva)
        if not refs:
            print(f"FAIL missing sibling sheet string refs for {label}")
            return 1
    print("OK sibling sheet strings Bits/Grey/Black/Additive/Smoke present")

    a3d = []
    spr = []
    tex = []
    wav = []
    for box_name in ("lemmings.box", "patch.box"):
        path = game_dir / box_name
        if not path.exists():
            print(f"FAIL missing {path}")
            return 1
        box = Box(str(path))
        for name in box.names:
            low = name.lower().replace("/", "\\")
            if "axe" not in low:
                continue
            if low.endswith(".a3d"):
                a3d.append(name)
            elif low.endswith(".spr"):
                spr.append(name)
            elif low.endswith(".bmp"):
                tex.append(name)
            elif low.endswith(".wav"):
                wav.append(name)
    if a3d:
        print(f"FAIL unexpected axe a3d {a3d}")
        return 1
    if wav:
        print(f"FAIL unexpected axe wav {wav}")
        return 1
    if not any(s.upper().endswith("AXE.SPR") for s in spr):
        print(f"FAIL expected SPRITES\\AXE.SPR, got {spr}")
        return 1
    if not any("$AXE0.BMP" in t.upper() or "AXE0.BMP" in t.upper() for t in tex):
        print(f"FAIL expected TEXTURES\\SHEETS\\$AXE0.BMP, got {tex}")
        return 1
    # SPR should embed the sheet path.
    box = Box(str(game_dir / "lemmings.box"))
    spr_blob = box.read("SPRITES\\AXE.SPR")
    if b"$Axe0" not in spr_blob and b"$AXE0" not in spr_blob.upper():
        print("FAIL AXE.SPR does not reference $Axe0 sheet")
        return 1
    print(f"OK no axe a3d/wav; spr={spr}; tex={tex}")

    print("AUDIT PASS: Axe is HUD sprite sheet only (no lemming gadget)")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--audit", action="store_true", help="offline Axe audit")
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
