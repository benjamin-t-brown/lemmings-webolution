"""Compare all installed packed texture codec blocks against unchanged original x86."""
import argparse
import hashlib
from pathlib import Path
import struct
import subprocess
import sys
import random
from verify_reference import EXE_SHA256, FUNCTIONS, pe_file_offset
from inspect_box import read_index

PINS = (
    (0x00467DE8, 425448, 43, "34f671313b4a7dcde9baa8a7ad2a2008b71c8dfb209d5ad8bd6e174e8cb01e1a"),
    (0x00467E13, 425491, 164, "bb5ef855690781789e22a33ecac2ddf138cab2277712a71c011d4871f5d6100a"),
    (0x00467EB7, 425655, 310, "8a1a0687aaf784e8b63a9200c29e20e43aa6e90219a9b99a94f1b1a8611ce814"),
    (0x00467FED, 425965, 135, "1e2f2fa1197d5e2696c18c0a56708fe0d0f8dee237f8e45ec367d03234fecfc9"),
    (0x0045015A, 328026, 845, "6bf8c822c659e2767194d6a67b6f73c146cb3cbe2de3dd18626ea3d62a98953f"),
)
ADDRESSES = tuple(row[0] for row in PINS)
CONSTANTS = ()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe', type=Path, required=True)
    parser.add_argument('--program', type=Path, required=True)
    parser.add_argument('archives', type=Path, nargs='*')
    args = parser.parse_args()
    if not args.archives:
        args.archives = [args.exe.parent / 'PATCH.BOX', args.exe.parent / 'LEMMINGS.BOX']
    data = args.exe.read_bytes()
    if hashlib.sha256(data).hexdigest() != EXE_SHA256: parser.error('reference digest mismatch')
    pins = {row[0]: row for row in (*FUNCTIONS, *PINS)}
    blocks = []
    for address in ADDRESSES:
        _, offset, size, digest = pins[address]
        if offset != pe_file_offset(data, address, size): parser.error('PE mapping mismatch')
        body = data[offset:offset + size]
        if hashlib.sha256(body).hexdigest() != digest: parser.error('code digest mismatch')
        blocks.append(struct.pack('<II', address, size) + body)
    for address, expected in CONSTANTS:
        offset = pe_file_offset(data, address, len(expected))
        body = data[offset:offset + len(expected)]
        if body != expected: parser.error('constant mismatch')
        blocks.append(struct.pack('<II', address, len(body)) + body)
    payload = struct.pack('<I', len(blocks)) + b''.join(blocks)
    cases=[]
    for archive in args.archives:
        with archive.open('rb') as stream:
            for entry in read_index(archive):
                if not entry.name.startswith('TEXTURES\\') or not entry.name.endswith('.BMP'): continue
                stream.seek(entry.offset); packed=stream.read(entry.size)
                cases.append(struct.pack('<III',1,len(packed),0)+packed)
                if packed[:2]==b'BM': continue
                cursor=1
                for _ in range(packed[0]):
                    size,output,codec=struct.unpack_from('<HHB',packed,cursor)
                    if codec==4:
                        content=packed[cursor+5:cursor+size]
                        cases.append(struct.pack('<III',0,len(content),output)+content)
                    elif codec!=3: parser.error('unexpected texture codec')
                    cursor+=size
    for content in (b'BM\0\1\2\3',bytes([0,255]),bytes([2,7,0,2,0,3,66,77,9,0,4,0,3,0,1,2,3,255])):
        cases.append(struct.pack('<III',1,len(content),0)+content)
    for length in range(1,257):
        value=length+1;k=value.bit_length()-1
        bits='1'*k+'0'+format(value-(1<<k),f'0{k}b') if k else '0'
        bits+=''.join(format((i*73+length)&255,'08b')for i in range(length))
        bits+='0'*((-len(bits))%8)
        content=b'\0\0\0'+bytes(int(bits[i:i+8],2)for i in range(0,len(bits),8))
        cases.append(struct.pack('<III',0,len(content),length)+content)
    randomizer=random.Random(0x467EB7)
    for symbols in range(3,18):
        for _ in range(4):
            lengths=[1,1]
            while len(lengths)<symbols:
                candidates=[i for i,value in enumerate(lengths)if value<15]
                chosen=randomizer.choice(candidates);depth=lengths.pop(chosen)+1
                lengths.extend((depth,depth))
            randomizer.shuffle(lengths)
            table=bytearray([(lengths[0]<<4)|(symbols-2)])
            for i in range(1,symbols,2):
                table.append(lengths[i]|((lengths[i+1]if i+1<symbols else 0)<<4))
            code=0;codes={}
            for width in range(1,max(lengths)+1):
                for symbol,depth in enumerate(lengths):
                    if depth==width:codes[symbol]=format(code,f'0{width}b');code+=1
                code<<=1
            for symbol in range(1,min(symbols,9)):
                length=1 if symbol==1 else (1<<(symbol-1))+randomizer.randrange(1<<(symbol-1))
                bits=codes[symbol]
                if symbol>=2:bits+=format(length-(1<<(symbol-1)),f'0{symbol-1}b')
                bits+=''.join(format((i*17+symbols)&255,'08b')for i in range(length))
                bits+='0'*((-len(bits))%8)
                content=bytes(table)*3+bytes(int(bits[i:i+8],2)for i in range(0,len(bits),8))
                cases.append(struct.pack('<III',0,len(content),length)+content)
    for length in range(2,258):
        def unary(value):
            value+=1;width=value.bit_length()-1
            return '1'*width+'0'+(format(value-(1<<width),f'0{width}b')if width else '')
        bits=unary(1)+'10100101'+unary(length-2)+unary(0)
        bits+='0'*((-len(bits))%8)
        content=b'\0\0\0'+bytes(int(bits[i:i+8],2)for i in range(0,len(bits),8))
        cases.append(struct.pack('<III',0,len(content),length+1)+content)
    payload += struct.pack('<I',len(cases))+b''.join(cases)

    for _ in range(8):
        result = subprocess.run([str(args.program)], input=payload, capture_output=True, timeout=120)
        if result.returncode != 3:
            sys.stdout.buffer.write(result.stdout)
            sys.stderr.buffer.write(result.stderr)
            return result.returncode
    return result.returncode


if __name__ == '__main__': raise SystemExit(main())
