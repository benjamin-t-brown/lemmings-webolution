# B1b: a 640x480 main-menu (highlight 0) reference screengrab — captured in English, not German

Queue id `b1b-reference-screengrab`. Brief: `build/tmp/b1b-reference-screengrab-brief.md`.
Worker output: `build/tmp/b1b-reference-screengrab/round-note.md`.

## Result

One live, no-RETURN run (the one authorized stock start) reached the main menu and
confirmed, from live memory reads bracketing the capture (not assumed): screen id
`0x4C416C` = 3, highlight address `0x27a987c` = 0, both immediately before and after the
capture call and again 0.5 s later (no flicker). The saved image
(`menu_highlight0.png`) is exactly 640x480. Method followed the mandatory path:
`acquire_game_foreground.acquire(pid)` (never `goto_level.focus()`'s banned input-queue
attachment path), `find_menu_index` plus a copy of `goto_level.enter()`'s own drive-to-0
loop stopped before its RETURN block, `screengrab.desktop_shot()` cropped to the live
window client rect. `dgVoodoo.conf` and the exe's install hashes are confirmed equal
before/after; no stray process remained.

## Unexpected, honestly-reported finding: the captured menu is English, not German

Structurally the menu matches every already-settled fact (five items, highlight 0 on the
first item, screen id 3), but its text is English ("Start New Game", "Play Current Game",
"Language Selection", "Options", "Quit"), not German ("Neues Spiel starten" etc.) as
`goto_level.py`'s own module docstring and `.claude/rules/match-the-original.md`'s
preserved "German menus" quirk describe.

`SavedGame.dat` (`C:\progs\Lemmings Revolution\SavedGame.dat`) was last modified at
11:58:35 today — the same timestamp as `d3f-timed-solution`'s own `run.log` (the previous
live round, which actually won `MM_CLIMB_002`, saving 10/10). D3f's own round-note already
reports `SavedGame.dat` restored with hashes matching its own before-state, meaning D3f's
run did not introduce a net change relative to whatever state already existed when D3f
started — the matching mtime is consistent with a restore-copy (same bytes, new mtime),
not evidence that D3f caused the drift. No earlier round's note mentions `SavedGame.dat`
specifically (only the exe and `dgVoodoo.conf` hashes), and no backup or pristine copy of
`SavedGame.dat` exists anywhere in the repo or the install directory to compare against.
The file's `CreationTime` is 2026-10-12, predating this project's entire Phase D standing
run (which began around 2026-10-20/21) — so the install has accumulated writes across many
live rounds (D2's skill-assignment rounds, D3d's actual level win, D3e/D3f, etc.) over
multiple days, and nothing in the project's history has ever checked `SavedGame.dat`
against a true pristine baseline — only ever against each round's own immediately-prior
snapshot. This round made no read or write access to `SavedGame.dat` at all.

**Two explanations remain open, neither confirmed:**
1. English is itself a legitimate, selectable retail language (`UK` is one of the base
   `.LTX` languages per the B1 recon), and some earlier round's live navigation (not
   necessarily this one, not documented as a language change anywhere) changed the
   persisted choice, which the game remembers across launches — itself correct stock
   behaviour, just not the fresh-install default `goto_level.py`'s docstring assumed.
2. Some round wrote to `SavedGame.dat` or a language global directly without recording it
   as a quirk-affecting live write.

Either way, **this capture is English and is not presented as "the German reference."**

## Facilitator check

- Reviewed `metadata.json` directly: self-consistent (`screen_id_before/after/second`,
  `highlight_before/after/second` all exactly 3/0 across all three samples;
  `install_hashes_before`/`install_hashes_after` identical; `install_hashes_restored: true`).
- Viewed `menu_highlight0.png` directly: a clean, correctly-cropped 640x480 image; confirms
  the reported English text exactly (not a transcription error) and "Start New Game"
  highlighted at the top (item 0), matching the metadata's `highlight_after_capture: 0`.
- Independently confirmed `SavedGame.dat`'s current `LastWriteTime` (2026-10-22 11:58:35,
  `CreationTime` 2026-10-12) and cross-referenced it against this session's own prior live
  rounds' log timestamps: `d1o-continuous-live-recheck/run_live.log` at 11:33:07,
  `d3f-timed-solution/run.log` at 11:58:35 — the exact match to D3f, read as a restore-copy
  (D3f's own note already claims a hash-matched restore), not new evidence of which round
  caused the underlying drift. Confirmed no backup or pristine `SavedGame.dat` exists
  anywhere in the repo or install directory (`find` over both, no `.bak` or alternate
  copy), and that `docs/LESSONS.md`/`docs/AGENT_SAFETY.md` have never previously mentioned
  `SavedGame.dat` at all — this is a genuinely new gap, not a previously-known and
  undocumented one.
- `git status --short` showed only this task's permitted `docs/tasks.json` fields; the
  round's own deliverables are under the gitignored `build/tmp/b1b-reference-screengrab/`.
  No `src/**`, `PLAN.md`, `.claude/rules/**`, or other `docs/**` file touched by the worker.

Round accepted as reported; nothing narrowed. The structural capture (640x480, screen id
3, highlight 0) is valid and usable. The language question is recorded as a pending
question for the user (`standing_run.pending_questions`) rather than decided unilaterally,
since it touches a judgment call about a shared, persistent install's state versus the
project's own documented "preserve quirks: German menus" rule. Other B1 work not blocked
by language (tracing the frontend draw dispatch and font layout) continues in parallel.
