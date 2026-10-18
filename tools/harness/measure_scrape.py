"""Round 25: what does Scrape do, and when is the cue played?

Stock exe. Cue Scrape is registered and played from dig-bite and bash
cell-helper steel/blocker refusal arms — SFX only, no scrape.a3d /
no Scrape gadget. Mine refuse uses MineMetal, not Scrape.

    python tools/harness/measure_scrape.py --audit

Predictions in docs/scratch/phase3-round25-scrape.md.
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

CUE_STR = 0x004C0CA4
CUE_OBJ = 0x004C6A08
SLOT_SCRAPE = 0x1C8C0
GLOBAL_SCRAPE = CUE_OBJ + SLOT_SCRAPE  # 0x004E32C8
GLOBAL_MINE = CUE_OBJ + 0x1C8E0  # 0x004E32E8  (success dig/bash/mine)
GLOBAL_MINE_METAL = CUE_OBJ + 0x1C8E4  # 0x004E32EC (mine refuse)
GLOBAL_BALLOON = CUE_OBJ + 0x1C8F8  # 0x004E3300

# Scrape +0x1c8c0 sites inside FUN_0045e51f / FUN_0045ed81 only.
REG_WRITE = 0x0045E5B4
PRELOAD = 0x0045E9E1
UNLOAD = 0x0045F05A
EXPECTED_SLOT_SITES = {REG_WRITE, PRELOAD, UNLOAD}

# Play loads; imm32 at +1 of mov eax,[DAT_004E32C8].
PLAY_BASH = 0x0040C139  # FUN_0040c0af refuse → FUN_00445274
PLAY_DIG = 0x00415B33  # FUN_00415a43 refuse → FUN_00406a00 → FUN_0044535b
PLAY_LOOKUP_CHANNEL = 0x00406A00
PLAY_SFX = 0x0044535B
PLAY_POS = 0x00445274

DIG_BITE = 0x00415A43
BASH_HELPER = 0x0040C0AF
BASH_BITE = 0x0040C357
MINE_BITE = 0x004188F6

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
    if raw[off : off + 7] != b"Scrape\x00":
        print(f"FAIL Scrape cue string missing at {CUE_STR:#x}")
        return 1
    if b"Bubble\x00\x00Scrape\x00\x00Ohno\x00" not in raw[off - 16 : off + 24]:
        print("FAIL Scrape not beside Bubble/Ohno SFX cues")
        return 1
    print(f"OK cue string Scrape at {CUE_STR:#x} (SFX neighborhood)")

    str_refs = find_imm32(text, text_va, CUE_STR)
    if set(str_refs) not in ({0x0045E5A1}, {0x0045E5A0}):
        print(f"FAIL Scrape string imm refs {list(map(hex, str_refs))}")
        return 1
    print(f"OK string imm refs {[hex(x) for x in str_refs]}")

    slot_refs = find_imm32(text, text_va, SLOT_SCRAPE)
    if set(slot_refs) != EXPECTED_SLOT_SITES:
        print(f"FAIL +0x1c8c0 sites {[hex(x) for x in slot_refs]} "
              f"expected {[hex(x) for x in sorted(EXPECTED_SLOT_SITES)]}")
        return 1
    print(f"OK +0x1c8c0 only register/preload/unload "
          f"{[hex(x) for x in sorted(slot_refs)]}")

    g_refs = find_imm32(text, text_va, GLOBAL_SCRAPE)
    if set(g_refs) != {PLAY_BASH + 1, PLAY_DIG + 1}:
        print(f"FAIL DAT_004E32C8 sites {[hex(x) for x in g_refs]} "
              f"expected [{PLAY_BASH + 1:#x}, {PLAY_DIG + 1:#x}]")
        return 1
    print(f"OK absolute Scrape global refs only at play loads "
          f"{PLAY_BASH:#x} / {PLAY_DIG:#x}")

    bash_off = PLAY_BASH - text_va
    if text[bash_off : bash_off + 5] != bytes.fromhex("a1c8324e00"):
        print(f"FAIL play-bash load bytes {text[bash_off:bash_off + 5].hex()}")
        return 1
    dig_off = PLAY_DIG - text_va
    if text[dig_off : dig_off + 5] != bytes.fromhex("a1c8324e00"):
        print(f"FAIL play-dig load bytes {text[dig_off:dig_off + 5].hex()}")
        return 1

    # Dig refuse: load Scrape; call channel; call SFX.
    if 0x00415B3C not in call_sites(text, text_va, PLAY_LOOKUP_CHANNEL):
        near = [hex(x) for x in call_sites(text, text_va, PLAY_LOOKUP_CHANNEL)
                if PLAY_DIG <= x <= PLAY_DIG + 0x40]
        print(f"FAIL missing FUN_00406a00 after dig Scrape; near={near}")
        return 1
    if 0x00415B4A not in call_sites(text, text_va, PLAY_SFX):
        near = [hex(x) for x in call_sites(text, text_va, PLAY_SFX)
                if PLAY_DIG <= x <= PLAY_DIG + 0x40]
        print(f"FAIL missing FUN_0044535b after dig Scrape; near={near}")
        return 1
    print("OK dig refuse play arm: FUN_00406a00 -> FUN_0044535b "
          f"inside dig-bite {DIG_BITE:#x}")

    # Bash refuse: load Scrape; call positional play helper.
    if 0x0040C148 not in call_sites(text, text_va, PLAY_POS):
        near = [hex(x) for x in call_sites(text, text_va, PLAY_POS)
                if PLAY_BASH <= x <= PLAY_BASH + 0x40]
        print(f"FAIL missing FUN_00445274 after bash Scrape; near={near}")
        return 1
    print(f"OK bash refuse play arm: FUN_00445274 inside helper "
          f"{BASH_HELPER:#x}")

    # Bash bite calls the helper (round 5).
    helper_calls = call_sites(text, text_va, BASH_HELPER)
    bash_bite_calls = [va for va in helper_calls
                       if BASH_BITE <= va <= 0x0040C6E7]
    if len(bash_bite_calls) < 1:
        print(f"FAIL FUN_0040c0af not called from bash bite; "
              f"all={list(map(hex, helper_calls))}")
        return 1
    print(f"OK bash bite {BASH_BITE:#x} calls helper "
          f"({len(bash_bite_calls)} sites)")

    # Success / mine contrast.
    mine_refs = find_imm32(text, text_va, GLOBAL_MINE)
    metal_refs = find_imm32(text, text_va, GLOBAL_MINE_METAL)
    balloon_refs = find_imm32(text, text_va, GLOBAL_BALLOON)
    if not any(DIG_BITE <= va <= 0x00415DD1 for va in mine_refs):
        print(f"FAIL dig success Mine cue missing; mine_refs="
              f"{list(map(hex, mine_refs))}")
        return 1
    if not any(BASH_HELPER <= va <= 0x0040C356 for va in mine_refs):
        print(f"FAIL bash success Mine cue missing; mine_refs="
              f"{list(map(hex, mine_refs))}")
        return 1
    mine_scrape = [va for va in find_imm32(text, text_va, GLOBAL_SCRAPE)
                   if MINE_BITE <= va <= 0x0041905E]
    if mine_scrape:
        print(f"FAIL mine bite loads Scrape {list(map(hex, mine_scrape))}")
        return 1
    if not any(MINE_BITE <= va <= 0x0041905E for va in metal_refs):
        print(f"FAIL mine refuse MineMetal missing; metal_refs="
              f"{list(map(hex, metal_refs))}")
        return 1
    if balloon_refs:
        print(f"FAIL Balloon global unexpectedly referenced "
              f"{list(map(hex, balloon_refs))}")
        return 1
    print("OK contrast: dig/bash success=Mine; mine refuse=MineMetal; "
          "Balloon still unused")

    actor_dig = [va for va in find_imm32(text, text_va, ACTOR_LIST)
                 if DIG_BITE <= va <= 0x00415DD1]
    actor_bash = [va for va in find_imm32(text, text_va, ACTOR_LIST)
                  if BASH_HELPER <= va <= 0x0040C356]
    if actor_dig or actor_bash:
        print(f"FAIL actor list imm in Scrape play fns "
              f"dig={list(map(hex, actor_dig))} "
              f"bash={list(map(hex, actor_bash))}")
        return 1
    print(f"OK no actor-list {ACTOR_LIST:#x} imm in dig-bite / bash helper")

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
            if "scrape" not in low:
                continue
            if low.endswith(".a3d"):
                a3d.append(name)
            elif low.endswith(".wav"):
                wav.append(name)
            else:
                other.append(name)
    if a3d:
        print(f"FAIL unexpected scrape a3d {a3d}")
        return 1
    if not any(w.upper().endswith("SCRAPE.WAV") for w in wav):
        print(f"FAIL expected SOUNDS\\SCRAPE.WAV, got {wav}")
        return 1
    if other:
        print(f"FAIL unexpected scrape non-wav assets {other}")
        return 1
    print(f"OK no scrape.a3d; WAV={wav}")

    print("AUDIT PASS: Scrape is dig/bash refuse SFX; no Scrape gadget")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--audit", action="store_true",
                    help="offline Scrape play-path audit")
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
