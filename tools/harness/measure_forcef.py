"""Round 22: what does ForceF do, and how is it detected?

Stock exe. Clean-negative audit: cue registered, never played; no force.a3d.

    python tools/harness/measure_forcef.py --audit

Predictions in docs/scratch/phase3-round22-forcef.md.
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

CUE_STR = 0x004C0CC8
CUE_OBJ = 0x004C6A08
SLOT_FORCEF = 0x1C8D0
SLOT_DIE = 0x1C8D4
GLOBAL_FORCEF = CUE_OBJ + SLOT_FORCEF  # 0x004E32D8
GLOBAL_DIE = CUE_OBJ + SLOT_DIE  # 0x004E32DC
GLOBAL_ACTION = CUE_OBJ + 0x1C8F4  # 0x004E32FC
GLOBAL_SPEED = CUE_OBJ + 0x1C8F0  # 0x004E32F8
GLOBAL_LEVER = CUE_OBJ + 0x1C8DC  # 0x004E32E4

# ForceF +0x1c8d0 sites inside FUN_0045e51f / FUN_0045ed81 only.
REG_WRITE = 0x0045E614
PRELOAD = 0x0045EA39
UNLOAD = 0x0045F0AA
EXPECTED_SLOT_SITES = {REG_WRITE, PRELOAD, UNLOAD}

PLAY_FUN = 0x0044535B
ALT_PLAY_FUN = 0x0044521B

GAME = Path(r"C:\progs\Lemmings Revolution")

A3D_NEEDLES = ("force", "fan", "blow", "wind")


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


def play_loads_forcef(text: bytes, text_va: int, plays: list[int]) -> list[int]:
    bad = []
    for va in plays:
        off = va - text_va
        window = text[max(0, off - 120) : off]
        if struct.pack("<I", GLOBAL_FORCEF) in window:
            bad.append(va)
        if struct.pack("<I", SLOT_FORCEF) in window:
            bad.append(va)
        if struct.pack("<I", CUE_STR) in window:
            bad.append(va)
    return bad


def audit(exe_path: Path, game_dir: Path) -> int:
    raw = exe_path.read_bytes()
    digest = hashlib.sha256(raw).hexdigest()
    if digest != EXPECTED_SHA256:
        print(f"FAIL unexpected exe sha256 {digest}")
        return 1

    text, text_va = pe_text(raw)

    if b"Splat\x00\x00\x00ForceF\x00\x00Die" not in raw:
        print("FAIL ForceF cue string missing between Splat and Die")
        return 1
    print(f"OK cue string ForceF at lead VA {CUE_STR:#x}")

    str_refs = find_imm32(text, text_va, CUE_STR)
    # push imm at 0x45E600; imm32 payload at +1
    if set(str_refs) not in ({0x0045E601}, {0x0045E600}):
        print(f"FAIL ForceF string imm refs {list(map(hex, str_refs))}")
        return 1
    print(f"OK string imm refs {[hex(x) for x in str_refs]}")

    slot_refs = find_imm32(text, text_va, SLOT_FORCEF)
    if set(slot_refs) != EXPECTED_SLOT_SITES:
        print(f"FAIL +0x1c8d0 sites {[hex(x) for x in slot_refs]} "
              f"expected {[hex(x) for x in sorted(EXPECTED_SLOT_SITES)]}")
        return 1
    print(f"OK +0x1c8d0 only register/preload/unload "
          f"{[hex(x) for x in sorted(slot_refs)]}")

    g_refs = find_imm32(text, text_va, GLOBAL_FORCEF)
    if g_refs:
        print(f"FAIL DAT_004E32D8 play/read sites {[hex(x) for x in g_refs]}")
        return 1
    # Re-verify facilitator claim: zero abs refs anywhere in the PE.
    pe_refs = find_imm32(raw, 0, GLOBAL_FORCEF)
    if pe_refs:
        print(f"FAIL DAT_004E32D8 imm32 somewhere in PE file-offs "
              f"{[hex(x) for x in pe_refs]}")
        return 1
    print(f"OK no absolute refs to ForceF global {GLOBAL_FORCEF:#x} "
          f"(.text and whole PE)")

    # Contrast: SpeedPad / Action / Lever play; Die sibling has abs refs.
    speed_refs = find_imm32(text, text_va, GLOBAL_SPEED)
    action_refs = find_imm32(text, text_va, GLOBAL_ACTION)
    lever_refs = find_imm32(text, text_va, GLOBAL_LEVER)
    die_refs = find_imm32(text, text_va, GLOBAL_DIE)
    if not speed_refs:
        print("FAIL SpeedPad global unexpectedly unreferenced (contrast broken)")
        return 1
    if not action_refs:
        print("FAIL Action global unexpectedly unreferenced (contrast broken)")
        return 1
    if not lever_refs:
        print("FAIL Lever global unexpectedly unreferenced (contrast broken)")
        return 1
    if not die_refs:
        print("FAIL Die global unexpectedly unreferenced (sibling contrast broken)")
        return 1
    print(f"OK contrast SpeedPad={len(speed_refs)} Action={len(action_refs)} "
          f"Lever={len(lever_refs)} Die={len(die_refs)}")

    plays = call_sites(text, text_va, PLAY_FUN)
    alts = call_sites(text, text_va, ALT_PLAY_FUN)
    forcef_play = play_loads_forcef(text, text_va, plays + alts)
    if forcef_play:
        print(f"FAIL play callers load ForceF {[hex(x) for x in forcef_play]}")
        return 1
    print(f"OK none of {len(plays)} FUN_0044535b + {len(alts)} "
          f"FUN_0044521b callers load ForceF")

    a3d = []
    wav = []
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
            else:
                other.append(name)
    if a3d:
        print(f"FAIL unexpected force/fan/blow/wind a3d {a3d}")
        return 1
    if not any(w.upper().endswith("FORCEF.WAV") for w in wav):
        print(f"FAIL expected SOUNDS\\FORCEF.WAV, got {wav}")
        return 1
    if not any(w.upper().endswith("FORCEFLD.WAV") for w in wav):
        print(f"FAIL expected SOUNDS\\FORCEFLD.WAV, got {wav}")
        return 1
    # FORCEFLD has no cue string in the exe.
    if b"ForceFld" in raw or b"FORCEFLD" in raw:
        print("FAIL unexpected ForceFld cue string in exe")
        return 1
    print(f"OK no force a3d; wav={wav}; other_tagged={len(other)}")

    print("AUDIT PASS: ForceF is cue-only unused (clean negative)")
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
