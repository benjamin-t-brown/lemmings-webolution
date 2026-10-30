# Frontend and navigation findings

> Dated historical research excerpts, preserved in their original wording and order within each source range.
> Earlier conclusions may be superseded by later entries; this is not current project status or
> authorization. See [STATUS](../STATUS.md) for current scope and [LESSONS](../LESSONS.md)
> for essential startup guidance. Some excerpts refer to findings that appeared earlier in the
> former combined file; use their linked evidence and the source-range comments below.

**RETURN correction (OBSERVED, 2026-10-28 19:51Z):** the blanket “RETURN only
at highlight0” claim is false for row1. [Predeclared negative check](../scratch/frontend-return-negative-prediction-2026-10-28.md)
observed DOWN selecting structural row1 (Play Current Game), then RETURN changing
Menu3/UI3 to Map4/UI3. The worker stopped on the falsified unchanged-counter
prediction; no positive-control or map-frame claim. Do not mistake the old
source model's unsupported-row guard for native behavior. Other rows and signed
B2 wording remain separate; preserve earlier evidence and the failed prediction.
The [production fix](../scratch/frontend-menu-focus-navigation-2026-10-28.md)
uses one model-owned focus for action and cached rendering; reset on page
replacement must invalidate the displayed selection, not only the action index.

**No/Resume return (OBSERVED, 2026-10-28 20:16Z):** [bounded connected capture](../scratch/frontend-no-resume-comparison-2026-10-28.md)
confirms No→Menu5 and mouse Resume menu9→Level9/UI0, then menu11→Quit Map13→Menu14.
No preserves the native baseline `[1]+[0]*101`; Yes resets it before entry. Clone
startup does not load that save, so screen/frame correspondence is not pre-Yes
progress parity. Keep pointer position and held-state assertions on composed
exports, and regenerate retained route exports rather than trusting stale files.

**Loss/Retry/Quit (OBSERVED, 2026-10-28 21:43Z):** [natural no-assignment losses](../scratch/frontend-loss-retry-quit-2026-10-28.md)
confirm Results7/UI5→Retry Level7/intro0→Results9/UI5→Quit Map11/UI3/intro1→Menu12.
Both saved0/needed1 and unchanged fresh flags. Result globals are DWORDs after
sign-extending controller words. The failed first attempt wrongly aimed gameplay
using frontend integer feedback; the successful follow-up sends no input during
gameplay waits and mode-guards frontend aiming. Timing/movies remain unverified.

**Frontend cursor (READ, 2026-10-28):** do not use the gameplay hover/Grab policy
for menus, map or results. `453BF6..453D98` tests frontend flag `4C369D`, chooses
normal1 (`_curNoLem`) or held10 (`_curMseDwn`) for either held button, then skips
gameplay selection. Right-hold still moves; it is not Grab. XY starts320,240 and
clamps inclusively to width/height. The software sprite is drawn after frontend
screens; the alternate backend uses different monochrome PE cursors. See the
[cursor and return-route check](../scratch/frontend-cursor-return-coverage.md)
and `tools/harness/read_frontend_cursor.py`. Retained normal-cursor RGB agrees
with a five-bit conversion, but the native conversion mechanism is unproved;
do not add a frontend-only color exception or claim exact native pixels.

**Root/page lifecycle (READ plus bounded observation, 2026-10-28):** a new
[native return attempt](../scratch/frontend-native-return-2026-10-28.md) stopped
when in-level Quit produced Map counter9, not the predicted8. `486F60` constructs
a root at `5D59EC` **and** a page at `5D59E4`; both consume a serial through
`4882C0` (initial counter1). Thus Level5 → menu7 → QuitMap9. Do not count only
visible pages or apply+2 to every transition: replacements inside an existing
frontend (including results Continue) count one; full results creation and loss
Quit create root+page. Resume/O-close/retry destroy without decrement. Captured
Map9 is a single read, not a returned-map image/UI/flags or completed B2 gate.
The same run observed normal1/right-held10 with software backend0; native cursor
colors and the second unlock state remain open.

The connected implementation now uses explicit root ownership and semantic menu
notifications, not legacy `press(Esc)` hold-count replay or a hard-coded return
offset. An actual SDL-dummy route checks the persistent counter across level
windows; isolated omission of either the root count or menu-open notification
fails its regression. Keep the observed prefix ending Map9 separate from the
READ-predicted footer→Menu10 and unobserved results routes.

