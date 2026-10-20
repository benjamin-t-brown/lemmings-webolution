"""Capture selected actor SPR frame records from one owned stock run; always restore."""
from pathlib import Path
import argparse, ctypes, hashlib, importlib.util, json, struct, subprocess, sys, time, traceback
ROOT=Path(r'C:\progs\lemmings-webolution')
parser=argparse.ArgumentParser(); parser.add_argument('--out',required=True); args=parser.parse_args()
OUT=(ROOT/args.out).resolve(); assert ROOT in OUT.parents
if OUT.exists() and any(OUT.iterdir()): raise SystemExit(f'refusing non-empty output directory: {OUT}')
OUT.mkdir(parents=True,exist_ok=True)
sys.path.insert(0,str(ROOT/'tools/harness'))
from acquire_game_foreground import acquire
from lrinput import LRInput
from lrstate import ui_state
from goto_level import loaded_level
from memdump import k32,read_mem,find_pids
from measure_spawn import actor_ptrs
INSTALL=Path(r'C:\progs\Lemmings Revolution'); EXE=INSTALL/'Lemmings Revolution.exe'
def load(name,path):
 spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
entry=load('entry',ROOT/'build/tmp/a1-zoom-elevation/enter_fast.py'); entry.WORK=OUT
old_find=entry.find_menu_index
def find_menu(h,inp):
 inp._set_u32(32,1); entry.aim_at(h,inp,(620,460)); return old_find(h,inp)
entry.find_menu_index=find_menu
conf=(INSTALL/'dgVoodoo.conf').read_bytes(); before={n:hashlib.sha256((INSTALL/n).read_bytes()).hexdigest() for n in ('Lemmings Revolution.exe','SavedGame.dat','dgVoodoo.conf')}
(OUT/'install-dgVoodoo.original.conf').write_bytes(conf)
(OUT/'install-SavedGame.original.dat').write_bytes((INSTALL/'SavedGame.dat').read_bytes())
assert before['Lemmings Revolution.exe']=='32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f'
owned=None; handle=None; backups={}; log={'writes':[],'frames':[]}
ntdll=ctypes.WinDLL('ntdll')
for n in ('NtSuspendProcess','NtResumeProcess'):getattr(ntdll,n).argtypes=[ctypes.c_void_p];getattr(ntdll,n).restype=ctypes.c_long
def valid_ptr(a,what):
 if not 0x10000<=a<0x80000000: raise RuntimeError(f'{what} pointer implausible: {a:#x}')
 return a
def rd(a,n):
 valid_ptr(a,'read')
 b=read_mem(handle,a,n)
 if b is None or len(b)!=n:raise RuntimeError(f'read failed {a:#x}/{n}')
 return b
def u32(a):return struct.unpack('<I',rd(a,4))[0]
def i32(a):return struct.unpack('<i',rd(a,4))[0]
def wr(a,b,why):
 backups.setdefault(a,rd(a,len(b))); log['writes'].append({'address':hex(a),'before':rd(a,len(b)).hex(),'after':b.hex(),'why':why})
 q=ctypes.c_size_t()
 if not k32.WriteProcessMemory(handle,ctypes.c_void_p(a),b,len(b),ctypes.byref(q)) or q.value!=len(b):raise RuntimeError(f'write failed {a:#x}')
 assert rd(a,len(b))==b
def capture(actor,label):
 c=actor+0x128; ai=i32(c+0x1c8); ti=i32(c+0x1cc); dindex=i32(c+0x1b8); mirror=rd(c+0x1b4,1)[0]
 cont=valid_ptr(u32(c+0x1f8),'container'); actarr=valid_ptr(u32(cont+0xc),'action array'); act=actarr+ai*0x98; temp_arr=valid_ptr(u32(act+0xc),'temporal array'); temp=temp_arr+ti*0x10
 count=u32(temp); assert count in (1,5,8),('direction count',count)
 dbase=valid_ptr(u32(temp+0xc),'direction array'); assert 0<=dindex<count,('direction index/count',dindex,count)
 rec=dbase+dindex*0x48; valid_ptr(rec,'frame record')
 log['frames'].append({'label':label,'actor':hex(actor),'controller':hex(c),'action_index':ai,'temporal_index':ti,'direction_index':dindex,'mirror':mirror,'container':hex(cont),'action':hex(act),'temporal':hex(temp),'temporal_bytes':rd(temp,16).hex(),'direction_base':hex(dbase),'frame_record':hex(rec),'frame_bytes_0x48':rd(rec,0x48).hex(),'count':count,'camera_bytes':rd(0x4cbfc8,0x168).hex(),'actor_draw_matrix_4c6b88':rd(0x4c6b88,0x60).hex(),'draw_position_bytes':rd(actor+0x108,24).hex(),'native_geometry_fields':{hex(off):rd(c+off,8).hex() for off in (0x20,0x28,0x30,0x38,0x88,0x90,0xf0,0xf8,0x158,0x160)}})
