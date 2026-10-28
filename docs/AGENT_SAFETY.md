# Mandatory project guardrails

Read this before project work, then the evidence rule sections specified in your brief.
The canonical evidence standard is [match-the-original](../.claude/rules/match-the-original.md).
Always read its "Separate observations from hypotheses", "Support the conclusion"
(opening rule), "Preserve quirks" and "Validate against the original" sections.
Its subject-specific findings are reference material; read those relevant to your task.


- **Write the prediction down before any live run**, with what would falsify it.
- **One cell is one world unit.** Never multiply a height or distance by `PIXELS_PER_UNIT`.
  The factor-of-2 raster is retired and doubling a world-unit delta is this project's most
  repeated mistake.
- **Read lemmings from the actor list at `0x004CBB18`**, never from billboard quads: that fit
  produced plausible numbers for nine rounds while tracking something that was not a lemming.
- **Check `.data` rawsize before citing a byte** — file bytes end at `0x004C6000`.
- **Expect the wrong *shape*, not the wrong number.** An "interval in ticks" was an animation
  cycle; an "explosion radius" was a stored 7x7 bitmap; an exit "rectangle" was an object AABB.
- **A clean negative is a result.** So is "this does not appear in any reachable level".
- **Preserve evidenced quirks, not disproved assumptions.** The blanket “RETURN only
  at menu highlight0” claim is contradicted by [row1 → Map observation](scratch/frontend-return-negative-prediction-2026-10-28.md).
  Other row actions remain separate. The "short ESC fails to leave pause" quirk was retracted 2026-10-23 (not reproduced; see
  the evidence rules). Menu language is not a quirk
  (user, 2026-10-22); match the reference capture and do not reset the install's language.
- **Every behavioural change needs a test that fails against the old behaviour**, verified by
  reverting that one thing alone. `test_world`, `test_terrain` and `test_levelobjects` are
  separate binaries; running the wrong one makes a good test look broken.
- **Restore every live write** — grid cells, save targets, anything written to a lemming — and
  log them so the facilitator can confirm.

For builds or scripts, read [Windows build rules](../.claude/rules/windows-msys2-build.md).
Use the documented wrapper for builds; keep UCRT64 off the harness Python PATH. All text
files must use LF. For live probes also read "Acquire foreground automatically" in the
canonical evidence rules, verify the loaded level, cap sampling, and log exact restoration.
Read [REA setup](REA_SETUP.md) before using REA. Report measured, read and unsettled
findings separately, including failed checks and scope limits.
