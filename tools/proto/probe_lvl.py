import sys, struct, collections, re
sys.path.insert(0, r'C:\progs\msys2\tmp\claude\C--progs-Lemmings-Revolution\179308a9-b8e4-4c71-a2b9-6db2e9e4196a\scratchpad')
from box import Box

BS = chr(92)
box = Box(r'C:\progs\Lemmings Revolution\lemmings.box')
lvls = [n for n in box.names if n.upper().endswith('.LVL')]

d = box.read('COMPILED' + BS + 'MM_CLIMB_002.LVL')
print('MM_CLIMB_002.LVL  size=%d' % len(d))

# length-prefixed-string walker: at each offset, is there a plausible u32 len + ascii?
def lp_strings(buf):
    out = []
    i = 0
    n = len(buf)
    while i + 4 <= n:
        (L,) = struct.unpack_from('<I', buf, i)
        if 1 <= L <= 120 and i + 4 + L <= n:
            s = buf[i + 4:i + 4 + L]
            if all(32 <= c < 127 for c in s):
                out.append((i, L, s.decode('ascii')))
                i += 4 + L
                continue
        i += 1
    return out

ss = lp_strings(d)
print('length-prefixed ASCII strings found: %d' % len(ss))
print('\nfirst 40:')
for o, L, s in ss[:40]:
    print('  %08x (%3d) %s' % (o, L, s))

print('\nlast 15:')
for o, L, s in ss[-15:]:
    print('  %08x (%3d) %s' % (o, L, s))

# where do strings live? histogram by 64KB bucket
buck = collections.Counter(o >> 16 for o, _, _ in ss)
print('\nstring density per 64KB block: %s' % sorted(buck.items()))

# classify strings
tex = [s for _, _, s in ss if '.BMP' in s.upper() or '.ALF' in s.upper()]
print('\ntexture refs: %d  e.g. %s' % (len(tex), tex[:5]))
sig = collections.Counter(s[0] for _, _, s in ss if s and not s[0].isalnum())
print('sigil first-chars: %s' % dict(sig.most_common(12)))

# look at the big binary region: sample floats
print('\n--- region after the string block ---')
last_str_end = ss[len(ss) // 2][0]
print('midpoint string offset 0x%x' % last_str_end)

# how uniform is the tail? try reading as float triples (vertices?)
import math
def sniff_floats(buf, off, n=12):
    vals = struct.unpack_from('<%df' % n, buf, off)
    return ['%.4g' % v for v in vals]

for off in (0x40000, 0x60000, 0x80000, 0xA0000):
    if off + 48 < len(d):
        print('  @0x%06x floats: %s' % (off, sniff_floats(d, off)))

print('\n--- all .LVL sizes ---')
sz = sorted((box.sizes[box.index[n.upper()]], n.split(BS)[-1]) for n in lvls)
print('  min %s' % str(sz[0]))
print('  max %s' % str(sz[-1]))
print('  median %s' % str(sz[len(sz) // 2]))
