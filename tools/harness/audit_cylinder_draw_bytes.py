"""Read source-backed raw bytes for the A1 cylinder-draw citations."""
import json
import struct
import hashlib
from pathlib import Path

EXE = Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")
OUT = Path("build/tmp/a1-cylinder-draw/citations.json")
RANGES = [
    (0x45E0E6, 5), (0x441845, 0x293), (0x43A4EC, 0x578),
    (0x43BD80, 0x69), (0x4623A5, 0x5F), (0x462404, 0x61),
    (0x462590, 0x13), (0x4599C0, 0x129), (0x46D372, 0x8E),
    (0x46D3C1, 0x9F), (0x46D460, 0x1A5), (0x439FD3, 0x1F3),
    (0x43AA91, 0x16C),
]
data = EXE.read_bytes()
digest = hashlib.sha256(data).hexdigest()
assert digest == "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f", "Stock authority differs"
pe = struct.unpack_from("<I", data, 0x3C)[0]
nsec = struct.unpack_from("<H", data, pe + 6)[0]
optsz = struct.unpack_from("<H", data, pe + 20)[0]
secbase = pe + 24 + optsz
sections = []
for i in range(nsec):
    h = secbase + 40 * i
    name = data[h:h+8].split(b"\0", 1)[0].decode("ascii")
    vsize, va, rawsize, raw = struct.unpack_from("<IIII", data, h + 8)
    sections.append((name, va + 0x400000, vsize, rawsize, raw))

records = []
for va, size in RANGES:
    mapped = None
    for name, start, vsize, rawsize, raw in sections:
        delta = va - start
        if 0 <= delta and delta + size <= rawsize:
            mapped = (name, raw + delta, data[raw+delta:raw+delta+size])
            break
    if mapped is None:
        records.append({"va":hex(va),"bytes":"","claim":"","role":"not fully file-backed"})
        continue
    name, offset, payload = mapped
    records.append({"va":hex(va),"bytes":payload.hex(" "),"file_offset":hex(offset),"section":name,
                    "claim":"complete raw executable window","role":"stock PE bytes"})
obj = {"raw_windows": records, "executable_file_sha256": digest,
       "scope": "Raw READ windows only; interpretations require independent verification"}
OUT.parent.mkdir(parents=True, exist_ok=True)
OUT.write_text(json.dumps(obj, indent=2), encoding="utf-8")
print(json.dumps({"output": str(OUT), "file_backed_windows": len(records), "sha256": digest}, indent=2))
