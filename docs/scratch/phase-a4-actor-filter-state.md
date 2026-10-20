# A4x: actor sprites draw under POINT filtering with alpha reference `0xEF`

Queue `a4x-actor-filter-state`. Static reads plus one live, read-only run on
`MM_CLIMB_002`:
- one owned game start, session-1 task `LemWeb-a4x-run1`;
- a `0xC10` probe handle (read, query, suspend);
- no debugger.

The only memory write is the established `enter_fast.py` level-record swap (restored).
Worker packet: `build/tmp/a4x-actor-filter-state/round-note.md`. The prediction
`prediction.md` was written before the launch. Raw report: `capture-run1/report.json`.

## Result

- **READ, the bracket.** The actor submit function `0x47A75D` (`ret 0x10` at `0x47AB93`)
  wraps its draw in two filter calls:
  - `0x47A766 cmp [0x58D53C],0 ; jne` gates `push 0 ; mov ecx,0x58CD80 ; call 0x470F29`
    at `0x47A776`;
  - the draw is `0x47AB7F call 0x47AB96`;
  - then comes the unconditional `push 1 ; call 0x470F29` at `0x47AB8B`.

  No `0x470F29` call sits between them.
- **READ, what that sets.** With argument 0, `0x470F29` pushes filter value 1 to stage-0
  TSS `0x10`/`0x11` and writes `[0x4C24E0]` = `0xEF`, whatever `[+0x3A1]` holds. The
  post call (argument 1, `[+0x3A1]` = 1) pushes 2 and writes `0xA0`. In D3D7 numbering
  that is POINT, then LINEAR (SDK inference).
- **READ, the draw path.** `0x47AB96` calls `0x414920` (at `0x47ABAF`), which calls both:
  - `0x47261F` (at `0x414944`): A4r's material path, holding the only reader of
    `0x4C24E0`, at `0x472896`, pushed as render state `0x18`;
  - `0x414D80` (at `0x41494F`): A4f's draw.

  So at actor draw the alpha reference is **`0xEF`** and the filter is **POINT**.
- **READ, the gate.** `[0x58D53C]` is set from `[[0x4CBFB0]+0x10]` at `0x44186B` and
  zeroed at `0x4419A0`. The object loop's `call [edx+0x10]` at `0x44190D` sits between
  them, and actor vtable `0x4B10E8` +0x10 is `0x42EEF2`.
- **MEASURED, live.** Six suspended pairs (paused, unpaused, paused at end), each pair
  identical: `[0x58D121]` (= `0x58CD80+0x3A1`) = 1, `[0x58D53C]` = 0,
  `[[0x4CBFB0]+0x10]` = 0, `[0x4C24E0]` = `0xA0`. Unsuspended sampling at ~1 kHz for
  30 s saw `0xEF` in 129 of 30,002 samples (0.43%) and `0xA0` otherwise. A4t's and A4w's
  `0xA0` is the value between draws.
- **READ, `0x4C24E0` writers.** The file holds exactly three references to the address:
  `0x470F39` (`0xEF`), `0x470F65` (`0xA0`) and the reader `0x472896`. Writes through a
  computed pointer are not excluded.
- **READ, dgVoodoo** (`dgVoodoo.conf` sha256 `9a34b1b3…ef4e`, unchanged). `[DirectX]`
  has `Filtering = appdriven` and `KeepFilterIfPointSampled = false`; `[Glide]` has
  `TMUFiltering = appdriven`. No forced filter.

Consequences:
- A4v's nearest-texel fit agrees with the original's own filter choice.
- **A4u's `> 0xA0` is the between-draw value; the in-draw reference is `0xEF`.** That is
  pixel-invisible on both fixtures, whose frames are 0/255 alpha. A code change belongs
  in its own round.

## Scope limits

- **A4z scope correction:** the passing A4x observation had zoom zero. The
  nonzero branch at `0x47A766..0x47A76D` skips the pre-draw POINT/EF setter.
  [A4z's matched-camera zoom1 fixture](phase-a4-actor-third-fixture.md) fails the
  production nearest/EF pixel test. This note does not establish zoom1's last
  inherited filter/reference or effective device state; do not generalize its
  headline to all zooms.

- **One indirect hop is not reread.** `0x42EEF2` reaches `0x43CC68` through
  `0x42EFFE call [[[this+0x340]]+0x10]`. `0x43CC68` is stored exactly once in the file,
  at `0x4B14AC`, which is consistent with that slot. Which object sits at `actor+0x340`
  was not read. `0x43CC68` then calls `0x47A75D` at `0x43CD4E` (reread).
- The live window with `[0x58D53C]` nonzero was never caught. A single `0xEF` sample
  cannot be attributed to the actor bracket rather than the other argument-0 callers
  (`0x44E165`, `0x454C89`, `0x45B6D9`).
- Device state below dgVoodoo (a GPU driver override) cannot be read from the process.
  `0x47022B`'s phase (init or frame) is unsettled. Only `MM_CLIMB_002`.

## Facilitator check (2026-10-20)

- **Own disassembly** of the stock exe (`build/tmp/a4x-actor-filter-state/facilitator/`
  `fread.py`, `fchain.py`, `fvt.py`, with outputs):
  - `0x47A75D..0x47A77B` and `0x47AB7F..0x47AB93`, the whole of `0x470F29`;
  - `0x414940..0x414944`, `0x472893..0x47289D`;
  - `0x44186B`, `0x44190D`, `0x4419A0`;
  - the call lists of `0x43CC68`, `0x47AB96`, `0x414920`, and of `0x47A75D` between the
    two filter calls (no `0x470F29`);
  - actor vtable `0x4B10E8`; the sole stored `0x43CC68` at `0x4B14AC`;
  - the `0x42EFFE` indirect site.

  All bytes match the worker's packet. The raw count of `e0 24 4c 00` in the file is 3.
- **Narrowed:**
  - the `0x42EEF2 → 0x43CC68` indirect hop (above);
  - "no live writes" in the brief meant no measurement writes. Both A4w and this run
    entered the level via the restored `enter_fast.py` swap. The A4w note is corrected
    to say so.
- **Report flags:** `task_session` 1, `install_restored` true, `process_stopped` true.
  The worker's postcheck shows install hashes unchanged.
- **Live side:** the worker's separate postcheck found 0 Lemmings processes and 0
  `LemWeb-*` tasks. My own SSH call agrees: 0 and 0.
