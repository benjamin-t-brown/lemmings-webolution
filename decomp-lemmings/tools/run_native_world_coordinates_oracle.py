"""Compare unchanged world coordinate routines with recovered interpolation."""
import argparse
import hashlib
from pathlib import Path
import struct
import subprocess
import sys
from verify_reference import EXE_SHA256, FUNCTIONS, pe_file_offset

ADDRESSES=(0x43D56C,0x43DA38,0x43E580,0x43EA20,0x4027C0,0x402680,
           0x401830,0x4017F0,0x403EB0,0x49C5AC,0x40DF30,0x49C8F4,0x49FC56)
CONSTANTS=((0x4B07F0,'0000000000002040'),(0x4B08F8,'0000000000003040'))

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe',type=Path,required=True)
    parser.add_argument('--program',type=Path,required=True)
    parser.add_argument('--archive',type=Path,nargs='*',default=[])
    args=parser.parse_args()
    data=args.exe.read_bytes()
    if hashlib.sha256(data).hexdigest()!=EXE_SHA256:
        parser.error('reference digest mismatch')
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
        result=subprocess.run([str(args.program),*[str(p) for p in args.archive]],
                              input=payload,capture_output=True,timeout=60)
        if result.returncode!=3:
            sys.stdout.buffer.write(result.stdout)
            sys.stderr.buffer.write(result.stderr)
            return result.returncode
    sys.stderr.buffer.write(result.stderr)
    return result.returncode

if __name__=='__main__':raise SystemExit(main())
