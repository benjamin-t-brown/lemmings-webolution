#!/usr/bin/env python3
"""Offline, deterministic B1 image measurement. No game/runtime access."""
import argparse
import hashlib
import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageChops

DENOMINATOR = 640 * 480 * 3 * 255
LABELS = [(125, "Start New Game"), (126, "Play Current Game"),
          (120, "Language Selection"), (3, "Options"), (43, "Quit")]


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def rgb(im):
    return im.convert("RGB")


def locate(reference, template):
    """Exhaustive declared integer scale 1..3, translation; full-template RGB MAE."""
    ref = rgb(reference)
    w, h = reference.size
    outcomes = []
    for scale in range(1, 4):
        t = template.resize((template.width * scale, template.height * scale), Image.Resampling.NEAREST)
        if t.width > w or t.height > h:
            continue
        ta = rgb(t)
        # Candidate origins are all in-frame. Full candidates retained for exact reproducibility.
        for y in range(h - t.height + 1):
            for x in range(w - t.width + 1):
                diff = ImageChops.difference(ref.crop((x, y, x+t.width, y+t.height)), ta)
                # Sum channel histograms for exact integer RGB absolute-error numerator.
                diff_sum = sum(sum(i*n for i,n in enumerate(diff.getchannel(c).histogram())) for c in range(3))
                outcomes.append((int(diff_sum), x, y, scale, t.width, t.height))
    outcomes.sort()
    return outcomes[:2]


def panel_runs(im):
    """Image-derived red/orange connected row bands; tolerate glyph holes and thin rows."""
    a = im.convert("RGB")
    # Explicit image-derived predicate: red/orange menu bars against black backdrop.
    pix = a.load()
    mask = [[(pix[x,y][0] > 130 and pix[x,y][1] < 100 and pix[x,y][2] < 80) or
             (pix[x,y][0] > 180 and 90 < pix[x,y][1] < 200 and pix[x,y][2] < 60)
             for x in range(a.width)] for y in range(a.height)]
    rows = []
    for y in range(a.height):
        xs = [x for x, yes in enumerate(mask[y]) if yes]
        # A handful of edge pixels is noise; unlike the old >=100 test this
        # preserves narrow rows and rows partially obscured by dark lettering.
        if len(xs) < 3:
            continue
        rows.append((y, xs[0], xs[-1]+1))
    bands = []
    for y, x0, x1 in rows:
        # Per-row projections may disappear at glyph-shaped holes. Join only
        # nearby rows whose x projections overlap; this is scale-independent.
        if bands and y - bands[-1]["y1"] <= 3 and x0 < bands[-1]["x1"] and x1 > bands[-1]["x0"]:
            bands[-1]["y1"] = y + 1
            bands[-1]["x0"] = min(bands[-1]["x0"], x0)
            bands[-1]["x1"] = max(bands[-1]["x1"], x1)
        else:
            bands.append(dict(x0=x0, y0=y, x1=x1, y1=y + 1))
    # Button-like candidates are broad, shallow horizontal regions in the
    # lower half; this removes similarly coloured scenery using geometry, not
    # fixed menu coordinates or a per-row pixel-count cutoff.
    return [b for b in bands if b["y0"] >= a.height//2 and
            b["x1"]-b["x0"] >= 3*(b["y1"]-b["y0"])]


def alpha_components(image):
    """Return connected atlas supports (4-neighbour alpha>0), retaining source boxes."""
    a = image.convert("RGBA").getchannel("A")
    w, h = a.size
    px = a.load(); seen = set(); result = []
    for y in range(h):
        for x in range(w):
            if px[x, y] == 0 or (x, y) in seen: continue
            stack = [(x, y)]; seen.add((x, y)); points=[]
            while stack:
                qx, qy = stack.pop(); points.append((qx,qy))
                for n in ((qx-1,qy),(qx+1,qy),(qx,qy-1),(qx,qy+1)):
                    nx,ny=n
                    if 0 <= nx < w and 0 <= ny < h and n not in seen and px[nx,ny] > 0:
                        seen.add(n); stack.append(n)
            # Keep disconnected speckles out of the glyph candidate pool.
            if len(points) >= 3:
                result.append((min(p[0] for p in points), min(p[1] for p in points),
                               max(p[0] for p in points)+1, max(p[1] for p in points)+1, len(points)))
    return sorted(result, key=lambda r:(r[1],r[0]))


