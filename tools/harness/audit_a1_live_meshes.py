#!/usr/bin/env python3
"""Parse captured A1 direct and gated inline quads against vertex/material banks.

This is a READ-only offline audit. It preserves raw payloads under ignored
build/tmp and reports +0, +4, and active +8 inline indices/UVs/materials
without rendering or claiming camera-bin selection.
"""
from __future__ import annotations

import hashlib
import json
import struct
import argparse
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CAPTURE = ROOT / "build/tmp/a1-cell-fixture/live-capture"
EXTRA = ROOT / "build/tmp/a1-cell-fixture/facilitator-extra-meshes"
OUT = ROOT / "build/tmp/a1-cell-fixture/live-mesh-audit.json"
AUTHORITY = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
POOL_HASH = "4e3a52f579c663fc0e97d217d8ea5d780c246f9c6bb33c112534ac6b20c3b630"


def parse_quad(raw: bytes) -> dict:
    if len(raw) != 0x80:
        raise ValueError(f"quad is {len(raw)} bytes, expected 0x80")
    corners = []
    for i in range(4):
        at = 0x20 + i * 0x18
        index = struct.unpack_from("<I", raw, at)[0]
        u, v = struct.unpack_from("<2d", raw, at + 8)
        corners.append({"source_index": index, "uv": [u, v]})
    return {"header_hex": raw[:0x20].hex(), "corners": corners}


