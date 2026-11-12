# save-research-001 — original SavedGame.dat format and persistence contract

Static research only. No original execution, hooks, capture, installed-file write or
production change. Authority: `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f` (registered in
`build/save-research-input/reference.json`; I did not recompute it — no shell in this
profile). Installed save sample `SavedGame.dat`, 1504 bytes, SHA-256
`f7f8603928890fb13d022d828c8a602cf7a7619ab24de82c7de4ae86ec337e6a` (same JSON; not
recomputed, size and content taken from `installed-save.hex.txt`, 94 hex rows).

Inputs (all static exports, treated as data): `reference-disassembly.txt` (objdump,
cited as `D:<line>`), `reference-headers.txt`, `installed-save.hex.txt`,
`save-loading-rea.json` (Ghidra pseudocode of 0x45F43C), plus ripgrep string matches on
the exe itself.

Labels: **[D]** disassembly, **[R]** REA pseudocode, **[S]** installed-sample
observation, **[I]** inference, **[U]** unresolved.

Address → file offset: every section has `VMA − 0x400000 == File off`
(`reference-headers.txt:385-392`: .text 0x401000/0x1000, .rdata 0x4b0000/0xb0000,
.data 0x4bf000/0xbf000), so e.g. VA 0x4C0E5C is file offset 0xC0E5C.

## 1. Verdict

The record layout and the read/write order are established by disassembly. The on-disk
file is a raw, unframed, little-endian image: **one int32 flag followed by 5 × 25 × 3
int32 values = 4 + 1500 = 1504 bytes**, which matches the installed file size [S].
There is no magic, version, count, checksum or transform in either routine [D].

Not claimable as a complete contract: field *meanings* of two of the three per-record
ints, the fopen mode string of the writer, the identity of two filename strings from
disassembly alone, the node↔(group,slot) table contents (`0x4B3948`), and original
behavior on write failure and short reads (section 6). None of these block a
byte-compatible codec; each is listed with exact addresses in section 8.

## 2. Filename binding

| Address | Evidence | Content | Used by |
|---|---|---|---|
| 0x4C0E44 | [R] `s_Compiled_LevelMap_dat_004c0e44` | `Compiled\LevelMap.dat` | loader 0x45F43C (not the save) |
| 0x4C0E5C | [R] `s_SavedGame_dat_004c0e5c` | `SavedGame.dat` | loader 0x45F43C `D:122165` |
| 0x4C0E6C | [D] `push 0x4c0e6c` at 0x45F971 (`D:122339`); [I] string | `SavedGame.dat` | writer 0x45F94D |
| 0x4C0E7C | [D] `push 0x4c0e7c` at 0x45FB02 (`D:122420`); [I] string | `Compiled\LevelMap.dat` | 0x45FADE (unreferenced, see 5) |

[I] for the last two: a ripgrep `-o` over the exe returned exactly the sequence
`Compiled\LevelMap.dat`, `SavedGame.dat`, `SavedGame.dat`, `Compiled\LevelMap.dat`, and
the slot spacing (0x4C0E44→5C = 24, 5C→6C = 16, 6C→7C = 16) fits `SavedGame.dat` + NUL
padding. NUL bytes defeat ripgrep byte-pattern matching, so adjacency was not proven.
Filenames are relative; the original does not build an absolute path [D].

## 3. In-memory record model

Game object base is VA 0x4C6A08 (`mov ecx,0x4c6a08` before every call: `D:105003`,
`D:175027`, `D:206656`). The save lives in a 125-element array of 0x118-byte records at
`this+0x1CBCC` (constructor/destructor `D:118775-118782`, `D:232733-232740` use
`0x118`, count `0x7d` = 125). Group stride 0x1B58 (= 25 × 0x118), 5 groups, 25 slots.

Record `[group g][slot s]` at `this + 0x1CBCC + g*0x1B58 + s*0x118`:

| Rec off | Width | Source file | Notes |
|---|---|---|---|
| +0x000 | char[0x80] | LevelMap.dat | name string (`0x48252d`→`0x47D940`, max 0x80) |
| +0x080 | char[0x80] | LevelMap.dat | second string |
| +0x100 | int32 | LevelMap.dat | 3rd value in LevelMap record |
| +0x104 | int32 | LevelMap.dat | 5th |
| +0x108 | int32 | LevelMap.dat | 4th (order in file: +0x100, +0x108, +0x104) |
| **+0x10C** | int32 | **SavedGame.dat** field A | no other access found |
| **+0x110** | int32 | **SavedGame.dat** field B | no other access found |
| **+0x114** | int32 | **SavedGame.dat** field C | **completion flag** |

