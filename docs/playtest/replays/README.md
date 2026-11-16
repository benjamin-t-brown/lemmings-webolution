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
