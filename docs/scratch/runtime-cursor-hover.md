# Runtime cursor and shared hover (2026-10-28)

Offline implementation after [visibility/support](first-level-pick-visibility-support.md).
No original-game launch, attachment, desktop input, registry access or installed
save/config writes. SOURCE application tests are not native cursor acceptance.

## Delivered

Windowed `--sim` and natural frontend level entry now compose the retail
NoLemming, Lemming and Grab cursor textures over the completed scene/panel/HUD.
Hover and click use one `hoveredActor` function, returning an optional actor
index. Both use the shared visibility-aware boxes, bounded skill exclusions,
first-level support and overlap priorities/distances already established.
Stock zero rejects both; synthetic unlimited stock remains available.
Re-evaluation after successful assignment removes the newly ineligible Climber
target, allowing the next overlapping eligible actor to be selected.

Panel hover and clicks share `runtimeSkillPanelIndex` (the existing SOURCE
plate rectangles). Panel hits suppress actor hover; available skill plates
show Lemming, unavailable plates NoLemming. The two unimplemented command
plates remain consumed and show NoLemming. Right-drag selects Grab before panel
override, suppresses actor hover and assignment, and freezes the drawn pointer.
Focus loss clears drag and hides the software cursor. The app hides SDL's OS
cursor while this runtime renderer is active and restores its previous setting
on exit. Other window/headless rendering defaults remain unchanged.

`CursorPolicy` now lives in `webo::app`, matching the other production modules;
the recovered ordered selection function and resource table are unchanged.
`LevelFrameView` exposes the composed cursor state/position and hover index for
read-only tests. It does not mutate the World or inspect a native process.

## Explicit SOURCE input and configuration boundary

Existing camera controls integrate relative motion into a separate virtual
cursor; existing clicks used absolute SDL window coordinates. Mixing those two
would put hover and click in different places. This implementation deliberately
uses **absolute SDL motion/button coordinates for both runtime hover and click**,
starting at (320,240) until the first motion/click, with right-drag freezing it.
The camera controller's accepted relative arithmetic (including its 2.0/2.4
cursor scales) is untouched. No physical SDL/DirectInput equivalence is claimed.
Startup without an SDL pointer event can leave the drawn center different from
the actual OS pointer until that event; no desktop warp was added.

`runtimeCursorState` explicitly chooses the software mode-2, display-mode-0
subset and maps our paused flag to UI0/1. It does **not** read the native
registry or claim mode2 is every install's default. Direction filters and
object/entrance/exit hit producers are not supplied. MouseDown/pressed-state
lifecycle, alternate OS-cursor rendering, other modes and native pointer/drawn
center disagreement remain outside this adapter. Existing incomplete general
skill applicability means the hover is not a promise of native assignability
for every skill/state; the tested first-level Climber path is the bounded case.

## New static evidence

Full packet: `build/tmp/enacter-runtime-cursor/findings.md`, aligned raw reads,
`rea-evidence.json`, `checker.json` and `check_cursor.py`. The authority SHA256
is `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

- **455CB0 is always true.** 455DD0 ->455DF0 tests mouse object +4==1;
  483C58..483C70 produces that held bit from device-state byte13. Installed
  `dinput.h` DIMOUSESTATE identifies this as DirectInput button1, not a press
  edge. This grab predicate does not use the swap-dependent path.
- Registry `CursorMode` ->G+55AC ->screen+8 is the real mode producer
  (45C1F0..45C23E,45C8B2..45C8C7). **2 is the failed-read fallback only**.
  Effective-mode45328C returns2 for a nonzero backend/window byte58CDAD,
  otherwise screen+8. No current registry value was measured.
- 58CDAD is renderer/window +2D. Zero selects software/fullscreen; nonzero
  selects OS cursor/windowed. Read zero-producing startup paths are45BF78 and
  450732; they are not a new live observation of a running process.
- 455774 supplies32x32 source rectangles;4577CD and4579D0/457A30 place the
  quad at integer center minus16. Ordinary native movement truncates cursor
  doubles; rebinding456042 preserves the previous sprite's integer center.
- Three loaded resources are complete32x32 BMPs, not sprite strips. NoLem
  has240 opaque pixels and Lem328, both black-key masks. Grab has399 zero,
  155 opaque and470 partial-alpha pixels; its RGB is white and the1024-byte
  ALF supplies the shape. Keep its four transparent top rows, not a cropped
  silhouette expanded to32.
- Grab's mode1 routes through414D80: alpha blend enabled, SRCALPHA /
  INVSRCALPHA (DERIVED using installed `d3dtypes.h`). NoLem/Lem use mode0
  and binary masks. `RuntimeCursor` reuses `compositeIconRegion` for the SOURCE
  straight-alpha integer blend. Hardware sampling, texture-coordinate biases,
  alpha-test/conversion and final native pixel quantization remain unverified.

Main independently reran the packet checker: native byte anchors and three
32x32 RGB/alpha comparisons pass. Independent Pillow BMP decoding and explicit
black-key/ALF alpha reconstruction agree with the existing decoder's PNGs.
These are file/asset checks, not screenshots from the native game.
The dependency-free retained checker `python tools/harness/read_runtime_cursor.py`
also passes22 stock-byte anchors without requiring the scratch packet or PIL.

## Tests and sensitivity

- `levelcursor`: ten actual app frames under SDL dummy. Motion with zero
  relative delta proves the absolute-pointer path; paused actor hover agrees
  with assignment, grabbed click on an eligible actor is rejected, release
  restores hover, assignment spends stock/resumes, duplicate eligibility clears,
  available/unavailable panel feedback works, drag freezes the pointer and focus
  loss hides it. Every opaque Lem/NoLem artwork texel is checked against the
  independently loaded resource, not merely an arbitrary changed scene patch.
- `leveloverlap`: hover identifies the non-first closest eligible actor before
  the click; same-cursor retry then assigns the next eligible overlap.
- `levelvisibility`: hidden actor click also produces no eligible hover.
- `runtimecursor`: independently computed pixels for all three resources over
  a known background; clipping at both edges, fully offscreen, Hidden no-op,
  represented partial alpha, and SOURCE policy inputs.
- `runtimewindow`: full-frame runtime/fixture equality at ticks0/1 now includes
  the explicitly requested headless `--cursor-fixture`. This new diagnostic draws
  centered NoLemming only; default headless fixtures remain unchanged. It is a
  wiring check using shared drawing primitives, not an independent native oracle.
  `frontendentry` likewise requests the cursor in its headless comparison frame;
  the first full suite caught this omitted expected overlay (62/63 passed),
  rather than any native-frame score change.

Isolated mutations each failed the intended test: omit application cursor
drawing; use the controller's virtual coordinates for hover while clicks stay
absolute; permit assignment during grab; use the cursor center as its top-left.
Production files were hash-restored using the packet's `pre-mutation.sha256`
before final full-suite verification. No golden image or native threshold was
changed to make these tests pass.

Final restored build/CTest: **63/63 PASS**,101.67seconds; `levelcursor`2.68s,
`runtimecursor`0.02s, `frontendentry`17.81s. Log:
`build/tmp/enacter-runtime-cursor/full-suite-restored.log`. This closes the
bounded SOURCE integration checks, not native C2 rendering acceptance.

Next connected-play deliverable is rescue -> results -> map with retained
in-memory progress. Native cursor/exit-marker acceptance, complete eligibility,
general terrain support, and parked simulation timing remain distinct gaps.
