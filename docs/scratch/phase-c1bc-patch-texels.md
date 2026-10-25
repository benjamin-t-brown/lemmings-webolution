# C1bc: conditional patch texel streams, 72x32 to 128x32

2026-10-25, `c1bc-patch-texels`, texel-word question round 1. Worker OpenCode
`disasm-agent`, session `ses_e8c186840ffexpZFloFJer1PNq`; dispatched 10:31:43 MDT.
Full packet and exact commands: `build/tmp/c1bc-patch-texels/round-note.md`.
Zero stock starts, installs/writes, production edits or observed uploads.

## Accepted conditional derivation

Installed patch C1bb identity unchanged. Converter RVA `0x14C6` gets the original
DIB dimensions, creates a separate 32-bpp BI_RGB DIB, SRCCOPYs source RGB, masks
its unused high byte (`RVA 0x16AB`), then inserts ALF bytes shifted by 24
(`0x16C8..0x16F5`). Positive-height DIB reads reverse rows; ALF addressing is
**y*72+x without a flip**. `^tab` RGB is all zero; `^tab_selected` RGB all 255.

The resizer at RVA `0x1254` grows dimensions to powers of two: **128x32** here.
Coordinates are binary32 stores of `x/(128-1)*(72-1)` and `y/(32-1)*(32-1)`.
It interpolates four channels independently, endpoint-aligned, then truncates
each to a byte (`0x1469..0x1476`, temporary x87 truncate CW). It does not
premultiply or quantize alpha to four bits. The original plate geometry is
unchanged. Native default UV writes in `0x44D880` use the **full [0,1] image**,
not a crop to 72/128. `adapter.json` carries the complete dimensioned streams.
Hook RVA `0x1AAA` copies width*height*4 contiguous bytes, ignoring returned pitch.

## Conditions, not new measured facts

These are **DERIVED conditional successful-upload streams**, not measured original
surfaces. The patch inherits arithmetic PC/RC. Nearest p24/53/64 yield the same
complete `^tab` stream; selected p24 differs from p53/64 at exactly `(11,24)`:
alpha 150 versus 151. No variant was chosen by fit. Other rounding variants are
retained. GDI completion before CPU DIB access and returned surface format/pitch
remain unobserved. Normal row interpretation assumes A8R8G8B8 and pitch 512;
the actual linear copy must not be silently repaired into a pitched copy.
Microsoft CreateDIBSection/GdiFlush contracts require completion, and the patch
does not explicitly call GdiFlush. This is a missing guarantee, **not an observed
race**. The packet supplies documented GDI, BITMAP, and DDSURFACEDESC2 links.

## Facilitator check

`python build/tmp/c1bc-facilitator-check.py` freshly authenticates **2,554 assembly
instruction lines** against both PE files and verifies member/header/digest
facts. A separate direct Python binary32-coordinate/binary64-arithmetic loop
reconstructs every p53-nearest word for both assets without importing the worker
model. Hashes:

- tab: `1e285c498e5ae2a4d2deae8029fdbc632d327d5ead5b86d0c35295f254b3cfdf`
- selected: `369ae5d52dc97b26ebb616420ffaa66984e4c8bc29d1c3bd87fb4d5d151375b3`

I read the full resizer and the converter/mask/ALF loop, checked sample rows and
the saved p24 discrepancy. Flipping only ALF row addressing in my checker changes
both full outputs; restoring it restores exact hashes. Worker separately mutates
ALF byte0 on an owned disk copy and restores exact source/output bytes. These are
model-output discriminators, **not a production behavioral/pixel test**.
I did not independently reproduce every directed-rounding output; those remain
candidate sensitivity data. Fresh wrapper rebuild and **34/34 CTest passed**.
No live process or writes were created; original install hashes remain unchanged.

Next: draw plates in the opt-in production fixture and **score** the frozen C1
footprint/views. Use an explicitly declared conditional arithmetic/layout case,
report sensitivity rather than choosing a case by score. No pixel pass or gate
acceptance is claimed here. Runtime-state questions need gate-error justification
before spending a start; an unread condition alone is not a stop.
