# B2l — reference-specific menu image admission, not live entry

2026-10-27. Campaign attempts10/12, tooling attempts1/2; historical B2a-c cap3
retained. Changed outcome-driven approach replaces unsafe heap-highlight discovery
with image admission for the next ordinary navigation observation. No native start.
Packet: `build/tmp/b2l-menu-image-qualification/round-note.md`; initial source and
failures retained under its repair1 directory.

`tools/harness/menu_image_qualification.py` identifies a single row from exact
orange(224,128,0)/normal(224,10,0) populations in five fixed bands. Its new
alignment guard requires exact reference RGB outside `[160,320,480,468)`;
RETURN additionally requires highlight0, identical paired frames, and caller-proven
menu identity/focus. Callers must provide fresh unmodified captures. This does not
prove freshness, live compatibility, universal menu identity or complete alignment
inside the excluded envelope. The retained reference is required at runtime;
missing/wrong-sized reference fails closed. No rescaling/fitting is performed.

Initial worker tests passed but only tested a639x480 crop. Facilitator's actual
same-size shifted/cropped640x480 controls admitted RETURN under four offsets.
That candidate was rejected; repair added the strict reference-specific exterior
guard without relaxing color floors. Arbitrary synthetic canvases no longer count
as valid geometry; synthetic row variants preserve the retained exterior. These
are SOURCE classifier tests, not five observed native highlight states.

## Facilitator check

Independent literal-color scan reproduces original row counts1399orange then
1603/1488/863/645normal. Own32highlight-subset variants produce all five single-row
indices, reject zero/multiple rows, and admit RETURN only for subset1. Eleven
same-size signed/mixed translation controls refuse; focus/identity/unstable-frame
controls refuse. Own isolated alignment-call bypass reintroduces unsafe admission
and fails the independent checker; exact source restored with `cmp` and restored
check passes. Worker six-method suite passes after restoration.

Original byte re-read `0x489ADC..0x489B36` corroborates the224/128/0 versus224/10/0
setter operands; row/layout provenance remains accepted B1k/B1y and measured
reference geometry, not new native measurement. Check artifacts:
`build/tmp/integrated-retail-2026-10-27/check-menu.py`, `menu-initial-independent.log`,
`menu-mutant-alignment.log`, `menu-restored-independent.log`, `menu-colors-bytes.log`.
Prior/current app tree has no render changes; final full48CTestPASS64.81s after
restoration (`suite-menu-final.log`). C3/C1 fixed gates unchanged,
not freshly improved. No references/masks/gates changed. Configured worker
independently confirmed terminal; no game/app/debugger/Frida/task observed.

**Next production decision:** one concrete ordinary B2 navigation/map-table
observation using the actual utility. Its exact exterior requirement may refuse a
fresh native menu (cursor, animation or other differences); such refusal must retain
screenshots and park this tool route, not relax thresholds or spend a third tooling
attempt. Full B2/B3 gates remain unpassed; cursor live arm and Phase D parks stand.
