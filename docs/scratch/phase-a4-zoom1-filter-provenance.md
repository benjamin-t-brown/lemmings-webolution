# A4aa: zoom1 predecessor state remains unresolved in the bounded static trace

Queue `a4aa-zoom1-filter-provenance`. Static-only worker session
`ses_ea5cd35d2ffeY5ERur9FMRvYN4`, followed by same-session packet repair.
Corrected packet: `build/tmp/a4aa-zoom1-filter-provenance/round-note.md`;
initial overclaims preserved in `round-note-initial.md`. No live process,
instrumentation, REA provider or production changes in this round.

## Accepted result

The last filter request and device alpha-reference assignment before A4z's
zoom1 actor draw are **not established by this bounded analysis**. That is not
a proof that a static answer is impossible. Neither a LINEAR/A0 renderer fix
nor an explanation of the A4z pixel failure is accepted.

The useful new boundary is the surrounding frame/tube path: explicit calls,
known non-filter writes, unresolved subdraws and independent material caches.
Finding no direct filter-setter call in the inspected bodies does not clear
their callees or prove the path leaves state unchanged.

## Byte-backed assignment table (READ)

Authority digest is the full stock digest in LESSONS. All byte anchors and six
explicit decoded windows are independently recorded by
`build/tmp/a4aa-facilitator/verify.py` in `verified.json` and per-window `.txt`.
Window ends are exclusive; these are explicit file-backed ranges, not inferred
nearest-prologue function ownership.

| site | bytes | supported conclusion |
| --- | --- | --- |
| `0x441853`, `0x44185B` | `8a8839510000`, `7505` | this+0x5139 guards frame body; zero jumps to epilogue, semantic meaning open |
| `0x441862..0x44186B` | `8b15b0bf4c008b4210a33cd55800` | copy control+0x10 into zoom gate 0x58D53C |
| `0x441879`, `0x441884` | `e82563ffff`, `e8638cffff` | calls 0x437BA3 then tube 0x43A4EC; other callees not certified state-free |
| `0x43A6BB`, `0x43A908` | `e8ef680300`, `e8a2660300` | tube calls 0x470FAF twice in the decoded body |
| `0x470FD6`, `0x470FE9` | `6a0c6a00`, `ff91a0000000` | that helper writes stage-0 state 0x0C, not MAG/MINFILTER 0x10/0x11 |
| `0x43A9BB`, `0x43AA47` | `ff5210`, `ff5210` | two tube subdraw calls have unresolved indirect targets here |
| `0x44190D` | `ff5210` | per-object draw dispatch; preceding object identities/order not recovered |
| `0x4419A0` | `c7053cd5580000000000` | clear zoom gate after the object loops |
| `0x441A71`, `0x441A7F` | `e8c39b0100`, `e882b20000` | calls 0x45B639/0x44CD06 occur after the loops and clear, not direct same-frame predecessors of the earlier actor draw |
| `0x47A766..0x47A76D` | `833d3cd5580000750c` | nonzero zoom skips actor pre-setter at 0x47A776 |
| `0x47AB7F`, `0x47AB84`, `0x47AB8B` | `e812000000`, `6a01`, `e89963ffff` | draw call, then argument-1 filter setter; post-call is not evidence of the pre-draw state |

The complete decoded frame window `[0x441845,0x441AD8)` and tube window
`[0x43A4EC,0x43AA63)` contain no immediate direct call to 0x470F29. Other direct
callees and indirect targets remain unexpanded. In particular, no transitive
absence or dominance conclusion follows from that direct-body inventory.

## Setter defaults are not two sequential device requests

`0x470F32 c745fc01000000` sets local filter value 1;
`0x470F39 c705e0244c00ef000000` stores EF to the global reference first.
The argument low byte (`0x470F46 25ff000000`, `0x470F4D 7420`) and the
receiver's +0x3A1 byte (`0x470F54 8a91a1030000`, `0x470F5C 7411`) gate an
overwrite to local value 2 and global A0 (`0x470F5E c745fc02000000`,
`0x470F65 c705e0244c00a0000000`).

Only after choosing that value does it issue stage-0 MAG/MINFILTER requests at
`0x470F86 ff90a0000000` and `0x470FA3 ff91a0000000`, using the same selected
local value. It does **not** necessarily request POINT then LINEAR. Numeric
state/value operands are READ; API names retain A4r's SDK/layout inference.
Setter success and effective wrapper/GPU state are unmeasured.

## Global reference is not device ALPHAREF

A4r's two material caches independently matter:

- `0x472626 a1f4d55c00` loads pointer cache, equality exits through
  `0x47262E 0f8441030000`.
- `0x47265C 8b15e4244c003b5124` compares cached flags to resource+0x24;
  equality exits through `0x472665 0f840a030000`.
- The changed-flags branch reads the global at
  `0x472896 8b15e0244c00` for its device alpha-reference request.

Changing the global alone therefore does not show which alpha reference was
last sent to the device. A4x's sampled option byte at zoom0 is not a new zoom1
observation. Cross-frame state cannot simply be attributed to the previous
actor's post-call without resolving intervening draws.

## Facilitator check (2026-10-20)

Separate checker uses pefile/Capstone, not the worker's stockbytes helper or
heuristic graph. It passes **38 original byte pins** and fully decodes six
explicit ranges: frame, tube, filter setter, state-0x0C helper, actor submit,
and material updater. Own range-end checks corrected an extra tube byte and
included the material RET; decoded inventories preserve unknown indirect edges.

Re-ran the repaired worker checker and the existing independent A4r checker:
all pass (A4r: 34 setter tuples, 21 anchors, 3 SDK offsets, 20 enum definitions).
`checks.py` records both outputs plus build/CTest under
`build/tmp/a4aa-facilitator/`. Build reports no work required, **20/20 CTest pass**.
No source mutation/pixel gate is applicable: no behavior changed and no new
pixel acceptance is claimed. A4z's full-scale failure remains.

**Narrowing/rejections:** the initial whole-graph absence claims came from
nearest-selected-prologue grouping, including a malformed sorted-start fallback;
the worker's graph scripts are discovery artifacts, not accepted call-graph
evidence. Literal imm32 matches cannot exclude computed-pointer writers. The
worker's ten raw E8/E9 target candidates are not used here to assert an exhaustive
native caller inventory. Unchecked object identity and cross-frame-last-writer
claims were removed or demoted. We land the scoped observations above, not the
initial claim that static resolution is impossible.

Own source/harness and exe/save/config hashes equal the already independently
checked A4z baseline. Separate fresh PowerShell check: zero Lemmings/webolution
processes and zero LemWeb tasks. No live writes/resources existed to restore or
close. A4z's historical missing level-slot raw-byte ledger is not retroactively
fixed. Pre-existing dirty work, A2/A3 failures and A5m integration gaps remain.

## Next bounded question

Observe the actual setter requests immediately preceding a selected zoom1 actor
draw, correlated by actor identity, thread and event sequence, rather than a
between-draw global sample. That is a separate live brief. Request arguments
remain distinct from effective device state; material-cache ALPHAREF and any
production raster change must stay independently gated.
