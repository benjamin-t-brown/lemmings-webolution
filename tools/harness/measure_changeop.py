"""Round 27: what does Changeop do, and when is the cue played?

Stock exe. Cue Changeop is registered and played from skill-panel
select FUN_0045ac5c and options SFX-slider FUN_0048e8d5 — UI SFX only,
no changeop.a3d / no Changeop gadget.

    python tools/harness/measure_changeop.py --audit

Predictions in docs/scratch/phase3-round27-changeop.md.
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

CUE_STR = 0x004C0CB4
CUE_OBJ = 0x004C6A08
SLOT_CHANGEOP = 0x1C8C8
GLOBAL_CHANGEOP = CUE_OBJ + SLOT_CHANGEOP  # 0x004E32D0
GLOBAL_BALLOON = CUE_OBJ + 0x1C8F8  # 0x004E3300
GLOBAL_FORCEF = CUE_OBJ + 0x1C8D0  # 0x004E32D8

REG_WRITE = 0x0045E5E4
PRELOAD = 0x0045EA0D
UNLOAD = 0x0045F082
EXPECTED_SLOT_SITES = {REG_WRITE, PRELOAD, UNLOAD}

PLAY_LOAD_A = 0x0045AD07  # imm32 of mov edx,[DAT_004E32D0]
PLAY_INSN_A = 0x0045AD05
PLAY_CALL_A = 0x0045AD11  # call FUN_0044521b
STEP_A = 0x0045AC5C
ALT_PLAY = 0x0044521B

PLAY_LOAD_B = 0x0048EB0C  # imm32 of mov eax,[DAT_004E32D0]
PLAY_INSN_B = 0x0048EB0B
PLAY_CALL_B_LOOKUP = 0x0048EB16  # call FUN_00404cc0
PLAY_CALL_B_START = 0x0048EB1D  # call FUN_00497f39
STEP_B = 0x0048E8D5
LOOKUP = 0x00404CC0
PLAY_START = 0x00497F39
VT_SLOT = 0x004B24D4

ACTOR_LIST = 0x004CBB18
SKILL_PANEL = 0x004C91D8

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
    if raw[off : off + 9] != b"Changeop\x00":
        print(f"FAIL Changeop cue string missing at {CUE_STR:#x}")
        return 1
    if b"Ohno\x00\x00\x00\x00Changeop\x00\x00\x00\x00Splat" not in raw[off - 16 : off + 24]:
        print("FAIL Changeop not beside Ohno/Splat SFX cues")
        return 1
    print(f"OK cue string Changeop at {CUE_STR:#x} (SFX neighborhood)")

    str_refs = find_imm32(text, text_va, CUE_STR)
    if set(str_refs) not in ({0x0045E5D1}, {0x0045E5D0}):
        print(f"FAIL Changeop string imm refs {list(map(hex, str_refs))}")
        return 1
    print(f"OK string imm refs {[hex(x) for x in str_refs]}")

    slot_refs = set(find_imm32(text, text_va, SLOT_CHANGEOP))
    if slot_refs != EXPECTED_SLOT_SITES:
        print(f"FAIL +0x1c8c8 sites {[hex(x) for x in sorted(slot_refs)]} "
              f"expected {[hex(x) for x in sorted(EXPECTED_SLOT_SITES)]}")
        return 1
    print(f"OK +0x1c8c8 only register/preload/unload "
          f"{[hex(x) for x in sorted(slot_refs)]}")

    g_refs = set(find_imm32(text, text_va, GLOBAL_CHANGEOP))
    if g_refs != {PLAY_LOAD_A, PLAY_LOAD_B}:
        print(f"FAIL DAT_004E32D0 sites {[hex(x) for x in sorted(g_refs)]} "
              f"expected {[hex(PLAY_LOAD_A), hex(PLAY_LOAD_B)]}")
        return 1
    print(f"OK absolute Changeop global refs at play loads "
          f"{PLAY_INSN_A:#x} and {PLAY_INSN_B:#x}")

    # Play A: gate + FUN_0044521b
    gate_off = 0x0045ACFA - text_va
    if text[gate_off : gate_off + 7] != bytes.fromhex("833d086a4c0002"):
        print(f"FAIL skill-panel gate bytes "
              f"{text[gate_off:gate_off + 7].hex()}")
        return 1
    play_a_off = PLAY_INSN_A - text_va
    if text[play_a_off : play_a_off + 6] != bytes.fromhex("8b15d0324e00"):
        print(f"FAIL play-A load bytes {text[play_a_off:play_a_off + 6].hex()}")
        return 1
    if PLAY_CALL_A not in call_sites(text, text_va, ALT_PLAY):
        print(f"FAIL missing FUN_0044521b at {PLAY_CALL_A:#x}")
        return 1
    print(f"OK play A: gate [0x4c6a08]==2 then FUN_0044521b inside "
          f"{STEP_A:#x}")

    # Play B: lookup + start
    play_b_off = PLAY_INSN_B - text_va
    if text[play_b_off : play_b_off + 5] != bytes.fromhex("a1d0324e00"):
        print(f"FAIL play-B load bytes {text[play_b_off:play_b_off + 5].hex()}")
        return 1
    if PLAY_CALL_B_LOOKUP not in call_sites(text, text_va, LOOKUP):
        print(f"FAIL missing FUN_00404cc0 at {PLAY_CALL_B_LOOKUP:#x}")
        return 1
    if PLAY_CALL_B_START not in call_sites(text, text_va, PLAY_START):
        print(f"FAIL missing FUN_00497f39 at {PLAY_CALL_B_START:#x}")
        return 1
    print(f"OK play B: FUN_00404cc0 -> FUN_00497f39 inside {STEP_B:#x}")

    # Vtable slot for slider handler
    vt_off = VT_SLOT - 0x400000
    if struct.unpack_from("<I", raw, vt_off)[0] != STEP_B:
        print(f"FAIL vtable {VT_SLOT:#x} not {STEP_B:#x}")
        return 1
    print(f"OK vtable slot {VT_SLOT:#x} -> {STEP_B:#x}")

    # Skill panel object imm in select caller FUN_00455aff
    panel_caller = find_imm32(text, text_va, SKILL_PANEL)
    if not any(0x00455AFF <= va <= 0x00455B60 for va in panel_caller):
        print(f"FAIL skill panel {SKILL_PANEL:#x} missing from FUN_00455aff")
        return 1
    print(f"OK skill-panel object {SKILL_PANEL:#x} referenced from select caller")

    for lo, hi, label in ((STEP_A, 0x0045AE0F, "play A"),
                          (STEP_B, 0x0048EB4C, "play B")):
        actor = [va for va in find_imm32(text, text_va, ACTOR_LIST)
                 if lo <= va <= hi]
        if actor:
            print(f"FAIL actor list imm in {label} {list(map(hex, actor))}")
            return 1
    print(f"OK no actor-list {ACTOR_LIST:#x} imm in either play path")

    # Contrast: Balloon / ForceF still unused
    if find_imm32(text, text_va, GLOBAL_BALLOON):
        print("FAIL Balloon global unexpectedly referenced")
        return 1
    if find_imm32(text, text_va, GLOBAL_FORCEF):
        print("FAIL ForceF global unexpectedly referenced")
        return 1
    print("OK contrast: Balloon/ForceF still unused")

    a3d = []
    wav = []
    other = []
    lvl_a3d = 0
    for box_name in ("lemmings.box", "patch.box"):
        path = game_dir / box_name
        if not path.exists():
            print(f"FAIL missing {path}")
            return 1
        box = Box(str(path))
        for name in box.names:
            low = name.lower().replace("/", "\\")
            if "changeop" not in low:
                continue
            if low.endswith(".a3d"):
                a3d.append(name)
            elif low.endswith(".wav"):
                wav.append(name)
            else:
                other.append(name)
        for name in box.names:
            low = name.lower().replace("/", "\\")
            if "compiled" not in low or not low.endswith(".lvl"):
                continue
            data = box.read(name)
            if b"changeop.a3d" in data.lower():
                lvl_a3d += 1
    if a3d or lvl_a3d:
        print(f"FAIL unexpected changeop a3d box={a3d} lvl={lvl_a3d}")
        return 1
    if not any(w.upper().endswith("CHANGEOP.WAV") for w in wav):
        print(f"FAIL expected SOUNDS\\CHANGEOP.WAV, got {wav}")
        return 1
    bad_other = [o for o in other if "texture" not in o.lower()]
    if bad_other:
        print(f"FAIL unexpected changeop non-wav assets {bad_other}")
        return 1
    print(f"OK no changeop.a3d; WAV={wav}")

    print("AUDIT PASS: Changeop is UI skill/volume SFX; no Changeop gadget")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--audit", action="store_true",
                    help="offline Changeop play-path audit")
    ap.add_argument("--game", type=Path, default=GAME)
    ap.add_argument("--exe", type=Path, default=None,
                    help="stock Lemmings Revolution.exe (default: locate)")
    args = ap.parse_args()
    if not args.audit:
        ap.error("use --audit (live watch not required; settled by static reads)")
    exe = args.exe if args.exe else locate_exe(None)
    return audit(Path(exe), args.game)


if __name__ == "__main__":
    raise SystemExit(main())
