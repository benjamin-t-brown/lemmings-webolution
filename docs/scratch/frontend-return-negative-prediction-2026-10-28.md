# Next bounded frontend observation — prediction, not a result

Written before any new native launch. No desktop access is claimed by this note.

## Question and predeclared failure

Does main-menu RETURN leave the page unchanged when structural focus is row1,
while the same key at row0 opens New Game confirmation?

Reuse the retained stock process envelope and structural reader from
`build/tmp/enacter-native-second-unlock-20261028/`; do not reuse its expired
authorization or overwrite its artifacts. Observe the page/manager/focused-row
chain, not a numeric heap-search candidate. Reference: [retained route](frontend-native-return-complete-2026-10-28.md).

Predicted sequence, starting at the ordinary startup menu:

1. Menu counter3, vtable `4B2528`, UI3, structural highlight0.
2. Balanced DOWN changes highlight0→1, without replacing the page or counter.
3. Balanced RETURN at row1 leaves counter3, page identity/vtable, UI3 and
   highlight1 unchanged. Record raw structural reads before and after input,
   shim-consumption evidence and a settled frame; absence of a visible change
   alone is insufficient.
4. Balanced UP returns highlight1→0. RETURN then reaches confirmation counter4,
   vtable `4B3F38`, UI3. This positive control distinguishes a rejected menu
   action from input that was never delivered.

A different row, changed page/counter/UI on step3, or failure of the step4
positive control falsifies the predicted correspondence. Lost foreground,
ambiguous input consumption, unstable structural reads or an unexpected starting
state invalidate the observation rather than becoming a pass. Preserve all
failure artifacts. Do not send another key after an unexpected page transition.

This is a state/input observation, not a new pixel gate or timing measurement.
It covers row1 only; other nonzero rows, confirmation No, in-level Resume,
natural loss/retry/Quit, physical device equivalence and C2 stay separate.
No game-state memory writes, save reset, gameplay timing probes or browser work.

## Access and cleanup

The exact missing evidence is a fresh stock row1 RETURN negative observation
with structural focus and a row0 positive control in the same owned process.
Smallest remedy: a short exclusive desktop window for one bounded launch using
the existing hidden-worker, foreground/geometry guards and owned-process cleanup.
Back up save/config before launch; restore and independently verify exact hashes
after the owned process exits. Abort if another stock process/session is active.
Historical grants do not authorize this run.

## Result: prediction falsified, not a negative-input pass

The user subsequently granted one launch in an exclusive window. Packet:
`build/tmp/enacter-native-return-negative-20261028/`. The unchanged owned-process
envelope was reused with the bounded input sequence above and deadline19:55:10Z.

Observed: Menu3/UI3 at structural row0; DOWN retained the page/counter and
selected structural row1. Both unscaled640×480 frames were inspected: the amber
panel moves from **Start New Game** to **Play Current Game**. Row1's labelID126,
owner, five-row navigation ring and focused-row pointer were verified in stable
raw-byte brackets. Native-to-native image delta is confined to `[207,322,434,376)`
(10,291 changed pixels); this isolates the focus-panel change, not source fidelity.
Balanced RETURN then changed the page to **Map4/UI3**, vtable
`4B3EB8`, recorded in `input-02.json`. The following expected-menu read rejected
counter4 and stopped the worker. No UP, positive-control RETURN, gameplay input,
or later native launch occurred. There is no returned-map screenshot or progress
vector from this run, so neither pixels nor save/load semantics are established.

This directly contradicts the blanket “RETURN only at highlight0” claim for
row1. It is not a passing negative test, and does not prove rows2–4 actions.
The original prediction and failed worker result are retained. The signed B2
gate wording is not silently revised; its quirk clause requires correction.
The older [B2f implementation note](phase-b2f-frontend-state-machine.md) explicitly
classified this as policy, observing that B2e only sent RETURN at0; [B2i](phase-b2i-frontend-measured.md)
retained the guard without adding a nonzero observation. A navigation helper's
“only send RETURN at0” safety guard is not evidence that other native rows ignore
the key. The new capture distinguishes those claims.
`verify_observation.py` replays both successful raw snapshots and the expected
failed read, checks input/page records, captured-image/source hashes and cleanup.
An offline verifier pass means the contradictory evidence is internally checked,
not that the original prediction passed.

Owned stock PID32368 exited; shim input restored. Independent verification at
**19:51:32.401286Z** found no stock process and exact save/config/exe/shim hashes.
The desktop was released immediately. No restoration debt or remaining launch
authority; further observation needs fresh exclusive access.

## Offline application comparison declared before scoring

When the production navigation adapter is corrected, compare an actual SDL
application export at row1, pointer320,240, against `menu-row1-before.png`.
Use full640×480 normalized absolute RGB difference, no exclusions, registration
or resizing, diagnostic threshold0.02. This is an additional diagnostic, not a
retroactive expansion of B1's signed row0 gate. A source-only navigation test
must use SDL keys rather than setting model highlight directly.

The [production correction](frontend-menu-focus-navigation-2026-10-28.md) now
replays ordinary startup Menu3→DOWN row1→RETURN Map4 via SDL. Its actual composed
`main-menu-row1-observed.png` scores **0.004960311989379085**, below the declared
0.02 diagnostic threshold (`main-menu-row1-score.json`), with both frames
visually inspected. No renderer colors/geometry were tuned for this comparison.
This is a row1 image/state correspondence, not full keyboard, cursor or B2
acceptance. The earned-progress retention regression is a separate synthetic
session-outcome setup, not a newly observed native save/load route.

The parent review additionally caught and fixed a duplicate-focus/cache bug on
map→menu return, with a failing-before/passing-after SDL round-trip regression.
Isolated native-row1-action, SDL-direction-delivery and click-focus mutations each
failed their targeted test; all changes were restored byte-exactly. See the
implementation note for scope and the packet's mutation/build logs.

Final UCRT64 build and retail-configured CTest: **70/70 PASS, 232.59s**
(`final-full-suite.log`). The final test export is byte-identical to the scored
candidate; a copied PNG, repeated score and final source-hash manifest are in
the packet. Offline evidence replay, documentation checks and `git diff --check`
pass. No commit or push; unrelated user work preserved. Remaining No/Resume/loss
native comparisons, rows2–4 actions, physical input, persistence and full C2/C3
remain open; simulation timing parks and browser-last sequencing are unchanged.
