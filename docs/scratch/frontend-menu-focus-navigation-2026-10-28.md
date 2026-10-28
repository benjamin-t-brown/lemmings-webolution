# Main-menu focus navigation and Continue — offline production update

## Evidence and boundary

The retained native three-DOWN differential in
[frontend-native-comparison](frontend-native-comparison-2026-10-28.md#observations)
confirmed focus changed across three DOWN inputs, but did not document the values
or provide a key-to-event byte chain. B2b's retained static read records only
that key poll `0x483212` marks any newly pressed key and shared handler events
2–5 update focus through `0x48F7A1`; it did not establish physical key identity.

Newly supplied native evidence supersedes the prior row1 RETURN expectation:
after the structurally bracketed menu focus changed row0→row1, RETURN changed
the page list from main-menu vtable `0x4B2528`, counter3, UI3 to Map vtable
`0x4B3EB8`, counter4, UI3. Packet:
`build/tmp/enacter-native-return-negative-20261028/`
(`menu-row1-before.json`, `input-02.json`, `worker-result.json`). This observed
transition falsifies the earlier prediction that nonzero RETURN is ignored.
The worker stopped on an unexpected counter expectation; that does not erase
the measured Menu3→Map4 transition, but the attempt did not complete its planned
route.
The observed English/UK row1 label is **Play Current Game** (`UK126`).

Offline REA reads of the stock PE
(`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`) checked
`0x48896A` (Evidence `ev_6b92378d9ae56940c278166e5ff2e33589db4d8220184c27c0642c9d26398e69`)
and `0x488D10` (Evidence
`ev_a4218f0c1e55afa30b824270e58cd245240bb9f2e1967cf49573cde75aeb12eb`).
`0x48896A` dispatches events2–5 through helpers `0x490226`, `0x49023B`,
`0x490250`, `0x490265`, then applies a non-null neighbor via `0x48F7A1`.
`0x488D10` checks `0x48382C` and emits event7. Neither read identifies which
physical key generates each directional event. REA was closed after the read;
the stock executable was not launched by this task.

Focus rendering has retained code support: B1y READ at `0x489ADC` chooses the
highlighted/normal fill colors using a row flag; `shell::renderMainMenu`
represents this as `MenuParams::highlight`. The earlier scored native image was
highlight0 only. The later row1 diagnostic and its predeclared scope are recorded
[with the new observation](frontend-return-negative-prediction-2026-10-28.md#offline-application-comparison-declared-before-scoring).

Physical UP/DOWN binding, direction symmetry, endpoint wrapping, and key-repeat
behavior remain unproved. The SDL UP/DOWN mapping and five-row cyclic behavior
are explicit bounded adapter choices, not native findings. The observed row1
RETURN route is implemented as Continue→Map with progress preserved. Rows2–4
remain unsupported. Clicking the existing New Game target resets focus to row0
and retains its earlier behavior. No other menu actions, mouse-hover focus,
mouse row selection, or repeat navigation were added.

The SDL-dummy regression includes a fresh startup route matching the observed
Menu3→DOWN row1→RETURN Map4 sequence. It exports a pointer-centered composed
frame to `WEBOLUTION_TEST_OUTPUT_DIR/main-menu-row1-observed.png`, without a
synthetic outcome, model focus setter or substituted render fixture.

A separate progress-retention setup **injects a LevelOutcome through the session
API**, not a natural simulation win. That route moves to row4, wraps both
directions, checks displayed selection, then exercises row1→Map with the earned
flag unchanged. It checks arrows remain inert on Map, footer Return builds a
row0 menu, repeat events are suppressed, row0 Return opens confirmation, and No
returns the row0 menu without clearing progress. These are source regressions,
not additional native observations.

Review found a duplicate-focus bug in the first implementation: the shell reset
focus on map→menu, but the session retained row1 and its image. The added SDL
round-trip test failed against that version (`focus-return-before-fix.log`).
The model is now the sole focus owner; the menu image cache checks its highlight
when rendered, so page replacement cannot leave action and visual focus apart.
The session test also checks New Game clicks from row1 still select New Game,
invalid directions are rejected, and unsupported rows2–4 return Unobserved.

Independent isolated mutations of row1 RETURN, SDL directional delivery and
New Game click focus reset each fail their targeted test. All three source files
were restored byte-exactly afterward. Logs are in the new native observation
packet; full build/test results are recorded with that observation. The full-frame
image comparison remains diagnostic, not new B1/B2 acceptance. No screenshot-
specific rendering constants were added.
