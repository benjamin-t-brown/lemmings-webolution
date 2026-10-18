# Phase 3 round 30 — inverted step-up / climb / dig

2026-10-16. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`, driven through `tools/dinput-shim`.

Harness: `tools/harness/measure_inverted.py`. Re-run:

```
python tools/harness/measure_inverted.py --audit
python tools/harness/read_steppers.py --at 0x00415A86 --len 0x40
python tools/harness/read_steppers.py --at 0x0041C48E --len 0x40
python tools/harness/read_steppers.py --at 0x00413869 --len 0x30
python tools/harness/read_steppers.py --at 0x0042F5E3 --len 0x90
# restart process between levels
python tools/harness/measure_inverted.py --enter MM_DIG_000 --dig \
       --force-vdir -1 --paint-mask 0x21 --seconds 3.5
# (place-under-ceiling pattern in session notes / _tmp_r30_dig_ystep.py)
python tools/harness/measure_inverted.py --enter m06_kf_1 --paint-step \
       --cols 72,73,74,75 --rise 1 --watch 25
```

Live spellings: `MM_DIG_000`, `m06_kf_1`.

---

## SUMMARY

| Thing | Status | Value |
| --- | --- | --- |
| **dig mask inverted** | **MEASURED** | `[+0x44]==-1` → mask **`0x20`** (`0x00415A96`); upright remains `0x10` |
| **inverted refuse of `0x20`** | **MEASURED** | forced vdir=-1 under painted `0x21` ceiling: dig state entered, **0** grid diffs, returned to walk |
| **inverted dig of `0x10`** | **MEASURED** | painted `0x11` ceiling cleared (word→0); thick slab: 12 cells across rows 71–74 |
| **inverted dig Y step** | **MEASURED** | `Y := Y - vdir` → with vdir=-1, **Y += 1** per cycle (70→71→72→73) then fall |
| **retail `0x10`** | **MEASURED offline** | **0** cells in 102 levels — paint required for the `0x10` half |
| **inverted step-up** | **MEASURED, m06** | hangs under ceiling row 19: **19× dy=-1**, **20× dy=-2** (Y-up) |
| **inverted climb / hoist** | **MEASURED, m06** | 2 climbs at col 84; climb→hoist at Y 14; hoist→walk at **Y 12** (net −2 = +2·vdir) |
| **bash `0x4` right-facing** | unread | no natural inverted/far-side run; not invented |
| **sim** | landed | walker/climber/hoister/digger use `verticalDir`; `Terrain::digRefused` / `0x10`/`0x20` from grid |

---

## Static reads

| claim | address | bytes / observation |
| --- | --- | --- |
| dig mask default | `0x00415A86` | `mov [ebp-0x10], 0x10` |
| dig mask inverted | `0x00415A96` | after `cmp [ecx+0x44], -1` → `mov [ebp-0x10], 0x20` |
| dig refuse test | `0x00415B2A` | `and edx, [ebp-0x10]` |
| dig row | `0x00415AA0` | `posY - vdir` |
| dig step-Y | `0x00415DD2` | `Y := Y - vdir` then ground check / fall |
| walk step-up | `0x0041C48E` / `0x0041C528` | `add …, [reg+0x44]` |
| climb Y advance | `0x00413869` | `fild [+0x44]; fmul 5.0; fmul dt; fadd Y` |
| hoist net | `0x00417D6F` / exit | `+3*vdir` then exit `−vdir` → **+2·vdir** |
| flip biases | `0x0042F5E3` | upright `[+0x358]=3,[+0x368]=0`; inverted swapped |

---

## Offline audit

```
levels 102
bit 0x10 cells: 0 across 0 levels
bit 0x20 cells: 10448 across 8 levels
  MM_FLOAT, MM_MINE, MM_BASH_DIG_001_A, MM_ALL_8_002, MM_BASH_MINE_WLR1,
  MM_DIG_000 (876), MM_ALL_8_003, MM_H_003
bit 0x8000 cells: 150 across 18 levels
```

---

## Live series

### Dig — inverted refuse of `0x21` (PASS)

MM_DIG_000. Placed walker at Y=70, wrote `[+0x44]=-1` (held each sample), hovered
and assigned digger (panel 7). Dig-row 71 was natural/painted `0x21`.

- `saw_dig=True`, stock spent, **grid_changes=0**, Y stayed 70.0, returned to walk.
- Restored Y, vdir, and any painted cells.

### Dig — inverted clear of `0x11` + Y step (PASS)

Same placement; painted rows 71–74 as `0x11` (wide).

```
t 0.000s dig   y=70.000
t 0.343s dig   y=71.000
t 1.538s dig   y=72.000
t 2.747s dig   y=73.000
t 3.946s fall  y=74.260 …
diffs 12: rows 71..74 cols 83..85
```

Y-up step is **+1 per cycle** with vdir=-1 (= `Y - vdir`). All paints restored.

### Step-up / climb — m06_kf_1 (PASS)

Natural flip at pad (68,7); walk_bias=3.0, climb_bias=0.0 as `0x0042F5E3`.
Painted hangs under ceiling row 19 (rise 1 at cols 72–75, rise 2 at 78–81,
3-high wall at 84–85). Poked `[+0x48]=1` on inverted walkers; restored.

| event | count |
| --- | --- |
| inv step-up dy=-1.000 | 19 |
| inv step-up dy=-2.000 | 20 |
| inv step-down dy=+1.000 | 19 |
| walk→climb | 2 (x=84) |
| hoist→walk | Y 14 → **12** |

Against gravity while inverted is **−Y** in the engine's Y-up world — the same
axis the upright series uses, with `[+0x44]` negated.

### Bash `0x4` / `0x8`

Not reached naturally while inverted. Left unread live (same as round 5's
right-facing `0x4` note). Static mask read at `0x0040C0DC` / `0x0040C0EC`
unchanged.

---

## Verdict

After anti-grav (`verticalDir = -1`), **step-up, climb/hoist, and dig match the
upright series with gravity flipped** along `[+0x44]`. Dig refuse bits behave
as the stack-mask reads claim: inverted diggers **refuse `0x20`** and **clear
`0x10`**. Sim walker/climber/hoister/digger now thread `verticalDir`; terrain
loads `0x10`/`0x20` into `digRefused`.

Mutate-alone: `Terrain::digRefused` → always false fails the refuse fixture
and clears 0x20 cells; stripping `rise * v` from the walker fails
`testInvertedStepUp` (hang 1 stays at y 41). Restored both. Full suite
16/16.

---

## Proposed `PLAN.md` §1.6 bullet

> Round 30 — inverted steppers: after anti-grav (`[+0x44]==-1`), step-up
> (dy ±1/±2), climb/hoist (net +2·vdir), and dig (`Y -= vdir`, mask `0x20`)
> match upright with gravity flipped. Inverted dig refuse of `0x20` / clear of
> `0x10` MEASURED (`measure_inverted.py`; m06_kf_1 / MM_DIG_000). Bash `0x4`
> right-facing still unread. Sim: `verticalDir` in walk/climb/hoist/dig;
> `Terrain::digRefused`.

---

## Left open

- **Bash `0x4` facing right** (and inverted bash if ever natural).
- **Cell-flag writers** for `0x4`/`0x8`/`0x10`/`0x20` — round 31 queue; bits
  arrive in `.LVL` bytes only.
- Walk Y-bias `[+0x368]=3` when inverted is observed on flip but not modelled
  as a separate sim field; steppers use `verticalDir` on the feet cell directly.
