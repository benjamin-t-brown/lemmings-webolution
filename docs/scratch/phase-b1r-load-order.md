# B1r: registration precedes the `^Font` request on the one chain read; loose route unread

2026-10-23. Q1 (generic texture loader) round 3 of 3, the cap. Packet:
`build/tmp/b1r-load-order/` (`round-note.md`, `prediction.md`, `decision-table.md`,
`raw-pins.json`, `pins.py`, `hashes-before.txt` / `hashes-after.txt`, per-call `rea-*.json`).
Zero stock starts, no process.

## Result (READ, not observed execution)

**Partial; no unique-source claim.**

- `^Font` is pushed at `0x4403B5` (`push 0x4BFF68`, the string `^Font`) in `0x4403A7`, which
  calls constructor `0x440404`. `0x4403A7` has one caller (`0x486C4E`, in `0x486BD8`), and
  `0x486BD8` has one (`0x486F88`, in `0x486F60`).
- Startup `0x450696` (called from `entry` `0x49D518`) calls the registrar `0x47E23B` at
  `0x450771` and later calls `0x486F60` with 7 (`0x450A46`). In straight-line order,
  registration precedes the `^Font` request on this chain.
- Not proved: the registrar body is skipped when `[0x5D5610] == 0` (via `0x47F230`, `[ecx+8]`
  of manager `0x5D5608`) and nothing read sets that field; the link from `0x414B90` (the
  constructor's call) to the load at `0x45019E` is unread; other entries to `0x486F60` and the
  path from `0x45BEA7` are unread.
- Loose route: the only helper read is `0x47E30C`, an archive validity check (`LEMBOX`, six
  bytes), not a loose route. `0x47E89B`, `0x49DD81`, `0x47E105`, `0x47EF48`, `0x47EF92` and the
  working directory (`0x450600`, `0x4A8EF4`) are unread. Reachability on stock rests on B1p's
  observation that the install has no `Textures\` directory.

Combined with B1p/B1q: `lemmings.box` `TEXTURES\^FONT.BMP` + `.ALF` is the source only if the
gate is nonzero, a loose probe misses, and this chain is the path taken. **Q1 stays
conditional at the cap; Q1 is parked, Q2 is not started.** Nothing was chosen by score.

## Process notes

36 minutes against 35; about 11 new bodies against 12. No canonical REA bundle (per-call JSON
only), no mutation test, no helper beyond `pins.py`. Hashes were taken before other work.
`prediction.md` is timestamped before the first body.

## Facilitator check

Re-read from the exe: `0x4403A7` (`push 0x4BFF68` at `0x4403B5`, `call 0x440404`; the string at
`0x4BFF68` is `^Font`), `0x47E23B` (`mov ecx,0x5D5608; call 0x47F230; test; jne; jmp 0x47E2EF`),
`0x47F230` (returns `[ecx+8]`) and the call at `0x450771`. My own call scan found one caller of
`0x4403A7` (`0x486C4E`), one of `0x486BD8` (`0x486F88`), three of `0x47E23B` (`0x450771`,
`0x45BEA7`, `0x47E576`) and seven of `0x486F60`, including `0x450A46` with `push 7` at
`0x450A38`. I did not confirm that nothing branches between `0x450771` and `0x450A46`, did not
read the gate's setter, `0x414B90`, or any loose-route helper. Exe and `dinput.dll` re-hashed
on mouv equal the startup record; no game process.
