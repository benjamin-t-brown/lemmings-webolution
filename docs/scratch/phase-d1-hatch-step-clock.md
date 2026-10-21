# Phase D1d: the hatch release countdown is frame-driven, not tick-gated

Queue id `d1d-hatch-step-clock`. Round 4 of the D1 gate question, following
D1a/D1b/D1c. Landed 2026-10-21. **Static read only — no live game process.**

## The caller chain, fully read

`0x00425A95` (the hatch step) has zero direct call sites — it is reached only
through virtual dispatch. It is vtable slot `+0x14` of the hatch's own
vtable, `0x004B0F50` (confirmed: the hatch constructor `0x004258C0` writes
`mov [ecx], 0x4b0f50`; `.rdata` at `0x004B0F64` literally holds the bytes for
`0x00425A95`; slot `+0x18` at `0x004B0F68` holds `0x00425E50`, which
`LevelObjects.cpp`'s own pre-existing comment already names "the hatch
loader" — independent corroboration this is the Hatch vtable specifically).

The generic per-object step dispatcher, `0x004423D2`, calls that slot
(`call dword ptr [edx + 0x14]` at `0x00442BDC`) on every element of roughly
ten typed object lists, forwarding its own incoming `dt` unchanged.
`0x004423D2` has **exactly three** direct callers, all inside one function,
`0x0045CFF9`:

- Two inside a time-ordered "catch-up" queue drain, each converting a queued
  gap (milliseconds, divided by `1000.0`) to a `dt` in seconds.
- One (`0x0045DCD4`) **unconditional**: whenever the queue has nothing due
  that fits the remaining real-time budget, this call fires anyway, every
  pass, carrying whatever real wall-clock time is left over.
- A third (`0x0045DD76`) is a paused/not-playing fallback with a **fixed
  `dt = 0.0`** (`0x004B17B8`, confirmed byte-exact zero) — stepped, but
  frozen, when not actively playing.

`0x0045CFF9` itself has **exactly one** caller in the whole binary,
`0x00450A53` — structurally once per rendered frame. The real-time budget
feeding all of this (`this+0x25C64`) was independently characterized by an
unrelated, already-landed round (A7l, `docs/scratch/phase-a7-25c64-writers.md`):
"a runtime tick, not a constant," clamped to 100 ms only on stalls — the
opposite of 200 ms quantization.

**Verdict: frame-driven**, the same continuous-accumulator shape this
project already documented for the `+0x1CC` animation clock
(`docs/LESSONS.md`), not the lemming stepper's discrete 200 ms tick. This
directly explains D1c's observation (uneven, sub-tick-scale per-sample
countdown deltas) as the real mechanism, not a sampling artifact.

## The `dt` getter — narrower than D1b's framing, and its target unresolved

Re-reading `0x00425A95` closely: the getter at `[hatch+0x44 sub-object]+0x48`
(`0x00425B3B`) fires **only once per hatch**, at the single "door just
finished, not yet armed" transition (gated by `0x00425AF7`/`0x00425B21`),
**not** on the recurring countdown decrements. Every later call reuses
whatever `dt` the generic per-frame dispatch chain above already supplied.
This narrows, but does not contradict, D1b's "the step's own dt" framing.

The getter's concrete target function could not be resolved by static
reading: the constructor chain supplying `hatch+0x44` terminates in an
opaque factory with no direct calls, and an independent Ghidra/REA pass on
the same constructor flagged the identical indirect calls as unresolved
("ReferenceManager supplies no call destination") — a genuine static-reading
limit, reported as such, not guessed around.

## Facilitator check (2026-10-21)

Independently re-read every load-bearing address from the installed
executable with fresh code (direct `struct` reads and full-`.text` E8 scans,
not the worker's own scripts):

- `0x00425957`: `mov dword ptr [ecx], 0x4b0f50` — byte-exact.
- `.rdata 0x004B0F64` = `95 5a 42 00` (→ `0x00425A95`) and
  `.rdata 0x004B0F68` = `50 5e 42 00` (→ `0x00425E50`) — both byte-exact.
- `0x004B17B8` = all-zero 8 bytes (`0.0`) — exact.
- `0x00442BDC` = `ff 52 14` (`call [edx+0x14]`) — exact.
- A full brute-force scan of every `E8` call in `.text` confirms **exactly
  three** callers of `0x004423D2` (`0x45dbe5`, `0x45dcd4`, `0x45dd76` — the
  identical three addresses, no more, no fewer) and **exactly one** caller
  of `0x0045CFF9` (`0x450a53`) — matching the round's claims exactly, found
  with independent scan code rather than by re-running `find_callers.py`.
- Cross-checked the "getter fires once, not recurring" narrowing against
  this facilitator's own full disassembly dump of `0x00425A95` taken while
  verifying D1b (`docs/scratch/phase-d1-release-trigger.md`): the getter
  call and seed sit strictly between the `jne 0x425b5c` (already-armed
  skip) and the shared fall-through label `0x425b5c` — confirms the
  narrowing without a new dump.
- `git status --short` shows only `docs/tasks.json`'s permitted fields
  changed; no game process was started, no production source touched.

Accepted as a landed finding. D1's release mechanism is now understood down
to a clean architectural verdict: a per-hatch, continuous real-time
countdown (not a tick-quantized one), seeded once from a fixed constant and
rolled over by a `RELEASERATE.PC`-derived interval, stepped every rendered
frame rather than every simulation tick. The getter's concrete target
(bearing on whether `0.723 s`, D1c's derived `arm_time`, generalizes beyond
`MM_CLIMB_002`) remains open. Next: an implementation round against this
now-fully-measured shape.
