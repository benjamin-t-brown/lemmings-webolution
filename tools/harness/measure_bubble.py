"""Round 24: what does Bubble do, and how is it detected?

Stock exe. Cue Bubble is registered and played from type-0x23 (fluid
mismatch / zone) state step — SFX only, no bubble.a3d / no Bubble gadget.

    python tools/harness/measure_bubble.py --audit

Predictions in docs/scratch/phase3-round24-bubble.md.
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

CUE_STR = 0x004C0C9C
CUE_OBJ = 0x004C6A08
SLOT_BUBBLE = 0x1C8BC
GLOBAL_BUBBLE = CUE_OBJ + SLOT_BUBBLE  # 0x004E32C4
GLOBAL_SPEED = CUE_OBJ + 0x1C8F0  # 0x004E32F8
GLOBAL_ACTION = CUE_OBJ + 0x1C8F4  # 0x004E32FC
GLOBAL_BALLOON = CUE_OBJ + 0x1C8F8  # 0x004E3300
GLOBAL_DIE = CUE_OBJ + 0x1C8D4  # 0x004E32DC

# Bubble +0x1c8bc sites inside FUN_0045e51f / FUN_0045ed81 only.
REG_WRITE = 0x0045E59C
PRELOAD = 0x0045E9CB
UNLOAD = 0x0045F046
EXPECTED_SLOT_SITES = {REG_WRITE, PRELOAD, UNLOAD}

# Play loads inside FUN_00404372 (type-0x23 step); imm32 at +2 of mov.
PLAY_LOAD_A = 0x00404697  # mov ecx, [DAT_004E32C4]
PLAY_LOAD_B = 0x004046C4  # mov edx, [DAT_004E32C4]
PLAY_LOOKUP = 0x00404CC0
PLAY_CHANNEL = 0x00406A00
PLAY_SFX = 0x0044535B

VT_TYPE23 = 0x004B03C0
CTOR_TYPE23 = 0x004041FD
STEP_TYPE23 = 0x00404372
ENTER_TYPE23 = 0x00404261
ATTACH_CALL = 0x0041C3BA  # sole call to ctor inside FUN_0041bffc
TYPE_IMM = 0x0040421B  # mov [ecx+0x14], 0x23  (imm32 at this VA)

ACTOR_LIST = 0x004CBB18
DROWN_STR = 0x004BF118
BUBBLES_FX = 0x004BF128  # sheet name "Bubbles" (not the SFX cue)
BUBBLES_LOW = 0x004BF120  # "bubbles"

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
    if raw[off : off + 7] != b"Bubble\x00":
        print(f"FAIL Bubble cue string missing at {CUE_STR:#x}")
        return 1
    if b"Explode\x00Bubble\x00\x00Scrape\x00" not in raw[off - 16 : off + 16]:
        print("FAIL Bubble not beside Explode/Scrape SFX cues")
        return 1
    print(f"OK cue string Bubble at {CUE_STR:#x} (SFX neighborhood)")

    str_refs = find_imm32(text, text_va, CUE_STR)
    if set(str_refs) not in ({0x0045E589}, {0x0045E588}):
        print(f"FAIL Bubble string imm refs {list(map(hex, str_refs))}")
        return 1
    print(f"OK string imm refs {[hex(x) for x in str_refs]}")

    slot_refs = find_imm32(text, text_va, SLOT_BUBBLE)
    if set(slot_refs) != EXPECTED_SLOT_SITES:
        print(f"FAIL +0x1c8bc sites {[hex(x) for x in slot_refs]} "
              f"expected {[hex(x) for x in sorted(EXPECTED_SLOT_SITES)]}")
        return 1
    print(f"OK +0x1c8bc only register/preload/unload "
          f"{[hex(x) for x in sorted(slot_refs)]}")

    g_refs = find_imm32(text, text_va, GLOBAL_BUBBLE)
    if set(g_refs) != {PLAY_LOAD_A + 2, PLAY_LOAD_B + 2}:
        print(f"FAIL DAT_004E32C4 sites {[hex(x) for x in g_refs]} "
              f"expected [{PLAY_LOAD_A + 2:#x}, {PLAY_LOAD_B + 2:#x}]")
        return 1
    print(f"OK absolute Bubble global refs only at play loads "
          f"{PLAY_LOAD_A:#x} / {PLAY_LOAD_B:#x}")

    # Decode play arm: lookup → channel → FUN_0044535b; one-shot this+0x1a.
    a_off = PLAY_LOAD_A - text_va
    if text[a_off : a_off + 6] != bytes.fromhex("8b0dc4324e00"):
        print(f"FAIL play-A load bytes {text[a_off:a_off + 6].hex()}")
        return 1
    b_off = PLAY_LOAD_B - text_va
    if text[b_off : b_off + 6] != bytes.fromhex("8b15c4324e00"):
        print(f"FAIL play-B load bytes {text[b_off:b_off + 6].hex()}")
        return 1

    lookups = call_sites(text, text_va, PLAY_LOOKUP)
    channels = call_sites(text, text_va, PLAY_CHANNEL)
    sfxes = call_sites(text, text_va, PLAY_SFX)
    if 0x004046A3 not in lookups:
        print(f"FAIL missing FUN_00404cc0 at 0x4046a3; "
              f"near={[hex(x) for x in lookups if 0x404680 <= x <= 0x4046e0]}")
        return 1
    if 0x004046CE not in channels:
        print(f"FAIL missing FUN_00406a00 at 0x4046ce; "
              f"near={[hex(x) for x in channels if 0x404680 <= x <= 0x4046e0]}")
        return 1
    if 0x004046DC not in sfxes:
        print(f"FAIL missing FUN_0044535b at 0x4046dc; "
              f"near={[hex(x) for x in sfxes if 0x404680 <= x <= 0x4046e0]}")
        return 1
    print("OK play arm: FUN_00404cc0 -> FUN_00406a00 -> FUN_0044535b")

    # Type-0x23 identity.
    if struct.unpack_from("<I", text, TYPE_IMM - text_va)[0] != 0x23:
        print(f"FAIL type imm at {TYPE_IMM:#x} not 0x23")
        return 1
    vt_refs = find_imm32(text, text_va, VT_TYPE23)
    if set(vt_refs) != {0x00404211, 0x00404251}:
        print(f"FAIL vtable 0x4b03c0 writers {list(map(hex, vt_refs))}")
        return 1
    ctor_calls = call_sites(text, text_va, CTOR_TYPE23)
    if ctor_calls != [ATTACH_CALL]:
        print(f"FAIL ctor call sites {list(map(hex, ctor_calls))} "
              f"expected [{ATTACH_CALL:#x}]")
        return 1
    print(f"OK type 0x23 ctor {CTOR_TYPE23:#x}; sole attach call "
          f"{ATTACH_CALL:#x} (FUN_0041bffc); vtable {VT_TYPE23:#x}")

    # Enter pushes "drown"; step is FUN_00404372; no actor-list imm in step.
    drown_refs = find_imm32(text, text_va, DROWN_STR)
    if 0x00404280 not in drown_refs:
        print(f"FAIL drown string not pushed from enter; refs="
              f"{list(map(hex, drown_refs))}")
        return 1
    step_imm = find_imm32(text, text_va, STEP_TYPE23)
    # Step only appears as vtable slot in .rdata, not as .text imm.
    if step_imm:
        print(f"FAIL unexpected .text imm to step {list(map(hex, step_imm))}")
        return 1
    actor_in_step = [
        va for va in find_imm32(text, text_va, ACTOR_LIST)
        if STEP_TYPE23 <= va <= 0x00404956
    ]
    if actor_in_step:
        print(f"FAIL actor list imm inside type-0x23 step "
              f"{list(map(hex, actor_in_step))}")
        return 1
    print(f"OK enter {ENTER_TYPE23:#x} sets anim drown; step {STEP_TYPE23:#x} "
          f"has no actor-list {ACTOR_LIST:#x} imm")

    # Contrast: Balloon unused; SpeedPad/Action/Die referenced.
    speed_refs = find_imm32(text, text_va, GLOBAL_SPEED)
    action_refs = find_imm32(text, text_va, GLOBAL_ACTION)
    balloon_refs = find_imm32(text, text_va, GLOBAL_BALLOON)
    die_refs = find_imm32(text, text_va, GLOBAL_DIE)
    if not speed_refs:
        print("FAIL SpeedPad global unexpectedly unreferenced (contrast broken)")
        return 1
    if not action_refs:
        print("FAIL Action global unexpectedly unreferenced (contrast broken)")
        return 1
    if balloon_refs:
        print(f"FAIL Balloon global unexpectedly referenced "
              f"{list(map(hex, balloon_refs))}")
        return 1
    if not die_refs:
        print("FAIL Die global unexpectedly unreferenced (enter sibling)")
        return 1
    # Die is loaded from enter when fluid+0x128==1 (imm at 0x4042cf).
    enter_die = [va for va in die_refs if ENTER_TYPE23 <= va <= 0x00404370]
    if 0x004042CF not in enter_die:
        print(f"FAIL Die not loaded in type-0x23 enter; "
              f"enter_die={list(map(hex, enter_die))}")
        return 1
    print("OK Die cue load in enter at 0x4042cd "
          "(fluid subtype 1 -> FUN_0044535b)")
    # FX name "Bubbles" / "bubbles" are particle sheet names, not this cue.
    fx_refs = find_imm32(text, text_va, BUBBLES_FX)
    low_refs = find_imm32(text, text_va, BUBBLES_LOW)
    if not fx_refs or not low_refs:
        print(f"FAIL Bubbles FX strings unreferenced fx={list(map(hex, fx_refs))} "
              f"low={list(map(hex, low_refs))}")
        return 1
    print(f"OK FX names Bubbles/bubbles at {BUBBLES_FX:#x}/{BUBBLES_LOW:#x} "
          f"(particle path; distinct from SFX cue)")

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
            if "bubble" not in low:
                continue
            if low.endswith(".a3d"):
                a3d.append(name)
            elif low.endswith(".wav"):
                wav.append(name)
            else:
                other.append(name)
    if a3d:
        print(f"FAIL unexpected bubble a3d {a3d}")
        return 1
    if not any(w.upper().endswith("BUBBLE.WAV") for w in wav):
        print(f"FAIL expected SOUNDS\\BUBBLE.WAV, got {wav}")
        return 1
    if not any("BUBBLES.SPR" in o.upper() for o in other):
        print(f"FAIL expected SPRITES\\BUBBLES.SPR, got {other}")
        return 1
    print(f"OK no bubble.a3d; WAV={wav}; other={other}")

    print("AUDIT PASS: Bubble is SFX on type-0x23 step; no Bubble gadget")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--audit", action="store_true",
                    help="offline Bubble play-path audit")
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
