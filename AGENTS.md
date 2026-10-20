# Lemmings Webolution — agent entry point

A 1:1 reimplementation of Lemmings Revolution. **The stock 640x480
`Lemmings Revolution.exe` is the authority** — not classic Lemmings, not plausibility, not a
decompiler's prose. A claim about the original needs an address and the bytes read there, or
a re-runnable script. A decompiler line is not a measurement.

Work on the `decompile` branch.

## Read these, in this order

| File | What it gives you |
| --- | --- |
| `docs/LESSONS.md` | **every fact already established**, each with the address or script behind it. Read it before touching anything, so you do not re-derive settled work |
| `PLAN.md` | the goal and the phases |
| `docs/tasks.json` | **what is in flight** — blocked / running / ready-for-verify (not a second plan) |
| `docs/ORCHESTRATION.md` | facilitator / round / explore split and dispatch loop for every harness |
| `docs/ROLES.md` | the model name for *your* harness |
| `.claude/rules/match-the-original.md` | the evidence standard — **canonical for every harness**, not Claude-specific |
| `.claude/rules/windows-msys2-build.md` | build and PATH traps |
| `docs/REA_SETUP.md` | the Ghidra-backed decompiler, installed and verified |
| `docs/FACILITATOR.md` | read this only if you are the facilitator |

`old/` holds the retired plan, four handoffs and 42 round notes from before the 2026-10-18
reset. Nothing was deleted. Code comments citing `docs/scratch/phase3-*.md` mean
`old/docs/scratch/`.

## Models and orchestration

Model names differ per client; `docs/ROLES.md` is the only place they belong. On **Cursor**,
the facilitator chat is Opus 5.5; workers are pinned in `.cursor/agents/` to Composer
(`composer-2.5-fast`), not inherited Opus. Resume the facilitator with `/facilitator`
(Cursor / Claude Code) or `@.opencode/commands/facilitator.md` (OpenCode). How sessions
split (facilitator plans and verifies; round agents do one brief; explore is
read-mostly): `docs/ORCHESTRATION.md`. Check `docs/tasks.json` before starting work so you
do not ignore a blocked or ready-for-verify round.

## Agent workflow

For feature requests that require code changes, act as the **facilitator**:

- Inspect the repository and current task queue, then turn the request into bounded tasks
  with acceptance criteria, file ownership, and dependencies. Follow `docs/ORCHESTRATION.md`
  and do not collide with work already in flight.
- Delegate implementation tasks to the appropriate available worker. Work is **targeted by
  default**: split the request into small tasks a cheap worker can finish and the facilitator
  can verify. Use the hard tier only when the work cannot be split, and record why. Keep
  workers' file ownership separate. Review their results and independently verify changes;
  verification does not delegate.
- If this harness cannot delegate, return the task plan and state that delegation is
  unavailable. Do not silently implement the feature yourself.

When assigned a specific worker task, implement only that scope. Do not re-plan the overall
request or take on sibling tasks. Report changed files, checks performed, and blockers to the
facilitator.

## The short version of the standard

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
- **Preserve quirks**: German menus, RETURN only when the menu highlight is exactly 0, a short
  ESC that fails to leave pause. Do not smooth them out.
- **Every behavioural change needs a test that fails against the old behaviour**, verified by
  reverting that one thing alone. `test_world`, `test_terrain` and `test_levelobjects` are
  separate binaries; running the wrong one makes a good test look broken.
- **Restore every live write** — grid cells, save targets, anything written to a lemming — and
  log them so the facilitator can confirm.

## Build

```powershell
.\scripts\Invoke-Ucrt64.ps1 "cmake --build --preset dev-debug"
.\scripts\Invoke-Ucrt64.ps1 "ctest --preset dev-debug"
```

MSYS2 UCRT64. Do **not** put UCRT64 `bin` on `PATH` for `tools/harness/*.py` — it shadows
the default Python and `capstone` disappears.

**All text files are LF, not CRLF.** `.gitattributes` enforces it. If git warns about CRLF,
convert the affected file(s) to LF before continuing.

## Who owns what

The facilitator owns `PLAN.md`, `docs/**`, `AGENTS.md` and the rules files, and verifies
results before promoting them. Round agents own the code, the tests, and new scripts under
`tools/harness/`; keep candidate prose outside `docs/` until it has been verified.

Report honestly. Do not round, smooth or over-claim: headline numbers get independently
re-verified, and across 31 rounds several claims were narrowed or retracted on re-check.
State plainly what you **measured**, what you only **read**, and what you could not settle.
