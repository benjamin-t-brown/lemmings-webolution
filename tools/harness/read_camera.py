"""Read the stock level-camera operands without REA or third-party packages.

This reports stored operands and derived binary64 algebra. It does not infer
near/far planes or reproduce the original x87 intermediate precision.
"""
import argparse
import hashlib
import json
import math
import struct
from pathlib import Path


def inspect(executable):
    data=Path(executable).read_bytes()
    pe=struct.unpack_from('<I',data,0x3c)[0]
    assert data[:2]==b'MZ' and data[pe:pe+4]==b'PE\0\0'
    opt=pe+24
    assert struct.unpack_from('<H',data,opt)[0]==0x10b
    base=struct.unpack_from('<I',data,opt+28)[0]
    table=opt+struct.unpack_from('<H',data,pe+20)[0]
    def read(va,size):
        for i in range(struct.unpack_from('<H',data,pe+6)[0]):
            _,rva,raw_size,raw=struct.unpack_from('<IIII',data,table+40*i+8)
            offset=va-base-rva
            if 0<=offset and offset+size<=raw_size:
                return data[raw+offset:raw+offset+size]
        raise ValueError(f'{va:#x}: not fully file-backed')
    def qword(va):return struct.unpack('<d',read(va,8))[0]
    def pushed_double(high_va,low_va):
        high,low=read(high_va,5),read(low_va,5)
        assert high[0]==low[0]==0x68
        return struct.unpack('<d',low[1:]+high[1:])[0]
    def call(va):
        raw=read(va,5);assert raw[0]==0xe8
        return va+5+struct.unpack_from('<i',raw,1)[0]
    assert call(0x47b095)==0x49d8d4
    assert read(0x49d8f2,2)==b'\xd9\xf2' # FPTAN, not FSIN
    assert read(0x49cba2,2)==b'\xd9\xfe'
    assert read(0x49cc52,2)==b'\xd9\xff'
    angle=pushed_double(0x45c8f8,0x45c8fd)
    pitch=pushed_double(0x45cf83,0x45cf88)
    divisor=qword(0x4b1348)
    pi=qword(0x4b1ab0)
    width_instruction,height_instruction=read(0x45c915,5),read(0x45c910,5)
    assert width_instruction[0]==height_instruction[0]==0x68
    width=struct.unpack_from('<I',width_instruction,1)[0]
    height=struct.unpack_from('<I',height_instruction,1)[0]
    g=qword(0x4b02d0)/(2*math.tan(pi*(angle/divisor)))
    operands=[('level_angle_degrees',0x45c8f8,10,angle),
              ('fixed_pitch_turns',0x45cf83,10,pitch),
              ('eye_distance_magnitude',0x4b1098,8,qword(0x4b1098)),
              ('height_bias',0x4b1808,8,qword(0x4b1808)),
              ('yaw_offset',0x4b1350,8,qword(0x4b1350)),
              ('yaw_units_per_turn',0x4b07a8,8,qword(0x4b07a8)),
              ('lens_degree_divisor',0x4b1348,8,divisor),
              ('lens_pi',0x4b1ab0,8,pi),('pose_pi',0x4b0f88,8,qword(0x4b0f88)),
              ('grab_mouse_gain',0x4c0398,8,qword(0x4c0398)),
              ('rotation_unit',0x4c0388,8,qword(0x4c0388))]
    return {'executable_sha256':hashlib.sha256(data).hexdigest(),
            'stored_operands':[{'name':name,'va':hex(va),'bytes':read(va,size).hex(),'binary64':value,'binary64_hex':value.hex()} for name,va,size,value in operands],
            'level_initializer_dimensions':[width,height],
            'derived_binary64':{'theta':angle/divisor,'g':g,'centered_focal_pixels':g*height},
            'scope':'Level pose/lens operand read. Runtime center and control state require a live dump. No conventional near/far values inferred. Derived arithmetic is not a bit-exact x87 replay.'}


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe',type=Path,default=Path(r'C:\progs\Lemmings Revolution\Lemmings Revolution.exe'))
    args=parser.parse_args()
    print(json.dumps(inspect(args.exe),indent=2))
