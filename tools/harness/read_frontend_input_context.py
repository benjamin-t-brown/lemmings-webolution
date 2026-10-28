"""Offline C -> game+55B8 -> shared input-context bridge; never runs the game.

Completes the specific storage gap retained by B3k continuation 6. This is a
conditional static chain, not observation of successful device initialization,
loaded shim identity, physical mouse mapping, or any live pointer value.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct

STOCK = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
EXPECTED = {
    0x4507AA: "75 0e",  # earlier predicate can skip this initialization arm
    0x4507AC: "8b 45 08 50 b9 08 6a 4c 00 e8 f3 e9 00 00",
    0x45F1C8: "89 4d e0",  # retain caller this
    0x45F2F3: "83 7d e4 00 74 2b",  # null allocation stores null instead
    0x45F317: "8b 4d e4 e8 9b 3c 02 00 89 45 dc eb 07",
    0x45F324: "c7 45 dc 00 00 00 00",
    0x45F32B: "8b 55 dc 89 55 e8 c7 45 fc ff ff ff ff 8b 45 e0 8b 4d e8 89 88 b8 55 00 00",
    0x450873: "a1 c0 bf 4c 00 50 e8 e0 61 03 00 83 c4 04",
    0x486A5E: "55 8b ec 8b 45 08 a3 dc 59 5d 00",
}


def ranges(data):
    pe = struct.unpack_from('<I', data, 0x3C)[0]
    if data[pe:pe+4] != b'PE\0\0':
        raise ValueError('not PE')
    count = struct.unpack_from('<H', data, pe+6)[0]
    opt_size = struct.unpack_from('<H', data, pe+20)[0]
    base = struct.unpack_from('<I', data, pe+24+28)[0]
    for i in range(count):
        s = pe+24+opt_size+40*i
        _virtual_size, rva, raw_size, offset = struct.unpack_from('<IIII', data, s+8)
        yield base+rva, raw_size, offset  # raw-backed only; never virtual BSS


def file_offset(data, address, size):
    for start, raw_size, offset in ranges(data):
        if start <= address and address+size <= start+raw_size:
            result = offset+address-start
            if result+size <= len(data):
                return result
    raise ValueError(f'not raw-backed: {address:#x}+{size}')


def verify(data):
    rows = []
    for address, text in EXPECTED.items():
        expected = bytes.fromhex(text)
        offset = file_offset(data, address, len(expected))
        actual = data[offset:offset+len(expected)]
        if actual != expected:
            raise ValueError(f'bridge instruction mismatch at {address:#x}')
        rows.append({'address': hex(address), 'bytes': actual.hex(), 'file_offset': offset})
    this = struct.unpack_from('<I', data, file_offset(data, 0x4507B1, 4))[0]
    member = struct.unpack_from('<I', data, file_offset(data, 0x45F340, 4))[0]
    shared_load = struct.unpack_from('<I', data, file_offset(data, 0x450874, 4))[0]
    if this+member != shared_load:
        raise ValueError('constructor destination does not match startup load')
    return {'classification': 'READ instructions; DERIVED conditional storage identity',
            'game_this': hex(this), 'member': hex(member), 'context_global': hex(shared_load),
            'shared_global': '0x5d59dc', 'instructions': rows,
            'limits': 'Normal initialization arm and successful construction required; no runtime/device observation.'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe', type=Path, default=Path('C:/progs/Lemmings Revolution/Lemmings Revolution.exe'))
    parser.add_argument('--self-test', action='store_true')
    args = parser.parse_args()
    data = args.exe.read_bytes()
    if hashlib.sha256(data).hexdigest() != STOCK:
        raise SystemExit('not the stock authority executable')
    result = verify(data)
    result['sha256'] = STOCK
    if args.self_test:
        for address in (0x4507B1, 0x45F340):
            mutant = bytearray(data)
            mutant[file_offset(data, address, 1)] ^= 1
            try:
                verify(mutant)
            except ValueError:
                continue
            raise AssertionError(f'undetected bridge mutation at {address:#x}')
        result['self_test'] = 'two isolated in-memory bridge mutations rejected; installed file untouched'
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
