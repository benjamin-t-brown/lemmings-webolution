#!/usr/bin/env python3
"""Render the bounded captured A1 cell mesh routes beside the retained outer skin.

The selected cells, indexed vertices, material descriptors, and UV pairs come
from the retained READ capture. This is an offline candidate raster, not a
complete D3D renderer: clipping, blend-state details, and sampler addressing
have only the explicit choices recorded in the output manifest.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import struct
from pathlib import Path

from PIL import Image

from camera_fixture import candidate_camera, grabbed_mode2_yaw

ROOT = Path(__file__).resolve().parents[2]
FIX = Path(__file__).resolve().parent / "fixtures/camera_a1"
PRIVATE = ROOT / "build/tmp/a1-cell-fixture"
CAMERA_PRIVATE = ROOT / "build/tmp/a1-facilitator"
OUTDIR = PRIVATE / "candidates"
CAPTURE = PRIVATE / "live-capture"
EXTRA = PRIVATE / "facilitator-extra-meshes"
INNER_TEXTURES = PRIVATE / "facilitator-inner-textures"
OUTER_TEXTURES = CAMERA_PRIVATE
AUDIT = PRIVATE / "live-mesh-audit.json"
POOL_SHA = "4e3a52f579c663fc0e97d217d8ea5d780c246f9c6bb33c112534ac6b20c3b630"


def load_groups(capture_dir=CAPTURE, extra_dir=EXTRA, audit_path=AUDIT):
    live = json.loads((capture_dir / "manifest.json").read_text(encoding="utf-8-sig"))
    extra_path=extra_dir/"manifest.json"
    extra = json.loads(extra_path.read_text(encoding="utf-8-sig")) if extra_path.exists() else None
    parsed = json.loads(audit_path.read_text(encoding="utf-8"))
    assert live["loaded_level"] == "Compiled\\MM_CLIMB_002.lvl"
    assert live["camera"]["exact_reference_match"] is True
    if extra:
        assert live["camera"]["sha256"] == extra["camera_sha256"]
    source = (capture_dir / live["source_pool"]["file"]).read_bytes()
    assert hashlib.sha256(source).hexdigest() == POOL_SHA
    level=ROOT/"build/tmp/a1-camera/assets/COMPILED/MM_CLIMB_002.LVL"
    level_bytes=level.read_bytes()
    signature=struct.pack("<IdI",112,10.0,7)
    assert level_bytes.count(signature)==1
    pool_start=level_bytes.index(signature)+len(signature)
    pool_start += 7*32*2*24 + 4 + 7*32*8 + 4 + 4
    assert level_bytes[pool_start:pool_start+len(source)]==source
    vertices = [struct.unpack_from("<3d", source, i * 24) for i in range(897)]
    mat_raw = (capture_dir / live["materials"]["file"]).read_bytes()
    assert len(mat_raw) == 48 * 24
    materials = [struct.unpack_from("<II2d", mat_raw, i * 24) for i in range(48)]

    rows = {(row["group"], row["entry"]): row for row in parsed["entries"]}
    inline_quad_count = 0
    inline_active_indices = []
    inline_uvs = []
    live_by_key={(entry["group"],entry["entry"]):entry for entry in live["entries"]}
    for key, row in rows.items():
        entry = live_by_key[key]
        for route in ("mesh0", "mesh4"):
            item = row["mesh_routes"][route]
            if not item["count"]:
                continue
            if route=="mesh4" and (entry.get("mesh4") or {}).get("raw_file"):
                base=capture_dir
            elif route=="mesh4":
                base=extra_dir
            else:
                base=capture_dir
            child_name = "children_88" if route == "mesh0" else "children_98"
            child = entry[child_name]
            child_raw = (capture_dir / child["raw_file"]).read_bytes()
            if len(child_raw) != child["count"] * 4 or child["count"] != item["count"]:
                raise ValueError(f"{key} {route}: child count/stride does not match mesh count")
            item["child_values_i16"] = [struct.unpack_from("<h", child_raw, i * 4)[0]
                                         for i in range(child["count"])]
            item["child_raw_sha256"] = hashlib.sha256(child_raw).hexdigest()
            item["group"] = key[0]
            item["entry"] = key[1]
            item["material_index"] = row["material_index"]
            if row["material_index"] > 23:
                raise ValueError(f"direct mesh route has out-of-bank material {row['material_index']}")
            item["material"] = materials[row["material_index"]]
            item["selected_descriptor"] = row["descriptor_texture_id"]
        # The B8 walker first checks byte +0xF8, then submits the inline
        # 0x80 quad at entry+8. Inactive bytes are not a quad and must not be
        # decoded. The offsets below match the executable's 0x18 corner stride.
        record = bytes.fromhex(entry["record_hex"])
        flag = record[0xF8]
        inline = None
        if flag:
            raw_quad = record[8:8 + 0x80]
            corners = []
            for corner in range(4):
                at = 0x20 + corner * 0x18
                source_index = struct.unpack_from("<I", raw_quad, at)[0]
                uv = struct.unpack_from("<2d", raw_quad, at + 8)
                if source_index >= len(vertices) or not all(math.isfinite(x) for x in uv):
                    raise ValueError(f"{key} active inline quad has invalid corner {corner}")
                corners.append({"source_index": source_index, "uv": list(uv)})
                inline_active_indices.append(source_index)
                inline_uvs.append(uv)
            inline = {"flag_f8": flag, "raw_sha256": hashlib.sha256(raw_quad).hexdigest(),
                      "corners": corners, "material_index": row["material_index"] + 24,
                      "cell_material_index": row["material_index"]}
            inline_quad_count += 1
        row["inline_inner"] = inline
    return vertices, materials, rows, live


def to_int_chop(value: float) -> int:
    return math.trunc(value)


def boundaries(selector: float, extent: float, inner_coefficient: float = 0.7) -> list[int]:
    # Literal operands at 4B03A8, 4B1050, and 4B1468: 0.5, 32, and 0.7.
    center = (selector - 0.5) * 32.0
    wide = extent * 32.0
    narrow = extent * inner_coefficient * 32.0
    values = [to_int_chop(center - wide), to_int_chop(center - narrow),
              to_int_chop(center), to_int_chop(center + narrow),
              to_int_chop(center + wide)]
    while values[0] < 0:
        values = [v + 32 for v in values]
    while values[0] > 32:
        values = [v - 32 for v in values]
    return values


def choose_routes(bounds: list[int]) -> list[tuple[int, str]]:
    a, b, middle, c, d = bounds
    # Exact inclusive/exclusive loops in 43A6D2..43A901. Indexing in those
    # loops is subsequently masked with 0x1f before selecting the 32 headers.
    routes = [(i & 31, "inner") for i in range(c, b - 1, -1)]
    routes += [(i & 31, "mesh0") for i in range(a, middle)]
    routes += [(i & 31, "mesh4") for i in range(middle + 1, d)]
    routes += [(middle & 31, "center")]
    return routes


def triangle(points, uvs, tex, depth, image, width, height, blend_selector=0, address_mode="repeat", depth_test=True):
    a, b, c = points
    area = (b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])
    if abs(area) < 1e-12:
        return
    xmin=max(0, math.floor(min(p[0] for p in points)))
    xmax=min(width-1, math.ceil(max(p[0] for p in points)))
    ymin=max(0, math.floor(min(p[1] for p in points)))
    ymax=min(height-1, math.ceil(max(p[1] for p in points)))
    tw, th = tex.size
    src=tex.load(); dst=image.load()
    for y in range(ymin, ymax+1):
        for x in range(xmin, xmax+1):
            xx, yy=x+.5, y+.5
            w0=((b[0]-xx)*(c[1]-yy)-(b[1]-yy)*(c[0]-xx))/area
            w1=((c[0]-xx)*(a[1]-yy)-(c[1]-yy)*(a[0]-xx))/area
            w2=1.0-w0-w1
            if min(w0,w1,w2) < 0:
                continue
            q=w0*a[2]+w1*b[2]+w2*c[2]
            if q <= 0:
                continue
            z=1.0/q; pos=y*width+x
            if depth_test and z >= depth[pos]:
                continue
            # Executable vertex builder writes descriptor-adjusted UV as
            # floats. Repeat + bilinear are explicit candidate sampler rules.
            u=sum((w0,w1,w2)[i]*points[i][2]*uvs[i][0] for i in range(3))/q
            v=sum((w0,w1,w2)[i]*points[i][2]*uvs[i][1] for i in range(3))/q
            sx=(u % 1.0)*tw-.5; sy=(v % 1.0)*th-.5
            ix=math.floor(sx); iy=math.floor(sy); dx=sx-ix; dy=sy-iy
            if address_mode=="clamp":
                samples=[src[max(0,min(tw-1,ix+i)),max(0,min(th-1,iy+j))]
                         for j in (0,1) for i in (0,1)]
            elif address_mode=="repeat":
                samples=[src[(ix+i)%tw,(iy+j)%th] for j in (0,1) for i in (0,1)]
            else:
                raise ValueError(f"unsupported address mode {address_mode}")
            weights=((1-dx)*(1-dy),dx*(1-dy),(1-dx)*dy,dx*dy)
            color=tuple(round(sum(samples[k][ch]*weights[k] for k in range(4))) for ch in range(3))
            if blend_selector:
                # Preserve parsed selector in the manifest; this bounded route
                # currently supports only selector zero.
                raise ValueError(f"unsupported captured blend selector {blend_selector}")
            dst[x,y]=color
            depth[pos]=z


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--case", choices=("initial","mouse","right","left"), required=True)
    ap.add_argument("--capture-dir",type=Path,default=CAPTURE,help="READ capture directory produced by capture_a1_live_cells.py")
    ap.add_argument("--extra-dir",type=Path,default=EXTRA,help="legacy supplemental +4 capture when capture has no mesh4 route")
    ap.add_argument("--audit",type=Path,default=AUDIT,help="parsed runtime mesh audit JSON")
    ap.add_argument("--output-dir",type=Path,default=OUTDIR,help="candidate output directory under ignored build/tmp")
    ap.add_argument("--outer-texture-dir",type=Path,default=OUTER_TEXTURES,help="decoded OUT HI tile directory")
    ap.add_argument("--inner-texture-dir",type=Path,default=INNER_TEXTURES,help="decoded INN HI tile directory")
    ap.add_argument("--input-manifest",type=Path,default=None,help="optional frozen input manifest; listed files are rehashed before rendering")
    ap.add_argument("--label", default="cell")
    ap.add_argument("--inner-coefficient", type=float, default=0.7,
                    help="single-value mutation hook; source operand is 0.7")
    ap.add_argument("--center-filter-delta", type=float, default=0.0,
                    help="single-value mutation hook; source dispatch threshold adds no delta")
    ap.add_argument("--no-outer-skin", action="store_true",
                    help="render only selected captured cell routes as a control")
    ap.add_argument("--disable-cell-depth-test",action="store_true",
                    help="diagnostic painter-order control: keep clamped outer underlay and all cell routes, but skip reciprocal-depth rejection for cell triangles only")
    args=ap.parse_args()
    input_manifest_sha256=None
    if args.input_manifest:
        manifest_path=args.input_manifest.resolve()
        frozen=json.loads(manifest_path.read_text(encoding="utf-8"))
        for item in frozen["files"]:
            path=Path(item["path"])
            if not path.is_file() or path.stat().st_size!=item["bytes"] or hashlib.sha256(path.read_bytes()).hexdigest()!=item["sha256"]:
                raise RuntimeError(f"frozen input changed: {path}")
        input_manifest_sha256=hashlib.sha256(manifest_path.read_bytes()).hexdigest()
    params_path=FIX/({"initial":"camera-fixture-params.json",
                      "mouse":"rotation-mouse-params.json",
                      "right":"rotation-key-right-params.json",
                      "left":"rotation-key-left-params.json"}[args.case])
    params=json.loads(params_path.read_text())
    outer_texture_dir=args.outer_texture_dir.resolve()
    inner_texture_dir=args.inner_texture_dir.resolve()
    vertices, materials, entries, capture=load_groups(args.capture_dir.resolve(),args.extra_dir.resolve(),args.audit.resolve())
    yaw=grabbed_mode2_yaw(params) if "control_case" in params else float(params["yaw_units"])
    selector=(yaw+float(params["yaw_offset"])) / float(params["yaw_units_per_turn"])
    extent=.36
    bounds=boundaries(selector, extent, args.inner_coefficient)
    selected=choose_routes(bounds)
    matrix, eye, fx, fy=candidate_camera(params)
    width,height=params["viewport"]
    cx,cy=params["center"]
    image=Image.new("RGB",(width,height),(0,0,0))
    depth=[math.inf]*(width*height)

    def project(v):
        x,y,z=v
        px=matrix[0]*x+matrix[3]*y+matrix[6]*z+matrix[9]
        py=matrix[1]*x+matrix[4]*y+matrix[7]*z+matrix[10]
        pz=matrix[2]*x+matrix[5]*y+matrix[8]*z+matrix[11]
        if pz <= 0:
            return None
        return cx+fx*px/pz, cy-fy*py/pz, 1.0/pz

    textures={}
    def texture_for(material_index):
        # Registration order is the source 8x3 tile grid. The B8 route adds
        # 24 and selects the separately decoded INN bank.
        tile=material_index % 24
        col,row=tile%8,tile//8
        is_inner = material_index >= 24
        key=(is_inner,col,row)
        if key not in textures:
            bank = inner_texture_dir if is_inner else outer_texture_dir
            p=bank/f"tile-{col}-{row}HI.bmp"
            textures[key]=Image.open(p).convert("RGB")
        return textures[key]

    # Outer skin is rasterized into the same depth buffer from the established
    # clean fixture algorithm, then the selected captured geometry participates
    # in that same software z test. Its pixels are reproduced from the original
    # source LVL ring bank and already accepted camera/texture inputs.
    if not args.no_outer_skin:
        render_outer(image, depth, params, matrix, fx, fy, outer_texture_dir)

    rendered_counts={"inner":0,"mesh0":0,"mesh4":0,"center":0}
    thresholds={"mesh0":[],"mesh4":[]}
    center_param=(selector-.5)*32.0*8.0+args.center_filter_delta

    def draw_route(row,route,predicate):
        item=row["mesh_routes"][route]
        if not item["count"]: return
        texture=texture_for(row["material_index"])
        desc=materials[row["material_index"]]
        du,dv=desc[2],desc[3]
        for i,quad in enumerate(item["quads"]):
            if not predicate(item["child_values_i16"][i]):
                continue
            pts=[]; uvs=[]
            for c in quad["corners"]:
                p=project(vertices[c["source_index"]])
                if p is None:
                    pts=[];break
                pts.append(p);uvs.append((c["uv"][0]+du,c["uv"][1]+dv))
            if len(pts)!=4: continue
            rendered_counts[route]+=1
            for ids in ((0,1,2),(0,2,3)):
                triangle([pts[j] for j in ids],[uvs[j] for j in ids],texture,depth,image,width,height,desc[1],depth_test=not args.disable_cell_depth_test)

    def draw_inline_inner(row):
        item = row.get("inline_inner")
        if item is None:
            return
        material_index = item["material_index"]
        texture = texture_for(material_index)
        desc = materials[material_index]
        du, dv = desc[2], desc[3]
        pts=[]; uvs=[]
        for corner in item["corners"]:
            p=project(vertices[corner["source_index"]])
            if p is None:
                return
            pts.append(p); uvs.append((corner["uv"][0]+du,corner["uv"][1]+dv))
        rendered_counts["inner"] += 1
        for ids in ((0,1,2),(0,2,3)):
            triangle([pts[j] for j in ids],[uvs[j] for j in ids],texture,depth,image,width,height,desc[1],depth_test=not args.disable_cell_depth_test)

    for group,route in selected:
        if route=="inner":
            for row in [r for (g,_),r in entries.items() if g==group]:
                draw_inline_inner(row)
        elif route=="center":
            # A group can have at most one captured cell entry per entry slot;
            # center route selects each entry's +0/+4 child arrays separately.
            for row in [r for (g,_),r in entries.items() if g==group]:
                for which in ("mesh0","mesh4"):
                    item=row["mesh_routes"][which]
                    pred=(lambda value: value<center_param) if which=="mesh0" else (lambda value: value>=center_param)
                    if item["count"]:
                        thresholds[which].append({"group":group,"entry":row["entry"],"kept":sum(pred(v) for v in item["child_values_i16"]),"child_count":item["count"]})
                        draw_route(row,which,pred)
        else:
            for row in [r for (g,_),r in entries.items() if g==group]:
                draw_route(row,route,lambda value: True)

    output_dir=args.output_dir.resolve()
    if (ROOT/"build/tmp").resolve() not in output_dir.parents:
        raise ValueError("candidate output must remain under ignored build/tmp")
    output_dir.mkdir(parents=True,exist_ok=True)
    out=output_dir/f"cell-candidate-{args.case}-{args.label}.png"
    image.save(out)
    report={"case":args.case,"output":str(out),"output_sha256":hashlib.sha256(out.read_bytes()).hexdigest(),
            "camera_params_sha256":hashlib.sha256(params_path.read_bytes()).hexdigest(),
            "selector":selector,"extent_world_408":extent,"inner_coefficient":args.inner_coefficient,"center_filter_delta":args.center_filter_delta,"bounds_43a4ec":[a for a in bounds],
            "selected_routes":[{"group":g,"route":r} for g,r in selected],"thresholds":thresholds,
            "rendered_quads":rendered_counts,"source_pool_sha256":POOL_SHA,
            "frozen_input_manifest_sha256":input_manifest_sha256,
            "active_inline_inner_quads":sum(1 for row in entries.values() if row.get("inline_inner") is not None),
            "selected_inner_bank_indices":sorted({row["inline_inner"]["material_index"] for row in entries.values() if row.get("inline_inner") is not None}),
            "material_table_sha256":hashlib.sha256((args.capture_dir.resolve()/capture["materials"]["file"]).read_bytes()).hexdigest(),
            "uv_rule":"captured two binary64 UV values per corner + descriptor offset pair; offsets are (0,0) in all 48 captured descriptors",
            "texture_rule":"descriptor registration order -> 8x3 tile grid; B8 material +24 selects the separately decoded INN bank; HI images; bilinear repeat address candidate",
            "depth_rule":"software reciprocal-depth z buffer jointly against retained outer skin and cell triangles",
            "outer_skin_included":not args.no_outer_skin,
            "cell_depth_rejection_enabled":not args.disable_cell_depth_test,
            "depth_painter_control":"cell triangle reciprocal-depth rejection disabled; outer underlay retains its established clamped raster and depth test" if args.disable_cell_depth_test else "candidate software reciprocal-depth rejection",
            "blend_selector_support":"all selected routes require selector 0; any other selector errors instead of guessing",
            "scope":"captured runtime-derived geometry; clipped to positive view depth; D3D near/far, draw ordering, and sampler address mode remain unsettled"}
    rep=output_dir/f"cell-candidate-{args.case}-{args.label}.json"
    rep.write_text(json.dumps(report,indent=2),encoding="utf-8")
    print(json.dumps(report,indent=2))


def render_outer(image, depth, params, matrix, fx, fy, texture_dir):
    # Match render_camera_skin's source-LVL ring/texture construction while
    # sharing its z buffer with the captured cell meshes.
    level=ROOT/"build/tmp/a1-camera/assets/COMPILED/MM_CLIMB_002.LVL"
    b=level.read_bytes(); sig=struct.pack("<IdI",112,10.0,7)
    assert b.count(sig)==1
    start=b.index(sig)+len(sig)
    vertices=[struct.unpack_from("<3d",b,start+24*i) for i in range(7*32*2)]
    textures={}
    for col in range(8):
        for row in range(3):
            textures[col,row]=Image.open(texture_dir/f"tile-{col}-{row}HI.bmp").convert("RGB")
    # Rasterizer closure used for the established six-ring cylindrical skin.
    width,height=params["viewport"];cx,cy=params["center"]
    def project(v):
        x,y,z=v
        px=matrix[0]*x+matrix[3]*y+matrix[6]*z+matrix[9]
        py=matrix[1]*x+matrix[4]*y+matrix[7]*z+matrix[10]
        pz=matrix[2]*x+matrix[5]*y+matrix[8]*z+matrix[11]
        if pz<=0:return None
        return cx+fx*px/pz,cy-fy*py/pz,1/pz
    for band in range(6):
        for step in range(32):
            nxt=(step+1)%32
            raw=[vertices[(r*32+s)*2+1] for r,s in ((band,step),(band,nxt),(band+1,nxt),(band+1,step))]
            pts=[project(v) for v in raw]
            if any(p is None for p in pts): continue
            u0=(step%4)/4;u1=u0+.25;v0=1-(band%2)/2;v1=v0-.5
            uv=[(u0,v0),(u1,v0),(u1,v1),(u0,v1)]
            tex=textures[step//4,2-band//2]
            for ids in ((0,1,2),(0,2,3)):
                triangle([pts[j] for j in ids],[uv[j] for j in ids],tex,depth,image,width,height,0,"clamp")


if __name__=="__main__":
    main()
