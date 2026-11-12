"""Execute original animation playback and wrappers with resource acquisition adapters."""
import argparse
import hashlib
from pathlib import Path
import struct
import subprocess
import sys
from verify_reference import EXE_SHA256, FUNCTIONS, pe_file_offset

ADDRESSES=(0x468833,0x468AB8,0x468BA3,0x468B5D,0x468B04,0x468BD9,
           0x46836D,0x468D00,0x45B8F0,0x4840F3,0x484150,0x41A4E0,
           0x403720,0x49C870,0x43CF30,0x43CF53,0x43CF72,0x43CFAF,
           0x43CF17,0x43CEFE,0x43CFC8,0x43D030)
CONSTANTS=((0x4B1940,'b81e85eb51b8ae3f'),(0x4B1928,'000000e08e79453e'))

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe',type=Path,required=True)
    parser.add_argument('--program',type=Path,required=True)
    args=parser.parse_args()
    data=args.exe.read_bytes()
    if hashlib.sha256(data).hexdigest()!=EXE_SHA256:parser.error('reference digest mismatch')
    blocks=[]
    for address in ADDRESSES:
        _,offset,size,digest=next(row for row in FUNCTIONS if row[0]==address)
        if offset!=pe_file_offset(data,address,size):parser.error('mapping mismatch')
        body=data[offset:offset+size]
        if hashlib.sha256(body).hexdigest()!=digest:parser.error('code digest mismatch')
        blocks.append(struct.pack('<II',address,size)+body)
    for address,expected in CONSTANTS:
        offset=pe_file_offset(data,address,8)
        body=data[offset:offset+8]
        if body.hex()!=expected:parser.error('constant mismatch')
        blocks.append(struct.pack('<II',address,8)+body)
    payload=struct.pack('<I',len(blocks))+b''.join(blocks)
    for _ in range(8):
        result=subprocess.run([str(args.program)],input=payload,capture_output=True,timeout=30)
        if result.returncode!=3:
            sys.stdout.buffer.write(result.stdout)
            sys.stderr.buffer.write(result.stderr)
            return result.returncode
    sys.stderr.buffer.write(result.stderr)
    return result.returncode

if __name__=='__main__':raise SystemExit(main())