Absolute addresses: A = 0x4E36E0, B = 0x4E36E4, C = 0x4E36E8 for (0,0)
(0x4C6A08 + 0x1CBCC + 0x10C = 0x4E36E0).

Separate flag byte: `this+0x55A5` = VA 0x4CBFAD.

## 4. Read path (load at startup)

`0x45F43C` (`D:122075-122280`), a `__fastcall` method on the game object. Called from
exactly one place, `0x45093B` (`D:105004`), in the startup sequence right after
`0x45E51F` and before the frontend work (`0x45C7E2`, `0x470E70`...). [D]

SavedGame half, `D:122165-122272`:

1. Construct file wrapper (`0x47DB73`, name = 0x4C0E5C) and open with `0x47E40D(mode=1, showErr=0)`.
   Return byte is tested.
2. **Open fails**: `D:122231-122272` zero fields A, B, C for all 5×25 records. The flag
   byte at +0x55A5 is **not touched** on this path. No message box (showErr = 0).
3. **Open succeeds**:
   1. `0x402560` reads one int32 → `flag = (value != 0)`; stored as byte at +0x55A5
      (`neg; sbb; neg` = bool normalize, `D:122177-122182`).
   2. For g in 0..4, for s in 0..24: read int32 → A (+0x10C), int32 → B (+0x110), int32 → C (+0x114),
      in that order (`D:122200-122225`). Values stored raw, no clamping.
   3. close (`0x47E844`).
4. Nothing is read after the 375th int. Extra file bytes are ignored. [D]

Primitive `0x402560` (`D:1938-1950`): `push &local; push 4; call 0x47DAC8`, returns
`[local]`. `0x47DAC8`→`0x47EEF7` (`D:163472`, `D:164964`) calls `0x49DA82` (CRT
read-like, args FILE\*, count, 1, buf, `D:164971-164976`) and adds the byte count to
`this+0x120`. **The returned count is never checked and the local is not initialised**, so a
short/truncated read yields an indeterminate stack value in the original [D/I].
Endianness: the host is x86 and values are copied as-is → little-endian [I from D].

Open routine `0x47E40D` (`D:164152-164408`), mode 1: tests disk file via `0x47E89B`
(→`0x49DD81`, a find-first style call, `D:164455-164476`); if absent it consults the
archive table object at 0x5D5608 (`0x47F759`) — so a name missing on disk can be served
from a `.box` archive [D]. Not relevant to the clone's own save, noted for completeness.

## 5. Write path

`0x45F94D` (`D:122330-122410`):

1. Wrapper over 0x4C0E6C; `0x47E40D(mode=3, showErr=0)` (`D:122343-122346`). **The return value
   is ignored.** Mode 3 has bit 2 set, which the write primitive requires (below).
   No existence check, no temp file, no backup; the file is opened in place.
2. `0x47EB26(byte [this+0x55A5] zero-extended)` — **int32** flag, so 0 or 1.
3. g 0..4 × s 0..24, each `0x47EB26`: A (+0x10C), B (+0x110), C (+0x114), same order as the reader.
4. Close (`0x47E844`).

Primitive `0x47EB26` (`D:164640-164669`): if `(mode & 2) != 2 || FILE* == 0` it calls
`0x482A02(file-string 0x4C2B5C, line 0x240, 0)` then `0x49D383(0)` — an assert/abort style
path; else `0x49DF93(&value, 4, 1, FILE*)` (fwrite-shaped, arg order same as the CRT
`fwrite`). Raw 4-byte store, no byte swap, no checksum.

Other writer-adjacent routines:

* `0x45F883` (`D:122281-122329`) — **reset**: byte +0x55A5 = 0 and all A/B/C = 0.
* `0x45FADE` (`D:122411-122491`) — reads `0x4C0E7C` ([I] LevelMap.dat) with a flag int then
  a string/record loop. **No direct call, no pointer reference anywhere in the export.**
  Treated as unreferenced/dead for this contract. Not a save routine.

### Write/reset callers (exhaustive over direct calls `call 0x45f...` in the export)

