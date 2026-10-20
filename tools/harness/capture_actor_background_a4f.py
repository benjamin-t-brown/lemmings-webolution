"""Capture native actor/background pairs with one paused actor position write."""
from __future__ import annotations
import argparse,ctypes,hashlib,importlib.util,json,math,struct,subprocess,sys,time,traceback
from pathlib import Path
ROOT=Path(r'C:\progs\lemmings-webolution')
ap=argparse.ArgumentParser();ap.add_argument('--out',required=True);args=ap.parse_args()
OUT=(ROOT/args.out).resolve();assert ROOT in OUT.parents
if OUT.exists() and any(OUT.iterdir()):raise SystemExit(f'refusing non-empty output dir: {OUT}')
OUT.mkdir(parents=True,exist_ok=True)
sys.path.insert(0,str(ROOT/'tools/harness'))
from acquire_game_foreground import acquire
from lrinput import LRInput
from lrstate import ui_state
from goto_level import loaded_level
from memdump import k32,read_mem,find_pids
from measure_spawn import actor_ptrs
INSTALL=Path(r'C:\progs\Lemmings Revolution');EXE=INSTALL/'Lemmings Revolution.exe'
def load(name,path):
 spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
entry=load('entry',ROOT/'build/tmp/a1-zoom-elevation/enter_fast.py');entry.WORK=OUT
cap=load('capture',ROOT/'build/tmp/a1-zoom-elevation/capture_branches.py')
class FocusedInput(LRInput):
 def tap(self,*a,**kw):acquire(owned.pid);return super().tap(*a,**kw)
entry.LRInput=FocusedInput
old_find=entry.find_menu_index
def focused_find(h,inp):
 acquire(owned.pid);inp._set_u32(32,1);entry.aim_at(h,inp,(620,460));return old_find(h,inp)
entry.find_menu_index=focused_find
conf=(INSTALL/'dgVoodoo.conf').read_bytes();before={n:hashlib.sha256((INSTALL/n).read_bytes()).hexdigest() for n in ('Lemmings Revolution.exe','SavedGame.dat','dgVoodoo.conf')}
assert before['Lemmings Revolution.exe']=='32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f'
(OUT/'install-dgVoodoo.original.conf').write_bytes(conf);(OUT/'install-SavedGame.original.dat').write_bytes((INSTALL/'SavedGame.dat').read_bytes())
owned=None;handle=None;suspended=False;backups={};log={'writes':[],'pairs':[]}
ntdll=ctypes.WinDLL('ntdll')
for n in ('NtSuspendProcess','NtResumeProcess'):getattr(ntdll,n).argtypes=[ctypes.c_void_p];getattr(ntdll,n).restype=ctypes.c_long
def save(): (OUT/'report.json').write_text(json.dumps(log,indent=2)+'\n',encoding='utf-8')
def ptr(v,what):
 if not 0x10000<=v<0x80000000:raise RuntimeError(f'{what} pointer out of 32-bit user range: {v:#x}')
 return v
def rd(a,n):
 ptr(a,'read');b=read_mem(handle,a,n)
 if b is None or len(b)!=n:raise RuntimeError(f'read failed {a:#x}/{n}')
 return b
def u32(a):return struct.unpack('<I',rd(a,4))[0]
def i32(a):return struct.unpack('<i',rd(a,4))[0]
def f64(a):return struct.unpack('<d',rd(a,8))[0]
def remember(a,n):backups.setdefault(a,rd(a,n))
def wr(a,b,why):
 remember(a,len(b));log['writes'].append({'address':hex(a),'before':rd(a,len(b)).hex(),'after':b.hex(),'why':why});save()
 q=ctypes.c_size_t()
 if not k32.WriteProcessMemory(handle,ctypes.c_void_p(a),b,len(b),ctypes.byref(q)) or q.value!=len(b):raise RuntimeError(f'write failed {a:#x}')
 assert rd(a,len(b))==b
 if a==0x0046840b:k32.FlushInstructionCache(handle,ctypes.c_void_p(a),len(b))
