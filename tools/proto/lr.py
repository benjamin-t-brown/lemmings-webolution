import pefile, capstone, sys, re
PATH=r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe"
pe=pefile.PE(PATH, fast_load=True)
pe.parse_data_directories()
IB=pe.OPTIONAL_HEADER.ImageBase
data=open(PATH,'rb').read()
secs=[(s.Name.rstrip(b'\0').decode(),s.VirtualAddress,s.Misc_VirtualSize,s.PointerToRawData,s.SizeOfRawData) for s in pe.sections]
def v2r(v):
    r=v-IB
    for n,va,vs,pr,rs in secs:
        if va<=r<va+max(vs,rs): return pr+(r-va)
    return None
def r2v(off):
    for n,va,vs,pr,rs in secs:
        if pr<=off<pr+rs: return IB+va+(off-pr)
    return None
TEXT=[s for s in secs if s[0]=='.text'][0]
md=capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
md.detail=True
# import thunk map
imps={}
for e in pe.DIRECTORY_ENTRY_IMPORT:
    dll=e.dll.decode()
    for i in e.imports:
        imps[i.address]= dll.split('.')[0]+"!"+(i.name.decode() if i.name else "ord%d"%i.ordinal)
def dis(va, n=60):
    off=v2r(va)
    out=[]
    for ins in md.disasm(data[off:off+n*8], va):
        s="%08x  %-8s %s"%(ins.address, ins.mnemonic, ins.op_str)
        m=re.search(r'dword ptr \[(0x[0-9a-f]+)\]', ins.op_str)
        if m and int(m.group(1),16) in imps: s+="   ; "+imps[int(m.group(1),16)]
        out.append(s)
        if len(out)>=n: break
    return "\n".join(out)
if __name__=='__main__':
    pass

def sweep(sec=None):
    """linear sweep with resync; yields instructions"""
    n,va,vs,pr,rs = sec or TEXT
    b=IB+va; code=data[pr:pr+rs]
    off=0
    while off < len(code):
        got=False
        for ins in md.disasm(code[off:], b+off):
            got=True
            yield ins
            off = ins.address - b + ins.size
        if not got:
            off += 1

import pickle, os
IDXF='idx.pkl'
def build_index():
    calls={}   # target -> [sites]
    allins={}  # addr -> (mnem,ops,size)
    for ins in sweep():
        allins[ins.address]=(ins.mnemonic,ins.op_str,ins.size)
        if ins.mnemonic in ('call','jmp') and ins.op_str.startswith('0x'):
            t=int(ins.op_str,16)
            calls.setdefault(t,[]).append(ins.address)
    return calls, allins
def load_index():
    if os.path.exists(IDXF):
        return pickle.load(open(IDXF,'rb'))
    i=build_index(); pickle.dump(i,open(IDXF,'wb')); return i

import bisect
_calls=_allins=None
def idx():
    global _calls,_allins,_addrs
    if _calls is None:
        globals()['_calls'],globals()['_allins']=load_index()
        globals()['_addrs']=sorted(_allins)
    return _calls,_allins,_addrs
def fstart(a):
    c,ai,ad=idx()
    i=bisect.bisect_right(ad,a)-1
    while i>0:
        x=ad[i]; m,o,sz=ai[x]
        if m=='push' and o=='ebp' and i+1<len(ad):
            m2,o2,_=ai[ad[i+1]]
            if m2=='mov' and o2=='ebp, esp': return x
        i-=1
    return None
def xrefs_data(val):
    """find instructions referencing immediate/absolute value val"""
    c,ai,ad=idx()
    h='0x%x'%val
    return [(a,)+ai[a] for a in ad if h in ai[a][1]]

def show(a,b):
    c,ai,ad=idx()
    import bisect, re
    i=bisect.bisect_left(ad,a)
    while i<len(ad) and ad[i]<b:
        x=ad[i]; m,o,sz=ai[x]
        s="%08x  %-8s %s"%(x,m,o)
        mm=re.search(r'\[(0x[0-9a-f]+)\]',o)
        if mm and int(mm.group(1),16) in imps: s+="   ; "+imps[int(mm.group(1),16)]
        print(s); i+=1
