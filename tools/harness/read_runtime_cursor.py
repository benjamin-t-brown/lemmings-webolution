"""Offline stock-byte anchors for the bounded software-cursor integration.

Never executes/attaches to the original or accesses registry/desktop state.
See docs/scratch/runtime-cursor-hover.md for the complete producer chains and
limitations. These anchors are not a live pixel or physical-input comparison.
"""
import argparse
import hashlib
from pathlib import Path
import struct


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe', type=Path,
                        default=Path(r'C:\progs\Lemmings Revolution\Lemmings Revolution.exe'))
    args = parser.parse_args()
    data = args.exe.read_bytes()
    assert hashlib.sha256(data).hexdigest() == (
        '32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f')
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    assert data[pe:pe+4] == b'PE\0\0'
    opt = pe + 24
    base = struct.unpack_from('<I', data, opt+28)[0]
    table = opt + struct.unpack_from('<H', data, pe+20)[0]
    sections = []
    for i in range(struct.unpack_from('<H', data, pe+6)[0]):
        off = table + 40*i
        _, va, rawsize, ptr = struct.unpack_from('<IIII', data, off+8)
        sections.append((base+va, rawsize, ptr))

    def read(va, size):
        for start, rawsize, ptr in sections:
            delta = va-start
            if 0 <= delta and delta+size <= rawsize:
                result = data[ptr+delta:ptr+delta+size]
                assert len(result) == size
                return result
        raise AssertionError(f'{va:#x}+{size:#x} is not wholly file-backed')

    checks = {
        0x455cb7: ('b001', 'first grab predicate is unconditional true'),
        0x455dda: ('8b480c', 'second grab predicate loads mouse pointer'),
        0x455ddd: ('e80e000000', 'calls455df0'),
        0x455dfc: ('837804010f94c1', 'held field mouse+4 equals1'),
        0x483c58: ('8b4df981e1ff00000081e180000000', 'device state byte13 high bit'),
        0x483c70: ('894a04', 'store held state at mouse+4'),
        0x453295: ('a0adcd5800', 'effective mode reads backend/window byte'),
        0x45329e: ('b802000000', 'windowed backend effective mode2'),
        0x4532a8: ('8b4108', 'software backend uses configured screen mode'),
        0x452cfd: ('c7420c00000000', 'constructor display mode0'),
        0x45c202: ('e89e690200', 'read CursorMode setting'),
        0x45c20e: ('751c', 'successful setting read skips fallback'),
        0x45c222: ('c785d4feffff02000000', 'failed setting read fallback2'),
        0x45c238: ('8988ac550000', 'store setting global+55ac'),
        0x45c8be: ('8b91ac550000', 'activation loads setting'),
        0x45c8c4: ('895008', 'activation stores screen+8'),
        0x45578c: ('6a206a206a006a00', 'NoLem source rectangle32x32'),
        0x455a0c: ('6a206a206a006a00', 'Grab source rectangle32x32'),
        0x4540ea: ('dd80a8160000', 'ordinary movement loads authoritative Y double'),
        0x4540fc: ('dd81a0160000', 'ordinary movement loads authoritative X double'),
        0x46ea7c: ('88482d', 'window creation writes backend flag+2d'),
        0x45bf78: ('c680b4c9010000', 'native constructor fullscreen/software producer'),
    }
    for va, (hex_bytes, meaning) in checks.items():
        expected = bytes.fromhex(hex_bytes)
        assert read(va, len(expected)) == expected, f'{va:#x}: {meaning}'
        print(f'PASS {va:08x}: {meaning}')
    print(f'PASS: {len(checks)} stock-byte anchors; no runtime or native pixel claim')


if __name__ == '__main__':
    main()
