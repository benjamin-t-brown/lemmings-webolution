"""Offline anchors for the frontend root/page construction lifecycle.

Never executes/attaches to the game or reads desktop/registry state. Native
observations and the failed first return prediction are preserved in
docs/scratch/frontend-native-return-2026-10-28.md. Indirect/aliased counter
writes and arbitrary allocation-failure paths are not ruled out by this check.
"""
import argparse
import hashlib
from pathlib import Path
import struct


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
        name = data[table+40*i:table+40*i+8].rstrip(b'\0')
        _, va, rawsize, offset = struct.unpack_from('<IIII', data, table+40*i+8)
        sections.append((name, base+va, rawsize, offset))

    def read(va, size):
        for _, start, rawsize, offset in sections:
            delta = va-start
            if 0 <= delta and delta+size <= rawsize:
                result = data[offset+delta:offset+delta+size]
                assert len(result) == size
                return result
        raise AssertionError(f'{va:#x}+{size:#x} is not wholly file-backed')

    anchors = {
        0x4c416c: ('01000000', 'initial construction serial is1'),
        0x488359: ('8b0d6c414c0089482c8b156c414c0083c20189156c414c00', 'store old serial at object+2c then increment'),
        0x486f95: ('6a74', 'root allocation size74'),
        0x486fd3: ('8915ec595d00', 'root stored outside page list at5d59ec'),
        0x487fa6: ('a3e4595d00', 'page-list head is separate5d59e4'),
        0x48bc94: ('c701881e4b00', 'root vtable4b1e88'),
        0x48f4f2: ('c700b0264b00', 'menu page vtable4b26b0'),
        0x49aab6: ('c702b83e4b00', 'map page vtable4b3eb8'),
        0x48f601: ('c605aabf4c0001', 'menu Quit requests map'),
        0x45d69b: ('6a23', 'Quit requests map selector23'),
        0x486090: ('6a23', 'win Continue selects map'),
        0x48609e: ('ff520c', 'win Continue dispatches virtual page replacement'),
        0x4865ac: ('6a23', 'loss Continue selects map'),
        0x4865ba: ('ff520c', 'loss Continue dispatches virtual page replacement'),
        0x4b1c3c: ('e4864800', 'win virtual replacement is4886e4'),
        0x4b1cbc: ('e4864800', 'loss virtual replacement is4886e4'),
    }
    calls = {
        0x45d432: (0x486f60, 'playing menu opening uses full frontend init'),
        0x45d4c4: (0x486f60, 'paused menu opening also uses full init'),
        0x486fb2: (0x48bc00, 'full init constructs root'),
        0x486ff7: (0x48726f, 'full init then constructs selected page'),
        0x48bc0a: (0x4882c0, 'root invokes counted base constructor'),
        0x48f4ea: (0x4882c0, 'menu invokes counted base constructor'),
        0x49aa9a: (0x4882c0, 'map invokes counted base constructor'),
        0x4873bc: (0x48f4e0, 'menu factory invokes menu constructor'),
        0x487911: (0x49aa90, 'map factory invokes map constructor'),
        0x45d55d: (0x487196, 'O-close tears frontend down'),
        0x45d84a: (0x487196, 'Resume tears frontend down'),
        0x45d5bb: (0x487196, 'menu Retry tears frontend down'),
        0x45d661: (0x487196, 'menu Quit tears old frontend down'),
        0x45d6af: (0x486f60, 'menu Quit creates fresh root and map'),
        0x45df25: (0x486f60, 'win results create root and page'),
        0x45e0a9: (0x486f60, 'loss results create root and page'),
        0x45d6f8: (0x487196, 'loss Retry tears frontend down'),
        0x45d79e: (0x487196, 'loss Quit tears old frontend down'),
        0x45d7ec: (0x486f60, 'loss Quit creates fresh root and map'),
        0x488713: (0x48726f, 'Continue replacement creates page only'),
        0x45d8d6: (0x487196, 'level load ends frontend lifetime'),
        0x485dfa: (0x4882c0, 'win page invokes counted base constructor'),
        0x4862fa: (0x4882c0, 'loss page invokes counted base constructor'),
    }
    for va, (hex_bytes, meaning) in anchors.items():
        want = bytes.fromhex(hex_bytes)
        assert read(va, len(want)) == want, f'{va:#x}: {meaning}'
        print(f'PASS {va:08x}: {meaning}')
    for va, (target, meaning) in calls.items():
        want = b'\xe8' + struct.pack('<i', target-va-5)
        assert read(va, 5) == want, f'{va:#x}: {meaning}'
        print(f'PASS {va:08x}: {want.hex()} {meaning}')
    _, start, size, offset = next(s for s in sections if s[0] == b'.text')
    code = data[offset:offset+size]
    operand = struct.pack('<I', 0x4c416c)
    sites = [start+i for i in range(len(code)-3) if code[i:i+4] == operand]
    assert sites == [0x48835b, 0x488364, 0x48836d], sites
    print('PASS: only three encoded counter-address operands in .text; not an alias analysis')
    print(f'PASS: {len(anchors)} byte anchors and {len(calls)} direct-call anchors; READ, not runtime validation')


if __name__ == '__main__':
    main()
