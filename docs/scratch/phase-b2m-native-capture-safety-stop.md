# B2m continuation — rejected capture route; extra start stops the pass

2026-10-27, approved pass `native-capture-first-entry-2026-10-27`.
First dispatch19:38:52.917Z; initial attempt1 returned without implementing its
retention prerequisite. Final repair attempt3 used the same round clock,
20:07worker return/20:22whole-round deadline. Both attempts conservatively count
as tooling, exhausting2/2; no new tooling pool or historical B2 count reset.
Packets under `build/tmp/native-capture-first-entry-2026-10-27/capture/`.

**SAFETY STOP:** repair made **two actual stock Popen starts against one authorized
pre-debited start**. No retroactive authorization/debit conceals the extra launch.
Neither required forced-exception preflight nor single-use launch control held.
No attempt4, retry, further native work or application integration is permitted in
this pass. The separately running offline worker was not interrupted; its later
terminal result is verified separately. Initial missing-evidence packet retained.

## Actual evidence and missing evidence

| Start | Popen UTC | PID / creation FILETIME | HWND | Reported raw client | Exit |
| --- | --- | --- | --- | --- | --- |
| authorized allowance, prerequisites NOT met |19:47:23Z|40140 /134376040433661182|11077918|2560x1440, origin0,0|1|
| unauthorized extra |19:48:14Z|16480 /134376040942080282|3411300|2560x1440, origin0,0|1|

Both identify the stock executable/path and desktop2560x1440. Worker code closes
its Popen-owned process via terminate/wait; neither is a demonstrated normal game
exit. Exit1 alone is not proof of a crash. No observed crash signature is claimed.
The worker did not acquire/verify foreground or sample native state/input.

The second retained desktop/client PNGs are2560x1440, exact raw crop of the same
desktop rectangle. **Visual inspection shows other desktop applications, NOT a
qualified native game frame.** A target-owned HWND and a desktop crop do not prove
that HWND's game pixels were visible or stable. No image was resized. There is no
640x480 qualification, menu/map/level screenshot, navigation sequence, physical
button/current-state observation or gate-reference replacement. DPI context
metadata is unqualified (missing pointer-safe API declarations); no DPI/root-cause
conclusion follows from those words. Installed fake-fullscreen config is a read
hypothesis, not a proved cause of the geometry/presentation failure.

The launcher reused output paths and removed worker-result before each execution.
First disk receipt and PNGs were overwritten. Facilitator recovered the complete
first textual receipt from the terminal session's actual shell output, with
message/tool/time provenance; **first images cannot be recovered**. No second
image is represented as the first run. Earlier HWND-conversion error and original
harmless exercise receipts were also overwritten on disk; retained tool history
and source/report distinguish them from the current overwritten paths.

## Facilitator check / causal correction

Own terminal-history parser finds two actual shell-result/native PID packets,
not merely two Popen strings in source. Independently reconciles PID/path/
creation/HWND/geometry/exit values, exact frozen worker/helper/capture hashes and
the prelaunch manifest. Frozen Python scripts match their prelaunch hashes; a
complete prelaunch identity for every evolving PowerShell launcher revision was
not retained, so do not call the entire launch chain immutable or qualified.

Actual same-path harmless-window wronggeometry404x261 saved raw images. The
purported forced-error receipt also reports geometry refusal: frozen helper
checks wronggeometry before force_error; frozen test only asserts a nonempty
error, then incorrectly prints PASS. Required exception retention was NOT tested
successfully before either launch. A later working-source ordering/assertion fix
was not the frozen code executed. No corrected exercise or new run authorized.

**Correct worker's causal explanation:** raw second command DID include `-TestOnly`.
Historical patch receipts show launcher switch support was installed before that
command. Bash misinterpreted the preceding PowerShell copy/hash commands, leaving
the old frozen worker in place. That frozen worker has NO `--test-only` branch,
and therefore could launch even when passed the flag. The raw record supports
failed re-freeze + unguarded old worker, not the claim that the command omitted
the switch. The launcher lacked a durable single-use refusal and overwrote prior
evidence. This is not a safe reusable launcher; no production helper is accepted.

Own verifier also exactly reproduces all three retained PNG crops and dimensions,
detects the false exception-test PASS, checks original executed source hashes,
and preserves full terminal messages/selected command outputs. Scripts/results:
`verify-capture-negative.py`, `verify-capture-cause.py`, `capture-independent/`,
`capture-terminal-messages.json`, `capture-test-switch-patches.json` under pass
prefix. No live re-execution or unrelated mutation was used to fake acceptance.

Untracked root NUL was attributed to this worker's actual
`python -m json.tool docs/tasks.json > NUL` commands, absent at activation.
Initial/final bytes and command provenance preserved; only then removed. The
new unaccepted `tools/harness/native_entry_capture.py` was copied byte-exact to
scratch then removed; frozen executed and later edited sources remain distinct.
Original B2m/B3k evidence was not changed. No source/test/renderer fix landed.

## Restoration, gates, disposition

All five protected install hashes match the before packet and independent prior
baseline: EXE, SavedGame.dat, dgVoodoo.conf, dinput.dll and patch.dll. No install,
save, language, progress, configuration or target-memory writes occurred; no
restoration owed. No debugger/Frida/hooks/injection used. Independent host inventory
19:50:57Z finds no game/app/debugger/Frida/LemWeb task. Capture session independently
succeeded/idle19:49:31.734Z; no REA used. Separate offline binding completion is
not inferred from this cleanup.

Own final fullbuild49/49CTestPASS68.19s; four actual map SOURCE frames and7badCLI
cases preserved. Fresh C3/C1 scores AND PNG hashes exactly unchanged:
C3`14934333/235008000=.06354818984885621`FAIL;
C1`285083/17721990=.0160863988750699` and
`285700/17721990=.016121214378294987`PASS. All16signed gates unchanged.

**Disposition:** rejected tooling/native route, safety-stop entire pass; raw
failure diagnostics only, no native entry/capture acceptance. Preserve1charged/
2actual starts and extra1unauthorized start. No unused allowance transferred or
remaining attempt spent. Future work needs a separate explicit safety/launch
decision addressing the exact stale-frozen-worker/single-use/append-only-evidence
defects, not a blind retry, broader observer core or gate relaxation.
