"""Offline, file-backed native anchors for the level-entry “Let's Go!” cue.

Checks selected stock-executable bytes only. This does not execute the EXE or
claim runtime/pixel fidelity; see docs/scratch/level-entry-start-cue.md.
"""
import argparse
import hashlib
from pathlib import Path
import struct


AUTHORITY = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
DEFAULT_EXE = Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    args = parser.parse_args()
    data = args.exe.read_bytes()
    assert hashlib.sha256(data).hexdigest() == AUTHORITY, "not the pinned stock executable"

    pe = struct.unpack_from("<I", data, 0x3C)[0]
    assert data[pe:pe + 4] == b"PE\0\0"
    coff = pe + 4
    section_count = struct.unpack_from("<H", data, coff + 2)[0]
    optional_size = struct.unpack_from("<H", data, coff + 16)[0]
    optional = coff + 20
    image_base = struct.unpack_from("<I", data, optional + 28)[0]
    section_table = optional + optional_size
    sections = []
    for index in range(section_count):
        at = section_table + 40 * index
        virtual_address, raw_size, raw_pointer = struct.unpack_from("<III", data, at + 12)
        sections.append((image_base + virtual_address, raw_size, raw_pointer))

    def read(va, size):
        for start, raw_size, raw_pointer in sections:
            delta = va - start
            if 0 <= delta and delta + size <= raw_size:
                offset = raw_pointer + delta
                result = data[offset:offset + size]
                assert len(result) == size, f"short raw read at {va:#x}"
                return result
        raise AssertionError(f"{va:#x}+{size:#x} is not wholly file-backed")

    # A compact set of instruction windows, not a copied function/trace.
    anchors = [
        (0x45CAEA, "6a47", "loader asks localized UK[0x47] cue"),
        (0x4151DA, "8b8070550000", "getter returns font pointer, not level name"),
        (0x45CAD6, "c6802460020001", "successful load activates cue"),
        (0x45CB21, "dd05f8174b00dc05e0174b00dc05e8174b00", "clock starts at 2+1+0.5=3.5"),
        (0x45CBA6, "dc25f8174b00", "no-intro loader subtracts two seconds"),
        (0x45CE8C, "83b92c57000000", "only UI zero advances cue clock"),
        (0x45CEA7, "dca068c90100", "subtracts frame seconds"),
        (0x45CED2, "7517", "strict expiry: exactly minus one remains active"),
        (0x45DD96, "83bd60ffffff02", "draw routing checks gameplay mode two"),
        (0x45E113, "dc9af85f0200", "post-update cue clock threshold"),
        (0x45E11E, "0f8596010000", "skip draw only when clock is above 1.5"),
        (0x45E124, "c745a8a0000000", "base rectangle left 160"),
        (0x45E12B, "c745ace0010000", "base rectangle right 480"),
        (0x45E132, "c745b4b4000000", "base rectangle top 180"),
        (0x45E139, "c745b004010000", "base rectangle bottom 260"),
        (0x45E18E, "dc4da0", "squared shrink fraction"),
        (0x45E194, "dd0518184b00", "horizontal growth scale 3040"),
        (0x45E1C0, "dd0510184b00", "vertical growth scale 760"),
        (0x45E1F7, "dd82f85f0200", "negative-clock alpha phase"),
        (0x45E24C, "6a03", "negative-clock additive blend mode"),
        (0x45E261, "6a01", "nonnegative source-over blend mode"),
        (0x45E2A4, "e8e3ca0100", "submit stretched text rectangle"),
        (0x45E2B5, "e896fefeff", "text submission helper"),
        (0x4403B5, "6868ff4b00", "normal material is ^Font"),
        (0x474471, "c745ec08000000", "caret material receives texture flag 8"),
        (0x47269D, "6a026a046a00", "flag-8 binder selects texture alpha"),
        (0x4726B7, "6a026a056a00", "alpha argument is TEXTURE"),
    ]
    for va, encoded, meaning in anchors:
        expected = bytes.fromhex(encoded)
        assert read(va, len(expected)) == expected, f"{va:#x}: {meaning}"
        print(f"PASS {va:08x}: {meaning}")

    # Clocks, shrink denominator, and rectangle dimensions used by the anchors.
    doubles = [
        (0x4B17E0, 1.0), (0x4B17E8, 0.5), (0x4B17F0, 1.0),
        (0x4B17F8, 2.0), (0x4B1810, 760.0), (0x4B1818, 3040.0),
        (0x4B0338, 0.0), (0x4B0400, 255.0),
    ]
    for va, expected in doubles:
        actual = struct.unpack("<d", read(va, 8))[0]
        assert actual == expected, f"{va:#x}: expected double {expected}, got {actual}"
        print(f"PASS {va:08x}: double {expected}")
    print(f"PASS: {len(anchors)} raw-backed anchors and {len(doubles)} doubles; offline byte check only")


if __name__ == "__main__":
    main()
