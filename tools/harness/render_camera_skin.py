"""Bounded retail skin hypothesis; no original-derived geometry, UV or camera fit."""
import hashlib
import json
import math
import struct
import sys
from pathlib import Path
from PIL import Image
import argparse

ap=argparse.ArgumentParser()
ap.add_argument('--hi',action='store_true')
ap.add_argument('--linear',action='store_true')
ap.add_argument('--parameters',type=Path)
ap.add_argument('--state',type=Path)
ap.add_argument('--label',default='initial')
args=ap.parse_args()

ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'build/tmp/a1-facilitator'
OUT.mkdir(parents=True,exist_ok=True)
FIXTURES=Path(__file__).resolve().parent/'fixtures/camera_a1'
AGENT=OUT.parent/'a1-camera'
from camera_fixture import candidate_camera

state_path=args.state or FIXTURES/'reference-state.json'
state=json.loads(state_path.read_text())
level=AGENT/'assets/COMPILED/MM_CLIMB_002.LVL'
b=level.read_bytes()
# Signature from retained parser; require one exact match and validate ring heights.
signature=struct.pack('<IdI',112,10.0,7)
assert b.count(signature)==1
start=b.index(signature)+len(signature)
vertices=[struct.unpack_from('<3d',b,start+24*i) for i in range(7*32*2)]
assert all(vertices[(ring*32+step)*2+1][1]==ring*16 for ring in range(7) for step in range(32))
params=json.loads((FIXTURES/'camera-fixture-params.json').read_text())
params.update({'yaw_units':state['control']['state_40_48_58_60_f64']['0x40'],
               'zoom_shift':state['control']['zoom_i32'],'level_eye_height':state['world_rows'],
               'elevation':state['control']['elevation_f64'],'center':state['center']})
baseline_params=params.copy()
if args.parameters:
    params=json.loads(args.parameters.read_text())
matrix,eye,fx,fy=candidate_camera(params)
# Verify this candidate independently against the facilitator's own replay.
oracle_path=OUT/'pose-independent.json'
if params==baseline_params and not args.state and oracle_path.exists():
    oracle=json.loads(oracle_path.read_text())['candidate_view_columns_then_translation']
    assert max(abs(a-c) for a,c in zip(matrix,oracle))<1e-10
cx,cy=params.get('center',state['center'])
width,height=640,480
image=Image.new('RGB',(width,height),(0,0,0));dst=image.load()
depth=[math.inf]*(width*height)
textures={}
for col in range(8):
    for row in range(3):
        path=(OUT/f'tile-{col}-{row}HI.bmp') if args.hi else (AGENT/f'assets/TEXTURES/CYLINDER/MM_CLIMB_002/OUT_MM_CLIMB_002_{col}_{row}.decoded.bmp')
        tex=Image.open(path).convert('RGB')
        textures[col,row]=(tex,tex.load())

def project(v):
    x,y,z=v
    px=matrix[0]*x+matrix[3]*y+matrix[6]*z+matrix[9]
    py=matrix[1]*x+matrix[4]*y+matrix[7]*z+matrix[10]
    pz=matrix[2]*x+matrix[5]*y+matrix[8]*z+matrix[11]
    assert pz>0
    return cx+fx*px/pz,cy-fy*py/pz,1/pz

def triangle(points,uvs,texture):
    a,b,c=points
    area=(b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])
    if abs(area)<1e-12:return
    xmin=max(0,math.floor(min(v[0] for v in points)))
    xmax=min(width-1,math.ceil(max(v[0] for v in points)))
    ymin=max(0,math.floor(min(v[1] for v in points)))
    ymax=min(height-1,math.ceil(max(v[1] for v in points)))
    tex,src=texture;tw,th=tex.size
    for y in range(ymin,ymax+1):
        for x in range(xmin,xmax+1):
            xx,yy=x+.5,y+.5
            w0=((b[0]-xx)*(c[1]-yy)-(b[1]-yy)*(c[0]-xx))/area
            w1=((c[0]-xx)*(a[1]-yy)-(c[1]-yy)*(a[0]-xx))/area
            w2=1-w0-w1
            if min(w0,w1,w2)<0:continue
            q=w0*a[2]+w1*b[2]+w2*c[2]
            z=1/q;index=y*width+x
            if z>=depth[index]:continue
            weights=(w0,w1,w2)
            u=sum(weights[i]*points[i][2]*uvs[i][0] for i in range(3))/q
            v=sum(weights[i]*points[i][2]*uvs[i][1] for i in range(3))/q
            tx=max(0,min(tw-1,int(u*tw)));ty=max(0,min(th-1,int(v*th)))
            if args.linear:
                sx=u*tw-.5;sy=v*th-.5
                ix=math.floor(sx);iy=math.floor(sy);dx=sx-ix;dy=sy-iy
                samples=[src[max(0,min(tw-1,ix+i)),max(0,min(th-1,iy+j))] for j in (0,1) for i in (0,1)]
                weights=((1-dx)*(1-dy),dx*(1-dy),(1-dx)*dy,dx*dy)
                dst[x,y]=tuple(round(sum(samples[i][c]*weights[i] for i in range(4))) for c in range(3))
            else:
                dst[x,y]=src[tx,ty]
            depth[index]=z

for band in range(6):
    for step in range(32):
        nxt=(step+1)%32
        raw=[vertices[(r*32+s)*2+1] for r,s in ((band,step),(band,nxt),(band+1,nxt),(band+1,step))]
        pts=[project(v) for v in raw]
        u0=(step%4)/4;u1=u0+.25
        v0=1-(band%2)/2;v1=v0-.5
        uvs=[(u0,v0),(u1,v0),(u1,v1),(u0,v1)]
        texture=textures[step//4,2-band//2]
        for indices in ((0,1,2),(0,2,3)):
            triangle([pts[i] for i in indices],[uvs[i] for i in indices],texture)

path=OUT/f'skin-candidate-{args.label}.png'
image.save(path)
report={'status':'Bounded outer-skin camera fixture; whole-scene rendering and cutaway geometry remain incomplete',
        'source_level_sha256':hashlib.sha256(b).hexdigest(),'geometry_offset':start,
        'camera_inputs':params,'eye':eye,'focal_x':fx,'focal_y':fy,
        'texture_selection':'Hi path; live detail flag verified in same-camera repeat reference' if args.hi else 'base texture hypothesis (rejected original baseline)',
        'sampling':'bilinear case; hardware filter flag and cached selection verified in repeat reference' if args.linear else 'nearest case (rejected original baseline)',
        'candidate_sha256':hashlib.sha256(path.read_bytes()).hexdigest(),
        'policy':'skin-fixture-policy.json; unchanged reference-roi.json'}
(OUT/f'skin-candidate-{args.label}.json').write_text(json.dumps(report,indent=2))
print(json.dumps(report,indent=2))
