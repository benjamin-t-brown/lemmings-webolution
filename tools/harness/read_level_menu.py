"""Offline raw-byte checks for the stock in-level Escape menu.

Checks selected file-backed executable anchors only. It does not execute the
EXE, load game assets, or claim runtime/pixel fidelity.
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

    def require_file(offset, size, what):
        assert offset >= 0 and size >= 0 and offset + size <= len(data), (
            f"{what} outside executable file")

    require_file(0, 0x40, "DOS header")
    assert data[:2] == b"MZ", "missing DOS signature"
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    require_file(pe, 24, "PE/COFF headers")
    assert data[pe:pe + 4] == b"PE\0\0", "missing PE signature"
    coff = pe + 4
    section_count = struct.unpack_from("<H", data, coff + 2)[0]
    optional_size = struct.unpack_from("<H", data, coff + 16)[0]
    optional = coff + 20
    require_file(optional, optional_size, "optional header")
    assert optional_size >= 32 and struct.unpack_from("<H", data, optional)[0] == 0x10B, (
        "expected PE32 optional header")
    image_base = struct.unpack_from("<I", data, optional + 28)[0]
    section_table = optional + optional_size
    require_file(section_table, 40 * section_count, "section table")
    sections = []
    for index in range(section_count):
        at = section_table + 40 * index
        virtual_address, raw_size, raw_pointer = struct.unpack_from("<III", data, at + 12)
        require_file(raw_pointer, raw_size, f"section {index} raw data")
        sections.append((image_base + virtual_address, raw_size, raw_pointer))

    def read(va, size):
        matches = []
        for start, raw_size, raw_pointer in sections:
            delta = va - start
            if delta >= 0 and delta + size <= raw_size:
                matches.append(raw_pointer + delta)
        assert len(matches) == 1, (
            f"{va:#x}+{size:#x} is not uniquely wholly file-backed (no BSS reads)")
        offset = matches[0]
        require_file(offset, size, f"raw read at {va:#x}")
        result = data[offset:offset + size]
        assert len(result) == size
        return result

    # Compact exact instruction windows covering menu entry, actions, routing,
    # strict content-hit edges, initial focus, and suspended-frame timing.
    anchors = [
        (0x45D40A, "c7822c57000002000000", "UI0 cancel opens UI2"),
        (0x45D41E, "6a1f", "menu frontend selector1F"),
        (0x45D440, "c681a055000001", "menu suspends level time"),
        (0x45D44D, "c682a155000000", "clear pending resume request on open"),
        (0x45D508, "c7812c57000000000000", "UI1 pause/cancel requests UI0"),
        (0x45D575, "c7802c57000001000000", "UI2 O-action closes to UI1, not Resume"),
        (0x45D597, "c680a455000000", "consume retry request"),
        (0x45D5E0, "6a01", "retry requests reload"),
        (0x45D606, "c7822c57000003000000", "UI2 Quit dispatches UI3"),
        (0x45D69B, "6a23", "UI3 Quit selects existing Map selector23"),
        (0x45D80C, "c7812c57000000000000", "Resume request selects UI0"),
        (0x45D81C, "c682a055000000", "Resume unfreezes level"),
        (0x45D858, "c680a155000000", "consume Resume request"),
        (0x45DAF9, "0f8564020000", "suspended arm routes to zero-dt simulation"),
        (0x45CE8C, "83b92c57000000", "cue advances only in UI0"),
        (0x45DD96, "83bd60ffffff02", "scene draw gate is gameplay mode2, not UI"),
        (0x48F4F2, "c700b0264b00", "menu widget vtable"),
        (0x48F522, "c705e0595d0000000000", "menu layout baseY zero and shared row offset"),
        (0x48F530, "6a056828424c00", "build five menu rows"),
        (0x48F599, "c605a9bf4c0001", "menu cancel requests Resume"),
        (0x48F5D2, "c605a9bf4c0001", "Resume row requests Resume"),
        (0x48F5E6, "c605acbf4c0001c60584354e0000", "Retry requests reload and disables intro"),
        (0x48F601, "c605aabf4c0001", "Quit row requests Map only"),
        (0x48F615, "6a006a1f6a09", "Options routes selector9 with return1F"),
        (0x48F637, "6a006a1f6a19", "empty fifth row routes selector19 with return1F"),
        (0x488B5F, "8b45088b0851", "shared initial focus is first row"),
        (0x48FB91, "c7401003000000", "menu widgets initialized visible and enabled"),
        (0x48686F, "7e30", "strict left content bound"),
        (0x48687A, "7e25", "strict top content bound"),
        (0x48688A, "7d15", "strict right content bound"),
        (0x48689B, "7d04", "strict bottom content bound"),
    ]
    for va, encoded, meaning in anchors:
        expected = bytes.fromhex(encoded)
        assert read(va, len(expected)) == expected, f"{va:#x}: {meaning}"
        print(f"PASS {va:08x}: {meaning}")

    row_table = struct.unpack("<5I", read(0x4C4228, 20))
    assert row_table == (44, 48, 43, 3, 0), f"unexpected localized menu rows: {row_table}"
    print("PASS 004c4228: localized menu table {44,48,43,3,0}")
    print(f"PASS: {len(anchors)} raw-backed anchors and menu table; offline byte check only")


if __name__ == "__main__":
    main()
