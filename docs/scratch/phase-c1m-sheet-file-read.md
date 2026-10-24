# C1m: handle 6 to sheet file, which prefixes are present, and what decides Hi / Lo (offline, round 1 of 2)

Queue `c1m-sheet-file-read`. Worker packet `build/tmp/c1m-sheet-file-read/round-note.md`, `prediction.md`. Capstone via `read_steppers.py --at`
(REA not used); no game, no debugger, no stock start. Dispatched 11:31 MDT, packet ~11:50, landed with c1n.

## Result

- **File name (READ).** Loader `0x4432C1..0x4438B8` builds `sprintf("%s%s", prefix[i], suffix[idx_i])` (fmt `0x4C0090` = `%s%s`; prefixes
  `0x4BFFC8` = `Lemmings`, `LemmingsWater`, `LemmingsAcid`, `EvilRat`; suffixes `0x4BFFD8` = `Lo`, `Hi`), calls `0x446040(0x58D23C, buf)` (`0x443874`) and
  stores the return at `0x4E335C + 4*i` (`0x44387F`). The registrar only stores the name (`0x449170`); the file is opened lazily by `0x484EB0`
  as `Sprites\%s.spr` (fmt `0x4C33C4`). So handle 6 names **`Sprites\LemmingsHi.spr` or `Sprites\LemmingsLo.spr`**. `lemmings.box` holds both paths
  (`SPRITES\LEMMINGSHI.SPR` at 18509, `...LO.SPR` at 18535); no loose `.spr` exists on disk. That the box serves the read is INFERRED.
- **Handle 6 is the `0x443874` call: INFERRED** from registration order (Bits 0, Grey 1, Black 2, Additive 3, Axe 4, Smoke 5, then the loader); it agrees with c1l's
  measured `0x4E335C` = {6, -1, -1, -1} but the registry slot-6 name was not read.
- **Prefix presence (READ; UNREAD in C1h).** `.LVL` header: `u32 8`, `"Level 23"`, six `u32`, then four flag bytes at 36..39, one per prefix. `MM_CLIMB_002.LVL`:
  u = (99, 1230482, 2999954, 110592, 110592, 469066), flags (1,0,0,0): only `Lemmings`.
- **Hi versus Lo (READ rule, UNREAD number).** Table `0x4BFF88`: entry value `size - count*0x3A`; prefix 0 Lo 733,644 / Hi 1,453,984. Threshold
  `T = free - u1 + 0x3A*u0` (`0x443570..0x443592`), `free = 0x47099B(0x58CD80)` = `[0x58CD80+0xCC]` after a COM caps query (`0x1000`). For this level **Hi iff
  free >= 2,678,724 bytes**, else Lo. A device query: not a constant, not a config read, no 640 compare in the loop. `free` as available texture video memory is INFERRED
  from the DirectDraw 7 vtable layout. The step-down for prefixes 1..3 has odd partial sums (kept as read; irrelevant to this level).

## Facilitator check

Re-derived independently (fresh script on the exe's file image, section-mapped): prefix and suffix strings, both format strings, table `0x4BFF88` and the four `v` values,
`MM_CLIMB_002.LVL` header and flags, and the 2,678,724 threshold, all equal to the worker's. Re-decoded `0x443570..0x443592`, `0x44386F..0x44387F`, `0x47099B`. Scanned
`lemmings.box` for the paths. **Not re-derived:** the step-down CFG (`0x443602..0x443733`), the lazy-load chain `0x42FF90` -> `0x484EB0`, the DirectDraw vtable
inference, and registration order. My decode of `0x484F2C` was misaligned and does not confirm the `Sprites\%s.spr` call site; the format string at `0x4C33C4` I did read.

## UNREAD, and what is needed

`[0x58CE4C]` (`free`) and the registry slot-6 name (`[[0x58D23C+0x3C] + 6*0x88]`) are live-only. The `.spr` parse `0x483EF0` and the box open `0x47E40D` are the next
offline body. The sheet is not yet extractable.
