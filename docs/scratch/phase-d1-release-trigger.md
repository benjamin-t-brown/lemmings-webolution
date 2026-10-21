# Phase D1b: the original's release-trigger mechanism (static disassembly read)

Queue id `d1b-release-trigger-read`. Round 2 of the D1 gate question, following
D1a (`docs/scratch/phase-d1-release-walkout-trace.md`). Landed 2026-10-21.
**Static read only — no live game process was started.**

## The mechanism, as read

`0x004CBA78` ("RELEASED") is **not a scalar**: it is a 16-bit word array
indexed by sheet type (`lemming+0x4c`), incremented in the Lemming
constructor (`0x0042DE51`, at `0x0042DFF1`/`0x0042E000`, reload+add+store) and
decremented in the destructor path (`0x004263DC` → `0x0042E17C`, at
`0x0042E1F2`/`0x0042E204`). It reads as "currently alive of this type", not
literally "released so far" — indistinguishable from D1a's measured series
only because that capture had no deaths and `MM_CLIMB_002` has a single
hatch of type 0.

The constructor's two callers are both inside the hatch step function
`0x00425A95` (`LevelObjects.h`'s own name for it). Structure, `this` = the
hatch object, independently re-disassembled in full (296 bytes) against the
installed exe:

- A one-time **arm**: once a door sub-object (`hatch+0x44`'s vtable slot
  `0x44`, a bool "finished opening?") reports true, `hatch+0xAC` is set, and
  the countdown field **`hatch+0xC0` (a double) is seeded to the fixed
  constant `1.3`** (confirmed at `0x004B0F48`, byte-exact IEEE-754 1.3).
- Every step while armed and lemmings remain (`hatch+0x98 > 0`): `hatch+0xC0
  -= dt` (the step's own real-time delta, from the same door sub-object's
  vtable slot `0x48`), then compared against the literal `0.0` at
  `0x004B0338`. While `> 0`, nothing fires.
- When `<= 0`: `hatch+0x98` decrements, a `Lemming` is constructed (pushing
  `hatch+0xB0` type, `hatch+0x9A`/`+0x9C` X/Y as the ctor args — the same
  hatch words D1a and `testRetailSpawn` already use), and then — critically —
  **`hatch+0xC0` is *not* reset; the recurring interval at `hatch+0xB8` is
  added back into whatever `0xC0` currently holds** (a rollover accumulator,
  so drift from one cycle's `dt` carries into the next).

So the first release and every later one are governed by the **same field**,
but **seeded with two different numbers**: a fixed `1.3` once, and the
per-level `hatch+0xB8` every time after.

## `hatch+0xB8` traces to `RELEASERATE.PC`, confirmed numerically

`hatch+0xB8` is set once at level load, in `0x00425DE3` (re-disassembled in
full, matches byte-exact): the per-hatch rate (`hatch+0xA0`) is clamped into
`[hatch+0xA4, hatch+0xA8]`, then used as a 1-based index into a 99-entry
runtime table at `0x004C56E8` (`index*8 + 0x004C56E8`, one `double` per
entry) to fill `hatch+0xB8`/`+0xBC`.

That table's one writer, `0x0042609D` (also re-disassembled in full,
byte-exact including the loop bound `cmp [...], 0x63` = 99 and the store
`fstp [ecx*8 + 0x4C56F0]`), opens the file named by the string at
`0x004BF6B0` — read directly from the image: **`Compiled\ReleaseRate.pc`**.
The writer's base (`0x4C56F0`) sits 8 bytes after the reader's base
(`0x4C56E8`): index 0 of the write loop lands on `0x4C56F0`, which the
1-based reader reaches at rate 1 (`0x4C56E8 + 8 = 0x4C56F0`) — `0x4C56E8`
itself is never written, a genuine dummy slot.

`src/assets/LevelObjects.cpp::findHatchRecords`'s **own existing comment**
(landed before this round) already names the exact three `u32` fields read
right after count/X/Y in the `.LVL` stream as `hatch+0xA0`/`+0xA4`/`+0xA8` —
this round is the first to attach meaning to them (preferred rate, min,
max). Reading them straight out of the real `MM_CLIMB_002.LVL` (extracted
read-only via `lrtool extract`; independently re-parsed from the raw bytes,
not via the worker's own script):

```
hatch record: type=0 count=10 x=32 y=42  rate=20 min=1 max=50
```

and `RELEASERATE.PC` (independently re-parsed, 99 doubles, 792 bytes):

```
rate 20 = 2.8864866275534937 s
```

against D1a's independently measured mean gap **2.8867 s** (10 releases on
the same level) — agreement to `0.0002 s`, an order of magnitude inside the
capture's own noted tolerance, and clearly discriminating against the
neighbouring rows (`2.94`, `2.83`). This also matches the number already
sitting in `src/sim/World.h`'s own comment ("...2.889 s... a level whose
table entry is 2.8865 s" — the same level, now traced to its exact
mechanism instead of only cited).

