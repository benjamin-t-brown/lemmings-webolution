# B1q: archive registration order read; boot ordering before the font load still open

2026-10-23. Generic texture-loader question, round 2 of 3 (round 1 is
[B1p](phase-b1p-texture-loader.md)). Packet: `build/tmp/b1q-archive-open/`
(`round-note.md`, `decision-table.md`, `prediction.md`, `raw-pins.json`, `commands.md`,
individual `rea-*.json`). Zero stock starts, no helper under `tools/`.

## Result (READ, not observed execution)

**Partial. Not a unique-source landing.**

- `0x47E23B` registers three archives, appended in this order: `PATCH.BOX` (`0x4C2A54`),
  `LEMMINGS.BOX` (`0x4C2A60`), then `LEMMINGS.BOX` under registry `CDPath` (`0x4C2A70`).
  The brief's hypothesis (lemmings first) was wrong on order.
- `0x47DD54` registers nothing: it builds a stack-local list and destroys it.
- `[0x5D5604]` has two readers (`0x47E3A6`, `0x47E434`) and no writer among exact
  references, so the non-archive route is not taken by default. Computed-pointer writes are
  not excluded.
- `patch.box` has no `TEXTURES\^FONT.*` member, so patch registering first does not change
  the `^Font` result: if the loose probe `0x47E89B` fails and the manager is populated, the
  first hit is a `lemmings.box` member. The `CDPath` copy on mouv
  (`C:\Users\bbrown\Downloads\Lemmings Revolution`) has the same SHA256 as the install copy,
  so the bytes are the same either way. B1p's convergence is **not falsified**.

## Still open

- Whether `0x47E23B` has run before the font load at `0x45019E`. Callers: `0x450771`,
  `0x45BEA7` (unread) and `0x47E576` (retry tail of `0x47E40D`, needs the not-found flag).
- Unread helpers: `0x47F230` (gate on `0x47E23B`), `0x47E105` (base directory),
  `0x47E30C` (registration condition), `0x47EF48` (REA decompile empty), `0x47EF92`,
  `0x49DD81` and the runtime cwd (loose-file probe).

## Process notes

11 bodies against a cap of 10; the extras (`0x47F130`, `0x47F200`, `0x47FAF0`) were not
needed. No canonical REA evidence bundle was exported, only individual outputs. Install
hashes were taken once at the end, not before and after. These are gaps against the brief,
recorded rather than waived. `prediction.md` predates the reads (10:39:58 against first
pins at 11:03).

## Facilitator check

Re-read `0x47E23B` from the exe: the three string pushes in order, each followed by
`0x47E069`. Scanned `.text` for `[0x5D5604]` operands: only `0x47E3A6` and the `or` at
`0x47E435`. Scanned for calls to `0x47E23B` (three callers), `0x47F230` (one) and
`0x47E89B` (one). SHA256 of both `lemmings.box` copies equal; registry `CDPath` read on
mouv. The packet's five pins equal the exe. Install hashes equal startup; no game process.
Not re-read: `0x47DD54`'s list handling, `0x47E40D` internals, `0x4805B8`.
