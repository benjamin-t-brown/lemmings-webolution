#!/usr/bin/env python3
"""Offline glyph-table/sheet predicate matrix. No fitting or resizing."""
import argparse, hashlib, json, struct
from pathlib import Path
from PIL import Image

TABLES = [("^Font", 0x4BF968, 0x20), ("_FontSmall", 0x4BFC10, 0x30)]
VA_BASE = 0x400000

def read_tables(exe):
    b = Path(exe).read_bytes()
    pe = struct.unpack_from('<I', b, 0x3c)[0]
    n = struct.unpack_from('<H', b, pe+6)[0]
    opt = pe + 24
    image_base = struct.unpack_from('<I', b, opt+28)[0]
    sec = opt + struct.unpack_from('<H', b, pe+20)[0]
    sections=[]
    for i in range(n):
        o=sec+40*i; name=b[o:o+8].split(b'\0')[0].decode('ascii','replace')
        vs,va,rs,rp=struct.unpack_from('<IIII',b,o+8)
        sections.append((name,image_base+va,rs,rp))
    def take(va,size):
        for name,start,rs,rp in sections:
            if start <= va and va+size <= start+rs:
                off=rp+va-start
                return b[off:off+size], {'section':name,'file_offset':off,'size':size}
        raise ValueError(f'VA range {va:#x}+{size:#x} not fully in section raw data')
    out=[]
    for name,va,first in TABLES:
        raw,where=take(va,4+256*3)
        h=struct.unpack_from('<I',raw)[0]
        records=[list(raw[4+i*3:7+i*3]) for i in range(256)]
        # ^Font's asserted 772-byte span overlaps _FontSmall at 0x4bfc10;
        # only records wholly before that address are attributable to ^Font.
        supported=256
        if name=='^Font': supported=max(0,(0x4BFC10-(va+4))//3)
        out.append(dict(name=name,va=hex(va),line_height=h,first_char=first,
                        raw_sha256=hashlib.sha256(raw).hexdigest(),raw=raw.hex(),
                        location=where,records=records,supported_records=supported,
                        unsupported_reason=(f'records {supported}..255 overlap/abut known _FontSmall table bytes; not treated as glyphs' if supported<256 else None)))
    return out

def ink(im, x,y,w,h):
    if x<0 or y<0 or x+w>im.width or y+h>im.height: return None
    crop=im.crop((x,y,x+w,y+h)).convert('RGBA')
    return any(a and (r,g,b)!=(0,0,0) for r,g,b,a in crop.getdata())

def check_cell(im, rec, height, code, first):
    x,y,w=rec; cell={'code':code,'char':repr(chr(code)),'x':x,'y':y,'width':w,'height':height}
    if w==0:
        cell.update(bounds=True,ink=None,reason='zero-width: no area; explicit blank/advance-zero entry')
        return cell
    present=ink(im,x,y,w,height)
    cell.update(bounds=present is not None,ink=present,reason=None if present is not None else 'rectangle out of bounds')
    return cell

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--exe',required=True); ap.add_argument('--exports',required=True); ap.add_argument('--out',required=True)
    a=ap.parse_args(); tables=read_tables(a.exe); root=Path(a.exports)
    manifest=json.loads((root/'manifest.json').read_text(encoding='utf8'))
    sheets=[]
    for row in manifest['assets']:
        if row.get('status')!='exported' or 'FONT' not in row['archive_name'].upper(): continue
        p=root/row['output']; im=Image.open(p).convert('RGBA')
        sheets.append({'archive_name':row['archive_name'],'image':row['output'],'image_sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'width':im.width,'height':im.height,'tables':[]})
        for t in tables:
            tests=[]; failures=[]
            for i,rec in enumerate(t['records'][:t['supported_records']]):
                code=(t['first_char']+i)&255
                c=check_cell(im,rec,t['line_height'],code,t['first_char']); tests.append(c)
                if not c['bounds']: failures.append(f'cell {code:#04x}: {c["reason"]}')
                # Blank rules: & and space must have zero width, nonzero glyph test chars ink.
                if t['name']=='^Font' and code==0x26 and rec[2]!=0: failures.append(f'& expected zero width, got {rec[2]}')
                if t['name']=='^Font' and code==0x20 and rec[2]>0 and c['ink'] is not False: failures.append('space expected blank support')
                if code in (ord('Q'),ord('M'),*range(ord('0'),ord('9')+1)) and rec[2]>0 and c['ink'] is not True:
                    failures.append(f'{code:#04x} expected ink')
            sheets[-1]['tables'].append({'table':t['name'],'cells':tests,'failures':failures,'pass':not failures})
    per_table={t['name']:('unique' if sum(next(q['pass'] for q in s['tables'] if q['table']==t['name']) for s in sheets)==1 else ('ambiguous' if any(next(q['pass'] for q in s['tables'] if q['table']==t['name']) for s in sheets) else 'negative')) for t in tables}
    result={'verdict':'not-combinable; see per_table_verdict','per_table_verdict':per_table,'tables':[{k:v for k,v in t.items() if k!='records'} for t in tables],'inventory_source':'export manifest supplied by caller; this script does not prove archive completeness or patch precedence','sheets':sheets}
    Path(a.out).write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
    print(result['verdict'])
if __name__=='__main__': main()
