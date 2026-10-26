#!/usr/bin/env python3
"""Repeat the fixed C3 repair4 masked source->accepted software-write trace."""
from pathlib import Path
import csv, hashlib, json, os, subprocess, sys
from PIL import Image

ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'build/tmp/c3bg-write-diagnostic'
OUT.mkdir(parents=True,exist_ok=True)
FIX=ROOT/'build/tmp/a6f-fresh-pause-gate/repair4'
MASKPNG=ROOT/'build/tmp/c3be-residual-attribution/repair1/primary-changed-rest-mask.png'
BASE=ROOT/'build/tmp/c3-residual-pass/fresh/repair4.png'
CAM=FIX/'live-camera-params.json'
WALK=FIX/'injected-plus8.json'
BIN=ROOT/'build/cmake/dev-debug/webolution.exe'
mask=Image.open(MASKPNG).convert('L')
assert mask.size==(640,480),mask.size
maskraw=bytes(1 if x else 0 for x in mask.tobytes())
maskbin=OUT/'primary-mask-640x480.bin'; maskbin.write_bytes(maskraw)
assert sum(maskraw)==100172
def render(name, diagnostic):
    png=OUT/f'{name}.png'
    def rel(p): return p.relative_to(ROOT).as_posix()
    cmd=['cd /c/progs/lemmings-webolution && build/cmake/dev-debug/webolution.exe','--native','--geom','route','--level','MM_CLIMB_002','--camera',rel(CAM),
         '--walkers',rel(WALK),'--skill-panel','--hud-ticks','70','--pause-caption','--shot',rel(png)]
    if diagnostic: cmd += ['--c3-write-mask',rel(maskbin),'--c3-write-events',rel(OUT/'masked-events.csv')]
    env={k:v for k,v in os.environ.items() if not k.startswith('WEBOLUTION_')}
    p=subprocess.run(['powershell','-NoProfile','-ExecutionPolicy','Bypass','-File',str(ROOT/'scripts/Invoke-Ucrt64.ps1'),' '.join(cmd)],cwd=ROOT,env=env,capture_output=True)
    (OUT/f'{name}.log').write_bytes(p.stdout+b'\n'+p.stderr)
    if p.returncode: raise RuntimeError(f'{name} failed rc={p.returncode}; see {OUT}/{name}.log')
    return png
off=render('repair4-diagnostic-off',False)
on=render('repair4-diagnostic-on',True)
assert off.read_bytes()==on.read_bytes(), 'diagnostic changed rendered PNG bytes'
assert off.read_bytes()==BASE.read_bytes(), 'ordinary repair4 image differs from fixed baseline'
rows=list(csv.DictReader((OUT/'masked-events.csv').open(newline='')))
candidates=[r for r in rows if r['kind']=='candidate']
billboards=[r for r in candidates if int(r['block'])==18446744073709551615]
mesh_candidates=[r for r in candidates if r not in billboards]
writes=[r for r in rows if r['kind']=='write']
from collections import defaultdict
tot=defaultdict(lambda:{'color_assignments':0,'depth_assignments':0,'unique_pixels':set(),'triangles':set()})
for r in writes:
    s=r['source']; t=tot[s]; t['color_assignments']+=1; t['unique_pixels'].add(int(r['pixel']));t['triangles'].add(int(r['triangleIndex']))
    t['depth_assignments']+=int(r['depthAssigned'])
records={r['source']:{'node':int(r['node']),'block':int(r['block']),'candidate_triangles':int(r['triangleIndex'])} for r in candidates}
for s in records: tot[s]
result={'scope':'software SOURCE only','mask_pixels':sum(maskraw),'candidate_mesh_records':len(mesh_candidates),
        'primary_billboard_candidates':len(billboards),
        'masked_color_assignments':len(writes),'masked_depth_assignments':sum(int(r['depthAssigned']) for r in writes),
        'masked_unique_pixels':len({int(r['pixel']) for r in writes}),
        'records':{s:{'node':records[s]['node'],'block':records[s]['block'],'candidate_triangles':records[s]['candidate_triangles'],
                      'color_assignments':v['color_assignments'],'depth_assignments':v['depth_assignments'],
                      'unique_pixels':len(v['unique_pixels']),'accepted_triangle_indices':sorted(v['triangles'])} for s,v in tot.items()},
        'raw_event_csv':str(OUT/'masked-events.csv'),'raw_event_sha256':hashlib.sha256((OUT/'masked-events.csv').read_bytes()).hexdigest(),
        'png_sha256':hashlib.sha256(on.read_bytes()).hexdigest(),'event_cap':None,'truncated':False}
(OUT/'masked-totals.json').write_text(json.dumps(result,indent=2)+'\n',newline='\n')
print(json.dumps({k:v for k,v in result.items() if k!='records'},indent=2))