def row_tint(reference, rect):
    """Infer foreground tint from the darkest non-panel pixels inside a row."""
    x0,y0,x1,y1=rect; im=reference.convert("RGB"); vals=[]
    for y in range(y0,y1):
        for x in range(x0,x1):
            r,g,b=im.getpixel((x,y))
            if max(r,g,b)<100: vals.append((r,g,b))
    return tuple(sorted(v[i] for v in vals)[len(vals)//2] for i in range(3)) if vals else (0,0,0)


def binary_components(mask):
    """Connected target supports, as (bbox, pixels); target is comparison-only."""
    w,h=mask.size; p=mask.load(); seen=set(); out=[]
    for y in range(h):
        for x in range(w):
            if p[x,y] == 0 or (x,y) in seen: continue
            todo=[(x,y)]; seen.add((x,y)); pts=[]
            while todo:
                q=todo.pop(); pts.append(q)
                for n in ((q[0]-1,q[1]),(q[0]+1,q[1]),(q[0],q[1]-1),(q[0],q[1]+1)):
                    if 0<=n[0]<w and 0<=n[1]<h and p[n[0],n[1]] and n not in seen:
                        seen.add(n); todo.append(n)
            if len(pts)>=2:
                box=(min(x for x,y in pts),min(y for x,y in pts),max(x for x,y in pts)+1,max(y for x,y in pts)+1)
                out.append((box,set(pts)))
    return sorted(out,key=lambda z:(z[0][1],z[0][0]))


def glyph_search(sheet, row, word, rect, cell=16, scales=(1,2,3)):
    """Bounded, per-letter alpha-shape search. Grid is explicit row-major ASCII32 hypothesis."""
    from PIL import ImageChops
    x0,y0,x1,y1=rect; ref=row.convert('RGB'); bg=ref.getpixel((0,0))
    # For these orange-backed button rows, foreground is dark; classify support, not RGB equality.
    target=Image.new('L',(x1-x0,y1-y0)); tp=target.load()
    for y in range(y0,y1):
        for x in range(x0,x1):
            r,g,b=ref.getpixel((x,y)); tp[x-x0,y-y0]=255 if max(r,g,b)<100 else 0
    comps=binary_components(target)
    atlas=sheet.convert('RGBA').getchannel('A'); results=[]
    # Falsifier geometry: 16x16 cells, row-major printable ASCII beginning at space.
    for char in sorted(set(word.replace(' ',''))):
        code=ord(char)-32; cx=(code%16)*cell; cy=(code//16)*cell
        if cy+cell>atlas.height: continue
        src=atlas.crop((cx,cy,cx+cell,cy+cell)); box=src.getbbox()
        if not box: continue
        glyph=src.crop(box); best=None
        for scale in scales:
            sw=max(1,round(glyph.width*scale)); sh=max(1,round(glyph.height*scale))
            shape=glyph.resize((sw,sh),Image.Resampling.NEAREST)
            sp={(x,y) for y in range(sh) for x in range(sw) if shape.getpixel((x,y))>0}
            for tb,points in comps:
                tx0,ty0,tx1,ty1=tb
                # Fit each target connected component after translation to its own top-left.
                dx=tx1-tx0; dy=ty1-ty0
                if max(dx,dy)>max(sw,sh)*2 or min(dx,dy)<1: continue
                # symmetric support error after normalized nearest resizing
                norm=Image.new('L',(dx,dy)); norm.paste(target.crop(tb))
                rs=shape.resize((dx,dy),Image.Resampling.NEAREST)
                err=sum(a!=b for a,b in zip(norm.getdata(),rs.getdata()))/max(1,dx*dy)
                item=dict(char=char,score=err,scale=scale,source=[cx+box[0],cy+box[1],cx+box[2],cy+box[3]],destination=[tx0+x0,ty0+y0,tx1+x0,ty1+y0])
                if best is None or (err,scale,tb)<(best['score'],best['scale'],tuple(best['destination'])): best=item
        if best: results.append(best)
    return dict(grid='16x16 row-major ASCII32 (hypothesis)',components=len(comps),matches=results,
                mean_score=sum(m['score'] for m in results)/len(results) if results else None,
                target_mask=target)


def main():
    p = argparse.ArgumentParser()
    p.add_argument("reference", type=Path)
    p.add_argument("manifest", type=Path)
    p.add_argument("output", type=Path)
    a = p.parse_args()
    a.output.mkdir(parents=True, exist_ok=True)
    ref = Image.open(a.reference).convert("RGB")
    if ref.size != (640, 480):
        raise SystemExit(f"reference must be 640x480, got {ref.size}")
    manifest = json.loads(a.manifest.read_text(encoding="utf-8"))
    root = a.manifest.parent
    assets = manifest["assets"]
    back = next(x for x in assets if x["archive_name"].upper().endswith("MAIN_LOGO.BMP"))
    backpath = root / back["output"]
    bg = Image.open(backpath).convert("RGB")
    fits = locate(ref, bg)
    best = fits[0]
    panels = panel_runs(ref)
    # The old 1..3 whole-template integer search is rejected for text: bitmap font
    # atlases are cell sources, not whole-screen templates. First test the explicit
    # 16x16/ASCII32 cell hypothesis at integer 1..3; a fractional sweep follows only
    # if these scores do not explain the target (0.5..2.5, step .25).
    font_assets=[x for x in assets if 'FONT' in x['archive_name'].upper() and 'MAIN_LOGO' not in x['archive_name'].upper()]
    glyph_families=[]
    row_panels=sorted(panels,key=lambda r:r['y0'])
    for asset in font_assets:
        sheet=Image.open(root/asset['output']).convert('RGBA')
        integer=[]
        for (idx,text),panel in zip(LABELS,row_panels):
            result=glyph_search(sheet,ref,text,(panel['x0'],panel['y0'],panel['x1'],panel['y1']),scales=(1,2,3))
            integer.append(dict(index=idx,text=text,score=result['mean_score'],matches=result['matches'],components=result['components']))
        if not any(r['score'] is not None and r['score']<.35 for r in integer):
            for (idx,text),panel,resultrow in zip(LABELS,row_panels,integer):
                result=glyph_search(sheet,ref,text,(panel['x0'],panel['y0'],panel['x1'],panel['y1']),scales=tuple(i/4 for i in range(2,11)))
                resultrow.update(score=result['mean_score'],matches=result['matches'],fractional_search='0.5..2.5 step .25')
        glyph_families.append(dict(family=asset['archive_name'],scale_search='integer 1..3 then fractional .5..2.5/.25 if integer family best >=.35',rows=integer))
    # Diagnostic combines only the matched retail backdrop and measured panel pixels.
    candidate = bg.copy()
    d = ImageDraw.Draw(candidate)
    panel_results = []
    for r in panels:
        # Primitive colour from border pixels only (avoid copying text/screenshot pixels).
        q = ref.crop((r["x0"], r["y0"], r["x1"], r["y1"])).convert("RGB")
        border = [q.getpixel((x, y)) for x in range(q.width) for y in (0, q.height-1)] + [q.getpixel((x,y)) for y in range(q.height) for x in (0,q.width-1)]
        color = tuple(sorted(c[i] for c in border)[len(border)//2] for i in range(3))
        panel_results.append(dict(rect=r, source="measured contiguous saturated-color image runs",
                                  color_rgb=list(color), primitive="flat measured color; bounds from image"))
        d.rectangle((r["x0"], r["y0"], r["x1"]-1, r["y1"]-1), fill=color)
    residual_image = ImageChops.difference(candidate, ref)
    numerator = sum(sum(i*n for i,n in enumerate(residual_image.getchannel(c).histogram())) for c in range(3))
    # annotation conveys measured panel boxes, not a production image.
    annotated = ref.copy()
    ad = ImageDraw.Draw(annotated)
    for r in panels:
        ad.rectangle((r["x0"], r["y0"], r["x1"]-1, r["y1"]-1), outline=(0, 255, 255))
    candidate.save(a.output / "candidate.png")
    residual_image.point(lambda x: min(x*4,255)).save(a.output / "residual_x4.png")
    annotated.save(a.output / "annotation.png")
    fit_json = []
    for item in fits:
        err = item[0] / (item[4] * item[5] * 3 * 255)
        fit_json.append(dict(numerator=item[0], denominator=item[4]*item[5]*3*255,
                             error=err, x=item[1], y=item[2], scale=item[3],
                             width=item[4], height=item[5]))
    gap = (fit_json[1]["error"] - fit_json[0]["error"]) if len(fit_json)>1 else None
    result = dict(inputs=dict(reference=str(a.reference), reference_sha256=sha(a.reference),
        template_manifest=str(a.manifest), manifest_sha256=sha(a.manifest),
        backdrop=str(back["archive_name"]), backdrop_sha256=sha(backpath)),
        method="exhaustive RGB absolute error; integer translation; nearest scale 1..3",
        backdrop_candidates=fit_json, backdrop_unique=bool(len(fit_json)>1 and fit_json[0]["error"]<=.08 and gap>=.01),
        panels=panel_results,
        strings=[dict(index=i, text=s, screen_rect=None, status="label verified; glyph positions not uniquely matched") for i,s in LABELS],
        glyph_matches=glyph_families, unresolved="per-character independent alpha-shape fits are diagnostic; shared word advances/placement and exact font identity require confidence controls",
        reconstruction=dict(candidate="candidate.png", numerator=numerator, denominator=DENOMINATOR,
                            score=numerator/DENOMINATOR, threshold=.02, all_pixels=True,
                            diagnostic_only=True))
    (a.output / "layout.json").write_text(json.dumps(result, indent=2)+"\n", encoding="utf-8")


if __name__ == "__main__":
    main()
