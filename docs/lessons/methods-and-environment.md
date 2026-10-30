# Methods and environment

> Dated historical research excerpts, preserved in their original wording and order within each source range.
> Earlier conclusions may be superseded by later entries; this is not current project status or
> authorization. See [STATUS](../STATUS.md) for current scope and [LESSONS](../LESSONS.md)
> for essential startup guidance. Some excerpts refer to findings that appeared earlier in the
> former combined file; use their linked evidence and the source-range comments below.

<!-- Source lines 117-125 in the pre-split docs/LESSONS.md; excerpt preserved, relative links rebased. -->
## The one rule

The stock 640x480 `Lemmings Revolution.exe` is the authority — not classic Lemmings, not
plausibility, not a decompiler's prose. A claim needs **an address and the bytes read
there**, or a re-runnable script. A decompiler line is not a measurement.

SHA-256 of the authority: `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`, 32-bit PE.


<!-- Source lines 1088-1145 in the pre-split docs/LESSONS.md; excerpt preserved, relative links rebased. -->
## Hard-won rules of thumb

**A6f failed acquisition** ([check](../scratch/phase-a6-fresh-pause-gate-failed.md)):
one start reached entry pause but failed before walking/PNG; no fresh A6 gate
measurement. Patching an importing module's `write_mem` does not intercept a
function's globals in its defining module: the only retained slot event was the
restore, not the initial `goto_level.select_level` write. Helper-only mocks missed
this integration boundary. Main CLI candidate removed; baseline restored, 24/24
CTest. Start allowance exhausted; no automatic live retry. A6e actor-2 failure stands.

