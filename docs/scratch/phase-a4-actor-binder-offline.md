# A4ac: offline actor-binder pointer and return-shape repair verified under `node:vm`

Queue `a4ac-actor-binder-offline`, worker `round-agent` (`a06b3cc7-49cf-4344-a194-1ac6353f1deb`),
with one same-worker repair (`build/tmp/a4ac-actor-binder-offline-repair1.md`).
Offline synthetic regression only: zero game launches, zero Frida attachments,
zero native providers. Final worker packet:
`build/tmp/a4ac-actor-binder-offline/round-note.md`; initial packet preserved at
`build/tmp/a4ac-actor-binder-offline/round-note-initial.md`.

## Verdict

The dormant actor-binder defects in `build/tmp/a4ab-zoom1-filter-requests/hooks_a4ab.js`
are repaired and protected by offline `node:vm` regression tests in
`build/tmp/a4ac-actor-binder-offline/hooks_a4ac_candidate.js` and
`build/tmp/a4ac-actor-binder-offline/test/run.mjs`. The failed A4ab script
remains byte-identical (`52c5f966b0ee3f0cd3e269df637a44244ab0aedd7d38b6e4bda3feec7a21d41d`).

This is **offline harness verification only**. It does not diagnose the A4ab
`D3DImm.DLL` crash, validate Frida hook relocation/callback safety, achieve live
hook-unload readback, establish effective zoom1 filter/alpha state, or explain
A4z's full-scale pixel failure.

## Binder defects repaired in `hooks_a4ac_candidate.js`

1. **Numeric array address vs `NativePointer` (`hooks_a4ab.js` lines 313–321):**
   `u32(ACTOR_LIST.add(0xc))` returns a JavaScript `Number` (or `null`), so
   `arr.add(i * 4)` threw `TypeError: arr.add is not a function` on any nonempty
   actor list. The candidate validates scalar `arrAddr`, converts it once via
   `const arr = ptr(arrAddr);`, and guards each slot read with
   `const a = u32(arr.add(i * 4));` while keeping count, vtable and action reads
   numeric.
2. **Return-shape bug on match (`hooks_a4ab.js` lines 352–354 vs 368–383):**
   In `hooks_a4ab.js`, `snapshotActors` returned `{ actors, match: null }` on a
   miss, but on a match set `match.actors = actors; return match;` (where `match`
   had only `{ row, relation }` and no `.match` property). The `DRAW` `onEnter`
   handler then checked `if (!snap.match)` and read `snap.match.row.actor`, which
   would have treated every matched call as a `BIND_MISS`. The candidate returns
   `{ actors: actors, match: match }` consistently and checks
   `if (snap.match === null)`.
3. **Pointer passed to scalar `hexN` in `DRAW` selection (`hooks_a4ab.js` line 384):**
   `hexN(ptr(snap.match.row.actor).add(0x128))` coerced a pointer object via
   `>>> 0` to `'0x0'`. The candidate uses
   `ptr(snap.match.row.actor).add(0x128).toString()`, emitting `ctrl: '0x10128'`.
4. **Unmeasured `action_obj+0xb0` alias binding removed (`hooks_a4ab.js` lines 342–348):**
   Only `ctrl.toString() === thisStr` (`relation: 'actor+0x128'`, established in
   A4s) is accepted as a match. Raw scalar `action_obj` (`[actor+0x340]`) and
   `obj_78` (`[actor+0x78]`) remain recorded on the row, while a submit pointer
   at `action_obj+0xb0` (`thisStr = '0x300b0'` with `a340 = 0x30000`) returns
   `match: null`.

## Executable byte pins re-read

Authority SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`:

- `0x47A75D`: `558bec83ec24894dec833d3cd5580000750c6a00b980cd5800e8ae67`
  (actor submit entry, `[0x58D53C]` zoom gate).
- `0x47AB7F`: `e8120000006a01b980cd5800e89963ffff8be55dc21000`
  (actor draw call `0x47AB96`, then `0x470F29(1)`).
- `0x4B10E8+0x10..0x18`: `f2ee4200bfe24200` (lemming vtable slots `+0x10` =
  `0x42EEF2`, `+0x14` = `0x42E2BF`).
- `0x4B0F6C+0x10..0x18`: `f2ee420014644200` (secondary actor vtable slot `+0x10`
  = `0x42EEF2`).

## Facilitator check (2026-10-20)

- **Repair 1 required and verified:** the initial worker submission did not load
  `hooks_a4ab.js` in `test/run.mjs` (it used a boolean toggle in
  `lib/snapshot-actors.js`), left `hooks_a4ac_candidate.js` as a comment stub,
  and retained `action_obj+0xb0` alias matching and `hexN(ptr(...))`. Repair 1
  fixed all four issues and preserved the initial packet as
  `round-note-initial.md`.
- **Independent checker (`build/tmp/a4ac-facilitator/verify_a4ac.cjs`):**
  1. Verifies `hooks_a4ab.js` SHA-256 (`52c5f966...`) and confirms lines before
     `snapshotActors` in `hooks_a4ac_candidate.js` (`9100d0d0...`) match
     `hooks_a4ab.js` verbatim.
  2. Mounts both real scripts in a separate `node:vm` harness with an independent
     `IndependentPointer` (`valueOf() -> NaN`, throwing on unmapped reads).
     Confirms unchanged `hooks_a4ab.js` throws `TypeError: arr.add is not a function`
     on a nonempty list, while `hooks_a4ac_candidate.js` passes all fixtures
     (`nonempty-match`, `DRAW` `BIND` + `TARGET_DRAW`, `unmeasured-action-obj-alias`,
     `empty-list`, `count=401`, `invalid-array`, `null-array-read`,
     `non-lemming-vtable`, and an unmapped-slot skip).
  3. Runs **four isolated single-line mutations** on `hooks_a4ac_candidate.js`
     against `node build/tmp/a4ac-actor-binder-offline/test/run.mjs`, restoring
     and verifying the exact SHA-256 (`9100d0d0...`) after each:
     - `M1_revert_ptr_arrAddr` (`const arr = arrAddr;`) -> FAILS test suite;
     - `M2_revert_hexN_ctrl_pointer` (`ctrl: hexN(ptr(...))`) -> FAILS test suite;
     - `M3_reenable_unmeasured_alias` (`|| altThis === thisStr`) -> FAILS test suite;
     - `M4_revert_match_return_shape` (`match.actors = actors; return match;`) -> FAILS test suite.
- **Suite and clean environment:** full build (`cmake --build --preset dev-debug`)
  and **20/20 CTest** pass. All files under `build/tmp/a4ac-actor-binder-offline`,
  `build/tmp/a4ac-facilitator` and `build/tmp/a4ac-actor-binder-offline-repair1.md`
  are LF. Zero `Lemming`/`webolution` processes and zero `LemWeb` scheduled tasks.

## Next bounded action

Repair and offline-test the remaining non-binder trace-script defects in a
separate scratch task (`HOOK_INSTALL` emitted only after `Interceptor.attach`
succeeds, and mid-call `0x470F86`/`0x470FA3` `esp[0]` device-pointer vs
`[esp+4..+12]` `(stage, state, value)` stack layout without mislabelling `esp[0]`
as `retaddr`) before considering any future stock instrumentation design.
