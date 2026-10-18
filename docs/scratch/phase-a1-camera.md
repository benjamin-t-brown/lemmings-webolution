# A1 camera: verified subset, round still open

Authority: the stock executable identified in [LESSONS](../LESSONS.md). The initial
checkout was clean on `decompile`, with `ff71d5e` above `562a099`; no Lemmings process
was running. `main` was left at its pre-reset commit.

**A1 is incomplete.** The retained offline camera matches the fixed static region in
the initial, mouse-drag and right-key captures. The left-key capture fails. No numeric
near/far planes, complete control-mode implementation, or whole-scene fidelity is accepted.
The simulation and production C++ source were not changed. There is still no game entry
point; these are standalone verification tools.

## Accepted operand reads

These are executable operands, not rounded estimates of live camera coordinates.
`tools/harness/read_camera.py` independently maps PE sections, reads the bytes, checks the
math callees, and reports binary64 hexadecimal values as well as decimal representations.

| Operand | Binary64 value | Address / instruction bytes |
| --- | --- | --- |
| Level angular argument | `19.03` | Split pushes at `0x0045C8F8`, `0x0045C8FD`: `68ae0733406848e17a14` |
| Local distance magnitude | `377.516` | `0x004B1098`: `c74b378941987740` |
| Local height bias | `17.392` | `0x004B1808`: `3108ac1c5a643140` |
| Fixed pitch, turns | `-0.03135770272046` | Split pushes at `0x0045CF83`, `0x0045CF88`: `681d0ea0bf684efa52e7` |
| Yaw offset | `128.0` | `0x004B1350`: `0000000000006040` |
| Yaw units per turn | `256.0` | `0x004B07A8`: `0000000000007040` |
| Lens degree divisor | `360.0` | `0x004B1348`: `0000000000807640` |
| Pose PI operand | `3.1415927410125732` | `0x004B0F88`: `00000060fb210940` |
| Lens PI operand | `3.1415927410125732` | `0x004B1AB0`: `00000060fb210940` |
| Grabbed mouse horizontal gain | `10.0` | `0x004C0398`: `0000000000002440` |
| Rotation unit / divisor | `24.0` | `0x004C0388`: `0000000000003840` |

The PI operands occupy separate locations. Each was perturbed **alone**, with the other
unchanged. The earlier shared-parameter mutation is superseded by those separate checks.

The level pose builder is `0x0045CC09`. Its unzoomed local eye vector uses the level row
count plus the height bias, and the negative distance operand. The row getter at
`0x0040BAA0` reads object field `+0x418`; it is not a fixed eye-height literal. Yaw rotates
the eye vector. The basis additionally includes the fixed pitch. The captured unzoomed
case is verified; other zoom/elevation branches remain outside this pixel acceptance.

The frame passes the camera object to `0x00468E22` at `0x0045DA5F`; the preceding machine
instruction adds `0x55C0` to the main-object pointer. The affine starts eight bytes into
that camera object. The render context copies the pose and transposes its basis for the
view transform; independent live reads agreed with that layout and sign convention.

The lens setter is `0x0047B042`; its scale builder is `0x0047B061`. The call at
`0x0047B095` reaches `0x0049D8D4`, whose instruction at `0x0049D8F2` is `D9 F2`, **FPTAN**.
The algebra is `theta = angularArgument / degreeDivisor`,
`g = 1 / (2 * tan(lensPI * theta))`. The scale vector is
`[g/aspect, g*height/width, 1/width]`. Projection divides transformed x and y by transformed
depth and reverses screen y; it does not use a fourth homogeneous coordinate.
The consumer chain includes `0x0047D0C0` and affine helper `0x0040EAE0`.

The captured stock viewport center was **`(319,239)`**, read from runtime globals
`0x0058D294` / `0x0058D298`. Each component was separately perturbed and restored.
The fixture's level-height input and aspect were also separately perturbed. Neither a
conventional half-width center nor a common eye height may replace the observed state.

Several live doubles contain values quantized more coarsely than the binary64 replay.
The exact x87 precision mode and operation-rounding sequence were not established. The
fixture passes the stated pixel criterion; it is **not** a bit-exact arithmetic replay.

## Pixel criterion and results

Before any original-matched candidate render, the facilitator fixed stock resolution
`640x480`, ROI `[100,80,280,410]`, and normalized mean absolute RGB error **at most `0.02`**:

`sum(abs(referenceRGB - candidateRGB)) / (255 * 3 * ROI_pixel_count)`.

No resizing, fitted alignment, mask changes or ROI changes were applied. Repeated original
captures, including a fresh owned process with identical camera bytes, differed by zero in
that region. The original was paused on `MM_CLIMB_002`; the input captures used predeclared
active-gameplay operations, then paused again. Opposite input restored the original camera
bytes after every capture case.

