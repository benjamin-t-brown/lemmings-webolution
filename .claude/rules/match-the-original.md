# Match the original

> **Canonical for every harness.** Claude Code, Codex and Cursor all read this file; it is
> not Claude-specific. Other clients' rule directories hold pointers, never copies — a copy
> in `.cursor/` went stale and wrong for weeks. Change this file, not a mirror.

This is a 1:1 reimplementation of Lemmings Revolution. The stock 640x480 executable is the authority.

## Separate observations from hypotheses

Say which is which, in notes and in code. `Rules` in `src/sim/World.h` labels each field MEASURED or PROVISIONAL. Do not promote a guess because it is plausible or because classic Lemmings used it.

- Measured: `climbSpeed` is **1** — one cell per 200 ms tick. Corrected 2026-10-16: this used to cite a
  billboard series of 34.190, 35.190, 36.190, and those absolute values are **withdrawn** — round 10 showed the
  quad that probe locks is not a lemming (`tools/harness/compare_tick.py` carries the retraction). The
  conclusion stands on better evidence: against the **actor list** at `0x004CBB18`, a climber matched our sim
  for 16 ticks on one wall and 13 on another (`measure_steppers.py --compare climber`). The lesson that
  survived intact: it read "2 mask rows" until the raster was retired; one cell is one world unit now, and
  doubling a world-unit delta is the single most repeated mistake in this project's history.
- There are no provisional `Rules` fields left: as of round 10 every field is a read or a measurement, which has
  never been true before. Keep it that way — a new field arrives with an address or a series, or it does not
  arrive. `explosionRadius` is the cautionary tale: it was a halved classic value, and the original turned out
  to store a 7x7 stencil at `0x004B1528` clearing 37 cells where the guess cleared 81. Halving a guess leaves a
  guess — `maxStepUp` and `lemmingHeight` came out 2 and 3, not the halved 3 and 4, so the halving was wrong
  both times it was checked.
- `fallSpeed = 3` used to sit on this list as the example hypothesis. It is now **4**, measured live over three walk-off edges and agreeing with `20.0 * 0.2 s`. Promoting it took a series, not an argument that it was plausible.

## Support the conclusion

A claim about the original needs an address and what was read there, a code reference, or a harness script someone can run again.

- `0x004C36A0` is the **frontend and level-map** click hit-test, not the in-level one. Corrected 2026-10-15: it has exactly two references in all of `.text`, both its own accessors `0x00455EA0` / `0x00455EC0`, whose readers are frontend and map and whose writer is gated on `byte [0x004C369D]`. Nothing in the level screen reads it, which is why writing it during play does nothing. In a level the cursor is the pair of doubles at `0x004CBFB0` +0x16A0 / +0x16A8 — `0x004CBFB0` is the level screen object (`0x0045DA9E` reads it as `[0x004C6A08 + 0x55A8]`), the drawn crosshair is written *from* those doubles by truncation, and the pick is an AABB against each lemming's own stored screen rectangle at `lemming+0x128`. The quirk below still holds: the drawn sprite is derived, so it can disagree with what is tested.
- **Read lemmings from the actor list at `0x004CBB18`, not from billboard quads.** `measure_motion.centroid` is
  world height and the screen pair is its projection, but matching a quad to a lemming is a fit, and round 10
  caught it fitting a non-lemming for nine rounds: the locked quad drifted at 3.8 world units a second where a
  walker moves 5.0, and sat at Y 32.190 where every lemming in the pen read 32.000. Any per-tick claim wants
  the actor list.
- `tools/harness/check_mask.py`, `measure_steppers.py`, `measure_stepup.py` and
  `measure_antigrav.py` are re-runnable. `compare_tick.py` is **superseded** —
  do not quote its numbers; use `measure_steppers.py --compare`.
- Anti-grav is cell bit `0x8000` (round 9): corpus 18/18 with `anti_grav.a3d`,
  walk probe at `0x0041C154`, gravity flip at `0x0042F5E3`. The bit alone is
  not enough — MM_CLIMB_002 painted with `0x8000` and no anti-grav object
  flipped nothing.
