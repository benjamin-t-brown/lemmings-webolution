# C3u — conditional scene gates and load associations

2026-10-25. READ + conditional DERIVED, not a historical pause trace.
Packet: `build/tmp/c3u-pause-scene-reachability/round-note.md`.

## Accepted scope

G=0x4C6A08. At45DD8E..45DD9D a **u32 G+0** mode is cached and compared
with2. Only mode2 takes the45E0E6→441845 edge. Mode3/4 take45DDBE/45DF5C;
other values take45E2BA. This says nothing about transitive calls on other arms.
At441853 **u8 G+5139** (4CBB41) controls this scene body: zero exits441AD4,
nonzero reaches primary holder dispatch441879 and tube441884.
The separate pause ui_state atG+572C is not consumed by these gates.

Primary source Object59637 is associated, on successful load, with holder
G+5150, root pointer atG+5154:437BA3→47496D→4749A7 calls root vtable**+4**
before the tube. Secondary Object91775 is associated with holderG+5194/root
G+5198;44199B→437BB9 tests **u8 G+51E8** (4CBBF0) then47491B→474955 calls
root vtable**+8**, after tube/object loops. **Slot+8 is not accepted as equivalent
to slot+4 rendering**; its concrete per-type effect needs the next bounded read.

437D48 loads primary holder+10, consumes/normalizes presence into receiver+A8,
then conditionally loads secondary+54. 474A28 stores factory result at holder+4.
LVL trailer9176E..91774 is01010000000001: mirror1, animation-present1,
zero u32 animation count, second-presence1. That is conditional **load** state,
not the old pause byte. Model type words9CC98/A0CA8 are0; names/header fields
lead to Object9CCC4/A0CD0 for Entrance/Exit. Their successful model construction
uses the separate model+B0 holder and model+B4 root, not the primary scene holder.
Entity+44 supplies model;451865 brackets holder draw with model+F8 depth bias.
Tube collection membership, entity+28, distance/angular interval and model
nonnull remain additional conditions. CSV Entrance/Exit columns mean only
**route eligible**, not called, emitted or visible.

Initializer444C1B writes enable0, loader tail4443BC writes1; entry45CA69 then
45CAB3 orders loader before postload444D1E, whose445207 writes mode2. Thus
successful path derives(2,1). This does not supply a last-writer timeline for
repair4. Reset stores and aliased/computed writes cannot be excluded by literal
operand scans. Historical mode/enable/secondary/heap-holder values stay **unknown**.
Retained capture only records ui_state1, camera and actor/control inputs. A new
pause observation cannot retroactively become memory evidence for that PNG.

## Facilitator check

`python -B build/tmp/c3u-facilitator-check.py` independently maps original PE raw
sections, authenticates all46 retained windows and writes fresh Capstone listings.
Re-read consumer widths/branches, both holder vcalls, loader presence/holder
stores, source-model route, tube entity predicates and successful-load ordering.
The initial facilitator anchor incorrectly used an8-bit displacement encoding;
fresh45DD88 disassembly corrected it to83BD60FFFFFF02 before acceptance.
No BSS game-state byte is inferred from file data (raw end4C6000).

Separate arithmetic reproduces all32256 conditional CSV rows,4590 primary-eligible
cases; source tags, trailer and model type/name offsets independently match raw
LVL. The candidate gate is used only as system under test: changing the single
private enable byte1→0 changes four eligibility outputs true→false; restore
returns all four. Secondary1→0 changes only secondary eligibility. No install,
LVL, captures or live inputs written. Effective geometry/GPU visibility and
slot+8 downstream submission are outside this check.

Fresh wrapper build and **35/35 CTest passed**; source changes from sibling C3x
remain independently unaccepted, and are not part of this evidence-only round.
No Lemmings process or owned live resources. Correct reference remains repair4;
baseline48188934/235008000=0.2050523, still above0.02. No render improvement.

Dispatch14:53:56; worker complete15:12:47 (18m51s); accepted15:21:16 MDT,
**27m20s** dispatch-to-acceptance. Next: bounded slot+8
effect read; verify C3v repaired mock safety and C3x strict reader separately.
