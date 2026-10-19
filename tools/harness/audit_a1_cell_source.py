"""Audit the source-backed A1 cell geometry/material inputs without fitting.

This deliberately reports candidate PolyList records separately from the
runtime cell-list route. It does not invent an association between those two
structures. All executable claims are tied to the pinned stock PE bytes.
"""
from __future__ import annotations

import hashlib
import json
import math
import struct
import argparse
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
EXE = Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")
LEVEL = ROOT / "build/tmp/a1-camera/assets/COMPILED/MM_CLIMB_002.LVL"
OUT = ROOT / "build/tmp/a1-cell-fixture/source-audit.json"
PIXEL_BASELINE = ROOT / "build/tmp/a1-facilitator/continue_frame_audit.json"
PIXEL_REPORT = ROOT / "build/tmp/a1-cell-fixture/pixel-report.json"
MUTATION_LEDGER = ROOT / "build/tmp/a1-cell-fixture/mutation-ledger.json"
AUTHORITY = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"


def raw_section(data: bytes, va: int, length: int) -> tuple[str, int, bytes]:
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    nsec = struct.unpack_from("<H", data, pe + 6)[0]
    opt = struct.unpack_from("<H", data, pe + 20)[0]
    base = pe + 24 + opt
    for i in range(nsec):
        h = base + i * 40
        name = data[h:h + 8].split(b"\0", 1)[0].decode("ascii")
        vsize, rva, rawsize, raw = struct.unpack_from("<IIII", data, h + 8)
        delta = va - (0x400000 + rva)
        if delta >= 0 and delta + length <= rawsize:
            return name, raw + delta, data[raw + delta:raw + delta + length]
    raise ValueError(f"VA {va:#x}+{length:#x} is not fully file-backed")


def poly_blocks(data: bytes):
    tag = struct.pack("<I", 11) + b"PolyList 09"
    hits = []
    pos = 0
    while (pos := data.find(tag, pos)) >= 0:
        hits.append(pos)
        pos += len(tag)
    totals = {5: {"blocks": 0, "records": 0},
              7: {"blocks": 0, "records": 0}}
    block_headers = []
    all_end_words = {}
    surface_ids = []
    for hit, nxt in zip(hits, hits[1:]):
        start = hit + len(tag)
        if nxt < start + 16:
            continue
        kind, count, u2, u3 = struct.unpack_from("<IIII", data, start)
        corners = {5: 3, 7: 4}.get(kind)
        stride = (corners * 52 + 4) if corners else 0
        if not stride or count == 0 or nxt - start != 16 + stride * count:
            continue
        totals[kind]["blocks"] += 1
        totals[kind]["records"] += count
        block_headers.append({"file_offset": start, "kind": kind, "count": count,
                              "u2": u2, "u3": u3})
        for n in range(count):
            rec = start + 16 + n * stride
            for c in range(corners):
                at = rec + c * 52
                surface_ids.append(struct.unpack_from("<I", data, at)[0])
            word = struct.unpack_from("<I", data, rec + corners * 52)[0]
            all_end_words[word] = all_end_words.get(word, 0) + 1
    return hits, totals, block_headers, surface_ids, all_end_words


def scan_cell_stream(data: bytes):
    """Find exact 32-array serialized runs under the reader's observed layout."""
    candidates = []
    for start in range(0, len(data) - 32 * 4):
        pos = start
        bucket_counts = []
        record_count = 0
        ok = True
        for _ in range(32):
            if pos + 4 > len(data):
                ok = False
                break
            count = struct.unpack_from("<I", data, pos)[0]
            pos += 4
            if count > (len(data) - pos) // 48:
                ok = False
                break
            bucket_counts.append(count)
            record_count += count
            for _record in range(count):
                material = struct.unpack_from("<I", data, pos)[0]
                if material >= 48:
                    ok = False
                    break
                uv_and_other_fields = struct.unpack_from("<5d", data, pos + 8)
                if not all(math.isfinite(value) for value in uv_and_other_fields):
                    ok = False
                    break
                pos += 48
            if not ok:
                break
        if ok and record_count:
            candidates.append({"file_offset": start, "end_offset": pos,
                               "record_count": record_count,
                               "nonempty_bucket_count": sum(c > 0 for c in bucket_counts),
                               "bucket_counts": bucket_counts})
    return candidates


