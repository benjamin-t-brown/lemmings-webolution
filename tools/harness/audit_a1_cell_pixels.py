#!/usr/bin/env python3
"""Recompute the fixed A1 ROI for the retained cell-fixture candidates."""
from __future__ import annotations

import hashlib
import json
from pathlib import Path

from PIL import Image, ImageChops

ROOT=Path(__file__).resolve().parents[2]
FIX=ROOT/"tools/harness/fixtures/camera_a1"
TMP=ROOT/"build/tmp/a1-cell-fixture"
OUTDIR=TMP/"candidates"
OUT=TMP/"pixel-report.json"
MUT=TMP/"mutation-ledger.json"
REFS={"initial":"original-0.png","mouse":"original-rotation-mouse-0.png",
      "right":"original-rotation-key-right-0.png","left":"original-rotation-key-left-0.png"}
ROI=(100,80,280,410)
DEN=255*3*(ROI[2]-ROI[0])*(ROI[3]-ROI[1])


def compare(case,candidate):
    ref=FIX/REFS[case]
    a=Image.open(ref).convert("RGB")
    b=Image.open(candidate).convert("RGB")
    assert a.size==b.size==(640,480)
    hist=ImageChops.difference(a.crop(ROI),b.crop(ROI)).histogram()
    total=sum((i%256)*n for i,n in enumerate(hist))
    return {"case":case,"original_sha256":hashlib.sha256(ref.read_bytes()).hexdigest(),
            "candidate_sha256":hashlib.sha256(candidate.read_bytes()).hexdigest(),
            "candidate_file":str(candidate.relative_to(ROOT)),"rgb_absolute_sum":total,
            "normalized_error":total/DEN,"passes_threshold":total/DEN<=.02}


def measured_candidate(case,label):
    stem=OUTDIR/f"cell-candidate-{case}-{label}"
    image=stem.with_suffix(".png")
    metadata=json.loads(stem.with_suffix(".json").read_text())
    metric=compare(case,image)
    metric.update({"candidate_metadata_sha256":hashlib.sha256(stem.with_suffix(".json").read_bytes()).hexdigest(),
                   "selector":metadata["selector"],"bounds":metadata["bounds_43a4ec"],
                   "drawn_quads":metadata["rendered_quads"],
                   "outer_skin_included":metadata["outer_skin_included"]})
    return metric,metadata


def main():
    incomplete=[]
    inline=[]
    cell_only=[]
    painter=[]
    input_manifest=TMP/"input-manifest-v2.json"
    input_manifest_hash=hashlib.sha256(input_manifest.read_bytes()).hexdigest()
    for case in ("initial","mouse","right","left"):
        direct_metric,_=measured_candidate(case,"pred")
        incomplete.append(direct_metric)
        inline_metric,inline_report=measured_candidate(case,"complete-v2")
        if inline_report["frozen_input_manifest_sha256"]!=input_manifest_hash:
            raise ValueError("complete candidate does not reference the frozen input manifest")
        inline_metric["active_inline_inner_quads"]=inline_report["active_inline_inner_quads"]
        inline.append(inline_metric)
        control,control_report=measured_candidate(case,"cell-only-v2")
        control["selected_routes"]=control_report["selected_routes"]
        cell_only.append(control)
        painter_metric,painter_report=measured_candidate(case,"painter-v2")
        if painter_report["frozen_input_manifest_sha256"]!=input_manifest_hash:
            raise ValueError("painter diagnostic does not reference the frozen input manifest")
        painter.append(painter_metric)

    mutation_rows=[]
    for name,label in (("inner_coefficient_0.7_to_0.6","mutation-inner06-v2"),
                       ("center_filter_delta_0_to_plus1","center-plus1-v2")):
        metric,metadata=measured_candidate("left",label)
        if metadata["frozen_input_manifest_sha256"]!=input_manifest_hash:
            raise ValueError("mutation does not reference the frozen input manifest")
        default=next(row for row in inline if row["case"]=="left")
        mutation_rows.append({"mutation":name,"mutated_candidate_sha256":metric["candidate_sha256"],
            "default_candidate_sha256":default["candidate_sha256"],
            "same_pixels_as_default":metric["candidate_sha256"]==default["candidate_sha256"],
            "mutated_error":metric["normalized_error"],"default_error":default["normalized_error"],
            "strictly_worsened_original_error":metric["normalized_error"]>default["normalized_error"],
            "mutated_bounds":metadata["bounds_43a4ec"],"mutated_drawn_quads":metadata["rendered_quads"],
            "protected_by_visible_pixels":False,
            "restoration":"default complete-v2 left was rerendered after each mutation; SHA-256 is recorded above"})
    report={"resolution":[640,480],"roi_xyxy":list(ROI),"metric":"sum(abs(originalRGB-candidateRGB))/(255*3*180*330)",
            "threshold":.02,"frozen_input_manifest":"input-manifest-v2.json","frozen_input_manifest_sha256":input_manifest_hash,
            "stale_78_quad_candidate_preserved_superseded":incomplete,
            "complete_98_active_inline_quad_candidates":inline,
            "no_surrogate_98_quad_controls":cell_only,
            "painter_order_depth_rejection_diagnostic":painter,
            "outcome":"The complete frozen-input candidate includes all 98 active +8 inline quads plus direct meshes. The correct-depth candidate leaves all fixed ROI sums equal to outer-only; the left view remains above 0.02. The one-variable painter-order control changes the ROI but worsens all four errors. The no-surrogate control is diagnostic only. No cell-fixture all-view acceptance.",
            "scope":"captured-geometry raster candidate; D3D clipping/order/blend/address semantics remain unsettled"}
    OUT.write_text(json.dumps(report,indent=2),encoding="utf-8")
    MUT.write_text(json.dumps({"status":"selection mutations are unprotected; complete-v2 mutation outcomes measured against immutable original ROI",
        "mutations":mutation_rows,"source_files_mutated":[],
        "baseline_restoration":"left default complete-v2 PNG rerendered after each mutation; SHA-256 is recorded above",
        "painter_order_control":"Separate predeclared diagnostic; retains the same clamped outer underlay and complete geometry, changes only cell triangle depth rejection.",
        "limitations":["Selection mutations changed bounds or child counts but did not improve fixed ROI error.",
                       "The software painter-order output worsens every fixed view and does not verify stock device depth/order state."]},indent=2),encoding="utf-8")
    print(json.dumps({"pixel_report":str(OUT),"mutation_ledger":str(MUT),"cases":inline,"painter":painter,"mutations":mutation_rows},indent=2))


if __name__=="__main__":main()