def save(): (OUT/'report.json').write_text(json.dumps(log,indent=2)+'\n',encoding='utf-8')
try:
 import json as _json
 native=_json.loads((ROOT/'build/tmp/a4f-actor-billboard/native-read.json').read_text(encoding='utf-8'))
 accessor=next(w for w in native['windows'] if w['address']=='0x0047abc0')
 raw=bytes.fromhex(accessor['bytes_hex'])
 assert raw.startswith(bytes.fromhex('558bec51894dfc8b45086bc0488b4dfc8b510c03c2')), '47ABC0 accessor bytes differ'
 log['accessor_evidence']={'address':'0x0047ABC0','raw_prefix':raw[:18].hex(),'meaning_read':'[this+0x0c] + index*0x48'}
 assert not find_pids('Lemmings'),'exclusive live ownership required'
 (INSTALL/'dgVoodoo.conf').write_bytes((ROOT/'build/tmp/a1-zoom-elevation/dgVoodoo.windowed.conf').read_bytes())
 owned=subprocess.Popen([str(EXE)],cwd=INSTALL); log['pid']=owned.pid;save();time.sleep(5)
 assert entry.main()==0
 handle=k32.OpenProcess(0xc38,False,owned.pid); assert handle
 assert loaded_level(handle).lower()==r'compiled\mm_climb_002.lvl',loaded_level(handle)
 with LRInput() as inp:
  inp.enable();inp.clear();acquire(owned.pid);inp.tap('P',2);assert ui_state(handle)==0;time.sleep(10);acquire(owned.pid);inp.tap('P',2);inp.wait_frames(3);inp.release()
 assert ui_state(handle)==1
 actors=[a for a in actor_ptrs(handle) if a and u32(a) in (0x4b10e8,0x4b0f6c)]
 def projected_anchor(a):
  pos=struct.unpack('<3d',rd(a+0x108,24)); mat=struct.unpack('<12d',rd(0x58d3d8,96))
  t=[sum(mat[k+3*j]*pos[j] for j in range(3))+mat[9+k] for k in range(3)]
  return None if t[2]<=0 else (319+t[0]/t[2],239-t[1]/t[2],t[2])
 baseline=[a for a in actors if i32(a+0x128+0x1c8)==13 and 0<i32(a+0x128+0x1b8)<5 and projected_anchor(a) and 32<projected_anchor(a)[0]<608 and 32<projected_anchor(a)[1]<448]
 assert baseline,'no selected visible walk actors'
 actor=baseline[0];log['five_eligibility']={'actor':hex(actor),'screen_anchor':projected_anchor(actor),'crop64':[int(projected_anchor(actor)[0])-32,int(projected_anchor(actor)[1])-32,int(projected_anchor(actor)[0])+32,int(projected_anchor(actor)[1])+32],'policy':'5-direction walk action13, nonzero selected index, projected in-front center with full 64x64 crop inside 640x480'}
 capture(actor,'five-walk-native')
 c=actor+0x128; backups[c+0x1c8]=rd(c+0x1c8,4);backups[c+0x1cc]=rd(c+0x1cc,4)
 wr(c+0x1c8,struct.pack('<i',1),'controlled block graphics action');wr(c+0x1cc,bytes(4),'controlled block temporal0')
 with LRInput() as inp:inp.enable();inp.clear();inp.wait_frames(3);inp.release()
 container=u32(c+0x1f8); act=u32(container+0xc)+0x98; temp=u32(act+0xc); assert u32(temp)==8,('controlled block temporal count',u32(temp))
 capture(actor,'eight-block-controlled')
 log['level']=loaded_level(handle);log['success']=True; save()
except Exception:log['error']=traceback.format_exc()
finally:
 if handle:
  try:
   status=ntdll.NtSuspendProcess(handle);log['suspend_status']=status
   assert status==0
   for a,b in reversed(list(backups.items())):
    q=ctypes.c_size_t();ok=k32.WriteProcessMemory(handle,ctypes.c_void_p(a),b,len(b),ctypes.byref(q));assert ok and q.value==len(b) and rd(a,len(b))==b
   log['restored']={hex(a):rd(a,len(b))==b for a,b in backups.items()};log['all_restored']=all(log['restored'].values())
  except Exception:log['cleanup_error']=traceback.format_exc()
  if log.get('suspend_status')==0:ntdll.NtResumeProcess(handle)
  k32.CloseHandle(handle)
 try:
  with LRInput() as inp:inp.release()
 except Exception:pass
 if owned:
  owned.terminate();owned.wait(timeout=10);log['process_stopped']=owned.poll() is not None
 try: (INSTALL/'dgVoodoo.conf').write_bytes(conf)
 except Exception:log['install_restore_error']=traceback.format_exc()
 log['install_restored']={n:hashlib.sha256((INSTALL/n).read_bytes()).hexdigest()==h for n,h in before.items()}
 save();print(json.dumps({k:log.get(k) for k in ('success','error','cleanup_error','all_restored','process_stopped','install_restored')}))
