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
