# A1: active cylinder route corroborated, pixel failure still open

This follows the [camera checkpoint](phase-a1-camera.md). The camera, original reference
region and acceptance criterion were held fixed. **A1 remains incomplete.** No renderer
was changed and no new calibrated camera, clipping or geometry value is accepted here.

## What was verified

The executable's forward path reaches the inline world surface walker from the gameplay
frame, then selects a material descriptor and submits a cell polygon through the existing
transform, clipping and device-draw helpers. The facilitator re-read the call operands
independently; the similar overlay route is a separate caller.

An independent hardware-breakpoint trace exercised that route at the retained left-view
camera. The camera bytes at the actual walker hit matched the retained reference. A fresh
original screenshot reproduced the fixed reference region exactly. Opposite input then
restored the complete starting camera block byte for byte, and all injected input was
released.

The observed submission selected the inner texture bank. Its source vertex pool matched
the stored level pool byte for byte, and the captured material descriptor matched the
live material table. These are bounded READ observations of an actual submission; they do
not establish every polygon or material in the mismatching region. In particular, the
outer-only fixture's missing inner path is established, but its contribution to the failed
pixels is not yet isolated.

The compact [READ record](../../tools/harness/fixtures/cylinder_draw/live-left-read.json)
contains independently checked call bytes and the live correlation. It deliberately keeps
dynamic record values separate from accepted calibrated parameters. Retail vertex and
material dumps remain under ignored `build/tmp/`.

## Corrections and retained failures

- The agent called a descriptor field a texture selector. Its callee sets blend state;
  the field is a blend selector. The texture record is resolved through the other field.
- The UV copy spans a pair of double values. Describing its word copies as separate UV
  parameters would confuse the storage shape with the values.
- A candidate note placed a table-offset instruction at the preceding move instruction.
  The facilitator used the independently decoded instruction boundary.
- The facilitator's first completion check matched debugger metadata. That result was
  discarded; the corrected check requires actual console output and checks the camera at
  the hit. An initial public-tool trial also failed on quoted dump filenames; its failure
  and detach were retained, and the corrected tool completed a fresh trace.
- An unmasked activation trial moved the camera off reference. It was excluded. The new
  foreground helper shields the game's input during activation; a fresh run reproduced
  the retained camera and original image region, then restored the starting camera.

The original fullscreen launch reached a fatal-error dialog whose text could not be
recovered. The previously verified temporary windowed setup reached the frontend. The
cause of the fatal launch was not established. An earlier process also exited unexpectedly;
no draw-path claim depends on that process.

The task agent exhausted its usage limit late in the round. Its files were preserved, and
the facilitator finished the independent raw and live verification without redispatching.

## Reproduction and next work

`tools/harness/acquire_game_foreground.py` acquires the selected stock game's foreground
and verifies the actual owner. Call it between input batches from a hidden worker. It
masks activation input from the game and restores the shim's previous input state. It does
not attach input queues. A successful activation API return alone is not a focus check.

With an owned original paused at the retained camera, run
`tools/harness/trace_cylinder_draw.py` with its PID and the appropriate `--case`. It guards
the loaded level and camera, writes raw captures only under ignored `build/tmp/`, and
detaches its debugger. `tools/harness/audit_cylinder_draw_bytes.py` re-reads the static
windows; a byte window alone does not prove its interpretation.

**Next question:** map the visible cell polygon groups and their UV/material records into
an executable-backed fixture. Isolate each structural selection or new value, restore it
before the next perturbation, and rerun every retained view under the unchanged criterion.
Do not redispatch camera-operand measurement or claim this trace fixes the failed image.
No new per-value mutation is claimed in this READ-only round.

**Subsequent checkpoint:** the [cell-fixture replay](phase-a1-cell-oracle.md) attempted
that question and retains the failed candidate and corrections. Its next probe is
the original's actual device-vertex emission; do not treat this earlier next-question
paragraph as an unstarted cell-capture task.

**Facilitator check:** independent raw call decoding, the retained-left live trace,
source-bank and material-table comparisons, exact camera/input restoration, and the required
build and full suite were checked. No simulation or production renderer source changed.
Original installation hashes were restored and the owned game and debugger were closed.
The protected-file byte guard flagged the rules files. **Correction to the initial
unexplained-discrepancy report:** concurrent policy commits advanced the branch during
this round. The saved rule hashes exactly match the preceding committed bytes; the new
canonical-rule headers explain the changes. Those commits and headers were preserved.
Earlier retractions remain.