**Capture precondition (2026-10-28 follow-up):** static initial XY320,240 is not
a guarantee after startup and input activation. The next authorized run captured
stable317,241 through Menu3/Confirm4/Map5 and stopped on the unestablished center
assumption, before testing return. Explicitly aim/verify a declared comparison
position with the bounded shim helper; retain pre-aim observations. Do not blame
the user or infer the offset's cause without evidence. See the
[failed follow-up](../scratch/frontend-native-return-2026-10-28.md#fresh-follow-up-stopped-on-cursor-precondition).

**Native progress reader:** address `0x4E36E8 + 0x1B58*group + 0x118*slot`,
**DWORD** width; strides are7000/280 decimal, not0x7000/0x280. Misreading the
radix corrupts101/102 node addresses while node0 and an all-zero fixture can
appear correct. Verify a non-first-node fixture and encoded stride bytes; save
unexpected raw values before asserting equality. Earlier2026-10-28 capture flag
vectors are retracted as full102-node evidence, not the separate map states or
images. See the [correction](../scratch/frontend-native-return-2026-10-28.md#completion-reader-radix-correction).

**Complete bounded return (18:27Z):** the [successful capture](../scratch/frontend-native-return-complete-2026-10-28.md)
now observes Menu3→Confirm4→Map5→Level5→menu7→Map9→Menu10, correct102-field
progress equality and returned-map/footer/menu images. Main-menu focus is a
**row pointer** at `[[page+0x10]+0x10]`, indexed among five pointers at page+0x40;
validate ownership, row identities and stable brackets, never choose a coincidental
0–4 heap DWORD. This structural reader was byte-tested and validated on initial/
returned native row0. No nonzero-focus RETURN or second unlock state is inferred.

**Natural second-state follow-up (18:58Z):** [native win/Continue capture](../scratch/frontend-native-second-unlock-2026-10-28.md)
now observes natural10/10 Climber completion, Results7 → Map8, flags
`[1]+[0]*101` and map states `[2,3,3,3,3]+[1]*97`. Both map states pass the
bounded B3 image gate. This supersedes the missing-second-state scope above,
not the timing park or broader frontend limits.

**UI mode is not page identity (READ + win observation):** `45DF33` writes UI4;
Continue `486090/48609E→4886E4→48726F` replaces only the page and retains UI4
and frontendflag1. The sampled completed map is **UI4**, not UI3. Quit's UI3
write `45D743` is request-gated. Keep dispatch mode independently in the model;
do not normalize it whenever Map/Menu is displayed. Loss Continue retention of
UI5 is static/source-tested, not a new live loss observation. `test_results`
rejects isolated Continue-reset and Quit-preservation mutations; full-init and
page replacement must differ in mode publication as well as root ownership.

<!-- Source lines 26-116 in the pre-split docs/LESSONS.md; excerpt preserved, relative links rebased. -->
**Later connected-frontend work:** under a separate explicit desktop-use window,
the [2026-10-28 comparison](../scratch/frontend-native-comparison-2026-10-28.md)
captured menu3 → confirm4 → fresh map5 → `MM_CLIMB_002`5 in one owned stock
process. Save/config restored byte-exactly; the original is closed. Menu/confirm/
one fresh-map-state frames pass0.02; two-state/back-route gates remain open.
The map footer is UK127, shared row builder488A92 at49AC90, textY440; semantic
event0 from `[map+40]` in49B1E4 constructs main menu selector7. This is now wired
in `FrontendSession`; SDL click/Return remain explicit adapters. The dated capture
script is spent/expired evidence, not permission to relaunch. Historical rejected
launcher/component findings above are unchanged.

**Opening camera:** do not replace the settled MM_CLIMB_002 yaw32 to fit an
entry screenshot. [45CC09's three-second intro](../scratch/level-entry-camera-intro.md)
adds an unscaled350-distance ease and a full pose revolution, computed before
clock advancement and frozen by pause. It is now wired for the first-level
frontend path; other-level selected-hatch metadata and clock-matched native
animation validation remain open.

**Skill interaction:** retained455611 refuses panel selection at stock<=0;
455756 resumes UI1→0 after a successful assignment. Both are now wired and
[tested through the application](../scratch/first-level-skill-interaction.md), without
another native run. [Overlap continuation](../scratch/first-level-overlap-selection.md)
now wires the controller target/raw-distance keys, four constructor-proven
priorities and necessary Climber exclusions. Do not conflate this with full
picking: native visibility, facing and complete skill applicability remain open.
Actor+29 is a tube-render visibility result (43A9A4/43AA30), not merely hatch
emergence. Controller+58 is the hover reference; physical eye/intro pose is not.
[Visibility/support continuation](../scratch/first-level-pick-visibility-support.md):
draw and pick now share the inclusive bucket gate for shellZ<1; shellZ>=1 keeps
the native bypass. Ordinary left-Walker support X bias-1 is cancelled by42E913's
+1, so the bounded first-level source support probe is `(x,clamp(y+1))`, not
`(x+1,y+1)`. The authored first-level word alphabet proves solidity equivalence
only within that scope; sim raw-word retention and inverted anchors remain open.

[Runtime cursor continuation](../scratch/runtime-cursor-hover.md):455CB0 is an
always-true stub;455DD0 tests DirectInput button1 **held**, not its edge. Mode2
is the failed registry-read fallback, not an unconditional native startup value.
The software resources are full32x32 bitmaps centered at integer cursor minus16;
Grab uses fractional ALF alpha and SRCALPHA/INVSRCALPHA. SDL absolute clicks must
not share feedback with a separately integrated virtual cursor: our bounded
adapter explicitly uses absolute window coordinates for both, without changing
the accepted camera controller's relative arithmetic. Native sampler/backend
pixel equivalence and alternate mode/pressed/object-marker producers remain open.

[Results/map continuation](../scratch/level-results-map-return.md): UI4 is **win**
results (mode3,selector21), UI5 is **loss** (mode4,selector20). They are not
successive finish stages. Both Continue handlers select map23. Win builder485E55
writes selected completion flag1 before Continue; loss does not erase it. Use
MapScene's native mapping/graph, not the older SOURCE sequential Progress unlock.
Results use SMALL_BACK, red titleY220, count linesY280/310; hidden empty row0 is
not a blank panel. This step left native initial focus repair and finish-delay/
list semantics unresolved; the latter are refined by the follow-up below.

[Pending-end closure](../scratch/level-end-transition.md) resolves the latter:
445436 counts retained non-rat actors; actor+4C is sheet type3=`EvilRat`, not a
death marker. Keep pending dying animations counted (`alive`, not `active`).
5098 is selected-type unreleased hatch population. Ordinary load seeds timer7;
the trigger call does not decrement it; pending uses strict<3 and<1. Simulation
and hatches continue. Win/loss is latched at trigger, but results counts are
current at the final call, so late saved>=needed must not upgrade a timeout loss
or be rejected as an inconsistent snapshot. Effect draw490ED6 is empty: do not
invent a fade. Results keyboard focus, movies and native dt equivalence stay open.

[Loss Quit closure](../scratch/loss-results-quit.md):486583 writes4CBFAA;45D72F
consumes it and builds map23, not Menu or process exit. Keep Quit distinct from
Continue despite their common destination.4E3584 controls next-load intro:
Retry=0, Quit/map selection=1; it is not a post-map routing flag. Map footer
selects Menu7; map cancel separately writes application exit4E3581. Clear clone
launch-request ownership on Quit but preserve completion flags; only confirmed
New Game resets owned progress. Native selected path5D59F0 can remain populated
because independent reload flag5D5A0A—not pointer existence—controls launching.

[Entry cue](../scratch/level-entry-start-cue.md): loader45CAEA pushes **hex47=71**,
UK `Let's Go!`, not decimal47/Continue or the selected level name.4151D0 is a
font getter. Title clock starts3.5 map/1.5 retry, decrements before draw (unlike
camera intro), appears at<=1.5 and expires onlybelow-1; pause freezes but still
draws. Squared stretch uses huge early rectangles and cumulative glyph edges.
Negative time writes vertex alpha and requests additive mode3, but normal
caret-font flags8 selects TEXTURE alpha, ignoring the vertex countdown. Do not
invent an opacity fade or paint a filled panel. Effective sampler/device pixels
remain unverified; the software POINT adapter is not native visual acceptance.

[In-level menu](../scratch/in-level-menu.md): selector1F/table4C4228 has
Resume/Try Again/Quit/Options atY100/129/158/187, plus an empty fifth widget.
No bound background or evidenced dimming quad: keep the scene/cue underneath.
Resume and menu cancel request UI0 even from paused-origin; only O-close writes
UI1. Do not generalize the old observed O→pause to every dismissal. Strict mouse
hits use content rectangles, not padded panels. Quit requests Map23 with no
outcome/progress reset and no intro-preference write; typed leg dispositions
are needed so the persistent app does not interpret abandonment as process exit.


<!-- Source lines 924-1074 in the pre-split docs/LESSONS.md; excerpt preserved, relative links rebased. -->
## Frontend checkpoint: B1 boot-menu vtables observed

[B1d](../scratch/phase-b1-menu-vtable-live-read.md), one no-input stock boot, three identical
screen-counter-3 reads: the active global `[0x5D59EC]` points to an object with vtable
`0x4B1E88`, and the observed list head `[0x5D59E4]` has vtable `0x4B2528` (B1c ctor#15,
one of its seven backdrop-sharing candidates). Getter bytes at `0x487F93` / `0x487FB3`
and next-link offset +4 at `0x487BCA` were independently re-read. Do not repeat the
seven-candidate identity search. This is not a traced draw routine, semantic proof of
the list's role, or a B1 pixel-gate pass. The saved poll cannot distinguish an unreadable
terminal next pointer from null; one observed node does not prove list cardinality.

## Menu text is not a FUN_004404fa caller

[B1j](../scratch/phase-b1j-menu-callers.md): 17 direct `E8` calls to `0x4404FA`,
in 12 procedures. None passes UK indices 125, 126, 120, 3, or 43. `push 126`
does not occur in `.text`. The only `push 120` (`0x487511`) calls allocator
`0x49C1A8`. The five labels' draw function is still unread. Do not repeat this
caller scan, and do not treat those calls as the main menu.

## Main-menu labels are the table at 0x4C4214

[B1k](../scratch/phase-b1k-menu-label-draw.md): the file dwords are 125, 126,
120, 3, 43. `FUN_0048ecdf` pushes that table with count 5. `FUN_0048aa5e`
walks the strings through `0x4404CF` on the `"^Font"` object at `[0x4CBF78]`.
The first row is 320. Horizontal placement is a signed half of the word at
`[0x58CB40]`, and that word's value is not in the file. No BMP filename is
on this path. Do not repeat the 17-call scan.
[B1l](../scratch/phase-b1l-menu-x-and-sheet.md): the address immediate occurs
63 times and none writes the word. `^FONT.BMP` is not in the exe.
`0x474519` looks `"^Font"` up on `0x58CB48`. Do not scan those 63 sites
again, and do not invent the horizontal pixel.

[B1m](../scratch/phase-b1m-runtime-font-read.md), one stock start plus an offline
repair: saved word `80 02` at `0x58CB40` is 640 (X320), but the highlight memory
bracket failed: B1b's heap address is not portable to another process. Font+8
is a manager-local index (36 in this ended process), not an archive ordinal.
**0x58CB48 is the embedded manager, not a pointer slot.** Dereferencing it
captures the vtable's `.rdata`, not the manager pool. The first missing sample
is pool base `[0x58CB84]`; source sheet/dimensions remain unknown. The one-start
allowance is spent; no live retry or B1 render/score accepted.

**B1n ([check](../scratch/phase-b1n-font-sheet-discriminator.md)):** the checked
`^Font` table prefix is consistent with five exported font sheets, so
bounds/ink/blank predicates do not identify the source. No renderer or B1
score. B1i's blanket256-record extent is unestablished: its772-byte window
overlaps `_FontSmall`'s known header by92bytes. The menu ASCII cells and
Quit replay stand. B1n's `_FontSmall` negative reads an unproved range and
is not a resource rejection; both-archive completeness was not proved.
Next is the directive's conditional offline live-acquisition preflight,
not a guessed font or another glyph fit.

**B1o ([check](../scratch/phase-b1o-font-binding-preflight.md)):** the fallback
preflight failed at its eight-body cap, **before launch**. `0x403720` and
`0x406500` return their input record address, not a string pointer field.
The loader at `0x47218A` has conditional Lo/Hi, BMP existence/fallback and
ALF paths, with key descendants unexamined. No actual source or highlight
accessor is established. Partial decoder tests do not close either gap.
B1 is parked under the directive; no fifth round or automatic use of the
unused conditional start. Own byte/bundle/mock checks and30/30CTest pass.

**B1p ([check](../scratch/phase-b1p-texture-loader.md)):** the generic loader is read,
the actual `^Font` source is not closed. `0x47244B` tests for `INN_`/`OUT_`; `^Font`
matches neither, so `[0x5CD5F2]==0` tries `^FontLo` and nonzero tries `^Font`
(`^FontHi` is unreachable). Neither archive has a `Lo`/`Hi` member, and `lemmings.box`
alone holds `TEXTURES\^FONT.BMP` / `.ALF` (ordinals 633 / 632), so both cases converge
there **if** only the two archives are registered and nothing loose overrides. That
premise is unproved: `0x47E40D`'s non-archive route, `[0x5D5604]`, `0x47E89B`, manager
`[0x5D5608]` order and `0x4805B8` are unread, and `0x47F759` takes the first
registered match. Source is a BMP and an ALF companion. Not a unique-source claim.

**B2a ([check](../scratch/phase-b2a-sequence-preflight.md)):** preflight failed closed at
the 12-body cap, zero starts. No byte-proved highlight accessor, native key producer
or safe confirm/map/level/pause/back route; `0x004C416C` stays a construction counter.

**B1q ([check](../scratch/phase-b1q-archive-open.md)):** `0x47E23B` registers `PATCH.BOX`,
then `LEMMINGS.BOX`, then `LEMMINGS.BOX` from registry `CDPath` (`0x4C2A54` / `0x4C2A60` /
`0x4C2A70`); `0x47DD54` registers nothing; `[0x5D5604]` has no exact writer. `patch.box`
has no `^Font` member and the `CDPath` copy of `lemmings.box` is byte-identical, so B1p's
convergence survives. Not shown: that registration ran before the font load at `0x45019E`
(callers `0x450771`, `0x45BEA7` unread). Still no unique-source claim.

**B2b ([check](../scratch/phase-b2b-highlight-keypath.md)):** `0x483212` sets `[0x5D5970]`
to "any key newly pressed" (a flag, not a key identity); `0x48896A` events 2..5 end in the
focus store `[container+0x10]` at `0x48F80C`. Candidate highlight
`rows.index([[menu+0x10]+0x10])` is a shape, not an accessor: no file-backed chain to the
`0x4B2528` menu and no key-to-event edge. The worker exceeded its body cap (about 17 of 12).

## Texture loader order (B1r, 2026-10-23, READ only)

Startup `0x450696` calls the archive registrar `0x47E23B` at `0x450771` before `0x486F60(.,7)`
at `0x450A46`, whose chain pushes `^Font` at `0x4403B5`. The registrar body is skipped when
`[0x5D5610]==0`; the setter is unread, as are the loose-file helpers and the working directory.
`^Font` -> `lemmings.box` BMP+ALF stays conditional. Evidence: `docs/scratch/phase-b1r-load-order.md`.

## Live `^Font` read (B1t, 2026-10-23, OBSERVED, negative)

One stock start under `trace_font_source.py --launch` (`starti`, one hardware execute breakpoint at
`0x47E4AD`, 90 s): no hit, so the `^Font` source is **not observed**. This does not distinguish the loose route,
a load before the breakpoint was live, a debug register that missed the loading thread, or a condition that did
not match. `starti` itself works on the stock exe. Evidence: `docs/scratch/phase-b1t-font-live-read.md`.

## Both-arms preflight (B1u, 2026-10-23, OBSERVED on a throwaway, READ on the exe)

Hardware breakpoints set at the `starti` stop do not fire in the main thread (throwaway exe, GDB 17.2); only later
threads. B1t's single breakpoint was probably blind for that reason. Arming from a one-shot software stop at the entry
point `0x49D518` works; it is a debugger probe beyond the two hardware breakpoints, so it needs authorization. A third
route (`jne 0x47E598` when `[0x5D5604] != 0`) executes neither arm. Not run against the stock exe. Evidence:
`docs/scratch/phase-b1u-font-both-arms-preflight.md`.

## Pause exit keys (B2g, 2026-10-23, READ only)

In pause (`0x45D459`) a newly pressed P (action `0x13`) **or** ESC (action `0xE`) stores `ui_state` 0 at
`0x45D508`; the key poll `0x483212` sets a per-action flag only on a new press, so the code has no hold-length
test. This agrees with B2e's one hold-25 sample and does not support the preserved short-ESC quirk, but the quirk
is neither confirmed nor dropped until the live sweep. ESC in a level stores `ui_state` 2 and builds screen `0x1F`
(not the map). Evidence: `docs/scratch/phase-b2g-esc-sweep-preflight.md`.

## Live ESC sweep (B2h, 2026-10-23, OBSERVED)

ESC held 2, 5, 10, 25 and 40 frames in pause: `ui_state` 0 every time, counter 5, list empty. The short-ESC quirk
did not reproduce and is retracted. ESC in a level: `ui_state` 2, counter 5 to 7, one head node with vtable
`0x4B26B0`; O returns to pause. `+8` on a stacked node and level-to-map remain unobserved. Evidence:
`docs/scratch/phase-b2h-live-esc-sweep.md`.

## Level to map, handler to store (B2j, 2026-10-23, READ only)

Screen 0x20's handler `0x486510` (vtable `0x4B1CB0` slot 11) calls slot 3 `0x4886E4` with selector 0x23 when the
event id equals `[screen+0x48]` (`0x486599`); the factory builds ctor `0x49AA90`, vtable `0x4B3EB8`, which is the head
node B2h saw after the map-node click (so 0x23 = map is an inference). The 0x1F dialog builds 9 / 0x19, never 0x23;
`[level+0x55A2]` has no setter. The input that produces the event id and the trigger that builds screen 0x20 are
unread; no key or click path is named. Stacking candidate: `0x4886E4` skips the `0x4884F4` call on the old head, but
whether the old node survives is unread. Evidence: `docs/scratch/phase-b2j-level-to-map-preflight.md`.

## Level to map, input end (B2k, 2026-10-23, READ only)

The `[screen0x20+0x48]` value is a button object pointer stored by the builder `0x486335` (table `0x4C3414`) through
`0x488A92`, not a control id from the ctor. Producer: button vtable `0x4B1E30` slot 1 `0x48FCDD`, calling parent slot 11
with the widget, fed by the key-state table `0x483212`. Screen 0x20 is built when the main-loop mode word `[0x4C6A08]`
is 4 (`0x45DF5C` -> `0x45E086`); its writer is unread (no direct immediate store in `.text`). The old head node
survives `0x4886E4`. No key or click path is named; no start. Evidence: `docs/scratch/phase-b2k-level-to-map-input-end.md`.

## B2 menu object chain (B2a-B2c, 2026-10-23, READ only)

`[0x5D59E4]` is the screen-list head (`0x487FA0` writes it; `0x487CC0` stores `[obj+4]`). The
main-menu object (vtable `0x4B2528`) is built only for screen id 7 (`0x48744C`, `0x5C` bytes,
sole ctor caller `0x487469`) and is found by walking `+4` from the head. `[0x5D59E8]` has one
writer (`0x486ECE`); `[0x5D59EC]` is the chrome object, not the menu. Open: key -> event
producer for `0x48896A`, and the writer of `[menu+0x10]`. `0x487C60`/`0x487C80` store
`+0x24`/`+0x28`, not `+0x10`. B2 is parked (PLAN Phase E); no stock start used.
Evidence: `docs/scratch/phase-b2c-menu-chain.md`.


<!-- Source lines 1448-1462 in the pre-split docs/LESSONS.md; excerpt preserved, relative links rebased. -->
## Live `^Font` source (B1v, 2026-10-23, OBSERVED)

One stock start with both arms armed from a one-shot entry-point stop (the exe tolerated the int3): two hits on the
archive arm `0x47E4AD`, `Textures\^Font.bmp` (77 bytes at 44312738) and `.alf` (61440 at 44251298), gate `[0x5D5610]`=1,
zero loose `^Font`. Matches `lemmings.box` ordinals 633 / 632 read independently. Exactly one source up to the second hit;
a later third source, TLS callbacks and the archive-only route are not excluded. Evidence: `docs/scratch/phase-b1v-font-live-read.md`.

## Main-menu render (B1w, 2026-10-23, OBSERVED + READ)

Highlight-0 main menu from the observed `^Font` sheet and the read charmap scores 0.005244 full-frame against the B1b
reference (gate 0.02; own independent re-score). Labels are **centre-anchored**: `0x48A720` tests flag `0x40` at `0x48A747`
and offsets by half the width, so the brief's left-edge guess was wrong. Panels are measured from the reference, not read;
the backdrop is up to 7 levels lighter than the reference (cause unmeasured). Evidence: `docs/scratch/phase-b1w-menu-render.md`.



<!-- Source lines 1471-1477 in the pre-split docs/LESSONS.md; excerpt preserved, relative links rebased. -->
## Menu panel geometry (B1x, 2026-10-23, READ, partial)

`0x48B1C7` is a constructor tail, not the panel draw. The panel object (vtable `0x4B1E28`, ctor `0x489999`) holds a fill quad
(`obj+8`) and a border quad (`obj+0x1B0`); `0x489B39` insets the fill by 2 and the border by 1 from the rect `0x48A190` builds from
fields `+0x34..+0x48`. Those fields, the colour meaning of the ctor's 255.0 / 64.0 doubles and the highlight setter are unread, so
the renderer's panel values stay measured. Evidence: `docs/scratch/phase-b1x-panel-draw-read.md`.


<!-- Source lines 1492-1498 in the pre-split docs/LESSONS.md; excerpt preserved, relative links rebased. -->
## Menu panel colours and X pad (B1y, 2026-10-23, READ, partial)

The panel fill colours are constants in the setter `0x489ADC`: (0xE0, 0x80, 0) when the flag byte `[+0x358]` is non-zero, (0xE0, 0x0A, 0)
otherwise, through `0x489A84` (0..255 channel values, no tint on the path); the border is 64.0 x3. The X pad is 0x14 minus the border
inset of 1 (derived). `panelTop` 1 and `panelHeight` 27 stay measured. Score unchanged at 0.005244. Evidence:
`docs/scratch/phase-b1y-panel-fields-colour.md`.


<!-- Source lines 1704-1708 in the pre-split docs/LESSONS.md; excerpt preserved, relative links rebased. -->
**B3k (READ/data, [negative](../scratch/phase-b3k-first-node-entry.md)): map selection uses indexed mask bytes, not RGB or a radius.**49ABC8 reads `(479-y)*640+x`, background equal to first stored byte;49B26A requires state2/3. Decoded named mask's background102 and node0's221pixels are resource-specific. The map's455DB0 query tests current object+0==1, fed from buffer byte12highbit by483B5D; it does NOT consume the separate new-press flag+10. Physical-device/button binding remains unclosed: ANYINSTANCE format declarations/nearby constructor are insufficient. No MapInput/actual app handoff landed; initial6+repair4bodies/campaign16attempts exhausted. Do not infer SDL down/up/left binding or repeat filename/counter crawls.

**B3k continuation6 (conditional DERIVED, [check](../scratch/phase-b3k-native-binding-partial.md)): constructed mouse is now linked to the custom format, NOT yet to the shared map context.**482FBA→482F76→482D40 creates enumerated mouse/4839A0 and stores C+C;450873→486A5E copies runtimeG4CBFC0 to sharedS5D59DC. C→G storage remains unclosed. ANYINSTANCE means first available button, generally left only under conventional unremapped/button0-first conditions. Installed shim bytes distinguish disabled forwarding from enabled syntheticIPC byte+1C→sample+12; no runtime route/physical sample observed. Six additional bodies, wrongwriter lead charged; no app entry. Concurrent [capture breach](../scratch/phase-b2m-native-capture-safety-stop.md) ends newpass:2actual/1authorized starts, failedforcedexceptionpreflight, firstimages overwritten, secondunqualifieddesktop frame. New helper rejected/preserved scratch; no automatic retry or finalattempt4.

**B2n (offline decision, [check](../scratch/phase-b2n-offline-recovery-review.md)): reuse established capture/navigation seams, not the rejected launcher.** B1b remains qualified640x480 English highlight0; B2h's loaded-path/raw packets prove one scripted shim map-to-level entry, not physical-left/SDL binding or complete B2/B3 acceptance. C→G/shared identity remains the minimum binding gap, not grounds to erase earlier entry. Saved executed source/terminal receipts confirm TestOnly present but failed refreeze left old worker ignoring it, geometry refusal falsely earned exception PASS, no durable single-use enforcement and overwritten first images. Later working-source guards are not frozen executed guards. Recommend separately approved offline safety-boundary repair only; no new capture system/live retry, no current authority or gate movement. Retained scores only; all historical budgets/parks unchanged.
