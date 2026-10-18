"""Name the two arrays beside the collision grid.

Static. Reads the stock exe and the `.LVL` files in `lemmings.box` (patch.box
wins on a shared name, same rule as AssetLibrary). Does not launch the game.

The loader is `FUN_00439FD3`. In one pass it bulk-reads four containers on
the world object, in this order:

    +0x42C  FUN_0043B640   length-prefixed u32 array   (the tube, N = 49858)
    +0x43C  FUN_0043B9C0   count, then R records of 12
    +0x41C  FUN_0043B640   the collision grid, 256 x height
    +0x664  FUN_0043B700   length-prefixed BYTE array  (the 12288 after the grid)

`FUN_0043B640` shifts the count by 2 and reads `N * 4` bytes. `FUN_0043B700`
pushes the count unshifted and reads `N` bytes. The 12288 in the file is that
byte count. It is not 12288 floats, and it is not the array loaded at +0x42C.

Usage:

    python tools/harness/read_grid_siblings.py
    python tools/harness/read_grid_siblings.py --level MM_CLIMB_002 MM_FLOAT
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import math
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from read_steppers import EXPECTED_SHA256, Image, _capstone, locate_exe  # noqa: E402

GAME = Path(r"C:\progs\Lemmings Revolution")
GRID_WIDTH = 256

# The world object the byte-map sampler and the tube lookup both take as this.
WORLD_GLOBAL = 0x004C6A10

# Movement and collision entry points. A reader is "in the sim" here only when
# its call site's containing function is one of these. Anything else is reported
# as a caller and left unnamed.
SIM_FUNCS = {
    0x0040DE60: "collision probe",
    0x004137CA: "climb stepper",
    0x004160BD: "fall stepper",
    0x0041A6D5: "fall (25.0 sibling)",
    0x0041B077: "bash/dig/mine stepper",
    0x0041BDD4: "walk stepper",
}

DETAIL_LEVELS = ("MM_CLIMB_002", "MM_FLOAT", "MM_DIG_000")


def game_dir() -> Path:
    import os

    env = os.environ.get("WEBOLUTION_GAME_DIR")
    return Path(env) if env else GAME


def levels_from_box(directory: Path) -> dict[str, bytes]:
    """COMPILED\\*.LVL. A later box overrides the same stem."""
    sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "proto"))
    from box import Box  # noqa: E402

    out: dict[str, bytes] = {}
    for box_name in ("lemmings.box", "patch.box"):
        path = directory / box_name
        if not path.exists():
            continue
        box = Box(str(path))
        n = 0
        for i, name in enumerate(box.names):
            upper = name.upper()
            if upper.startswith("COMPILED\\") and upper.endswith(".LVL"):
                out[Path(name).stem.upper()] = box.read(i)
                n += 1
        print(f"# {box_name}: {n} COMPILED *.LVL")
    return out


def u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def walk_cylinder(data: bytes):
    """The same walk CollisionGrid.cpp uses, continued one chunk past the grid.

    Returns the tube, the grid, the byte-count chunk, and the fixed 0x1800
    bytes the loader reads into +0x678 immediately after that chunk.
    """
    needle = struct.pack("<I", 8) + b"Cylinder"
    cyl = data.find(needle)
    if cyl < 0:
        raise ValueError("no Cylinder chunk")
    p = cyl + len(needle)
    if p + 5 * 4 + 4 + 8 > len(data):
        raise ValueError("truncated in the Cylinder preamble")
    zeros = [u32(data, p + i * 4) for i in range(5)]
    height = u32(data, p + 20)
    scale = struct.unpack_from("<d", data, p + 24)[0]
    p += 5 * 4 + 4 + 8

    tube_n = u32(data, p)
    p += 4
    tube_at = p
    if tube_n > len(data) // 4 or p + tube_n * 4 > len(data):
        raise ValueError("tube runs past the file")
    p += tube_n * 4

    rec_n = u32(data, p)
    p += 4
    if rec_n > len(data) // 12 or p + rec_n * 12 > len(data):
        raise ValueError("records run past the file")
    p += rec_n * 12

    grid_n = u32(data, p)
    p += 4
    grid_at = p
    if grid_n > len(data) // 4 or p + grid_n * 4 > len(data):
        raise ValueError("grid runs past the file")
    p += grid_n * 4

    byte_n = u32(data, p) if p + 4 <= len(data) else None
    byte_at = p
    payload = b""
    table = b""
    if byte_n is not None and byte_n <= len(data) and p + 4 + byte_n <= len(data):
        payload = data[p + 4 : p + 4 + byte_n]
        table = data[p + 4 + byte_n : p + 4 + byte_n + 0x1800]
    return {
        "cyl": cyl,
        "zeros": zeros,
        "height": height,
        "scale": scale,
        "tube_n": tube_n,
        "tube_at": tube_at,
        "rec_n": rec_n,
        "grid_n": grid_n,
        "grid_at": grid_at,
        "byte_n": byte_n,
        "byte_at": byte_at,
        "payload": payload,
        "table": table,
    }


def disasm(img: Image, start: int, length: int):
    md = _capstone()
    return list(md.disasm(img.read(start, length), start))


def calls_in(img: Image, targets: set[int]) -> dict[int, list[int]]:
    """Direct E8 rel32 call sites in .text. Indirect calls are not in this set."""
    _name, va, size, ptr = img.section(".text")
    blob = img.data[ptr : ptr + size]
    found = {t: [] for t in targets}
    for i in range(len(blob) - 5):
        if blob[i] != 0xE8:
            continue
        rel = struct.unpack_from("<i", blob, i + 1)[0]
        dest = va + i + 5 + rel
        if dest in found:
            found[dest].append(va + i)
    return found


def disp_hits(img: Image, wanted: set[int]):
    """Every .text instruction whose immediate or displacement is in `wanted`.

    Linear disassembly from the section start. A hit inside an embedded data
    island would be missed; the raw dword scan below is the cross-check.
    """
    import capstone

    md = _capstone()
    md.detail = True
    _name, va, size, ptr = img.section(".text")
    blob = img.data[ptr : ptr + size]
    hits = {t: [] for t in wanted}
    for ins in md.disasm(blob, va):
        for op in ins.operands:
            val = None
            if op.type == capstone.x86.X86_OP_IMM:
                val = op.imm & 0xFFFFFFFF
            elif op.type == capstone.x86.X86_OP_MEM:
                val = op.mem.disp & 0xFFFFFFFF
            if val in hits:
                hits[val].append(ins)
    return hits


def _dword_kind(img: Image, addr: int) -> str:
    """What instruction the dword at `addr` is the immediate of, if it is one."""
    if img.read(addr - 2, 2) == b"\x81\xc1":
        return "add ecx, imm"
    prev = img.read(addr - 1, 1)[0]
    if prev == 0xE8:
        return "call rel32"
    if prev == 0xE9:
        return "jmp rel32"
    two = img.read(addr - 2, 2)
    if two[0] == 0x0F and 0x80 <= two[1] <= 0x8F:
        return "jcc rel32"
    return f"unclassified {img.read(addr - 2, 8).hex(' ')}"


def raw_dwords(img: Image, section: str, value: int) -> list[int]:
    _name, va, size, ptr = img.section(section)
    blob = img.data[ptr : ptr + size]
    needle = struct.pack("<I", value)
    out = []
    start = 0
    while True:
        i = blob.find(needle, start)
        if i < 0:
            break
        out.append(va + i)
        start = i + 1
    return out


def func_start(img: Image, call: int, text_va: int) -> int | None:
    """Nearest preceding `push ebp; mov ebp, esp` whose disassembly reaches `call`."""
    start = max(call - 0x2000, text_va)
    raw = img.read(start, call - start)
    cands = []
    for off in range(0, max(0, len(raw) - 3)):
        if raw[off : off + 3] == b"\x55\x8b\xec":
            cands.append(start + off)
    md = _capstone()
    for cand in reversed(cands):
        for ins in md.disasm(img.read(cand, call - cand + 5), cand):
            if ins.address == call:
                return cand
            if ins.address > call:
                break
    return None


def report_exe(img: Image) -> None:
    print("# loader FUN_00439FD3, the adds and the call each one makes")
    pending = None
    for ins in disasm(img, 0x00439FD3, 0x200):
        if ins.address > 0x0043A1C3:
            break
        if ins.mnemonic == "add" and ins.op_str.startswith("ecx, 0x"):
            pending = (ins.address, ins.op_str)
            continue
        if pending and ins.mnemonic == "call":
            print(f"#   {pending[0]:08x}  add {pending[1]:<16}  call {ins.op_str} at {ins.address:08x}")
            pending = None

    print()
    print("# element size: FUN_0043B640 shifts the count, FUN_0043B700 does not")
    for start, stop in ((0x0043B67D, 0x0043B690), (0x0043B73D, 0x0043B750)):
        for ins in disasm(img, start, stop - start):
            print(f"#   {ins.address:08x}  {ins.bytes.hex(' '):<16}  {ins.mnemonic} {ins.op_str}")
        print("#   ---")

    print()
    print("# byte-map index, FUN_00403F40 / FUN_00403FC0 / FUN_00403FA0")
    for start, stop in ((0x00403F40, 0x00403F96), (0x00403FA0, 0x00403FB4), (0x00403FC0, 0x00403FDB)):
        for ins in disasm(img, start, stop - start):
            print(f"#   {ins.address:08x}  {ins.bytes.hex(' '):<20}  {ins.mnemonic} {ins.op_str}")
        print("#   ---")

    print("# tube index, FUN_0043C930: 2 * (arg1 + 257 * arg2) + arg3")
    for ins in disasm(img, 0x0043C930, 0x20):
        print(f"#   {ins.address:08x}  {ins.bytes.hex(' '):<20}  {ins.mnemonic} {ins.op_str}")
        if ins.mnemonic == "ret":
            break

    print()
    wanted = {0x41C, 0x42C, 0x43C, 0x664, 0x678}
    hits = disp_hits(img, wanted)
    print("# .text instructions whose immediate or displacement is one of these fields")
    for field in sorted(wanted):
        print(f"#   +{field:#x}  {len(hits[field])} instructions")
        for ins in hits[field]:
            print(f"#     {ins.address:08x}  {ins.mnemonic} {ins.op_str}")
    print("# raw little-endian dword, the cross-check on the operand scan")
    print("# a dword inside `call/jcc rel32` is a branch displacement, not a field")
    for field in (0x42C, 0x664):
        text = raw_dwords(img, ".text", field)
        rdata = raw_dwords(img, ".rdata", field)
        covered = {ins.address for ins in hits[field]}
        print(f"#   {field:#x}  .text {len(text)} dwords  .rdata {len(rdata)}")
        for addr in text:
            if any(ins <= addr < ins + 8 for ins in covered):
                continue
            kind = _dword_kind(img, addr)
            nxt = ""
            if kind == "add ecx, imm":
                insns = list(disasm(img, addr - 2, 12))
                if len(insns) >= 2:
                    nxt = f"  then {insns[1].mnemonic} {insns[1].op_str}"
            print(f"#     {addr:#010x}  {kind}{nxt}")

    targets = {
        0x00403F40: "byte-map reader FUN_00403F40",
        0x0043A31D: "tube lookup FUN_0043A31D",
        0x0043C960: "tube slot pointer FUN_0043C960",
        0x0043B640: "u32 bulk-read FUN_0043B640",
        0x0043B700: "byte bulk-read FUN_0043B700",
        0x00462264: "tube consumer FUN_00462264",
        0x00463100: "tube walker FUN_00463100",
        0x00463410: "tube walker FUN_00463410",
        0x00439FD3: "loader FUN_00439FD3",
    }
    found = calls_in(img, set(targets))
    _name, text_va, _size, _ptr = img.section(".text")
    print()
    print("# direct E8 calls. Indirect calls are outside this search.")
    for dest, label in targets.items():
        sites = found[dest]
        print(f"#   {label}  {len(sites)} calls")
        for site in sites:
            fn = func_start(img, site, text_va)
            pre = img.read(site - 5, 5)
            ecx_imm = None
            if pre[0] == 0xB9:
                ecx_imm = struct.unpack_from("<I", pre, 1)[0]
            sim = SIM_FUNCS.get(fn) if fn else None
            extra = ""
            if ecx_imm is not None:
                extra += f"  ecx={ecx_imm:#x}"
            if sim:
                extra += f"  SIM {sim}"
            print(f"#     {site:#010x}  in {fn and hex(fn)}{extra}")


def f32_stats(payload: bytes):
    n = len(payload) // 4
    if n == 0:
        return None
    vals = struct.unpack_from(f"<{n}f", payload)
    finite = [v for v in vals if math.isfinite(v)]
    # Zero is excluded. A real float layer has non-zero magnitudes; the small
    # integers in this payload become subnormals and fail the range.
    plausible = sum(1 for v in finite if 1e-3 <= abs(v) <= 1e6)
    return {
        "n": n,
        "nan": sum(1 for v in vals if math.isnan(v)),
        "max_abs": max((abs(v) for v in finite), default=None),
        "plausible": plausible,
    }


def report_level(name: str, info: dict) -> None:
    payload: bytes = info["payload"]
    hist = collections.Counter(payload)
    print(f"\n{name}")
    print(
        f"  Cylinder at {info['cyl']}, zeros {info['zeros']}, "
        f"height {info['height']}, f64 {info['scale']}"
    )
    print(f"  tube  N={info['tube_n']} at {info['tube_at']}  records R={info['rec_n']}")
    print(
        f"  grid  C={info['grid_n']} at {info['grid_at']}  "
        f"({GRID_WIDTH} x {info['grid_n'] // GRID_WIDTH if info['grid_n'] else 0})"
    )
    print(f"  after grid: u32 {info['byte_n']} at {info['byte_at']}")
    if not payload:
        print("  payload missing")
        return
    print(
        f"  payload {len(payload)} bytes, distinct {len(hist)}, "
        f"min {min(payload)} max {max(payload)}, nonzero {sum(1 for b in payload if b)}"
    )
    print(f"  top bytes {hist.most_common(8)}")
    stats = f32_stats(payload)
    print(
        f"  as f32: {stats['n']} values, nan {stats['nan']}, "
        f"max abs {stats['max_abs']}, plausible {stats['plausible']}"
    )
    # 128-wide rows: the index FUN_00403FC0 computes is y*128 + x/2.
    nz = next((i for i, b in enumerate(payload) if b), None)
    if nz is not None:
        print(
            f"  first nonzero byte {payload[nz]} at index {nz} "
            f"= row {nz // 128}, half-x {nz % 128} of the 128-wide index"
        )
    table: bytes = info["table"]
    if len(table) >= 24:
        a, b, c = struct.unpack_from("<ddd", table, 0)
        print(f"  next 0x1800 bytes, record 0 as 3 f64: {a} {b} {c}")
        used = sorted(hist)
        shown = 0
        for index in used:
            if index == 0 or shown >= 4:
                continue
            off = index * 24
            if off + 24 > len(table):
                break
            triple = struct.unpack_from("<ddd", table, off)
            print(f"    byte {index} -> record {triple}")
            shown += 1


def tube_words(data: bytes, info: dict):
    return struct.unpack_from(f"<{info['tube_n']}I", data, info["tube_at"])


def report_files(directory: Path, focus: tuple[str, ...]) -> int:
    lvls = levels_from_box(directory)
    if not lvls:
        print(f"no levels under {directory}", file=sys.stderr)
        return 1
    print(f"# levels {len(lvls)}")

    tube_ns = collections.Counter()
    byte_ns = collections.Counter()
    heights = collections.Counter()
    tube_payloads = {}
    byte_payloads = {}
    bad = []
    tube_sentinel = collections.Counter()
    byte_max = collections.Counter()
    for name, data in sorted(lvls.items()):
        try:
            info = walk_cylinder(data)
        except ValueError as exc:
            bad.append((name, str(exc)))
            continue
        tube_ns[info["tube_n"]] += 1
        byte_ns[info["byte_n"]] += 1
        heights[info["height"]] += 1
        words = tube_words(data, info)
        tube_payloads.setdefault(words, []).append(name)
        tube_sentinel[words.count(0xFFFFFFFF)] += 1
        byte_payloads.setdefault(info["payload"], []).append(name)
        if info["payload"]:
            byte_max[max(info["payload"])] += 1
        if name in focus:
            # stash for the detail pass; re-walk is cheap and keeps this loop flat
            pass

    print(f"# walk failed: {bad or 'none'}")
    print(f"# height {dict(heights)}")
    print(f"# tube count {dict(tube_ns)}  distinct payloads {len(tube_payloads)}")
    print(f"# u32 after the grid {dict(byte_ns)}  distinct payloads {len(byte_payloads)}")
    sentinels = list(tube_sentinel)
    print(
        f"# tube 0xffffffff per level: min {min(sentinels)} max {max(sentinels)} "
        f"across {sum(tube_sentinel.values())} levels"
    )
    print(f"# byte-map max value (value -> levels) {dict(byte_max)}")

    for name in focus:
        data = lvls.get(name)
        if data is None:
            print(f"\n{name}: not in the box")
            continue
        info = walk_cylinder(data)
        report_level(name, info)
        words = tube_words(data, info)
        hist = collections.Counter(words)
        others = [(v, k) for v, k in hist.items() if v != 0xFFFFFFFF]
        unique = sum(1 for _v, k in others if k == 1)
        print(
            f"  tube words: {hist[0xFFFFFFFF]} are 0xffffffff, "
            f"{len(words) - hist[0xFFFFFFFF]} are not, "
            f"{unique} of those values occur once"
        )
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--exe", help="stock Lemmings Revolution.exe")
    ap.add_argument("--game", help="install directory containing lemmings.box")
    ap.add_argument("--level", nargs="*", default=list(DETAIL_LEVELS))
    ap.add_argument("--no-digest-check", action="store_true")
    args = ap.parse_args()

    exe = locate_exe(args.exe)
    if not exe.exists():
        print(f"not found: {exe}", file=sys.stderr)
        return 2
    img = Image(exe)
    digest = hashlib.sha256(img.data).hexdigest()
    if not args.no_digest_check and digest != EXPECTED_SHA256:
        print(f"{exe}\n  sha256 {digest}\n  expected {EXPECTED_SHA256}", file=sys.stderr)
        return 2
    print(f"# {exe}")
    print(f"# sha256 {digest}")
    print(f"# image base {img.base:#010x}")
    report_exe(img)
    print()
    directory = Path(args.game) if args.game else game_dir()
    return report_files(directory, tuple(a.upper() for a in args.level))


if __name__ == "__main__":
    raise SystemExit(main())
