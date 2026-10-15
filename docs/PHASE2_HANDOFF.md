# Phase 2 Handoff — Finishing the `.LVL` Format

2026-10-14

Lemmings Webolution — a 1:1 reimplementation of Lemmings Revolution. This is the
handoff for finishing Phase 2, the level format. It is written for whoever picks
up the work and dispatches agents at it; it assumes no context beyond the repo.

See [PLAN.md](../PLAN.md) §1.6 for the full reverse-engineering record and
`.claude/rules/match-the-original.md` for the evidence standard everything here
is held to.

---

## Where Phase 2 stands

The terrain format is solved. The collision mask is a length-prefixed array of
256 × 96 `u32` flag cells stored in every `.LVL`, and the simulation now runs on
it. What remains is a short list of unread fields, not a research problem.

### The finding that rewrote the phase

For most of this project the terrain was assumed to be derived from the level's
surface lattice, and a rasteriser was built to reconstruct it. That was wrong.
The engine reads its collision grid straight out of the file:

- `FUN_0043B640` bulk-reads the array into the world object's `+0x41C`
- `+0x41C` is the container `FUN_0040DE60` probes for collision
- `FUN_004311D0`'s box stamps and the builder's `or 1` are runtime edits on a
  mask that arrived whole

So the surface lattice is what the engine **draws**. It is not what a lemming
stands on.

### What is proven, and how

The grid chunk is anchored by walking the level's own chunk order from the
literal `"Cylinder"` string. Pure arithmetic, no search, and it lands in all 102
levels with `C == 256 * height` as a cross-check:

```
u32 8, "Cylinder"
u32[5]            all zero, in all 102 levels
u32 height        96, in all 102 levels
f64               0.36, in all 102 levels
u32 N, u32[N]     N = 49858 in all 102 levels -- the tube, identical per level
u32 R, R x 12     the records the engine loads into +0x43C
u32 C, u32[C]     the collision grid; C == 256 * height
```

| Claim | Evidence |
| --- | --- |
| The file chunk is the collision grid | Byte-identical to the live grid read from the running game, on 5 levels entered independently |
| The anchor is a parse, not a fingerprint | Lands in 102/102 levels; reimplemented from scratch and reproduces every offset |
| The resolution is the engine's | 256 × 96, one cell per world unit, confirmed by `FUN_0040DE60`'s `and eax, 0xff` and its row-count clamp |
| Our raster was the problem | Engine 35–63% solid per level; our raster 1.35–23% |

Wiring the real mask in moved releasable levels from **22 of 101 to 81 of 102**,
and the median entrance drop from 66 world units to 9 cells.

### What the previous plan got wrong

The four-item plan to finish Phase 2 did not get completed. It got invalidated.
Two of its items were work aimed at a target that turned out not to exist:

1. *Read the ring stride off the binary* — retired. It was the critical path on
   the assumption the lattice was collision.
2. *De-fit `SurfaceMesh.cpp`* — demoted. The three fits are still in the code but
   now affect only rendering.
3. *Make the corpus invariant a test* — done, then superseded by a stronger
   per-cell oracle.
4. *Widen the live check past one ledge* — done. Navigation is solved; any level
   can be entered.

The lesson worth carrying: the cheap hypothesis — *is it simply stored in the
file?* — went unasked for a long time while expensive reconstruction work
proceeded.

---

## What is left

One task closes the format. The rest is debt that no longer blocks Phase 3.

| # | Task | Where | Depends on | Done when |
| --- | --- | --- | --- | --- |
| 1 | Read the **seven unread flag bits** | `CollisionGrid.h:87`, named `kCellUnreadBits` | — | Each bit cited to the instruction that tests it, or recorded as untested-by-anything |
| 2 | Identify the grid's **two neighbour arrays** | `+0x42C` and the 12,288-entry array after the grid | — | Both named with addresses, or ruled out as not simulation-relevant |
| 3 | Settle the **spawn model** | sim + harness | navigation (done) | Known whether a lemming starts at the entrance object's position |
| 4 | Fix the **lattice array placement** | `LevelIndex`, `header[3]` | — | `grid0`/`grid1` located correctly, or the discrepancy explained |
| 5 | De-fit `SurfaceMesh.cpp` | `:52`, `:88`, `:155`, `:187` | 4 | The three MM_CLIMB_002 fits replaced or justified |
| 6 | Decode **PolyList kind 5/7 record tails** | `PolyList.cpp` | — | Trailing bytes read, or recorded as unused |

