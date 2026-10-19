#!/usr/bin/env python3
"""Freeze hashes for the raw inputs used by the bounded A1 cell fixture."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]


def sha(path:Path)->str:
    h=hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda:stream.read(1024*1024),b""):
            h.update(chunk)
    return h.hexdigest()


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--capture-dir",type=Path,default=ROOT/"build/tmp/a1-cell-fixture/live-capture")
    ap.add_argument("--audit",type=Path,default=ROOT/"build/tmp/a1-cell-fixture/live-mesh-audit.json")
    ap.add_argument("--source-audit",type=Path,default=ROOT/"build/tmp/a1-cell-fixture/source-audit.json")
    ap.add_argument("--outer-texture-dir",type=Path,default=ROOT/"build/tmp/a1-facilitator")
    ap.add_argument("--inner-texture-dir",type=Path,default=ROOT/"build/tmp/a1-cell-fixture/facilitator-inner-textures")
    ap.add_argument("--extra-dir",type=Path,default=ROOT/"build/tmp/a1-cell-fixture/facilitator-extra-meshes",
                    help="optional legacy supplemental mesh capture consumed by the renderer")
    ap.add_argument("--output",type=Path,default=ROOT/"build/tmp/a1-cell-fixture/input-manifest-v2.json")
    args=ap.parse_args()
    out=args.output.resolve()
    if (ROOT/"build/tmp").resolve() not in out.parents:
        raise SystemExit("input manifest must be written under ignored build/tmp")
    if out.exists():
        raise SystemExit(f"refusing to overwrite frozen manifest: {out}")
    paths=set()
    paths.update(p.resolve() for p in args.capture_dir.rglob("*") if p.is_file())
    extra_dir=args.extra_dir.resolve()
    if extra_dir.is_dir():
        paths.update(p.resolve() for p in extra_dir.rglob("*") if p.is_file())
    for folder in (args.outer_texture_dir,args.inner_texture_dir):
        paths.update(p.resolve() for p in folder.glob("tile-*-*HI.bmp") if p.is_file())
    paths.update({args.audit.resolve(),args.source_audit.resolve(),
                  ROOT/"build/tmp/a1-camera/assets/COMPILED/MM_CLIMB_002.LVL"})
    fixture=ROOT/"tools/harness/fixtures/camera_a1"
    paths.update(p.resolve() for p in fixture.iterdir() if p.is_file() and
                 (p.name.startswith("original-") or p.suffix==".json"))
    files=[]
    for path in sorted(paths):
        if not path.is_file():
            raise FileNotFoundError(path)
        files.append({"path":str(path),"bytes":path.stat().st_size,"sha256":sha(path)})
    report={"scope":"frozen input bytes for complete captured A1 cell candidate; READ-only inputs under ignored build/tmp plus committed fixture parameters",
            "optional_extra_dir":str(extra_dir) if extra_dir.is_dir() else None,
            "files":files}
    report["manifest_sha256"]=hashlib.sha256(json.dumps(report,sort_keys=True,separators=(",",":")).encode()).hexdigest()
    out.parent.mkdir(parents=True,exist_ok=True)
    out.write_text(json.dumps(report,indent=2),encoding="utf-8")
    print(json.dumps({"output":str(out),"manifest_sha256":report["manifest_sha256"],"file_count":len(files),"total_bytes":sum(x["bytes"] for x in files)},indent=2))


if __name__=="__main__":main()