| Call site | Context | Effect |
|---|---|---|
| 0x485E65 (`D:175028`) | method at 0x485E2B: sets `[0x5D59E0]=0xDC`, then `*(int*)(0x4E36E8 + [0x5D598C]*0x1B58 + [0x5D5988]*0x118) = 1` (`D:175019-175026`), then `0x45F94D` | level-**win** handler: mark current (group,slot) field C = 1, write file. [I] "win handler": 0x485E2B has no direct caller (virtual/data), classification comes from the mark+save semantics and the clone's existing `READ485E55` note. |
| 0x49BCF4 / 0x49BCFE (`D:206657-206659`) | event dispatcher 0x49BC59; event id equals `[this+0x40]` | `0x45F883` reset then `0x45F94D` write: zeroed file. [I] New Game → Yes (the clone's `confirmChoice(yes)`); event-id→UI binding not proven here. |

No save is written on exit, retry, abandon or loss in the call graph [D].
Group/slot globals `0x5D598C` (group) and `0x5D5988` (slot) come from the loaded level
object (`D:88927-88941`: group = `[lvl+0x50AC]`, slot = `movsx [lvl+0x50B0]`).

## 6. Error/default behavior (what is and is not evidenced)

| Situation | Original | Basis |
|---|---|---|
| File missing at load | A/B/C := 0 for all 125; flag byte untouched | [D] `D:122231-122272`; flag default [U], reset routine would give 0 |
| Truncated file | not detected; unread ints are stale stack garbage | [D] `0x402560` + unchecked count |
| Trailing bytes | ignored; next write rewrites the file | [D] |
| Wrong magic/version/checksum | not applicable, none exists | [D] |
| Flag value not 0/1 on disk | read as `!= 0`; rewritten as 0/1 | [D] |
| Field A/B/C values | stored raw; C only ever tested `!= 0` | [D] 0x45E508, 0x49B6FF |
| Write open failure | return ignored; `0x47EB26` assert/abort path ([U] exact effect) | [D] |
| Write granularity | whole file rewritten every save; no atomicity | [D] |

The clone must therefore **not** copy truncated-read behavior (indeterminate values) and
should add atomic replacement. Those are deliberate deviations, not original behavior.

## 7. Field table (final)

Offsets are file offsets. Total 1504 bytes (0x5E0); `n = g*25 + s`, g<5, s<25.

| File offset | Size | Type | Meaning | Status |
|---|---|---|---|---|
| 0x000 | 4 | int32 LE | flag → byte at game+0x55A5 (VA 0x4CBFAD); nonzero→1 | read/write [D]. Consumed once, at 0x45DE86 (`D:120702`): tested only when 0x45E4AC (all "base==1" nodes complete) returns true, then picks string 0x4C0C60 if set, 0x4C0C50 if clear. Sole static consumer. Setter other than the loader: none found by displacement search [D-limited]. Semantic label (e.g. "ending shown") **[I]** |
| 0x004 + 12n + 0 | 4 | int32 LE | A (+0x10C) | semantics [U]; no consumer |
| 0x004 + 12n + 4 | 4 | int32 LE | B (+0x110) | semantics [U]; no consumer |
| 0x004 + 12n + 8 | 4 | int32 LE | C (+0x114): completion; written 1 on win; tested `!= 0` by 0x45E4AC (`D:121087`) and the map state routine 0x49B647 (`D:206099`) | [D] |

Consumer caveat: "no consumer" is limited to accesses of the forms `[reg+reg*1+0x10c|110|114]`,
`[ds:0x4e36e0|e4]` and `this+0x1cbcc` bases. Pointer-to-record indirect accesses with
different displacement forms were not exhaustively excluded.

### Installed sample [S]

All 1504 bytes are `00` (`installed-save.hex.txt`: rows 0x000–0x5D0 all zero). That fixes:
file size = 1504 = 4 + 125×12, and the fresh/new-game state. It does **not** exercise any
nonzero flag, A, B or C, so the sample alone confirms size and the zero state only.
Field positions come from code, not from the sample; no field-order claim rests on one
sample.

### Node mapping evidence

* Map node completion in the original: field C of record (g,s) selected by the 102-entry
  12-byte table at VA 0x4B3948 (`D:121076-121087`, `D:206093-206099`):
  `{int group @+0, int slot @+4, byte flag @+8}`; 0x45E4AC requires C != 0 for all entries
  with `flag == 1` (`D:121076-121078`).
* Clone: `src/app/MapScene.cpp:47-150` `kMapping[102] = {group, slot, base}`, with
  `levelIds_[node]` taken from `levelTable.groups[group][slot]` (`MapScene.cpp:309`).
  B3i (`docs/scratch/phase-b3i-map-preparation.md`) records this as the exact table
  4B3948 (three fields, third = `base`); the `base==1` set is nodes 88..101 (14 entries) in
  `kMapping`, i.e. the entries whose completion 0x45E4AC requires if the third field is
  the same byte at table +8 [I].
  The table bytes are **not in this task's export**, so node↔(g,s) equality is
  **[I] from B3i, to be rechecked** by the codec task (see 8).
* `kMapping` covers groups 0 and 1 fully (25 each), group 2 minus slots 18 and 24 (23),
  group 3 minus slots 1, 3, 19 (22) and only 7 group-4 slots (4, 6, 11, 12, 13, 14, 15):
  25+25+23+22+7 = 102. The other 23 of 125 records are not map nodes. Their A/B/C stay in
  the file and must be preserved verbatim.
* Original keying is (group, slot), not map-node index. The clone's
  `completionFlags_[102]` is keyed by node index.

## 8. Unresolved items and exact missing data

| # | Item | Needed (static) | Blocks codec? |
|---|---|---|---|
| U1 | fopen mode string for mode 3 (truncate vs append) | bytes at VA 0x4C2AEC (file 0xC2AEC); mode 1 at 0x4C2AE4; table `D:164274` dispatch → 0x47E628 | No: clone writes the full image; sample size 1504 is consistent with truncate |
| U2 | Filenames at 0x4C0E6C / 0x4C0E7C | 32 bytes at file 0xC0E6C | No (reader/writer symmetry + string order) |
| U3 | Contents of node table | 1224 bytes at VA 0x4B3948 (file 0xB3948..0xB3D8F) | Needed for the **mapping** check, not the codec |
| U4 | Meaning of A, B | no static consumer; would need dynamic evidence | No: preserve |
| U5 | Flag default when the file is absent; any setter of 0x4CBFAD | object ctor near 0x45BD..; REA xrefs on 0x4CBFAD | No: use 0 |
| U6 | Identity of the 0x49BC59 `[this+0x40]` event and the vtable that reaches 0x485E2B | vtable data around 0x4B1C30 / 0x4B3F38 | No for codec; confirms save-trigger sites |
| U7 | Effect of `0x482A02`/`0x49D383` on write failure | decompile those two | No: clone must return an error and keep the prior save |
| U8 | `0x47E89B`/`0x49DD81` exact CRT call | decompile | No |

None of U1–U8 is a format-critical blocker for a read/write codec. U3 is a verification
prerequisite for the node mapping, and the codec task must run it before wiring.

## 9. Production state trace (current checkout)

* `src/shell/Progress.{h,cpp}` define a custom WEBOPROG file (`kMagic = "WEBOPROG"`,
  version, language, volumes, 3-state slots). Its header comment says it is *not* the stock
  layout. `defaultProgressPath()` returns `<WEBOLUTION_GAME_DIR>/webolution_progress.dat`.
  Reachability: `Progress.cpp` is built (`cmake/webolution_sources.cmake:40`) and only
  `src/__test__/test_progress.cpp` includes it; **no production caller** of
  `saveProgress`/`loadProgress`/`defaultProgressPath`/`Progress` exists under `src/app`
  (grep for `Progress` in `src/app/main.cpp` returns only the help string at line 241).
  Its `complete()` also invents an unlock rule (next playable slot) with no original
  evidence. It must not be wired.
* `src/app/main.cpp:1060-1093` `runApplication`: opens `lemmings.box`/`patch.box`, creates one
  `FrontendSession`, loops legs; on `LegDisposition::Completed` calls
  `session->completeLevel(*library, *completed)` (`:1089`). Help text (`:239-241`) says
  "Progress starts empty in memory (no save writes)".
* `FrontendSession` (`src/app/FrontendSession.h:100`, `.cpp:222-250`): state is
  `std::array<int32_t,102> completionFlags_{}` (all zero at `create()`); never loaded from
  disk. `completeLevel` validates `selectedLevel_/selectedNode_`, builds results + a new
  `MapScene` from a *copy* of the flags with `flags[*selectedNode_] = 1` on
  `outcome.success`, then commits flags/map/results together (`.cpp:236-247`). This is the
  clone analogue of 0x485E2B and the single place a save must be triggered.
* New Game confirm: `confirmChoice(yes)` (`.cpp:95-106`) zeroes `completionFlags_`, restores
  `initialMap_`; comment says "never touch the installed save".
* `MapScene::build(lib, flags)` consumes `flags[i] != 0` only (`MapScene.cpp:300`), the same
  predicate as the original (`!= 0`).
* There is no restart-time load: `FrontendSession::create(library)` has no flags input
  (`FrontendSession.h:27`). `readMapFlags` (`MapScene.cpp:273`) reads a decimal text file for
  the `--map` viewer (`main.cpp:379`) and is not a save format.
* Mapping gap: `kMapping` is file-local to `MapScene.cpp` (anonymous namespace); nothing
  exposes node→(group,slot).
* Settings (language/volume) are not in the original file; WEBOPROG's settings fields have
  no original counterpart in SavedGame.dat.

## 10. Follow-on production scope (one bounded task: `save-codec-001`)

Milestone step: *finish a level → retain progress after restarting the clone*. Observed
failure (source-traced): `FrontendSession::create` starts at all-zero flags and
`completeLevel` only mutates memory, so any completed node is lost at process exit. This is
the earliest unparked dependency of the restart step.

Deliverables:

1. **Codec** (new, pure, no I/O): `SavedGame` = `int32 flagRaw` + `std::array<SaveRecordInts,125>`
   {A, B, C raw int32}. `parse(span<const std::byte>)`: require size exactly 1504 (clone policy;
   the original tolerates oversize and mis-reads short files) → `std::expected`. `serialize()`
   emits 1504 bytes little-endian independent of host endianness. Unknown values (A, B, flag,
   all 23 unmapped records, nonzero C values other than 1) are kept verbatim; flag is emitted
   as the stored value unless the clone itself changes it (documented difference: the original
   would normalise to 0/1, which is not byte preserving for odd values). Completion update
   sets C=1 only on the (group,slot) being completed.
2. **Mapping**: expose node↔(group,slot) from the recovered 102-entry table; first verify it
   against file bytes 0xB3948.. (resolves U3) with a unit test that parses the exe read-only
   as static data. Map `completionFlags_[node]` ⇄ C of record (group*25 + slot); don't route
   via `Progress`.
3. **Storage adapter** (separate from the codec): path
   `%LOCALAPPDATA%\LemmingsWebolution\SavedGame.dat` resolved in a Windows adapter
   (`SHGetKnownFolderPath(FOLDERID_LocalAppData)` or `LOCALAPPDATA`), plus an injected-path
   constructor for tests and a future web store. Create directory; refuse any path that
   resolves inside the installed game directory (never write the installed
   `Lemmings Revolution/SavedGame.dat`). Write to a temp file in the same directory then
   atomically replace (`MoveFileExW` with `MOVEFILE_REPLACE_EXISTING` / `std::filesystem::rename`
   on the portable path); on any failure the previous good file remains and an error returns.
   No import or copy of the installed save (no silent import).
4. **Integration** (small): `FrontendSession` receives an optional `SaveStore`; `create()` loads
   (missing file → all zero, i.e. original "open fails" behavior; corrupt/short file → error
   surfaced, never overwritten, session starts empty with a visible diagnostic) and
   `completeLevel` saves after the in-memory commit when `outcome.success`; New Game Yes
   writes the zeroed image (matches 0x49BCF4+0x49BCFE) to the clone path only. Failed save
   reports an error and keeps results on screen (original behavior on failure unknown).
5. **Tests with temporary paths only**: round-trip (parse→serialize byte-identical for
   crafted 1504-byte images including nonzero A/B/flag and unmapped records), the
   all-zero 1504-byte shape equals the installed-sample shape, truncated (0, 3, 4, 1503) and
   oversize inputs rejected, failed write preserves prior bytes (read-only directory and
   injected failing sink), path guard rejects the installed directory, mapped-node
   round trip into `MapScene::build` states, 125-record unmapped preservation.
6. **Restart demonstration** (before/after): before — complete `MM_CLIMB_002`-class level
   in the connected route, exit, restart: map shows only initial states. After — same run
   with a temp `LemmingsWebolution` directory shows node complete after restart and
   `SavedGame.dat` differing from the zero image only in the C field of that record. Uses
   the existing connected route (native-021); no original execution.

Out of scope: language/volume persistence, A/B semantics, ending-flag behavior beyond
preservation, Emscripten store, WEBOPROG removal.

## 11. Integration dependencies

* node↔(group,slot) export and U3 table verification (step 2) must precede wiring.
* Decision on the hash gate: the research accepts the exe hash from `reference.json`;
  the codec task's table test must verify the exe hash before reading offsets.
* `src/shell/Progress.*` stays unreachable; its test may stay, but it must not appear in the
  restart path or documentation as the save.
* Controller can pin `save-codec-001` after this research is `done`; U1/U2/U5–U8 can be
  queued only if a later task needs them (they do not block the codec).

## 12. Checks performed

Read-only: read the five `build/save-research-input` exports, Grep over the disassembly for all
references to 0x45F43C/0x45F883/0x45F94D/0x45FADE, to the three record offsets, to
0x4CBFAD, and to the four filename addresses; Read of the routines cited above;
Grep of the exe for the filename strings; Read of `Progress.*`, `FrontendSession.*`,
`MapScene.*`, `main.cpp:1040-1100`, B3i/B3k notes. No tests run by the worker; the supervisor
verifier runs separately. Research acceptance is not runtime completion and not a
claim of save compatibility until the codec task lands.
