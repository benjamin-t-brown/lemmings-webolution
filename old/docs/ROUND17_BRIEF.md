# Round 17 brief — SpeedPad (Round 9 gadget repeat)

This file is a **self-contained prompt**. Paste everything below the rule into a
fresh session. A facilitator owns `PLAN.md` §1.6, `docs/PHASE3_HANDOFF.md`, and
`.claude/rules/` — do **not** edit those; report proposed ledger text instead.

Written 2026-10-16, after rounds 1–16 closed. Tree green at 16/16. Anti-grav,
trampoline, water/acid drown, and teleport are done; this is the next open
gadget.

---

You are picking up Phase 3 Round 17 of Lemmings Webolution at
`C:\progs\lemmings-webolution`, a 1:1 reimplementation of Lemmings Revolution.
**The stock 640x480 executable is the authority.**

## Facilitator ownership

**Must not touch:** `PLAN.md`, `docs/PHASE3_HANDOFF.md`, `.claude/rules/*`.
Park prose in `docs/scratch/phase3-round17-speedpad.md`. Propose a `PLAN.md`
§1.6 bullet in your final message.

**You own:** scratch note, sim/asset/harness/tests in scope.

**One agent owns the running game — you.** Restore every live write.

## Read first

| File | What it gives you |
| --- | --- |
| `.claude/rules/match-the-original.md` | evidence standard |
| `.claude/rules/windows-msys2-build.md` | build + PATH traps |
| `docs/PHASE3_HANDOFF.md` | rounds table; gadget pattern |
| `docs/scratch/phase3-round13-trampoline.md` | `pressure_pad.a3d` clean negative for bounce |
| `docs/scratch/phase3-round16-teleport.md` | object AABB / cue-string pattern |
| `PLAN.md` §1.6 | ledger (read-only) |

## Your question — one mechanic

**What does a SpeedPad do to a lemming, and how does the engine detect it?**

Settle that and stop. Do **not** take Balloon, Fire, Witch, Lever, or other
gadgets.

### Not this round

- Do not remeasure trampoline / teleport / drown / anti-grav / exit.
- `pressure_pad.a3d` was a clean negative for trampoline (round 13). Do not
  reopen that claim unless SpeedPad evidence forces it — and if so, say so
  explicitly without rewriting trampoline behaviour.

### Leads (observations, not conclusions)

- Cue string **`SpeedPad`** at **`0x004C0D10`** in the stock exe `.data` cue
  / name table (neighbours: `AntiGrav`, `Action`, `Balloon`, `Fire`,
  `Witch`). No `"speed_pad"` / `"speedpad"` ASCII elsewhere in a quick image
  search.
- Level assets: **`speedup_two_way.a3d`** in **9** retail COMPILED levels
  (16 placements; facilitator census):
  ```
  H08_MT_1 H10_MT_1 H15_MT_1 H17_MG_1 M21_KF_1 SHOW_MT3 VH06_MT VH16_MT VH21_JC
  ```
  Strongest model-name lead for this cue. Re-run your own census.
  Co-occurrence with `pressure_pad.a3d` on some of those levels is not
  identity.
- **`pressure_pad.a3d`** appears in many levels (46) and was **not** the
  trampoline source on H01_KF_1. It may or may not relate to SpeedPad —
  treat as a candidate to falsify or confirm, not as settled.
- Detection may be cell bit, object AABB (`0x00430BA1` still has unread
  call sites), a dedicated list, or a state attach that changes walk speed
  temporarily. Do not invent classic Lemmings conveyor rules.
- Cue leftovers next to SpeedPad (`Balloon`, `Fire`, `Witch`, `ForceF`,
  `Changeop`) are **out of scope** unless the SpeedPad path lands there.

### Shape to expect

Wrong shape is common. Expect a temporary walk-speed multiplier, a one-way
boost, a facing lock, a state strip, or "cue only / FX only". A clean
negative ("string is UI-only; behaviour is elsewhere") is a result — but
then name what *does* own `speedup_two_way.a3d` if anything.

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
`drownMs=2000`. Teleport object AABB / `teleportDelayTicks=4`. Exit AABB
2×3, column `round`. Blocker `0x800`. Do not move settled fields; stop and
report if a finding seems to require it.

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

1. `docs/scratch/phase3-round17-speedpad.md`
2. Sim changes for what you settled, MEASURED/READ + addresses
3. Tests that fail when reverted alone; suite 16/16
4. Do **not** update PLAN/handoff/rules — propose the §1.6 bullet
5. Process state + every live write restored

**Report honestly.** Do not round, smooth, or over-claim.