## What is still unmeasured

`1.3 s < 2.020 s` (D1a's measured first release): the remaining `~0.72 s` is
presumably however long the door-opening animation itself takes before the
hatch arms — the sub-object at `hatch+0x44` (vtable slots `0x44`/`0x48`) was
read to exist but its own duration was not measured. This round also did not
check whether an unclamped `hatch+0xA0` ever falls outside `[min, max]` on
any level besides `MM_CLIMB_002` (where it already sits inside the clamp).

## Facilitator check (2026-10-21)

Independently re-disassembled every cited function from the installed
executable (`read_steppers.py --at`, a tool this round's own scripts also
build on, but the dumps below are fresh, direct reads, not a re-run of the
worker's scripts), and independently re-parsed both extracted asset files
with fresh code:

- **`0x00425A95` (296 bytes), `0x00425DE3`, `0x0042609D`**: re-disassembled
  in full. `0x00425DE3` and `0x0042609D` match the round's byte/address
  citations **exactly**, including the loop bound (`0x63` = 99) and the
  exact store address (`0x00426116`). `0x00425A95` matches on every
  substantive point (the arm gate, the `1.3` seed at `0x004B0F48`, the `dt`
  subtraction, the `0.0` compare at `0x004B0338`, the ctor call and its
  three pushed args, the `+0xB8` rollover-add) but has several minor
  citation slips worth recording as narrowings, not retractions: the
  "`add ax, 1`" instruction's cited bytes (`66 83 c0 01`) do not match the
  actual bytes at that address (`66 05 01 00`) — same semantic effect
  (`add ax, 1`), different encoding; the `je 0x425b5c` after the door-check
  call is actually at `0x00425b21`, not the cited `0x00425b1f` (which is
  `test eax, eax`); the second half of the `1.3` store cites `[edx+0xc4]`
  where the register is actually `eax`; and the `hatch+0x98` decrement at
  "`0x00425bbd`" is really three instructions (load, `sub cx,1`, store)
  spanning `0x00425bbd`-`0x00425bcb`, not one memory-operand `sub`. None of
  these affect the two load-bearing claims (the `1.3` seed and the `+0xB8`
  rollover tracing to `RELEASERATE.PC`). Separately, the round's parenthetical
  reading of the `0x00425bd4 jle 0x425d36` branch ("0 means this was the
  last; skip the ctor call") does not hold up under close reading — the
  tested value (`edx`, loaded at `0x00425bb3`) is the same, unmodified
  `hatch+0x98` already proven `> 0` by the earlier gate at `0x00425b7b`, so
  this particular branch cannot fire via this path in the traced control
  flow; recorded as an open, unexplained detail rather than corrected to a
  specific alternative.
- **The filename string** at `0x004BF6B0`: read directly from the file —
  `Compiled\ReleaseRate.pc` (null-terminated), exact match.
- **The `1.3` and `0.0` constants**: confirmed byte-exact IEEE-754 doubles at
  `0x004B0F48` and `0x004B0338` respectively.
- **`RELEASERATE.PC` row 20**: independently re-parsed the extracted file
  with fresh `struct.unpack` code (not the worker's `parse_hatch_rate.py`):
  `2.8864866275534937`, exact match.
- **`MM_CLIMB_002`'s hatch tail fields**: independently re-parsed the
  extracted `.LVL` at the cited file offset with fresh code, using
  `findHatchRecords`'s own pre-existing field-offset comment (landed in an
  earlier round, not written for this one) to locate `type`/`count`/`x`/`y`
  and the three tail `u32`s: `type=0 count=10 x=32 y=42 rate=20 min=1 max=50`
  — exact match, and the record's seven leading doubles satisfy the
  locator's own box-shape filter (confirms this is genuinely the record the
  existing parser would find, not an arbitrary offset).
- `git status --short` shows only `docs/tasks.json`'s permitted fields
  changed; no live game process was started this round
  (`stock_starts_consumed: 0`, nothing to restore); no production source
  file (`src/sim/World.*` included) was touched — `proposed-world-patch.md`
  is sketched and unapplied.

Accepted as a landed static finding, with the citation corrections above.
D1's dominant first-tick divergence (D1a) is now traced to a specific,
numerically-confirmed mechanism; the remaining open piece (the door-opening
animation's own duration) is the natural next D1 round.
