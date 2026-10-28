"""Offline stock-byte checks for pending completion, results and map routes.

No process launch/attachment, desktop input, registry or save access. Native
timing, physical input and final image fidelity are not established by this.
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
        0x45df11: ('6a21', 'win factory selector21'),
        0x45e095: ('6a20', 'loss factory selector20'),
        0x45df33: ('c7812c57000004000000', 'win UI state4'),
        0x45e0b7: ('c7802c57000005000000', 'loss UI state5'),
        0x485e55: ('c78408e8364e0001000000', 'win selected completion flag1'),
        0x485e7f: ('6a32', 'win title UK50'),
        0x48635d: ('6a31', 'loss title UK49'),
        0x486090: ('6a23', 'win Continue selects map23'),
        0x4865ac: ('6a23', 'loss Continue selects map23'),
        0x4c33fc: ('000000002f000000', 'win row table: empty,Continue'),
        0x4c3414: ('00000000300000002f0000002b000000', 'loss rows: empty,Retry,Continue,Quit'),
        0x4b3948: ('000000000000000000000000', 'first map node group0/slot0/extra0'),
        0x444bce: ('8b0df0154b008988305100008b15f4154b00899034510000', 'ordinary load/retry seeds timer7'),
        0x444c25: ('c680d1500000008b4df8c681d250000000', 'reset win and pending latches'),
        0x445477: ('837a4c03', 'retained update-list count excludes sheet3'),
        0x4454fa: ('8a91d250000083fa010f85e1000000', 'choose pending branch at call entry'),
        0x44550c: ('dd8030510000dc6508', 'pending subtracts simulation-slice dt'),
        0x445527: ('dc1dd8034b00dfe0f6c4017451', 'strict less-than3 effect boundary'),
        0x44558e: ('dc1dd0024b00dfe0f6c401744d', 'strict less-than1 result boundary'),
        0x445611: ('c681d1500000018b55fcc682d250000001', 'timeout win latch precedes pending'),
        0x445657: ('c681d1500000018b55fcc682d250000001', 'exhaustion win latch precedes pending'),
        0x44562f: ('0fbf919850000003c285c0752c', 'signed unreleased plus retained count exact-zero test'),
        0x490ed6: ('558bec51894dfc8be55dc3', 'end-effect draw helper is empty'),
        0x4c0040: ('4576696c52617400', 'sheet3 is EvilRat, not a death-state marker'),
        0x486568: ('c605acbf4c0001c60584354e0000', 'loss Retry requests reload and disables intro'),
        0x486583: ('c605aabf4c0001c60584354e0001', 'loss Quit requests map and enables next intro'),
        0x45d743: ('c7812c57000003000000', 'Quit dispatch selects UI3'),
        0x45d76d: ('c681a255000000', 'Quit dispatch clears request'),
        0x45d7d8: ('6a23', 'Quit dispatch constructs map23'),
        0x49b242: ('6a07', 'map footer selects main menu7'),
        0x49b1f3: ('c60581354e0001', 'map cancel uses separate application-exit latch'),
        0x49b1cc: ('c60584354e0001', 'successful map activation enables intro'),
        0x45cac4: ('8a917ccb0100889098c80100', 'loader copies preference to camera-intro-active'),
        0x45cba6: ('dc25f8174b00', 'no-intro loader subtracts2 from title clock'),
        0x45cbbe: ('c68198c8010000', 'no-intro loader disables camera intro'),
    }
    for va, (hex_bytes, meaning) in checks.items():
        expected = bytes.fromhex(hex_bytes)
        assert read(va, len(expected)) == expected, f'{va:#x}: {meaning}'
        print(f'PASS {va:08x}: {meaning}')
    constants = [(0x4b15f0, 7.0), (0x4b03d8, 3.0), (0x4b02d0, 1.0),
                 (0x4b17f8, 2.0), (0x4b17e0, 1.0), (0x4b17e8, 0.5)]
    for va, value in constants:
        assert struct.unpack('<d', read(va, 8))[0] == value, f'{va:#x}: constant {value}'
        print(f'PASS {va:08x}: double {value}')
    print(f'PASS: {len(checks)} raw-backed anchors and {len(constants)} doubles; not a live comparison')


if __name__ == '__main__':
    main()
