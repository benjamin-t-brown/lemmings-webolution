"""Read-only, offline checks for the bounded runtime picking port.

No process launch, attachment, input, or install writes. This checks the stock
artifact's bytes, not live behavior or completeness of the SOURCE adapters.
"""
import argparse
import hashlib
from pathlib import Path
import struct

SHA256 = '32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe', type=Path,
                        default=Path(r'C:\progs\Lemmings Revolution\Lemmings Revolution.exe'))
    parser.add_argument('--first-level', type=Path,
                        help='optional already-extracted MM_CLIMB_002.LVL for support alphabet proof')
    args = parser.parse_args()
    data = args.exe.read_bytes()
    assert hashlib.sha256(data).hexdigest() == SHA256, 'wrong stock executable'
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
        0x41bd7b: ('c7411412000000', 'Walker category18'),
        0x415fcc: ('c7421413000000', 'Faller category19'),
        0x41376b: ('c7411400000000', 'Climber category0'),
        0x413c8b: ('c7411400000000', 'Hoister category0'),
        0x43a9a4: ('884a29', 'tube visibility writes actor+29'),
        0x43aa30: ('884829', 'outer actor path writes actor+29'),
        0x42e06f: ('c6827003000001', 'spawn sets actor+370'),
        0x426409: ('c6817003000000', 'exit path clears actor+370'),
        0x40c064: ('6a0e', 'Climber gate rejects category0e'),
        0x40c077: ('6a03', 'Climber gate rejects category3'),
        0x40c08a: ('6a00', 'Climber gate rejects category0'),
        0x4183e8: ('6a13', 'support gate rejects fall category13'),
        0x4183fb: ('6a21', 'support gate rejects bounce category21'),
        0x41840e: ('6a1e', 'support gate rejects category1e'),
        0x45dab9: ('8b88a8550000', 'main loads combined controller at global+55a8'),
        0x4532ed: ('898d58ffffff', 'controller update retains this pointer'),
        0x4537e6: ('8b8d58ffffff', 'hover uses same controller pointer'),
        0x454d77: ('898d1cffffff', 'hover retains incoming controller'),
        0x4551ab: ('8b851cffffff83c05850', 'hover reference is controller target+58'),
        0x452fe1: ('83c158', 'target setter writes same+58'),
        0x44503f: ('680000e03f6a00', 'startup target shell coefficient .5'),
        0x453f08: ('dd5a58', 'flip writes target yaw'),
        0x454133: ('dd5958', 'ordinary input writes target yaw'),
        0x45414e: ('dd5860', 'ordinary input writes target height'),
        0x4568a7: ('6a006a00', 'replay exception supplies target shell coefficient zero'),
        0x43a953: ('dc1dd0024b00', 'actor shellZ compared with1'),
        0x43a95e: ('7563', 'shellZ<1 enters bucket-filtered branch'),
        0x43a9e7: ('837dc8207c11', 'seam adjustment when upper bound>=32'),
        0x43aa04: ('7c1d', 'reject bucket below lower bound'),
        0x43aa0c: ('7f15', 'reject bucket above upper bound'),
        0x42f6a4: ('c7826003000000000000c782640300000000f0bf', 'left-facing bias -1'),
        0x42f6d4: ('c7816003000000000000c7816403000000000000', 'right-facing bias zero'),
        0x42e97d: ('dc05d0024b00', 'left Walker support +1 cancels facing bias'),
        0x42e989: ('db4144dc6de0', 'support sample subtracts gravity direction'),
    }
    for address, (expected, label) in checks.items():
        actual = read(address, len(expected)//2).hex()
        assert actual == expected, (hex(address), actual, expected)
        print(f'{address:08x} {actual} {label}')
    priorities = struct.unpack('<20i', read(0x4c05d0, 80))
    assert priorities == (2,1,1,4,2,3,3,3,3,1,0,2,2,2,2,2,2,0,0,2)
    print('priority table first20:', priorities)
    for site, target in ((0x45dabf, 0x4532e2), (0x4537ec, 0x454d6c),
                         (0x452fe4, 0x406700), (0x44507e, 0x452fd3),
                         (0x4551c2, 0x401830), (0x4551df, 0x401830),
                         (0x4551e6, 0x418350)):
        instruction = read(site, 5)
        assert instruction[0] == 0xe8
        assert site + 5 + struct.unpack('<i', instruction[1:])[0] == target
        print(f'{site:08x} call {target:08x}')
    # Replay exact historical D2d bytes, not rounded worker decimal strings.
    reference = struct.unpack('<3d', bytes.fromhex(
        '00000000000040400000000000004840000000000000e03f'))
    for actor_hex, expected in (
        ('00000040dc434e400000000000004040000000000000d03f', 1070.0324723702506),
        ('000000a0cb844a400000000000004040000000000000e83f', 698.6373975194438)):
        actor = struct.unpack('<3d', bytes.fromhex(actor_hex))
        assert sum((a-r)**2 for a, r in zip(actor, reference)) == expected
    print('PASS: retained D2d raw hex distances; no new native observation')
    if args.first_level:
        from collections import Counter
        from read_lvl_grid import find_grid
        level = args.first_level.read_bytes()
        assert hashlib.sha256(level).hexdigest() == (
            '95f84f86fb53ab29ce2ce3aed794cf69a06e5754d42ef78657f3af1a5db4c567')
        found = find_grid(level)
        assert found is not None
        _, cells = found
        histogram = Counter(cells)
        assert histogram == {0: 8973, 1: 9339, 3: 6224, 0x82: 40}
        def native_support(word):
            return bool(word and not (word & (0x80 | 0x1000)) and
                        word not in (0x200, 0x2000, 0x4000)) # sheet0
        height = len(cells) // 256
        for y in range(height):
            native_y = max(0, min(height - 1, height - 1 - y - 1))
            source_y = min(height - 1, y + 1)
            assert height - 1 - native_y == source_y
            for x in range(256):
                word = cells[native_y * 256 + x]
                assert native_support(word) == bool(word and not word & 0x80)
                for facing in (-1, 1):
                    bias = -1 if facing == -1 else 0
                    assert x + bias + (facing == -1) == x
        print('PASS: first-level alphabet and 49152 upright coordinate/word cases (static derivation)')
    # Print bounded helper instruction windows, not just selected immediate bytes.
    import capstone
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    for begin, end in ((0x4550a3, 0x455237), (0x42ecc9, 0x42ed4d),
                       (0x45a0e0, 0x45a17a), (0x40c061, 0x40c0af),
                       (0x4183d3, 0x41843c), (0x42e913, 0x42ea32),
                       (0x42e8c7, 0x42e911), (0x43a4ec, 0x43a615),
                       (0x43a901, 0x43aa4f), (0x40dec0, 0x40df2c)):
        print(f'\n-- {begin:08x}..{end:08x} --')
        for instruction in md.disasm(read(begin, end-begin), begin):
            print(f'{instruction.address:08x} {instruction.bytes.hex():22s} '
                  f'{instruction.mnemonic} {instruction.op_str}')
    print('PASS: pinned artifact, raw-backed bytes and bounded picking checks')


if __name__ == '__main__':
    main()
