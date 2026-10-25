# C1bb: the installed alpha-upload patch bypasses native format selection

2026-10-25, `c1bb-derived-format`, first value-framed question round under the
DERIVED directive. OpenCode `disasm-agent`, session `ses_e8c31d3d1ffeSS4IPwDispD1up`.
Dispatch 10:04:03 MDT. Zero stock starts, attaches, install writes or production edits.
Worker packet: `build/tmp/c1bb-derived-format/round-note.md`; reproduction and full
address windows are in that packet. This is static READ plus documented DERIVED
interpretation, **not observed execution**.

## Result

The proposed install-specific A4R4G4B4 conclusion is falsified. The exe's IAT
`0x4B0020` names **patch.dll!DirectDrawEnumerateA**. `0x472FA8..0x472FB2`
pushes the texture, then 0 and 1, calls thunk `0x4A6EF0`, and calls its return.
Installed patch SHA-256 is
`c9a09caaa09e89df9bcbb7cb639688916ebf58d2b96ed676d9f58ceab64405ad`.
At preferred base `0x6CBC0000`, export RVA `0x1AE9` returns RVA `0x1879` for
argument 1 (`0x6CBC1AF5` compare, `0x6CBC1AFB` pointer, `ret 8` at `0x6CBC1BAB`).

The hook reads `object+0xC` at `0x6CBC1926`. A nonzero alpha pointer enters
the patch upload; zero returns 0 and permits the native continuation. The
successful flag-8 `^tab` load allocates that pointer (`0x472361 89 48 0c`).
The patch's request writes at `0x6CBC19B7..0x6CBC19F2` encode:

```
size 32; flags 0x41; FourCC 0; bpp 32;
R 0x00FF0000; G 0x0000FF00; B 0x000000FF; A 0xFF000000
```

**DERIVED A8R8G8B8 request**, from the documented DDPIXELFORMAT DWORD layout
(`dwRGBBitCount +0xC`, R/G/B/A masks `+0x10/+0x14/+0x18/+0x1C`):
<https://learn.microsoft.com/en-us/windows/win32/api/ddraw/ns-ddraw-ddpixelformat>.
The worker's Microsoft SDK/header packet supplies DDSURFACEDESC2 and COM offsets.
This is a requested format conditional on successful API calls, not a measured
returned surface. No pixel-fit selection was used.

Normal completion returns 4 at `0x6CBC1ADA`; exe `0x472FB7 83 f8 04`,
`0x472FBA 75 05`, `0x472FBC e9 57 0b 00 00` jumps to epilogue `0x473B18`,
before native GetObjectA, context +6, callback and 16-bit shift. The original
HBITMAP is retained; the patch converter creates a separate temporary 32-bpp DIB.
This predicts native shift silence for successful alpha uploads but does not
retroactively prove the cause of any earlier empty trace or crash.

## Native counterfactual

Conditional on reaching native code and successful GetObjectA returning the
24-bpp BITMAP, context +6 is 0: BITMAP32.bmBitsPixel is WORD +0x12 and the native
comparison is <=8. The entire ordered 20-record c1as `$LemBits0` list then admits
only line 29, A4R4G4B4 (alpha F000). It is **not a measured ^tab enumeration**,
nor the successful installed patch route. Do not implement 4-bit alpha from it.
The API-success condition matters; the native code does not test GetObjectA's
return before reading the buffer.

## Facilitator check

`python build/tmp/c1bb-facilitator-check.py` independently reads 3,444 complete
retained assembly lines and all 72 byte anchors against fresh PE raw bytes,
checks the import DLL and export RVA, decodes the dispatch/bypass targets and
request immediates, and separately filters all 20 saved format records. I read
the complete hook and converter windows and checked Microsoft's DDPIXELFORMAT
definition. The separate predicate yields only line 29 for the counterfactual.
Changing only the request bit-count immediate from 32 to 16 changes the decoded
request; the unchanged installed bytes are re-read afterwards. Worker replay
F000->8000 removes the only native keep and restores the complete trace.

Narrowing: these are byte/branch and replay checks, not behavioral or pixel
acceptance. An attempted Unicorn execution check could not run because this
Python has no `unicorn`; I installed nothing and replaced it with explicit
static checks, not a claimed execution. No production behavior changed.
Fresh wrapper build succeeded and **34/34 CTest passed**. Independent process
check found no Lemmings/Frida process; install digests agree with the worker
packet. Earlier retractions remain intact. Gate and frozen footprint unchanged.

Next value: the patch converter/resizer's exact uploaded **32-bit** words for
`^tab` and `^tab_selected`, then a plate render and unchanged C1 score. This is
a corrected implementation route within C1, not a gate amendment.