def main() -> None:
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--capture-dir",type=Path,default=CAPTURE,help="READ capture directory produced by capture_a1_live_cells.py")
    ap.add_argument("--extra-dir",type=Path,default=EXTRA,help="legacy supplemental +4 capture; optional when capture contains mesh4")
    ap.add_argument("--output",type=Path,default=OUT,help="audit JSON output path")
    args=ap.parse_args()
    capture_dir=args.capture_dir.resolve()
    live = json.loads((capture_dir / "manifest.json").read_text(encoding="utf-8-sig"))
    extra_path=args.extra_dir/"manifest.json"
    extra=json.loads(extra_path.read_text(encoding="utf-8-sig")) if extra_path.exists() else None
    if live["loaded_level"].lower()!="compiled\\mm_climb_002.lvl":
        raise RuntimeError(f"unexpected loaded level {live['loaded_level']}")
    if extra and live["camera"]["sha256"] != extra["camera_sha256"]:
        raise RuntimeError("legacy supplemental +4 capture camera does not match")
    source = (capture_dir / live["source_pool"]["file"]).read_bytes()
    materials_raw = (capture_dir / live["materials"]["file"]).read_bytes()
    assert hashlib.sha256(source).hexdigest() == POOL_HASH
    assert len(source) == 897 * 24
    materials = [struct.unpack_from("<II2d", materials_raw, i * 24)
                 for i in range(len(materials_raw) // 24)]
    assert len(materials) == 48
    extra_by_key = {(row["group"], row["entry"]): row for row in extra["entries"]} if extra else {}
    rows = []
    route_totals = Counter()
    global_indices = Counter()
    global_uv = []
    material_to_textures = Counter()
    inline_inner_count = 0
    inline_inner_indices = Counter()
    inline_inner_uv = []
    for entry in live["entries"]:
        group, index = entry["group"], entry["entry"]
        key = (group, index)
        material_index = entry["serialized_material_runtime"]
        assert 0 <= material_index < 48
        selected = materials[material_index]
        routes = {}
        for route in ("mesh0","mesh4"):
            if route == "mesh0":
                mesh = entry.get("mesh") or {}
                filename = mesh.get("raw_file")
                count = mesh.get("count", 0)
                directory = capture_dir
            else:
                mesh=entry.get("mesh4") or {}
                if mesh.get("raw_file"):
                    filename=mesh["raw_file"];count=mesh.get("count",0);directory=capture_dir
                else:
                    extra_row = extra_by_key.get(key)
                    filename = extra_row["raw_file"] if extra_row else None
                    count = extra_row["count"] if extra_row else 0
                    directory = args.extra_dir
            if not filename:
                routes[route] = {"count": 0, "quads": [], "source_indices_in_range": True}
                continue
            raw = (directory / filename).read_bytes()
            if len(raw) != count * 0x80:
                raise ValueError(f"{filename}: {len(raw)} bytes does not match count {count}")
            quads = [parse_quad(raw[i * 0x80:(i + 1) * 0x80]) for i in range(count)]
            indices = [c["source_index"] for q in quads for c in q["corners"]]
            uv = [c["uv"] for q in quads for c in q["corners"]]
            in_range = all(value < 897 for value in indices)
            finite = all(all(map(lambda v: v == v and abs(v) != float("inf"), pair)) for pair in uv)
            route_totals[route] += count
            global_indices.update(indices)
            global_uv.extend(uv)
            routes[route] = {
                "count": count,
                "raw_file": filename,
                "raw_sha256": hashlib.sha256(raw).hexdigest(),
                "source_indices_in_range": in_range,
                "source_index_minmax": [min(indices), max(indices)] if indices else None,
                "distinct_source_indices": len(set(indices)),
                "uv_all_finite": finite,
                "uv_minmax": [[min(pair[i] for pair in uv), max(pair[i] for pair in uv)]
                              for i in range(2)] if uv else None,
                "quads": quads,
            }
        material_to_textures[str(material_index)] += 1
        record=bytes.fromhex(entry["record_hex"])
        inline_inner=None
        if record[0xF8] != 0:
            inline_material=material_index+24
            if inline_material>=len(materials):
                raise ValueError(f"{key}: active B8 inline material {inline_material} outside descriptor table")
            corners=[]
            for corner in range(4):
                at=8+0x20+corner*0x18
                source_index=struct.unpack_from("<I",record,at)[0]
                uv=struct.unpack_from("<2d",record,at+8)
                if source_index>=897 or not all(map(lambda value:value==value and abs(value)!=float("inf"),uv)):
                    raise ValueError(f"{key}: invalid active B8 inline corner {corner}")
                inline_inner_indices[source_index]+=1
                inline_inner_uv.append(uv)
                corners.append({"source_index":source_index,"uv":list(uv)})
            inline_inner_count+=1
            inline_inner={"flag_f8":record[0xF8],"count":1,"stride":0x80,
                          "raw_sha256":hashlib.sha256(record[8:0x88]).hexdigest(),
                          "material_index":inline_material,
                          "descriptor_texture_id":materials[inline_material][0],
                          "descriptor_blend_selector":materials[inline_material][1],
                          "descriptor_uv_doubles":list(materials[inline_material][2:]),
                          "corners":corners}
        rows.append({
            "group": group, "entry": index,
            "material_index": material_index,
            "descriptor_texture_id": selected[0],
            "descriptor_blend_selector": selected[1],
            "descriptor_uv_doubles": list(selected[2:]),
            "inline_inner":inline_inner,
            "mesh_routes": routes,
        })
    report = {
        "authority": {"sha256": AUTHORITY},
        "runtime_capture": {
            "pid": live["pid"], "loaded_level": live["loaded_level"],
            "camera_sha256": live["camera"]["sha256"],
            "group_count": 32, "entry_count": len(rows),
            "source_pool_sha256": hashlib.sha256(source).hexdigest(),
            "source_pool_vertices": len(source) // 24,
            "materials_sha256": hashlib.sha256(materials_raw).hexdigest(),
            "material_descriptors": len(materials),
        },
        "parsed_meshes": {
            "stride": 0x80, "vertex_corner_stride": 0x18,
            "mesh0_quads": route_totals["mesh0"],
            "mesh4_quads": route_totals["mesh4"],
            "all_referenced_source_indices_in_range": all(
                row["mesh_routes"][route]["source_indices_in_range"]
                for row in rows for route in ("mesh0", "mesh4")),
            "source_index_minmax": [min(global_indices), max(global_indices)] if global_indices else None,
            "source_index_count_occurrences": sum(global_indices.values()),
            "source_index_distinct_count": len(global_indices),
            "uv_minmax": [[min(pair[i] for pair in global_uv), max(pair[i] for pair in global_uv)]
                          for i in range(2)] if global_uv else None,
            "material_index_entry_counts": dict(material_to_textures),
            "inline_inner_active_quad_count":inline_inner_count,
            "inline_inner_all_references_in_pool":all(index<897 for index in inline_inner_indices),
            "inline_inner_source_index_minmax":[min(inline_inner_indices),max(inline_inner_indices)] if inline_inner_indices else None,
            "inline_inner_uv_minmax":[[min(pair[i] for pair in inline_inner_uv),max(pair[i] for pair in inline_inner_uv)] for i in range(2)] if inline_inner_uv else None,
        },
        "entries": rows,
        "unsettled": [
            "Which angle-bin entries the walker submits for each retained reference camera",
            "Order and depth/sort interpretation for +0 and +4 mesh routes",
            "Material registration name-to-texture mapping for all captured descriptors",
            "Complete original depth clipping, blending and visibility behavior",
        ],
        "scope": "bounded runtime-derived geometry oracle; READ capture only; no pixels rendered",
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(json.dumps({"output": str(args.output), "mesh0_quads": route_totals["mesh0"],
                      "mesh4_quads": route_totals["mesh4"],
                      "source_index_minmax": report["parsed_meshes"]["source_index_minmax"],
                      "all_references_in_pool": report["parsed_meshes"]["all_referenced_source_indices_in_range"]}, indent=2))


if __name__ == "__main__":
    main()