**A6f repair3 failed acquisition** ([check](../scratch/phase-a6-fresh-pause-gate-failed.md#repair3-renewed-attempt-also-failed-acquisition-before-game-creation)):
the renewed attempt failed before game creation: copying the capture script without
its `OUT/dgVoodoo.windowed.conf` dependency makes `read_bytes()` fail before the
empty-file fallback. Independent pure-read reproduction supports this diagnosis;
the PowerShell wrapper lost the full traceback. Finalizer's config rewrite is not
in `writes: []`; hash equality supports restored contents, not a complete write log.
No fresh gate score or production change; own build and 24/24 CTest pass. Stop-on-failure
forbids automatic retry despite no new stock process. The stricter old audit was waived.

**A6f repair4 (offline MEASURED, [check](../scratch/phase-a6-fresh-pause-pixel-result.md)):**
one carried-over authorized start produced a fresh image/table: ROI **708159/45441000**
passes, and all three scored components improve. Own rerenders and isolated input-loader
bypass discriminate the optional `--walkers` path; exact source restored, 24/24 CTest.
**Not A6 acceptance:** `same_pause` is a literal with no second stability snapshot;
`actors[:3]` omits full-list count/identity, and box component rank is not actor-row
identity. The worker corrected those labels offline without changing any scores.
Full frame remains 0.2122073 (diagnostic). Old actor-2 failure stands; no slope sample
or cause established. Install hashes match, no game/tasks remain, start consumed.
Further live work needs explicit authorization and a full-list/stability brief;
do not revive the waived initial-write audit or call passing arithmetic full qualification.

1. **One cell is one world unit.** Never multiply a height or distance by
   `PIXELS_PER_UNIT`. The factor-of-2 lattice raster is retired, and doubling a world-unit
   delta is the single most repeated mistake in this project's history — it survived in the
   plan's own prose for weeks after being disproved.
2. **Expect the wrong *shape*, not the wrong number.** Every provisional value that turned
   out wrong was wrong structurally: three "intervals in ticks" were animation cycles; a
   blocker "list scan" was a terrain field; an "explosion radius" was a stored bitmap; a
   "wait of 2 ticks" was a 14-frame animation; an exit "rectangle" was an object AABB.
3. **Halving a classic-Lemmings value leaves a guess.** `maxStepUp` and `lemmingHeight` came
   out 2 and 3, not the halved 3 and 4. The blast clears 37 cells, not the 81 a halved
   radius-5 circle would.
4. **A plausible series can come from the wrong object.** A billboard-quad probe was fitted
   to "a lemming" and produced clean, plausible numbers for **nine rounds** before anyone
   noticed the quad was not a lemming. Two published measurements had to be retracted.
   Read lemmings from the **actor list at `0x004CBB18`**.
5. **Check `.data` rawsize before citing a byte.** `.data` has va `0x004BF000`, vsize
   `0x1185B8`, but **rawsize only `0x7000`** — file bytes exist only to `0x004C6000`. A
   round once asserted a gate byte of `0xFF` that the loader actually zero-fills.
6. **Mutate one thing alone and demand a behavioural failure.** A test whose expectation is
   derived from the field it tests passes at any value. Reverting a whole round at once
   proves only that *something* was tested.
7. **A clean negative is a result.** So is "this does not appear in any reachable level".
8. **Preserve quirks.** RETURN only when the menu highlight is exactly 0, a
   short ESC that fails to leave pause (retracted 2026-10-23, B2h), the drawn crosshair disagreeing with the tested
   cursor. All real. Do not smooth them out. Menu language is not a quirk: the German
   menus were an early harness side effect (user, 2026-10-22); the English install is fine.


<!-- Source lines 1289-1343 in the pre-split docs/LESSONS.md; excerpt preserved, relative links rebased. -->
## The tooling

- **Static disassembly**: `python tools/harness/read_steppers.py --at 0x00430BA1 --len 0x90`
  (also `--states` for the 24 animation sites and 46 vtables, `--cell-flags`). Capstone.
- **REA** (Ghidra-backed decompilation) — see `docs/REA_SETUP.md`. This is the new primary
  tool for reading code we cannot measure live, which is most of rendering.
- **Live reads**: 40+ scripts under `tools/harness/`, all re-runnable, driven through
  `tools/dinput-shim`. `measure_steppers.py --compare walker|climber` is the per-tick oracle.
  `read_lvl_grid.py` and `read_grid.py` read the stored and live grids.
- **Screenshots**: `screengrab.py` captures via a **desktop BitBlt** — `PrintWindow` returns
  all black because dgVoodoo presents through a D3D11 swapchain. Needs the game foreground.
- **`compare_tick.py` is superseded** — it fits a billboard quad that is not a lemming. Do
  not quote its numbers.

### Environment traps, all learned the hard way

- Build through `.\scripts\Invoke-Ucrt64.ps1 "<cmd>"` (MSYS2 UCRT64). Test binaries need
  UCRT64 `bin` on `PATH` or they fail to load silently.
- **Do not put UCRT64 `bin` on `PATH` for `tools/harness/*.py`** — it shadows the default
  Python and `capstone` disappears.
- `tools/dinput-shim` is 32-bit mingw32, not UCRT64.
- `lrstate.attach` returns `(handle, pid)`. `read_lvl_grid.find_grid` returns
  `(offset, cells)`.
- **Skill assignment must go through the panel.** Proof is the stock count at `0x004C91D8`
  **decrementing**, not a sprite change. Physical indices ([C1a READ](../scratch/phase-c1-panel-indices.md)):
  0 climber, 1 parasol, 2 bomber, 3 blocker, 4 builder, 5 basher, 6 miner, 7 digger;
  8 fast-forward and 9 Armageddon are commands, not ordinary per-lemming skills.
  `0x45A4DD` is eligibility; `0x45B570` is a separate factory with a latent special
  index8 path. Do not infer physical index8 behavior from its `questionmark` art key.
- **Cap live sampling.** An uncapped single-lemming run at ~300k `ReadProcessMemory`/s killed
  two game processes. `--hz 2000` is safe.
- `goto_level` **can silently fail** — it once reported entering one level while the loaded
  path stayed another, and the dump was of the wrong level. Read back the loaded path.
- `goto_level.focus()` has crashed dgVoodoo on this machine.
- **Correction to the retired `2 = pause` shortcut:** P gameplay pause is `ui_state 1`
  (store at `0x0045D226`), and P resumes `0` (store at `0x0045D508`). The clock was
  independently observed stopped/running/stopped; changing only the harness's pause
  classification to the old value failed, then its exact bytes were restored and the
  suite passed. Do not conflate gameplay pause with the separate menu state.
  Frontend screen id `0x004C416C` remains: 3 menu, 4 confirm, 5 map.
- If you write to the live collision grid (a legitimate, proven probe), **log every cell and
  restore all of them**.
- **Open lead, not a diagnosed cause (D2e, 2026-10-22):** two consecutive stock-exe crashes
  (`0xC0000005`, access violation) on back-to-back rounds, each earlier in the launch
  sequence than the last (main-menu foreground polling, then the boot-wait/first-`attach()`
  window). Neither round's own evidence names a faulting module, and a facilitator-only
  read-only Application-event-log/WER pass covering both crash windows found no matching
  `AppCrash_Lemmings*` report for either specific incident. Two things that check *did*
  find: this exe has a long pre-existing history of crashing on this machine in unrelated
  prior sessions (`WER\ReportArchive`), and the host was cycling `Microsoft-Windows-Winlogon`
  logon/logoff and `CodexSandboxService.OpenAI.Codex` restarts roughly every 40-70 seconds
  through both crash windows. The correlation is unproven, not a cause — reported so a
  future round reads the event log immediately after any crash rather than re-discovering
  this from scratch. See `docs/scratch/phase-d2-rescore-crash.md`.


