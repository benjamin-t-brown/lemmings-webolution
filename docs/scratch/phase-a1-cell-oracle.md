# A1 cell fixture: retained pixel failure

This follows the [active cylinder route check](phase-a1-cylinder-draw.md). The
captured cell geometry has now been replayed in a separate offline candidate.
**It does not fix the failed left view. A1 remains open.** No new camera,
geometry-selection, sampler, clipping or depth value is accepted by this round.

The facilitator independently re-read the candidate's cited file-backed operands,
compared the captured cell records, nested meshes, source vertices and material
records with the paused original, and reconstructed the serialized cell metadata.
These are bounded READ checks. They do not establish that the candidate reproduces
the original's final device vertices or raster state. Generic serialized polygon
lists were not assigned to the generated cell meshes merely because their indices
looked plausible.

## Pixel verdict and mutation gate

The final replay kept the committed camera parameters, original screenshots,
resolution, ROI and acceptance threshold unchanged. The facilitator hashed the
candidate source and all consumed capture, audit, texture and reference inputs
before and after the run, and used a separate direct pixel loop to score it.

With the retained outer surface present, the candidate's fixed-region errors equal
the earlier outer-only errors in every retained view. Left still fails at
`0.033010761206839635` against the existing `0.02` criterion. Removing that outer
surface fails in every view. A separate predeclared control disabled only cell
software-depth rejection while preserving the outer underlay and submission order;
that also fails and worsens every view. This is a rejected raster hypothesis, not
evidence that the original disables depth testing.

The facilitator independently perturbed the inner group-selection coefficient and
the center child-selection threshold, each alone. Neither worsened the fixed left
ROI. The default invocation, complete dependency hashes and exact default PNG were
restored and checked **before** the next perturbation. These selections remain
unprotected; their read values are not promoted as pixel-accepted measurements.

## Corrections and excluded attempts

- The agent initially treated the empty auxiliary list as excluding an inner draw.
  The executable also submits a gated inline quad before visiting that list. The
  final candidate includes this route and parses only active inline storage.
- The agent's first additive candidate also changed the retained outer surface's
  sampler behavior through a shared raster function. That version was rejected;
  the outer sampler was restored before the final controls.
- A stale per-entry lookup then omitted active inline geometry when the direct
  meshes were empty. Earlier controls belong to that superseded version. The
  final replay uses the corrected lookup and frozen inputs.
- **Facilitator correction:** I initially attributed that discrepancy to an audit
  file changing during replay without before/after hashes proving it. That causal
  claim is withdrawn. The identified cause was the stale lookup.
- **Facilitator correction:** my first prediction of the live group boundaries
  assumed an inner coefficient before reading its operand. It failed. I retained
  that script and failure, read the actual bytes, then made a fresh prediction
  which matched the initial-view breakpoint locals. The other retained views have
  offline selector replays, not independently observed live boundary locals.
- A fresh original capture initially differed because the cursor occupied the
  fixed region. Ordinary bounded cursor input, bracketed by resume/pause, parked
  it outside the region without changing the complete camera block. The fresh
  original region then matched the retained reference exactly. Neither reference
  nor ROI was edited; the contaminated capture remains excluded and preserved.
- Early sandboxed workers could not enumerate the game window. This was an
  inconclusive desktop-access failure, not proof that the original had no window.
  The successful capture used normal-user access for both game and hidden worker.

## Reproduction and next question

The new harnesses are `capture_a1_live_cells.py`, `audit_a1_live_meshes.py`,
`audit_a1_cell_source.py`, `freeze_a1_cell_inputs.py`,
`render_a1_cell_fixture.py` and `audit_a1_cell_pixels.py` under `tools/harness/`.
They retain raw retail payloads and candidate outputs under ignored `build/tmp/`.
The public capture refactor has not received a fresh live trial; the independently
checked live capture preceded that refactor. Do not treat a successful help or
compilation check as a live validation.

The retained local checkpoint is `build/tmp/a1-cell-fixture/checkpoint.md`.
Independent replay scripts, input hashes, pixel reports, isolated mutation and
restore records, raw-byte rereads, live comparisons and cleanup evidence use the
`facilitator-` prefix in that directory. Superseded results remain alongside them.
These ignored artifacts are evidence from this installation, not portable assets.

**Next question:** correlate a selected original cell quad with the transformed
vertices and final device-draw arguments actually emitted for it. Verify corner
order, projected coordinates, depth, reciprocal homogeneous value, color and UV
through that path before changing the candidate raster. The full outer surrogate
can hide candidate geometry; neither its retained scores nor the failed controls
establish the original's effective device state. Numeric near/far remain open.

**Facilitator check:** the independent frozen replay and isolated restorations
above completed. The required UCRT build and full suite passed 14/14. No simulation
or production renderer source changed. The owned game closed normally; debugger,
input state, executable, save and wrapper configuration were checked and restored.
Earlier retractions remain. The failed image and unprotected selections prevent
promotion of new calibrated values.