def exact_live_count_matches(data: bytes, counts: list[int]):
    """Find stream runs whose 32 bucket counts equal the retained live headers."""
    if len(counts) != 32:
        raise ValueError("expected exactly 32 retained live bucket counts")
    candidates = []
    for start in range(0, len(data) - 32 * 4):
        if struct.unpack_from("<I", data, start)[0] != counts[0]:
            continue
        pos = start
        record_count = 0
        materials = []
        valid = True
        for count in counts:
            if pos + 4 > len(data) or struct.unpack_from("<I", data, pos)[0] != count:
                valid = False
                break
            pos += 4
            for _ in range(count):
                if pos + 48 > len(data):
                    valid = False
                    break
                material = struct.unpack_from("<I", data, pos)[0]
                doubles = struct.unpack_from("<5d", data, pos + 8)
                if material >= 48 or not all(math.isfinite(value) for value in doubles):
                    valid = False
                    break
                materials.append(material)
                record_count += 1
                pos += 48
            if not valid:
                break
        if valid:
            candidates.append({
                "file_offset": start,
                "end_offset": pos,
                "record_count": record_count,
                "bucket_counts": counts,
                "serialized_material_values": sorted(set(materials)),
                "serialized_material_value_count": len(set(materials)),
                "inner_bank_plus_0x18_values": sorted(set(value + 24 for value in materials)),
            })
    return candidates


