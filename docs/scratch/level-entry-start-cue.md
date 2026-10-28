# Level-entry “Let's Go!” cue (2026-10-28)

Offline READ/DERIVED implementation, not native pixel or timing acceptance.
Packet: `build/tmp/enacter-level-title/`; main reran `verify.py` and
`assets_check.py`. Standalone `tools/harness/read_entry_start_cue.py` retains27
raw anchors and8 doubles against the pinned stock SHA256 without scratch imports.
No original process/desktop, registry or installed-save access was used.

## What the native code actually asks for

Loader45CAEA pushes **hex47**, i.e. localized string**71**, `Let's Go!` in UK.
It is not decimal47 (`Continue`) or the selected level's name.4151D0 returns the
font pointer;4404FA builds a proportional glyph run using ^Font/4BF968. The
rectangle is glyph stretch geometry, never a filled panel.

The title clock is independent of the camera opening move. Loader45C9AB seeds
**3.5** for map entry or **1.5** without intro (including loss Retry).45CC09
subtracts frame time only in playing UI0, **before simulation/drawing**. Pause
and in-level menus freeze it without hiding it; pending completion does not
suppress it. Only loaded gameplay mode2 draws; normal results teardown retires
the cue. It expires strictly **below -1.0**, remaining active at exactly -1.

For post-update clock `t <= 1.5`, the base rectangle is(160,180,480,260). If
`t > .5`, set `q=(t-.5)^2`, dx=trunc(3040*q), dy=trunc(760*q), then expand both
sides by dx/dy. At1.5 it is(-2880,-580,3520,1020); at1 it is(-600,-10,1240,450).
Each glyph is stretched vertically to the whole rectangle; cumulative
proportional horizontal edges are truncated individually. It is not ordinary
29px text centered on that rectangle.

At nonnegative `t`, mode1 requests SRCALPHA/INVSRCALPHA. Negative `t` requests
mode3 SRCALPHA/ONE and vertex alpha `255+t*255`. **That is not final opacity.**
The normal caret-font constructor supplies flags8; binder47261F then requests
alpha SELECTARG1(TEXTURE), ignoring vertex alpha. Consequently this supported
route uses texture-alpha source-over followed by texture-alpha additive
blending, not a guessed opacity fade. These are static requests conditional on
the normal font/binder path and successful device calls, not observed effective
device pixels.490ED6 remains an empty helper, not the text renderer.

Native pooled glyph allocations can retain old alpha across reloads, because
the nonnegative phase does not reset it. SOURCE creates a fresh cue per level
leg; its nonnegative alpha metadata describes fresh allocation255, not native
pool contents. This does not change supported texture-alpha RGB output.

## Application integration and limits

`EntryStartCue` owns the clock/geometry. `drawEntryStartCue` fits the accepted
ASCII font cells, clips before looping over huge rectangles, preserves texture
alpha and uses source-over or saturating additive RGB composition. It preserves
the destination alpha plane like existing HUD text.

Natural `--frontend` launch loads UK71 and creates a cue using the session's
intro preference. It is **not restricted to the first level**, unlike the
currently bounded camera-intro implementation. Direct-level/headless diagnostics
are unchanged and do not acquire a cue merely by constructing World. Input is
processed before clock advancement; focus loss and pause freeze the cue.
Compositing is scene/skill panel/HUD → cue → cursor → pause caption. The combined
HUD helper remains backward compatible; main explicitly separates its pause draw.

Sampling is currently the SOURCE software POINT convention. Native47AD8C requests
linear only when renderer+3A1 also enables it; effective sampler/cache/device
state, stretched raster edges, filtering and rounding remain unaccepted. This
implementation does not claim a native no-fade visual measurement. General
in-level menu integration, non-ASCII font ranges, movie transitions and full
event-sliced simulation timing remain separate gaps.

## Tests

- `entrystartcue`: literal map/Retry clocks, update-before-sample, frozen but
  drawable cue, level-scene gate, exact1.5/.5/0/-1 boundaries, squared geometry,
  cumulative glyph fitting, clipped huge geometry, source-over/additive texture
  alpha independent of vertex alpha, transparency and safe no-op inputs.
- `entrycuewindow`: actual SDL dummy app, half-second frame clock; observes
  shrink/base/additive/retired phases, pauses/resumes while the cue is visible,
  compares opaque retail-font samples against independent composition, and
  confirms the retired cue no longer covers the scene. Shared existing cursor
  and pause primitives form the later-layer oracle, not a native screenshot.
- `levelreturn`: actual loss Retry also checks its distinct1.5 cue seed via the
  first frame's post-update clock(-.5 with the bounded2s test clock).

Preparation corrections: missing GameData include was caught by the first
build. The first test mask demanded opaque3x3/2x2 neighborhoods that the thin
retail glyphs did not supply in the visible crop; it now explicitly checks the
SOURCE point-sampled opaque texels instead, without changing renderer behavior
or claiming sampling independence. The injected pause key initially omitted
its scancode; corrected to match the application's SDL adapter.

Targeted tests passed (`entrystartcue`0.03s, `entrycuewindow`3.52s). Six isolated
mutations failed: omit the actual app draw; advance while paused; multiply by
vertex alpha (invented fade); expire at exactly-1; linear instead of squared
geometry; sample the old clock like camera intro. The app detects failures at
frames4/5/9/3 respectively for its four mutations. Production files were restored
against the packet's `pre-mutation.sha256` before the full regression build.

Restored full build/CTest: **68/68 PASS**,211.35seconds; `entrystartcue`0.03s,
`entrycuewindow`3.38s, full win/timeout/Retry/Quit/reentry `levelreturn`105.94s.
Log: `build/tmp/enacter-level-title/full-suite.log`. These are SOURCE passes,
not native pixel/timing acceptance.
