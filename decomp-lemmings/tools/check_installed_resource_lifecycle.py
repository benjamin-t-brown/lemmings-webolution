"""Exercise recovered resource lifecycle against installed SPR data in place."""
import argparse
from collections import Counter
import json
import math
from pathlib import Path
import struct
import subprocess
from check_installed_sprite_resources import parse
from inspect_box import read_index

def texture_names(data):
    cursor=0
    names=set()
    def number():
        nonlocal cursor
        result=struct.unpack_from('<I',data,cursor)[0];cursor+=4
        return result
    def text(capacity):
        nonlocal cursor
        size=struct.unpack('<i',struct.pack('<I',number()))[0]
        if size>capacity or size<=0:return b''
        value=data[cursor:cursor+size];cursor+=size
        return value[:capacity-1].split(b'\0',1)[0]
    for _ in range(number()):
        for _ in range(number()):
            for _ in range(number()):
                cursor+=64;names.add(text(260))
        text(128);cursor+=8
    return names

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
            name=entry.name.upper()
            if name.startswith('SPRITES\\') and name.endswith('.SPR'):
                entries.setdefault(name,(archive,entry))
    if not entries:parser.error('no sprite resources found')
    stats=Counter()
    expected_animations=0
    textures=set()
    for archive,entry in entries.values():
        with archive.open('rb') as source:
            source.seek(entry.offset)
            data=source.read(entry.size)
            fields=parse(data,stats);textures.update(texture_names(data))
        cursor=1
        usable=False
        for _ in range(fields[0]):
            frames=fields[cursor]
            speed=struct.unpack('<d',struct.pack('<Q',fields[cursor+2]))[0]
            usable |= frames>0 and math.isfinite(speed) and 0<speed<1000
            cursor+=3+frames*3
        expected_animations+=usable
    command=[str(args.program),'--archive',str(args.archive),'--all']
    if args.patch:command+=['--patch',str(args.patch)]
    result=subprocess.run(command,capture_output=True,text=True,timeout=60,check=True)
    observed=json.loads(result.stdout)
    expected={
        'resources':len(entries),'clips':stats['clips'],'frames':stats['frames'],
        'pieces':stats['pieces'],'created':len(entries)+2,'destroyed':len(entries)+2,
        'animation_checks':expected_animations,'loaded_after_cleanup':0,'slot_reused':True,
        'texture_names':len(textures),'stream_reads':len(entries)+2,'stream_closes':len(entries)+2,
        'stream_seeks':len(entries)+2,
        'quads_checked':stats['pieces'],
    }
    if observed!=expected:raise AssertionError(f'{observed} != {expected}')
    print(f"{len(entries)} installed sprites: cache hits, animation binding, unload/recreate, eviction and slot reuse passed")

if __name__=='__main__':main()
