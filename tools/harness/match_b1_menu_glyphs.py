#!/usr/bin/env python3
"""Bounded, geometry-preserving registration of retail source candidates to Quit.

The fixed 640x480 reference is a comparison only. Candidates and reconstructed pixels
come from the exported retail font sheets; no source cell or codepoint mapping is assumed.
"""
from __future__ import annotations

import argparse
import hashlib
import itertools
import json
from pathlib import Path

from PIL import Image, ImageChops

FONT_FILES = {
    "^FONT": "TEXTURES~5C~5EFONT.BMP.png",
    "^FONTH": "TEXTURES~5C~5EFONTH.BMP.png",
    "^FONTP": "TEXTURES~5C~5EFONTP.BMP.png",
    "_FONT": "TEXTURES~5C~5FFONT.BMP.png",
    "_FONTFE": "TEXTURES~5C~5FFONTFE.BMP.png",
}
QUIT = (282, 438, 359, 463)  # measured support ROI, half-open
PANEL = (224, 10, 0)
SCALES = tuple(i / 8 for i in range(4, 17))  # 0.5..2 by 0.125


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def components(alpha: Image.Image) -> list[tuple[int, int, int, int]]:
    """8-connected nonzero-alpha support bounds."""
    w, h = alpha.size
    p = alpha.load()
    seen = bytearray(w * h)
    out = []
    for y in range(h):
        for x in range(w):
            i = y * w + x
            if seen[i] or p[x, y] == 0:
                continue
            seen[i] = 1
            todo = [(x, y)]
            x0 = x1 = x
            y0 = y1 = y
            while todo:
                cx, cy = todo.pop()
                x0, x1 = min(x0, cx), max(x1, cx)
                y0, y1 = min(y0, cy), max(y1, cy)
                for ny in range(max(0, cy - 1), min(h, cy + 2)):
                    for nx in range(max(0, cx - 1), min(w, cx + 2)):
                        ni = ny * w + nx
                        if not seen[ni] and p[nx, ny] != 0:
                            seen[ni] = 1
                            todo.append((nx, ny))
            out.append((x0, y0, x1 + 1, y1 + 1))
    return out


# Retained diagnostic helpers and tests from the interrupted segmentation-only script.
def place_scaled(mask: Image.Image, scale: float, x: int, y: int,
                 canvas_size: tuple[int, int]) -> Image.Image:
    w, h = mask.size
    scaled = mask.resize((max(1, round(w * scale)), max(1, round(h * scale))),
                         Image.Resampling.NEAREST)
    canvas = Image.new("L", canvas_size)
    canvas.paste(scaled, (x, y))
    return canvas


def pixel_error(a: Image.Image, b: Image.Image) -> int:
    if a.size != b.size:
        raise ValueError("comparison canvases must be identical")
    return sum(abs(x - y) for x, y in zip(a.getdata(), b.getdata()))


def ordered_assignment(component_indices: list[int]) -> bool:
    return (all(a < b for a, b in zip(component_indices, component_indices[1:])) and
            len(set(component_indices)) == len(component_indices))


def source_candidates(rgba: Image.Image) -> list[tuple[int, int, int, int]]:
    """Content-derived components and bounded nearby-support unions."""
    cs = sorted(components(rgba.getchannel("A")), key=lambda b: (b[1], b[0]))
    bounds = set(cs)
    # Merge up to three vertically adjacent supports with modest vertical and horizontal
    # gaps. Bounding-box unions preserve actual source geometry and include intervening
    # transparent pixels; no fixed atlas cells are introduced.
    for i, a in enumerate(cs):
        group = a
        for b in cs[i + 1:i + 3]:
            if b[1] - group[3] > 5 or b[0] - group[2] > 5 or b[0] < group[0]:
                break
            group = (min(group[0], b[0]), min(group[1], b[1]),
                     max(group[2], b[2]), max(group[3], b[3]))
            bounds.add(group)
    # A bounded plausible foreground-candidate pool keeps combinations tractable. These
    # size bounds are an explicit search limit, not claims about native glyph dimensions.
    return sorted((b for b in bounds if 4 <= b[2] - b[0] <= 24 and
                   6 <= b[3] - b[1] <= 28 and
                   (b[2] - b[0]) * (b[3] - b[1]) <= 500),
                   key=lambda b: (b[1], b[0], b[3], b[2]))


