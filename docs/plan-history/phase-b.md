# Plan history — Phase B

Moved verbatim from `PLAN.md` on 2026-10-24 (user, overlord review 2026-10-24). Only link
paths were rebased. The live milestone table, exit gates, status cells and user decisions
are in [the plan](../../PLAN.md#phase-b--the-frontend). New round notes are appended under
the milestone they serve, newest last.

## Phase B — the frontend

The original's own screens: backgrounds, buttons, fonts, `.LTX` localised text (English in the
reference install), the screen state machine, level select and the map. Preserve the quirks —
**RETURN only when the menu highlight is exactly 0**. (The "short ESC that fails to leave
pause" quirk was retracted 2026-10-23: ESC at 2/5/10/25/40 frames all left pause, B2h.) Frontend screen ids read as `0x004C416C`: 3 menu, 4 confirm, 5 map — **B1's own recon
corrected the reading of this global**: it is not a hand-picked enum, it is a monotonic
object-creation counter shared by a common frontend-screen base class (ctor `0x004882c0`,
independently re-read byte-for-byte), and each screen captures the pre-increment value as
its own id at construction. 3/4/5 are simply which screens get constructed first in the
boot sequence, not semantically chosen constants — the observed values are unaffected,
only their explanation is corrected.

**Milestones and exit gates, drafted 2026-10-21 (CEO review), signed off as drafted by the
user 2026-10-22.** The metric is the A4d fixed changed-pixel score, scored with
`tools/harness/score_a4_render.py` methods on a full 640x480 frame unless stated. The
threshold is 0.02, the same as A1/A2/A7.

| Milestone | Status | Exit gate |
| --- | --- | --- |
| B1 frontend assets and text | **done** (B1w, own re-score 0.005244; panels measured from the reference, see below). Accepted on condition (user, overlord review 2026-10-23): the panel draw is read and re-scored; a failing re-score reopens B1 | Background, button and font sprites and the `.LTX` strings for the reference language (English) load. One reference screen (main menu, highlight 0) renders full-frame ≤ 0.02 against a stock screengrab |
| B2 screen state machine | queued | Scripted key sequences (menu, confirm, map, into a level, back) visit the same screen ids (`0x004C416C`) in the same order as the original. Quirk preserved: RETURN only at highlight 0. (Short-ESC clause dropped, user, overlord review 2026-10-23: retracted after B2h.) |
| B3 level select and map | queued | Map screen and level-select full-frame ≤ 0.02 on two unlock states. Selecting `MM_CLIMB_002` from the map reaches the D3 level |

**B1 recon** ([check](../scratch/phase-b1-frontend-recon.md)): background/button/font art
is plain LRZ-compressed `.BMP` under `TEXTURES\`, already decoded by the existing
`Box`/`Bitmap`/`Lrz` pipeline; `.LTX` text parsing (`GameData.cpp`) is already implemented
and tested against the real archive. Nothing in `src/app/` wired either into a frontend
load yet; no usable reference screengrab existed.

**B1a: already-tested readers wired into an app-side load** ([check](../scratch/phase-b1-asset-loading.md)):
a new test, `test_frontend`, loads `MAIN_LOGO.BMP` (640×480, opaque), `_FONT.BMP`
(256×240, colour-keyed on black, the key demonstrably fires on real glyph data) and
`GERMAN.LTX` (232 strings) through the unmodified production readers. 29/29 tests pass.
`src/assets/**` untouched.

**B1b: reference screengrab captured — in English, not German** ([check](../scratch/phase-b1-reference-screengrab.md)):
one live, no-RETURN run confirmed (live memory reads bracketing the capture) screen id 3,
highlight 0, image exactly 640×480. Unplanned finding: the menu's own text is English, not
German as this project's own preserved-quirk note describes. The shared install's
`SavedGame.dat` (created 2026-10-12, written many times since across many live rounds) has
no pristine baseline anywhere to compare against, so the cause (an earlier round's
undocumented navigation, or genuine persisted stock behaviour) cannot be determined from
this install alone. Initially recorded as a pending question rather than decided
unilaterally. **Resolved by the user, 2026-10-22:** the German-menu quirk was a mistake
from an early harness language change, not behaviour to preserve. The English reference
is accepted; leave the install's language unchanged. B1's pixel metric is unchanged.

**B1c: the frontend's construction factory traced; main-menu vtable narrowed, not
confirmed** ([check](../scratch/phase-b1-draw-dispatch-trace.md)): a single 37-case
factory (`0x0048726f`) behind two thin wrappers constructs all 20 frontend-screen
subclasses from a remapped selector. Load-bearing negative, independently re-derived from
raw file bytes: the screen-id counter's menu/confirm/map values (3/4/5) and the factory's
own selector are two independent numbering schemes — the menu cannot be found by calling
the factory with 3. A shared "chrome" class (vtable `0x4b1e88`) is the strongest candidate
for list-style navigation screens generally; the menu's actual *content* is narrowed to 1
of 7 candidate vtables (all sharing the `main_logo.bmp` backdrop), not resolved to 1. The
step/input side of the interface is confirmed to stay inside the frontend's own address
range, separate from the in-level draw chain, one level deep; the specific draw call was
not isolated within budget. Font layout remains a clean negative (`_FONT.BMP`'s name does
not appear anywhere in the exe's own bytes). A cheap, live, no-input follow-up (read which
content vtable is active at screen id 3) is queued next to settle the remaining
uncertainty.

**B1d: boot-menu vtables read live** ([check](../scratch/phase-b1-menu-vtable-live-read.md)):
the interrupted worker's saved script/prediction were reused for one no-input stock
start. Three identical counter-3 polls read active-object vtable `0x4B1E88` and an
observed list-head vtable `0x4B2528` (B1c ctor#15), selecting one of the seven content
candidates for that boot state. Facilitator independently checked the accessor bytes,
raw-PE vtable entries, JSON consistency, install/save hashes and cleanup; 29/29 tests
pass. Narrowed the worker's claimed null termination: the poll does not distinguish
an unreadable terminal link from zero, so this establishes one observed node, not
complete-list cardinality. Actual draw submission and font metrics remain unlocated;
no menu render or B1 full-frame score exists yet.

**Approach steering (user, CEO review 2026-10-22):** measure the retained menu image by
template-matching retail sprites and font glyphs in a rerunnable script, then render
that measured layout in `webolution` and score the unchanged full-frame 0.02 gate.
Read executable code only for facts the image cannot show; do not make another native
menu-vtable/draw-chain trace a prerequisite. B1e is the offline layout-measurement
round; the production renderer follows its independently verified measurements.
An independent offline C1 index read using REA may run in parallel.

**B1e: retail-template export prerequisite landed** ([check](../scratch/phase-b1-template-export.md)):
after a prose-only first pass, the brief was split. `FrontendAssetDump` now exports seven
candidate backdrop/font images and all 232 indexed UK strings through existing readers.
The five visible menu labels are UK indices 125/126/120/3/43. Facilitator independently
parsed raw UK.LTX, checked exported pixels/metadata, discriminated an isolated wrong-
language mutation and verified restoration; 29/29 tests pass. No actual image matching
or gate score yet; B1f is the next bounded matching round, then the app renderer.

**B1f: partial image work; glyph inference rejected after3 dispatches**
([check](../scratch/phase-b1-template-match-attempts.md)): background registration and
five candidate colour-support bands are available, but no text reconstruction. The
glyph attempt assumed unverified fixed-cell ASCII packing and normalized source
silhouettes to target size, erasing scale discrimination; its poor scores do not
reject the retail fonts. The text-free diagnostic under0.02 is not a gate pass.
That method is parked in Phase E. B1h has an explicitly justified fourth-pass exception
to test atlas-independent matching with preserved geometry; no fifth automatic pass.

**B1h returned incomplete; standing run stopped by user2026-10-22**
([check](../scratch/phase-b1h-interrupted-quit-registration.md)): original dispatch
hit quota; explicitly counted Mac continuation was interrupted after~65min against35min
budget, without final worker report. Independent saved-output verification reproduces
actual Quit-row crops/control pixels, but wrong letters and substantial residual remain;
no glyph identities/layout accepted. Latest FONT/FONTFE row29712/1472625=.0201762 is
**not** a full-frame B1 score. Six diagnostic tests and29CTests pass. No production
menu/B1 pass. User ends this run here: no new round/repair/phase or milestone advance.

**B1i: the font charmap, advance and placement rule read from bytes, not fitted**
([check](../scratch/phase-b1i-font-charmap.md)), 2026-10-22, authorized by
`docs/DIRECTIVE.md` as the replacement for B1f/B1h's parked pixel-fitting approach. The
per-character build loop `FUN_004404fa` (`0x4404FA`) computes
`cellRecordPtr = glyphTable + (((charByte & 0xff) - firstChar) & 0xff) * 3 + 4`
(`FUN_004404cf` + `FUN_00440d50`) into a per-font-object blob: a 4-byte shared `lineHeight`
header followed by 256 packed 3-byte `{X, Y, width}` records — **proportional width, one
shared height per font**, not a fixed grid (the brief's own fixed-width Hypothesis 1 and
the worker's own pre-reading guess are both falsified; only the `charCode - firstChar`
subtraction survives). The advance is `X += pbVar10[2]` (the table's own per-glyph width
byte), unconditional, with no fixed immediate anywhere in the function. Two real instances
were decoded from raw bytes: `"^Font"` (`lineHeight=29`, proportional) and `"_FontSmall"`
(`lineHeight=14`, digits uniformly width 6 by data, not by mechanism); a genuine quirk —
`&` in `^Font` is an explicit zero-width table entry (`X=74,Y=0,width=0`), present but
invisible, a real preserved-quirk candidate per `match-the-original.md`. Resources resolve
by a short internal name string (`"^Font"`/`"_FontSmall"`), not the archive filename —
refining, not contradicting, B1c's already-settled `_FONT.BMP`-absence negative. A
code-only replay of `"Quit"` reproduces exact per-glyph cells/UVs and a 41 px final cursor
advance, independently re-derived by the facilitator from the raw `^Font` table bytes.
**This is a READ result, not a rendered pixel** — it does not move B1's full-frame gate by
itself (no glyph was drawn in `webolution`), but it removes the "no glyph-layout rule was
ever read" blocker B1f/B1h's parked pixel-fitting attempts left behind. Open: whether
`_FONTFE`/`^FONTH`/`^FONTP` share this exact mechanism (no literal name string found for
any of the three — a well-supported inference, not an observation); `FUN_00474519`
(name→resource-index resolver) untraced. An incidental, unverified lead from the same
read: `FUN_004404fa`'s caller `0x44CD06` is the exact function A7d–A7g already traced from
the Paused-caption receiver `H`, and its body shows `H+0x758`/`H+0x77C`/`H+0x224` each
filled through this same glyph-builder — offered to whoever next picks up that Phase C
thread, not claimed as resolved here. C2a re-read that function from the file;
see Phase C.

**B1j: none of `FUN_004404fa`'s callers draws the five main-menu labels**
([check](../scratch/phase-b1j-menu-callers.md)), 2026-10-22, round 1 of the
B1 render question. A raw-`.text` scan finds 17 direct `E8` calls to
`0x4404FA`, in 12 procedures. None passes UK indices 125, 126, 120, 3, or 43.
`push 126` does not occur in `.text`. The only `push 120` (`0x487511`) calls
allocator `0x49C1A8`. The call-site font operand is `[0x4CBF78]` or
`[0x4CBF7C]`, and constructor `0x440330` stores vtable `0x4B15C8` and
`+8 = -1`, not a glyph table. B1k, below, names the draw. The full-frame
gate is unchanged. No stock start.

**B1k: the five labels are the dwords at `0x4C4214`**
([check](../scratch/phase-b1k-menu-label-draw.md)), 2026-10-22, round 2 of 3.
The file dwords are 125, 126, 120, 3, 43. `FUN_0048ecdf` (`0x48ECDF`, vtable
`0x4B252C`) pushes that table with count 5. `FUN_0048aa5e` walks each string
and calls `0x4404CF` on the font at `[0x4CBF78]`, bound to `"^Font"` / table
`0x4BF968` / `lineHeight` 29. It does not call `FUN_004404fa`. The first row
is 320 when `0x488554` is called with 1 against base `0xDC`. Horizontal
placement is a signed half of the BSS word `[0x58CB40]`; the `sub eax, edx`
before the shift is the `cdq` sign mask, not a second coordinate. The pixel
X and the texture behind the name `"^Font"` are unread. No BMP filename is
on the path. The gate was not scored.

**B1l: the horizontal word and the `^Font` filename are file-negatives**
([check](../scratch/phase-b1l-menu-x-and-sheet.md)), 2026-10-22, round 3 of 3.
The immediate `40 CB 58 00` occurs 63 times in `.text` and none of those
instructions writes the word. `^FONT.BMP` is not in the exe. `0x474519`
looks the short name up on `0x58CB48` and does not take a BMP operand.
The B1 full-frame gate was not scored. This question is at its cap.

**B1m: runtime acquisition incomplete; offline diagnosis verified**
([check](../scratch/phase-b1m-runtime-font-read.md)), 2026-10-23. One stock
start read word 640 (X320), but copied B1b's stale heap highlight address and
mistook the embedded manager at 0x58CB48 for a pointer slot. The resulting
snapshot is executable `.rdata`, not the runtime resource pool. Offline repair
establishes font+8 as a manager-local index 36 and the first missing pointer at
0x58CB84; no source sheet or dimensions can be recovered from this packet's
recorded chain. Two dispatches including repair, sole start spent, no retry.
All install hashes restored and 30/30 CTest pass. B1 remains unaccepted and
unscored; question 2 render is blocked. Run stops pending a separately authorized,
offline-preflighted acquisition, not another fitted sheet or guessed placement.

**B1n: offline predicates remain ambiguous**
([check](../scratch/phase-b1n-font-sheet-discriminator.md)), 2026-10-23,
runtime-question round3. Five existing font exports pass the checked
`^Font` prefix predicates, not six as the candidate reported. No sheet is
accepted; B1 remains unscored. Physical both-archive inventory was not
proved. B1i's blanket256-record extent is narrowed: its asserted `^Font`
window overlaps `_FontSmall` by92 bytes; the known menu ASCII cells remain
valid. `_FontSmall`'s overlong-window negative is not a resource rejection.
Own cell/byte checks, isolated ink mutation and30/30CTest pass. The active
directive now permits the conditional fourth-round fallback, with full
offline pointer/source/highlight preflight before any stock launch.

**B1o: conditional fallback preflight failed; B1 parked**
([check](../scratch/phase-b1o-font-binding-preflight.md)), 2026-10-23,
runtime-question round4 explicit exception. At the eight-body cap neither
current-process highlight nor full actual-source attribution was proved.
The loader has Lo/Hi and original-name BMP fallback paths; dimensions and
the short name alone do not select the source. Partial mocks are not full
preflight. No stock start spent, no render/score. Own41pin/eight-range
checks, canonical14record authentication, isolated offset mutation and
30/30CTest pass. Per directive stop, no fifth round or milestone advance.
Resume requires new authorization for the exact unresolved native edges,
not a guessed sheet, pixel fit or automatic use of the unused start.

**B1p / B2a (2026-10-23, directive questions 1 and 3, round 1 each):** the generic
loader is read ([B1p](../scratch/phase-b1p-texture-loader.md)): `^Font` converges on
the base-archive BMP+ALF pair only under a two-archive, no-override premise, so no
unique source and no B1 render yet. B2 preflight
([B2a](../scratch/phase-b2a-sequence-preflight.md)) failed closed at its body cap with
zero stock starts. Neither moves a gate.

**B1q / B2b (round 2 each):** archive order is PATCH, LEMMINGS, CDPath LEMMINGS
([B1q](../scratch/phase-b1q-archive-open.md)); `^Font` still converges on the base pair
but registration-before-load is unread. B2b
([check](../scratch/phase-b2b-highlight-keypath.md)) read the key-poll flag and the focus
store, not a menu chain. Gates unmoved. B2 round 3 (`b2c-menu-chain`) is running.

**B3a: no instruction selects screen id 5**
([check](../scratch/phase-b3a-map-arm.md)), 2026-10-25, map-draw round 1
of 3. `0x488359` loads `0x4C416C`, stores it at `[eax+0x2c]`, adds 1, and
writes it back. `0x488377` returns with no call. The facilitator found
the address three times, and expecting add immediate `2` failed. No
stock start. The draw call stays unread. The gate is not scored.

**B3b: the screen creations are not in one order**
([check](../scratch/phase-b3b-map-object.md)), 2026-10-25, map-draw
round 2 of 3. `0x4872B8` jumps through a table indexed from
`[ebp-0xb4]`. The facilitator counted 20 direct calls of `0x4882C0`,
and expecting the bound `0x25` failed. No stock start. The object
stamped 5 stays unread. The gate is not scored.

**B3c: the jump index is a stack argument**
([check](../scratch/phase-b3c-map-index.md)), 2026-10-25, map-draw
round 3 of 3. `0x487294` loads `[ebp+8]` and `0x487297` stores it at
`[ebp-0xb4]`. The facilitator re-read that pair, and expecting
displacement `0x0C` failed. No stock start. The argument is not
`0x4C416C`. The gate is not scored.

**B3d: both callers pass their own argument**
([check](../scratch/phase-b3d-map-callers.md)), 2026-10-25, map-draw
round 4. `0x486FF7` and `0x488713` push `[ebp+0xC]`, not the creation
counter. The facilitator re-read both calls, and expecting displacement
`0x08` failed. No stock start. The question is parked. The gate is not
scored.

**B3e: `MM_CLIMB_002` is not in the executable**
([check](../scratch/phase-b3e-level-name.md)), 2026-10-25, level-pick
round 1 of 3. The exe has no copy of the name and no `.LVL`. The
facilitator read `lemmings.box` at `0x6F77` as
`COMPILED\MM_CLIMB_002.LVL`, and expecting `0x4E` failed. No stock
start. No code reference. The gate is not scored.