### Task 1 is the one that matters

Seven bits — `0x4 0x8 0x20 0x40 0x100 0x400 0x8000` — are parsed and ignored.
They are not rare: `0x403` appears on 29,654 cells corpus-wide and `0x21` on
20,896. Until they are read, the mask is not understood, and any of them could
carry behaviour the simulation is silently dropping.

Two specifics already observed, both unexplained:

- `0x82` appears **exactly 40 times in every level sampled** — a fixed feature,
  not terrain
- `0x20` does not occur in MM_CLIMB_002 at all, but accounts for 876 cells on
  MM_DIG_000 and 3,024 on MM_FLOAT

The method is the one that has worked all along: find the instructions that test
the bit, cite the address and bytes, make it re-runnable.
`tools/harness/read_steppers.py` is the vehicle — it already has the PE image
reader, the disassembler and the constant annotation.

### Task 3 decides whether anything else is wrong

81 of 102 levels pass the entrance-drop survey. The 21 failures are **probably
not a terrain problem**. The engine writes a released lemming at y = 32.190 on
MM_CLIMB_002, and that level's entrance object sits 12 cells above the floor
beneath it — so the entrance position is likely not the spawn position.

Until that is settled, do not treat 81/102 as a defect count, and do not tune
anything to improve it.

### Explicitly not Phase 2

These look like format work and are not. They belong to Phase 3 and should not
be bundled into a Phase 2 task:

- Object zones `0x200`/`0x2000`/`0x4000` and blocker `0x800` are parsed into
  named constants and ignored by the simulation — wiring them is behaviour, not
  parsing
- Gadget behaviour from the 417 parsed placed models
- `diggerDepth` now sinks twice as fast per tick, because it could not halve with
  the other distances when the resolution changed. Flagged in `World.h`. It is a
  provisional value either way, and the fix is to read it off the original

---

## What you inherit to verify with

This is the part that makes the handoff checkable. Phase 2 now has a **per-cell
oracle**: the original's own collision grid, readable both from the file and from
the running process. Any claim about terrain can be tested against it cell for
cell.

| Asset | What it does | What it proves |
| --- | --- | --- |
| `tools/harness/read_lvl_grid.py` | Reads the grid from a `.LVL`. No game, no exe | The file side, standalone |
| `tools/harness/read_grid.py` | Reads the grid from the running process | The live side; `--compare` diffs against one of ours |
| `tools/harness/dump_grids.py` | Enters levels and dumps each grid, verifying the level is genuine first | Multi-level ground truth |
| `tools/harness/read_steppers.py` | The whole static read: state vtables, animation sites, speed constants, thresholds | Any claim about the binary, re-runnably |
| `tools/harness/goto_level.py <LEVEL>` | Reaches any level by script | Unblocks every live check |
| `src/__test__/test_collisiongrid.cpp` | Parser vs 5 live dumps byte-for-byte, plus all 102 levels | A disagreement here is a **bug**, not a baseline |
| `src/__test__/test_corpus.cpp` | Corpus survey, both readings | A **baseline** — it pins current behaviour so changes are visible |
| `lrtool lvlstats <game-dir>` | Per-level survey, lattice raster and stored grid side by side | Breadth, offline |

Five live dumps are on disk in `build/live_dump/grid_*.bin`: MM_CLIMB_002,
MM_BUILD, MM_FLOAT, MM_BOMB, MM_DIG_000.

### The distinction that matters

`test_collisiongrid` and `test_corpus` look alike and are not. One is an oracle,
the other is a change detector.

- **Oracle** — `test_collisiongrid` compares against the original itself. If it
  fails, the parser is wrong. Never "update the expected value" here.
- **Baseline** — `test_corpus` pins what our code currently does so a change
  shows up. Its numbers are *not* targets, and 22-of-101 in particular measures
  our rasteriser, not the game.

