# A3 — ordered sprite selection and mirroring

2026-10-19, branch `decompile`. **Complete for the tested selection scope.**
The score/selection helpers are implemented and independently corroborated against
native actor fields and bounded stock-raster replays. Replacement sprite
rasterization and production integration remain A4. The stricter worker pixel
sensitivity diagnostic below remains a failure.

Authority: stock 640x480 `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
`tools/harness/verify_spriteview_a3.py` parses PE raw sizes, checks that hash,
and records literal bytes plus disassembly in `raw-address-audit.json`.
Live globals below are memory observations, not initialized file bytes;
the `.data` file boundary remains `0x004C6000`.

## Correction: the SPR axes were reversed in the plan

The old assertion “walk 12 views, block 8, fall 7” was wrong. Those are **outer
temporal counts in the LO resource**, not directional counts. A fresh independent
parser fully consumed all six normal/acid/water LO/HI SPR files and checked every
integer rectangle's extents. The new inventory records file SHA-256, record
offsets, source rectangles, counts and palette hashes.

| Action | LO temporal entries | HI temporal entries | Directional images in each temporal entry |
| --- | ---: | ---: | ---: |
| walk | 12 | 12 | 5 |
| block | 8 | 16 | 8 |
| fall | 7 | 15 | 5 |
| cartwheel | 8 | 16 | 8 |
| shrug | 4 | 8 | 1 |

These rows hold across the six files. All other named lemming actions have five
directional images per temporal entry. Extracting the first walk temporal entry
also visibly produces five different viewing angles. The legacy public API names
are preserved: `Sprite::animations` indexes time;
`SpriteAnimation::frames` indexes direction. Header comments now explain this.
The parser's behavior is unchanged.

The executable independently settles the semantics. In temporal advance
`0x468833`, `0x46887F 8b5010` and `0x468885 8b4014` read the action's double
rate at `+0x10`; `0x46889E dd4508` starts elapsed-time accumulation. After
comparison with `0.06` (`0x4688C6 dc1d40194b00`),
`0x4688E0 83c101` increments the temporal index and
`0x4688E6 898acc010000` stores it at controller `+0x1CC`.
The wrap compares it with the action's outer count through `0x45B8F0`
(`0x45B8FA 8b00`). **Controller `+0x1D8` is not this SPR rate**, despite
older harness field labels; the chooser consumes it as a rotation-phase input.
This correction does not retract the previously observed simulation series.

## READ: native inputs and chooser

For a lemming actor, the graphics controller begins at actor `+0x128`.
Native chooser `0x46840B` writes ordered index at controller `+0x1B8` and
mirror byte at `+0x1B4`. It resolves the current directional list as follows:

1. action = `[container+0x0C] + actionIndex*0x98`, via `0x468D00`
   (`0x468D0A 69c098000000`, `0x468D13 8b510c`).
2. temporal record = `[action+0x0C] + temporalIndex*0x10`, via `0x468CE0`
   (`0x468CEA c1e004`, `0x468CF0 8b510c`). This is a **second dereference**.
3. directional count = `[temporalRecord]`, via `0x468CD0`
   (`0x468CDA 8b00`).

Controller `+0x1C8/+0x1CC/+0x1F8` supply action, temporal index and container.
Flattening those two arrays produced an invalid multi-million-entry count in an
early worker attempt; that is not accepted evidence.

Actor update `0x42EEF2` calls the chooser at
`0x42EFD8 e82e940300`. Its basis producer copies actor position at `+8`, adds
integer facing `+0x40` times `0.1` to its first component
(`0x42EF38 db4040`, `0x42EF3B dc0d40104b00`), calls `0x43D56C`, subtracts
the actor model position, and normalizes. Setter `0x4683C5` copies **six
DWORDs = 24 bytes = three doubles** into controller `+0x1E0/+0x1E8/+0x1F0`
(`0x4683DA b906000000`, `0x4683DF f3a5`). It does not copy six doubles.

The passed view vector is actor model position minus camera translation:
`0x42DCBA 0590000000` makes the actor-model getter return actor `+0x108`;
camera getter `0x412C2A 83c048` returns `0x4CBFD0+0x48 = 0x4CC018`.
Helper `0x401830` performs componentwise subtraction
(`0x401841 dd4010`, `0x401844 dc6110`, with corresponding Y/X operations),
not a matrix transform. The chooser normalizes its passed view vector through
`0x480AC8` at `0x46857F e844850100`.

The native score producer rotates the stored basis about Y twice. Starting with
phase `q = controller[+0x1D8] + 0.0625`, it evaluates the first rotation at `q`
and the second at `q + 0.1875`. Both angles are multiplied by twice
`3.1415927410125732` (bytes `00000060fb210940` at `0x4B0F88`).
`0x426D90` calls the sine helper (`0x49CBA2 d9fe`, **FSIN**), and
`0x426DB0` calls the cosine helper (`0x49CC52 d9ff`, **FCOS**).

For stored basis `(bx,by,bz)`, the constructed vector is
`(cos*bx - sin*bz, by, sin*bx + cos*bz)`. The push sequence at
`0x46848E..0x4684D9` and constructor `0x4017F0` fix the X/Z order.
Dot product `0x418350` produces each score. Scores below -1 get **one** +2;
scores above +1 get **one** -2 (`0x4685A6 dc05f8034b00` /
`0x4685C4 dc25f8034b00`, repeated for the second score). This is neither
clamping nor repeated wrapping.

The final comparison stage is count-dependent:

- Count <=1 forces index zero and preserves mirror.
- Count 5 clears mirror, then selects an index in 0..4 and may set mirror
  (`0x46861E 83f805`, `0x46862A c680b401000000`).
- Every other count >=2 follows the eight-sector branch and preserves mirror.
  The six lemming resources actually supply 1, 5 or 8, not seven for fall.

`src/gfx/SpriteView.h/.cpp` separates score generation from these explicit
comparisons. It uses platform sin/cos, so exact x87 intermediate rounding at
geometric sector boundaries is not accepted. The native decision-stage replay
does cover finite signed zeros, exact diagonal ties and both sides of boundaries.
Do not replace this with assumed uniform angular bins.

## Independent live corroboration

Predictions, falsifiers and the crop policy were written before the facilitator
run in `build/tmp/a3-facilitator/live-prediction.md`. The fixture uses a fixed
64x64 box around the floored projected actor world origin, clipped to 640x480;
no alignment or rescaling. It waits ten seconds before pausing to clear start text.
The originally proposed raw “screen corner” offsets were not established and
were explicitly replaced by this policy before facilitator capture.

The facilitator used a separate field reader and a statically linked invocation
of the actual C++ helper. Actors came from `0x4CBB18`, with lemming vtables
`0x4B10E8/0x4B0F6C`, never billboard tracking. Raw world positions, camera eye,
basis, phase, nested list pointers and native index/mirror were retained.

`build/tmp/a3-facilitator/live/report.json` has **42/42 matching native
index/mirror predictions**: 41 five-entry observations and one controlled
eight-entry observation across three actual lemmings. Twelve camera-yaw batches
crossed sectors at samples `yaw-5` (index 1→2) and `yaw-11` (2→3).
Actor positions, basis, phase, action and temporal frame remained identical
across the baseline/yaw samples; camera changes explain those transitions.

Eight-entry coverage temporarily selects stock block graphics ordinal 1,
temporal entry zero on the same paused actor, then restores it. This is a
controlled graphics-list fixture, **not naturally assigned blocker behavior**.
One-entry behavior and other non-five fallback counts have READ/test evidence;
they are not additional live action coverage.

## Selection pixels: stock raster replay, bounded passes

The predeclared facilitator criterion was candidate RGB MAE/255 <=0.02, plus
strictly worse error and nonzero changed pixels from an isolated view mutation.
The candidate injects its computed index/mirror into the same paused actor and
uses the **stock rasterizer**. It does not render a replacement sprite quad.
Chooser entry bytes `55 8b ec` are temporarily replaced with stack-correct
`c2 04 00` (RET 4), so a render refresh cannot overwrite the injected fields.
Every field and the code bytes are restored.

Both crops were `(437,268)..(501,332)`; metrics were independently recomputed
from the retained PNGs:

| Fixture | Candidate MAE | Index-zero MAE / changed pixels | Mirror-flip MAE / changed pixels | Restored MAE |
| --- | ---: | ---: | ---: | ---: |
| native five-entry | 0 | 0.003642004 / 134 | 0.004532718 / 139 | 0 |
| controlled eight-entry | 0 | 0.004004545 / 159 | 0.006397123 / 169 | 0 |

Both candidate crops and both restores are byte-identical within the crop.
Both index and mirror mutations are independently pixel-visible.

**Retained failure:** the worker additionally declared a strict sensitivity
gate requiring index-zero MAE >0.02. Its final attempt measured
`0.003705193`, 99 changed pixels, so it failed. The independent fixtures also
fail that stricter sensitivity gate. It was not loosened or converted into a
pass: the bounded selection replay passes its separately declared criterion,
while the >0.02 diagnostic remains false. None of these crops accepts full-scene
fidelity or proves a replacement raster matches the original.

Early worker attempts are retained under `build/tmp/a3-sprite-view`: no actors,
invalid nested-array reads, native recomputation undoing a forced index, transient
overlay interference, and a cleanup bookkeeping error. Attempt 6's saved
Python-derived scores had X/Z reversed; **only its raw inputs may be reused**.
The persistent worker driver is corrected. Earlier attempts whose final camera
equality was false are not described as clean restores. They terminated their
owned processes and restored install files. The final worker run and independent
facilitator run verify exact memory restoration.

## Facilitator check, tests and cleanup

The facilitator independently reread every address above through initialized
PE ranges, parsed all six assets, and emulated the native comparison/store
instructions from `0x46844A` through `0x46882B` with explicit supplied scores.
**608/608 C++ results match**; the geometric producer is deliberately outside
that instruction interpreter and is corroborated by the separate live inputs.

Eleven independent isolated source mutations failed `test_spriteview`: old
constant zero, X/Z output swap, either phase offset, five-entry mirror default,
eight-entry mirror reset, singleton mirror reset, diagonal tie, missing
normalization, clamp in place of single correction, and idealized PI. After
**each** mutation, the exact original source bytes were restored, rebuilt and
the focused test passed. Record: `build/tmp/a3-facilitator/mutations-independent.json`;
source SHA-256 `1d8249204ec75fbc6165d3ea25d463b52a675fdd19dca0529ff37eafca965a0a`.
The worker's separate nine-mutation record is retained too.

The full build succeeds and **15/15 CTest tests pass** (60.32 seconds).
Python harness compilation and `git diff --check` pass. No simulation code was
changed. The source and focused test are registered with CMake.

The independent run restored all six canonical memory targets while the owned
process was suspended (successful status 0), then resumed and terminated it.
Those targets comprise the camera block, chooser bytes, index, mirror, action
and temporal index. The entry helper restored the temporary level-table slot.
Input was released; no game process remains. No debugger/hardware breakpoints
were used; the worker's actual REA session is closed. Executable/save/wrapper
hashes are unchanged:

```
exe   32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f
save  f7f8603928890fb13d022d828c8a602cf7a7619ab24de82c7de4ae86ec337e6a
conf  d639e220814d80b7956b96bad0ca708b768458bd348bfd607afbf1913598e72b
```

Re-run the offline check with normal Windows Python:

```powershell
python tools/harness/verify_spriteview_a3.py --check-cpp --spr-dir build/tmp/a3-facilitator --live-dir build/tmp/a3-facilitator/live
```

Fresh live verification is `tools/harness/verify_spriteview_live_a3.py --out
<fresh-directory>`; `--prepare-only` checks helper compilation without launching
the game. It requires the retained A1 entry/capture helpers and windowed wrapper
configuration under `build/tmp/a1-zoom-elevation`, plus the retail install. SPR
inventory input is the six freshly extracted `SPRITES/LEMMINGS*.SPR` assets.
Reports/PNGs remain local ignored artifacts; existing runs are never overwritten.

**Next: A4.** Wire these helpers to the preserved simulation and the original
basis/phase producer, use the correct temporal/directional axes, build the 3D
renderer and entry point, restore lost asset tests, and make revolve discoverable.
Retain A2's wider/cavity/rim failures, omitted scene inventory and this round's
strict sensitivity failure. Production sprite placement, UVs, depth, scaling,
animation integration, inverted poses and untested actions still need integration
checks; passing a shared-stock-raster replay does not settle them.
