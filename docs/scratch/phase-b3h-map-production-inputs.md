# B3h — node-atlas inputs recovered; full-map preparation remains open

2026-10-27. Campaignattempt9, historical map-objectattempt5, explicit changed
approach: reuse B2's observed4B3EB8/ctor49AA90 instead of chasing construction
stamp5. Packet `build/tmp/b3h-map-production-inputs/round-note.md`; eight deliberately
interpreted bodies, no live access or production edits. **Not B3 acceptance.**

The supported prefix is integer **2D surface blits**, not an invented camera/3D
scene. This does not exclude surrounding presentation. Setup binds
`TEXTURES\LEVEL MASK.BMP` separately from source atlas
`TEXTURES\LEVEL SELECT OBJECTS.BMP`: base archive ordinals693/695, offsets44507865/
45168491, sizes4515/7880; neither exact name occurs in patch.box. Both decode640x480,
so dimensions alone do not identify the correct source.

Ctor copies103 records of28bytes from4B2E00; draw49B8BA visits102. Base byte at
4B3950+12*i is0 for88 entries and1 for14. For explicit effective state s,
`j=base+(s==2?2:s==3?4:0)` selects a six-int record at4B3E10+24*j:
`sx,sy,w,h,ax,ay`. Source rectangle is `[sx,sy,sx+w,sy+h)`, destination `(x-ax,y-ay)`.
Only records0–5 are selected by this loop; neighboring row6 serves another setup
operation. Constructor node0 `(209,205)` conditionally yields `(197,192)`,34x30;
this is not a measured live node position or a fitted click coordinate.

Zero source key / flags8 and keyed wait blit flags11 are READ requests. MinGW
ddraw.h slots74/1C and constants support conditional DERIVED SetColorKey/BltFast
semantics; interface binding, upload/format and HRESULT success are not observed.
RGBA black-key replay is SOURCE, not native final pixels.

**Critical open boundary:**49B8BA first calls unread49B647 before consuming
`[map+78]` records and destination`[map+3C]`. Constructor state0 is not effective
unlock state. Background/state preparation, surface wrappers49BE30/49BA90 and
inherited presentation remain unclosed. `[map+64]=32` is not established as a
background resource. Never cast our Progress enum into native states2/3.

## Facilitator check

Own PE parser authenticates authority/raw bounds and re-reads all eight complete
windows. Actual node-loop bytes corroborate bound66hex, state+2/+4 branches,
rectangle loads, anchor subtraction and blit operands. Independent archive-header
parser joins exact native pointer strings to both installed payload hashes and
excludes patch matches. Raw103records/102base bytes/sevenrectangles agree;510
conditional tuples independently recomputed, not510 native observations.

Separate compiled caller of unchanged production asset readers reproduces both
complete RGBA streams exactly against retained decoder exports. Isolated atlas→mask
input substitution retains640x480 but changes866448 bytes and fails the exact RGBA
check; normal binding restores exact output. No native install bytes changed.
Checks/logs in `build/tmp/integrated-retail-2026-10-27/map-check/`, `check-map.py`.
Full48/48CTest/fixed C3/C1 checks already ran on this unchanged production tree;
no pixel/gate movement or new reference. Worker configured Sol/high independently
confirmed succeeded/idle; static REA closed; no live resources/restoration owed.

Next concrete render decision: close the **whole49B647 background/state preparation
boundary** and source/destination wrapper binding, then build the supported map
composition—not another creation-counter search or an unused node-only component.
Two unlock-state comparisons and normal level entry remain the signed B3 gate.