| Case | Error | Decision |
| --- | --- | --- |
| Initial camera, base textures / nearest | `0.032037939305913164` | Failed |
| Initial camera, Hi textures / nearest | `0.020158337184480974` | Failed; not rounded into a pass |
| Initial camera, Hi / linear | `0.01846389824167602` | Passed in this ROI |
| Grabbed mouse input, Hi / linear | `0.016010321075680553` | Passed in this ROI |
| Grabbed right-key input, Hi / linear | `0.01477834994828459` | Passed in this ROI |
| Grabbed left-key input, Hi / linear | `0.033010761206839635` | **Failed; retained** |

Texture selection was corrected from target evidence, not by changing the camera:
`0x004446AE` selects the Hi path conditionally. `0x00470F29` selects the texture-filter
case. The fresh reference run recorded the detail flag, hardware filter flag and cached
filter selection; its camera bytes and fixed ROI reproduced the initial reference.

The fixture draws a complete outer skin. The original also shows cutaways and scene
geometry that this fixture omits. Their contribution to the failed left view has not been
isolated from other causes. Its camera-angle transition matched the prediction, but its
image still failed. **Do not promote the left binding as pixel-accepted or mark A1 done.**

Thirteen camera-field mutations each changed exactly one scalar, failed the pixel limit,
and strictly worsened error. Each input file was restored byte-for-byte, hash checked,
and re-rendered to the identical baseline PNG before the next mutation. Separate input
mutations accepted the mouse gain, rotation divisor, captured inversion factor and right
binding. The left-binding mutation worsened error but remains rejected because its
baseline already failed. The complete records, including failures, are in
[`tools/harness/fixtures/camera_a1`](../../tools/harness/fixtures/camera_a1).

## Controls: bounded acceptance

In the captured default control mode, unmodified configured rotation keys moved the
virtual cursor when the grab gate was off. With an active right-button grab starting over
empty scenery, the mouse and keys changed yaw. Selection, mouse-swap configuration and
window activation participate in that gate; an action label alone does not establish a
global input-to-camera mapping.

For the accepted grabbed mouse case, horizontal input contributes the stored mouse gain,
then the rotation divisor and captured inversion factor determine yaw change. The
constructor's right binding is scan code `0x4D` (NUMPAD6), stored by the instruction at
`0x004833A4` into `0x005D5938`; its complete bytes are `c70538595d004d000000`.
The live configured value agreed in this run. Bindings remain configurable.

The narrow predictor in `camera_fixture.grabbed_mode2_yaw` covers only those captured
unmodified grab cases. It does not implement modifiers, edge rotation, flip animation,
other modes, or a measured speed per second. Failed or inactive input probes are not
evidence that an input is unsupported.

## Corrections, including facilitator errors

- **Retracted:** the helper was initially called sine, and the facilitator repeated that
  interpretation before checking the math backend. Raw FPTAN proved tangent. All
  sine-derived focal/optical-angle claims are withdrawn; none was promoted to project docs.
- **Corrected:** the sine/cosine wrapper labels had been reversed. The independent raw
  checks found FSIN at `0x0049CBA2` and FCOS at `0x0049CC52`.
- **Corrected:** the early main-object pointer interpretation confused typed pointer
  units with byte offsets. The camera argument and affine start above are the raw-byte
  interpretation. The affine translation getter writes the eye destination directly;
  it was not an unused local result. The transpose copies a basis, not an enlarged matrix.
- **Corrected:** the facilitator's explicit mutation parameter file briefly used a
  conventional viewport center. Its baseline PNG hash check failed before mutations ran.
  Restoring the already captured center reproduced the previous passing image exactly.
- **Superseded:** a shared PI perturbation changed both operand sites. Separate pose and
  lens perturbations now provide the required individual tests.
- **Corrected:** LESSONS' old pause-state shortcut did not describe P gameplay pause.
  The stock writes state `1` at `0x0045D226` and resumes state `0` at `0x0045D508`.
  A stopped/running/stopped clock series and an isolated classification mutation verified
  that correction; it is a UI behavior check, not a camera-pixel precision claim.
- **Retained failures:** base/nearest texture cases, the left-key frame, inactive-input
  attempts, and a cursor helper that did not converge in gameplay. No threshold was moved.

## Facilitator check and reproduction

The facilitator wrote independent PE/lens and pose replays, re-read the cited addresses,
and checked the live forward/inverse pose. A recursive raw-byte audit of the agent's
candidate manifests re-read 216 cited addresses and compared 66 byte literals. Four
transcription/address errors were corrected before acceptance; the final audit had no
literal mismatches. This does not make every abandoned candidate interpretation true.

The Luna agent exhausted its usage limit late in the round. Its files were preserved;
the facilitator finished the verification instead of redispatching the same work.

After all individual restores, the required UCRT64 build and **14/14** suite passed.
Those C++ tests protect the retained project; the separate pixel audits provide the camera
evidence. No simulation constant was re-measured or changed.

