"""Replay the original x87 comparison/store stage, not a transcription of its cases.

The orientation scores are explicit inputs, so this does NOT validate their
geometric producer. Instructions/branches come from initialized stock PE bytes.
Unsupported instructions fail closed. Finite values only in this bounded oracle.
"""
from pathlib import Path
import argparse
import csv
import hashlib
import json
import math
import shlex
import struct
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'tools/harness'))
from verify_tube_a2 import raw_reader
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
from capstone.x86 import X86_OP_REG,X86_OP_IMM,X86_OP_MEM

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--exe',type=Path,default=Path(r'C:\progs\Lemmings Revolution\Lemmings Revolution.exe'))
parser.add_argument('--out',type=Path,default=ROOT/'build/tmp/a3-facilitator')
parser.add_argument('--spr-dir',type=Path,help='Freshly extracted six LEMMINGS*.SPR files')
parser.add_argument('--check-cpp',action='store_true',help='Compile selector and compare with raw-instruction oracle')
parser.add_argument('--live-dir',type=Path,help='Audit a facilitator stock-raster replay report and screenshots offline')
args=parser.parse_args()
_,raw=raw_reader(args.exe)
section,offset,code=raw(0x46840b,0x426)
md=Cs(CS_ARCH_X86,CS_MODE_32);md.detail=True
instructions={i.address:i for i in md.disasm(code,0x46840b)}

def inventory(directory):
    """Independent parser; names describe layout axes, not legacy C++ fields."""
    report={}
    paths=sorted(directory.glob('LEMMINGS*.SPR'))
    if len(paths)!=6:
        raise ValueError('expected exactly the six LEMMINGS*.SPR assets')
    for path in paths:
        data=path.read_bytes();at=0
        def u32():
            nonlocal at
            value,=struct.unpack_from('<I',data,at);at+=4
            return value
        def string():
            nonlocal at
            size=u32();value=data[at:at+size].decode('ascii');at+=size
            return value
        actions=[]
        for _ in range(u32()):
            entries=[]
            for _ in range(u32()):
                directions=[]
                for _ in range(u32()):
                    start=at
                    bottom,top,left,right=struct.unpack_from('<4I',data,at)
                    at+=16+32
                    anchor0,anchor1,width,height=struct.unpack_from('<4I',data,at)
                    at+=16;sheet=string()
                    assert width==right-left and height==bottom-top
                    directions.append(dict(offset=start,rect=[left,top,right,bottom],
                                           unknown=[anchor0,anchor1],sheet=sheet))
                entries.append(directions)
            name=string();rate,=struct.unpack_from('<d',data,at);at+=8
            actions.append(dict(name=name,temporal_count=len(entries),
                                directional_counts=[len(e) for e in entries],
                                first_entry=entries[0],rate=rate))
        palettes=u32();palette_start=at;at+=palettes*(4+48*8)
        assert at==len(data),(path.name,at,len(data))
        report[path.name]=dict(sha256=hashlib.sha256(data).hexdigest(),bytes=len(data),
                               actions=actions,palette_count=palettes,
                               palette_sha256=hashlib.sha256(data[palette_start:]).hexdigest())
    return report

def choose(x,y,count,mirror):
    assert math.isfinite(x) and math.isfinite(y)
    regs={'eax':count,'ebp':0x2000,'ecx':0,'edx':0}
    local={-0x78:0x1000,-4:0x3000,-0x54:x,-0x5c:y}
    out={'index':None,'mirror':mirror}
    fp=[];status=0;zero=False;cmp_result=0
    pc=0x46844a

    def value(ins,op):
        if op.type==X86_OP_IMM:return op.imm
        if op.type==X86_OP_REG:
            name=ins.reg_name(op.reg)
            if name=='ah':return (regs['eax']>>8)&255
            return regs[name]
        assert op.type==X86_OP_MEM
        base=ins.reg_name(op.mem.base) if op.mem.base else None
        assert not op.mem.index
        if base=='ebp':return local[op.mem.disp]
        if base is None:
            return struct.unpack('<d',raw(op.mem.disp,8)[2])[0]
        raise AssertionError(('unsupported memory read',ins.address,ins.op_str))

    for _ in range(200):
        if pc==0x468453:pc=0x468616 # score producer is deliberately outside scope
        if pc==0x46882b:
            assert out['index'] is not None
            return out['index'],bool(out['mirror'])
        ins=instructions[pc];op=ins.operands;next_pc=pc+ins.size
        if ins.mnemonic=='mov':
            v=value(ins,op[1])
            if op[0].type==X86_OP_REG:regs[ins.reg_name(op[0].reg)]=v
            else:
                address=regs[ins.reg_name(op[0].mem.base)]+op[0].mem.disp
                if address==0x11b8:out['index']=v
                elif address==0x11b4:out['mirror']=v
                else:raise AssertionError(('unsupported store',hex(pc),hex(address)))
        elif ins.mnemonic=='call':
            assert op[0].imm==0x468cd0
            regs['eax']=count
        elif ins.mnemonic=='cmp':
            cmp_result=value(ins,op[0])-value(ins,op[1]);zero=cmp_result==0
        elif ins.mnemonic=='fld':fp.insert(0,float(value(ins,op[0])))
        elif ins.mnemonic=='fchs':fp[0]=-fp[0]
        elif ins.mnemonic in ('fcomp','fcompp'):
            a=fp[0];b=fp[1] if ins.mnemonic=='fcompp' else value(ins,op[0])
            status=0x0100 if a<b else (0x4000 if a==b else 0)
            del fp[:2 if ins.mnemonic=='fcompp' else 1]
        elif ins.mnemonic=='fnstsw':regs['eax']=(regs['eax']&~0xffff)|status
        elif ins.mnemonic=='test':zero=(value(ins,op[0])&value(ins,op[1]))==0
        elif ins.mnemonic in ('jmp','je','jne','jle'):
            take={'jmp':True,'je':zero,'jne':not zero,'jle':cmp_result<=0}[ins.mnemonic]
            if take:next_pc=op[0].imm
        else:raise AssertionError(('unsupported instruction',hex(pc),ins.mnemonic,ins.op_str))
        pc=next_pc
    raise AssertionError('comparison stage did not terminate')