- Trampoline / bounce is cell bit `0x1000` (round 13): fall land
  `0x00416560` attaches bounce `0x004B0810` before splat; **0/102** `.LVL`
  grids carry it (runtime paint by trampoline enter `0x0041B838`);
  `pressure_pad.a3d` is not the source. Re-run `measure_trampoline.py`.
- Water / acid hazard drown is cell bit `0x40` (rounds 14–15): spawn
  side-effect `0x2f` tests it at `0x00415386` and attaches drown
  `0x004B0570` (~2.0 s). Acid is the **same** path — no separate state;
  type `0x2e` unused in retail; `0x100` only sets `lem[+0x5f]`.
  `Entrance_water` / `Entrance_acid` are sheets / zones, not the hazard.
  Re-run `measure_water.py` / `measure_acid.py`.
- Teleport is object-only (round 16): `teleport.a3d` pads (vtable
  `0x004B1250`) use overlap `0x00430BA1` at `0x00435467` / `0x004354CB`;
  partner `+0x260` from id `+0x98` / `0x00444BA1`; attach `0x28`→`0x2c`
  (countdown 4); relocate `0x0043503C` (Y+2.0 at `0x004B1238`). Live AABB
  12×8 (col±6 × [ty..ty+8]); facing preserved. `teleport_fx.a3d` FX-only;
  no exclusive cell bit (18/18). Re-run `measure_teleport.py`.
- SpeedPad is object-only (round 17): `speedup_two_way.a3d` (vtable
  `0x004B1178`) → FASTER `0x004B0A38` for 5.0 s (`+0x50=2`);
  `0x0042E2BF` doubles the main state while boosted. Overlap
  `FUN_00432742` / `0x00430BA1`; cue `SpeedPad` `0x004C0D10`. Live AABB
  2×6; no exclusive cell bit (9/9). `pressure_pad.a3d` is not this object.
  Re-run `measure_speedpad.py`.
- Balloon cue is unused in retail (round 18, clean negative): string
  `0x004C0D24` → slot `+0x1c8f8` / `DAT_004E3300`; register/preload/unload
  only; **0** play refs. No `balloon.a3d`. Exit `balloon_*` meshes are
  decorations. Re-run `measure_balloon.py --audit`.
- Fire cue is unused in retail (round 19, clean negative): string
  `0x004C0D2C` → slot `+0x1c8fc` / `DAT_004E3304`; **0** play refs; no
  fire a3d. Re-run `measure_fire.py --audit`.
- Witch cue is UI-only (round 20): string `0x004C0D34` → `DAT_004E32B0`;
  sole play is pause easter egg on `e10_kf_1` (`0x0045D200`); no lemming
  gadget / no `witch.a3d`. Re-run `measure_witch.py --audit`.
- Lever is object-only (round 21): `lever.a3d` / vtable `0x004B1288`;
  overlap `0x00436D30` facing-gates vs `+0xb8`; activate toggles and fans
  type 4/5 messages; no lemming attach. Re-run `measure_lever.py`.
- ForceF cue is unused in retail (round 22, clean negative): string
  `0x004C0CC8` → `DAT_004E32D8`; **0** play refs. Re-run
  `measure_forcef.py --audit`.
- Axe is a HUD sprite sheet (round 23): `0x004C0D6C` → sprite registrar
  `FUN_00446040` / `DAT_004E331C`; UI bind `0x00496E40`; no lemming
  gadget. Re-run `measure_axe.py --audit`.
- Bubble cue is SFX on fluid type `0x23` (round 24): `0x004C0C9C` /
  `DAT_004E32C4` played from `FUN_00404372`; no Bubble gadget / no
  `bubble.a3d`. Distinct from drown `0x22`. Re-run
  `measure_bubble.py --audit`.
- Scrape cue is dig/bash refuse SFX (round 25): `0x004C0CA4` /
  `DAT_004E32C8` at `0x0040C139` / `0x00415B33`; no Scrape gadget. Re-run
  `measure_scrape.py --audit`.
