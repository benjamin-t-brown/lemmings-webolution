# C3m: why `find_menu_index` returned None in C3j (offline, directive Q2)

Worker note: `build/tmp/c3m-menu-entry-diagnosis/{prediction.md,diagnosis.md,fix.patch}`. Offline; no game, no debugger.

**Cause: NOT NAMED.** c3j's script, launcher (`Popen`, 5 s, `open_game`, `enter_level`), session-1 one-shot task, windowed conf and install hashes equal c2h and a7p. The harness files `enter_level` calls were last modified 2026-10-13..19, before every run compared.

| Item | Tag | Evidence |
| --- | --- | --- |
| c3j `enter` 23:32:43 -> `no menu highlight` 23:32:58 (+15 s) | MEASURED | `c3j-stack-bottom-live/run.log` |
| Entered runs take 11-12 s from `enter` to `ui 3 screen 5`, covering the same acquire and scan plus the walk to 0, two RETURN taps and the select (a7n, a7o, c1f, c2h, a7p) | MEASURED | each `run.log` |
| c3j kept no `state.json` (no `entry_focus`) and no stdout, so the `menu-index candidates:` line and DOWN delivery are unrecorded | MEASURED | `read_stack_bottom.py` has no `save_state()`; `run_live.ps1` does not redirect |
| Same helper printed `menu-index candidates: 0` once before (d2b pid 47392, via `measure_steppers.enter`); its repair start found 1 and entered | MEASURED | `d2b-pick-capture/round-note.md:18` |
| Scan ran in a contested or slow window and lost a DOWN press | INFERRED, untested | |
| "Menu not interactive yet" | disfavoured: entered runs scan at the same point after launch | |

`fix.patch` (not applied): retry the three-press scan up to three times and log per-press key polls and foreground pid in `find_menu_index`. `git apply --check` passes. It is untested live and would not help if the cause is persistent focus loss; the logging would show that.

**Facilitator check.** Re-read the c3j log and the five entered runs' `enter` / `screen 5` timestamps (11-12 s each, c3j 15 s); the d2b line; `LRInput.key_frame` exists (`lrinput.py:13,148`); `git apply --check` passes on the working tree. Not re-derived: the launcher diff (taken from the worker's side-by-side), `d2b run.log` (UTF-16). Narrowed: "not named" stands; the timing difference is real but its cause is not shown.

**Consequence for the directive.** Q3's second condition is "question 2 named the entry failure and fixed it". Q2 did neither (patch proposed, cause unnamed), so Q3's live read is not triggered unless the user changes that.