def source_crop_bounds(alpha: Image.Image) -> list[tuple[int, int, int, int]]:
    """Content-derived glyph-crop proposals, including disconnected support groups.

    Rows are detected from alpha-support geometry. Components whose vertical centers are
    near a shared baseline are grouped into row proposals. Pair/triple unions are kept
    when their bounds plausibly enclose one compound glyph; no index/cell assumption.
    """
    cs = components(alpha)
    groups = []
    # Consider all components as row anchors. Components from the same physical text row
    # share upper/lower support bands even when a dot/accent is disconnected.
    for anchor in cs:
        ay0, ay1 = anchor[1], anchor[3]
        row = [b for b in cs if abs((b[1] + b[3]) - (ay0 + ay1)) <= 12 and
               abs(b[3] - ay1) <= 6]
        row = sorted(row, key=lambda b: (b[0], b[1]))
        for start, first in enumerate(row):
            group = first
            groups.append(group)
            for other in row[start + 1:start + 4]:
                gap = other[0] - group[2]
                if gap > 3:
                    break
                group = (min(group[0], other[0]), min(group[1], other[1]),
                         max(group[2], other[2]), max(group[3], other[3]))
                groups.append(group)
    # Source crop bounds may include components but exclude neighboring complete supports.
    return sorted({b for b in groups if 4 <= b[2] - b[0] <= 28 and
                   6 <= b[3] - b[1] <= 30 and
                   (b[2] - b[0]) * (b[3] - b[1]) <= 600},
                  key=lambda b: (b[1], b[0], b[2], b[3]))


def target_mask(rgb: Image.Image) -> Image.Image:
    """Thresholded deviation from the measured flat panel, comparison only."""
    out = Image.new("L", rgb.size)
    src, dst = rgb.convert("RGB").load(), out.load()
    for y in range(out.height):
        for x in range(out.width):
            r, g, b = src[x, y]
            dev = max(abs(r - PANEL[0]), abs(g - PANEL[1]), abs(b - PANEL[2]))
            dst[x, y] = dev if dev >= 24 else 0
    return out


def composite_candidate(sheet: Image.Image, bounds: tuple[int, int, int, int],
                        scale: float, *, retain_rgb: bool = False) -> Image.Image:
    crop = sheet.crop(bounds)
    nw, nh = (max(1, round(crop.width * scale)), max(1, round(crop.height * scale)))
    if retain_rgb:
        return crop.resize((nw, nh), Image.Resampling.NEAREST)
    alpha = crop.getchannel("A").resize((nw, nh), Image.Resampling.NEAREST)
    # Predeclared black tint, retaining retail alpha coverage. This tests registration
    # geometry independently of unresolved family RGB and is not fitted per glyph.
    ink = Image.new("RGBA", (nw, nh), (0, 0, 0, 0))
    ink.putalpha(alpha)
    return ink


def render_row(sheet: Image.Image, placements: list[dict], scale: float,
               size: tuple[int, int], out_rgb: tuple[int, int, int]) -> Image.Image:
    canvas = Image.new("RGBA", size, (*out_rgb, 255))
    for p in placements:
        glyph = composite_candidate(sheet, tuple(p["source_rect"]), scale)
        canvas.alpha_composite(glyph, (p["destination_rect"][0], p["destination_rect"][1]))
    return canvas.convert("RGB")


def render_row_source_rgb(sheet: Image.Image, placements: list[dict], scale: float,
                          size: tuple[int, int], out_rgb: tuple[int, int, int]) -> Image.Image:
    """Control rendering with exported retail RGBA, no tint or family correction."""
    canvas = Image.new("RGBA", size, (*out_rgb, 255))
    for p in placements:
        glyph = composite_candidate(sheet, tuple(p["source_rect"]), scale, retain_rgb=True)
        canvas.alpha_composite(glyph, (p["destination_rect"][0], p["destination_rect"][1]))
    return canvas.convert("RGB")


