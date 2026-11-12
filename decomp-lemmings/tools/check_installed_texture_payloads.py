"""Compare portable bitmap decoding totals with the installed block envelopes."""
import argparse
import json
from pathlib import Path
import struct
import subprocess
from inspect_box import read_index

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--program',type=Path,required=True)
    parser.add_argument('--archive',type=Path,required=True)
    parser.add_argument('--patch',type=Path)
    args=parser.parse_args()
    archives=([args.patch] if args.patch else [])+[args.archive]
    entries={}
    for archive in archives:
        for entry in read_index(archive):
            if entry.name.upper().endswith('.BMP'):entries.setdefault(entry.name.upper(),(archive,entry))
    if not entries:parser.error('no BMP entries found')
    expected=dict(bitmaps=len(entries),packed_bitmaps=0,packed_bytes=0,decoded_bytes=0,codec3_blocks=0,codec4_blocks=0)
    for archive,entry in entries.values():
        with archive.open('rb') as source:
            source.seek(entry.offset);data=source.read(entry.size)
        expected['packed_bytes']+=len(data)
        if data.startswith(b'BM'):expected['decoded_bytes']+=len(data);continue
        expected['packed_bitmaps']+=1;cursor=1
        for _ in range(data[0]):
            packed,output,codec=struct.unpack_from('<HHB',data,cursor)
            if packed<5 or cursor+packed>len(data) or codec not in (3,4):raise ValueError('invalid envelope')
            expected['decoded_bytes']+=output;expected[f'codec{codec}_blocks']+=1;cursor+=packed
    command=[str(args.program),'--archive',str(args.archive),'--all']
    if args.patch:command+=['--patch',str(args.patch)]
    result=subprocess.run(command,capture_output=True,text=True,timeout=180,check=True)
    observed=json.loads(result.stdout)
    if observed!=expected:raise AssertionError(f'{observed} != {expected}')
    print(f"{len(entries)} installed BMP payloads decoded; {expected['decoded_bytes']} bytes match envelope sizes")

if __name__=='__main__':main()
