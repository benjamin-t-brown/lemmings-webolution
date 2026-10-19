# A1: projection/cache replay, pixel gate pending

This follows the [device-emission check](phase-a1-cell-emission.md). The
facilitator independently replayed the projection construction from raw executable
operands and retained camera bytes. Rounding each arithmetic operation to single
precision reproduces the retained matrices and the selected initial quad's defined
cache and emitted coordinate/depth fields exactly. Ordinary double-precision
arithmetic does not. This is post-exploratory verification, not a blind prediction
or proof of the runtime floating-point control word.

**A1 remains open. No new calibration is promoted.** No candidate render or isolated
pixel mutation ran in this checkpoint. The failed left image, prior unprotected
selection mutations, physical clip planes and effective device depth state remain
unresolved. Cross-capture byte agreement does not establish same-frame identity
or explain the image failure.

## Reproduction and corrections

The read-only [projection/cache auditor](../../tools/harness/audit_a1_projection_cache.py)
pins the executable, reads its projection literals and replays retained camera
states. Optional source/emission inputs check the selected initial quad. It reports
residuals and limits; it neither launches the original nor renders pixels. Retail
payloads and unpromoted operands remain in ignored local evidence.

The facilitator's separate scripts, `facilitator-projection-check.py` and
`facilitator-first-reference-check.py`, their reports, fresh raw rereads and ledger
are in `build/tmp/a1-depth-producer/`. The latter script finds candidate first
references in frozen records, not observed runtime first projections or proof
against intervening cache invalidation. The public auditor was read and rerun.

Corrections are explicit:

- The earlier unlocated matrix copy stage is now located by static tracing.
  Retained snapshot agreement corroborates the copy, without proving complete
  runtime stack provenance.
- An agent conflated adjacent restore and cache-reset functions. Independent byte
  rereads separate them; the restore helper does not contain the claimed reset.
- An agent conflated an active material-setup entry with a neighboring constructor.
  The active route calls a separate setup method. That constructor-based
  counterclaim was rejected before promotion.
- Facilitator review rejected the access-watch controller prototypes. They were
  shelved when static tracing located the writer, never executed, and remain
  unapproved probes.

## Environment and next step

Automatic approval review twice failed because its model was at capacity; neither
fresh-game startup executed. No process was acquired or installation file changed
by those attempts. Detailed sandbox process inventory was denied; a process-name
check subsequently found a running original that this round did not launch. It
was left untouched, with ownership unestablished. Installation hashes still match
the baseline. The latest REA session could
not be exported or confirmed closed because review capacity also failed there.
Its resource state remains unknown; cached evidence is preserved.

The initial sandbox suite failed progress save/load. A normal-user rerun passed.
A subsequent sandbox full-suite run also passed after setting the temporary
directory inside the Bash command supplied to `Invoke-Ucrt64.ps1`, using writable
`build/tmp/` storage. PowerShell-only environment settings had not survived MSYS
startup. Failure and successful rerun logs are retained locally.

**Facilitator check:** independent source replay, executable rereads, the public
auditor and unchanged baseline inputs were checked. The required build and full
suite passed 14/14. No production renderer or simulation source changed. The
missing acceptance work is a sourced candidate render against every retained
original, then each new value mutated alone and restored before the next mutation.
The existing pixel criterion stays fixed. Pixel execution currently needs the
normal-user review path: the sandbox cannot read the installed image dependency.
Do not bypass review or soften the criterion.