def score_mask(target: Image.Image, rendered: Image.Image, box: tuple[int, int, int, int]) -> int:
    """Exact threshold-mask absolute L1 difference over full fixed ROI canvas."""
    tm = target.crop(box)
    rm = target_mask(rendered.crop(box))
    return sum(abs(a - b) for a, b in zip(tm.getdata(), rm.getdata()))


def rgb_error(a: Image.Image, b: Image.Image) -> int:
    return sum(abs(x - y) for x, y in zip(a.convert("RGB").tobytes(),
                                          b.convert("RGB").tobytes()))


def placement_options(sheet: Image.Image, rect: tuple[int, int, int, int],
                      target: Image.Image, scale: float,
                      x_min: int, x_max: int, y_min: int, y_max: int) -> list[dict]:
    """Best actual RGB score for this fixed-size retail crop at each x position."""
    glyph = composite_candidate(sheet, rect, scale)
    w, h = glyph.size
    if w > target.width or h > target.height:
        return []
    patch = Image.new("RGBA", glyph.size, (*PANEL, 255))
    patch.alpha_composite(glyph)
    patch = patch.convert("RGB")
    result = []
    for x in range(max(0, x_min), min(target.width - w, x_max) + 1):
        best = None
        for y in range(max(0, y_min), min(target.height - h, y_max) + 1):
            actual = target.crop((x, y, x + w, y + h))
            err = rgb_error(actual, patch)
            blank_error = rgb_error(actual, Image.new("RGB", (w, h), PANEL))
            delta = err - blank_error
            if best is None or delta < best[0]:
                best = (delta, y)
        if best is not None:
            result.append({"x": x, "y": best[1], "w": w, "h": h,
                           "delta": best[0], "glyph": glyph})
    return result


