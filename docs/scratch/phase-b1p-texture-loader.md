# B1p: generic texture loader read; `^Font` convergence is conditional

2026-10-23. New question (the loader shared by every texture), round 1 of 3, opened by
the active directive. Worker packet: `build/tmp/b1p-texture-loader/` (`round-note.md`,
`decision-table.md`, `raw-audit.json`, `rea-evidence.json`). Helper:
`tools/harness/read_texture_loader_b1p.py`. Zero stock starts.

## Result (READ, not observed execution)

**Not landed as a unique source.** Inside the inspected bodies the answer is a
conditional convergence; the actual stock selection is not closed.

- `0x47244B` is a case-sensitive substring test for `INN_` then `OUT_` through
  `0x49CE20` (literals `0x4C2548` / `0x4C2550`). `^Font` matches neither.
- So at `0x472223`: `[0x5CD5F2] == 0` tries `^FontLo`; nonzero tries unsuffixed `^Font`.
  `^FontHi` is unreachable for the literal name `^Font`. A failed first probe at
  `0x4722B6` resets to the original name (`0x4722EB`).
- `0x43B2EE` copies its argument into `[0x5CD5F2]`; sole direct caller is `0x443789`
  (via `0x4432C1`), from `0x47099B(0x58CD80)` and stream inputs. Not traced; no stock
  value of the byte is asserted.
- A flagged name (`^Font` is flag 8) also opens `Textures\<W>.alf` and fails fatally if
  absent, so the source is a BMP **and** an ALF companion.
- Archive inventory (both name tables walked): `lemmings.box` holds
  `TEXTURES\^FONT.BMP` (ordinal 633) and `TEXTURES\^FONT.ALF` (632); no `Lo`/`Hi`
  member in either archive; `patch.box` holds `^FONTP` and `^FONTH` (different names), and
  no `^FONT`. No loose `Textures\` directory on the install. Under "two archives, no
  loose override" both quality cases converge on the base pair.

## What is still open

`0x47E40D` has a non-archive route gated by byte `[0x5D5604]` and `0x47E89B(path)`; the
initial `0x47DD54`, the manager `[0x5D5608]` population and order, and the `0x4805B8`
table parser are unread. `0x47F759` takes the **first** registered archive with a
matching name, so patch-wins is not proved. The convergence therefore depends on the
premise and is not a unique-source claim. Directive question 2 (render) stays
unstarted.

## Facilitator check

Re-ran the worker's helper from the Mac against the mounted install; `raw-audit.json`
equals the worker's field for field. Own regex over both archives found the same
`^FONT` members. Re-read `0x47244B`, `0x49CE20`, `0x472210` and `0x4C24E8` /
`0x4C2548` / `0x4C2550` from the exe (`Lo`, `Hi`, `INN_`, `OUT_`). Isolated mutation
(`Lo` to `Hi` on the zero branch) fails the helper's controls; unmutated passes 5/5.
Install hashes equal the startup record; 30/30 CTest. I did not re-read `0x47E40D`,
`0x45015A`, `0x47F759`, `0x4432C1`, `0x443789`; those rest on the worker's pins.
