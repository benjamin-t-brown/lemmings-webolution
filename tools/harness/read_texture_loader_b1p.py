"""Offline B1p raw-PE/physical LEMBOX audit. Never opens a process.

Prerequisite: ordinary Python with capstone (not UCRT64 Python).
Native branch semantics are documented in the accompanying decision table;
this checker authenticates bytes/inventory, not runtime source selection.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
from pathlib import Path

from capstone import Cs, CS_ARCH_X86, CS_MODE_32

AUTHORITY = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
ROOT = Path(r"C:\progs\Lemmings Revolution")
OUT = Path("build/tmp/b1p-texture-loader")


def sha(data):
    return hashlib.sha256(data).hexdigest()


class PE:
    def __init__(self, data):
        self.data = data
        pe = struct.unpack_from("<I", data, 0x3c)[0]
        count = struct.unpack_from("<H", data, pe + 6)[0]
        opt_size = struct.unpack_from("<H", data, pe + 20)[0]
        self.base = struct.unpack_from("<I", data, pe + 24 + 28)[0]
        self.sections = []
        for i in range(count):
            p = pe + 24 + opt_size + i * 40
            vs, va, size, off = struct.unpack_from("<IIII", data, p + 8)
            self.sections.append(dict(name=data[p:p + 8].rstrip(b"\0").decode(),
                                      va=self.base + va, virtual_size=vs,
                                      raw_size=size, raw_offset=off))

    def offset(self, va, size):
        for s in self.sections:
            rel = va - s["va"]
            if 0 <= rel and rel + size <= s["raw_size"]:
                return s["raw_offset"] + rel
        raise ValueError(f"not file-backed: {va:#x}+{size:#x}")

    def read(self, va, size):
        p = self.offset(va, size)
        return self.data[p:p + size]


def inventory(path):
    data = path.read_bytes()
    assert data[:6] == b"LEMBOX"
    count, name_size = struct.unpack_from("<II", data, 6)
    pos = 14
    names = []
    for i in range(count):
        length = struct.unpack_from("<I", data, pos)[0]
        pos += 4
        assert pos + length <= 14 + name_size
        names.append((pos, data[pos:pos + length]))
        pos += length
    assert pos == 14 + name_size
    assert struct.unpack_from("<I", data, pos)[0] == count
    op = pos + 4
    offsets = struct.unpack_from(f"<{count}I", data, op)
    pos = op + count * 4
    assert struct.unpack_from("<I", data, pos)[0] == count
    sp = pos + 4
    sizes = struct.unpack_from(f"<{count}I", data, sp)
    end = sp + count * 4
    relevant = []
    # Exhaust all names, but report only exact loader-derived members.
    targets = {f"TEXTURES\\^FONT{suffix}.{ext}" for suffix in ("", "LO", "HI")
               for ext in ("BMP", "ALF")}
    for i, ((np, name), off, size) in enumerate(zip(names, offsets, sizes)):
        assert end <= off <= off + size <= len(data)
        text = name.rstrip(b"\0").decode("cp1252")
        if text.upper() in targets:
            relevant.append(dict(ordinal=i, name=text, name_offset=np,
                                 name_hex=name.hex(), offset_field=op + i * 4,
                                 size_field=sp + i * 4, payload_offset=off,
                                 payload_size=size, payload_sha256=sha(data[off:off + size]),
                                 payload_prefix=data[off:off + 24].hex()))
    return dict(path=str(path), sha256=sha(data), file_size=len(data), count=count,
                name_block_size=name_size, tables_end=end, relevant=relevant,
                requested=sorted(targets), exhausted=True)


def candidates(name, quality, eligible):
    """Independent simple statement of 472223..4722FF, not a native emulator."""
    suffix = "Lo" if not quality else "Hi" if eligible else ""
    return f"Textures\\{name}{suffix}.bmp", f"Textures\\{name}.bmp"


def controls():
    assert candidates("^Font", 0, False)[0] == "Textures\\^FontLo.bmp"
    assert candidates("^Font", 1, False)[0] == "Textures\\^Font.bmp"
    assert candidates("INN_X", 1, True)[0] == "Textures\\INN_XHi.bmp"
    assert candidates("^Font", 255, False)[0] == "Textures\\^Font.bmp"
    # Discriminating single-input control: quality flips the candidate spelling.
    assert candidates("^Font", 0, False) != candidates("^Font", 1, False)
    return {"passed": 5, "production_mutation": "none; read-only audit"}


def main():
    ap = argparse.ArgumentParser(__doc__)
    ap.add_argument("--root", type=Path, default=ROOT)
    ap.add_argument("--out", type=Path, default=OUT)
    args = ap.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    data = (args.root / "Lemmings Revolution.exe").read_bytes()
    assert sha(data) == AUTHORITY
    pe = PE(data)
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    # Bounded citation windows, not a claim that each is a full function.
    ranges = {0x47244b: 0x45, 0x445830: 0x1d, 0x472490: 0x10b,
              0x43b2ee: 0x30, 0x47e40d: 0x40f, 0x45015a: 0x32c,
              0x47f759: 0x259, 0x49ce20: 0x148, 0x4432c1: 0x599,
              0x47ef92: 0x54}
    report = {"authority": sha(data), "sections": pe.sections, "ranges": [],
              "controls": controls()}
    asm = []
    for va, size in ranges.items():
        raw = pe.read(va, size)
        fn = f"raw-{va:08x}.bin"
        (args.out / fn).write_bytes(raw)
        report["ranges"].append(dict(address=hex(va), size=size,
                                      file_offset=pe.offset(va, size),
                                      sha256=sha(raw), file=fn))
        asm.append(f"\n# {va:#x}+{size:#x}")
        asm.extend(f"{i.address:08x} {i.bytes.hex():24} {i.mnemonic} {i.op_str}"
                   for i in md.disasm(raw, va))
    # Reuse, do not reacquire/redecompile, B1o's two full loader ranges.
    old = Path("build/tmp/b1o-font-binding-fallback")
    for va in (0x47218a, 0x4743dd):
        raw = (old / f"raw-{va:08x}.bin").read_bytes()
        assert pe.read(va, len(raw)) == raw
        (args.out / f"reused-{va:08x}.bin").write_bytes(raw)
        report.setdefault("reused", []).append(dict(address=hex(va), size=len(raw),
              file_offset=pe.offset(va, len(raw)), sha256=sha(raw)))
    report["literals"] = []
    for va in (0x4c24e8, 0x4c24ec, 0x4c24f0, 0x4c2500, 0x4c2510,
               0x4c2548, 0x4c2550, 0x4c2bbc, 0x4c2bc0):
        p = pe.offset(va, 1)
        raw = data[p:data.index(b"\0", p) + 1]
        report["literals"].append(dict(address=hex(va), file_offset=p,
                                      raw=raw.hex(), value=raw[:-1].decode()))
    report["quality_literal_references"] = []
    needle = struct.pack("<I", 0x5cd5f2)
    s = next(s for s in pe.sections if s["name"] == ".text")
    raw = data[s["raw_offset"]:s["raw_offset"] + s["raw_size"]]
    for p in range(len(raw) - 3):
        if raw[p:p + 4] == needle:
            report["quality_literal_references"].append(hex(s["va"] + p))
    try:
        pe.read(0x5cd5f2, 1)
        raise AssertionError("BSS unexpectedly file backed")
    except ValueError:
        report["quality_file_value"] = None
    report["archives"] = [inventory(args.root / n) for n in ("lemmings.box", "patch.box")]
    report["loose_candidates"] = {n: (args.root / n).is_file() for n in
        [f"Textures/^Font{s}.{e}" for s in ("", "Lo", "Hi") for e in ("bmp", "alf")]}
    (args.out / "raw-audit.json").write_text(json.dumps(report, indent=2) + "\n", newline="\n")
    (args.out / "disassembly.txt").write_text("\n".join(asm) + "\n", newline="\n")
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
