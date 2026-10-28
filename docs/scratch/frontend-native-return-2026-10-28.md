# Native cursor / menu-return attempt — 2026-10-28

**Subsequent bounded route captured successfully:** see the
[18:27 complete return capture](frontend-native-return-complete-2026-10-28.md).
This note retains the preceding failed attempts and corrections, not current
unqualified claims that no returned-map observation exists.

**Partial observation, failed return-counter prediction.** One owned stock
process reached the in-level menu, then Quit returned to a Map page at counter9,
not the predeclared8. The worker stopped immediately. It did not capture the
returned map, inspect its flags/UI state, activate the footer, or reach a second
unlock state. No retry was made. B2/B3 are not closed.

This follows the [offline cursor package](frontend-cursor-return-coverage.md).
Do not replace the original failed prediction with the later explanation below.
A separately authorized [17:43 follow-up](#fresh-follow-up-stopped-on-cursor-precondition)
also stopped, this time before level entry on an assumed cursor position.
**Later correction:** the earlier flag samplers misread decimal strides as hex;
their102-word vectors are not valid102-node completion evidence. See the
[radix correction](#completion-reader-radix-correction); page sequences, pixels
and separately sampled map node states are not retracted by this reader bug.

## Authorization, ownership and restoration

The user explicitly granted one capture in an exclusive desktop window of up to
15minutes. First clock read after approval:16:34:31Z; fixed expiry16:49:00Z.
The requested route was New Game → map → first level → in-level Quit → map →
main menu, plus cursor checks. Timing research was not reopened.

Packet: `build/tmp/enacter-native-return-20261028/`. The previously qualified
owned-process envelope was copied into a new packet, with cursor brackets and
the declared return inputs added; the old packet was not changed. Syntax and
harmless owned-Python-child tests passed. Read-only preflight at16:38:24Z found
no stock-like process and the authority SHA256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

The single-use claim was recorded at16:39:28Z, before config/process effects.
The supervisor backed up save/config; only dgVoodoo FullScreenMode was temporarily
changed. The hidden worker owned stock PID47984 in a kill-on-close job, verified
foreground after the reviewed masked/balanced ALT acquisition, and used the
installed shim. Each saved image has stable screen/cursor/UI brackets and a
visible, unobstructed640×480 client box `(10,0)-(650,480)`. Main-agent inspection
of all six images confirms game pixels and English labels. No masks, crops,
alignment changes or corner exclusions were applied to scoring.

On the mismatched counter, cleanup restored the input snapshot and verified the
owned stock process handle signaled. Worker PID17972 exited naturally; the
supervisor verified no stock-like process before restoring save/config. All four
hashes (save, config, exe, shim) agree before/after. Independent main-agent checks
at16:40:05Z confirmed those hashes and no stock-like process. The user was told
the desktop was free. This one launch is spent; the unused time is **not** a
second-launch grant. Backups, failure report and executed source remain retained.

`python build/tmp/enacter-native-return-20261028/verify_packet.py` checks source
and image hashes, snapshot sequence, sampled words, held state, the expected failure and
restoration receipts. It does not retroactively supply missing observations.

## Observed sequence and limits

| Capture / final read | Counter | Page-list vtable | UI | Frontend flag / cursor state |
| --- | ---: | --- | ---: | --- |
| `menu.png` | 3 | `4B2528` | 3 | 1 / 1 |
| `confirm.png` | 4 | `4B3F38` | 3 | 1 / 1 |
| `map.png` | 5 | `4B3EB8` | 3 | 1 / 1 |
| `map-right-held.png` | 5 | `4B3EB8` | 3 | 1 / 10 |
| `level-entry.png` | 5 | empty | 0 | 0 / 1 |
| `level-menu.png` | 7 | `4B26B0` | 2 | 1 / 1 |
| after Quit, read only | **9, expected8** | `4B3EB8` | not sampled | not sampled |

All captured backend bytes `58CDAD` were0, directly confirming the software
backend in this process. Holding shim button1 selected cursor10 at320,240;
release restored1 before the first-node click. This is native shim-input/state
evidence, not a physical SDL/DirectInput binding test. The fresh map's102 flags
were originally reported zero, but101 addresses were wrong (correction below);
separately read states were3 for nodes0–2 and1 for the remaining99.

Map selection reached `Compiled\MM_CLIMB_002.lvl`. ESC opened UI2 and counter7;
the subsequent strict-content click `(320,170)` returned a Map vtable. The
predicted8 was falsified by a single read of9; later return checks were not run.
The level images have no matched tick/camera sample and cannot score level
rendering or simulation. Packet cursor XY is **frontend global XY**: it remains
206,204 during level entry even though the gameplay cursor is drawn centrally;
the menu restores the frontend cursor there. Do not treat it as gameplay picking
coordinates or claim clone pointer-handoff equivalence from this packet.

## Cursor/frame comparison

The same production composer is used by `FrontendRenderDump`; the explicitly
named `map-held-fixture` supplies the sampled held state at320,240. It is a
composition fixture, not a replacement for the actual SDL held-input tests.
Full640×480 normalized absolute RGB difference, unchanged threshold0.02 and
no exclusions:

| Frame | Error |
| --- | ---: |
| Menu | 0.004946576286764706 |
| Confirmation | 0.0022311495778867103 |
| Fresh map, released | 0.007830729166666666 |
| Fresh map, right held | 0.00783883527369281 |

The native images visibly confirm normal versus smaller pressed crosshair.
Passing averages do not establish exact cursor RGB: the [five-bit conversion
residual](frontend-cursor-return-coverage.md#static-recovery-and-retained-image-finding)
is unresolved and no screenshot-specific color correction was added. Return-map
and second-unlock frames remain missing. Raw source/difference PNGs and JSONs
are retained separately from the native images.

## Why Quit constructed two objects — READ after the failed prediction

The extra object is the **frontend root/controller**, not another Map, a Quit
confirmation or a row widget. `486F60` constructs root `48BC00`, stores it at
`5D59EC`, then calls selected-page factory `48726F`. The page list captured by
the worker is a different global, `5D59E4`. Both root and page invoke base
constructor `4882C0`: `488359..48836B` assigns the old `4C416C` to object+2C and
increments the counter. The file initializer is1, not0. Destruction does not
decrement the counter in the read normal chains; encoded-address searches do
not exclude arbitrary indirect writes.

| Actual lifecycle | Constructor effects / settled counter |
| --- | --- |
| Level5 → menu | root(serial5), menu page(serial6) → **7** |
| Menu7 → Resume/cancel | destroy page/root, no construction → **7**, playing |
| Menu7 → O-close | destroy page/root, no construction → **7**, paused |
| Menu7 → successful Retry | destroy/reload, no construction → **7** |
| Menu7 → Quit | destroy page/root; new root(serial7), Map(serial8) → **9** |

Serials and unexercised dismissal/retry rows are READ/DERIVED, not new live
samples. Playing/paused openings call full init at `45D432/45D4C4`. Quit tears
down at `45D661`, then `45D6AF` calls full init with selector23. The missing
root explains both the observed9 and why adding one per visible page is wrong.
Page-only transitions within an existing frontend still count one, not two.

A bounded same-mechanism follow-up finds win/loss results creation at
`45DF25/45E0A9 →486F60` (root+page); loss Quit tears down at `45D79E` then
`45D7EC →486F60` (new root+Map); either Continue uses virtual `4886E4 →48726F`
(page only, root retained). Loss Retry tears down at `45D6F8`; level load tears
down at `45D8D6`; neither adds a frontend construction. These results routes
were **not** exercised by this live attempt.

Detailed READ evidence, REA IDs, raw assembly and failed preliminary checker
assertions are in `build/tmp/enacter-menu-construction/`. The main agent reran
both checkers and independently disassembled full-init, base-counter and Quit
calls. Durable reproduction:
`python tools/harness/read_frontend_construction.py` (raw-byte/direct-call anchors;
no game execution). This explanation does not turn the stopped capture into a
successful complete return comparison.

## Connected implementation and offline regression

The correction models root ownership, not a counter offset: `shell::Frontend`
starts the native serial at1, full initialization counts root+page, page
replacement preserves the root, and teardown never decrements the allocator.
Production menu open/Resume/O-close/retry/abandon now notify the persistent
`FrontendSession`; results creation/Continue/Retry/Quit use the same lifecycle.
`LevelFrameView::frontendConstructionCounter` exposes that connected state for
tests (absent without a persistent frontend), rather than a test-only counter.

The SDL-dummy application regression follows the observed prefix Menu3 →
Confirm4 → Map5 → Level5 → Dialog7 → Map9. Its footer→Menu10 suffix is a
READ-derived expectation, **not observed** in the stopped native attempt.
Longer menu tests cover repeated open/dismiss/retry and paused-origin routes,
including Map17/Map23; results tests distinguish root replacement from page-only
Continue and retain progress checks. These are source regressions, not new native
event timing, focus, returned-map pixels or unlock-state acceptance.

Independent mutations were built and tested one at a time:

| Mutation | Expected rejection observed |
| --- | --- |
| Omit root's counted base construction | `frontend_sequence`:119 checks,28 failures |
| Omit production menu-open notification | `levelmenuwindow`:menu mismatch phase0/frame1 |
| Supply released buttons to `map-held-fixture` | `frontend_menu`:held fixture did not change normal map |

All four files in `pre-mutation.sha256` were restored byte-exactly afterward.
Failure logs and identical failed-fixture images remain separately under
`build/tmp/enacter-menu-construction/`. The focused correct-source run before
mutations passed6/6 (11.72s); final full-suite verification is recorded below.

Final restored-source Windows UCRT64 build succeeded and configured CTest passed
**70/70,224.55s**, with the retail asset directory exported. Fresh production
dumps of menu, confirmation, normal map and held map are byte-identical to the
four retained scored source PNGs above; no reference images were overwritten.
The independent cursor-composition checker, native-packet verifier and both
durable static byte checkers passed again. Documentation/LF and `git diff --check`
checks passed. Logs and fresh images are in the construction packet as
`final-full-suite.log`, `final-frame-check.log`, `final-frames/` and
`final-*-check.log` / `final-independent-composition.log`.

No new native process was launched for this correction. Returned-map frame/UI/
flags, footer activation and the second unlock state still need observation
under fresh exclusive-desktop authorization. Exact cursor colors, pointer handoff
and in-level-menu held-cursor integration remain open; no B2/B3/C2 closure is
claimed. Changes remain uncommitted; pre-existing user edits were preserved.

Reproduction on Windows (retail installation required; no original-game launch):

```powershell
.\scripts\Invoke-Ucrt64.ps1 "export WEBOLUTION_GAME_DIR='C:/progs/Lemmings Revolution'; cmake --build --preset dev-debug && ctest --preset dev-debug --output-on-failure"
python tools/harness/read_frontend_construction.py
python tools/harness/read_frontend_cursor.py
python build/tmp/enacter-native-return-20261028/verify_packet.py
```

The packet verifier checks the retained capture/restoration receipts; it does not
claim the installation is presently unused or authorize another launch.

## Fresh follow-up stopped on cursor precondition

After proposing the corrected3→4→5→5→7→9→10 route and missing return-map/UI/flags/
footer checks, the user said **"go ahead"**. The main agent announced one bounded
capture, up to15minutes exclusive desktop use, temporary windowing and exact
save/config restoration. First clock17:40:24Z; fixed expiry17:55:00Z.
Packet: `build/tmp/enacter-native-return-20261028-1740/`; the earlier failed
counter packet was not edited. The reviewer caught a pre-existing safety gap:
the supervisor tested worker termination after restoration. The fresh copy now
blocks all installed-file restoration unless worker death is verified first,
then also requires no stock-like process. Owned-Python-child tests passed.

The single-use claim was written17:43:24Z. Owned stock PID43448 reached Menu3 →
Confirm4 → Map5, each with stable UI3, frontend flag1, cursor1 and backend0.
All102 fresh completion flags were zero; states were3 for nodes0–2 and1 for the
other99. The cursor was **317,241**, stable before/after all three screenshots,
not the predeclared320,240. The worker's fresh-map assertion failed and stopped
before holding the cursor, clicking a node or exercising any return route.

This was a **harness setup assumption**, not evidence that the game's return
route failed. The static320,240 initializer and previous centered captures do
not establish a centered pointer after process startup/input activation. The
source did not explicitly aim to center before those captures. What caused the
three-pixel/one-pixel offset was not sampled and is unknown; do not infer user
interference, input-device behavior or a different native initializer.

Three unscaled640×480 images and their state brackets are retained. Main-agent
visual inspection confirms game pixels, English labels and unobstructed client
geometry. They were **not scored** against the centered source fixtures: the
predeclared coordinate condition failed. The original full-route verifier is
retained and rejects `ok:false` (`verification-failed-route.log`). A separate
post-failure `verify_failed_packet.py` verifies only evidence integrity, observed
317,241, fresh flags/states, missing later captures and restoration receipts; it
does not relabel the route as passing. The previous packet verifier still passes
its own expected-failure integrity checks.

Input restoration and stock-process termination were verified; worker PID49808
exited naturally. Independent read-only checks at **17:43:55Z** confirmed exact
save/config/exe/shim hashes and no stock-like process. The user was told the
desktop was free. No retry was made; this authorization is spent. Production
code was not changed, and its four pre-mutation hashes still match; the previous
70/70 suite remains the latest full run, not a rerun for this capture.

Smallest next remedy, **only under fresh authorization**: observe and retain the
startup pointer, then explicitly aim and verify320,240 before the first scored
frontend image using the existing bounded shim helper. Keep the executed packets
immutable, keep the corrected route prediction and unchanged full-frame gate,
and do not fit or align the captured image afterward. Native returned-map/UI/
flags/footer and second-unlock-state observations are still missing.

## Centered follow-up: flag-dword comparison failed

The user's next **"go ahead"** authorized one centered-pointer attempt. Packet
`build/tmp/enacter-native-return-20261028-1803/` retained the prior scripts and
added explicit bounded centering/settling plus exact frontend capture coordinates.
The first clock was18:03:00Z; expiry18:17:50Z; claim18:05:41Z. The independent
safety review approved that delta before execution. Startup and post-centering
observations both read320,240 this time (not evidence that centering is unnecessary).

Owned stock PID7264 followed Menu3 → Confirm4 → Map5 → Level5 → menu7. Six
screenshots bracketed their declared cursor/UI/page state and geometry; English
labels and unobstructed640×480 pixels were visually inspected. Right-held cursor10
and released1/backend0 were observed again. After Quit, the script passed the
Map9 and UI3 checks but failed `return_flags != flags`: the102 dwords sampled at
`4E36E8 + 7000*g + 280*s` differed from the all-zero fresh vector.

**Evidence limitation:** the script asserted before persisting the returned
values, records or image. We know the comparison failed, not which values changed
or why. Do not infer a completion write, unlock change or exact returned states
from this error. Footer activation was not attempted. The expected-success
verifier remains failing; `verify_failed_packet.py` separately checks only the
six captured snapshots, source hashes and restoration integrity. The earlier
failed packets remain unchanged.

Input restoration and stock termination were verified, followed by supervisor
restoration. Independent read-only checks at18:06:16Z found no stock-like process
and exact save/config/exe/shim hashes. The user was told the desktop was free.
This launch was spent; the subsequent **"you can retry it"** is a separate grant,
not retroactive acceptance of the failed flag prediction. Offline native-field
research and pre-assertion diagnostics are required before that retry.

## Completion-reader radix correction

Offline stock-byte research proved a **reader bug**, not native completion
mutation. `49B6E7:69c0581b0000` multiplies group by0x1B58=7000 decimal;
`49B6F9:69d218010000` multiplies slot by0x118=280 decimal;
`49B6FF:83bc10e8364e0000` tests the indexed DWORD at4E36E8. The old sampler
used0x7000/0x280, giving101 wrong addresses out of102. Only node0 matched.
For node1, native address is4E3800, not4E3968. `485E55` stores DWORD1 on win;
no inspected Quit path awards completion or restores a saved file.

This invalidates the purported102-flag observations in the initial entry packet,
the16:39 return attempt and the17:43/18:05 follow-ups. Node0 and the separately
sampled map records/states remain valid; images/counters are unaffected. The
failed18:05 words were never persisted, so the changed unrelated addresses and
their writers remain unknown. Do not retrospectively manufacture those values.
The same radix trap was already documented in B2m; it should not have recurred.

Evidence and32-anchor checker: `build/tmp/enacter-return-flags/`; the main agent
reran the checker and checked the operand bytes. New capture helper tests derive
strides from encoded instruction immediates and compare all102 addresses; a
node1-only completion fixture rejects the old hex-stride mutation. No production
gameplay change is justified. The source receives a flattened flag array and
does not use the faulty native-memory formula.

## Corrected-reader retry: ambiguous menu scan

User **"you can retry it"** authorized another attempt; first clock18:07:45Z,
expiry18:22:30Z. Packet `build/tmp/enacter-native-return-20261028-1807/` uses the
corrected central address helper and saves entry/menu/return diagnostics before
equality assertions. Offline helper/mutation tests and independent review passed.

Claim18:16:22Z; owned stock PID43704. Before any screenshot or map navigation,
the three-DOWN differential highlight scan retained **two** candidate addresses,
40868892 and230656168. It refused to guess and stopped. There is **no screenshot,
corrected live flag vector, level entry or return observation from this attempt**.
This is a harness ambiguity, not a new frontend behavior failure. The next probe
must use a byte-supported structural highlight reader, not select the first hit.

Input/process cleanup verified; independent18:16:52Z checks confirmed no stock
process and exact save/config/exe/shim hashes. The user was told the desktop was
free. A subsequent explicit grant to continue until a full capture is separate
authority; it does not turn this failed attempt into accepted evidence.
