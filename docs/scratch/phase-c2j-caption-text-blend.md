# C2j: "Paused" text drawn; A7 without the caption exclusion passes 9 of 10 views, MM_DIG_000 fails by 2,660

2026-10-23. Directive Q1 round 1 of 2 (offline). Worker note `build/tmp/c2j-caption-text-blend/round-note.md`. `PauseCaption.{h,cpp}` gain `drawCaption` (panel, then
HudText's `drawHud`, not forked); `--pause-caption` now draws panel + text, `--caption-no-text` gives the C2i render; `test_pausecaption` gains `testText()`;
`score_a7_caption.py` renders before/after itself. No start, no debugger, no game process. 6 of 6 bodies read; REA not used.

## What is read and what is not

| Item | Value | Tag |
| --- | --- | --- |
| `0x4017F0` | stores arg dword i at `this+4*i`: six dwords are three doubles | READ |
| `0x44DBC0` / `0x44DD20` | colour block of 24 bytes per vertex; the double 128.0 at `+0x58` (and `+0xC0/+0x128/+0x190`), the double right after the block | READ layout; alpha role and 0..255 scale INFERRED (sibling callers push `0x406FE000` = 255.0 at `0x44FF28..0x44FF3B`, `0x44FF71`) |
| Fill (0,0,0), alpha 128 | six `push 0` at `0x45D37C..0x45D384` plus the call at `0x45D392`; `push 0x40600000; push 0` at `0x45D397` | READ immediates |
| `STR_PAUSED` | slot 84 (`0x54`) of the pointer table at `0x4C3494`; `0x45D262` pushes `0x54`, calls `0x486847` | READ |
| "Paused" width | 14+11+12+11+11+11 = 70 in the `^Font` table at `0x4BF968` | READ; equals the derived W |
| Text origin 285,422 | rect x0 + 8, y0 | MEASURED live (C2h) |
| String "Paused" and white ink | the exe holds only the key | MEASURED-FROM-REFERENCE (A7 baseline capture) |
| Blend `(px*(255-a)+fill*a+127)/255` | `0x46D460` not read | OURS |
| `0x486847` lookup, 0.00225, the `1` passed to `0x44DBA0`, text colour in the exe, quad doubles as final pixels | | UNREAD |

## Result (caption exclusion removed, ten skill boxes kept; pass iff numerator*50 <= N*765)

initial 2879335 -> 2678209 (threshold 2742219, **pass**), canonical 2892671 -> 2691545 (**pass**), mouse, right, left, zoom1, elev_up, elev_down, elev0_neg pass both ways,
**MM_DIG_000 2882064 -> 2686464, threshold 2683804, fails by 2,660.** Region residual inside `[277,422,363,454)` is down on 10 of 10 views (initial 412464 -> 136273,
MM_DIG_000 413691 -> 110005). The worker predicted initial, canonical and MM_DIG_000 would keep failing; it was wrong for the first two. Sensitivity (alpha 0 and 255, white
fill) reported in `score.txt`, not used to choose; alpha 0 fails initial, canonical and MM_DIG_000. "Before" numerators differ from C2i's by 1-2 units on initial and
canonical (2879335 vs 2879334): the same five blue-channel pixels that differ between the Mac build and the A6d24 PNG in C3a; not investigated.

Gate status for Q1: **not met.** Nine of ten views pass; the tenth is 0.1% over.

## Facilitator check

Own pixel loop (`build/tmp/c2j-fac/check.py`, the c2i checker pointed at the worker's saved PNGs; not `verify.wider`): numerators, N, thresholds and region residuals equal the
worker's on all ten views, the same single failure. Re-read on the exe: `0x44DD20` (stores `+0x58/+0x5C`, `+0xC0/+0xC4`), `0x44DBC0` (calls `0x4017F0`, `rep movsd` 24 bytes),
`0x45D262..0x45D286` (`push 0x54; call 0x486847`), `0x45D37C..0x45D3AA` (five `push 0`, call `0x44DBC0`, `push 0x40600000; push 0`, call `0x44DD20`), `0x44FF28..0x44FF40` and
`0x44FF71` (255.0 pushes). Mutation alone: `textY` 422 -> 421 in `PauseCaption.h` fails three `pausecaption` checks; restored, sha1 `a6cfe752...` equal. Full build and ctest on mouv
34/34 after copying the five changed files (sha1 equal on both). The worker ran `git stash` / `git stash apply` while c3a was running; the tree matched afterwards, the stash list is empty and
nothing outside its paths changed, but the standing run forbids stash and this is recorded. The test target relies on `#include` of `HudText.cpp` / `MenuRender.cpp` instead of listing them
in `CMakeLists.txt:199`; left as is. Not verified: the worker's other seven single-input mutations, the sensitivity renders beyond the table, `actorcompose` flaking once in the worker's run
(34/34 on mouv).