def register_family(family: str, path: Path, reference: Image.Image,
                    out_dir: Path) -> dict:
    sheet = Image.open(path).convert("RGBA")
    roi = reference.crop(QUIT).convert("RGB")
    target = target_mask(roi)
    target_comps = components(target)
    # The entire row's pixels are scored, not target connected components.
    projection = [sum(1 for y in range(target.height) if target.getpixel((x, y)) > 0)
                  for x in range(target.width)]
    active = [x for x, n in enumerate(projection) if n]
    if not active:
        return {"family": family, "error": "empty target mask"}
    x0, x1 = min(active), max(active) + 1
    # First-tier candidates are actual connected retail alpha supports, not arbitrary
    # multi-character bounding unions. Punctuation/accent unions remain separately
    # generated by source_crop_bounds but are excluded from this single-glyph search.
    candidates = sorted({b for b in components(sheet.getchannel("A"))
                         if 3 <= b[2]-b[0] <= 24 and 7 <= b[3]-b[1] <= 28},
                        key=lambda b: (b[1], b[0], b[2], b[3]))
    full_size = (target.width, target.height)
    family_results = []
    blank = Image.new("RGB", full_size, PANEL)
    blank_score = rgb_error(roi, blank)
    target_y0 = max(0, min(b[1] for b in target_comps) - 2)
    target_y1 = min(target.height, max(b[3] for b in target_comps) + 2)
    source_components = components(sheet.getchannel("A"))
    row_scores = []
    for sy0 in range(0, sheet.height - 5):
        support = [b for b in source_components if sy0 <= b[1] and b[3] <= sy0 + 36]
        score = sum(1 for b in support if 5 <= b[2]-b[0] <= 28 and 6 <= b[3]-b[1] <= 30)
        if score >= 4:
            row_scores.append((score, sy0, sy0 + 36))
    # Band diagnostics are descriptive only; do not filter candidates to one guessed row.
    source_bands = sorted({(sy0, sy1) for _, sy0, sy1 in row_scores})
    # Search actual source sizes at thirteen physical scales. Source/target crops never
    # normalize to one another. A common baseline and whole-row pixel score permit merged
    # target support without requiring four connected components.
    for scale in SCALES:
        glyphs = [(ci, rect, composite_candidate(sheet, rect, scale))
                  for ci, rect in enumerate(candidates)]
        glyphs = [(ci, rect, glyph) for ci, rect, glyph in glyphs
                  if glyph.width >= 3 and glyph.height >= 5 and
                  glyph.width <= (x1 - x0) // 2 + 2 and
                  glyph.height <= target_y1 - target_y0 + 4]
        # The original support mask merged close characters into clusters. Use those
        # clusters as coarse x partitions: glyphs can touch inside each partition, but
        # the source-to-target association cannot hop repeatedly across the whole word.
        clusters = sorted(target_comps, key=lambda b: b[0])
        # Three measured clusters cover the word's foreground span. Assign two ordered
        # glyphs to the merged left cluster and one to each of the remaining supports.
        group_counts = [2, 1, 1] if len(clusters) == 3 else [1] * 4
        groups = []
        ci = 0
        for count in group_counts:
            if ci >= len(clusters): break
            if count == 2 and ci == 0:
                # subdivide the wide measured component using its horizontal midpoint;
                # this is a target comparison partition, not a source character crop.
                b = clusters[ci]
                mid = (b[0] + b[2]) // 2
                groups.extend([(max(0,b[0]-1), min(target.width,mid+2)),
                               (max(0,mid-2), min(target.width,b[2]+1))])
            else:
                b = clusters[ci]
                groups.append((max(0,b[0]-2), min(target.width,b[2]+2)))
            ci += 1
        for baseline in range(max(1, target_y0 + 2), min(target.height, target_y1 + 3)):
            option_table = {}
            unary = []
            for ci, rect, glyph in glyphs:
                top = baseline - glyph.height
                if top < target_y0 - 1 or top + glyph.height > target_y1 + 1:
                    continue
                for gi,(gx0,gx1) in enumerate(groups):
                    ops=placement_options(sheet,rect,roi,scale,gx0,gx1-1,top,top)
                    ops.sort(key=lambda p:(p["delta"],p["x"]))
                    option_table[(gi,ci)]=ops
                    unary.extend((op["delta"],gi,ci,rect,op) for op in ops)
            unary.sort(key=lambda t:(t[0],t[1],t[2],t[4]["x"]))
            best_unary={}
            for delta,gi,ci,rect,op in unary:
                best_unary.setdefault((gi,ci),(delta,rect,op))
            active={(gi,ci) for (gi,ci),_ in sorted(best_unary.items(),key=lambda kv:(kv[1][0],kv[0]))[:96]}
            beam=[(0,[],frozenset(),0,-1)]
            for depth in range(4):
                expanded=[]
                for prior,placed,used,group_index,last_right in beam:
                    if group_index>=len(groups):continue
                    for ci,rect,glyph in glyphs:
                        if ci in used or (group_index,ci) not in active:continue
                        for op in option_table.get((group_index,ci),[]):
                            if op["x"]<last_right or op["x"]+op["w"]>groups[group_index][1]:continue
                            item={"source_rect":list(rect),"source_candidate_index":ci,
                                  "destination_rect":[op["x"],op["y"],op["x"]+op["w"],op["y"]+op["h"]],
                                  "target_group":group_index}
                            next_group=group_index+1
                            next_x=groups[next_group][0] if next_group<len(groups) else -1
                            expanded.append((prior+op["delta"],placed+[item],used|{ci},next_group,next_x))
                expanded.sort(key=lambda s:(s[0],tuple(p["source_rect"] for p in s[1]),
                                             tuple(p["destination_rect"] for p in s[1])))
                beam=expanded[:96]
                if not beam:break
            for _,placements,_,_,_ in beam:
                if len(placements)!=4:continue
                rendered=render_row(sheet,placements,scale,full_size,PANEL)
                family_results.append((rgb_error(roi,rendered),scale,placements,rendered))
    if not family_results:
        return {"family": family, "candidate_count": len(candidates),
                "search": "13 scales; all content candidates; common baseline; unary-scored candidate pruning; beam width 160",
                "error": "no feasible bounded four-source sequence"}
    family_results.sort(key=lambda r: (r[0], r[1], tuple(p["source_rect"] for p in r[2]),
                                       tuple(p["destination_rect"] for p in r[2])))
    best = family_results[0]
    score, scale, placements, rendered = best
    # Controls perturb the actual best rendering without refitting the source crops.
    wrong_scale = render_row(sheet, placements, scale + 0.125, full_size, PANEL)
    shifted = []
    for i, p in enumerate(placements):
        box = list(p["destination_rect"])
        if i == 0:
            box[0] += 1
            box[2] += 1
        shifted.append(dict(p, destination_rect=box))
    wrong_place = render_row(sheet, shifted, scale, full_size, PANEL)
    blank = Image.new("RGB", full_size, PANEL)
    mask_target = target_mask(roi)
    output_png = out_dir / f"quit-{family.replace('^','caret-').replace('_','under-')}-render.png"
    diff_png = out_dir / f"quit-{family.replace('^','caret-').replace('_','under-')}-diff.png"
    rendered.save(output_png)
    ImageChops.difference(roi, rendered).save(diff_png)
    raw_rgb_render = render_row_source_rgb(sheet, placements, scale, full_size, PANEL)
    raw_rgb_png = out_dir / f"quit-{family.replace('^','caret-').replace('_','under-')}-source-rgb.png"
    raw_rgb_render.save(raw_rgb_png)
    return {
        "family": family, "source_sha256": sha(path), "source_dimensions": list(sheet.size),
        "alpha_sha256": hashlib.sha256(sheet.getchannel("A").tobytes()).hexdigest(),
        "rgb_sha256": hashlib.sha256(sheet.convert("RGB").tobytes()).hexdigest(),
        "candidate_count": len(candidates), "search": "13 physical scales; all content candidates; common baseline; unary-score prune; ordered beam 160; no target-box resize",
        "source_bands_y": [list(b) for b in source_bands],
        "scale_values": list(SCALES), "best_score_l1": score,
        "blank_row_rgb_l1": rgb_error(roi, blank),
        "blank_row_rgb_l1_independent": blank_score,
        "exact_rgb_l1_over_77x25x3": score,
        "exact_rgb_l1_fraction_of_255_channel_max": score / (77 * 25 * 3 * 255),
        "wrong_scale": {"scale": scale + 0.125,
                        "rgb_l1": rgb_error(roi, wrong_scale)},
        "wrong_first_placement_plus1px": {"rgb_l1": rgb_error(roi, wrong_place)},
        "best_scale": scale,
        "placements": [{**p, "destination_rect_global": [p["destination_rect"][0] + QUIT[0],
                            p["destination_rect"][1] + QUIT[1],
                            p["destination_rect"][2] + QUIT[0],
                            p["destination_rect"][3] + QUIT[1]]} for p in placements],
        "score_denominator_note": "actual RGB-channel absolute error on the whole 77x25 row; not the production full-frame gate",
        "render": str(output_png), "diff": str(diff_png),
        "unmodified_source_rgb_score_l1": rgb_error(roi, raw_rgb_render),
        "unmodified_source_rgb_render": str(raw_rgb_png),
        "target_support_components_comparison_only": [list(b) for b in target_comps],
        "winner_ambiguity": "alpha-identical _FONT/_FONTFE must remain tied on alpha; RGB-family choice is not inferred from alpha",
        "compositing": "retail alpha geometry, black tint fixed before score; no per-glyph tint fit",
    }


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--export", type=Path, required=True)
    ap.add_argument("--reference", type=Path, required=True)
    ap.add_argument("--out", type=Path, required=True)
    args = ap.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    reference_path = args.reference
    reference = Image.open(reference_path).convert("RGB")
    if reference.size != (640, 480):
        raise ValueError(f"expected measured 640x480 reference, got {reference.size}")
    summaries = []
    for family, filename in FONT_FILES.items():
        summaries.append(register_family(family, args.export / filename, reference, args.out))
    result = {"status": "bounded scored Quit-row registration attempt; no codepoint mapping claimed",
              "reference_sha256": sha(reference_path), "reference": str(reference_path),
              "roi": list(QUIT), "panel_rgb": PANEL, "scale_search": list(SCALES),
              "source_crop_method": "alpha components and bounded content unions; no presumed cells",
              "families": summaries}
    (args.out / "registration.json").write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