def polls(n=3):
 with LRInput() as inp:inp.enable();inp.clear();inp.wait_frames(n);inp.release()
def state_stamp(actor):
 c=actor+0x128
 return {'level':loaded_level(handle),'ui_state':ui_state(handle),'camera':rd(0x4cbfc8,0x168).hex(),'action':i32(c+0x1c8),'temporal':i32(c+0x1cc),'direction':i32(c+0x1b8),'actor_source_position':rd(actor+8,24).hex(),'draw_position':rd(actor+0x108,24).hex(),'controller_flags':{'1b0':u32(c+0x1b0),'1b4':u32(c+0x1b4)},'draw_globals':{'58d53c':i32(0x58d53c),'58cb38':i32(0x58cb38)}}
def shot(name,actor):
 assert ui_state(handle)==1,('pause state lost',ui_state(handle))
 acquire(owned.pid);polls(3)
 before=state_stamp(actor)
 image,_=cap.shot(cap.find_hwnd(owned.pid));assert image.size==(640,480),image.size
 after=state_stamp(actor)
 log.setdefault('state_stamps',{})[name]={'before':before,'after':after};save()
 image=image.convert('RGB');image.save(OUT/f'{name}.png');return image

def actor_row(actor):
 c=actor+0x128;ai=i32(c+0x1c8);ti=i32(c+0x1cc);di=i32(c+0x1b8)
 cont=ptr(u32(c+0x1f8),'container');arr=ptr(u32(cont+0xc),'action array');assert 0<=ai<128
 act=arr+ai*0x98;ta=ptr(u32(act+0xc),'temporal array');assert 0<=ti<256
 temp=ta+ti*0x10;count=u32(temp);assert count in (1,5,8)
 dbase=ptr(u32(temp+0xc),'direction records');assert 0<=di<count
 rec=dbase+di*0x48;raw=rd(rec,0x48)
 pos=struct.unpack('<3d',rd(actor+0x108,24));mat=struct.unpack('<12d',rd(0x58d3d8,96));trans=[sum(mat[k+3*j]*pos[j] for j in range(3))+mat[9+k] for k in range(3)]
 anchor=None if trans[2]<=0 else [319+trans[0]/trans[2],239-trans[1]/trans[2]]
 delta=i32(0x58d53c)-i32(0x58cb38)
 vertex_bases=(0x8,0x70,0xd8,0x140)
 material_vertices=[]
 for index,off in enumerate(vertex_bases):
  raw_vertex=rd(c+off,0x68)
  rgba=struct.unpack('<4d',rd(c+off+0x38,32))
  material_vertices.append({'index':index,'base_offset':hex(off),'record_bytes_0x68':raw_vertex.hex(),'rgba_f64':rgba,'rgba_bytes':rd(c+off+0x38,32).hex()})
 return {'actor':hex(actor),'action':ai,'temporal':ti,'direction':di,'count':count,'blend_selector_word':u32(c),'flag_1b0':u32(c+0x1b0),'flag_1b4':u32(c+0x1b4),'container':hex(cont),'action_record':hex(act),'temporal_record':hex(temp),'temporal_bytes':rd(temp,16).hex(),'direction_base':hex(dbase),'frame_record':hex(rec),'frame_bytes_0x48':raw.hex(),'position':pos,'actor_source_position_bytes':rd(actor+8,24).hex(),'projected_origin':anchor,'global_scale_operands':{'58d53c':i32(0x58d53c),'58cb38':i32(0x58cb38),'delta':delta},'camera_bytes':rd(0x4cbfc8,0x168).hex(),'matrix_bytes':rd(0x58d3d8,96).hex(),'actor_draw_matrix_4c6b88':rd(0x4c6b88,96).hex(),'draw_position_bytes':rd(actor+0x108,24).hex(),'render_geometry_fields':{hex(off):rd(c+off,8).hex() for off in (0x20,0x28,0x30,0x38,0x88,0x90,0xf0,0xf8,0x158,0x160)},'material_vertices':material_vertices,'material_state_bytes':rd(0x58d280,16).hex()}

