#!/usr/bin/env python3
"""Re-readable A4f code/data windows with PE raw-range provenance."""
from __future__ import annotations
import hashlib
import json
import sys
from pathlib import Path

import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

EXPECTED = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
TARGET = Path(sys.argv[1] if len(sys.argv) > 1 else r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")
# Small windows around actor-controller selection, actual draw/quad consumer,
# and the generic (unproven-for-lemmings) billboard object constructor.
WINDOWS = [
    (0x0042DE51, 0x110, "actor constructor"),
    (0x0042E0A0, 0x160, "actor construction and resource initialization"),
    (0x0043B517, 0x220, "actor material/lighting initialization lead"),
    (0x0042E140, 0x80, "actor constructor post-resource call site"),
    (0x0042E14D, 0x40, "actor constructor callsite 42E14D"),
    (0x004233A0, 0x180, "actor resource property accessor target"),
    (0x0045942D, 0x260, "actor render update before draw"),
    (0x0043D56C, 0x200, "actor draw subobject update call target"),
    (0x0042EEF2, 0x400, "actor update, movement source and chooser call"),
    (0x0042DCB0, 0x100, "actor draw-object position update from actor state"),
    (0x0043CE00, 0x90, "actor screen-draw object constructor"),
    (0x004680F0, 0x220, "actor graphics-controller constructor and vertex defaults"),
    (0x004409E0, 0x180, "base graphics-controller constructor"),
    (0x00440A10, 0x200, "inline quad vertex constructors"),
    (0x00420D60, 0x100, "quad vertex constructor A"),
    (0x00420DA0, 0x100, "quad vertex constructor B"),
    (0x0043CC68, 0x110, "actor screen-draw method and selected-frame dispatch"),
    (0x0043C9D2, 0x110, "actor scaled screen-draw variant"),
    (0x00468D00, 0x80, "action record accessor"),
    (0x00468CE0, 0x60, "temporal record accessor"),
    (0x0047A75D, 0x500, "selected directional frame to actor quad"),
    (0x00440C50, 0x100, "four actor destination corner submitter"),
    (0x0047A190, 0x220, "selected directional frame scaled quad variant"),
    (0x0047ABC0, 0x20, "directional record stride"),
    (0x0047AB96, 0x30, "texture selection and quad submit"),
    (0x00414920, 0x200, "sprite/material submit caller"),
    (0x00414D80, 0x180, "sprite material/vertex setup"),
    (0x0046D3C1, 0xA0, "texture-stage material helper"),
    (0x0046D460, 0x220, "vertex color packing and submission"),
    (0x0049C5AC, 0x80, "native double-to-byte color conversion"),
    (0x0047B320, 0x100, "generic resource sprite constructor"),
    (0x0047B44C, 0x140, "generic resource sprite descriptor initializer"),
    (0x0047D0C0, 0x100, "generic billboard corner/projector; actor identity unproven"),
    (0x004B1AD8, 0x10, "generic billboard vtable"),
    (0x004B14A8, 0x20, "actor screen-draw vtable"),
]


def main() -> int:
    data = TARGET.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if digest != EXPECTED:
        raise SystemExit(f"wrong authority digest: {digest}")
    pe = pefile.PE(data=data, fast_load=True)
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = False
    rows = []
    for va, length, label in WINDOWS:
        rva = va - pe.OPTIONAL_HEADER.ImageBase
        section = pe.get_section_by_rva(rva)
        if section is None:
            raise SystemExit(f"unmapped VA 0x{va:x}")
        delta = rva - section.VirtualAddress
        if delta < 0 or delta + length > section.SizeOfRawData:
            raise SystemExit(f"VA 0x{va:x} window is not fully inside initialized section raw bytes")
        off = section.PointerToRawData + delta
        raw = data[off:off + length]
        if off + length > len(data):
            raise SystemExit(f"VA 0x{va:x} window runs past file")
        rows.append({
            "label": label,
            "address": f"0x{va:08x}",
            "section": section.Name.rstrip(b"\0").decode("ascii", "replace"),
            "file_offset": f"0x{off:x}",
            "requested_bytes": length,
            "bytes_hex": raw.hex(),
            "instructions": [f"0x{i.address:08x}: {i.bytes.hex()} {i.mnemonic} {i.op_str}" for i in md.disasm(raw, va)],
        })
    result = {
        "authority": str(TARGET),
        "sha256": digest,
        "image_base": f"0x{pe.OPTIONAL_HEADER.ImageBase:x}",
        "pe_sections": [
            {"name": s.Name.rstrip(b"\0").decode("ascii", "replace"),
             "rva": f"0x{s.VirtualAddress:x}", "raw_offset": f"0x{s.PointerToRawData:x}",
             "raw_size": f"0x{s.SizeOfRawData:x}",
             "raw_end": f"0x{s.PointerToRawData + s.SizeOfRawData:x}"}
            for s in pe.sections
        ],
        "windows": rows,
    }
    rendered = json.dumps(result, indent=2)
    if len(sys.argv) > 2:
        Path(sys.argv[2]).write_text(rendered + "\n", encoding="utf-8")
    else:
        print(rendered)
    return 0

if __name__ == "__main__":
    raise SystemExit(main())

