# Texture pixel-format selection: 473B1E

Authority is installed `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The complete routine is 636 contiguous bytes. Exact bytes, PE file offset,
digest, REA identity and pseudocode are in `texture_pixel_format_metadata.json`;
`texture_pixel_format.asm` independently records objdump disassembly.

This callback is passed by texture graphics initializer `472F99` to device
enumeration slot +20. The recovered function accepts the original eight-word
pixel-format record and an explicit selection record, using native pointers
on the host. Unknown bytes +09..0B are retained.

## Supported behavior

* Null candidate/context and candidates with flags masked by `0xE0000` are
  rejected with enumeration status 1.
* Nonzero selection byte +06 requests palette formats. Candidate flag `0x20`
  accepts immediately, copies all 32 bytes, sets found+08 to 1, and returns 0.
  Other selection criteria and alpha-bit count are unchanged in this branch.
* Nonpalette candidates must match the requested bit count and exact byte +07
  against the canonical boolean `fourcc != 0`. Noncanonical byte values do not
  match. With +04 zero, byte +05 must exactly match candidate flag bit zero.
* Nonzero +04 requires flag bit zero and more than one set bit in the alpha
  mask. Accepted formats update alpha-bit count+0C with that population count.
* Once found+08 is nonzero, a new RGB-mask union must be at least the previous
  union under **signed 32-bit comparison**. Equal unions replace the previous
  format. A selected RGB format returns 1 so enumeration continues.

The output may be the candidate itself. Other overlapping selection/format
storage is excluded. A selected-format pointer must be valid whenever read or
written. The routine does not validate DDPIXELFORMAT size or mask consistency.
No graphics or enumeration driver is implemented here.

## Verification

Static inspection covers the whole function. x86 and x64 unit runs each cover
2,457,600 selection cases plus independent assertions for alpha-mask counting,
palette stop status, signed mask ordering, and preservation of unknown bytes.
The systematic cases vary noncanonical byte values 0/1/2/255, masks with
0/1/2/32 set bits, signed RGB-mask boundaries, prior selection, mismatched bit
counts, palette and rejection flags, FourCC presence, null inputs and output
self-aliasing.

The x86 native oracle executes all 636 original bytes unchanged at their
original address. Only the CRT memcpy dependency is adapted to host memcpy.
Both `-O0` and `-O2` builds match all 2,457,600 cases, comparing the full
candidate/output formats, all selection bytes except the host-address-valued
output pointer, and the return status. The runner verifies the complete EXE
digest and the routine's exact mapping and bytes before each comparison.
