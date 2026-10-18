# Phase 3 round 13 — trampoline

2026-10-16. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`, driven through `tools/dinput-shim`.

Harness: `tools/harness/measure_trampoline.py` (new). Re-run:

```
python tools/harness/measure_trampoline.py --audit
python tools/harness/read_steppers.py --at 0x004B0D08 --len 0x40
python tools/harness/read_steppers.py --at 0x0041B838 --len 0x100
python tools/harness/read_steppers.py --at 0x00416433 --len 0x100
# restart the process, then
python tools/harness/measure_trampoline.py --enter MM_CLIMB_002 --paint \
        --cols 32,33 --row 31 --watch 15 --hz 80
python tools/harness/measure_trampoline.py --enter MM_CLIMB_002 --paint --pad \
        --cols 32,33 --row 31 --watch 18 --hz 80
python tools/harness/measure_trampoline.py --enter H01_KF_1 --watch-only 50 --hz 40
```

---

## SUMMARY

| Thing | Status | Value |
| --- | --- | --- |
| **detection** | **MEASURED live** | foot/landing cell bit **`0x1000`** → attach bounce vtable **`0x004B0810`** (type `0x21`) at fall land `0x00416560`, **before** splat |
| **what bounce does** | **MEASURED + READ** | parabola height `-12.5*t² + 35*t` (peak **24.5** at `t=1.4` = live `DAT_004c55a8`); X += facing×5.0×dt; anim string `"fall"` |
| **pad cell shape** | **READ + MEASURED** | trampoline enter paints `\|0x1083` then clears bit 0 → **`0x1082`** (not solid). Solid+`0x1000` still starts bounce, but bounce collision treats it as floor and exits immediately |
| **authored in .LVL?** | **MEASURED offline** | **0 / 102** grids carry `0x1000` |
| **pressure_pad.a3d** | **CLEAN NEGATIVE** | 23 levels; H01_KF_1 watch 50 s: **0** live `0x1000` cells, **0** bounce, **0** trampoline vtable |
| **trampoline state `0x004B0D08`** | **READ** | skill/state type `0xe` ("TRAMPOLINE"); enter paints pads; exit `and 0xf77c` clears `0x883`; not entered on paint-only runs |
| **sim** | landed | `Terrain::trampoline`; fall→`State::Bounce`; rise then fall |

---

## PREDICTIONS (written before live runs)

### P1 — detection bit is `0x1000`, runtime-only

**Holds.** MM_CLIMB_002 paint `0x00000001 → 0x00001001` on (32,31)/(33,31): **6** fall→bounce attaches in 15 s; every hatch faller. Restored both cells to `0x1`.

### P2 — trampoline enter paints the bit; pressure_pad may show live `0x1000`

**Partially holds / pressure_pad falsified.** Enter paint shape confirmed by matching `--pad` (`|0x1083 & ~1` → `0x1082`): **13** bounce attaches with sustained rise y 31→~40.8 then fall. H01_KF_1 (has `pressure_pad.a3d`): **0** live `0x1000` cells in 50 s — pressure_pad is **not** the trampoline pad source on that level.

### P3 — pressure_pad is only a candidate

**Clean negative** for H01_KF_1 natural play (no bounce / no trampoline state / no bit).

### P4 — bounce motion formula

**Holds as READ**, corroborated live: with pad-style cells, Y rose ~10 world units before head-hit cut the arc (pen ceiling). Open-air peak from formula is 24.5 at t=1.4.

---

## Static reads

| claim | address | bytes / observation |
| --- | --- | --- |
| trampoline vtable | `0x004B0D08` | `0x41bbe0` / `0x41ba81` / `0x41b838` / `0x41b683` |
| enter `"trampoline"` | `0x0041B856` | `push 0x4bf634` |
| enter paint `\|0x883`×4 / `\|0x1083` | enter loops | `"Trampoline Block..."` xref `0x41ba44` |
| exit `and 0xf77c` | `0x0041B783` | `81 e2 7c f7 00 00` (= `~0x883`) |
| bounce vtable | `0x004B0810` | ctor `0x40ec7e`, type `0x21` |
| fall `0x1000` → bounce | `0x00416560` | `81 e2 00 10 00 00` |
| bounce `0x1000` re-hit | `0x0040f309` | `25 00 10 00 00` (needs `t > 1.4`) |
| ground skip `0x1000` | `0x0042e9e3` | `FUN_0042e913` |
| skill name TRAMPOLINE→`0xe` | `0x00459f8b` | `mov …, 0xe` ; `push 0x4c08c0` |
| bounce threshold | live `0x004c55a8` | double **1.4** (BSS; not in `.data` rawsize) |
| bounce peak twin | live `0x004c5598` | double **2.8** |
| `.data` debug strings | `0x4bf150`…`0x4bf640` | inside rawsize `0x7000` (to `0x004C6000`) |

`FUN_0041B683` remains trampoline exit only — no direct `e8` in `.text` (round 5).

---

## Live series

### Offline audit

```
levels 102
pressure_pad.a3d: 23
any cell & 0x1000: 0
```

### MM_CLIMB_002 — paint bit only (`0x1001`)

Loaded path `Compiled\MM_CLIMB_002.lvl`. Six bounce attaches; each bounce→walk within ~15 ms (solid floor wins bounce collision before `0x1000` re-hit). Cells restored.

### MM_CLIMB_002 — paint pad (`0x1082`)

Thirteen bounce attaches. First arc (pid-local):

```
t 0.270  fall->bounce  x=32.030 y=31.210
t 0.282..0.579  Y rises to ~40.8, X drifts +facing
t 0.592  bounce->fall   x=33.615 y=40.819
t 1.041  fall->bounce again on pad column 33
…
```

All painted cells restored to `0x1`.

### H01_KF_1 — watch-only (pressure_pad present)

Loaded path `Compiled\H01_KF_1.lvl`. live `0x1000` cells: **0**. No bounce / trampoline state in 50 s.

---

## Verdict

A trampoline pad is **runtime cell bit `0x1000`**. Landing on it (fall path) attaches **bounce** (`0x004B0810`), which throws the lemming on a parabola and can re-bounce when the pad cell is non-solid and `t > 1.4`. The bit is painted by **trampoline state enter** (`0x004B0D08` / type `0xe`), not by `.LVL` authoring. **`pressure_pad.a3d` is not the trampoline** on the level checked (clean negative).

Sim: `Terrain::trampoline`, `State::Bounce`, fall-land branch, integer rise stand-in for the formula. Test in `test_world` (`testTrampolineBounce`); mutate-alone of the fall branch fails that test; suite **16/16**.

---

## Left open

- Assigning trampoline skill through the panel (type `0xe` / panel case 8) to watch enter paint live without harness OR — not required to settle detection/effect; stock on H01 had panel[8]=0.
- Exact discrete tick mapping of the continuous bounce ODE (sim uses `bounceSpeed=7`, `bounceRiseCells=24` from the formula bounds).
- Whether any retail level ships a non-zero trampoline skill stock / natural pads without paint.
- Landing-path zone arms `0x200` / `0x4000` (still open; not this gadget).
- Water / acid / teleport / other hazards (out of scope).