```powershell
python tools/harness/read_camera.py
python tools/harness/prepare_camera_fixture.py
python tools/harness/render_camera_skin.py --hi --linear --parameters tools/harness/fixtures/camera_a1/camera-fixture-params.json --label baseline
python tools/harness/compare_camera_frame.py tools/harness/fixtures/camera_a1/original-0.png build/tmp/a1-facilitator/skin-candidate-baseline.png --roi 100 80 280 410
python tools/harness/mutate_camera_fixture.py
python tools/harness/mutate_camera_input_fixture.py
.\scripts\Invoke-Ucrt64.ps1 "cmake --build --preset dev-debug"
.\scripts\Invoke-Ucrt64.ps1 "ctest --preset dev-debug"
```

The input audit deliberately exits nonzero after recording the rejected left-key baseline.
A completed mutation audit must not be reported as an all-views pass. The camera library
requires explicit fixture operands, so an untested duplicate default cannot silently
replace a measured value.

Pillow and the local retail installation are required for pixel replay. Preparation reads
the installation and extracts inputs only under ignored `build/tmp/`; no retail level or
texture payload is committed. The retained PNGs are reference screenshots.

All injected input was released, the camera bytes were restored, and the owned game was
closed. The executable, wrapper configuration and save file matched their initial hashes.
One earlier owned process exited unexpectedly; its cause was not established. A fresh run
reproduced the reference, and the final owned process closed normally. No live game-memory
patches were used. The temporary windowed wrapper setting was restored exactly.

**Next work remains within A1:** isolate the failed view and establish depth/clipping
behavior without inventing conventional near/far planes. The current ROI cannot pin a
uniform depth scale, because it cancels in the projection ratios. No numeric clipping
range is accepted. Preserve the fixed criterion and these failures when extending the
fixture; do not soften the overall screenshot-fidelity goal or re-measure simulation.

## Failed-view follow-up: carrier alpha ruled out, cause still open

The facilitator independently re-rendered every retained view and recomputed the fixed
ROI error with a separate pixel loop. Every error and candidate PNG hash reproduced the
checkpoint above. The failed view remains failed; no camera, threshold or region changed.

The decoded Hi outer BMP carriers contain no embedded alpha for the fixture's RGB
conversion to discard. The facilitator checked the actual Hi inputs independently of the
agent's base-texture audit, using file headers and decoded channels. The reproducible
header reader is `tools/harness/audit_camera_bmp.py`:

```powershell
python tools/harness/audit_camera_bmp.py build/tmp/a1-facilitator --pattern "tile-*-*HI.bmp"
```

This rules out dropped embedded BMP alpha in those fixture inputs. It does **not** rule out
external masks, material alpha state, missing inner geometry or a different stock draw
route. Texture registration and a generic vertex-submit helper do not establish which
surface the original draws in the mismatching region. That connection remains open.

**Corrections during review:** the agent initially described the error concentration as
being on the left; it is near the steel panel's right edge. A quoted instruction byte and
the wording of a caller relationship also failed the facilitator's raw reread. Those
candidate descriptions were corrected outside the accepted evidence; no new routing
claim is promoted here.

**Facilitator check:** independent pixel and carrier audits, original-installation hash
checks, and the required build and full suite passed. No camera or clipping value was
added, so there was no new parameter mutation to accept. No production code or original
installation file changed, and no game process was started. The failed frame and all
earlier retractions remain in place. Depth/clipping is the next bounded investigation;
this follow-up does not close A1.

## Depth follow-up: static inventory does not establish runtime planes

The facilitator independently scanned every byte of each file-backed executable section
for literal memory-indirect calls through the state-method slot, then compared raw windows
and argument operands with the agent's device-global inventory. Those checks agreed. No
depth-enable, depth-write, depth-comparison or clipping-state write appeared in that
bounded direct-global inventory. This is **not** proof that depth testing is disabled:
register-indirect calls, nonliteral slot selection, other aliases and runtime defaults
remain outside that conclusion.

The conservative scan is reproducible; its output includes unrelated interfaces and must
be reviewed rather than treated as a list of device-state calls:

```powershell
python tools/harness/scan_camera_state_calls.py --output build/tmp/a1-depth-followup/independent-state-call-superset.json
```

The facilitator also re-read the conditional creation and attachment of a depth surface
in device setup and checked the local desktop headers. Static setup capability does not
prove that branch was active in the accepted gameplay frame or establish the effective
runtime state. Candidate depth predicates and alternate vertex-emitter conventions remain
in the research record; none is accepted here as a physical camera plane.

**Correction:** the agent initially mislabeled source/destination blend-state calls as
texture-map blending and culling. The installed desktop header disproved those names;
the candidate note retains an explicit retraction. No clipping claim may rely on that
interpretation. The archived Windows CE vertex description was also insufficient evidence
for the stock desktop runtime.

**Facilitator check and next question:** raw callsite/operand rereads and an independently
written scan checked the static headline. No new numeric depth parameter was promoted or
behaviorally tested. The required suite remains green; production code was unchanged.
The next depth probe must establish the active gameplay draw family and runtime state,
then use an original fixture with stable overlapping surfaces or a visible depth boundary.
Its pixel criterion must be declared before capture. The existing static region cannot
pin a uniform depth scale. Both the failed left-key view and clipping acceptance remain
open; A1 is still incomplete. No original process was launched during these follow-ups.
