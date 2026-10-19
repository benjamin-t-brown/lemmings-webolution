"""Facilitator-owned actor input and stock-raster selection replay.

Not a replacement rasterizer. Run only after round worker releases ownership.
Uses established A1 entry/capture plumbing, not worker A3 field/score reader.
"""
from pathlib import Path
import argparse
import ctypes
import hashlib
import importlib.util
import json
import math
import shlex
import struct
import subprocess
import sys
import time
import traceback

ROOT=Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--out',type=Path,default=ROOT/'build/tmp/a3-facilitator/live')
parser.add_argument('--prepare-only',action='store_true')
args=parser.parse_args()
OUT=args.out.resolve()
if not args.prepare_only and (OUT/'report.json').exists():
    raise SystemExit('Choose a fresh --out directory; retained runs are never overwritten.')
OUT.mkdir(parents=True,exist_ok=True)
sys.path.insert(0,str(ROOT/'tools/harness'))
from acquire_game_foreground import acquire
from lrinput import LRInput
from lrstate import attach,ui_state
from goto_level import loaded_level
from memdump import k32,read_mem,find_pids
from measure_spawn import actor_ptrs
from PIL import ImageChops

def load(name,path):
    spec=importlib.util.spec_from_file_location(name,path)
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    return module
cap=load('fac_capture',ROOT/'build/tmp/a1-zoom-elevation/capture_branches.py')
entry=load('fac_enter',ROOT/'build/tmp/a1-zoom-elevation/enter_fast.py')
entry.WORK=OUT
INSTALL=Path(r'C:\progs\Lemmings Revolution')
CAMERA=0x4cbfc8
CHOOSER=0x46840b
owned=None;handle=None;suspended=False
backups={};log={'writes':[],'samples':[],'visual':[]}
conf=(INSTALL/'dgVoodoo.conf').read_bytes()
before={n:hashlib.sha256((INSTALL/n).read_bytes()).hexdigest()
        for n in ('Lemmings Revolution.exe','SavedGame.dat','dgVoodoo.conf')}

# Compile a static helper so normal Windows Python needs no UCRT64 PATH.
PREDICT=OUT/'predict.exe'
predict_source=OUT/'predict.cpp'
predict_source.write_text('#include "gfx/SpriteView.h"\n#include <cstdlib>\n#include <iomanip>\n#include <iostream>\nint main(int argc,char** argv) {\n  if(argc!=10) return 2;\n  auto d=[&](int i){return std::strtod(argv[i],nullptr);};\n  auto scores=webo::gfx::spriteViewScores({d(1),d(2),d(3)},{d(4),d(5),d(6)},d(7));\n  auto got=webo::gfx::selectSpriteView(scores[0],scores[1],std::strtoul(argv[8],nullptr,10),d(9)!=0);\n  std::cout<<std::setprecision(17)<<"{\\"index\\":"<<got.index<<",\\"mirror\\":"<<int(got.mirrored)\n           <<",\\"scores\\":["<<scores[0]<<\',\'<<scores[1]<<"]}\\n";\n}\n')
quote=lambda p:shlex.quote(p.resolve().as_posix())
command=f'g++ -std=c++23 -static -Isrc src/gfx/SpriteView.cpp {quote(predict_source)} -o {quote(PREDICT)}'
subprocess.run(['powershell','-NoProfile','-File',str(ROOT/'scripts/Invoke-Ucrt64.ps1'),command],cwd=ROOT,check=True)
if args.prepare_only:raise SystemExit(0)
policy=dict(crop='64x64 centered on floored projected actor world origin, clipped to640x480',
            eligibility='first native nonzero-index5-entry lemming; x(110,530),y(80,405), ten seconds after level entry',
            candidate_mae_max=.02,mutation='index0 and mirror flip isolated; error strictly worse and changed pixels nonzero',
            comparison='RGB MAE/255, no alignment or rescale, stock-raster submission replay only',
            eight='controlled graphics-only block ordinal1, temporal0; not natural blocker simulation')
