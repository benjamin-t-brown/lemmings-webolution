# C3aw — static scene-fan observation-site preflight

2026-10-25. Packet: `build/tmp/c3aw-site-preflight/round-note.md`,
`site-spec.md`, `commands.md`, raw pins and packet manifest. No native observer
was deployed or executed. This is a bounded static design, not native ABI,
safe-hook, effective-state or full-frame acceptance.

## Accepted static scope

- Proposed PRE **0x46D5EC** (`8b15b8cd5800 52`, seven whole bytes) and POST
  **0x46D5F6** (`b940cb5800`, five bytes) do not overlap. The actual
  `0x46D5F3 ff5170` slot call is not patched by this proposal. Hooking directly
  at that call needs eight whole bytes and overlaps POST: that pair is rejected.
- At PRE, five words are topology/FVF/vertices/count/flags; the device has not
  yet been pushed. Under the conditional x86 Device3/stdcall interpretation,
  PRE ESP is EBP-0x28 and POST ESP is PRE+20; POST EAX must be retained before
  `0x46D5FB` calls the next helper. SDK slot28/offset0x70 supports the candidate
  layout, not runtime IID or implementation identity.
- The exact ten-frame EBP-chain discriminator selects primary-selector kind5/7
  invocation-local descriptor, source record/ordinal and saved node. Strides are
  0xB0/0xE8. It excludes shared4/kind6, secondary-selector and unknown paths.
  It does **not** identify durable assets/file offsets, resource generations or
  repair4 scene-root/pass membership.
- Proposed policy caps: 128 vertices/4096 payload bytes, 8192 target-read bytes
  per candidate, one accepted raw diagnostic in a three-second/1024-callback
  arm interval. These are design ceilings, not measured safe native rates.
  Raw specular survives with unknown writer/history; no invented state defaults.
  The synthetic reference always returns `comparable=false`.

The site specification retains install/observer identity, atomicity, register/
FPU preservation, per-thread pairing, omission counters and independent complete
span readback requirements. There is no executable observer identity yet: only
the design and synthetic reference have hashes. Bundled REA Node Frida is
17.23.2; installed Python Frida metadata is separately 17.24.3, not interchangeable.

## Facilitator check

Independently parsed the installed PE section table, authenticated all **31**
raw windows against actual file bytes and companion binaries, and re-decoded/read
their **877** instruction lines. Re-derived pending stack pushes, return arithmetic,
source-pointer stores, loop/ordinal relations, thunk dispatch and stride helpers;
checked the SDK Device3 method position and TL/FVF declarations. Seven installed
file hashes match. Original executable SHA256 remains
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

Own fresh private-memory implementation/fixtures, independent of the worker's
fixture generator, pass **2 positive/16 negative** cases (both source kinds,
wrong caller/field and missing final payload byte). Worker static edge checker
and **52** synthetic controls also pass. Own one-change mutation replaced only
the payload read with zero bytes: the independent raw-payload assertion failed,
not a parser/import error. Restored source byte-identically to backup, SHA256
`f2a51613ac2e09e7ba49ee960acfe6f655405d50c603ae303a277d6848f39aec`;
independent and worker checks pass again; complete packet manifest matches.
Checks/logs: `build/tmp/c3-preflight-facilitator/check_aw.py`,
`aw-own-disassembly.txt`, `aw-*.log`.

**Narrowing:** whole instruction envelopes and synthetic stack checks do not
establish Frida relocation, actual interface ABI, source reachability, complete
history, native collection/transport accounting or non-interference. No live
process/task was acquired by AW. Fresh wrapper build and **44/44 CTest** pass;
framework/comparator **37/37 Python** tests pass. No renderer behavior changed.
Gates are retained baselines, not freshly rendered here: C3 **0.0635481771 FAIL**,
C1 zoom1/elev_down **0.0160864/0.0161212 PASS**; no masks/references/floor changes.

## Next boundary

Keep [capture proposal steps2/3](../RENDER_CAPTURE_PROPOSAL.md) unapproved.
After controller/observer review, the smallest proposed later observation is one
ordinary start and one bounded REA-CLI Frida attach, expressly including temporary
hook injection, for one **unqualified raw slot-call diagnostic**. Navigation input
and any necessary level-slot write/restore require explicit scope. Semantic
qualification and matched logger-off/on non-interference are separate claims.
No automatic native continuation or full-frame logger follows this landing.
