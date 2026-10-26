# Plan history — Phase C

Moved verbatim from `PLAN.md` on 2026-10-24 (user, overlord review 2026-10-24). Only link
paths were rebased. The live milestone table, exit gates, status cells and user decisions
are in [the plan](../../PLAN.md#phase-c--the-in-level-hud). New round notes are appended
under the milestone they serve, newest last.

## Phase C — the in-level HUD

The skill panel art and counts, and the counters the engine names itself: `^saved`, `^out`,
`^time`, the icon sets `GreyIcons` / `BlackIcons` / `AdditiveIcons`, and the cursor set
(`_curLem`, `_curNoLem`, `_curEnt`, `_curExit`, and the left/right variants). C1a identifies
all physical panel indices: 0 climber, 1 floater, 2 bomber, 3 blocker, 4 builder,
5 basher, 6 miner, 7 digger, 8 fast-forward, 9 Armageddon (READ, not new live assignments).

The ten skill-panel buttons (x `[18,90)`, objects at `0x4C9208`) are excluded from the A7
gate for the same reason and are drawn here.

The **Paused** caption panel (`[277,422,363,454)` on the 640x480 frame) belongs here. A7c–A7l
traced it without finding its draw owner, parked 2026-10-21: `STR_PAUSED` key at `0x4C3C18`;
`G+0x25C68` list drawn by `0x44E150` only while pause state is 1; rectangle
`G+0x26000` via `0x47AD8C`, whose top edge scales with `[G+0x25FF8]` and
`[G+0x25C64] / 1000.0`, the latter a tick delta or a conditional `100`. Resume from a
live read of `[G+0x25C64]` at pause, in Phase C. Evidence:
[A7c](../scratch/phase-a7-pause-caption-native.md) through
[A7l](../scratch/phase-a7-25c64-writers.md).

**C2a: `0x44CD06` fills two indexed lists and a separate timer**
([check](../scratch/phase-c2a-pause-timer.md)), 2026-10-22, round 1 of the
pause-caption question. This is groundwork. The C2 gate was not attempted.
All three lists use `ecx = [0x4CBF78]`. `H+0x758` gets string index `0x44`
at horizontal 520, vertical 15. `H+0x77C` gets index `0x45` at horizontal
520, vertical 45. `H+0x224` formats `[0x4CBAAC] / 60` as `"%02d:0%d"` when
the remainder is below 10, otherwise `"%02d:%02d"`, at horizontal 560,
vertical 430. The second divide reloads the same dword. The binder at
`0x486C4E` registers that font object with table `0x4BF968` and the name
`"^Font"`. The in-range characters for indices `0x44` and `0x45` are a
runtime lookup; the file fallback is `"Unknown String"`, and these bytes do
not say the text is `Paused`. `[0x4E33C8] == 1` skips the timer submit and
is not identified as pause state. No stock start.

**C2b: indices `0x44` and `0x45` are `Out` and `Saved`**
([check](../scratch/phase-c2b-pause-index-text.md)), 2026-10-22, round 2.
The file language slot `[0x4C3434]` is 2, which selects `Uk` and
`Language\Uk.ltx`. Both `LANGUAGE\UK.LTX` copies store `Out` at index `0x44`
and `Saved` at index `0x45`. The exe fallback remains `"Unknown String"`.
These two lists are the counters, not the word `Paused`. A later store can
change the language slot; the file image selects English. The C2 gate was
not attempted.

**Milestones and exit gates, drafted 2026-10-21 (CEO review), signed off as drafted by the
user 2026-10-22.**

| Milestone | Status | Exit gate |
| --- | --- | --- |
| C1 skill panel | indices identified; icon-draw mechanism live-read, redirected, parked (Phase E); C1g (2026-10-24) READ the base icon draw: skill-panel object `0x4C91D8`, `0x45B639` -> `0x47A75D`, cell by animation name, not `widget_index+8` ([note](../scratch/phase-c1g-base-icon-draw-read.md)); C1h READ the scale mode (`S = [0x58D53C] - [0x58CB38]`) and the `Lo`/`Hi` budget loop ([note](../scratch/phase-c1h-icon-placement-count-read.md)); `[[0x4CBFB0]+0x10]`, the sheet frame values and the count font and colour UNREAD. C1i spent a start and the menu entry failed before any read ([note](../scratch/phase-c1i-icon-live-read.md)); C1k made the entry observable and C1l (2026-10-24) read `[levelScreen+0x10]` = 0, the `0x4E335C` / `+0x1FC` handles, all ten frame records and the count font objects live ([note](../scratch/phase-c1l-icon-live-read.md)); C1m (2026-10-24) READ handle 6 = `Lemmings` + `Hi` or `Lo` (`Sprites\\LemmingsHi.spr` / `...Lo.spr`; the choice is a video-memory query, value UNREAD; [note](../scratch/phase-c1m-sheet-file-read.md)); C1n READ the two-digit count text and that the font is `_FontSmall` (archive ordinal 656; pixels and tint UNREAD; [note](../scratch/phase-c1n-count-font-read.md)); C1o proved Frida attach through the REA CLI; C1p (2026-10-24) parsed `LemmingsHi.spr` offline and matched c1l's live frame records 7/7 (Hi, not Lo; [note](../scratch/phase-c1p-spr-sheet-read.md)); C1q decoded the `_FontSmall` bitmap (64 x 16, 4 bpp, white digits; tint writers unread; [note](../scratch/phase-c1q-font-bitmap-read.md)). C1r (2026-10-24) decoded the sheet members through `0x467DE8` ([note](../scratch/phase-c1r-sheet-pixels-decode.md)); C1s measured the ten draw positions and white count quads ([note](../scratch/phase-c1s-frida-icon-hook.md)); C1t drew the icons and two-digit counts and the gate fails ([note](../scratch/phase-c1t-q3-panel-render.md)): scorer `skill_mae` 0.10767973856209151 (`initial`, 15157/140760) and 0.0554187566499468 (`MM_DIG_000`, 3646/65790), both above 0.02 and equal to the pre-panel figures. Blend, colour key and the button plates stay unread. C1u (2026-10-24) committed the icon and glyph footprint and scored the saved C1t renders ([note](../scratch/phase-c1u-footprint-mask.md)): `initial` render 0.22416530739836685 (880240/3926745) and no-panel 0.3317052673397432, both above 0.02. Plates are empty. `MM_DIG_000` has no named stock frame. C1v (2026-10-24) READ the ten plate quads ([note](../scratch/phase-c1v-plate-draw-read.md)): `0x45B66A call 0x414920` on `0x4C9208+i*0x1B0`, box x `[18, 90]` y `[12+40*i, 12+40*i+32]`, sheet name `^tab`. A zero `[this+i*4]` skips the text submit. The `^tab` pixels, colour key and blend stay unread. C1w (2026-10-24) added the ten plate boxes to the footprint and rescored the saved renders ([note](../scratch/phase-c1w-plate-footprint.md)): `initial` render 0.29181384257636983 (5171522/17721990) and no-panel 0.315641979258537, both above 0.02. Icon and glyph edges are unchanged. C1x (2026-10-24) read the two named sites ([note](../scratch/phase-c1x-colour-blend-read.md)): `0x47218A` is the texture-loader entry, not a colour-key test, and `0x46D5F3` is `call [ecx+0x70]`, not a blend-mode write. The `^tab` pixels stay unread. Part 2 is at its cap, so the render was not started. The milestone is not accepted | Panel indices 2, 5, 6, 8, 9 identified (jump table `0x0045A4DD`). The ten buttons and their counts render inside `x [18,90)` ≤ 0.02 against the A7p/A7q captures, with the A7 panel exclusion removed. **Measured over the panel footprint** (user, overlord review 2026-10-24): the union of the ten button plates, icon rectangles and count-glyph rectangles at their read positions, fixed and committed before any render it scores. Not the C1b `baseline != no-cell-routes` mask, which meets the panel in 0 pixels (C1t). **Gated views** (user, overlord review 2026-10-24, second amendment): A7p `zoom1`, `elev_up` and `elev_down`, each ≤ 0.02, rendered with their A7p camera params; `initial`, the other A7p views and `MM_DIG_000` are scored and reported, not gated, until C3 draws the backdrop behind the panel |
| C2 counters, cursor, pause caption | **in progress**: counters and timer drawn (C2g; amended gate passes on all ten views, region residual down 27/27; anchor, Out definition and default colour UNREAD); caption panel read (C2e/C2f), measured live (C2h) and drawn (C2i), text drawn (C2j) and `0x46D460` read (C2k: packs a D3D diffuse dword, the blend is the device's; render state unread); with the caption exclusion removed the gate passes on 9 of 10 views, `MM_DIG_000` fails by 2,660; parked in Phase E; cursor not started | `^saved`/`^out`/`^time` and the Paused caption `[277,422,363,454)` drawn. The A7 nine-view + second-level score passes with **no** HUD exclusions. Cursor sprite set matches on hover/no-hover/exit |
| C3 full frame | queued; residual breakdown measured (C3a, 2026-10-23): full frame 0.2098, `rest` 84.7%, skill boxes 11.3%, HUD and caption 2.5%, tube ROI 1.4%, floor ring unbounded, no cause attributed ([note](../scratch/phase-c3a-residual-breakdown.md)). Next: backdrop scenery, entrance and exit models before C1 (user, overlord review 2026-10-23). C3b read the backdrop ([note](../scratch/phase-c3b-backdrop-read.md)): model-driven, LVL-embedded node tree (root `thenull` 933 vertices, sand/sky/hose slots), dispatched before the tube; root world transform, node walk and floor ring UNREAD/UNBOUNDED. C3e read the draw-time callee ([note](../scratch/phase-c3e-scene-callee-read.md)): node matrix composed with the stack top once and nested through children, operand order, child/sibling relation and stack bottom UNREAD; the backdrop question is parked in Phase E. C3d read the call skeleton of the scene walk ([note](../scratch/phase-c3d-scene-transform-read.md)): `0x47496d` pushes `0x58D5A0` and calls `[[this+4]+4]`; the callee and whether root scale 10 reaches children are UNREAD. C3h named the stack bottom ([note](../scratch/phase-c3h-stack-bottom-preflight.md)): element 0 is a per-frame push of `[0x58D3D8]` at `0x45DD7B`, not identity; read-only live script ready, not yet run. C3i read the type-1 draw skeleton ([note](../scratch/phase-c3i-node-draw-read.md)): visibility byte, compose onto the stack top, vertex loop in file order READ; `0x40eb50` convention, per-kind draws, kinds 5/7 and the type-2 load UNREAD, so no render. C3g read the type-1 child load ([note](../scratch/phase-c3g-node-geometry-load.md)): c3d's "thenull 933 vertices" was `sand7`'s mesh; the root scale 10 does not reach Entrance/Exit `Top`; kind-6 record 116 B READ; Exit `cyl2` projects wider than the reference (width gap does not close); draw `0x46c55d`, vertex convention and stack bottom UNREAD. C3j spent the directive's one start on the stack-bottom read and it failed in A7p `enter_level` (`no menu highlight`) before any read ([note](../scratch/phase-c3j-stack-bottom-live.md)); P1-P8 untested, install restored, so the render gate still fails. C3c read Entrance/Exit placement ([note](../scratch/phase-c3c-entrance-exit-read.md)): Entrance `Top` projects within 1 px of the reference on three edges, Exit width disagrees (~15 px projected against ~40), draw order and door state UNREAD C3k settled the stack bottom offline ([note](../scratch/phase-c3k-stack-bottom-offline.md)): nothing writes `[0x58D3D8]` between the push and the scene call, value MEASURED-FROM-REFERENCE from the A1 camera; `0x40eb50` is a row vector times a row-major 3x3. C3l read kinds 5/7 (160/212 B) and the type-2 load ([note](../scratch/phase-c3l-scene-reads.md)) and the scenery record now parses past `0x967a2`; `0x43bd80`, `0x418350`, `0x46d372` and the type-2 draw `0x47d0c0` are UNREAD, so the render did not run. C3m did not name the c3j entry failure ([note](../scratch/phase-c3m-menu-entry-diagnosis.md)). | The A6d24 full-frame score (0.2123 today) ≤ 0.02 on the A6 repair 4 pause with actors, the floor ring included |

**C1 gate measurement amended (user, overlord review 2026-10-24).** C1t showed the scorer could not see the panel: its mask inside the skill bands is x 86..89, the drawn icons x 26..76, overlap 0, so `skill_mae` stayed at the pre-panel 0.1077 / 0.0554. The threshold (0.02), region and captures are unchanged; the pixels scored become the panel's own footprint, built from read geometry (plates, c1s icon positions with the c1p/c1r frame sizes, count glyphs at the c1s text-object positions) and committed before the render. Pixels where a plate is translucent still include whatever the stock frame shows behind it; that share is reported, not masked out.

**C1u footprint baseline (2026-10-24).** The icon and glyph rectangles are committed in `tools/harness/fixtures/c1-footprint/footprint.json` ([check](../scratch/phase-c1u-footprint-mask.md)). Against `original-0.png` the saved C1t panel scores 0.22416530739836685 and the no-panel image scores 0.3317052673397432 (5133 pixels). Plate rectangles are not in the fixture yet. `MM_DIG_000` is reference-unnamed. The milestone is not accepted.

**C1v plate boxes (2026-10-24).** The ten plates are the quads at `0x4C9208+i*0x1B0`, drawn by `0x45B66A call 0x414920` ([check](../scratch/phase-c1v-plate-draw-read.md)). Each box is x `[18, 90]`, y `[12+40*i, 12+40*i+32]`. The sheet name is `^tab` (`0x4C0E00`); its pixels are unread, and the bytes do not say the texels are an oval. A zero count skips `0x44E150`. Colour key `0x47218A` and blend `0x46D5F3` stay unread.

**C1w with-plates baseline (2026-10-24).** The ten C1v boxes are in the footprint ([check](../scratch/phase-c1w-plate-footprint.md)). Against `original-0.png` the saved C1t panel scores 0.29181384257636983 and the no-panel image scores 0.315641979258537 (23166 pixels). Both are above 0.02. Icon and glyph edges are the C1u rectangles. `MM_DIG_000` stays reference-unnamed. The milestone is not accepted.

**C1x colour key and blend (2026-10-24).** `0x47218A` is `push ebp`, the texture-loader entry, and the plate draw does not call it ([check](../scratch/phase-c1x-colour-blend-read.md)). `0x46D5F3` is `call dword ptr [ecx+0x70]` with `ecx = [[0x58CDB8]]`; the method and the blend mode are unread. The plate reaches that call only when four `eax` checks in `0x46D3C1` are nonzero, and those values are unread. Part 2 is at its cap. The `^tab` pixels are still unread, so part 3 was not started. The milestone is not accepted.

**C1 gated views amended (user, overlord review 2026-10-24, second amendment).** The plates are translucent: all 18033 plate-only footprint pixels differ between the eight A7p stock views, and they carry 83% of c1w's 0.2918. On the 4536 scenery-only pixels between the plates, our render differs from the stock frame by 0.48–0.52 on `initial`, `left`, `right`, `elev0_neg` and `canonical` (sand, sky, balloon: parked C3 scenery) and by 0.0183 / 0.0209 / 0.0174 on `zoom1` / `elev_up` / `elev_down` (the tube wall we draw). So the gate is taken on those three views; the threshold, footprint and region are unchanged. The scenery error alone is near 0.02 there, so a pass is not assured. Overlord scripts `build/tmp/overlord-2026-10-24/split.py`, `constancy.py`, `gaps.py`, `views.py` (measured from the saved A7p PNGs; evaluation, not a new capture).

**C1a: physical panel mapping read** ([check](../scratch/phase-c1-panel-indices.md)):
the ten-entry table at `0x45A4DD` is eligibility, distinct from the nine-entry skill
factory table `0x45B570`. Physical index8 bypasses selection and requests a seven-pass
simulation-budget loop; index9 requests Armageddon, not an ordinary per-lemming skill.
Both native paths and the 0..7 factory/constructor mapping were independently re-read
from raw bytes; REA evidence canonically validated. Exact input/stock/timing behavior
and all panel rendering remain unmeasured here. C1's gate is not yet accepted.

**C1b: the ten base icons' sheet was not located (clean negative), round 1 of 3**
([check](../scratch/phase-c1-panel-icon-render.md)): the three sprite-sheet names
LESSONS.md already flagged as unchased — `GreyIcons`/`BlackIcons`/`AdditiveIcons` — each
decode (two independent decoders) to exactly **one** sprite (`bomb` 18x25, `questionmark`
16x20x19, `armageddon` 13x15x18), not ten 72x32 panel-index-ordered icons; the shared
registrar `FUN_0045e51f` only loads handles, with no draw call, corroborating round 23's
`Axe` finding rather than contradicting it. Every other `.SPR`, every `TEXTURES\SHEETS\*`
and bare `TEXTURES\*` entry, and the exe's PE resource table were searched with no
ten-icon candidate found. The actual draw call sits behind an untraced per-skill vtable.
A tested, additive compositing primitive (`src/app/SkillPanel.h`/`.cpp`) is built but not
wired to any guess. The skill-only MAE (reusing A7p's own `verify.py` isolate) fails on
both fixtures: `initial` 0.10767973856209151, `a7q` 0.0554187566499468 — both independently
reproduced and the `a7q` figures (86 pixels, 3646 error units) match
[A7q's own already-landed numbers](../scratch/phase-a7-second-level-mask.md) exactly.
30/30 CTest; a mutate-one-alone swap of the one blend-formula line fails exactly one check
and restores byte-identically. Two rounds remain before parking per the round cap.

**C1c: both pre-registered leads also falsified; one concrete thread found, round 2 of 3**
([check](../scratch/phase-c1-panel-icon-render.md#c1c-both-pre-registered-leads-also-falsified-one-concrete-unresolved-thread-found-round-2-of-3)):
all ~50 virtual-method bodies across all nine skill-constructor vtables (C1a) read; none
draws — they are simulation physics/eligibility/cell-flag code or the lemming's own
in-level pose-name selection, explaining C1b's `LemmingsLO` name coincidence without
confirming it as the panel's source. `LemmingsLO`'s string does not exist anywhere in the
exe, so that lead is unconfirmable, not merely untraced. A new per-frame candidate,
`FUN_0045cff9` (confirmed sole caller of the main loop), contains one confirmed real draw
call (`FUN_00414920`, independently corroborating A4r/A4t) but for exactly one overlay
quad/text label, never ten. `panel_boxes` has exactly 9 xrefs, all inside the already-read
hit-test function — no second draw-side reader exists. One concrete unresolved lead
remains: an indirect virtual dispatch on `levelScreen+0x3c`, reported unresolved by REA
itself. One round remains before parking per the round cap; dispatched to trace that
object.

**C1d: the ten-widget array and its real icon draw are located; full dispatch and the
per-widget icon index remain open, round 3 of 3**
([check](../scratch/phase-c1-panel-icon-render.md#c1d-the-ten-widget-array-and-its-real-icon-draw-are-located-the-full-dispatch-to-all-ten-and-the-per-widget-icon-index-remain-open-round-3-of-3)):
the level-screen constructor (`0x452bc2`) builds exactly **ten** identical sub-objects at
a constant `0x238`-byte stride — exactly the panel's physical button count (C1a) — each
sharing one constructor (`FUN_004576b0`, used nowhere else) and one six-slot vtable
(`0x4b1774`). `levelScreen+0x3c` (the field `FUN_00454a85`'s dispatch reads) is set
unconditionally to the first widget at construction; no other writer exists. The vtable's
slot+4 (`FUN_00457868`) is a genuine, confirmed real icon draw — resource lookup by the
widget's own index, UV rect from the widget's own pixel rect divided by the resource's
width/height, drawn via `FUN_00414920` (A4r/A4t/C1c's confirmed primitive) — exactly the
shape a per-button icon draw needs. But the dispatch actually reaches only **one** of the
ten per frame; no resolved caller reaches the other nine or the `+8` resource-index
writer. Independently re-verified byte-for-byte (constructor, per-widget ctor, vtable
bytes, slot+4 body, three `procedure_callers` checks all exact matches).

**Round-cap exception, recorded per `docs/ORCHESTRATION.md#milestones-gates-and-round-caps`**
("record in the next brief why a fourth round is expected to move the gate"): this is not
a residual that leaves the gate passing, nor an open-ended fishing expedition — it is the
literal missing piece of C1's gate, now narrowed from "where is the draw call" (open) to
two concrete, bounded, nameable questions: (1) is there a stride-`0x238` dispatch loop
elsewhere in `.text` that REA's resolved-call analysis cannot see because it is computed/
indirect (a literal-operand search for the constant `0x238`/`568`, the same technique
this project's own prior rounds have used for other stride constants, e.g. `panel_boxes`'s
`0x1b0`); and (2) where each widget's own `+8` resource index is written (a field-store
search, not a call-graph search — distinct from what C1b/C1c already looked for). A fourth
round is authorized on this specific, narrow continuation; it is not a reset of the
question's round count and no fifth round is pre-authorized.

**C1e: both cap-exception sub-questions are clean negatives — question parked**
([check](../scratch/phase-c1-panel-icon-render.md#c1e-both-cap-exception-sub-questions-are-clean-negatives--question-parked-round-4)):
an exhaustive `search_bytes` scan for the `0x238` stride literal across all of `.text`
found exactly 7 matches, none a dispatch loop and none near `levelScreen`'s constructor —
no computed loop reaching the other nine widgets exists in statically-visible form. A
second exhaustive scan, for all ten widgets' absolute `+8` offsets plus a full re-read of
the five most-likely candidate functions, found no writer of any widget's icon-index
field anywhere in `.text`. Both independently re-verified exactly. Per the round's own
"no fifth round regardless of outcome" instruction, **this question is parked in Phase E**
(see below). C1's rendering clause does not pass; it is blocked on two parked
dependencies (this icon-draw mechanism, and B1's already-parked digit-font question for
the counts half of the same gate clause).

**C1f: the live-read resume — two static-era assumptions overturned; the real base-icon
source still unidentified** ([check](../scratch/phase-c1-panel-icon-render.md#c1f-live-read-falsifies-two-static-era-assumptions-fun_00457868-is-not-the-per-button-renderer-round-1-of-a-new-live-read-question-one-stock-start)),
2026-10-22, authorized by `docs/DIRECTIVE.md` question 2, exactly one stock start. Live
reads (no debugger/breakpoint) **MEASURED** all ten widgets' `+8` field as the consecutive
sequence **8–17** (`widget_index + 8`, not `0–9` as predicted) — C1e's "maybe `+8` is
simply never written" residual is refuted. `levelScreen+0x3C` is **not** permanently fixed
to widget 0: it held widget 0's address at level entry, then moved to widget 6's address
after one unpause/pause cycle with no explicit skill click sent — overturning C1d/C1e's
"no writer exists" as a true report of what static resolved-call analysis can see, not of
the live system; the actual writer is still unidentified (candidate, unverified: cursor
hover/hit-test against `panel_boxes` at `0x4C9208`). A Frida hook on `FUN_00457868`'s
entry (461 hits in 3.2 s, matching the independently measured 144 Hz poll rate) found
`distinct_ecx_count == 1`: every hit's `this` equalled the one widget `levelScreen+0x3C`
named at that moment — it fires once per frame, for exactly one widget, never more. A
screenshot taken *before* any unpause input (while `+0x3C` still named widget 0) already
shows roughly nine distinct icon images on the panel simultaneously, independently
confirmed by the facilitator opening the saved PNG. **This redirects C1's open question
rather than closing it**: `FUN_00457868`/vtable slot+4 looks like a single-target draw
(selection highlight?), not the base per-button icon renderer C1b–C1e were chasing: the
real source of the other icons' base artwork, and the `+0x3C` writer, remain
unidentified. C1's rendering clause still does not pass; both halves (this redirected
icon-source question, and B1's digit-font question — now substantially de-blocked by B1i,
above, though not yet wired into a renderer) stay in Phase E pending the overlord's next
ranking, per the directive's stop point.

**C1y: `^tab` decodes to a 72 by 32 BMP whose colour bytes are zero**
([check](../scratch/phase-c1y-tab-pixels-decode.md)), 2026-10-24, directive part 1a,
one offline round. `0x45E8F7` pushes `0x4C0E00` (`^tab`) and `0x45E909` stores the
slot. A leading `5E` sets flag 8 (`0x474471`), and `0x47236B` pushes
`Textures\%s.alf`. `TEXTURES\^TAB.BMP` is 6966 decoded bytes, SHA-256
`d38db65196988c5b3d018a5a5f6386ba6333c5590e64ad4bfe67a55bac1403ed`, and all 6912
colour bytes are `0x00`. `TEXTURES\^TAB.ALF` is 2304 bytes, values 0 through 128,
2129 nonzero. The consumer `0x47E40D` and `^tab_selected` (`0x45E90F`) were unread.
The one Frida start was not spent. The facilitator re-ran the decode and re-read
the registration, the flag-8 test and the `.alf` format.

**C1z: plate blend is `0x414D80`, stage state is `0x47261F`, and the hook is `0x41494F`**
([check](../scratch/phase-c1z-plate-blend-hook.md)), 2026-10-24, directive part 1b,
one offline round. `0x45B66A` calls `0x414920` with no branch before `0x41494F`
(`call 0x414D80`). Slot `+0x58` there is blend enable `0x1B`, source `0x13` and
destination `0x14`, skipped when the selector equals `[0x4C227C]`. `0x414944`
calls `0x47261F`, which binds a texture through `+0x98` and writes stage-0 colour
and alpha through `+0xA0`, plus alpha test through `+0x58`, unless the descriptor
or the flags match their caches. The hook samples the flag cache after `0x47261F`
returns, so that sample does not say whether the stage writes ran. The live
selector is unread. The facilitator re-disassembled the four ranges and re-ran the
dry check. The Frida start was not spent.

**C1ab: the plate hook samples the stage caches before the stores**
([check](../scratch/phase-c1ab-prestage-hook.md)), 2026-10-24, round 1 of 3,
offline. `0x414944` is `call 0x47261F`. Its `onEnter` reads the descriptor,
the flags, and the caches `[0x5CD5F4]` and `[0x4C24E4]` before that call.
`0x41494F` still reads the blend selector before `call 0x414D80`. The bytes
from `0x414920` to `0x414944` are straight-line; the first branch is
`0x41496E`. The callee can skip the stores: `0x47262E` jumps to `0x472975`
when the descriptor matches the cache, and `0x472665` jumps there when the
flags match, after the texture bind at `0x472653`. The live values are unread.
The facilitator re-disassembled both ranges, re-ran the dry check, and a
one-address change of the descriptor attach made the check fail. `ctest` 34/34.
No stock start.

**C1aa: `^tab` `.alf` bytes are shifted into a 16-bit pixel**
([check](../scratch/phase-c1aa-alf-alpha.md)), 2026-10-24, round 1 of 3,
offline. `0x47388F` loads one byte, `0x473897` is `sar eax, cl`, `0x47389F` is
`shl eax, cl`, and `0x473943` stores the 16-bit word. The sar count is
`8 - [ebp-8]`. The shl count is the trailing-zero count of `[ebp-0x128]`.
A zero mask at `0x4736B6` skips the `sar`. The popcount callback is
`0x473B1E`, passed with `lea [ebp-0x14]` to `[vtable+0x20]`. `^tab_selected`
uses the same `0x45FE10` registration; its BMP is 72 by 32 with nonzero
colour, and its `.alf` max is 248. The writer of `[ebp-0x128]` was not found.
The facilitator re-disassembled the shifts and the callback, re-ran the rule
command, and a one-byte change of the `sar` check made it fail. No stock start.

**C1ad: the paused plate blend selector is 1**
([check](../scratch/phase-c1ad-plate-blend-live.md)), 2026-10-24, round 1 of 3,
one stock start. `Popen` 16:48:18, pid 3996, session 1, paused
`MM_CLIMB_002`. The hook logged 8650 plate hits. All 40 stored records have
selector 1, descriptor `0x2627270`, flags 8, and blend cache `0x10620C1F`.
The flag cache matches on every record. The descriptor cache differs only on
plate 0 of each frame. `[0x4C24E0]` is `0xA0`. The blend-cache compare does
not skip the enable arm. The flag-cache compare skips the later stage writes,
so the alpha-ref dword was not shown to be stored. Install hashes match.
The facilitator re-parsed the hook log and re-hashed the three install files.

**C1ac: the mask slot is filled by a 32-byte copy**
([check](../scratch/phase-c1ac-alpha-mask-writer.md)), 2026-10-24, round 2 of 3,
offline. The upload never stores `[ebp-0x128]`. That slot is
`[ebp-0x144+0x1c]`. `0x473C95` calls `0x49D010` with count `0x20`, and
`0x49D043` / `0x49D1C4` are `rep movsd` into that destination. The popcount
reads the source dword; the shift reads the copy. Which `rep movsd` runs
depends on a method that is not in the exe. `call [edx+0x64]` at `0x4736F9`
receives the record after the zero test and before the shift load, so the
shift is not shown to count the copied mask. The facilitator re-ran the
command, re-listed the five reads, and a one-byte change of the `movsd` check
made it fail. No stock start.

**C1ae: the shift-count hook is dry-checked at `0x473897`**
([check](../scratch/phase-c1ae-shift-count-hook.md)), 2026-10-24, round 3 of 3,
offline. `onEnter` at `0x473897` is before `sar eax, cl`. `0x473891` has just
reloaded the sar count into `ecx`. The shl count is still in `[ebp-0xad8]`,
and the mask dword is still `[ebp-0x128]`. The span has no `call` and no store
of the mask. The numeric counts were not read. The facilitator re-ran the dry
check, re-read the five instructions, and a one-address change made the check
fail. No stock start.

**C1af: the paused attach saw no shift**
([check](../scratch/phase-c1af-shift-count-live.md)), 2026-10-24, round 4,
one stock start. `Popen` 17:15:44, pid 32972. The click, the pause, and
`ready.json` are the same second, 17:16:16. The hook then logged `hits` 0.
The `sar` did not run in the six seconds it was installed. The upload at the
click was not observed. Install hashes match. The facilitator re-read the
hook JSON and re-hashed the three install files. The counts stay unread.

**C1ag: the probe waits for the hook before it clicks**
([check](../scratch/phase-c1ag-arm-before-click.md)), 2026-10-24, round 5,
offline. After `select_level` and before `aim_at`, the copy waits for
`hook-armed.flag` and raises if it is absent. `ready.json` is written only
after `enter_level()` returns. The attach stays `0x473897`. The facilitator
re-ran the dry check and, with that one `raise` removed, the check failed.
No stock start. The 6000 ms report still starts when the script loads.

**C1ah: the armed click did not call the shift**
([check](../scratch/phase-c1ah-shift-count-armed.md)), 2026-10-24, round 6,
the last stock start. `Popen` 17:32:40, pid 42720. The arm wait is 17:33:10
and the click is 17:33:39. `C1AE_HOOK` is `hits` 0. `0x473897` did not run
while the hook was installed. `select_level` at 17:33:10 is before the arm
wait. Install hashes match. The facilitator re-read the hook JSON and
re-hashed the three install files. The counts stay unread. Start 3 of 3.

**C1aj: the format callback copies 32 bytes and returns 1**
([check](../scratch/phase-c1aj-format-keep.md)), 2026-10-24, format-enumeration
round 1 of 3, offline. `0x473B1E` returns only at `0x473D97` `ret 8`. The keep
that also stores the popcount is `0x473C95` `call 0x49D010` with count `0x20`.
It requires `[format+4] & 0xE0000` clear, `[format+0xc] == 0x10`, bit 0 of
`[format+4]` set, and the popcount of `[format+0x1c]` greater than 1. That arm
and the reject both execute `mov eax, 1`. The only `eax` 0 is `0x473B85`, after
the copy at `0x473B76`, and the upload does not read `eax` after either
`call [eax+0x20]`. The stop is not in this function. The facilitator
re-disassembled the body and re-read the keep and the `ret`. A one-byte change
of the expected `ret` failed. No stock start. The shift counts stay unread.

**C1ai: `^tab` is registered at startup, and the upload's phase is unnamed**
([check](../scratch/phase-c1ai-upload-when.md)), 2026-10-24, alpha-count
round 1 of 3 this run, offline. Seventeen direct calls reach `0x45FE10`.
Sixteen, including `^tab` at `0x45E901` and `^tab_selected` at `0x45E919`,
run from the PE entry `0x49D518` through `0x49D5E1` and `0x450895`.
`0x45FE10` does not call `0x472F99`. The only direct call of the upload is
`0x47241A`, reached from `0x4744B1` inside `0x4743DD`, and nothing in `.text`
calls `0x4743DD`. Its address sits at vtable slot `0x4B19FC`. The facilitator
re-scanned `.text` and re-read the chain. A one-byte change of the expected
`0x45E901` call failed. No stock start. The shift counts stay unread.

**C1ak: the upload slot is called at `0x471746`, and that phase is unnamed**
([check](../scratch/phase-c1ak-vtable-call.md)), 2026-10-24, alpha-count
round 2 of 3 this run, offline. `0x471746` is `call dword ptr [edx+0x14]`
inside `0x471720`. That function is vtable `0x4B19E8` slot `+0x0C`, and on
that vtable the call target is `0x4743DD`. The same instruction on vtable
`0x4B1A00` calls `0x49C134`. There is no direct caller of `0x471720`.
`0x471230` replaces `0x4B19E8` with `0x4B1A00` before it returns. `0x474380`
leaves `0x4B19E8` in place, and its only direct caller is `0x46F16F`. The
facilitator re-read the call and re-scanned `.text`. A one-byte change of
the expected call failed. No stock start. The shift counts stay unread.

**C1al: the upload vtable is constructed at startup, and the upload is not called there**
([check](../scratch/phase-c1al-ctor-caller.md)), 2026-10-24, alpha-count
round 3 of 3 this run, offline. `0x49D5AB`, inside PE entry `0x49D518`, calls
`0x49D345`. That walker calls the dword at `0x4BF0B0`, which is `0x468D20`.
The chain reaches `0x46F16F` `call 0x474380` on `this+8` and does not reach
`0x471720`. The facilitator re-read the chain and the walker. A one-byte
change of the expected entry call failed. No stock start. The shift counts
stay unread. This question is at its cap.

**C1am: the launch-time alpha hooks dry-check, and the frida module is absent**
([check](../scratch/phase-c1am-launch-hook-dry.md)), 2026-10-24, launch-attach
round 1 of 3, offline. The script appends from `onEnter` at `0x473B1E`,
`0x472F99`, `0x4736B6` and `0x473897`. The probe source uses `frida.spawn`,
then `script.load`, then `frida.resume`, and reads the log after the pause.
The facilitator re-read the four sites. A one-address change of the `sar`
attach failed the dry check. `import frida` is missing, so nothing was
spawned. No stock start. The shift counts stay unread.

**C1an: this python imports frida 17.24.3**
([check](../scratch/phase-c1an-frida-import.md)), 2026-10-24, launch-attach
round 2 of 3, offline. `pip install frida` added only that package. The c1am
hook and probe are unchanged. The facilitator imported `frida` 17.24.3 and
re-ran both dry checks. Nothing was spawned. No stock start. The shift
counts stay unread.

**C1ao: the launch-time start exited before the level**
([check](../scratch/phase-c1ao-launch-read.md)), 2026-10-24, launch-attach
round 3 of 3, one stock start. Pid 20612 resumed at 21:43:53 and was gone
at 21:43:58, before `MM_CLIMB_002`. The log has 9 uploads, 28 format
records and masks 0, 0, 0, 0, 0, 0, 0, 0, 32768. The shift site has 0 lines.
The facilitator recounted the log and re-hashed the install. The probe was
not run again. The paused-level shift counts stay unread.

**C1ap: the count hook moves to `0x47383C`**
([check](../scratch/phase-c1ap-hook-repair.md)), 2026-10-24, hook-repair
round 1 of 3, offline. No branch target falls inside `0x473898`..`0x47389E`.
`[ebp-0xae8]` is stored at `0x4737F4` and `[ebp-0xad8]` at `0x473806`, and
`0x47381B` reaches `0x47383C` before the per-pixel loop at `0x473866`.
The facilitator re-disassembled that span, re-scanned `.text` branches, and
a one-address change of the new attach failed the dry check. No stock start.
The early exit stays unnamed. The paused-level shift counts stay unread.

**C1aq: the repaired probe exited before the level**
([check](../scratch/phase-c1aq-launch-read.md)), 2026-10-24, hook-repair
round 2 of 3, one stock start. Pid 25964 resumed at 23:20:21 and Frida
reported `process-terminated` at 23:20:23, before `MM_CLIMB_002`. The log
has 9 uploads, 28 format records and masks 0, 0, 0, 0, 0, 0, 0, 0, 32768.
`0x47383C` has 0 lines. The facilitator recounted the log and re-hashed the
install. `OpenProcess` failed with 87, so the exit code is unread. The
probe was not run again. The paused-level shift counts stay unread.

**C1ar: the exit code moves into the detach callback**
([check](../scratch/phase-c1ar-exit-capture.md)), 2026-10-24, hook-repair
round 3 of 3, offline. `on_detached` calls `read_exit_code` before it
returns. The mask hook logs `[[ebp+8]+0x24]`, the dword `0x4737E6` tests
with AND `0x18`. The facilitator re-disassembled that span and a removal
of the callback read failed the dry check. No stock start. The paused-level
shift counts stay unread.

**C1as: the remaining start exited with `0xC0000005`**
([check](../scratch/phase-c1as-launch-read.md)), 2026-10-24, hook-repair
round 4, one stock start. Pid 44772 resumed at 23:38:48 and was gone at
23:38:50, before `MM_CLIMB_002`. `exit_code` is 3221225477. `skip_input`
is 1 eight times and 2 once, and each value AND `0x18` is 0. `0x47383C`
has 0 lines. The facilitator recounted the log and re-hashed the install.
The probe was not run again. The paused-level shift counts stay unread.
The start pool is spent.

**C1av: the skip path stores masked pixels**
([check](../scratch/phase-c1av-skip-write.md)), 2026-10-25, alpha-count
offline round 3 of 3, parallel with c1at and c1au. From `0x47395A`
through `ret 4` at `0x473B1B`, bits 0, 1 and 2 of `[object+0x24]` store
16-bit pixels through `[ebp-0xad4]`, ANDed with `[ebp-0xad0]` and ORed
with the low word of `[ebp-0x128]`. The span does not enter the shift
loop, so `0x473943` does not run. The facilitator re-disassembled the
stores, and one wrong expected byte at `0x4739A6` failed the file check.
No stock start. The numeric mask stays unread. The gate is not scored.

**C1at: the plate pointer and the upload object are different loads**
([check](../scratch/phase-c1at-descriptor-link.md)), 2026-10-25, alpha-count
offline round 1 of 3, parallel with c1au and c1av. `[ebp-4]` at
`0x414944` is the return of `0x414A10`. The object `0x472F99` receives
is a `0x28`-byte allocation whose `+0x24` is the name-prefix flag; a
leading `^` stores 8. The facilitator re-disassembled both stores, and
one wrong immediate at `0x474471` failed the file check. No stock start.
Whether that allocation is later inserted as the pool node is unread.
The gate is not scored.

**C1au: context bytes are read and the kept mask is conditional**
([check](../scratch/phase-c1au-context-keep.md)), 2026-10-25, alpha-count
offline round 2 of 3. `+4` follows `[object+0x24] & 0x18`, `+5` follows
the low 3 bits, `+6` follows the word at `[ebp-0x192] <= 8`, and `+7`
follows `[ebp-0x140] & 4`. A `^` name makes `+4` 1 and leaves `+5` at 0.
Nothing in `0x472F99` writes `[ebp-0x140]`. If `+7` is 0, c1as line 29
(mask `0x0000F000`) is the only record the popcount arm keeps; if `+7`
is 1, none are. The facilitator re-disassembled the span and re-applied
the rule. No stock start. The gate is not scored.

**C1aw: the menu-attach start exited before `^tab`**
([check](../scratch/phase-c1aw-menu-context.md)), 2026-10-25, the one
live round. Pid 35216, one normal `Popen`, armed at the menu. Exit code
3221225477 (`0xC0000005`) at the click, before the pause. `hits.log` has
one line, `Sheets\$LemSheetHi8`, not `^tab`. The facilitator recounted
it, re-hashed the three install files, and a one-address change of the
dry check failed. The install files match. The `^tab` dword stays unread.
The gate is not scored. The start was not repeated.

**C1ax: the fill writes the +7 dword as 0**
([check](../scratch/phase-c1ax-fill-coverage.md)), 2026-10-25, fill-coverage
round 1 of 3. `0x473051` calls `0x49D760` with count `0x7c` and value 0
at `[ebp-0x18c]`. `[ebp-0x140]` is inside that interval, and nothing
before the read stores it. The `sete` at `0x473157` stores 0. The
facilitator walked the function, and a count of `0x7b` failed the check.
No stock start. The c1au coverage sentence is rejected. The gate is not
scored.

**C1ay: +6 reads a GetObjectA word**
([check](../scratch/phase-c1ay-kept-format.md)), 2026-10-25, kept-format
round 1 of 3. `0x473040` calls `GetObjectA` on a `0x18`-byte buffer.
`[ebp-0x192]` is byte `+0x12` of that buffer, and `0x473179` compares
the word with 8. The facilitator re-read the call and the name, and
expecting `GetObjectB` failed. No stock start. The number, and the
kept record, stay unread. The gate is not scored.

**C1az: the bit-count word is not in the file**
([check](../scratch/phase-c1az-dib-handle.md)), 2026-10-25, kept-format
round 2 of 3. `0x47255B` stores the `CreateDIBSection` return at
`[object+4]`, the field `GetObjectA` later pushes. The bit-count word
is at `0x58D5D4`, past the `.data` raw end, copied from the buffer
`0x45015A` returns. The facilitator re-read the store, and expecting
`0x88` failed. No stock start. `+6` stays unread. The gate is not scored.

**C1ba: 0x45015A does not write the bit-count word**
([check](../scratch/phase-c1ba-bitcount-source.md)), 2026-10-25, kept-format
round 3 of 3. The function compares the buffer with bytes `0x42` `0x4D`
and has no store at offset `0x1C`. A match keeps the pointer from
`0x49C1A8`. The facilitator walked 217 instructions, and expecting `0x41`
failed. No stock start. The question is parked. The gate is not scored.

**C2l: `_curExit` is a registrar push**
([check](../scratch/phase-c2l-cursor-exit.md)), 2026-10-25, cursor clause
round 1 of 3. The string is at `0x4C0DD0`. Its one reference is
`push 0x4C0DD0` at `0x45E89B`, then `call 0x45FE10`. The facilitator
found that dword once, and expecting `0x79` failed. No stock start.
The hover rule was not read. The gate is not scored.

**C2m: `_curExit` is stored at `0x4E333C`**
([check](../scratch/phase-c2m-cursor-slot.md)), 2026-10-25, cursor clause
round 2 of 3. `0x45E8AD` stores the registration return at
`[0x4C6A08+0x1C934]`. The facilitator re-read the store and the one
direct caller, and expecting displacement byte `0x35` failed. No stock
start. The slot is past the file image. The hover rule was not read.
The gate is not scored.

**C2n: two instructions load the `_curExit` slot**
([check](../scratch/phase-c2n-cursor-read.md)), 2026-10-25, cursor clause
round 3 of 3. `0x45ECBF` calls `0x414B90` with the slot, and `0x45EE3A`
calls `0x414D20` unless `[object+0x1C9B1]` is nonzero. The facilitator
found the displacement three times, and expecting opcode `0x89` failed.
No stock start. The callees were not opened. The question is parked.
The gate is not scored.

### C1 skill panel — DERIVED value-framed run, 2026-10-25

**C1bb: installed patch alpha upload bypasses native format enumeration**
([Facilitator check](../scratch/phase-c1bb-derived-format.md)). The native +6=0 /
A4R4G4B4 replay is conditional, not this install's successful `^tab` path.
The imported patch returns its alpha hook, requests DERIVED A8R8G8B8, returns 4,
and sends the exe to its epilogue before +6 and shifts. Own fresh bytes, import/
export/branch/request decoding, full-list predicate, isolated immediate mutation
and 34/34 CTest pass. No live observation or pixel score. Next: patch texel
conversion and resize, then render over the unchanged frozen footprint.

**C1bc: conditional patch plate words and full UV**
([Facilitator check](../scratch/phase-c1bc-patch-texels.md)). Both assets derive
128x32 uploaded streams from 72x32 via endpoint-aligned bilinear/truncated channels.
Own complete p53-nearest replay, source bytes, 2,554 instruction-line rereads,
row-flip mutation and 34/34 CTest pass. Actual PC/RC, completed GDI and returned
layout stay conditional; no original upload or pixel pass measured. Next is a
declared conditional render and unchanged C1 gate score, with sensitivity reported.

**C1bd: conditional plate render, all three fixed gates fail**
([Facilitator check](../scratch/phase-c1bd-plate-render.md)). Initial attempt plus
two bounded repairs add the opt-in asset-derived plates, portable interpolated
alpha test and exact nine-view footprint scores. Own re-render/re-score:
zoom1/elev_up/elev_down0.0343726071/0.0553698541/0.0344632854 >0.02.
An isolated alpha-only mutation worsens zoom1 and fails the pixel test; exact
restore and34/34CTest pass. No original runtime state or raster assumption is
accepted by these numbers. Plate-exclusive region ranks first; next is its
numerical filter/alpha state, not a fourth integration dispatch or gate change.

**C1bf: zero-count text skipped, gate still fails**
([Facilitator check](../scratch/phase-c1bf-zero-count.md)). The native
`0x45B6E4` compare skips only the text submit. Own re-render of the three
gated views matches the worker PNGs. Scores 414292/784866/415022 over
17721990, all above 0.02 and all below C1bd. Removing only the guard fails
one destination test and returns zoom1 to 609151. Restored source and 34/34
CTest. Next is the selected plate, then the filter only if the gate still fails.

**C1bg: selected index 0 and enable 1, derived**
([Facilitator check](../scratch/phase-c1bg-selected-index.md)). Normal init
writes fallback 7, then keeps the first strictly positive count. This level's
trailer is 10,0,0,0,0,0,0,0,0,1, so the result is index 0 and enable 1.
The extra quad is rectangle 18,12,90,44 for that index, submitted after the
ten base plates. Not a pause-memory read. The retained A7p script's primary
clicks are the map node. Next/previous handlers were not re-opened. No score.

**C1bh: selected quad drawn, elev_up still fails**
([Facilitator check](../scratch/phase-c1bh-selected-draw.md)). The extra
`^tab_selected` quad is index 0, rectangle 18,12,90,44, after the ten base
plates. Own re-renders match the worker PNGs. Scores 286491/580420/287373
over 17721990. zoom1 and elev_down pass 0.02; elev_up does not. Removing
only that call returns zoom1 to 414292. Restored source and 34/34 CTest.
The sampler stays a hypothesis. Next is the plate filter.

**C1be: stage-0 mag and min share one local**
([Facilitator check](../scratch/phase-c1be-plate-filter.md)). `0x470F29`
requests 1 unless the argument and `[this+0x3A1]` are both nonzero, in
which case both `0x10` and `0x11` get 2. The ten plate submits are before
this function's argument-0 call. The init store writes 1 at
`[0x58CD80+0x3A1]`. Header names: 16/17 are mag/min, 2 is LINEAR. Indirect
callees before the panel call were not re-opened. No device read and no
score. Next is sampling the plates with that request.

**C1bi: linear plate sample, elev_up still fails**
([Facilitator check](../scratch/phase-c1bi-linear-plate.md)). The sample
blends the two texels around `s = x * 128 / 72`. Own re-renders match.
Scores 285083/580972/285700 over 17721990. zoom1 and elev_down pass;
elev_up is worse than C1bh by 552. Plate-only elev_up error is 466081 of
that numerator. Omitting the plates makes elev_up worse, 2055954. Restoring
nearest returns zoom1 to 286491. 34/34 CTest. The weights stay a hypothesis.

**C1bj: plates over the stock frame raise the error; run stopped**
([Facilitator check](../scratch/phase-c1bj-plate-over-stock.md)). The stock
baseline already contains its plates. Drawing ours on top scores full
1123813/1145908/1114207 and plate-only 669750/691289/666306. All three
plate-only numbers are higher than plates over our frame. The elev_up
interior is 384401, against 1589553 with the plates omitted. Opaque alpha
raises elev_up plate-only to 2519199. Icon and glyph error, 114891, cannot
bring 580972 under 354440. The C1-only run stops here.

**C1 closed under the third amendment (2026-10-25)**
([Facilitator check](../scratch/phase-c1-close-amended-gate.md)). Fresh independent
renders/rescore give zoom1 285083/17721990 and elev_down 285700/17721990, both
below 0.02. elev_up 580972 and seven other diagnostics are reported in the note;
none is declared passing. Frozen footprint and source hashes match the directive;
34/34 CTest pass. A single image shift fails the gate. No panel implementation
round or stock start. New run `c1-close-c3-scene` advances to C3 full frame.

**C3p: kind-5 scalar predicate derived (2026-10-25)**
([Facilitator check](../scratch/phase-c3p-kind5-culling.md)). The scalar is
`dot(E-first_vertex, selected_vector)`, passing only above zero; E comes from
the inverse composed rotation and negated translation. Independent raw-level
parse/Gaussian solve reproduces 306 pass/159 skip across 465 records. One
normal-only sign mutation flips exactly one record; original bytes unchanged.
35 native windows authenticated, 34/34 CTest. Conditional predicate only, not
final draw visibility or native precision. Correct repair4 baseline is 0.2050523;
the old C3 scorer uses A6d3 and needs explicit reference selection. No render change.

**C3o: repaired type-2 slot/UV pairing (2026-10-25)**
([Facilitator check](../scratch/phase-c3o-type2-draw.md)). Original own-Y evaluator
and first facilitator checker are rejected. Native Y stores cross generated point
indices; repaired slots and all 20 known-field vertices independently match.
Five conditional submissions/twelve clip-empty nodes remain, but all five old
packed streams fail, as does the isolated own-Y mutation. 58 byte windows and
install hashes authenticated; 34/34 CTest. No GPU-state, general clipping,
original precision or pixel acceptance; no production edit or stock start.

**C3n: source fields, binding and pack layout verified (2026-10-25)**
([Facilitator check](../scratch/phase-c3n-mesh-submission.md)). Independent raw
parser confirms1177 records/4243 corners in19 mesh nodes, kinds5/6/7;24 alias
records preserve last-store attributes;304 records bind null. All source/texture
fields and sequential transforms match. Final pack is RHW0.25/f32(z), not1/z;
specular is unwritten. Known bytes for2077 candidate intermediates match a
separate formula; alpha-only mutation changes every diffuse word and no other
field.84 raw windows authenticated;34/34CTest. Candidate clipping count is not
native draw acceptance. Device/texture upload values and integration remain.

**C3s: explicit reference scorer repaired (2026-10-25)**
([Facilitator check](../scratch/phase-c3s-explicit-reference-scorer.md)). CLI now
selects/reports --reference; historical A6d3 default remains labeled. Independent
histogram/region checks reproduce current repair4 baseline48188934 and distinct
old-candidate repair4/A6d3 totals49336286/49297573. A one-line reference bypass
fails two permanent tests and restores byte-identically;3/3tests and34/34CTest.
No gate/region/metric change, original-game read or scene-render improvement.

**C3t: annular-family bound verified, floor identity remains partial (2026-10-25)**
([Facilitator check](../scratch/phase-c3t-annular-family-bounds.md)).128 base/top
records produce a conditional candidate bound[69,0,569,480). Independent raw-source
and different polygon-square clipping check reproduces55571 diagnostic pixels;
one projected-vertex mutation changes143 pixels and restores. Mask is unpacked
0/1 bytes, not a packed LSB-first bitset despite candidate wording.34/34CTest.
Native visibility/clipping and complete floor identity remain unaccepted; no
scoring, gate/region changes, render implementation or stock start.

**C3r: scene constructor flags and conditional texture streams verified (2026-10-25)**
([Facilitator check](../scratch/phase-c3r-scene-texture-values.md)).33 resources,
52 member slices and192512 source pixels independently reproduced. Normal
constructor flags:13 resources0,19 caret resources8,Water4.19 conditional patch
streams and28 illustrative native GDI/formula variants match; alpha-only and
one-blue-byte mutations discriminate/restore.58anchors/five raw windows and
install hashes authenticated;34/34CTest. Actual native format/wrapper conversion
and returned layouts remain unresolved; no pixel-fit choice or scene render.

**C3q: conditional material/blend/filter/shade state machine verified (2026-10-25)**
([Facilitator check](../scratch/phase-c3q-scene-device-state.md)).32 native windows
authenticated and key setter/wrapper order re-read; separate expectations reproduce
600 full state rows. Ordered cache tests protect latched alpha reference and null
selector4 blend preservation; input/branch mutations discriminate and restore.
34/34CTest. Effective pause Z/cull/fog/API/prestate remains typed input, not SDK
defaults or measured device state. No production render or gate improvement.

**C3w: source integration map verified (2026-10-25)**
([Facilitator check](../scratch/phase-c3w-source-integration-map.md)). Existing
PolyList and TubeScene paths are diagnostics, not a recursive scene reader or
accepted mesh/material pipeline; Raster is not an explicit C3 device-state model.
Checked source symbols and selected a separate strict asset-reader task first,
before submission/raster/app wiring. No source edit or original measurement.

**C3u: conditional scene gates/load associations (2026-10-25)**
([Facilitator check](../scratch/phase-c3u-pause-scene-reachability.md)).32256 branch
rows and46raw windows authenticated; enable-byte mutation discriminates/restores.
Mode2/nonzero-enable admits primary slot4 before tube; nonzero second presence
admits secondary slot8 after tube. Separate model holders serve Entrance/Exit.
Historical repair4 values stay unknown; slot8 downstream is not equated to slot4
rendering.35/35CTest; no original start, production change or score improvement.

**C3v repair1: draft mock failure bounds, not live preflight (2026-10-25)**
([Facilitator check](../scratch/phase-c3v-repaired-mock-bounds.md)). Original
unexecuted mutation/error-retry claims rejected and preserved. Separate VM checks
verify17finite failure fixtures, exact32getter ABI/output calls, recursion/runtime
rejection and an actually executed slot mutant.35/35CTest. Draft remains disabled;
scene correlation, concrete COM/surface identity and live cleanup unproved. No
live use, format observation or pixel acceptance is authorized by this landing.

**C3x repair1: strict source reader implemented (2026-10-25)**
([Facilitator check](../scratch/phase-c3x-scene-node-reader.md)). New explicit-offset
SceneNode reader preserves supported recursive types0/1/2 and mesh5/6/7 fields.
Independent reencoding matches every field/offset across all4retained slices.
Impossible-count eager allocation rejected in repair; guarded negatives and
normalIndex-only mutation discriminate; exactSHArestore,35/35CTest. No app scene
render or pixel improvement; unsupported variants fail closed.

**C3y: secondary slot8 sorts, then invokes child slot4 (2026-10-25)**
([Facilitator check](../scratch/phase-c3y-secondary-slot8.md)). Full46B73Fbody
and24windows authenticated: root computes translation-rotation-Z+child108 keys,
descending/nonstable insertion order, then29childslot4calls (12mesh/17type2).
Independent source ranks and isolated bias mutation match/restore;35/35CTest.
No root own draw or slot8=slot4 equivalence; inherited mesh clipping counts,
runtime visibility/gates/effective state and pixels remain conditional.

**C3z: conditional observation contracts; runtime arm parked (2026-10-25)**
([Facilitator check](../scratch/phase-c3z-observation-provenance.md)).46windows,
fourSDKinterfaces/threeGUIDs authenticated; original DD4/D3D3/Device3/Surface4/
Texture2 producer outputs and class7 scope distinguish descriptor from texture/
surface. Independent synthetic request tuple/isolated field mutation;35/35CTest.
Actual producer/lifetime/class/runtime cleanup/effective state and texels remain
unknown; observation parked atVinitial+repair+Zround3, no live recipe authorized.

**C3aa: explicit-matrix mesh source preparation (2026-10-25)**
([Facilitator check](../scratch/phase-c3aa-scene-mesh-source.md)). Independent
1177record/4243corner table/raw-byte comparison protects source identity, normals,
materials, per-record alias winners and transforms; maxerror2.84e-14. Reversed
hierarchy-only mutant fails and restores;38/38CTest. In-flight preparation task
completed under render-first steering; app integration/render+score is next, not
additional readers. No GPU submission/state/visibility or pixel acceptance.

**C3ab: explicit-input software scene raster (2026-10-25)**
([Facilitator check](../scratch/phase-c3ab-scene-raster.md)). Independent exact
rational oracle passes5120cases/61440pixels over explicit state options; equality
alpha mutant fails and restores exactly;38/38CTest. Declared software component,
not native coverage/precision or runtime-state acceptance. App render still pending.

**C3ac initial/repair1: native-compatible billboard source (2026-10-25)**
([Facilitator check](../scratch/phase-c3ac-billboard-source.md)). Initial native
matrix reinterpretation and omitted retail comparison rejected. Repair converts
native layout explicitly, preserving sourceOffset. Independent17node/340field
comparison passes below1e-9; own-Y mutation fails/restores;443retail checks and
38/38CTest pass. Preclip compatibility, not visibility/state/pixels. All three
preparation components are ready for immediate app render/score under steering.

**C3ad initial/repair1: pause-value dry checks, live recipe rejected (2026-10-25)**
([Facilitator check](../scratch/phase-c3ad-pause-values-live.md)). Independent
129-bind fixture caught suppression of the requested pause snapshot. Stream
isolation fixes that offline failure; ABI/anchor/HRESULT checks remain bounded.
Controller then fails static export verification on `entry.RECORD_SIZE` after
Popen. No live approval or start; no pause/device values obtained. Two attempts
count toward the unchanged24-round run cap. This is a failed dry recipe, not a
missing user authorization or a runtime absence result.

**C3ae initial/repair1: partial conditional app scene, gate still FAIL (2026-10-25)**
([Facilitator check](../scratch/phase-c3ae-scene-render-score.md)). Fresh matched
repair4 render and independent histogram sum21707403/235008000=0.0923687832,
versus scene-off exact baseline0.2050523131. Mesh/type2 adapters now feed pixels;
single yaw mutation worsens error and restores;39/39CTest. C1zoom1/elev_down
remain0.0160864/0.0161212; elev_up0.0250557 and allotherreports retained. Narrowed
landing is software integration/scoring only: known ancestor/S/predicate inputs,
sorted dispatch/type1/clip/packing/state and AA/ACENV omissions remain, not merely
GPU precision. Largest error remains rest19234589,skills1484307,tube682387;
floor included, exact visibility unresolved. Initial wrong-input packet rejected
and preserved. Two attempts exhaust24-round run with AD; no further dispatch.

**C3ag: configured AA/AC retail environment (2026-10-25)**
([Facilitator check](../scratch/phase-c3ag-retail-env.md)). Own parent-env-unset
CTest runs443/31checks; removing only AC's ENV entry makes ACskip/48 while AA31
persists; exact restore and39/39CTest pass. No production pixels change from this
wiring. Required fresh score0.0838931 includes rejected/uncommitted initial AF,
not an accepted C3 improvement; accepted baseline remains0.0923688. C1 gates pass.
New directive run owns C3 only; initial AF and AH packets need bounded repairs.

**C3ah initial/repair1: actual controller mocked, deployment still pending (2026-10-25)**
([Facilitator check](../scratch/phase-c3ah-controller-lifecycle.md)). Initial
abstract state machine rejected. Repaired actual-main suite8tests passes; own
three independent boundary fixtures and one cleanup-source mutation protect
false-success correction. Helper import/0x118 contract independently checked.
39/39CTest and fresh C1 reports pass; C3 score diagnostic still rejected AF,
not accepted improvement. Actual launch paths/helper binding and remaining
failure coverage require final deployed-script drycheck; no approval/start.

**C3aj: exact deployed-controller contract drycheck (2026-10-25)**
([Facilitator check](../scratch/phase-c3aj-deployed-controller.md)). Correct helper
and unique defaults authenticated; actual-main10tests plus four independent
fixtures cover prelaunch size, stale/owned backups and fail-closed cleanup.
Single size-guard removal fails/restores.39/39CTest; fresh in-flight AF diagnostic
0.0846922 and C1 passing retained, no C3 promotion. No live marker/start/observation.

**C3af initial/repair1: conditional actual billboard chain (2026-10-25)**
([Facilitator check](../scratch/phase-c3af-billboard-chain.md)). Initial68draws
and lens-cancelling guess rejected/preserved. Actual shared builder and ancestor
walk reproduce17/340old C3o fields under raw reference operands; render17once.
Source-only S mutation fails/restores;39/39CTest. Repaired fresh score0.0846922
still fails. S identity's pixel error is lower, so source-value protection is not
claimed as native pixel protection; no score selection. Binary32 context model
is diagnostic, not READ runtime precision. C1 gates pass; known remaining gaps stand.

**C3ak: explicit final packed-vertex component (2026-10-25)**
([Facilitator check](../scratch/phase-c3ak-vertex-pack.md)). Separate formulas
reproduce2077retained plus2077synthetic known-field byte records and preserve
specular; raw pack/ftol bytes re-read. Single RHW-numerator mutation fails,
exact restore and40/40CTest. App source registered but Backdrop call site remains
unwired; no packing-caused pixel improvement or native clipping claim.

**C3ai: camera-composed kind5 source predicate (2026-10-25)**
([Facilitator check](../scratch/phase-c3ai-predicate-context.md)). Independent
raw-LVL/35byte-window oracle and actual source-offset visitor agree465/465,
306pass/159skip. Single predicate-context-return identity changes136decisions
(not worker's465), worsens full-frame error5065521 and restores exactly;
40/40CTest. Fresh fixed score0.0631375060 still fails; C1gatespass, floor included.
Dispatch/clip/packing adoption/state remain open; no native visible-count claim.

**C3al: conditional caret8 alpha row, worse screenshot (2026-10-25)**
([Facilitator check](../scratch/phase-c3al-caret-alpha.md)). Source/request row
authenticated17/17names; actual caller uses disabled-test/texture-alpha policy.
Own768destinationRGBA cases pass; cutoff-only source mutant fails/restores;
41/41CTest. Fixed score worsens to0.0712348771 (+1902947), C1 gates pass.
No value selection by score or effective-native/pixel-protection claim. Common
depth-unit consistency is the next explicit source-contract lead.

**C3am initial/repair1: conditional shared comparison-depth units (2026-10-25)**
([Facilitator check](../scratch/phase-c3am-shared-depth.md)). Corrected rejected
generated-offset equation; actual source-to-submission asymmetric test protects
19.5/19 projected to39/38 equivalent physical depths. Own192corner/two-pixel
oracle passes; raw-slot-only mutant fails and worsens score1911602; exact restore,
42/42CTest and17/340source replay pass. Fresh fixed score0.0631006774 still
fails; C1 gates pass. Worker scratch-report overwrite disclosed and excluded from
acceptance; fresh independent reports retained. No effective native Z claim/start.

**C3an initial/repair1: actual raw diffuse adoption (2026-10-25)**
([Facilitator check](../scratch/phase-c3an-raw-diffuse.md)). Shared raw truncate/
shiftOR word feeds actual mesh adapter and existing packer. Initial rescale-only
mutant survived; repaired adapter/destinationRGBA test now rejects it alone.
Own6561word/adapter/pixel cases,2077+2077known-byte fixtures and43/43CTest pass
after exact restore. Fresh fixed score0.0631364464 worsens8406; rescale improves
the screenshot, so no native pixel-protection claim or value selection by score.
C1gatespass,sceneoff exact,0starts. Secondary dispatch/clip/f32/state remain.

**C3ao initial/repair1: mixed secondary-child dispatch (2026-10-25)**
([Facilitator check](../scratch/phase-c3ao-secondary-order.md)). Actual render and
test share29child offset/key dispatcher; source geometry unchanged. Initial missing
scores/source-onlymutation and out-of-allowlist source-list entry returned/repaired,
preserved and disclosed. Own raw/sequential keys match29/29 within5.7e-14;
bias-onlysource omission fails/restores,44/44CTest. Fresh fixed0.0635481771
worsens96760; biasomission improves screenshot81848, so no native pixel-protection
claim/score selection. C1gatespass,0starts. Finite distinct keys only; ties and
primary/tube/type1/clip/f32/effectivestate remain open; final attempt observes pause.

**C3 final one-shot: host preflight rejects before stock Popen (2026-10-25)**
([Facilitator check](../scratch/phase-c3-pause-one-shot.md)). Actual session1task
state serialized numeric4, while exact AJcontroller expects Running/Absent strings;
Popen0,starts0,attach0,writes0, no pause values. No retry. Own actual-main fixtures
discriminate numeric4 versus stringRunning; real PowerShell enum serialization
confirmed, exact artifacts/install hashes and independent process/task cleanup pass.
44/44CTest;40source/10C1hashes and savedAOscore authenticated, no new render:
C3.0635481771FAIL,C1gatespass. Run stops16/16attempts,0/3starts; host enum bridge
requires bounded offline repair before any later execution. Remaining known
primary/type1/clip/f32/native-state debts not closed and no method permission missing.

### C3 rendering pivot — offline feasibility, 2026-10-25

**C3au boundary assessment:** conditionally prefer one caller-scoped pre-wrapper
scene-mesh submission over equating translated D3D11 work with original calls.
Own SDK/endpoint-byte check supports the layout candidate, not runtime identity
or capture compatibility. The next thin path is synthetic/no-forwarding; native
reachability, history, attribution and matched non-interference remain blockers.
[Check and contract](../scratch/phase-c3au-comparison-boundary.md). Fresh own
C3 baseline14934330/235008000 FAIL; C1zoom1/elev_down285083/285700 over17721990
PASS,44/44CTest. No production change, gate movement or stock start.

**C3at initial/repair1 frame map:** consolidate eight production families and
C3u's conditional primary-slot4→tube→secondary-slot8 spine. Initial missing
spine/stale references returned for repair. Own source/byte checks expose
secondary-before-tube integration mismatch, without attributing pixels; full
pack adoption, clipping, runtime gates and effective state remain open.
[Map](../RENDER_FRAME_MAP.md), [check](../scratch/phase-c3at-frame-map.md).
Same fresh frozen baseline/C1 passes and44/44CTest; no renderer change or start.

**C3av initial/repair1/repair2 thin path:** no-forwarding synthetic JSONL
recorder/comparator, explicitly limited to typed per-draw request/resource
checkpoints. Own verification caught4incomplete false equalities, then a known
attribute hidden by an unknown-only exclusion; repaired at attempt3/3. Own9valid
single-field controls differ and9incomplete/out-of-scope controls fail closed;
isolated normalized-X bypass fails permanent test, exact restore,12/12Python
and44/44CTest. [Check](../scratch/phase-c3av-thin-comparison.md),
[later capture proposal](../RENDER_CAPTURE_PROPOSAL.md). Final gates unchanged:
C3.0635481771FAIL,C1gatedPASS. Six-attempt offline pass complete,0stockstarts;
native ABI/attribution/history/non-interference still require preflight/approval.

### C3 bounded offline capture preflight, 2026-10-25

**C3aw static site/ABI/attribution:** independently authenticate31PE windows and
read877instruction lines; proposed PRE46D5EC/POST46D5F6 do not overlap, unlike
the rejected direct-call pair. Conditional stack and kind5/7 source discriminator
checked, not runtime interface/reachability or durable asset identity. Own fresh
2positive/16negative controls, single payload-copy mutation failure/exact restore,
worker52synthetic controls and packet manifest pass. [Check](../scratch/phase-c3aw-site-preflight.md).
No native observer deployed; retained C3FAIL/C1PASS unchanged,44/44CTest,
37/37Python. AX host prerequisite remains separate; no start or attach authorized.

**C3ax initial/repair1 controller preflight:** initial standalone guard returned
for missing actual-main integration. Repair1 imports real helper; own five main
outcomes and independent real-session1 replay pass, PID26112 exited/task absent;
single-State-guard mutation reaches forbidden barrier, exact source restored,
worker9/9tests pass. [Check](../scratch/phase-c3ax-controller-preflight.md).
Narrowed again on diff: returned main is reconstructed with operational suffix
removed; unchanged C3aj still has old serialization. Accept host bindings/barrier,
not launch readiness. AW observer remains design-only. Pass3/3attempts,0starts;
stop with offline integration/review blocker, no automatic native execution.

### C3 integrated observer and conditional observation, 2026-10-26

**C3ay initial/repair1 actual controller:** Facilitator independently reproduces
four unsafe dry-run outcomes, returns repair, then verifies preserved operational
main/suffix,15fresh actual-main outcomes,13worker tests and one isolated action-count
guard mutation/exact restoration. Exact default main replayed in real session1,
PID44240 gone/task absent, zero game starts/attaches/writes, install hashes equal.
[Check](../scratch/phase-c3ay-actual-controller.md). Controller-only acceptance;
observer binding/cleanup and integrated readiness remain pending, no launch marker.
Fresh C3.0635481771FAIL/C1gatedPASS unchanged; initial packet preserved, no renderer change.

**C3az initial/repair1/repair2 minimal observer:** Facilitator verifies22independent
private-memory/lifecycle controls and35worker tests after returning stale-code/read-fault
counterexamples; own primitive-only mutation fails and restores exactly. Scoped API check
does not provide required engine-span proof producer; actual RPC remains hard-disabled.
[Check](../scratch/phase-c3az-bounded-observer-blocked.md). Accept synthetic tooling/blocker,
not integrated/native readiness. Park at3attempts; stop run on unresolved safety prerequisite,
0starts/attaches. C3/C1gates unchanged,44/44CTest/37Python pass; no renderer/reference change.

### C3 Astra observer-readiness review, 2026-10-26

**C3ba initial/repair1 requirement audit:** Facilitator separates preinstall safety,
postinstall outcome and postdetach restoration; returns the missing constructor-proof
ordering and unarmed-installation hazard for correction. AZ requires supplied exact
proof before attach and has no postinstall verifier; unarmed is not uninstrumented.
Own65-byte/21instruction site read,12source hashes and3RPC refusals checked; no native
API accessed. [Check](../scratch/phase-c3ba-requirement-audit.md). No policy change or
implementation accepted; C3bb assesses concrete feasibility.44/44CTest,25framework
tests pass. Zero starts/attaches/host tasks; C3FAIL/C1PASS baselines unchanged.

**C3bb pinned-tooling feasibility:** Facilitator authenticates source/artifact pins,
rejects worker PRE7 prediction (default IA32 loop yields6/5), and narrows the claimed
need for a custom getter: source stores a context pointer before entry trampoline,
providing a concrete candidate read-only route, not a completed verifier. Missing
getter does not prove inaccessible evidence. [Check and recommendation](../scratch/phase-c3bb-verifier-feasibility.md).
Request explicit staged evidence-protocol approval and a narrow offline cap exception;
keep runtime refusal. Build/source binding, private ABI, installation/context safety,
actual decoder and controller/pause/ledger/host-readback integration remain blockers.
No next brief/implementation/native action dispatched.3/3attempts,0starts; C3/C1gates
unchanged,44/44CTest and25framework tests pass. Prior candidate errors retained.

### C3 bounded offline verifier/integration pass, 2026-10-26

**C3bc initial/repair1:** initial prerequisite summary returned for missing concrete
source/artifact inspection and conflating deferred runtime receipts with offline
implementation prerequisites. Facilitator independently checks transaction/RWX
path, Windows's omitted remap-false predicate, full FXSAVE/FXRSTOR callback path,
package/source hashes and executable guard. Source-supported prospective
concurrency safety gap remains; no parser/verifier or controller integration
accepted. [Facilitator check](../scratch/phase-c3bc-verifier-safety.md).
Refusal still returns before native access; seven install hashes and sixteen
initial dirty-file hashes match before landing.44/44CTest,25framework tests pass.
Two attempts spent,0starts/attaches/injection/host tasks; stop and park route for
direction, not automatic integration/live follow-on. C3FAIL0.0635481771 and C1
gatedPASS0.0160864/0.0161212 retained, no rendering/gate/reference change.

### C3 scene-stage ordering, 2026-10-26

User-approved bounded rendering directive, zero native operations; observer park
retained. **C3bd initial/repair1** separates supported primary root59644 from
secondary91782 and calls the latter after TubeScene through the shared production
scheduler. Entrance/Exit remain legacy-unattributed at their former timing, not
primary by exclusion. C3ao mixed-secondary order remains unchanged.
Initial packet returned for missing fresh scores and real-source mutation.
Facilitator re-reads conditional native order/slots, independently parses retained
raw trees (923/82/16/156mesh records,17secondary billboards,29flat secondary children),
reviews actual loops/callback bindings, and discriminates the isolated production
order regression with permanent and separate oracles; exact restoration and44/44CTest.
Stage counters are source candidates, not native visible submissions. Own fresh
repair4/all10C1 images are unchanged: C3 **14934330/235008000 FAIL**, C1zoom1/down
**285083/285700 over17721990 PASS**. Preselected initial,left,elev_up,MM_DIG_000
diagnostics and scene-off also equal baselines; no pixel improvement/protection
claimed. [Check and diagnostic table](../scratch/phase-c3bd-stage-order.md).
Two attempts,0starts; directive's verified-correction/report stop reached, not C3
acceptance. Admission/model-holder timing/clip/packing/effective-state gaps remain;
no automatic next rendering/observer question. Initial unrelated edits preserved
with own PLAN status/history append staged separately; seven install hashes match,
no game/LemWeb task. Actual times and commit in queue/archive.
