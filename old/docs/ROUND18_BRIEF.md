# Round 18 brief — Balloon (Round 9 gadget repeat)

This file is a **self-contained prompt**. Paste everything below the rule into a
fresh session. A facilitator owns `PLAN.md` §1.6, `docs/PHASE3_HANDOFF.md`, and
`.claude/rules/` — do **not** edit those; report proposed ledger text instead.

Written 2026-10-16, after rounds 1–17 closed. Tree green at 16/16. Anti-grav,
trampoline, water/acid drown, teleport, and SpeedPad are done; this is the next
open gadget.

---

You are picking up Phase 3 Round 18 of Lemmings Webolution at
`C:\progs\lemmings-webolution`, a 1:1 reimplementation of Lemmings Revolution.
**The stock 640x480 executable is the authority.**

## Facilitator ownership

**Must not touch:** `PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`.
Park prose in `docs/scratch/phase3-round18-balloon.md`. Propose a `PLAN.md`
§1.6 bullet in your final message.

**You own:** scratch note, sim/asset/harness/tests in scope.

**One agent owns the running game — you.** Restore every live write.

## Read first

| File | What it gives you |
| --- | --- |
| `.claude/rules/match-the-original.md` | evidence standard |
| `.claude/rules/windows-msys2-build.md` | build + PATH traps |
| `docs/PHASE3_HANDOFF.md` | rounds table; gadget pattern |
| `docs/scratch/phase3-round17-speedpad.md` | cue-string → object → parallel-state pattern |
| `docs/scratch/phase3-round11-exit.md` / round 12 | exit models — include `balloon_*` **decorations** |
| `PLAN.md` §1.6 | ledger (read-only) |

## Your question — one mechanic

**What does a Balloon do to a lemming, and how does the engine detect it?**

Settle that and stop. Do **not** take Fire, Witch, Lever, ForceF, or other
gadgets.

### Not this round

- Do not remeasure SpeedPad / teleport / drown / trampoline / anti-grav / exit
  save AABB.
- Exit mesh paths like `models\exit\balloon_default_corded` and
  `balloon_water` / `balloon_any_*` / `balloon_acid` are **exit decorations**,
  not this gadget. Do not reopen the exit save test.

### Leads (observations, not conclusions)

- Cue string **`Balloon`** at **`0x004C0D24`** in the stock exe `.data` cue
  table (neighbours: `Action`, `Fire`, `Witch`; after `SpeedPad`).
- Registration mirrors SpeedPad: `push 0x4c0d24` at `0x0045E6F0` into the cue
  registry; stored on a large object at **`+0x1c8f4`**.
- Imm32 refs to offset `0x1c8f4`: **`0x0045E6EC`** (write at register),
  **`0x0045EB15`**, **`0x0045F15E`** — the latter two are the inventory for
  "who plays / reads the Balloon cue". Re-disassemble them yourself.
- **No `balloon.a3d`** (or similar) in COMPILED levels / dumped `.a3d` files
  (facilitator census). Detection may be a differently named model, a cell
  bit, an object vtable without a balloon filename, a skill/state strip, or
  cue-only with no world gadget. Prove which.
- Substring `balloon` in `.LVL` data is dominated by **exit** model paths —
  do not treat those as Balloon-gadget placements.
- Shape unknown: lift, float, temporary rise, attach parallel state, kill,
  or clean negative ("cue registered, never played in retail"). A clean
  negative is a result — then say what you searched.

### Shape to expect

Wrong shape is common. Do not invent classic Lemmings ballooner rules.
Expect anything from a SpeedPad-like parallel state to "string only".

## Evidence standard

1. Address + bytes, or re-runnable script.
2. Predictions **before** each live run, with falsifier.
3. Actor list `0x004CBB18` only — never billboards.
4. `.data` citations inside section rawsize `0x7000` only.
5. **One cell is one world unit.** Never multiply by `PIXELS_PER_UNIT`.
6. No classic Lemmings; no halving classic values.
7. Clean negative is a result.
8. Preserve quirks.
9. Read back the loaded path after `--enter` before trusting a dump.

## Settled — do not redo

Anti-grav `0x8000`. Trampoline `0x1000`. Water/acid drown `0x40` /
`drownMs=2000`. Teleport object AABB / `teleportDelayTicks=4`. SpeedPad
`fasterMs=5000` / `+0x50=2`. Exit AABB 2×3, column `round`. Blocker
`0x800`. Do not move settled fields; stop and report if a finding seems to
require it.

## Build and test

```powershell
.\scripts\Invoke-Ucrt64.ps1 "cmake --build --preset dev-debug"
.\scripts\Invoke-Ucrt64.ps1 "ctest --preset dev-debug"
```

No UCRT64 `bin` on PATH for harness Python. Mutate each new constant
**alone**, behavioural fail, restore (**touch** source), suite 16/16.

## Must not touch

`PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`,
`test_collisiongrid.cpp`, `test_corpus.cpp` and its expected numbers,
settled `Rules` fields.

## Deliverables

1. `docs/scratch/phase3-round18-balloon.md`
2. Sim changes for what you settled, MEASURED/READ + addresses
3. Tests that fail when reverted alone; suite 16/16
4. Do **not** update PLAN/handoff/rules — propose the §1.6 bullet
5. Process state + every live write restored

**Report honestly.** Do not round, smooth, or over-claim.
