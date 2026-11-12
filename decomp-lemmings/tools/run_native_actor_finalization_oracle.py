"""Execute unchanged actor-update tail and sample lookup with explicit model/effect adapters."""
import argparse
import hashlib
from pathlib import Path
import struct
import subprocess
import sys
from verify_reference import EXE_SHA256, FUNCTIONS, pe_file_offset

PINS = (
    (0x004658D6, 415958, 42, "cac3984f894e28afcd554c5060d5a84fc2cb4cbfb4cb47bedbe9809714037236"),
    (0x00465900, 416000, 36, "08c93f4542f307e2bc33864d37d92f196711c9c1a7f1786ac782ba6a9d9f61fc"),
    (0x00403F40, 16192, 83, "ea2146855495c43c0f52a09e87c2b3414198596a1c8ad0597a6585d19b8f7fbf"),
    (0x00403FA0, 16288, 22, "38b9321c0f2f58e27d286e746e4e6e727ed39abdd983cad0ed8fb86c4aa65569"),
    (0x00403FC0, 16320, 29, "f5f6290fedae9b78ff635315f8fefd8b67873dc29303f01d83cdf79fc55ea51e"),
    (0x00403820, 14368, 108, "1c1917eab1053ab387b6cec82ed61fbdd2d713b350ea0e49bd861f0b5ed02e95"),
    (0x0041A500, 107776, 20, "83773417e7a4f1befacf0638a811f8a5b3bbeb9eeaf20d092ce68f2d0298da02"),
)
ADDRESSES = (0x42E2BF,0x430170,0x430190,0x41A500,0x41A4E0,0x403720,
             0x49C870,0x49C5AC,0x403F40,0x403FA0,0x403FC0,0x403820,
             0x4658D6,0x465900,0x43D56C,0x43E580,0x43EA20,0x4027C0,0x402680,
             0x401830,0x4017F0,0x403EB0,0x40DF30,0x49C8F4,0x49FC56)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe',type=Path,required=True)
    parser.add_argument('--program',type=Path,required=True)
    args=parser.parse_args()
    data=args.exe.read_bytes()
    if hashlib.sha256(data).hexdigest()!=EXE_SHA256: parser.error('reference digest mismatch')
    pins={r[0]:r for r in (*FUNCTIONS,*PINS)}
    blocks=[]
    for address in ADDRESSES:
        _,offset,size,digest=pins[address]
        if pe_file_offset(data,address,size)!=offset: parser.error('PE mapping mismatch')
        body=data[offset:offset+size]
        if hashlib.sha256(body).hexdigest()!=digest: parser.error(f'code digest mismatch at {address:X}')
        blocks.append(struct.pack('<II',address,size)+body)
    # Literal C string and +0.0 comparison constant read by the original routine.
    for address,expected in ((0x4BF814,b'drown\0'),(0x4B0338,bytes(8)),
                             (0x4B07F0,struct.pack('<d',8.0)),(0x4B08F8,struct.pack('<d',16.0))):
        offset=pe_file_offset(data,address,len(expected))
        if data[offset:offset+len(expected)]!=expected: parser.error('literal mismatch')
        blocks.append(struct.pack('<II',address,len(expected))+expected)
    payload=struct.pack('<I',len(blocks))+b''.join(blocks)
    for _ in range(8):
        result=subprocess.run([str(args.program)],input=payload,capture_output=True,timeout=30)
        if result.returncode!=3:
            sys.stdout.buffer.write(result.stdout);sys.stderr.buffer.write(result.stderr)
            return result.returncode
    return result.returncode

if __name__=='__main__':raise SystemExit(main())
