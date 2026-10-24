# C2e: caption preflight, offline; G is a static and the 0x47AD8C rect is probably off in the paused frame

2026-10-23. Directive Q3 round 1 of 3. Worker note `build/tmp/c2e-caption-preflight/round-note.md`; script
`build/tmp/c2e-caption-preflight/read_caption_inputs.py`. No stock start, no debugger; the live path was **not run**.

## READ

- **G is the static `0x4C6A08`**, not the level screen object. The only direct call of `0x45CFF9` is `0x450A53`, preceded by
  `mov ecx, 0x4C6A08` (`0x450A4E`). The level screen `0x4CBFB0` is the pointer stored at `G+0x55A8`. Every input below is a fixed
  absolute address, with no heap chase. (The directive text and A7k/A7l prose read G the other way round.)
- Inputs: `[G+0x25FF8]` (double), `[G+0x25C64]` (i32), `[G+0x26024]` (rect enable, byte), `[G+0x572C]` (ui_state), `[G+0x1C968]`,
  `[G+0x25C60]`, `[G+0x1C9B3]`, `[G+0x1CB7C]`; each with its address and reading instruction in the worker note.
- `[G+0x25C64]` is the per-frame delta of the `0x4657D1` tick reads (bounded to 100 at `0x45D927` / `0x45D97B`), so it varies.
- `[G+0x25FF8]` is decremented only when `[G+0x26024] != 0` **and `[G+0x572C] == 0`** (`0x45CE86..0x45CEB3`), and
  `[G+0x26024]` is cleared once the field is strictly below -1.0 (`0x45CEC7..0x45CED4`). `0x45E0F3..0x45E0FB` skips the whole
  `0x47AD8C` rect and its draw when `[G+0x26024] == 0`. So after A7p's 14 s unpaused walk the rect is predicted **off** in the
  paused frame, and A7k's "1020" top was an intro-window value (first ~2.5 s). A prediction from bytes, not a live read.

## UNREAD

- What draws the caption's edge. At `ui_state == 1` the frame also draws `G+0x25C68` (`0x44E150`) and `G+0x25C90` (`0x44DE60`);
  A7h placed `G+0x25C68` from `[0x58CB40]`, `[0x58CB44]` and sizes from `0x47ACF9` / `0x47AC50`, whose bodies are unread.
- The list layout (inline records or pointers), so whether a block dump saves anything useful once the process is gone.
- Whether `[G+0x26024]` is really 0 after the walk, and `[G+0x1CB7C]` on MM_CLIMB_002.

Script: read-only handle (`PROCESS_VM_READ|QUERY`), 21/21 selftest on mouv, preflight pass; `--live` needs session 1 and
`--one-start-confirm c2e-1`. It reuses A7p `enter_level()` / `walk_14()` unedited (A7p opens its own write handle for the level
slot and restores it). Worker recommends one more offline round before the start. Worker read 9 of 10 bodies.

## Facilitator check

Re-read on mouv: `0x450A48..0x450A66` (call site and `mov ecx, 0x4C6A08`), `0x45E0E0..0x45E11B` (the `[eax+0x26024]` gate and
`je 0x45E2BA`), `0x45CE80..0x45CED4` (`cmp [ecx+0x572C], 0`, `fsub [+0x1C968]`, strict compare with -1.0). Reran `--selftest`
on mouv: `selftest OK`. Read `live()` and `cleanup()` in the script: the Popen is guarded by the confirm flag, hashes are
restored in `cleanup`. Not verified: the script against a process, the 14 s unpaused walk's effect on `[G+0x26024]`, the 21 checks
beyond the ones shown.
