"""Independent bounded check of A4r operands and installed SDK layout."""
from pathlib import Path
import hashlib
import json
import re
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

folder = Path('build/tmp/a4r-actor-render-state')
folder.mkdir(parents=True, exist_ok=True)
target = Path(r'C:\progs\Lemmings Revolution\Lemmings Revolution.exe')
raw = target.read_bytes()
digest = hashlib.sha256(raw).hexdigest()
assert digest == '32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f'
pe = pefile.PE(data=raw)
md = Cs(CS_ARCH_X86, CS_MODE_32)

def window(address, length):
    rva = address - pe.OPTIONAL_HEADER.ImageBase
    section = pe.get_section_by_rva(rva)
    assert section is not None
    delta = rva - section.VirtualAddress
    assert delta + length <= section.SizeOfRawData
    start = section.PointerToRawData + delta
    return raw[start:start + length]

stage_expected = {
    0x4726B1: (0, 4, 2), 0x4726CB: (0, 5, 2),
    0x4726E5: (0, 1, 4), 0x4726FF: (0, 2, 2), 0x472719: (0, 3, 0),
    0x47275E: (0, 4, 4), 0x472778: (0, 5, 2), 0x472792: (0, 6, 0),
    0x4727AC: (0, 1, 4), 0x4727C6: (0, 2, 2), 0x4727E0: (0, 3, 0),
    0x4727FF: (0, 1, 4), 0x472819: (0, 2, 2), 0x472833: (0, 3, 0),
    0x47285F: (0, 4, 2), 0x47287A: (0, 5, 2),
    0x472901: (0, 4, 4), 0x47291C: (0, 5, 2), 0x472937: (0, 6, 0),
    0x472954: (0, 4, 2), 0x47296F: (0, 5, 2),
}
material_render_expected = {
    0x47269A: (15, 0), 0x472747: (15, 0), 0x472893: (15, 1),
    0x4728AD: (24, 'edx'), 0x4728C2: (25, 5), 0x4728DC: (15, 0),
}
blend_expected = {
    0x414DB8: (27, 1), 0x414DD4: (19, 2), 0x414DEA: (20, 2),
    0x414E02: (19, 5), 0x414E1E: (20, 2), 0x414E36: (20, 6),
    0x414E4E: (27, 0),
}
observed = []
for base, length, expected, slot, n in [
    (0x47261F, 0x35A, stage_expected, '0xa0', 3),
    (0x414D80, 0xD3, blend_expected, '0x58', 2),
    (0x47261F, 0x35A, material_render_expected, '0x58', 2),
]:
    pushes = []
    matched = set()
    for ins in md.disasm(window(base, length), base):
        if ins.mnemonic == 'push':
            pushes.append(ins.op_str)
        if ins.mnemonic == 'call':
            if ins.op_str.endswith(' + ' + slot + ']'):
                assert len(pushes) >= n + 1
                args = tuple(int(x, 0) if re.fullmatch(r'-?(0x[0-9a-f]+|[0-9]+)', x) else x
                             for x in reversed(pushes[-n-1:-1]))
                assert args == expected[ins.address], (hex(ins.address), args)
                matched.add(ins.address)
                observed.append({'call': hex(ins.address), 'vtable_offset': slot, 'args': args})
            pushes.clear()
    assert matched == set(expected), (matched, set(expected))

anchors = {
    0x47ABAF: 'e86c9df9ff', 0x41494F: 'e82c040000',
    0x414D86: '3b057c224c00', 0x414D8C: '0f84bf000000',
    0x472626: 'a1f4d55c00', 0x47262E: '0f8441030000',
    0x472662: '3b5124', 0x472665: '0f840a030000',
    0x47267D: '83e008', 0x47272A: '83e210',
    0x47283F: '83e103', 0x4728E5: '83e104',
    0x440C3D: '8988a8010000', 0x472653: 'ff9298000000',
    0x47271F: 'e951020000', 0x4727E6: 'e98a010000',
    0x472682: '0f849c000000', 0x47272F: '0f84b6000000',
    0x472844: '0f8480000000', 0x4728EA: '7453',
    0x472896: '8b15e0244c00526a18',
}
for address, value in anchors.items():
    assert window(address, len(bytes.fromhex(value))).hex() == value, hex(address)

include = Path(r'C:\progs\msys2\mingw32\include')
header = (include / 'd3d.h').read_text(encoding='utf-8')
body = header.split('DECLARE_INTERFACE_(IDirect3DDevice3,IUnknown)', 1)[1].split('};', 1)[0]
methods = re.findall(r'STDMETHOD(?:_\([^,]+,\s*(\w+)\)|\((\w+)\))', body)
names = [a or b for a, b in methods]
offsets = {name: i * 4 for i, name in enumerate(names)}
for name, value in {'SetRenderState': 0x58, 'SetTexture': 0x98, 'SetTextureStageState': 0xA0}.items():
    assert offsets[name] == value, (name, offsets[name])
types = (include / 'd3dtypes.h').read_text(encoding='utf-8')
enums = {
    'D3DRENDERSTATE_ALPHABLENDENABLE': 27, 'D3DRENDERSTATE_SRCBLEND': 19,
    'D3DRENDERSTATE_DESTBLEND': 20, 'D3DBLEND_ONE': 2,
    'D3DBLEND_SRCALPHA': 5, 'D3DBLEND_INVSRCALPHA': 6,
    'D3DTSS_COLOROP': 1, 'D3DTSS_COLORARG1': 2, 'D3DTSS_COLORARG2': 3,
    'D3DTSS_ALPHAOP': 4, 'D3DTSS_ALPHAARG1': 5, 'D3DTSS_ALPHAARG2': 6,
    'D3DTOP_SELECTARG1': 2, 'D3DTOP_MODULATE': 4,
    'D3DRENDERSTATE_ALPHATESTENABLE': 15, 'D3DRENDERSTATE_ALPHAREF': 24,
    'D3DRENDERSTATE_ALPHAFUNC': 25, 'D3DCMP_GREATER': 5,
}
for name, expected in enums.items():
    match = re.search(r'\b' + name + r'\s*=\s*(\d+)', types)
    assert match and int(match[1]) == expected, name
for name, expected in {'D3DTA_DIFFUSE': 0, 'D3DTA_TEXTURE': 2}.items():
    match = re.search(r'#define\s+' + name + r'\s+(0x[0-9a-fA-F]+|\d+)', types)
    assert match and int(match[1], 0) == expected, name

result = {'sha256': digest, 'setter_calls': observed, 'anchors': len(anchors),
          'sdk_method_offsets': {k: offsets[k] for k in ('SetRenderState', 'SetTexture', 'SetTextureStageState')},
          'enum_assertions': len(enums) + 2,
          'limits': 'Static encoded calls; interface inferred from paired layout, actor selector/flags unestablished.'}
(folder / 'facilitator-verification.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8', newline='\n')
print(f'PASS: {len(observed)} setter tuples, {len(anchors)} byte anchors, 3 SDK offsets, {len(enums)+2} enums')
