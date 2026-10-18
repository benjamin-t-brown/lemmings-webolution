# Round 29 brief — splat exemption (zone bits `0x200` / `0x4000`)

This file is a **self-contained prompt**. Paste everything below the rule into a
fresh session. A facilitator owns `PLAN.md` §1.6, `docs/PHASE3_HANDOFF.md`, and
`.claude/rules/` — do **not** edit those; report proposed ledger text instead.

Written 2026-10-16, after rounds 1–28 closed. Tree green at 16/16. Queue after
this: inverted steppers (30), cell-flag writers (31). **One mechanic: splat
exemption only.**

---

You are picking up Phase 3 Round 29 of Lemmings Webolution at
`C:\progs\lemmings-webolution`, a 1:1 reimplementation of Lemmings Revolution.
**The stock 640x480 executable is the authority.**

## Facilitator ownership

**Must not touch:** `PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`.
Park prose in `docs/scratch/phase3-round29-splat-exempt.md`. Propose a
`PLAN.md` §1.6 bullet in your final message.

**You own:** scratch note, sim/asset/harness/tests in scope.

**One agent owns the running game — you.** Restore every live write.

## Read first

| File | What it gives you |
| --- | --- |
| `.claude/rules/match-the-original.md` | evidence standard |
| `docs/scratch/phase3-round3b-float-splat.md` | splat path / floater exemption |
| `docs/scratch/phase3-round9-antigrav.md` | hatch type → `[+0x4c]=1` |
| `docs/scratch/phase3-round14-water.md` | Entrance_water ∩ `0x200` ≠ drown |
| `PLAN.md` §1.6 / cell-bit table | stamp sites; zone naming |
| `src/assets/CollisionGrid.h` | zone bit names |

## Your question — one mechanic

**Which objects stamp zone bits `0x200` / `0x4000`, what do lemming
`[+0x4c]` values 1 and 2 mean for landing, and does a matching zone skip
splat?**

Settle that and stop. Do **not** take inverted steppers or cell-flag writers.

### Leads (observations, not conclusions)

- Landing path: `and 0x200` / `cmp [eax+0x4c], 1` at **`0x0041662B`** and
  `and 0x4000` / `cmp [edx+0x4c], 2` at **`0x00416641`** — both jump to the
  safe-landing target (same as floater exemption arm). Prove live that a
  long fall onto a matching zone lands walk instead of splat.
- Zone stamp sites (PLAN table): `0x00428D63`, `0x00428D84`, `0x00428DA5`,
  `0x0042AFA3`, `0x0042AFC4`, `0x0042B395`, `0x0042B3B6`, `0x0042B4AB`,
  `0x0042B4CC` — object type field `+0x128` = 0 / 1 / 2 → bits
  `0x200` / `0x2000` / `0x4000`. Census which a3d / object types stamp which.
- Hatch type 1 → live `[+0x4c]=1` (round 9). Find what sets `[+0x4c]=2`.
- **Not** water drown (`0x40`). Entrance_water correlates with `0x200` but
  drown is a separate mechanic (round 14).

## Evidence standard

Same as prior rounds. Actor list `0x004CBB18`. No `PIXELS_PER_UNIT`.
Mutate-alone + touch + suite 16/16. Windows build via
`scripts/Invoke-Ucrt64.ps1`. Do **not** put UCRT64 `bin` on PATH for
`tools/harness/*.py`.

## Settled — do not redo

Cue/gadget rounds through Die (28). Floater `[+0x49]` exemption. Water
drown `0x40`. Anti-grav `0x8000`. Do not move `splatDistance` unless
evidence forces it.

## Deliverables

1. `docs/scratch/phase3-round29-splat-exempt.md`
2. Harness (`measure_splat_exempt.py` or extend an existing measure script)
3. Sim only if live proves exemption (wire zone + `[+0x4c]` into faller land)
4. Tests + suite 16/16
5. Propose §1.6 bullet only
6. Process / restores clean

**Report honestly.** In your final message include: (a) verdict, (b) key
addresses/evidence, (c) proposed PLAN §1.6 bullet, (d) suite status,
(e) files changed.
