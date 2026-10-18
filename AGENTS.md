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
| `docs/ROLES.md` | the facilitator / round-agent split and the model name for *your* harness |
| `.claude/rules/match-the-original.md` | the evidence standard — **canonical for every harness**, not Claude-specific |
| `.claude/rules/windows-msys2-build.md` | build and PATH traps |
| `docs/REA_SETUP.md` | the Ghidra-backed decompiler, installed and verified |
| `docs/FACILITATOR.md` | read this only if you are the facilitator |

`old/` holds the retired plan, four handoffs and 42 round notes from before the 2026-10-18
reset. Nothing was deleted. Code comments citing `docs/scratch/phase3-*.md` mean
`old/docs/scratch/`.

## Models for this harness (Codex)

**Facilitator: `sol`. Round agent: `luna`.** `docs/ROLES.md` carries the full mapping and is
the only place model names belong — other harnesses use different ones, so do not copy a
name into a rules file or the plan.

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

## Who owns what

The facilitator owns `PLAN.md`, `docs/**`, `AGENTS.md` and the rules files, and verifies
results before promoting them. Round agents own the code, the tests, and new scripts under
`tools/harness/`; keep candidate prose outside `docs/` until it has been verified.

Report honestly. Do not round, smooth or over-claim: headline numbers get independently
re-verified, and across 31 rounds several claims were narrowed or retracted on re-check.
State plainly what you **measured**, what you only **read**, and what you could not settle.