(OUT/'pre-run-policy.json').write_text(json.dumps(policy,indent=2)+'\n')

ntdll=ctypes.WinDLL('ntdll')
for name in ('NtSuspendProcess','NtResumeProcess'):
    getattr(ntdll,name).argtypes=[ctypes.c_void_p]
    getattr(ntdll,name).restype=ctypes.c_long

def save():
    (OUT/'report.json').write_text(json.dumps(log,indent=2)+'\n')
def rd(address,size):
    data=read_mem(handle,address,size)
    if data is None or len(data)!=size:raise RuntimeError(f'bad read {address:#x}/{size}')
    return data
def unpack(address,fmt):return struct.unpack(fmt,rd(address,struct.calcsize(fmt)))
def remember(address,size):backups.setdefault(address,rd(address,size))
def wr(address,data,reason):
    remember(address,len(data))
    log['writes'].append(dict(address=hex(address),before=rd(address,len(data)).hex(),
                              after=data.hex(),reason=reason));save()
    n=ctypes.c_size_t()
    if not k32.WriteProcessMemory(handle,ctypes.c_void_p(address),data,len(data),ctypes.byref(n)) or n.value!=len(data):
        raise RuntimeError(f'bad write {address:#x}')
    if address==CHOOSER:k32.FlushInstructionCache(handle,ctypes.c_void_p(address),len(data))
    assert rd(address,len(data))==data
def polls(count=3):
    with LRInput() as inp:
        inp.enable();inp.clear();inp.wait_frames(count);inp.release()
def shot(name):
    assert ui_state(handle)==1
    polls()
    image,_=cap.shot(cap.find_hwnd(owned.pid))
    assert image.size==(640,480),image.size
    image=image.convert('RGB');image.save(OUT/f'{name}.png')
    return image

class FocusedInput(LRInput):
    def tap(self,*args,**kwargs):
        acquire(owned.pid)
        return super().tap(*args,**kwargs)
entry.LRInput=FocusedInput
old_find=entry.find_menu_index
def find_menu(h,inp):
    acquire(owned.pid);inp._set_u32(32,1);entry.aim_at(h,inp,(620,460))
    return old_find(h,inp)
entry.find_menu_index=find_menu

def row(actor):
    c=actor+0x128
    action_index,frame=unpack(c+0x1c8,'<2i')
    container=unpack(c+0x1f8,'<I')[0]
    action_base=unpack(container+12,'<I')[0]
    action=action_base+action_index*0x98
    temporal_base=unpack(action+12,'<I')[0]
    temporal=temporal_base+frame*16
    count=unpack(temporal,'<I')[0]
    assert count in (1,5,8),(hex(actor),count)
    position=unpack(actor+0x108,'<3d')
    camera=unpack(0x4cc018,'<3d')
    basis=unpack(c+0x1e0,'<3d')
    phase=unpack(c+0x1d8,'<d')[0]
    mirror=rd(c+0x1b4,1)[0]
    index=unpack(c+0x1b8,'<i')[0]
    vector=[p-e for p,e in zip(position,camera)]
    # Independent C++ invocation uses the actual production helper, not worker
    # A3's Python score transcription. Native output remains an independent input.
    args=[*vector,*basis,phase,count,mirror]
    result=subprocess.run([str(PREDICT),*[str(x) for x in args]],
                          text=True,capture_output=True,check=True,creationflags=subprocess.CREATE_NO_WINDOW)
    predicted=json.loads(result.stdout)
    matrix=unpack(0x58d3d8,'<12d')
    transformed=[sum(matrix[k+3*j]*position[j] for j in range(3))+matrix[9+k] for k in range(3)]
    anchor=None if transformed[2]<=0 else [319+transformed[0]/transformed[2],239-transformed[1]/transformed[2]]
    return dict(actor=hex(actor),action=action_index,frame=frame,count=count,
                container=hex(container),action_record=hex(action),temporal_record=hex(temporal),
                position=position,camera=camera,basis=basis,phase=phase,vector=vector,
                native_index=index,native_mirror=mirror,predicted=predicted,anchor=anchor,
                camera_bytes=rd(CAMERA,0x168).hex(),projection_matrix=matrix)
