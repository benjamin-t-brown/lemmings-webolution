/*
 * Reference LRZ decoder: calls the original game's own routine.
 *
 * "Lemmings Revolution.exe" has its relocations stripped, so it only ever
 * loads at 0x400000 and every internal address is fixed. The decompressor at
 * .text:00467DE8 is self-contained hand-written assembly -- it touches only
 * its arguments and a few globals in .data, and calls nothing. So we can map
 * the image at its preferred base and call straight into it.
 *
 *     int __cdecl lrzDecodeChunk(const void *src, void *dst,
 *                                int packedSize, int unpackedSize);
 *     returns the number of bytes produced.
 *
 * `src` points just past the 5-byte chunk header.
 *
 * This exists to validate src/assets/Lrz.cpp. It is a development tool, not
 * part of the game: it is 32-bit, Windows-only, and needs the original exe.
 *
 * Usage: lrz_oracle <Lemmings Revolution.exe> <chunk-file> <unpacked> <out>
 *        where <chunk-file> is a chunk body (header already stripped).
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define IMAGE_BASE   0x00400000u
#define DECODE_ADDR  0x00467DE8u

typedef int(__cdecl *lrz_fn)(const void *src, void *dst, int packed, int unpacked);

static int mapImage(const char *path) {
  FILE *f = fopen(path, "rb");
  if (!f) { fprintf(stderr, "cannot open %s\n", path); return 0; }
  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fseek(f, 0, SEEK_SET);
  unsigned char *raw = malloc(size);
  if (fread(raw, 1, size, f) != (size_t)size) { fclose(f); return 0; }
  fclose(f);

  IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER *)raw;
  IMAGE_NT_HEADERS32 *nt = (IMAGE_NT_HEADERS32 *)(raw + dos->e_lfanew);
  if (nt->OptionalHeader.ImageBase != IMAGE_BASE) {
    fprintf(stderr, "unexpected image base 0x%x\n",
            (unsigned)nt->OptionalHeader.ImageBase);
    return 0;
  }

  SIZE_T imageSize = nt->OptionalHeader.SizeOfImage;

  /* 0x400000 must be free in this process. Build with a high image base so we
     are not sitting on it ourselves (see build.sh). Do not try to release a
     reservation found here -- it belongs to the loader, and freeing it
     crashes the process. */
  void *base = VirtualAlloc((void *)IMAGE_BASE, imageSize, MEM_RESERVE | MEM_COMMIT,
                            PAGE_EXECUTE_READWRITE);
  if (base != (void *)IMAGE_BASE) {
    MEMORY_BASIC_INFORMATION mbi;
    DWORD err = GetLastError();
    fprintf(stderr, "could not reserve 0x%x size 0x%x (got %p, err %lu)\n", IMAGE_BASE,
            (unsigned)imageSize, base, err);
    if (VirtualQuery((void *)IMAGE_BASE, &mbi, sizeof mbi))
      fprintf(stderr, "  at 0x%x: alloc %p size 0x%x state 0x%lx\n", IMAGE_BASE,
              mbi.AllocationBase, (unsigned)mbi.RegionSize, mbi.State);
    return 0;
  }

  memcpy(base, raw, nt->OptionalHeader.SizeOfHeaders);
  IMAGE_SECTION_HEADER *sec = IMAGE_FIRST_SECTION(nt);
  for (int i = 0; i < nt->FileHeader.NumberOfSections; i++) {
    if (sec[i].SizeOfRawData)
      memcpy((unsigned char *)base + sec[i].VirtualAddress,
             raw + sec[i].PointerToRawData, sec[i].SizeOfRawData);
  }
  free(raw);
  return 1;
}

int main(int argc, char **argv) {
  if (argc < 5) {
    fprintf(stderr, "usage: %s <game exe> <chunk body> <unpackedSize> <out>\n", argv[0]);
    return 2;
  }
  if (!mapImage(argv[1])) return 1;

  FILE *f = fopen(argv[2], "rb");
  if (!f) { fprintf(stderr, "cannot open %s\n", argv[2]); return 1; }
  fseek(f, 0, SEEK_END);
  long packed = ftell(f);
  fseek(f, 0, SEEK_SET);
  unsigned char *src = malloc(packed);
  if (fread(src, 1, packed, f) != (size_t)packed) { fclose(f); return 1; }
  fclose(f);

  int unpacked = atoi(argv[3]);
  unsigned char *dst = calloc(unpacked + 64, 1);

  lrz_fn decode = (lrz_fn)DECODE_ADDR;
  int produced = decode(src, dst, (int)packed, unpacked);

  fprintf(stderr, "produced %d of %d bytes\n", produced, unpacked);
  if (produced <= 0) return 1;

  /* Dump the decoder's working state so a port can be diffed against it:
     markers at 0x4C0EA2, per-tree table pointers at 0x4C0EA6, node arena
     from 0x4C11A4. */
  {
    const unsigned char *marker = (const unsigned char *)0x4C0EA2u;
    const unsigned int *table = (const unsigned int *)0x4C0EA6u;
    fprintf(stderr, "markers: %02x %02x %02x\n", marker[0], marker[1], marker[2]);
    for (int t = 0; t < 3; t++)
      fprintf(stderr, "tree%d root offset %d\n", t,
              table[t] ? (int)(table[t] - 0x4C11A4u) : -1);
    const unsigned char *arena = (const unsigned char *)0x4C11A4u;
    fprintf(stderr, "arena:");
    for (int i = 0; i < 96; i++) fprintf(stderr, " %02x", arena[i]);
    fprintf(stderr, "\n");
  }

  FILE *o = fopen(argv[4], "wb");
  if (!o) { fprintf(stderr, "cannot write %s\n", argv[4]); return 1; }
  fwrite(dst, 1, produced, o);
  fclose(o);
  return produced == unpacked ? 0 : 1;
}
