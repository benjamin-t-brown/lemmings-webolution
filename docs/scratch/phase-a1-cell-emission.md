# A1: one quad's device emission independently checked

This follows the [failed cell fixture](phase-a1-cell-oracle.md). The facilitator
independently followed the selected original quad to its final device call and
reproduced the agent's emitted vertex buffer byte for byte. **A1 remains open.**
No new camera, depth, clipping or raster value is promoted as pixel-accepted.

## What this establishes

The selected inline quad, source indices, material descriptor and UVs match the
retained cell capture. At the final call, its ordered emitted coordinates and UVs
agree with the candidate under the comparison bounds declared before the probe.
The captured polygon list and emitter cache reads preserve the expected corner
order in this case. This does not establish all polygons, clipped polygons,
materials or retained views.

The facilitator used a separate hardware-breakpoint controller and offline
decoder. Its checked cache-to-device float, diffuse and UV conversions match the
actual emitted bytes exactly. Specular words were captured, but the builder does
not write that field; their interpretation remains unknown. The registered texture
name remains a static registration/formatter inference rather than a separately
observed runtime string.

**The submitted depth and reciprocal homogeneous values are not the candidate's
raw view depth and its reciprocal.** Their cache producer and effective device
state still need to be established. Matching projected coordinates does not
establish depth equivalence, physical near/far planes or the cause of the failed
left image. Do not convert an observed renderer adjustment into a physical plane
distance without following its producer and consumers.

## Original-image control and limits

The first fresh screenshot still contained the startup “Let's Go!” overlay, while
the retained reference did not. That capture was preserved and excluded. Before
another capture, the facilitator declared an ordinary-input control targeting the
reference's displayed level clock, with no rotation, camera change or pixel-based
time search. The complete camera stayed exact. The fresh original region then
matched the retained reference exactly under the unchanged ROI and resolution.

The cursor had also moved between separate workers. A guard refused input before
the temporal control; that failure was preserved. The subsequent control recorded
and retained the actual ungrabbed cursor outside the existing ROI. Neither the
reference nor the mask, threshold or region was edited.

There was no candidate renderer change and no new per-value original-image
mutation acceptance in this READ round. The previous cell-selection mutation
survivors remain unprotected. Left still fails at `0.033010761206839635` against
the existing `0.02` criterion. The simulation suite does not close that failure.

## Corrections and probe failures

- Earlier predictions used the wrong texture identity and registration coordinates,
  packed the wrong alpha, and used conventional PI instead of the accepted operand.
  The facilitator caught these before the accepted probe. Superseded predictions
  remain; a failed redirection also left an empty prediction file. The corrected
  prediction was saved under a new filename and hash before the accepted run.
- One prediction field mislabeled a source-bank hash as a quad hash. The actual
  hashes were checked and the label correction recorded separately, preserving
  the immutable prediction.
- **Facilitator correction:** my first analytic preflight used the wrong pitch
  signs. It failed and was retained before correction. The corrected expression
  then rejected the agent's PI discrepancy and matched its corrected prediction.
- The agent read the frontend cursor as the gameplay cursor and compared an entire
  transient control block. Those guards were retracted and replaced with the
  actual gameplay cursor and declared semantic fields.
- An attempted capture exceeded the hardware-breakpoint limit. The original later
  exited; the facilitator found a matching Windows crash event reporting a debug
  trap at an installed breakpoint site. The initial unexplained-exit description
  was narrowed accordingly. The exact cleanup mechanism remains unestablished.
- A cache probe sampled a local before its store and captured stale records. That
  version was rejected; the corrected capture used the current source register.
  The facilitator's separate capture read the final polygon references.
- The public CLI's first trial rejected a bare GDB stop notification. That failure
  and clean detach remain. The corrected trial required actual console completion
  and reproduced the independently checked buffer and camera.

## Reproduction, cleanup and next work

`tools/harness/trace_cell_emission.py` requires an explicit owned PID and a fresh
output directory under ignored `build/tmp/`. It guards the stock executable,
paused level, complete camera, ungrabbed controls and chosen quad. It captures the
actual final buffer, cache records and call arguments. It deletes hardware
breakpoints while stopped before detach, and treats cleanup failure as failure.
This public CLI received a fresh live trial. Raw retail payloads remain ignored.
The compact [READ record](../../tools/harness/fixtures/cell_emission/live-initial-read.json)
keeps the checked call operands and capture hashes separate from calibration.

The local ledger is `build/tmp/a1-cell-emission/checkpoint.md`; independent checks,
raw rereads, screenshots, crash event, public-tool trials and restoration records
use the `facilitator-` prefix in that directory. The task agent exhausted its usage
limit late. Its files were preserved and the facilitator finished verification
without redispatching that unfinished work.

**Next question:** derive the selected quad's submitted depth from its source
coordinates through the original transform/cache producer and projection path.
Identify which renderer inputs change that mapping and where clipping consumes it,
then test any sourced fixture change against every retained image with isolated
mutations. Do not repeat the coordinate/UV emission trace or infer device state
from an unsuccessful painter-order control.

**Facilitator check:** independent raw-byte rereads, analytic preflight, live final
buffer/cache correlation, original-region repeatability, public CLI trial and
installation restoration were checked. The owned original closed after an ordinary
window-close request; its cleanup check found no game or debugger remaining. A later
Explorer-launched original was detected and left untouched, outside that owned
session. The installation hashes still matched. The required UCRT
build and full suite passed 14/14. No simulation or production renderer source
changed, and no new calibrated value cleared the pixel gate.