That distinction was learned by getting it wrong: an earlier round treated a
survey number as a quality target and nearly tuned the rasteriser to improve a
statistic that was measuring the wrong thing.

### What the oracle cannot prove

It covers terrain cells and nothing else. It says nothing about timing, state
transitions, skill behaviour, or whether a lemming spawns where we think it does.
Those still need a live series against the running original, which is what
PLAN.md §2.2 is for.

---

## Environment

Five traps here have each cost a round. Put them in agent briefs verbatim.

### Build and test

```sh
export PATH="/c/progs/msys2/ucrt64/bin:/c/progs/msys2/usr/bin:$PATH"
make -C src test
```

16 tests. `world` runs 70 checks with the game directory reaching it, 66 without
— if you see 66 under `ctest`, the game dir is not reaching that test.

### The traps

1. **Executables need UCRT64 `bin` on `PATH` or they fail to load.** `lrtool` and
   the test binaries silently produce no output otherwise. This looks like a
   broken tool and is not.
2. **That same export breaks the harness scripts.** It puts the MSYS2 python
   first, and `capstone` lives in the *default* python. Symptom:
   `needs capstone: pip install capstone` from a script that worked an hour ago.
   Run harness scripts without the export.
3. **`clang-format` is a native binary** and needs Windows paths —
   `--style=file:/c/...` fails silently and reports zero churn, which reads as
   success.
4. **clangd reports phantom errors on new files** (`'assets/Box.h' file not
   found`, `no template named 'span'`). Its compile database is stale until the
   file is in a build. The real build is the authority.
5. **`.clang-format` was corrected** so a format run no longer rewrites 2,839
   lines. 1,431 lines of genuine pre-formatter drift remain; a one-time
   `clang-format -i` pass is still owed and should be its own commit.

### Driving the original

`goto_level.py <LEVEL>` reaches any level. It works by **overwriting the level
record that the one known map node points at** — the map is a 3D scene the camera
orbits, so node screen positions are projections, not stored data.

Hazards, all learned the hard way and all preserved deliberately:

- **RETURN only when the menu highlight is exactly 0.** The fifth main-menu item
  is "Beenden" and a blind RETURN has closed the game outright.
- The menu highlight index is a **heap** variable; it moves every run and must be
  located by diffing across a key press.
- **ESC from a post-level screen can quit the process.** A 25-frame ESC can fail
  to leave pause; 40 frames after `focus()` worked.
- **Both screen-capture paths return black** — dgVoodoo presents through a D3D11
  swapchain. Navigation is memory-driven only; do not spend time on screenshots.
- Changing level means **restarting the process**. There is no safe route from
  inside a level back to the map.
- `find_menu_index` fails about one run in six and the script refuses to press
  RETURN blind. That is the safety working. Retry.

### Where the addresses come from

