"""Reader for the LEMBOX archive used by Lemmings Revolution.

Layout (all little-endian):
    char[6]  "LEMBOX"
    u32      count
    u32      nameBlockBytes
    u8[nameBlockBytes]   count x (u32 len, char[len])    -- no NUL terminators
    u32      count ; u32[count] offsets  (absolute, from start of file)
    u32      count ; u32[count] sizes
    ... payload ...
"""
import struct, os, collections


class Box:
    def __init__(self, path):
        self.path = path
        self.f = open(path, 'rb')
        magic = self.f.read(6)
        if magic != b'LEMBOX':
            raise ValueError('not a LEMBOX file: %r' % magic)
        count, nameblk = struct.unpack('<II', self.f.read(8))
        raw = self.f.read(nameblk)
        o = 0
        self.names = []
        for _ in range(count):
            (n,) = struct.unpack_from('<I', raw, o); o += 4
            self.names.append(raw[o:o + n].decode('latin-1')); o += n
        self.name_bytes_used = o
        (c1,) = struct.unpack('<I', self.f.read(4))
        self.offsets = list(struct.unpack('<%dI' % c1, self.f.read(4 * c1)))
        (c2,) = struct.unpack('<I', self.f.read(4))
        self.sizes = list(struct.unpack('<%dI' % c2, self.f.read(4 * c2)))
        self.data_start = self.f.tell()
        self.count, self.c1, self.c2 = count, c1, c2
        self.index = {n.upper(): i for i, n in enumerate(self.names)}

    def read(self, i):
        if isinstance(i, str):
            i = self.index[i.upper()]
        self.f.seek(self.offsets[i])
        return self.f.read(self.sizes[i])

    def validate(self):
        sz = os.path.getsize(self.path)
        problems = []
        if self.name_bytes_used != len(self.names) and False:
            pass
        if not (self.count == self.c1 == self.c2):
            problems.append('count mismatch %d/%d/%d' % (self.count, self.c1, self.c2))
        prev_end = self.data_start
        gaps = 0
        for i in range(self.count):
            if self.offsets[i] < prev_end:
                problems.append('entry %d overlaps (off %d < %d)' % (i, self.offsets[i], prev_end))
                break
            if self.offsets[i] != prev_end:
                gaps += 1
            prev_end = self.offsets[i] + self.sizes[i]
        if prev_end > sz:
            problems.append('last entry runs past EOF: %d > %d' % (prev_end, sz))
        return problems, gaps, sz, prev_end


if __name__ == '__main__':
    import sys
    b = Box(sys.argv[1] if len(sys.argv) > 1 else 'lemmings.box')
    print('entries      : %d' % b.count)
    print('data starts  : 0x%x' % b.data_start)
    probs, gaps, sz, end = b.validate()
    print('validate     : %s' % ('OK' if not probs else probs))
    print('gaps         : %d ; payload ends %d ; file size %d ; slack %d'
          % (gaps, end, sz, sz - end))

    ext = collections.Counter(n.rsplit('.', 1)[-1].upper() if '.' in n else '(none)'
                              for n in b.names)
    print('\nextensions   : %s' % dict(ext.most_common(15)))
    top = collections.Counter(n.split(chr(92))[0].upper() for n in b.names)
    print('top dirs     : %s' % dict(top.most_common(15)))

    print('\nmagic sniff per extension:')
    seen = set()
    for i, n in enumerate(b.names):
        e = n.rsplit('.', 1)[-1].upper() if '.' in n else '(none)'
        if e in seen:
            continue
        seen.add(e)
        d = b.read(i)[:16]
        print('  %-7s %-52s size=%-9d %r' % (e, n[:52], b.sizes[i], d[:12]))
