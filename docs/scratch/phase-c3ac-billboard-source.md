# C3ac — repaired native-compatible preclip billboard source

Packet `build/tmp/c3ac-billboard-source/round-note.md`, including preserved rejected
initial packet. Initial implementation reinterpreted native row-vector12 as
row-major3x4, losing translation and basis values; its synthetic tests shared the
wrong convention. Required all17 retail comparison was also omitted. Repair1
converts native matrices explicitly and returns parsed sourceOffset identity.

## Facilitator check

Read repaired source/header/test; verify translation indices9/10/11 convert to
custom input rows correctly. Native identity+translation regression now uses
(10,20,30), and noncommuting caller matrices test order. Permanent optional-install
test embeds all17 C3o XY/Z/UV tuples; optional means skip only without installation.
Worker's actual old-fail/new-pass and own-Y mutation logs are preserved.

Independent C++ parser/traversal driver and Python checker
`build/tmp/c3ac-facilitator-dump.cpp`, `c3ac-facilitator-check.py` supply accepted
reference matrices from their raw fixture, compose actual parsed ancestors, and
compare **17 nodes /340 slot fields** to repaired C3o native_slots. Maximum error
**9.094947017729282e-13**, below frozen1e-9. No worker test constants feed driver.

Private own-Y-only mutation, keeping reciprocal pairing unchanged, fails at
source967A2 with **75.35755695025995** screen-coordinate error. First private
compile lacked src/app include path (not counted as behavioral failure); corrected
command compiled and the actual comparison failed. Restored private source exactly
to production SHA256 `17e8bbcf817f84235d0bf9a8711c18f6ba10249db3514eba5042d5d5f24626ae`.
Final shared-mutex build, explicit-install **443 checks**, and **38/38 CTest**
passed at16:32:43. An initial wrapper command lost nested environment quoting and
ran only build; it is not claimed as test coverage. Repeated with parent-process
environment, requiring visible focused-test and complete CTest output, passed.

Limits: preclip formula/reference-input compatibility only, not native visibility,
clipping/state/texture or pixels. No original starts, attach or native/DLL calls;
no live resource remains. Separate adapter inputs retain documented custom row-major
3x4 representation, so integration must convert its native matrices explicitly.
No production camera constant or scratch-file dependency was added.

Dispatch15:57:12, repair16:14:58, accepted16:33:26 MDT: **36m14s** total,
**18m28s** repair-to-acceptance. Initial and repair count as two run attempts.
