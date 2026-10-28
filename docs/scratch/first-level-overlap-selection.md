# First-level overlap selection (offline continuation)

The all-zero-key production caller was a real gameplay defect. Retained D2b
already recorded native actor6 versus source actor2; retained D2d gives a
fully decoded same-priority case with native actor3 versus first-listed actor2.
This continuation does not start or attach to the original, use desktop input,
or write the installed save/config. The spent capture window is not reused.

## Controller target and raw distance

The object identity is closed, not inferred from matching offsets:
45C7E2 constructs452BC2 and stores its result at global4C6A08+55A8
(4CBFB0). 45DAB9 passes this to4532E2, whose4537EC calls454D6C;
the hover's4551B1/4551CE use this same object's+58. Startup44507E
calls the+58 setter452FD3 on4CBFB0. `screen+58` is therefore the
controller **target** vector, not the physical eye or current pose.

Ordinary startup seeds `(hatchBaseYaw,trunc(levelRows/2),0.5)`—first level
`(32,48,0.5)`. The controller now retains this target alongside its existing
camera parameters. Input moves target yaw and height using the recovered
normal-mode2 gains. Height clamps to `[0,rows]` on each update, preserving
saturation/reversal history. Target yaw retains exact256 (native wraps only
when `>256`), even though the rendered periodic yaw can be0. Flip wraps occur
before the subsequent grab delta. Zoom's eye-elevation variable is separate.
The opening camera animation does not alter this target.

The application snapshots target coordinates beside the displayed camera and
uses the injected actors' **pre-model** `plus8X/Y/Z` samples. It does not
reconstruct target height from the displayed camera, project the target into
pixels, or substitute the actor's cylindrical model position.

Scope remains ordinary source controls: native mode1 seam/current-target
constraints and replay/restoration are not implemented. Replay4568DC can
write target Z=0; thus startup Z=.5 is not a universal invariant. Simultaneous
zoom/input ordering and the broader physical-input equivalence remain open.
Other-level hatch yaw metadata is still the previously documented limitation.

Full byte/dataflow packet: `build/tmp/enacter-pick-reference/README.md`;
`check_evidence.py` independently passed26 raw-backed authority ranges plus
the retained D2d raw-hex replay. Its line216's default-Z=.5 description is
incorrect for the current source header, whose default is.75; production
consumes each actual injected sample rather than that prose value.

## Supported rules and recovered state inputs

`Pick` keeps the native ordering: highest priority, then strictly smaller raw
three-component squared distance; exact ties retain list order. It is not
distance to the cursor in pixels, not model-space distance on the cylinder,
and raw X subtraction is not wrapped across the cylinder seam.

`test_pick` replays the D2d raw vector operands and rectangles. Reference
`(32,48,0.5)`, actor2 `(60.53015899658203,32,0.25)`, actor3
`(53.0374641418457,32,0.75)` produce distances `1070.0324723702506`
and `698.6373975194438`. The cursor box `(432,274,464,306)` overlaps both;
the recovered comparator chooses actor3, matching the retained native hover.
This is a retained-observation regression, not a fresh native run.

Fresh constructor reads close four state-category mappings:

| Simulation state | Native vtable | Category store | Category | Priority |
| --- | --- | --- | --- | --- |
| Walker | 4B0D28 | 41BD7B | 18 | 0 |
| Faller | 4B0A20 | 415FCC | 19 | 2 |
| Climber | 4B0960 | 41376B | 0 | 2 |
| Hoister | 4B0978 | 413C8B | 0 | 2 |

`RuntimePick` returns an explicitly unknown priority for unmapped states; the
runtime retains a labelled SOURCE zero fallback for those states. The first20
table dwords at4C05D0 are verified; that is **not** a complete state map.

## Necessary Climber exclusions, not complete applicability

45A0E0 uses42ECC9 to check categories in both the current state and the
parallel state list. Climber's arm calls40C061, which excludes categories
14,3,0 and calls4183D3; that helper calls42E913 (terrain support predicate)
and excludes19,33,30. Already-assigned Climber capability is therefore
filtered **before** choosing an overlap winner, rather than winning the pick
and only then failing `World::assign`. Faller, Climber, Hoister and Bounce
are also rejected by the recovered necessary category tests.

The implementation does not pretend `World::assign` is native can-apply.
It does not yet reproduce the full terrain-support predicate, pending-state
representation, or applicability of the other seven skills. In particular
42E913 uses actor+360/+368 coordinate offsets, facing-adjusted X, gravity,
and terrain-cell flags;42E8C7 distinguishes exact fluid-zone words. Those
inputs are not replaced with a guessed `terrain.solid()` test.

**Correction to old terminology:** actor+29 is not just hatch emergence.
43A9A4/43AA30 in the tube pass write it from the bucket-visibility test and
actor+28 (REA evidence `ev_19290ac3cfc467cc18c6015f382bfda87fb84cc59289af15a0f01b7b4ba4854c`).
Actor+370 is set at spawn42E06F and cleared by exit426409; deserialization
42FBB9 also writes it. Neither flag is fully represented by a projected box
or `alive()`. The runtime still lacks these complete gates and facing filters.

## Reproducible evidence

`python tools/harness/read_runtime_pick.py` pins the stock SHA256, checks
raw-backed section extents, verifies constructor/table and flag-writer bytes,
and prints the bounded native helper/hover instructions. It is read-only and
does not execute the target. Further retained scratch is under
`build/tmp/enacter-pick-eligibility/` and `build/tmp/enacter-pick-integration/`.

The source actor adapter still uses integer simulation positions and its
existing Z slab. Exact native continuous pose and native visibility/hover
timing are not established by this work. Full D2/native overlap acceptance
remains open even when the application regression passes.

## Production regression and mutation checks

`leveloverlap` runs the actual app under SDL's dummy driver with retail
`MM_CLIMB_002`, prepared at100 diagnostic simulation ticks. It finds an
overlapping group of unassigned Walkers where the raw-distance winner is not
the first actor, clicks it, and checks that exactly that actor gains Climber,
stock drops and pause resumes. After pausing again, the same cursor must
assign the next eligible actor rather than fail on the already-assigned one.
The test does not mutate the World or call `World::assign` itself. No natural
spawn-timing or new native-comparison claim follows from this seed.

The old production caller failed this test. Separate mutations also fail:
all-zero candidate distance (actual application); omit already-Climber
filter (actual application); all-zero priority (runtime candidate test);
omit target-height clamp and wrap target256 to0 (camera-input tests).
Removing the known Climber state exclusions also fails the runtime test.
The retained-native-vector test also fails when its distance helper returns0.
The earlier single-actor assignment/pause/count test remains in place.

Final restored build/CTest: **59/59 PASS**,98.96seconds. Verbose app replay
selects actor5 at cursor`(416,270)` instead of first-listed actor3, then
assigns actor3 on the same-cursor retry; isolated replay1.83seconds.
Logs: `build/tmp/enacter-pick-integration/final-suite.log`; final production
hashes: `final-source.sha256` in that packet. These are SOURCE integration
results, distinct from the retained native D2d actor2/actor3 observation.
