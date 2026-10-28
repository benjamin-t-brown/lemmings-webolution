"""Offline stock-byte anchors for the frontend software cursor.

Never launches/attaches to the game or accesses desktop, registry or saves.
See docs/scratch/frontend-cursor-return-coverage.md. These are static checks,
not native input/pixel acceptance or proof of the texture conversion format.
"""
import argparse
import hashlib
from pathlib import Path
import struct


ANCHORS = {
    0x4c369d: ('01000040010000f0000000', 'frontend flag1 and initial XY320,240'),
    0x455ee3: ('a09d364c00', 'frontend flag getter'),
    0x455ef3: ('8a4508a29d364c00', 'frontend flag setter'),
    0x453bf6: ('e8e522000025ff00000085c00f8491010000', 'false flag branches to gameplay'),
    0x453c08: ('e8b32200008bf08b9558ffffff8b0ae81422000003f08975b0e8aa2200008bf08b8558ffffff8b08e81b22000003f08975ac', 'frontend prior XY plus relative mouse deltas'),
    0x453c3a: ('e8d12200000fbfc83b4db07d10', 'signed width comparison, inclusive upper X clamp'),
    0x453c60: ('83bd38ffffff007d0cc78534ffffff00000000', 'lower X clamp zero'),
    0x453cb0: ('e84b2200000fbfc03b45ac7d10', 'signed height comparison, inclusive upper Y clamp'),
    0x453cd6: ('83bd2cffffff007d0cc78528ffffff00000000', 'lower Y clamp zero'),
    0x453d2e: ('e86d21000083c4088b8d58ffffff8b09e84d20000025ff00000085c075168b9558ffffff8b0ae87720000025ff00000085c0740f8b8558ffffffc740040a000000eb0d8b8d58ffffffc7410401000000', 'publish XY; either held button selects10, else1'),
    0x453d7e: ('8b55ac528b45b0508b8d58ffffff8b493ce8393a0000e96f0c0000', 'move sprite and skip gameplay hover policy'),
    0x453283: ('e8ba2d0000', 'post-update cursor binder456042'),
    0x45604d: ('a0adcd580085c00f8478010000', 'backend byte0 selects software'),
    0x45620b: ('8b4dec8379040175118b55ec83c2708b45ec89503ce9ff000000', 'state1 binds screen+70'),
    0x4562e7: ('8b4dec8379040a75118b55ec81c2681400008b45ec89503ceb23', 'state10 binds screen+1468'),
    0x45577b: ('a120334e00508b4dfc83c170e8dc2100006a206a206a006a00', 'normal resource slot4e3320 and32x32 source'),
    0x455a46: ('a144334e00508b4dfc81c168140000e80e1f00006a206a206a006a00', 'held resource slot4e3344 and32x32 source'),
    0x45e7f3: ('68780d4c00b948cb5800e80e1600008b4df8898118c90100', 'register normal name into4e3320'),
    0x45e8cb: ('68e80d4c00b948cb5800e8361500008b55f889823cc90100', 'register held name into4e3344'),
    0x4c0d78: ('5f6375724e6f4c656d00', 'normal _curNoLem'),
    0x4c0de8: ('5f6375724d736544776e00', 'held _curMseDwn'),
    0x454c77: ('33c0a0adcd580085c07532', 'skip software draw on alternate backend'),
    0x454c8e: ('8b4df483793c0074118b55f48b4a3c8b45f48b503c8b02ff5004', 'draw current cursor sprite'),
    0x45dab9: ('8b88a8550000e81e58ffff8b8d6cffffff8b89a8550000e85255ffff', 'main loop updates cursor then binds'),
    0x45e2c7: ('e8418d0200', 'frontend draw before final cursor draw'),
    0x45e30b: ('8b8aa8550000e86f67ffff', 'final cursor draw454a85'),
    0x455dbc: ('8338010f94c18ac1', 'first button held, not new-press field'),
    0x455dfc: ('837804010f94c1', 'second button held, not new-press field'),
    0x483c3e: ('8b55f881e2ff00000081e280000000f7da1bd2f7da8b45e88910', 'sample byte12 highbit to first held field'),
    0x483c58: ('8b4df981e1ff00000081e180000000f7d91bc9f7d98b55e8894a04', 'sample byte13 highbit to second held field'),
    0x455e3a: ('8b480c8b4120', 'relative mouse X field'),
    0x455e5a: ('8b480c8b4124', 'relative mouse Y field'),
    0x455ec3: ('a1a0364c00', 'previous frontend X'),
    0x455ed3: ('a1a4364c00', 'previous frontend Y'),
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe', type=Path,
                        default=Path(r'C:\progs\Lemmings Revolution\Lemmings Revolution.exe'))
    data = parser.parse_args().exe.read_bytes()
    assert hashlib.sha256(data).hexdigest() == (
        '32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f')
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    assert data[pe:pe+4] == b'PE\0\0'
    opt = pe + 24
    base = struct.unpack_from('<I', data, opt+28)[0]
    table = opt + struct.unpack_from('<H', data, pe+20)[0]
    sections = []
    for i in range(struct.unpack_from('<H', data, pe+6)[0]):
        _, va, rawsize, offset = struct.unpack_from('<IIII', data, table+40*i+8)
        sections.append((base+va, rawsize, offset))

    def read(va, size):
        for start, rawsize, offset in sections:
            delta = va-start
            if 0 <= delta and delta+size <= rawsize:
                result = data[offset+delta:offset+delta+size]
                assert len(result) == size
                return result
        raise AssertionError(f'{va:#x}+{size:#x} is not wholly file-backed')

    for va, (hex_bytes, meaning) in ANCHORS.items():
        expected = bytes.fromhex(hex_bytes)
        assert read(va, len(expected)) == expected, f'{va:#x}: {meaning}'
        print(f'PASS {va:08x}: {meaning}')
    print(f'PASS: {len(ANCHORS)} stock-byte anchors; no runtime or native pixel claim')


if __name__ == '__main__':
    main()
