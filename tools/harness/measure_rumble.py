"""Round 26: what does Rumble do, and when is the cue played?

Stock exe. Cue Rumble is registered and played from object step
FUN_00423749 (vtable 0x004B0F00) on deferred state commit when the
previous committed state was nonzero — SFX only, no rumble.a3d /
no Rumble gadget.

    python tools/harness/measure_rumble.py --audit

Predictions in docs/scratch/phase3-round26-rumble.md.
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

CUE_STR = 0x004C0CFC
CUE_OBJ = 0x004C6A08
SLOT_RUMBLE = 0x1C8E8
GLOBAL_RUMBLE = CUE_OBJ + SLOT_RUMBLE  # 0x004E32F0
GLOBAL_BALLOON = CUE_OBJ + 0x1C8F8  # 0x004E3300

REG_WRITE = 0x0045E6A4
PRELOAD = 0x0045EAD3
UNLOAD = 0x0045F122
EXPECTED_SLOT_SITES = {REG_WRITE, PRELOAD, UNLOAD}
# Imm32 search also matches call+0x1c8 (e8 c8 01 00); not displacements.
SLOT_CALL_FALSE = {0x0046DFC3, 0x00489453}

PLAY_LOAD = 0x00423823  # imm32 of mov eax,[DAT_004E32F0]; opcode at -1
PLAY_INSN = 0x00423822
STEP_FN = 0x00423749
COUNT_HELPER = 0x00423858
PLAY_LOOKUP_CHANNEL = 0x00406A00
PLAY_SFX = 0x0044535B
VT_OBJECT = 0x004B0F00
VT_INSTALL = 0x00423505

ACTOR_LIST = 0x004CBB18
LEM_LIST = 0x004C88D0

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


def real_slot_sites(text: bytes, text_va: int) -> set[int]:
    """Displacement sites only.

    Imm32 0x1c8e8 begins with byte 0xe8, so a naive 'starts with call
    opcode' test rejects the real mov [reg+0x1c8e8] sites. Distinguish
    by the two bytes before the imm: register/preload/unload use
    89 82 / 8b 91 / 8b 88; the false positives are call rel32 where the
    e8 is the opcode (preceded by add ecx,0x10 / mov ecx,[ebp-14]).
    """
    out = set()
    for va in find_imm32(text, text_va, SLOT_RUMBLE):
        off = va - text_va
        prev2 = text[off - 2 : off]
        if prev2 in (b"\x89\x82", b"\x8b\x91", b"\x8b\x88"):
            out.add(va)
    return out


def audit(exe_path: Path, game_dir: Path) -> int:
    raw = exe_path.read_bytes()
    digest = hashlib.sha256(raw).hexdigest()
    if digest != EXPECTED_SHA256:
        print(f"FAIL unexpected exe sha256 {digest}")
        return 1

    text, text_va = pe_text(raw)

    off = CUE_STR - 0x400000
    if raw[off : off + 7] != b"Rumble\x00":
        print(f"FAIL Rumble cue string missing at {CUE_STR:#x}")
        return 1
    if b"MineMetal\x00\x00\x00Rumble\x00\x00AntiGrav" not in raw[off - 16 : off + 24]:
        print("FAIL Rumble not beside MineMetal/AntiGrav SFX cues")
        return 1
    print(f"OK cue string Rumble at {CUE_STR:#x} (SFX neighborhood)")

    str_refs = find_imm32(text, text_va, CUE_STR)
    if set(str_refs) not in ({0x0045E691}, {0x0045E690}):
        print(f"FAIL Rumble string imm refs {list(map(hex, str_refs))}")
        return 1
    print(f"OK string imm refs {[hex(x) for x in str_refs]}")

    raw_slot = set(find_imm32(text, text_va, SLOT_RUMBLE))
    if not SLOT_CALL_FALSE.issubset(raw_slot):
        print(f"FAIL expected call false-positives missing; all="
              f"{[hex(x) for x in sorted(raw_slot)]}")
        return 1
    slot_refs = real_slot_sites(text, text_va)
    if slot_refs != EXPECTED_SLOT_SITES:
        print(f"FAIL +0x1c8e8 real sites {[hex(x) for x in sorted(slot_refs)]} "
              f"expected {[hex(x) for x in sorted(EXPECTED_SLOT_SITES)]}")
        return 1
    print(f"OK +0x1c8e8 only register/preload/unload "
          f"{[hex(x) for x in sorted(slot_refs)]}; "
          f"call-FP {[hex(x) for x in sorted(SLOT_CALL_FALSE)]}")

    g_refs = find_imm32(text, text_va, GLOBAL_RUMBLE)
    if set(g_refs) != {PLAY_LOAD}:
        print(f"FAIL DAT_004E32F0 sites {[hex(x) for x in g_refs]} "
              f"expected [{PLAY_LOAD:#x}]")
        return 1
    print(f"OK absolute Rumble global ref only at play load {PLAY_INSN:#x}")

    play_off = PLAY_INSN - text_va
    if text[play_off : play_off + 5] != bytes.fromhex("a1f0324e00"):
        print(f"FAIL play-load bytes {text[play_off:play_off + 5].hex()}")
        return 1

    # Play arm: load Rumble; call channel; call SFX.
    if 0x0042382B not in call_sites(text, text_va, PLAY_LOOKUP_CHANNEL):
        near = [hex(x) for x in call_sites(text, text_va, PLAY_LOOKUP_CHANNEL)
                if PLAY_INSN <= x <= PLAY_INSN + 0x40]
        print(f"FAIL missing FUN_00406a00 after Rumble load; near={near}")
        return 1
    if 0x00423839 not in call_sites(text, text_va, PLAY_SFX):
        near = [hex(x) for x in call_sites(text, text_va, PLAY_SFX)
                if PLAY_INSN <= x <= PLAY_INSN + 0x40]
        print(f"FAIL missing FUN_0044535b after Rumble load; near={near}")
        return 1
    print(f"OK play arm: FUN_00406a00 -> FUN_0044535b inside step "
          f"{STEP_FN:#x}")

    # Step is vtable slot; count helper walks lemming list.
    if COUNT_HELPER not in [
            text_va + i + 5 + struct.unpack_from("<i", text, i + 1)[0]
            for i in range(STEP_FN - text_va, 0x00423857 - text_va)
            if text[i] == 0xE8]:
        # softer: any call from step to count helper
        if not any(STEP_FN <= va <= 0x00423857
                   for va in call_sites(text, text_va, COUNT_HELPER)):
            print(f"FAIL step does not call count helper {COUNT_HELPER:#x}")
            return 1
    print(f"OK step calls count helper {COUNT_HELPER:#x}")

    list_refs = [va for va in find_imm32(text, text_va, LEM_LIST)
                 if COUNT_HELPER <= va <= 0x00423E68]
    if len(list_refs) < 1:
        print(f"FAIL count helper missing list {LEM_LIST:#x}")
        return 1
    print(f"OK count helper walks list {LEM_LIST:#x} "
          f"({len(list_refs)} imm hits)")

    vt_refs = find_imm32(text, text_va, VT_OBJECT)
    if VT_INSTALL not in vt_refs:
        print(f"FAIL vtable install missing; refs="
              f"{[hex(x) for x in vt_refs]}")
        return 1
    print(f"OK vtable {VT_OBJECT:#x} installed (incl. {VT_INSTALL:#x})")

    actor = [va for va in find_imm32(text, text_va, ACTOR_LIST)
             if STEP_FN <= va <= 0x00423857]
    if actor:
        print(f"FAIL actor list imm in Rumble play step "
              f"{list(map(hex, actor))}")
        return 1
    print(f"OK no actor-list {ACTOR_LIST:#x} imm in step")

    balloon_refs = find_imm32(text, text_va, GLOBAL_BALLOON)
    if balloon_refs:
        print(f"FAIL Balloon global unexpectedly referenced "
              f"{list(map(hex, balloon_refs))}")
        return 1
    print("OK contrast: Balloon still unused")

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
            if "rumble" not in low:
                continue
            if low.endswith(".a3d"):
                a3d.append(name)
            elif low.endswith(".wav"):
                wav.append(name)
            else:
                other.append(name)
            if "compiled" in low and low.endswith(".lvl"):
                data = box.read(name)
                if b"rumble.a3d" in data.lower():
                    lvl_a3d += 1
        # also scan COMPILED levels for rumble.a3d even if path lacks rumble
        for name in box.names:
            low = name.lower().replace("/", "\\")
            if "compiled" not in low or not low.endswith(".lvl"):
                continue
            data = box.read(name)
            if b"rumble.a3d" in data.lower():
                lvl_a3d += 1
    if a3d or lvl_a3d:
        print(f"FAIL unexpected rumble a3d box={a3d} lvl={lvl_a3d}")
        return 1
    if not any(w.upper().endswith("RUMBLE.WAV") for w in wav):
        print(f"FAIL expected SOUNDS\\RUMBLE.WAV, got {wav}")
        return 1
    # TEXTURES paths must not be treated as gadgets; only flag non-wav
    # rumble* assets that are not textures.
    bad_other = [o for o in other if "texture" not in o.lower()]
    if bad_other:
        print(f"FAIL unexpected rumble non-wav assets {bad_other}")
        return 1
    print(f"OK no rumble.a3d; WAV={wav}")

    print("AUDIT PASS: Rumble is object state-commit SFX; no Rumble gadget")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--audit", action="store_true",
                    help="offline Rumble play-path audit")
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
