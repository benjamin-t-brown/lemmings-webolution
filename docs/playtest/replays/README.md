Only promote recordings of human playtests here. The clone records attempts to
`build/playtest/replays/` by default through `scripts/Play-Clone.ps1`.

After a win, run `webolution --game-dir "Lemmings Revolution" --replay <file>`.
Require `PASS`, `won=1` and `completed=1`, then copy that `.replay` file here
with a descriptive level name. Keep the original file and its build identity.
Do not promote `.partial` files, abandoned attempts, or generated test routes.

The legacy CTest `playtest_replays` checks every `.replay` recursively and prints
each level's result. An empty folder passes. Replay never reads or writes a save,
opens a window, or loads the original executable. A replay win is clone regression
evidence; it does not establish original fidelity or human playability by itself.

## Retired recordings
A recording whose inputs depend on a defect that was later fixed on purpose is renamed
`*.replay.retired` by the lead, with the reason here. It stays as the record of the human win
but is no longer replayed, and "all promoted replays reproduce" means the `.replay` files only.
- 2026-11-16-playtest-9/MM_BLOCK_BUILD_BASH and MM_E4_4_SKILLS: each contains a click on a
  shrugging builder that the build of commit 75e89f4 refused (step 2850 and step 2075). The
  playtest 9 Y6 fix (builder-shrug-assign-001) accepts that click, so the recordings diverge
  there by design. Both passed (21/20, 28/20) on the build they were recorded on. Retired
  2026-11-16; the levels need a fresh winning recording to regain replay coverage.
- 2026-11-16-playtest-10/MM_E2_4SKILLS (The Abyss): at step 1195 a Floater walks off the lower
  ledge into the open-bottomed hole and, on the build of commit f825802, survived on an invisible
  floor. That is playtest 10 Z5; fall-out-death-001 makes the fall fatal, so the recording
  diverges there by design. It passed (12/10) on the build it was recorded on. Retired 2026-11-16.
- 2026-11-15-playtest-7/M06_KF_1 (Sometimes up..sometimes down) and 2026-11-16-playtest-8/MM_FLOAT_CLIMB2
  (Float and Climb): both levels have two hatches. Playtest 10 Z6 (multi-hatch-simultaneous-001)
  makes every hatch release on the same tick, so each recording diverges at its second release
  (steps 175 and 163) by design. Both passed on the builds they were recorded on. Retired 2026-11-16.
