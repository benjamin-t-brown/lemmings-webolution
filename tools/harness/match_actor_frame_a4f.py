"""Match live actor frame record prefixes against all six retail lemming SPR files."""
from __future__ import annotations
import argparse, hashlib, json, struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
DEFAULT=ROOT/'build/tmp/a4f-actor-billboard/live-frame-read-4/report.json'
ASSETS=ROOT/'build/tmp/a3-facilitator'
def u32(data,at): return struct.unpack_from('<I',data,at)[0]
def text(data,at):
 n=u32(data,at); start=at+4
 try: value=data[start:start+n].decode('ascii')
 except Exception as e: raise ValueError(f'text error at {at:#x}, n={n}, bytes={data[start:start+32].hex()}') from e
 return value,start+n

def parse(path):
 data=path.read_bytes(); at=0; records=[]
 sprite_count=u32(data,at);at+=4
 for action_i in range(sprite_count):
  anim_count=u32(data,at);at+=4
  for temporal_i in range(anim_count):
   count=u32(data,at);at+=4
   for direction_i in range(count):
    start=at; raw=data[start:start+64]
    bottom,top,left,right=struct.unpack_from('<4I',raw,0)
    uv=struct.unpack_from('<4d',raw,16)
    unknown0,unknown1,width,height=struct.unpack_from('<4I',raw,48)
    at+=64; sheet,at=text(data,at)
    records.append({'offset':start,'action_index':action_i,'temporal_index':temporal_i,'direction_index':direction_i,'raw64':raw,'rect_ltrb':[left,top,right,bottom],'uv_bottom_top_left_right':list(uv),'unknown':[unknown0,unknown1],'size':[width,height],'sheet':sheet})
  _,at=text(data,at);at+=8
 palette_count=u32(data,at);at+=4+palette_count*(4+48*8)
 if at!=len(data): raise ValueError(f'{path.name}: parser ended at {at:#x}/{len(data):#x}')
 return data,records

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--capture',type=Path,default=DEFAULT);ap.add_argument('--out',type=Path,default=ROOT/'build/tmp/a4f-actor-billboard/resource-match.json');a=ap.parse_args()
 capture=json.loads(a.capture.read_text(encoding='utf-8')); resources={}
 for path in sorted(ASSETS.glob('LEMMINGS*.SPR')):
  print('parsing',path.name)
  resources[path.name]=parse(path)
 if len(resources)!=6: raise SystemExit(f'expected six retail SPRs, found {len(resources)}')
 frames=[]
 for native in capture['frames']:
  raw=bytes.fromhex(native['frame_bytes_0x48'])[:64];matches=[]
  for name,(data,records) in resources.items():
   for rec in records:
    if rec['raw64']==raw:
     matches.append({k:v for k,v in rec.items() if k!='raw64'}|{'resource':name,'resource_sha256':hashlib.sha256(data).hexdigest(),'record_sha256':hashlib.sha256(raw).hexdigest()})
  frames.append({'fixture':native['label'],'native_action':native['action_index'],'native_temporal':native['temporal_index'],'native_direction':native['direction_index'],'native_mirror':native['mirror'],'native_record_sha256':hashlib.sha256(raw).hexdigest(),'unique_match':len(matches)==1,'matches':matches})
 result={'evidence':'live frame first 64 bytes compared exactly against every parsed descriptor in the six extracted retail SPR assets','capture':str(a.capture),'frames':frames}
 a.out.parent.mkdir(parents=True,exist_ok=True);a.out.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
 print(json.dumps(result,indent=2))
 return 0 if all(x['unique_match'] for x in frames) else 1
if __name__=='__main__':raise SystemExit(main())
