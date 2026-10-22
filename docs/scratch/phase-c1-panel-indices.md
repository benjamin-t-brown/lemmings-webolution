# C1a: physical skill-panel indices identified (READ)

Queue `c1a-panel-index-read`; packet `build/tmp/c1a-panel-index-read/round-note.md`.
Offline REA + original-byte read, zero stock starts. Server recovery resumed the same
worker from saved evidence, not another native trace.

## Result and the two distinct tables

| Physical index | Action |
| --- | --- |
| 0 | climber |
| 1 | parasol/floater attribute |
| 2 | bomber |
| 3 | blocker |
| 4 | builder |
| 5 | basher |
| 6 | miner |
| 7 | digger |
| 8 | fast-forward command |
| 9 | Armageddon command |

`0x45A4DD` is a real ten-dword table, but it belongs to eligibility predicate
`0x45A0E0`: dispatch `0x45A12C` is `ff 24 95 dd a4 45 00`, upper bound 9.
The per-lemming factory `0x45AFEB` uses a different nine-dword table at `0x45B570`
(`0x45B051: ff 24 95 70 b5 45 00`, upper bound 8). Both consume selected index
at skills+0x28, written by selection at `0x45ACE4`. Indices 0..7 reach constructors
`413C70,417E40,413D40,40DFB0,40F8D1,40BBA0,418470,415850` respectively.
The bomb/index-2 evidence was already in measure_bomb.py; this is corroboration,
not a newly discovered live assignment.

The last two physical controls must not be labelled from the skill factory alone:

- Index 8: handler `0x455AFF` compares hover+0x38 to 8 at `0x455B12`, takes an
  input-gated special branch and stores 1 to `[levelScreen+0x20]` at `0x455B30`.
  It bypasses normal selection at `0x455B58`. Consumer `0x45DB1A` chooses loop count
  from `0x4B15F8` (file dword 7), versus ordinary 1 at `0x45DB2B`. This is a repeated
  simulation-budget path, **not** hatch release-rate adjustment and **not** evidence
  of seven discrete 200 ms ticks or an exact measured wall-clock speedup. Its shipped
  art key is `questionmark`; the factory's latent special-skill index-8 path is distinct.
- Index 9: branch at `0x45472B` calls request `0x444A98` at `0x454765`, setting
  game+0x5138 (`0x444AB1: c6 82 38 51 00 00 01`). Consumer at `0x442646` checks
  the flag; `0x44292B` constructs `0x416910` / explode-animation vtable `0x4B0A58`,
  attached via `0x442962` -> `0x42E696`. The request-side stock guard indexes
  **selected**, not explicitly hovered (`0x454747` / `0x45474C`). No click timing,
  live stock decrement, completion duration or survivor count is established here.

## Reproduction and REA evidence

```text
python tools/harness/read_c1_panel_indices.py --self-test --output <mapping.json>
```

Normal Windows Python + existing Capstone; no game/REA session needed for this checker.
It records source jump displacements, both raw tables, calls/ctors/state types, art
keys and command consumers, and rejects non-authority files or reads beyond raw sections.
Full REA bundle and query index are in the packet (`rea-evidence.json`, `rea-queries.json`).
Bundle SHA-256 `8e925e3bc84fea376d9da9304e56116b0c220b71485f30b44c0f0b48f09b6eba`.
Principal physical-command evidence: `ev_7ced603c026fe9c2d11a9a3509a2c99c889764a307210719024788e0382f78f2`
(8 handler), `ev_3f05cde526333781ba815a8f84b8a63555bcc55a00036c4b092fd815f05bcd56`
(budget-loop consumer), `ev_86a7b2520b9886b30460fe93f6c1c4a00278dad5fcd31e4f106d5c4fe5b93488`
(9 command). Indirect-call coverage and latent reachability remain limited as the packet states.

## Facilitator check

Independent checker `build/tmp/c1a-facilitator/verify.py`, with no worker imports,
parsed the stock PE and re-read all 836 original-byte records in the packet (excluding
the explicitly synthetic mutation record). Independently decoded both dispatches'
table addresses/dwords, factory allocation/constructor calls and art-key strings;
the physical mapping agrees. Walked the actual handler/consumer instructions for
8 and 9, not merely the labels or decompiler prose. An initial interior mass-action
diagnostic started mid-instruction; re-decoding from the packet's established
`0x442644` boundary gave the aligned `0x442905` allocation/tail and exact ctor/attach
calls above. The faulty interior decode was not used as evidence.

Own isolated index control (opposite direction to worker's): factory[6] -> factory[5]
changes extracted constructor from miner to basher and fails index-6 expectation.
Worker's independent in-memory [5]->[6] semantic rejection also reproduced. Neither
control writes the installed file. REA canonical bundle import validated all 26 retained
Evidence records, no unknowns; no decompilation was repeated after restart. Bundle and
checker hashes match the packet.

No production code changed. Same-source full suite freshly passed 29/29 during B1e
verification; C1 adds only an offline Python checker. Own post-restart process check
found no original game; exe/config/save hashes still match the pre-run baselines.
No live writes or instrumentation require restoration.

C1's **index-identification clause is complete**, not its rendering/count/pixel gate.
No latent special-skill retail reachability or additional native round is authorized
by this result; the next C1 work is actual panel art/count composition.