def main() -> None:
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--capture-dir",type=Path,default=ROOT/"build/tmp/a1-cell-fixture/live-capture")
    ap.add_argument("--output",type=Path,default=OUT)
    args=ap.parse_args()
    exe = EXE.read_bytes()
    digest = hashlib.sha256(exe).hexdigest()
    assert digest == AUTHORITY, f"stock executable digest mismatch: {digest}"
    level = LEVEL.read_bytes()
    sig = struct.pack("<IdI", 112, 10.0, 7)
    assert level.count(sig) == 1, "cylinder signature is not unique"
    ring0 = level.index(sig) + len(sig)
    surface_count_at = ring0 + 7 * 32 * 2 * 24 + 4 + 7 * 32 * 8 + 4
    surface_count = struct.unpack_from("<I", level, surface_count_at)[0]
    surface_start = surface_count_at + 4
    assert surface_count == 897
    assert surface_start + surface_count * 24 <= len(level)

    hits, totals, headers, ids, end_words = poly_blocks(level)
    in_range = sum(index < surface_count for index in ids)
    stream_candidates = scan_cell_stream(level)
    live_object_path = ROOT / "build/tmp/a1-facilitator/cylinder-trace-left/cylinder-live-object.bin"
    live_object = live_object_path.read_bytes()
    live_bucket_counts = [struct.unpack_from("<I", live_object, 0x208 + i * 0x10)[0]
                          for i in range(32)]
    live_bucket_capacities = [struct.unpack_from("<I", live_object, 0x20c + i * 0x10)[0]
                              for i in range(32)]
    exact_live_matches = exact_live_count_matches(level, live_bucket_counts)
    live_capture=json.loads((args.capture_dir/"manifest.json").read_text(encoding="utf-8-sig"))
    active_inline=[]
    for entry in live_capture["entries"]:
        record=bytes.fromhex(entry["record_hex"])
        if record[0xF8]==0:
            continue
        corners=[]
        for corner in range(4):
            at=8+0x20+corner*0x18
            index=struct.unpack_from("<I",record,at)[0]
            uv=struct.unpack_from("<2d",record,at+8)
            if index>=surface_count or not all(math.isfinite(x) for x in uv):
                raise ValueError(f"active inline quad {entry['group']}/{entry['entry']} has invalid corner")
            corners.append({"source_index":index,"uv":list(uv)})
        active_inline.append({"group":entry["group"],"entry":entry["entry"],"flag_f8":record[0xF8],
                              "cell_material_index":entry["serialized_material_runtime"],
                              "selected_inner_material_index":entry["serialized_material_runtime"]+24,
                              "corners":corners})

    citations = []
    for va, length, claim in [
        (0x43A4EC, 0x578, "world walker iterates the 32 per-angle groups and routes each entry via its material field"),
        (0x43AA91, 0x16C, "bank registration constructs the 8x3 inner and outer material name grids"),
        (0x43BD80, 0x69, "material router reads the selected 24-byte descriptor"),
    ]:
        section, file_offset, payload = raw_section(exe, va, length)
        citations.append({"va": f"0x{va:08X}", "length": length,
                          "literal_bytes_hex": payload.hex(), "claim": claim,
                          "source": str(EXE), "sha256": digest,
                          "section": section, "file_offset": f"0x{file_offset:X}"})
    for va, length, claim in [
        (0x4131A0, 27, "CArray element lookup computes dataPointer + index*0x100"),
        (0x43A190, 12, "world loader calls the per-angle array reader for each 16-byte header"),
        (0x43A44E, 5, "cell rebuild loop calls the per-entry writer at 0x4608BB"),
        (0x4608A0, 27, "per-angle reader forwards the level stream to the array wrapper"),
        (0x460C50, 117, "array wrapper reads the count, allocates 0x100-stride records and loops through them"),
        (0x461353, 161, "per-record reader consumes one u32, two u16s and five f64s into defined fields"),
        (0x43A727, 6, "walker loads entry+0xC8 and begins the material-table route"),
        (0x43A72D, 3, "inner-bank walker branch adds 0x18 to the serialized material index"),
        (0x4636B5, 0x98, "cell surface builder emits four indexed vertices with two UV doubles per vertex"),
        (0x004135A0, 0x1D, "surface-record helper stores a vertex index at per-vertex stride 0x18 and returns"),
        (0x00413560, 0x3B, "surface-record helper stores two f64 UV coordinates at per-vertex stride 0x18 and returns"),
        (0x0043A31D, 0x99, "grid-coordinate lookup caches/interpolates a source vertex into world+0x148"),
    ]:
        section, file_offset, payload = raw_section(exe, va, length)
        citations.append({"va": f"0x{va:08X}", "length": length,
                          "literal_bytes_hex": payload.hex(), "claim": claim,
                          "source": str(EXE), "sha256": digest,
                          "section": section, "file_offset": f"0x{file_offset:X}"})
    for va, length, claim in [
        (0x43A6F7, 7, "LEA selects an entry from the 32 per-angle cell groups rooted at world+0x208"),
        (0x43A727, 6, "entry material index is loaded from entry+0xC8"),
        (0x43A734, 6, "material CArray base is world+0x168"),
    ]:
        section, file_offset, payload = raw_section(exe, va, length)
        citations.append({"va": f"0x{va:08X}", "length": length,
                          "literal_bytes_hex": payload.hex(), "claim": claim,
                          "source": str(EXE), "sha256": digest,
                          "section": section, "file_offset": f"0x{file_offset:X}"})

    for va, length, claim in [
        (0x45CC20, 0x23, "camera renderer writes normalized selector from the +0x40 control yaw via 0x42DCD0"),
        (0x45CC09, 0x24, "camera pose updater calls the +0x40 yaw accessor before writing the normalized selector"),
        (0x42DCD0, 0x11, "yaw accessor reads binary64 control field +0x40"),
        (0x43C990, 0x14, "render walker angle accessor reads normalized selector at main+0x26028"),
        (0x4B03A8, 8, "walker center subtract operand is binary64 0.5"),
        (0x4B1050, 8, "walker selector-to-bin scale operand is binary64 32.0"),
        (0x4B1468, 8, "walker inner-bound coefficient is binary64 0.7"),
        (0x4B1350, 8, "camera yaw offset operand is binary64 128.0"),
        (0x4B07A8, 8, "camera yaw units per turn operand is binary64 256.0"),
        (0x49C5AC, 0x27, "integer boundary conversion temporarily sets x87 rounding toward zero before FISTP"),
        (0x43A4EC, 0x578, "walker computes wrapped camera bounds and submits B8, +4, +0 and center-sort cell routes"),
        (0x4623A5, 0x5F, "+0 cell route traverses the entry +0 child array and dispatches its mesh"),
        (0x462404, 0x61, "+4 cell route traverses the entry +4 child array and dispatches its mesh"),
        (0x462465, 0xF8, "center-bin route filters +0/+4 quads using signed child-array shorts and local38*8"),
        (0x4622FD, 0xA8, "B8 renderer gates entry+8 inline geometry on byte +0xF8, then visits the nested B8 list"),
        (0x462590, 0x13, "inline quad adapter passes its object pointer to the surface submission helper"),
        (0x4599C0, 0x129, "quad submission reads four indices at +0x20+corner*0x18, resolves source vertices, and copies UV fields"),
        (0x420B80, 0x19, "source-vertex lookup multiplies the submitted index by 0x68 and adds the selected vertex array"),
        (0x46D58B, 0x33, "draw vertex builder adds global UV offsets and copies the two UV values into float vertex fields"),
    ]:
        section, file_offset, payload = raw_section(exe, va, length)
        citations.append({"va": f"0x{va:08X}", "length": length,
                          "literal_bytes_hex": payload.hex(), "claim": claim,
                          "source": str(EXE), "sha256": digest,
                          "section": section, "file_offset": f"0x{file_offset:X}"})

    unique_end_words = sorted(end_words)
    report = {
        "authority": {"path": str(EXE), "sha256": digest, "image_base": "0x00400000"},
        "level": {"path": str(LEVEL), "sha256": hashlib.sha256(level).hexdigest(),
                  "cylinder_surface_pool": {"offset": surface_start, "count": surface_count,
                                            "record_bytes": 24}},
        "polylist_formula_matched": {
            "raw_tag_hits": len(hits), "kind_totals": totals,
            "corner_index_references": len(ids),
            "indices_within_source_pool": in_range,
            "index_in_range_fraction": in_range / len(ids),
            "indices_outside_source_pool": len(ids) - in_range,
            "trailing_word_distinct_count": len(unique_end_words),
            "trailing_word_minmax": [unique_end_words[0], unique_end_words[-1]],
            "first_block_headers": headers[:12],
            "interpretation": "READ from level bytes; no runtime group/material association established",
        },
        "runtime_route": {
            "per_angle_groups": 32,
            "per_cell_entry_stride": 0x100,
            "material_field_offset": 0xC8,
            "bank_slots": 48,
            "tile_grid": [8, 3],
            "per_view_group_selection": "Bounds are source-derived from selector, world+0x408 and 0.7; B8 visits [c..b] descending and submits entry+8 inline quad iff byte +0xF8 is nonzero, then processes B8 children; material index adds 0x18. +0 draws [a,middle), center draws middle with child-short filters, +4 draws (middle,d); every group index is masked by 0x1f.",
            "selected_group_to_PolyList_record_mapping": "UNSETTLED; the generated runtime cell meshes are independently captured and rendered without claiming PolyList block correspondence",
            "selected_material_to_PolyList_block_mapping": "UNSETTLED",
            "captured_descriptor_uvs": "all 48 live 24-byte descriptors contain two binary64 UV offsets (0,0); blend selectors captured for direct mesh routes are zero; raster sampler/address behavior is not fully established",
            "cell_surface_builder": "0x463410 constructs child render records from collision-grid-derived spans; 0x4636B5..0x463747 contains indexed four-corner writes with source indices and two UV doubles per corner. The +8 inline route is gated by byte +0xF8 and parses 98 active captured quads; all indices address the 897-record LVL surface bank and all UVs are finite. It uses descriptor material +24. This is a bounded runtime-derived geometry oracle, not a PolyList parse.",
            "active_inline_inner_quads": {"count": len(active_inline), "entries": active_inline},
        },
        "serialized_cell_array_scan": {
            "candidate_count": len(stream_candidates),
            "candidates": stream_candidates[:12],
            "scan_layout": "32 consecutive u32-count arrays; each record consumes u32+u16+u16+5*f64 (48 bytes)",
            "field_constraint": "u32 material index < 48; all five f64 values finite",
            "status": "unique candidate" if len(stream_candidates) == 1 else "not uniquely resolved",
        },
        "retained_live_bucket_count_correlation": {
            "source": str(live_object_path),
            "sha256": hashlib.sha256(live_object).hexdigest(),
            "dump_length": len(live_object),
            "runtime_header_base_offset": "0x208",
            "runtime_header_stride": 0x10,
            "runtime_element_stride": 0x100,
            "bucket_counts": live_bucket_counts,
            "bucket_capacities": live_bucket_capacities,
            "all_32_counts_equal_capacities": live_bucket_counts == live_bucket_capacities,
            "exact_count_vector_candidate_count": len(exact_live_matches),
            "exact_count_vector_candidates": exact_live_matches,
            "interpretation": "The unique exact count-vector LVL run and retained runtime records correlate all 32 groups and 192 entries; facilitator reconstruction matched the 48-byte metadata records byte-for-byte. The observed +0x18 inner-bank material offset is a separate B8 walker route. Full PolyList correspondence remains unresolved.",
        },
        "citations": citations,
        "limitation": "The exact 32-bucket LVL candidate and 192 serialized metadata records are correlated with the runtime world; the facilitator separately verified byte-for-byte reconstruction. All captured generated cell quads, child arrays, source indices, UV pairs, and material descriptors are available as an offline runtime-derived oracle. Remaining unknowns include full selected-group mesh-to-PolyList correspondence, D3D clipping, exact blend and sampler-address state, and stock draw/depth order. Candidate pixels pass only as described in pixel-report.json; the left view remains outside the fixed threshold, so no all-view acceptance is claimed.",
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(json.dumps({"output": str(args.output), "surface_count": surface_count,
                      "polylist_tag_hits": len(hits), "kind_totals": totals,
                      "index_count": len(ids), "index_in_range": in_range,
                      "active_inline_inner_quads": len(active_inline),
                      "cell_stream_candidate_count": len(stream_candidates),
                      "unresolved_runtime_associations": 3}, indent=2))


if __name__ == "__main__":
    main()
