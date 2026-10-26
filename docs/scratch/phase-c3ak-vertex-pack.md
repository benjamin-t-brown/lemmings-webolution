# C3ak — explicit final vertex-pack component, not app adoption

2026-10-25. Packet `build/tmp/c3ak-vertex-pack/round-note.md`. Adds
`SceneVertexPack` and one focused CMake test/app source entry. BackdropScene does
**not** call the helper yet; that integration debt remains explicit.

## Facilitator check

Read header/source/test and both CMake hunks. Known32-byte layout writes f32
XY/Z, RHW from rounded/reloaded f32Z, raw truncated diffuse OR/shift word and
f32 offset UV, preserving specular+0x14..17. Inputs/domain/rejection-without-write
are declared software contracts, not native out-of-range/error behavior.

Independent raw-intermediate/struct formulas in
`build/tmp/c3ak-independent-cases.py` reproduce all **2077 retained candidate
known-field records**, then generate **2077 separate signed/overflow/UV/RHW
synthetic cases**. Both sets execute the actual C++ helper through its fixture
driver; all28known bytes agree and the specular canary survives. No worker
generator or packing function supplies these expectations. Candidate2077 remains
a constructed table, **not measured native clipping/submission counts**.

Fresh executable bytes re-read46D4F6..46D5BC and49C5AC: f32Z is reloaded at
46D51C before reverse division by raw-backed0.25 at4B19D8; diffuse uses raw
truncation/OR/shifts, no1to255 rescale; both UV stores are f32 after offset adds.
ftol sets chop and returns low32of its qword conversion; the helper's supported
signed32domain is intentionally narrower, not a claim about native overflow.
Specular remains unwritten. Source reads retain original precision limitations.

Own isolated source mutation changes **only** RHW numerator0.25to0.5. Rebuild
succeeds; the permanent known-byte assertion fails (not a loader/build failure).
Exact source restoration SHA256
`74805a4e2aa6ca9b3df367e7b2fda833779cd765743c35edaae99ecf35d9c866`;
own final rebuild, **40/40 CTest**, and both2077-record sets pass at18:43:27.
Logs: `build/tmp/c3ak-independent/`. Worker's separate raw-colour-only mutant
also has a real assertion failure despite its wrapper reporting exit127.

Required fresh repair4/per-region and all10 C1/full-frame reports are in
`build/tmp/c3-check-ak-component/`. They include **unverified sibling AI predicate
changes**, not a packing-caused improvement; do not promote that C3 score through
this component landing:14837819/235008000=0.0631375060, rest12430637,
skills1490654,tube674411,counters151435,caption75548,actor boxes15134.
Last accepted C3 remains0.0846921807 until AI verification.
Both frozen C1 gated panel views still pass0.0160864/0.0161212. Gate/footprint
and references are unchanged; no original visibility/precision/GPU claim.

Zero stock starts/attach/install/game writes, no live resources. Scope is explicit
numeric packing compatibility. Next adopt the helper in the actual scene after
AI releases its files; clipping, bias/state and native pixel fidelity remain open.
Dispatch18:31:17; landing and elapsed recorded in archive.