def rows(label):
    result=[]
    for actor in actor_ptrs(handle):
        if actor and unpack(actor,'<I')[0] in (0x4b10e8,0x4b0f6c):result.append(row(actor))
    log['samples'].append(dict(label=label,actors=result));save()
    return result
def error(a,b,rect):
    diff=ImageChops.difference(a.crop(rect),b.crop(rect))
    count=diff.width*diff.height
    return dict(mae=sum(sum(pixel) for pixel in diff.get_flattened_data())/(count*3*255),
                changed=sum(pixel!=(0,0,0) for pixel in diff.get_flattened_data()))
def replay(actor,label):
    r=row(actor);c=actor+0x128
    sx,sy=r['anchor']
    rect=(max(0,math.floor(sx)-32),max(0,math.floor(sy)-32),
          min(640,math.floor(sx)+32),min(480,math.floor(sy)+32))
    assert rect[0]<rect[2] and rect[1]<rect[3]
    record=dict(label=label,input=r,rect=rect)
    log['visual'].append(record);save()
    original=shot(f'{label}-original')
    assert rd(CHOOSER,3)==b'\x55\x8b\xec'
    wr(CHOOSER,b'\xc2\x04\x00','freeze chooser; preserve one-argument callee cleanup')
    p=r['predicted']
    wr(c+0x1b8,struct.pack('<i',p['index']),'computed candidate index')
    wr(c+0x1b4,bytes([p['mirror']]),'computed candidate mirror')
    candidate=shot(f'{label}-candidate')
    record['candidate']=error(original,candidate,rect)
    record['candidate_readback']=[unpack(c+0x1b8,'<i')[0],rd(c+0x1b4,1)[0]]
    wr(c+0x1b8,bytes(4),'isolated constant-zero view mutation')
    forced=shot(f'{label}-constant0')
    record['constant0']=error(original,forced,rect)
    record['constant0_readback']=unpack(c+0x1b8,'<i')[0]
    wr(c+0x1b8,struct.pack('<i',p['index']),'restore candidate index')
    wr(c+0x1b4,bytes([1-p['mirror']]),'isolated mirror mutation')
    mirrored=shot(f'{label}-mirror-flip')
    record['mirror_flip']=error(original,mirrored,rect)
    record['mirror_flip_readback']=rd(c+0x1b4,1)[0]
    wr(c+0x1b4,bytes([r['native_mirror']]),'restore original mirror')
    wr(c+0x1b8,struct.pack('<i',r['native_index']),'restore original index')
    wr(CHOOSER,backups[CHOOSER],'restore native chooser')
    restored=shot(f'{label}-restored')
    record['restored']=error(original,restored,rect)
    record['pass']=(record['candidate']['mae']<=.02 and
                    record['candidate_readback']==[p['index'],p['mirror']] and
                    record['constant0_readback']==0 and
                    record['constant0']['mae']>record['candidate']['mae'] and record['constant0']['changed']>0 and
                    record['mirror_flip_readback']==1-p['mirror'] and
                    record['mirror_flip']['mae']>record['candidate']['mae'] and record['mirror_flip']['changed']>0 and
                    record['restored']['mae']<=.02)
    save()

