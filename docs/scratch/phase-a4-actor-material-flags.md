# A4t: actor material flags measured live, `0x2` on both sheets (A4r row 3)

Queue `a4t-actor-material-flags`; one live, read-only capture plus a reversible paused
graphics fixture. Authority SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Accepted packet: `build/tmp/a4t-actor-material-flags/round-note-repair2.md`, raw
`capture-a4t-repair2-run1/report-repair2.json`, reader `capture_flags_repair2.py`,
session-1 launcher `run_capture_repair2.ps1`. Chain reads: `resolver-reads-repair1.txt`.

## Accepted result

On `MM_CLIMB_002` (pid 25236, session 1, `ui_state 1`, level path recorded before any
actor read), the resolved material descriptor `+0x24` flags are **`0x2`** for every
actor read: two five-direction walkers and one controlled eight-direction
block-graphics fixture. Those are **two distinct descriptors**, one per sprite sheet:

| Fixture | action / temporal / dir / count | handle | descriptor | sheet | flags | selector |
| --- | --- | --- | --- | --- | --- | --- |
| five, actor A | 13 / 9 / 1 / 5 | 117 | `0x27D14E0` | `Sheets\$LemSheetHi15` | `0x2` | 0 |
| five, actor B | 13 / 8 / 2 / 5 | 120 | `0x27D13A0` | `Sheets\$LemSheetHi12` | `0x2` | 0 |
| eight, actor A | 1 / 0 / 7 / 8 | 120 | `0x27D13A0` | `Sheets\$LemSheetHi12` | `0x2` | 0 |

Handles, pointers and actor addresses are session-local. The eight fixture resolves
to the same descriptor as five actor B, so it is not an independent descriptor
observation. Every row read the same values before the eight write and after its
restoration.

Through [A4r's ordered table](phase-a4-actor-render-state.md#material-states), `0x2`
fails `and 8` (`0x47267D 83e008`) and `and 0x10` (`0x47272A 83e210`), and passes
`and 3` (`0x47283F 83e103`). That selects **row 3**: alpha `SELECTARG1(TEXTURE)`,
alpha test enabled, GREATER, reference from `[0x4C24E0]`. That reference is
`0xA0` both in the file's `.data` and in this run's process memory. The RGB path is
A4r's `MODULATE(TEXTURE, DIFFUSE)`. Selector 0 is A4r's blend-disabled encoding
(no SRC/DEST assignment).

What this is **not**: effective device state. The flags cache `[0x4C24E4]`
(`0x47265C..0x472665`) skips setup when it equals the new flags. Its value changed
between snapshots (8, then 0) and was never `0x2` when read, but no draw-time read
was made. The raster residual, other actions/levels, and why the cache changed are
not addressed.

## Chain (READ, original bytes)

- Submit object is the embedded `actor+0x128` (A4s); handle at submit `+0x1A8`.
- `0x414934 b948cb5800`: `mov ecx, 0x58CB48`, so the manager is the object at
  `0x58CB48`, not a pointer to it.
- `0x414BA0 83c130` + `call 0x414D00`: the pool is manager `+0x30` (`0x58CB78`).
- `0x414D0A 69c088000000`, `0x414D13 8b510c`, `0x414D16 03c2`: entry = handle·`0x88`
  + `[0x58CB84]`.
- `0x414C34 8b8280000000`, `0x414C3A 8b00`: descriptor = `[[entry+0x80]]`.

## Measured links (this run)

Pool base `[0x58CB84]` = `0x0FF09464`. For each row, the submit handle equals the
selected direction record's `+0x40`. Each actor was read twice while the process
was suspended (`NtSuspendProcess`), and both reads matched. The raw bytes for every
link are in the report.

## Facilitator check

- **Independent recompute:**
  `python3 build/tmp/a4t-actor-material-flags/facilitator_check.py` trusts only raw
  byte fields and addresses. It recomputes submit, handle/record equality, the pool
  entry, wrapper, descriptor, flags and the A4r row for all five rows: **PASS**.
- **Prediction timing:** `prediction-repair2.md` (23:35:30) predates the task start
  (23:35:46).
- **Byte rereads:** reread on the stock exe with `read_steppers.py`:
  - `0x414934`, `0x414B98..0x414BA8`, `0x414C30..0x414C3C` and `0x414D00..0x414D1B`;
  - the three flag tests and the flags-cache compare at `0x47265C..0x472682`,
    `0x47272A..0x472735` and `0x47283F..0x47284A`;
  - `.data` `0x4C24E0` = `a0 00 00 00`.
  
  The A4r checker passes (34 setter tuples, 21 byte anchors, 3 SDK offsets,
  20 enums).
- **Mutation:** no production behavior changed, so no source or pixel mutation
  applies.
- **Suite:** build plus CTest **20/20** on mouv.
- **Live side:** a separate SSH call found no Lemmings process and no `LemWeb-*`
  task. The exe, `SavedGame.dat` and `dgVoodoo.conf` hashes equal the run's
  before-hashes. Three pre-existing `frida-helper-*` processes were left untouched.
  Fixture writes were restored while suspended.

**Narrowed from the worker packet:**

1. **Write ledger.** The worker's summary says it wrote only `+0x1C8` and `+0x1CC`.
   The ledger also shows restoration writes to `+0x1B8` (7→1, after the game itself
   moved it 1→7) and `+0x1B4` (1→1). All four are brief-permitted fields and were
   restored.
2. **Eight fixture.** It shares five actor B's descriptor and was formed by setting
   action/temporal only; direction 7 and count 8 are the game's own values.
3. **Repair 1's entry failure.** The cause is not separated: a concurrent second
   game and the added per-edge foreground acquire are confounded.

**Evidence discarded:**

- **First packet (`round-note.md`, overwritten).** Its session-0 diagnosis was
  wrong: its own run 1 log shows a correct session-1 launch, and the real defect
  was dereferencing `actor+0x128`. Its second run launched the game from the SSH
  shell (session 0), which breaks the rule; that is not a result.
- **First worker's later artifacts.** After it was told to stand down, that worker
  kept running and wrote `round-note-final.md`, `result-final1.json`,
  `capture_a4t_complete.py` and `capture_a4t_verified.py`. Those reads were never in
  a level (`ui_state 3`, empty path), and its "manager" `0x004B19E8` and "pool"
  `0x00471720` are not the chain. It also drove a second game during repair 1's
  start. None of its values are used.