def get_actor():
 found=[]
 for a in actor_ptrs(handle):
  if not a or u32(a) not in (0x4b10e8,0x4b0f6c):continue
  row=actor_row(a);origin=row['projected_origin'];x,y=(origin[0],origin[1]) if origin else (None,None)
  if row['count']==5 and row['action']==13 and row['direction']>0 and x is not None and 32<x<608 and 32<y<448:
   found.append((a,row))
 if not found:raise RuntimeError('no actor-list walk/action13/count5/index>0 with full in-bounds 64x64 crop')
 return found[0]

def metrics(a,b):
 from PIL import ImageChops
 d=ImageChops.difference(a,b);pix=list(d.getdata());changed=[(i%a.width,i//a.width) for i,p in enumerate(pix) if p!=(0,0,0)]
 box=None if not changed else [min(x for x,y in changed),min(y for x,y in changed),max(x for x,y in changed)+1,max(y for x,y in changed)+1]
 return {'changed':len(changed),'bbox':box,'same':not changed}

def pair(actor,label):
 row=actor_row(actor);assert row['projected_origin'];ax,ay=row['projected_origin'][:2]
 crop=[math.floor(ax)-32,math.floor(ay)-32,math.floor(ax)+32,math.floor(ay)+32];assert 0<=crop[0]<crop[2]<=640 and 0<=crop[1]<crop[3]<=480
 pair={'label':label,'actor_row':row,'crop':crop,'suppression':{'mechanism':'actor+0x38C scaled draw branch; zero the two 0x47A190 scale arguments at actor+0x390/+0x398','flag_address':hex(actor+0x38c),'flag_before':rd(actor+0x38c,1).hex(),'args_address':hex(actor+0x390),'args_before':rd(actor+0x390,16).hex(),'args_after':'00'*16}}
 log['pairs'].append(pair);save()
 orig=shot(f'{label}-native',actor);pair['native_sha256']=hashlib.sha256((OUT/f'{label}-native.png').read_bytes()).hexdigest()
 remember(actor+0x38c,1);remember(actor+0x390,16)
 wr(actor+0x38c,b'\x01',f'{label}: select actor-specific scaled draw branch')
 wr(actor+0x390,bytes(16),f'{label}: zero only the two scaled draw extents for actor {actor:#x}')
 polls(3);suppressed_row=actor_row(actor);pair['suppression']['geometry_after_update']=suppressed_row['render_geometry_fields'];pair['suppression']['draw_globals_after_update']={'58d53c':i32(0x58d53c),'58cb38':i32(0x58cb38)}
 bg=shot(f'{label}-background-no-actor',actor);pair['background_sha256']=hashlib.sha256((OUT/f'{label}-background-no-actor.png').read_bytes()).hexdigest()
 pair['suppression']['flag_during_capture']=rd(actor+0x38c,1).hex();pair['suppression']['args_during_capture']=rd(actor+0x390,16).hex()
 wr(actor+0x390,pair['suppression']['args_before'] and bytes.fromhex(pair['suppression']['args_before']),f'{label}: restore scaled draw args')
 wr(actor+0x38c,bytes.fromhex(pair['suppression']['flag_before']),f'{label}: restore scaled draw selector')
 restored=shot(f'{label}-restored',actor);pair['restored_sha256']=hashlib.sha256((OUT/f'{label}-restored.png').read_bytes()).hexdigest()
 pair['native_vs_restored']=metrics(orig,restored);pair['native_vs_background']=metrics(orig,bg)
 x0,y0,x1,y1=crop;pair['crop_native_vs_restored']=metrics(orig.crop(crop),restored.crop(crop));pair['crop_native_vs_background']=metrics(orig.crop(crop),bg.crop(crop))
 pair['outside_crop_changes']=sum(1 for y in range(480) for x in range(640) if not (x0<=x<x1 and y0<=y<y1) and orig.getpixel((x,y))!=bg.getpixel((x,y)))
 stamps=log['state_stamps'];base=stamps[f'{label}-native']['before'];rest=stamps[f'{label}-restored']['after'];supp=stamps[f'{label}-background-no-actor']['after']
 stable=('level','ui_state','camera','action','temporal','direction','controller_flags')
 pair['stable_state_fields']={k:base[k]==rest[k]==supp[k] for k in stable};pair['stable_state']=all(pair['stable_state_fields'].values())
 pair['suppression']['restored']=rd(actor+0x38c,1)==bytes.fromhex(pair['suppression']['flag_before']) and rd(actor+0x390,16)==bytes.fromhex(pair['suppression']['args_before'])
 pair['draw_position_matches_captured']=rd(actor+0x108,24)==bytes.fromhex(row['draw_position_bytes']);save()
 return pair
try:
 assert not find_pids('Lemmings'),'exclusive live ownership required'
 (INSTALL/'dgVoodoo.conf').write_bytes((ROOT/'build/tmp/a1-zoom-elevation/dgVoodoo.windowed.conf').read_bytes())
 owned=subprocess.Popen([str(EXE)],cwd=INSTALL);log['pid']=owned.pid;save();time.sleep(5)
 assert entry.main()==0
 handle=k32.OpenProcess(0xc38,False,owned.pid);assert handle
 assert loaded_level(handle).lower()==r'compiled\mm_climb_002.lvl'
 with LRInput() as inp:
  inp.enable();inp.clear();acquire(owned.pid);inp.tap('P',2);assert ui_state(handle)==0;time.sleep(10);acquire(owned.pid);inp.tap('P',2);inp.wait_frames(3);inp.release()
 assert ui_state(handle)==1;log['loaded_level']=loaded_level(handle)
 actor,first=get_actor();log['actor_list_selected']=hex(actor);pair(actor,'five')
 c=actor+0x128;remember(c+0x1c8,4);remember(c+0x1cc,4);remember(c+0x1b8,4);remember(c+0x1b4,1)
 wr(c+0x1c8,struct.pack('<i',1),'controlled block graphics action ordinal1');wr(c+0x1cc,bytes(4),'controlled block temporal0');polls(4)
 second=actor_row(actor);assert second['count']==8 and second['action']==1 and second['temporal']==0
 pair(actor,'eight')
 log['success']=True;save()
except Exception:log['error']=traceback.format_exc();save()
finally:
 if handle:
  try:
   status=ntdll.NtSuspendProcess(handle);log['suspend_status']=status;assert status==0;suspended=True
   for address,data in reversed(list(backups.items())):
    wr(address,data,'final canonical-backup restoration while suspended');assert rd(address,len(data))==data
   log['restored']={hex(a):rd(a,len(b))==b for a,b in backups.items()};log['all_restored']=all(log['restored'].values())
  except Exception:log['cleanup_error']=traceback.format_exc()
  if suspended:ntdll.NtResumeProcess(handle)
  k32.CloseHandle(handle)
 try:
  with LRInput() as inp:inp.release()
 except Exception:pass
 if owned:
  try:owned.terminate();owned.wait(timeout=10);log['process_stopped']=owned.poll() is not None
  except Exception:log['process_cleanup_error']=traceback.format_exc()
 try:(INSTALL/'dgVoodoo.conf').write_bytes(conf)
 except Exception:log['install_restore_error']=traceback.format_exc()
 log['install_after']={n:hashlib.sha256((INSTALL/n).read_bytes()).hexdigest() for n in before};log['install_before']=before;log['install_restored']=log['install_after']==before
 save();print(json.dumps({k:log.get(k) for k in ('success','error','cleanup_error','all_restored','process_stopped','install_restored')}))
