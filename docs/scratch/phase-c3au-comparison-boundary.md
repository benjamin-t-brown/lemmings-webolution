# C3au — conditional pre-wrapper comparison boundary

2026-10-25, offline rendering pivot. Candidate assessment:
`build/tmp/c3au-boundary-assessment/{round-note,decision-spec}.md`.
This is a **design assessment**, not native capture acceptance or a new renderer.

## Decision and evidence

Prefer a **caller-scoped game→wrapper submission observation**, conditional on
subsequent native validation. Start with one kind-5/7 scene-mesh fan route from
[C3n](phase-c3n-mesh-submission.md), not an all-frame census. Preserve source
caller/object association rather than equating screen position with identity.
The selected candidate device endpoint is the call at `0x46D5F3`.

The alternate dgVoodoo→D3D11 boundary observes the translator's output. It may
be valuable, but original call/object identities, topology and order cannot be
assumed to survive translation one-to-one. Tool compatibility is **unknown**;
RenderDoc was neither tested nor rejected. No proxy or live method was deployed.

Existing [C3q](phase-c3q-scene-device-state.md) separates cached game requests
from accepted/effective device state. A between-draw global, SDK declaration,
nonempty trace or callback-install announcement is not evidence of the latter.
[C3ak](phase-c3ak-vertex-pack.md) and [C3an](phase-c3an-raw-diffuse.md) establish
bounded software/source compatibility, not visible native submission counts.
These accepted findings are reused, not globally re-certified by this round.

## Thin-path contract (PROPOSAL)

- Preserve immutable raw records and link every normalized field back to them.
  Record schema/capture/boundary identity, source provenance and event sequence.
- Geometry: topology, counts, raw vertex/index payload, stride/FVF and explicit
  coordinate space; retain unwritten/unknown attributes, not fabricated zeros.
- Resources: generation, content version and update/lifetime lineage. Bare
  pointers or session-local handles are not durable content identities.
- State: ordered requests plus returns and known/unknown history. Pre-frame
  state is unknown absent a trustworthy checkpoint. Successful requests and
  observed effective results are distinct; do not invent defaults.
- Preserve ordering, relevant clears/viewport/depth and omission counters.
  Dropped/decode-failed/missing records prevent a completeness claim.
- No coordinate fitting, numeric tolerance, call merging, sorting, redundant
  state deletion or texture-equivalence normalization. Any later fan expansion
  must preserve triangle order/winding and original call boundaries.

The next experiment is a **no-forwarding synthetic recorder/comparator**. It
must detect independent geometry, state, ordering, resource-version, viewport,
clear and dropped-record changes; missing state must be indeterminate, not an
equal default. It establishes tooling mechanics only, not hook/ABI feasibility.
No production logger is required for this offline contract test.

## Native blockers and later permission proposal

Unresolved: actual interface/method identity and ABI; selected route reachability
in repair4; object/resource attribution; state/resource history before attach;
API success/effective state; dropped-call accounting; observer safety and timing;
wrapper mapping and matched non-interference. These are not filled by synthesis.

Propose, **for later approval only**, one stock start and one narrow read-only
observer for that route, with an offline-preflighted site/signature and bounded
logs. It can establish sample reachability, not its own counterfactual. A separate
matched logger-off/on observation (plan for another start unless same-process
matched state is established) is needed for non-interference. No automatic retry.
The observation method, exact site coverage, restoration/readback and process/
install checks must be approved in the eventual live brief. A proxy DLL, launch
injection, debugger or RenderDoc is a separate permission, not implied here.

Preselected fixtures: repair4 current pause diagnostic; A7p zoom1/elev_down view
diagnostics; MM_DIG_000 second-level holdout. Saved states are **not** asserted
matched. They are validation roles, not a parameter-search set or new gates.

## Facilitator check

Independently read the four cited accepted notes and SDK `d3d.h` Device3 method
declaration. Re-read actual PE bytes `0x46D5BC..0x46D604`: `push 0x1C4` at
`0x46D5DE`, `push 6` at `0x46D5E3`, device slot load and
`0x46D5F3 ff5170` (`call [ecx+0x70]`). Counting the SDK methods places
DrawPrimitive at slot 28/offset 0x70. **Narrowing:** this supports the documented
layout candidate, not a runtime IID, successful HRESULT, exact source-object
identity or capture compatibility. No forwarding implementation was tested.
Evidence: `build/tmp/c3-offline-pivot-startup/au-endpoint-bytes.txt` and SDK
`C:/progs/msys2/mingw32/include/d3d.h:1146..1203`.

Fresh independent render/histogram baseline check reproduces repair4 PNG
`9a5baba7…`, **14934330/235008000 = 0.06354817708333334**, FAIL. Frozen C1
footprint remains 23166 pixels: zoom1 **285083/17721990**, elev_down
**285700/17721990**, both PASS. Fresh wrapper build and **44/44 CTest** pass.
Scripts/logs: `build/tmp/c3-offline-pivot-startup/`. No behavioral mutation is
applicable to an assessment-only landing. No gate moved or native observation
was made; zero starts/attaches/install/game writes. Startup process/task check
was empty; no live resource owned. Dispatch/landing times are in the queue archive.
