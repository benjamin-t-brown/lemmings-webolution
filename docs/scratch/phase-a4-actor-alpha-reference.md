# A4y: actor alpha reference corrected to `0xEF`, protected by raster tests

Queue `a4y-actor-alpha-ref-ef`. Offline implementation round following
[A4x's filter bracket](phase-a4-actor-filter-state.md); no stock game launch or
new live measurement. Worker packet: `build/tmp/a4y-actor-alpha-ref-ef/round-note.md`.
The OpenCode round worker completed on session `ses_ea63befc7ffesrrQIB1GjK9kBH`;
the requested model/variant is recorded in the queue, not independently observed.

## Accepted change

- `src/assets/LightMap.h`: `kActorAlphaTestReference` is now `0xEF`, not the
  between-draw `0xA0`. The helper remains strict `textureAlpha > reference`.
- `test_actorsprite` draws alpha `0xA1 / 0xEF / 0xF0 / 0x00` into a sentinel
  destination. Raster assertions precede helper/literal assertions: below/at the
  reference leave the destination untouched; `0xF0` writes modulated RGB opaquely.
- The public `ActorSprite.h` comment was corrected by a bounded comment-only
  repair. No raster geometry, filtering or diagnostic behavior changed.

This changes drawn to rejected **only for alpha `0xA1..0xEF`** relative to A4u.
The independent exhaustive check covers all 256 alpha values: 240 rejected,
16 drawn, with destination RGBA checked as well as the helper result.

## Original evidence (READ, not a new at-draw measurement)

Facilitator reread the authority executable and verified its full SHA-256 against
`LESSONS.md`. Relevant bytes:

| address | bytes | read |
| --- | --- | --- |
| `0x47A766` | `83 3d 3c d5 58 00 00` | gate on `[0x58D53C]==0`; `jne` skips pre-call |
| `0x47A776` | `e8 ae 67 ff ff` | call `0x470F29` after `push 0` |
| `0x470F39` | `c7 05 e0 24 4c 00 ef 00 00 00` | store reference `0xEF` |
| `0x470F65` | `c7 05 e0 24 4c 00 a0 00 00 00` | LINEAR branch stores `0xA0` |
| `0x472896` | `8b 15 e0 24 4c 00` | reference load, pushed with state `0x18` |
| `0x4728B0` / `0x4728B2` | `6a 05` / `6a 19` | function 5 / state `0x19` (GREATER, SDK interpretation) |
| `0x47AB8B` | `e8 99 63 ff ff` | post-draw `0x470F29(1)` |

File `.data` at `0x4C24E0` remains `a0 00 00 00`, inside rawsize. That initial
value is not the pre-draw setter value. Full disassembly windows are retained in
`build/tmp/a4y-actor-alpha-ref-ef/facilitator/bytes.txt`.

## Facilitator check (2026-10-20)

Independently verified the offline headline using a separate 256-case C++ raster
checker (`facilitator/alpha_check.cpp`), not the worker's four-texel test. Expected
RGBA uses a literal 240 threshold and fixed sentinel/tint outputs, never the
production constant. The checker passes before and after restoration.

Own isolated mutations, fresh build each time:

| mutation | actual `test_actorsprite` failure | exit |
| --- | --- | --- |
| constant alone `0xEF -> 0xA0` | `alpha 0xA1 below the reference leaves the destination untouched` | 1 |
| helper alone `> -> >=` | `alpha 0xEF at the reference is rejected (strictly greater) and leaves the destination untouched` | 1 |

Each restored source matches the worker's final SHA-256 exactly, with fresh mtime
and actual recompilation. `LightMap.cpp` has no net A4y change. The final header
repair strips to identical non-comment text. All four source files use LF.
Fresh full build and **20/20 CTest pass**, including after that repair.

Own A4f gate reruns before and after mutations/repair exactly equal A4v's entire
`results.json`, including both composed and overlay hashes:

| fixture | crop MAE | support MAE | composed PNG SHA-256 |
| --- | --- | --- | --- |
| five | `0.000044998468137254905` | `0.0014287885696914425` | `6d2f6cc786fff4aff861ce79279e098b7204c96964b66bc28f7ec1c5144b81f9` |
| eight | `0.000051062091503267975` | `0.0015155820782419248` | `68f3b5e534366fd391129b575f37d9140b40b33a54ce847d57de830344cd4792` |

Reproduction: `python build/tmp/a4y-actor-alpha-ref-ef/facilitator/verify.py`
with `bytes`, `hashes`, `comments`, or `gates`; build the independent checker with
`cmake -S build/tmp/a4y-actor-alpha-ref-ef/facilitator -B build/tmp/a4y-actor-alpha-ref-ef/facilitator/build -G Ninja`
then build/run `alpha_check.exe` through `scripts/Invoke-Ucrt64.ps1` (production
objects must already be built). Own mutation/suite/process logs are in that same
facilitator directory. Gate invocation is in the worker packet; own outputs are
`facilitator/gate/` and `facilitator/gate-postrestore/`.

**Narrowing and negatives:** the retained frames use only 0/255 alpha, so these
unchanged pixel gates cannot distinguish `0xA0` from `0xEF`. Synthetic raster tests
carry this correction; no new stock pixel acceptance is claimed. The worker's
initial wrong-backup restore was discarded; the facilitator's independent
mutations and hash checks do not depend on it. A4x's indirect-hop,
cache/effective-device and level coverage limits stand. A2's broader failures and
A3's sensitivity failure remain retained; actors are not yet wired into the app.

No live operations were needed. Own final check: **0 Lemmings/webolution processes,
0 `LemWeb-*` tasks**. Prior dirty work and historical retractions were preserved.
No commit was requested or made. Next queued action is the read-only A5m
integration map; A4z's third live fixture dependency on A4y is now satisfied.