- Rumble cue is object state-commit SFX (round 26): `0x004C0CFC` /
  `DAT_004E32F0` played from `FUN_00423749` (vtable `0x004B0F00`) when
  prior `+0xa4 ≠ 0`; no Rumble gadget / no `rumble.a3d`. Re-run
  `measure_rumble.py --audit`.
- Changeop cue is UI skill/volume SFX (round 27): `0x004C0CB4` /
  `DAT_004E32D0` at skill-panel `0x0045AD07` and options slider
  `0x0048EB0C`; no Changeop gadget / no `changeop.a3d`. Re-run
  `measure_changeop.py --audit`.
- Die cue is SFX on type-`0x23` enter (round 28): `0x004C0CD0` /
  `DAT_004E32DC` at `0x004042CF` when fluid `+0x128==1`; distinct from
  FallSplat / unused Splat; no Die gadget. Re-run `measure_die.py --audit`.
- Splat-exemption zones (round 29): Fluid `+0x128` 0/1/2 stamps
  `0x200`/`0x2000`/`0x4000`; hatch → `[+0x4c]` via `0x004BFFC8`; land
  `FUN_00416433` skips splat for `0x200`+[+0x4c]==1 and `0x4000`+[+0x4c]==2
  (lava has no arm). Not drown `0x40`. Re-run `measure_splat_exempt.py --audit`.
- Inverted steppers (round 30): after anti-grav, step-up / climb / dig match
  upright with gravity flipped; dig refuse `0x20` when `[+0x44]==-1`
  (`0x00415A96`). Re-run `measure_inverted.py --audit`.
- Cell-flag writers (round 31): `0x100` authored-only (3 levels; no runtime
  setter); `0x400` door/gate stamp vtable `0x004B0F00` (`|0x403` /
  `&0xfbfc`). Round 15 `0x100` reader unchanged. Re-run
  `measure_cell_writers.py --audit`.
- The exit save test is object-overlap AABB `0x00430BA1` (round 11), not a
  guessed 8×8 / 12×14. Exit step `0x0042688D` / list `0x004C88D0`; save state
  `0x004B0B28` increments `0x004CBAA4`. Live boxes are 2×3 inclusive
  (`[round(ang×width)±1] × [ty−9 .. ty−6]`) for **all four** retail exit
  models — `Exit.a3d` (84), `Exit_water.a3d` (14), `Exit_any.a3d` (9),
  `Exit_acid.a3d` (1) — dumped round 12 on alone levels of each; column
  centre is `round`, not `floor` (round 11's formula said floor; 46 of 111
  placements disagree, including MM_FLOAT). Re-run
  `measure_exit.py --dump`; scratch `docs/scratch/phase3-round12-exit-models.md`.

## Preserve quirks

Keep behavior that looks wrong when the original does it. German menus, RETURN only when the highlight is exactly 0, a short ESC that fails to leave pause, and the drawn crosshair disagreeing with the tested cursor are all real. Do not smooth them out.

## Validate against the original

A green unit test shows our sim matches itself. Agreement with the running original is a live read or a differential check, on the stock executable, through `tools/dinput-shim`.

## Acquire foreground automatically

The user authorized automatic foreground acquisition for probes. Use
`acquire_game_foreground.acquire(pid)` from a hidden worker between input batches, then
verify its actual foreground owner before sending game input. The helper masks activation
input from the game and restores the shim's prior input state. Do not ask the user to
click the window for every probe. A tool console can take focus back immediately, so keep
acquisition and the input batch in the same hidden worker.

Do not use the older `goto_level.focus()` input-queue attachment path, which has crashed
the original. An activation API returning success does not prove that the game owns focus.
Preserve the loaded-path, camera, pause-state and exact-restoration checks. The
[draw-path facilitator check](../../docs/scratch/phase-a1-cylinder-draw.md) records the
bounded live verification and its corrections; it does not mark A1 complete.

For debugger probes, explicitly delete hardware breakpoints while the original is
stopped before detaching. Stay within the target's hardware-breakpoint limit, require
actual console completion markers, and verify cleanup and the surviving process.
A successful detach message alone is insufficient. The
[emission check](../../docs/scratch/phase-a1-cell-emission.md) preserves a matching
Windows debug-trap crash record and the subsequent independent capture and cleanup.