try:
    assert not find_pids('Lemmings'),'exclusive live ownership required'
    assert before['Lemmings Revolution.exe']=='32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f'
    (INSTALL/'dgVoodoo.conf').write_bytes((ROOT/'build/tmp/a1-zoom-elevation/dgVoodoo.windowed.conf').read_bytes())
    owned=subprocess.Popen([str(INSTALL/'Lemmings Revolution.exe')],cwd=INSTALL)
    log['pid']=owned.pid;save();time.sleep(5)
    assert entry.main()==0
    handle=k32.OpenProcess(0xc38,False,owned.pid)
    assert handle
    assert loaded_level(handle).lower()==r'compiled\mm_climb_002.lvl'
    with LRInput() as inp:
        inp.enable();inp.clear();acquire(owned.pid);inp.tap('P',2)
        assert ui_state(handle)==0
        time.sleep(10)
        acquire(owned.pid);inp.tap('P',2);inp.wait_frames(3);inp.release()
    assert ui_state(handle)==1
    log['loaded_level']=loaded_level(handle)
    remember(CAMERA,0x168)
    baseline=rows('baseline')
    assert baseline
    eligible=[r for r in baseline if r['count']==5 and r['native_index']!=0 and r['anchor'] and 110<r['anchor'][0]<530 and 80<r['anchor'][1]<405]
    assert eligible,'no bounded visible actor'
    actor=int(eligible[0]['actor'],16);c=actor+0x128
    replay(actor,'five')
    # Controlled stock block graphics action only, not a claim of blocker sim.
    remember(c+0x1c8,4);remember(c+0x1cc,4)
    wr(c+0x1c8,struct.pack('<i',1),'controlled block graphics action (SPR ordinal1)')
    wr(c+0x1cc,bytes(4),'controlled temporal entry0')
    polls(4)
    eight=rows('controlled-block-graphics')
    assert row(actor)['count']==8
    replay(actor,'eight')
    wr(c+0x1c8,backups[c+0x1c8],'restore original graphics action')
    wr(c+0x1cc,backups[c+0x1cc],'restore original temporal index')
    polls()
    for step in range(12):
        with LRInput() as inp:
            inp.enable();inp.clear();acquire(owned.pid)
            inp.button(1,True);inp.wait_frames(2);inp.key(0x4b,True)
            inp.wait_frames(4);inp.key(0x4b,False);inp.wait_frames(2)
            inp.button(1,False);inp.wait_frames(2);inp.release()
        rows(f'yaw-{step+1}')
    log['native_matches']=all(
        (a['native_index'],a['native_mirror'])==(a['predicted']['index'],a['predicted']['mirror'])
        for sample in log['samples'] for a in sample['actors'])
    initial={a['actor']:a for a in baseline}
    yaw=[a for sample in log['samples'] if sample['label'].startswith('yaw-') for a in sample['actors']]
    log['actor_inputs_frozen']=all(
        all(a[k]==initial[a['actor']][k] for k in ('position','basis','phase','action','frame')) for a in yaw)
    log['camera_sector_crossing']=any(
        (a['native_index'],a['native_mirror'])!=(initial[a['actor']]['native_index'],initial[a['actor']]['native_mirror']) for a in yaw)
    log['success']=(log['native_matches'] and log['actor_inputs_frozen'] and log['camera_sector_crossing'] and
                    all(v['pass'] for v in log['visual']))
except Exception:
    log['error']=traceback.format_exc()
finally:
    if handle:
        try:
            status=ntdll.NtSuspendProcess(handle);log['suspend_status']=status
            suspended=status==0
            assert suspended
            for address,data in reversed(list(backups.items())):
                wr(address,data,'final canonical-backup restoration while suspended')
            log['restored']={hex(address):rd(address,len(data))==data for address,data in backups.items()}
            log['all_restored']=all(log['restored'].values())
        except Exception:
            log['cleanup_error']=traceback.format_exc()
        if suspended:log['resume_status']=ntdll.NtResumeProcess(handle)
        k32.CloseHandle(handle)
    try:
        with LRInput() as inp:inp.release()
    except Exception:pass
    if owned:
        owned.terminate();owned.wait(timeout=10);log['process_stopped']=owned.poll() is not None
    (INSTALL/'dgVoodoo.conf').write_bytes(conf)
    log['install_before']=before
    log['install_after']={n:hashlib.sha256((INSTALL/n).read_bytes()).hexdigest() for n in before}
    log['install_restored']=log['install_after']==before
    save()
    print(json.dumps({k:log.get(k) for k in ('success','error','cleanup_error','all_restored','install_restored','process_stopped')}))

raise SystemExit(0 if log.get('success') and log.get('all_restored') and log.get('install_restored') and log.get('process_stopped') else 1)