def verify_live(directory):
    from PIL import Image,ImageChops
    live=json.loads((directory/'report.json').read_text())
    assert live.get('success') and live.get('all_restored') and live.get('install_restored') and live.get('process_stopped')
    assert live['install_before']==live['install_after']
    cases=[];score_error=0.;counts={};crossings=[];previous={};frozen={}
    for sample in live['samples']:
        for actor in sample['actors']:
            vector=[p-e for p,e in zip(actor['position'],actor['camera'])]
            length=math.sqrt(sum(v*v for v in vector))
            if length:vector=[v/length for v in vector]
            bx,by,bz=actor['basis'];first=actor['phase']+.0625;scores=[]
            for phase in (first,first+.1875):
                angle=(phase+phase)*3.1415927410125732
                sine,cosine=math.sin(angle),math.cos(angle)
                basis=(cosine*bx-sine*bz,by,sine*bx+cosine*bz)
                value=sum(v*b for v,b in zip(vector,basis))
                scores.append(value+2 if value < -1 else value-2 if value>1 else value)
            predicted=choose(*scores,actor['count'],actor['native_mirror'])
            actual=(actor['native_index'],bool(actor['native_mirror']))
            assert predicted==actual,(sample['label'],actor['actor'],predicted,actual)
            cpp=actor['predicted']
            assert (cpp['index'],bool(cpp['mirror']))==actual
            score_error=max(score_error,*[abs(a-b) for a,b in zip(scores,cpp['scores'])])
            counts[actor['count']]=counts.get(actor['count'],0)+1
            cases.append(dict(sample=sample['label'],actor=actor['actor'],count=actor['count'],index=actual[0],mirror=actual[1]))
            if sample['label']=='baseline' or sample['label'].startswith('yaw-'):
                inputs=[actor[k] for k in ('position','basis','phase','action','frame')]
                if sample['label']=='baseline':frozen[actor['actor']]=inputs
                else:assert inputs==frozen[actor['actor']],(sample['label'],'actor inputs changed')
                if actor['actor'] in previous and previous[actor['actor']]!=actual:
                    crossings.append(dict(sample=sample['label'],actor=actor['actor'],before=previous[actor['actor']],after=actual))
                previous[actor['actor']]=actual
    assert score_error<1e-12,score_error
    assert set(counts)=={5,8} and crossings
    visual=[]
    for fixture in live['visual']:
        name=fixture['label'];rect=fixture['rect'];metrics={}
        original=Image.open(directory/f'{name}-original.png').convert('RGB')
        for suffix in ('candidate','constant0','mirror-flip','restored'):
            other=Image.open(directory/f'{name}-{suffix}.png').convert('RGB')
            diff=ImageChops.difference(original.crop(rect),other.crop(rect))
            pixels=list(diff.get_flattened_data())
            metrics[suffix]=dict(mae=sum(sum(p) for p in pixels)/(len(pixels)*3*255),
                                 changed=sum(p!=(0,0,0) for p in pixels))
        assert metrics['candidate']['mae']<=.02
        assert metrics['constant0']['mae']>metrics['candidate']['mae'] and metrics['constant0']['changed']>0
        assert metrics['mirror-flip']['mae']>metrics['candidate']['mae'] and metrics['mirror-flip']['changed']>0
        assert metrics['restored']['mae']<=.02
        visual.append(dict(fixture=name,rect=rect,metrics=metrics,
                           strict_worker_sensitivity_pass=metrics['constant0']['mae']>.02))
    return dict(scope='native actor selection and shared-stock-raster replay; not replacement raster fidelity',
                rows=len(cases),counts=counts,max_cpp_score_error=score_error,crossings=crossings,visual=visual,
                actor_inputs_frozen_across_yaw=True,
                all_restored=live['all_restored'],install_restored=live['install_restored'],process_stopped=live['process_stopped'])

