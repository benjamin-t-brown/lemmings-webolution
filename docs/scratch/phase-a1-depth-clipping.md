# A1 depth/clipping: clean near/far negative, wrapper-managed depth

This follows the [live-arena pass](phase-a1-live-arena.md). **A1 is still open only for the
control mapping and the remaining zoom/elevation branches.** The fixed criterion was not
touched.

## Verdict

- **No numeric near/far planes exist in the executable.** `0x46DDF0` is a direct perspective
  divide by transformed depth: `fld 1.0; fdiv [edx+0x10]; ...; fsub [ecx+0x770]` (context
  bias). There is no near/far term, no `(far+near)/(far-near)`, and no clip-plane constant
  pair. The lens builder `0x47B061` computes only the focal/aspect scale.
- **The only depth range is the viewport's normalized `dvMinZ = 0.0`, `dvMaxZ = 1.0`**
  (`0x4702d5`/`0x4702dc`), with clip rect `(-1, +1, 2, 2)`. That is not a physical camera
  distance.
- **The executable sets no depth or clip render state.** A whole-`.text` scan (716,800 bytes)
  finds **zero** `SetRenderState` writes of ZENABLE (7), ZWRITEENABLE (14), ZFUNC (23),
  CLIPPING (136) or CLIPPLANEENABLE (152). It does write states 9, 15, 19, 20, 24, 25, 27, 29.
- **Depth ordering is nevertheless effective in the frames.** The retained ROI contains a
  strong depth boundary: the depth-on candidate passes all four views (0.0148–0.0186); the
  painter-order control fails all four (0.0619–0.0767), differing at 18,515–20,597 ROI pixels.
- **The device is the shipped wrapper.** Live read: `[0x58CDB8] = 0x0092CAB8`, whose vtable is
  in `D3DImm.DLL` (dgVoodoo2); `[0x58D5B4] = 0` (the executable's own "skip Z-buffer" gate).
  `dgVoodoo.conf` has `DepthBuffersBitDepth = appdriven`.

## Limits

- The effective ZENABLE/ZWRITEENABLE/ZFUNC bits **inside the wrapper are not readable**; no
  documented default was substituted as a measurement.
- Whether the conditional Z-buffer surface was created is inference from the gates, not a
  read; the renderer object could not be located.
- The literal-displacement scan does not prove a register-indexed state slot or a
  runtime-computed state operand absent.
- The live drive left a camera that did not match the retained reference; the camera block was
  not written. The device-state read is camera-independent.
- The clean negative is about the **executable**: it has no near/far planes to omit. The
  wrapper still owns the effective depth test.

## Process state

The owned process was stopped; no game or debugger remains. The executable, save and wrapper
config match their recorded hashes. No hardware breakpoints were installed; only the
established level-select swap was used.

## Reproduction

```text
python build/tmp/a1-depth-clipping/scan_state_calls.py --output build/tmp/a1-depth-clipping/verify-state-calls.json
python build/tmp/a1-depth-clipping/score_fixture.py --dir build/tmp/a1-depth-clipping/candidates --labels depthoff
```

Artifacts: `build/tmp/a1-depth-clipping/round-note.md`, `state-table.json`,
`state-calls.json`, `prediction.md`, `live/depth-reads.json`, `live/modules.json`, and the
depth-off control PNGs.

**Next A1 item:** the complete input-to-camera mapping (modifiers, edge rotation, flip
animation, other modes, measured speed per second), then the remaining zoom/elevation
branches.