Every address belongs to the stock 640×480 executable, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`, image base
`0x400000`. `read_steppers.py` checks that digest before reporting anything,
because a different build moves everything.

Relocations are stripped, so the exe always loads at `0x400000` and static
addresses are stable across runs. REA/Ghidra is available for `xrefs` but is
rarely needed — the capstone-based reader covers most of it.

---

## Dispatching agents on this repo

Six agent rounds produced these. Two of them come from collisions that actually
happened.

### Ownership, declared per round

**One agent owns PLAN.md — and it should be you.** `PLAN.md` is the project's
memory and every agent wants to write to it. Two agents contended for §1.6 in one
round and it had to be hand-managed. The working rule since: agents commit code
and park PLAN-ready prose in a scratchpad file, and the manager lands all
`PLAN.md` edits as one reconciled pass. This is also better for the document — it
stays coherent with a single editor, and you get to resolve anything two agents
contradict each other on.

**One agent owns the running game.** Process contention is worse than file
contention because the symptom is bad data, not a merge conflict. Static reads
need no process; say so explicitly in the brief so an agent does not restart a
game another is measuring.

**Name the files each agent owns, and the files it must not touch.** Agents will
otherwise reasonably improve a shared tool mid-round.

### Briefing

- **Give the cheap hypothesis first.** The biggest result of this project came
  from telling an agent to spend ten minutes checking whether the mask was simply
  in the file, before any disassembly. It was. Ask what would make the task
  unnecessary.
- **Say what a clean negative is worth.** "A clean negative is as valuable as a
  positive" produces honest reports instead of forced findings.
- **Give permission to fail honestly.** Telling an agent that an acknowledged
  stopgap beats a false parse is what produced a documented fingerprint-vs-parse
  distinction rather than a loader that silently guessed.
- **Pre-empt the attribution error.** An agent that finds a function doing the
  right thing will attribute it to the state it was looking at. Ask explicitly:
  *what calls this?*
- **Forward context beats a cold start.** These tasks carry heavy context —
  addresses, prior negatives, the standard of evidence. Agents that inherit the
  conversation outperform cold ones badly here.

### Verify before you commit an agent's work

Every agent round in this project produced at least one claim that did not
survive checking. None were careless — they were the kind of error that looks
right.

- A removal function was attributed to the skills; it was the trampoline state's
  slot 3, with no caller.
- A corpus median was computed over 98 of 101 levels because three filenames
  contained `[patched]`.
- A measurement harness reported zero churn because the tool it invoked was
  failing silently.

Re-run the headline number yourself. Reimplement the critical algorithm
independently if it is small — the grid anchor took ten lines of Python to
confirm, and that confirmation is worth more than the agent's own test.

### Rate limits

Agents get killed mid-work by session limits, and their final line is then a
fragment of in-progress reasoning, **not a conclusion**. Do not read it as a
finding. Resume them — they keep their context — and check what landed in
`git status` first, because an agent may have finished its work and died before
committing.

---

## Failure modes this project has actually hit

The project's standard is in `.claude/rules/match-the-original.md`: separate
observations from hypotheses, support every claim with an address and what was
read there or a re-runnable script, preserve quirks, and validate against the
original rather than against our own tests. These are the ways that standard has
been broken here.

### A negative result is only as good as its search criteria

PLAN.md carried "there is no occupancy buffer in the engine — scanned for across
all 182 MB and absent" and concluded the original does not rasterise. Both were
wrong. The sweep looked for a byte-per-cell bitmap, or a byte grid with at most
four distinct values. The grid is `u32` flag words and matched neither.

The cost was months of reconstruction work aimed at a thing that was sitting in
the file. **When recording a negative, record what you searched for**, so the
next person can see what it would have missed.

### Fitting to one level looks like progress

`rasteriseSurface` was tuned until MM_CLIMB_002's climb ledge passed a live check
at 1,231/1,231 foot cells. That result was real and the mask was still wrong
everywhere else — the level it was tuned on is an outlier, and the code still
carries `lo < 31.0`, which is that level's ledge height written as a constant.

A test that passes on the level you tuned against proves very little. Breadth
first.

### Plausible numbers get promoted

`fallSpeed` was set from the 25.0 constant in a state that *is* a faller by its
animation — and is not the lemming's own fall. The right state carries the splat
test, the umbrella and the floater check, and its constant is 20.0. The read was
corrected within the hour, but only because the state map was built afterwards.

Classic-Lemmings values are the specific hazard: they are always plausible.
`splatDistance` was 63 and the real figure is 30 cells. Being close is not
evidence.

### Our own units leak into claims about the original

Our mask was 256 × 192, two rows per world unit. The engine's is 256 × 96, one
cell per unit — it cannot represent a half-unit feature at all. Every constant
quoted in "mask rows" carried a factor of ours. That is now fixed, but it is the
kind of error that reads as a fact for a long time.

### The checklist

Before promoting anything into `Rules` or into PLAN.md as settled:

- [ ] Is there an address and the bytes read there, or a script that re-runs it?
- [ ] Is this an observation or a hypothesis, and does the text say which?
- [ ] If it is a negative, is the search criterion recorded?
- [ ] What calls this? Attribution is the most common error here
- [ ] Does it hold on a level other than the one it was found on?
- [ ] Whose units is the number in?
- [ ] Does it merely match classic Lemmings?