if __name__=='__main__':
    here=args.out
    here.mkdir(parents=True,exist_ok=True)
    values=[-2.,-1.,-1e-12,-0.,0.,1e-12,1.,2.]
    pairs=[(x,y) for x in values for y in values]
    for sx in (-1,1):
        for sy in (-1,1):
            for delta in (-1e-12,0,1e-12):pairs.append((sx*(1+delta),sy*1.))
    rows=[]
    for x,y in pairs:
        for count in (0,1,5,8):
            for prior in (0,1):
                index,mirror=choose(x,y,count,prior)
                rows.append((x,y,count,prior,index,int(mirror)))
    with (here/'native-oracle.csv').open('w',newline='') as f:
        writer=csv.writer(f);writer.writerows(rows)
    report={'scope':'raw comparison/store stage only; finite explicit scores; no geometric-producer or live claim',
            'code_start':'0x46840b','code_bytes':len(code),'section':section,'file_offset':hex(offset),
            'code_sha256':hashlib.sha256(code).hexdigest(),'rows':len(rows),
            'emulated_entry':'0x46844a','score_producer_skip':['0x468453','0x468616'],
            'emulated_end':'0x46882b'}
    (here/'native-oracle.json').write_text(json.dumps(report,indent=2)+'\n')
    # Every range is checked against section rawsize by raw_reader, including
    # constants. Disassembly is evidence to inspect, not decompiler prose.
    ranges=[(0x46840b,0x426),(0x42eef2,0xeb),(0x4683c5,0x44),
            (0x426d90,0x3e),(0x49cb84,0x22),(0x49cc34,0x22),
            (0x468cd0,0x4e),(0x4017f0,0x9e),(0x418350,0x33),
            (0x480ac8,0x6c),(0x42dcb0,0x13),(0x412c20,0x10),
            (0x468833,0x17b),(0x45b8f0,0x10),
            (0x4b1940,24),(0x4b0f88,8)]
    audit=[]
    for address,length in ranges:
        sec,off,data=raw(address,length)
        audit.append(dict(address=hex(address),section=sec,file_offset=hex(off),
                          bytes=data.hex(),sha256=hashlib.sha256(data).hexdigest(),
                          disassembly=[f'{i.address:08x} {i.bytes.hex()} {i.mnemonic} {i.op_str}'
                                       for i in md.disasm(data,address)] if sec=='.text' else []))
    (here/'raw-address-audit.json').write_text(json.dumps(audit,indent=2)+'\n')
    if args.spr_dir:
        result=inventory(args.spr_dir)
        (here/'spr-inventory.json').write_text(json.dumps(result,indent=2)+'\n')
        report['spr_files']=len(result)
    if args.live_dir:
        result=verify_live(args.live_dir)
        (here/'live-check.json').write_text(json.dumps(result,indent=2)+'\n')
        report['live_check']=result
    if args.check_cpp:
        driver=here/'check-selector.cpp';binary=here/'check-selector.exe'
        driver.write_text(r'''#include "gfx/SpriteView.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
int main(int argc,char** argv) {
  if(argc!=2) return 2;
  std::ifstream in(argv[1]); if(!in) return 2;
  std::string line; int rows=0,errors=0;
  while(std::getline(in,line)) {
    for(char& c:line) if(c==',') c=' ';
    std::istringstream row(line); double x,y;
    std::size_t count,index; int prior,mirror;
    if(!(row>>x>>y>>count>>prior>>index>>mirror)) return 3;
    auto got=webo::gfx::selectSpriteView(x,y,count,prior!=0); ++rows;
    if(got.index!=index || got.mirrored!=(mirror!=0)) {
      ++errors;
      if(errors<10) std::cerr<<line<<" got "<<got.index<<' '<<got.mirrored<<'\n';
    }
  }
  std::cout<<"native instruction-oracle rows "<<rows<<", mismatches "<<errors<<'\n';
  return errors?1:0;
}
''')
        quote=lambda p:shlex.quote(p.resolve().as_posix())
        commands=[f'g++ -std=c++23 -Isrc src/gfx/SpriteView.cpp {quote(driver)} -o {quote(binary)}',
                  f'{quote(binary)} {quote(here/"native-oracle.csv")}']
        outputs=[]
        for command in commands:
            completed=subprocess.run(['powershell','-NoProfile','-File',str(ROOT/'scripts/Invoke-Ucrt64.ps1'),command],
                                     cwd=ROOT,text=True,capture_output=True,timeout=120)
            outputs.append(dict(command=command,exit=completed.returncode,
                                stdout=completed.stdout,stderr=completed.stderr))
            if completed.returncode:
                (here/'cpp-check.json').write_text(json.dumps(outputs,indent=2)+'\n')
                raise SystemExit(completed.stdout+completed.stderr)
        (here/'cpp-check.json').write_text(json.dumps(outputs,indent=2)+'\n')
        report['cpp_check']=outputs[-1]['stdout'].strip()
    print(json.dumps(report))
