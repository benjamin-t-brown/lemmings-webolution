# B1c: main-menu vtable and draw dispatch — static trace

Queue id `b1c-draw-dispatch-trace`. Brief: `build/tmp/b1c-draw-dispatch-trace-brief.md`.
Worker output: `build/tmp/b1c-draw-dispatch-trace/round-note.md`. Dispatch tag:
disassembly read. Read-only static disassembly; no live game, no `src/**` change.

**Disclosed procedural deviation**: `prediction.md` was written after the deep call-site
trace, not before, as the brief required. Disclosed by the round itself, not found
independently. Not escalated — the same posture D1m's own disclosed-after-the-fact
prediction received: the verdict here rests on reproducible bytes, independently re-read
below, not on the prediction's blindness.

## Result

Five findings, each either confirmed or reported as an honest, bounded negative:

1. **Closes the recon's gap and corrects it**: all 20 derived-ctor call sites now pair
   cleanly to a vtable. The recon's partial list had included `0x4b2600`, which this round
   found does not belong to any of the 20 — a genuine correction, not a restatement.
2. **The construction mechanism**: a single 37-case factory switch (`0x0048726f`) behind
   two thin wrappers, driven by a selector remapped through a 37-byte table at `0x487b0f`
   into a 37-entry jump table at `0x487ab3`.
3. **Load-bearing negative**: the factory's selector values 3, 4 and 5 — the exact numbers
   the already-known screen-id counter (`0x4C416C`) uses for menu/confirm/map — all remap
   to index 22, the same jump-table slot every invalid/unused selector resolves to
   (`0x004879bb`). The screen-id counter and the factory's own selector are two independent
   numbering schemes; the menu cannot be identified by calling the factory with 3.
4. **Main-menu vtable narrowed, not confirmed**: a shared "chrome" class (ctor#3, vtable
   `0x4b1e88`) is the strongest candidate for list-style navigation screens generally
   (its slot-7 override has an input-poll/item-cycle/confirm shape matching the documented
   quirks), but the actual menu *content* is narrowed from 20 candidates to 7 (all loading
   the shared `main_logo.bmp` backdrop via a 5-entry string table at `0x4c08f0`), not to 1.
5. **Draw path**: tracing ctor#3's slot-7 override one level deep, every call target stays
   inside the frontend's own `0x00486000`-`0x0049c000` address range, never reaching the
   known in-level Phase A/C draw-submission chain — supporting the recon's "separate
   frontend draw path" hypothesis for the step/input side specifically. No call in the
   traced chain was identified as unambiguously a *draw* call within budget.
6. **Font layout**: clean negative, extended. `_FONT.BMP`'s own name does not appear
   anywhere in the executable's bytes (only in the data archive), consistent with the
   glyph sheet being requested through a non-string identifier. No glyph-grid/metrics
   table located.

A concrete, cheap, live-checkable next step is proposed (not run, per this round's
no-live-game scope): read which content vtable is active via the parallel node list at
`0x5d59e4` the moment the screen-id counter reads 3, settling the menu-content question in
one no-input live poll.

## Facilitator check

Independently re-derived the round's most load-bearing claims directly from the raw PE
file (not trusting the round's own printed tables):

- Read the factory's prologue and dispatch instructions at `0x0048726f`-`0x004872c4` with
  `read_steppers.py`: byte-for-byte match (`cmp [ebp-0xb4],0x24; ja 0x4879bb`, the
  remap-then-jump sequence `mov cl,[edx+0x487b0f]; jmp [ecx*4+0x487ab3]`).
- **Independently decoded the 37-byte remap table at `0x487b0f` and the 37-entry jump
  table at `0x487ab3` from scratch** with a standalone Python script reading the raw file
  (not using the round's own tool or disassembler): the remap table matches the round's
  cited bytes exactly, and selectors 3, 4, 5 each independently resolve to remap index 22
  → jump target `0x4879bb` — reproducing the round's central negative finding from first
  principles, not by re-running its own script.
- **Independently confirmed vtable slot 3 is the identical value `0x4886e4` across all 20
  vtables** (using the round's own corrected 20-address list) with a from-scratch struct
  unpack of the raw `.rdata` bytes at each vtable address — every one of the 20 decoded to
  sensible function-pointer values (not garbage), corroborating the corrected vtable list
  itself, not just the slot-3 claim.
- **Independently confirmed the font-layout negative**: `data.count(b"_FONT.BMP")` on the
  raw exe bytes returns 0, matching exactly. Also independently confirmed
  `main_logo.bmp`/`small_back.bmp` each occur exactly once as literal strings in the exe
  (consistent with the round's claim of a shared pointer table rather than duplicated
  per-case strings).
- `git status --short` showed only `docs/tasks.json`'s two permitted fields — no `src/**`,
  `PLAN.md`, or other `docs/**` file. No live game or debugger used by this round or by
  this check.

Round accepted as reported; nothing narrowed, one genuine correction to the prior recon
confirmed as correct. Real, substantive progress on a hard reverse-engineering question,
honestly reported as incomplete where it is incomplete (the exact menu vtable, and the
specific draw call) rather than rounded up to "done." The round's own proposed live,
no-input next step (read the active content vtable at screen id 3) is adopted as the next
B1 round.
