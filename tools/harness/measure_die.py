"""Round 28: what does Die do (vs Splat / FallSplat), and when is it played?

Stock exe. Cue Die is registered and played solely from type-0x23 enter
FUN_00404261 when fluid+0x128==1 — SFX only, no die.a3d / no Die gadget.

    python tools/harness/measure_die.py --audit

Predictions in docs/scratch/phase3-round28-die.md.
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

CUE_STR = 0x004C0CD0
CUE_OBJ = 0x004C6A08
SLOT_DIE = 0x1C8D4
GLOBAL_DIE = CUE_OBJ + SLOT_DIE  # 0x004E32DC

GLOBAL_SPLAT = CUE_OBJ + 0x1C8CC  # 0x004E32D4
SLOT_SPLAT = 0x1C8CC
GLOBAL_FALLSPLAT = CUE_OBJ + 0x1C8D8  # 0x004E32E0
GLOBAL_BALLOON = CUE_OBJ + 0x1C8F8  # 0x004E3300
GLOBAL_FORCEF = CUE_OBJ + 0x1C8D0  # 0x004E32D8

REG_WRITE = 0x0045E62C
PRELOAD = 0x0045EA4F
UNLOAD = 0x0045F0BE
EXPECTED_SLOT_SITES = {REG_WRITE, PRELOAD, UNLOAD}

PLAY_IMM = 0x004042CF  # imm32 of mov ecx,[DAT_004E32DC]
PLAY_INSN = 0x004042CD
PLAY_CHANNEL = 0x004042D7  # call FUN_00406a00
PLAY_SFX = 0x004042E5  # call FUN_0044535b
ENTER = 0x00404261
CHANNEL = 0x00406A00
SFX = 0x0044535B

VT_TYPE23 = 0x004B03C0
CTOR_TYPE23 = 0x004041FD
ATTACH_CALL = 0x0041C3BA
DROWN_STR = 0x004BF118

# FallSplat contrast (splat state type 0x1a)
FALLSPLAT_IMM = 0x0040B4B6
FALLSPLAT_ENTER = 0x0040B3EC
VT_SPLAT = 0x004B0668

ACTOR_LIST = 0x004CBB18
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
    if raw[off : off + 4] != b"Die\x00":
        print(f"FAIL Die cue string missing at {CUE_STR:#x}")
        return 1
    if b"ForceF\x00\x00Die\x00FallSplat" not in raw[off - 16 : off + 16]:
        print("FAIL Die not beside ForceF/FallSplat SFX cues")
        return 1
    print(f"OK cue string Die at {CUE_STR:#x} (SFX neighborhood)")

    str_refs = find_imm32(text, text_va, CUE_STR)
    if set(str_refs) not in ({0x0045E619}, {0x0045E618}):
        print(f"FAIL Die string imm refs {list(map(hex, str_refs))}")
        return 1
    print(f"OK string imm refs {[hex(x) for x in str_refs]}")

    slot_refs = set(find_imm32(text, text_va, SLOT_DIE))
    if slot_refs != EXPECTED_SLOT_SITES:
        print(f"FAIL +0x1c8d4 sites {[hex(x) for x in sorted(slot_refs)]} "
              f"expected {[hex(x) for x in sorted(EXPECTED_SLOT_SITES)]}")
        return 1
    print(f"OK +0x1c8d4 only register/preload/unload "
          f"{[hex(x) for x in sorted(slot_refs)]}")

    # Register store after Die push: mov [ecx+0x1c8d4], eax
    store_off = 0x0045E62A - text_va
    if text[store_off : store_off + 6] != bytes.fromhex("8981d4c80100"):
        print(f"FAIL Die register store bytes "
              f"{text[store_off:store_off + 6].hex()}")
        return 1
    print(f"OK register store +0x1c8d4 at {REG_WRITE:#x}")

    g_refs = set(find_imm32(text, text_va, GLOBAL_DIE))
    if g_refs != {PLAY_IMM}:
        print(f"FAIL DAT_004E32DC sites {[hex(x) for x in sorted(g_refs)]} "
              f"expected [{PLAY_IMM:#x}]")
        return 1
    print(f"OK sole absolute Die global ref at play load {PLAY_INSN:#x} "
          f"(imm {PLAY_IMM:#x})")

    play_off = PLAY_INSN - text_va
    if text[play_off : play_off + 6] != bytes.fromhex("8b0ddc324e00"):
        print(f"FAIL play load bytes {text[play_off:play_off + 6].hex()}")
        return 1
    # Gate: cmp [local_fluid_subtype], 1 just before the play arm
    gate_off = 0x004042C0 - text_va
    if text[gate_off : gate_off + 4] != bytes.fromhex("837d8c01"):
        print(f"FAIL subtype==1 gate bytes "
              f"{text[gate_off:gate_off + 4].hex()}")
        return 1
    if PLAY_CHANNEL not in call_sites(text, text_va, CHANNEL):
        print(f"FAIL missing FUN_00406a00 at {PLAY_CHANNEL:#x}")
        return 1
    if PLAY_SFX not in call_sites(text, text_va, SFX):
        print(f"FAIL missing FUN_0044535b at {PLAY_SFX:#x}")
        return 1
    print(f"OK play arm inside enter {ENTER:#x}: subtype==1 -> "
          f"FUN_00406a00 -> FUN_0044535b")

    # Vtable enter slot
    vt_off = VT_TYPE23 - 0x400000
    vt = struct.unpack_from("<III", raw, vt_off)
    if vt[2] != ENTER:
        print(f"FAIL vtable {VT_TYPE23:#x}+8 = {vt[2]:#x}, expected {ENTER:#x}")
        return 1
    print(f"OK vtable {VT_TYPE23:#x}+8 enter -> {ENTER:#x}")

    drown_refs = find_imm32(text, text_va, DROWN_STR)
    if 0x00404280 not in drown_refs:
        print(f"FAIL drown string not pushed from enter; refs="
              f"{list(map(hex, drown_refs))}")
        return 1
    ctor_calls = call_sites(text, text_va, CTOR_TYPE23)
    if ctor_calls != [ATTACH_CALL]:
        print(f"FAIL ctor call sites {list(map(hex, ctor_calls))} "
              f"expected [{ATTACH_CALL:#x}]")
        return 1
    actor_in_enter = [
        va for va in find_imm32(text, text_va, ACTOR_LIST)
        if ENTER <= va <= 0x00404371
    ]
    if actor_in_enter:
        print(f"FAIL actor list imm inside enter "
              f"{list(map(hex, actor_in_enter))}")
        return 1
    print(f"OK type-0x23 enter sets drown; sole attach {ATTACH_CALL:#x}; "
          f"no actor-list {ACTOR_LIST:#x} imm in enter")

    # FallSplat contrast: sole abs play in splat enter
    fs_refs = set(find_imm32(text, text_va, GLOBAL_FALLSPLAT))
    if fs_refs != {FALLSPLAT_IMM}:
        print(f"FAIL FallSplat global sites {[hex(x) for x in sorted(fs_refs)]} "
              f"expected [{FALLSPLAT_IMM:#x}]")
        return 1
    splat_vt = struct.unpack_from("<III", raw, VT_SPLAT - 0x400000)
    if splat_vt[2] != FALLSPLAT_ENTER:
        print(f"FAIL splat vtable +8 = {splat_vt[2]:#x}, "
              f"expected {FALLSPLAT_ENTER:#x}")
        return 1
    type_imm = 0x0040B3BE  # imm32 of mov [ecx+0x14], 0x1a in ctor
    if struct.unpack_from("<I", text, type_imm - text_va)[0] != 0x1A:
        print(f"FAIL splat ctor type imm at {type_imm:#x} not 0x1a")
        return 1
    print(f"OK FallSplat sole play imm {FALLSPLAT_IMM:#x} in splat enter "
          f"{FALLSPLAT_ENTER:#x} (vtable {VT_SPLAT:#x}, type 0x1a)")

    # Splat contrast: unused absolute global (ForceF-shaped)
    splat_abs = find_imm32(text, text_va, GLOBAL_SPLAT)
    if splat_abs:
        print(f"FAIL Splat global unexpectedly referenced "
              f"{list(map(hex, splat_abs))}")
        return 1
    splat_slot = set(find_imm32(text, text_va, SLOT_SPLAT))
    if splat_slot != {0x0045E5FC, 0x0045EA23, 0x0045F096}:
        print(f"FAIL Splat +0x1c8cc sites "
              f"{[hex(x) for x in sorted(splat_slot)]}")
        return 1
    print("OK Splat cue has 0 abs play refs "
          "(register/preload/unload only)")

    if find_imm32(text, text_va, GLOBAL_BALLOON):
        print("FAIL Balloon global unexpectedly referenced")
        return 1
    if find_imm32(text, text_va, GLOBAL_FORCEF):
        print("FAIL ForceF global unexpectedly referenced")
        return 1
    print("OK contrast: Balloon/ForceF still unused")

    die_a3d = []
    die_wav = []
    lvl_a3d = 0
    for box_name in ("lemmings.box", "patch.box"):
        path = game_dir / box_name
        if not path.exists():
            print(f"FAIL missing {path}")
            return 1
        box = Box(str(path))
        for name in box.names:
            low = name.lower().replace("/", "\\")
            base = low.rsplit("\\", 1)[-1]
            if base == "die.wav":
                die_wav.append(name)
            elif base.startswith("die") and base.endswith(".a3d"):
                die_a3d.append(name)
        for name in box.names:
            low = name.lower().replace("/", "\\")
            if "compiled" not in low or not low.endswith(".lvl"):
                continue
            data = box.read(name)
            if b"die.a3d" in data.lower():
                lvl_a3d += 1
    if die_a3d or lvl_a3d:
        print(f"FAIL unexpected die a3d box={die_a3d} lvl={lvl_a3d}")
        return 1
    if not die_wav:
        print("FAIL expected SOUNDS\\DIE.WAV")
        return 1
    print(f"OK no die.a3d; WAV={die_wav}")

    print("AUDIT PASS: Die is SFX on type-0x23 enter (fluid subtype 1); "
          "no Die gadget")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--audit", action="store_true",
                    help="offline Die play-path audit")
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
