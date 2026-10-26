# C3bb — pinned-source feasibility review; no native readiness

2026-10-26. Worker packet: `build/tmp/c3bb-verifier-feasibility/`.
**Accept the versioned source inventory; reject its PRE7 prediction and narrow
its custom-getter recommendation.** Three review attempts are spent (BA plus
repair, BB); no additional repair/implementation was dispatched. Initial packet
is preserved verbatim, including the rejected claims.

## Independently checked evidence

Installed REA-bundled Node package is Frida17.23.2, distinct from Python17.24.3.
Authenticated package/wrap/declaration/native-binding hashes in the worker manifest.
Native binding SHA256 is
`32c73230bdfd4b9a8f1f1871ef616001e486269919f18a111a195854352057dc`.
Independent remote tag queries and clean source-tree checks match:

- core17.23.2: `78028da11b850329cb4bfa665a6034bf49b78249`;
- Gum17.23.2: `2b0e6a661702097d77474223ebc429ece092cb46`.

Package metadata and source tags establish intended versions, **not a reproduced
build or attestation that the loaded 32-bit agent executes those exact sources**.
No new mandatory reproducible-build policy is adopted here; this remains a
provenance limitation to address in the proposed safety case.

### Concrete numerical correction

At that Gum commit, `gum/backend-x86/guminterceptor-x86.c:125-128` chooses a5-byte
default IA32 redirect. `313-320` reads whole instructions until the cumulative
length is **at least5**, not until the AW proposed envelope is exhausted.
`gum/arch-x86/gumx86relocator.c:164-225` returns cumulative decoded length.

The actual executable re-read gives:

| Site | First instruction | Conditional source-predicted displacement |
| --- | --- | --- |
| PRE0x46D5EC | `8b15b8cd5800`, six-byte `mov edx,[0x58CDB8]` | **6**, ends0x46D5F2; following `push edx` stays at its original site |
| POST0x46D5F6 | `b940cb5800`, five-byte `mov ecx,0x58CB40` | **5**, ends0x46D5FB |

Thus the worker's **7/5 is false for the algorithm it cites**. AW's7-byte PRE
window remains a valid *proposed* whole-instruction envelope, not the minimum
or an observed engine result. AZ `validEngineProof` hard-codes7 and would reject
the source-predicted6 even if a producer existed. Do not simply change that
constant: this pass authorizes review only, and actual native strategy is unobserved.

Relocator `303-365` copies ordinary IA32 nonbranch instruction bytes. Backend
`322-335` emits continuation and records saved original bytes/length; `386-429`
emits redirect/padding and restores saved bytes on deactivation. These are source
contracts under default IA32/no-custom-redirect conditions, not native execution.
Wrong architecture, foreign/preexisting redirect, changed bytes, alternative
strategy/build or failed relocation invalidate the derivation.

### Missing getter is not inaccessible evidence

Private `GumFunctionContext` (`gum/guminterceptor-priv.h:49-89`) contains saved
prologue bytes/length and entry/invoke trampoline pointers. The checked QuickJS
and V8 listener method tables expose `detach`, not an actual-span/context getter;
QuickJS attach returns that listener wrapper. This confirms the **limited public
getter absence**, not impossibility of observing memory through existing APIs.

Independent read of the worker-cited backend's **complete** create function found
an omitted alternative: `245-260` writes the context pointer into the trampoline
slice immediately before `on_enter_trampoline`, then emits the entry stub;
`295-335` records `on_invoke_trampoline` and displaced bytes/length. Activation
`407-418` redirects the site to that entry trampoline. Consequently a **candidate
version-pinned, read-only decoder** could follow observed redirect bytes to entry
and context, validate bounded context fields, then read the actual relocated
instructions/continuation and saved originals. Normal bounded memory reads are
the proposed producer, not a caller boolean, custom redirect or modified engine.

This is a concrete source-supported lead, **not a completed verifier**. The exact
private-layout ABI, writer encodings/landing-pad variants, address/read bounds,
loaded engine identity, race handling, flushing and safe read timing still need
review and tests. Do not guess offsets, scan arbitrary engine memory, invoke private
functions, or read after context reclamation. A native receipt would need raw
addresses/bytes and independent host checks; source predictions alone are not it.
The worker's claim that a new getter/build is necessary is therefore not accepted.

## Facilitator check

Own `build/tmp/c3-readiness-review/check_bb.py` authenticates the manifest/clean
checkouts and independently applies the cited threshold to decoded executable
bytes:6/5, with three boundary controls (7-byte threshold, five1-byte instructions,
and the rejected8-byte CALL/POST-overlap case). It explicitly rejects7. This is
bounded independent verification, not production verifier implementation or Frida
execution. Full functions/branches, private fields and both listener method tables
were read; public API absence is not generalized beyond them. No behavioral code
was changed; mutation/restore of a production field is inapplicable.

BA's separate actual-RPC refusal checks and source hash remain intact. Fresh
44/44CTest and25framework tests pass. Seven install hashes match; no game/task
was acquired or created. C3 **0.0635481771 FAIL**, C1 **0.0160864/0.0161212 PASS**
remain retained baselines, with no rendering/gate/reference/mask change.

## Selected disposition: request a staged evidence protocol, not a bypass

**No native readiness under the current implementation.** Recommend a separately
approved, bounded offline verifier pass using the unmodified REA-CLI method and
the read-only source-aware path above, **conditional on explicit approval of the
evidence phases below**. No custom engine build is the default recommendation.

| Phase | Required evidence / producer | What it cannot prove |
| --- | --- | --- |
| Prelaunch/preinstall | Reviewed exact tooling/source provenance, default IA32 relocation/context/transaction safety case for the two sites; complete original/guard snapshots and immediate recheck; verified actual controller and bounded cleanup capabilities | Future installed bytes or successful future restoration |
| Postinstall, before collection | Bounded raw read of actual redirect, context-owned saved originals/length, relocated bytes and continuation; cross-check against prospective6/5 or fail closed; check each installed hook, preserve receipts | That installing an unarmed hook was harmless; a post-check cannot protect the first redirect execution |
| Postdetach/host completion | Independent host readback of complete *actual* spans and guard against preinstall snapshots; flush/unload/session outcome, process/task/slot/install reconciliation | Non-interference, transient correctness, semantic graphics identity or complete history |

This resolves the **ordering of evidence**, not all risks. Before any installation,
the first row must justify the hazard of executing the redirect/trampoline while
unarmed; verified source/contract is distinct from an observed outcome. Blanket
`atomicAttachSafe`/FPU booleans must not become an unverifiable promise. Any
replacement of those assertions by scoped reviewed evidence, movement of actual
proof to a postinstall phase, or accepted residual risk needs the user's explicit
approval. Canonical restoration/ownership/no-retry standards stay intact.

### Next proposed Sol scope and stop tests — not dispatched

1. **One <=45-minute offline verifier task:** resolve supported build/ABI/source
   binding and implement/test only a fail-closed byte/receipt decoder for the two
   default IA32 sites, preserving the runtime refusal. Use source-shaped buffers
   and adversarial single-field inputs to test the decoder, labelled synthetic.
   Include the read-only entry→context→invoke chain, exact displacement correction,
   concurrency/context-preservation safety case and limits. No engine build,
   private invocation, target attach or new API hook. Stop if the chain/identity
   cannot be bounded; do not replace it with a generic success flag.
2. **Only if independently accepted, one <=45-minute integration task:** wire the
   actual AY controller successor, pause handshake, exact level-slot ledger,
   observer/verifier and independent restored-code reader, with <=8s host unload
   deadline and actual owned-resource cleanup. Keep default terminal dry-run and
   fresh artifact-specific readiness authorization. Host scheduled-task drychecks,
   if wanted, need fresh explicit permission (not granted by this review).
3. Reserve up to30minutes independent integrated review. **Then stop.** Native
   execution needs a fresh explicit one-start/one-attach authorization; the stopped
   run's allowance is gone. Matched off/on non-interference remains separately
   unapproved and is not silently made a prelaunch prerequisite.

Proposed cost: up to2hours offline including review; this is a ceiling, not an
estimate of guaranteed success. A narrow cap exception for the parked AZ question
is justified by the newly checked6/5 mismatch and concrete read-only engine-record
route, not by renaming the previous design/mock work. No exception is enacted here.

### Alternatives and falsifiers

- **Keep the present refusal/defer:** safe default if the protocol or narrow cap
  exception is not approved, or private-layout/build/safe-install evidence fails.
- **Just flip the flag / accept attach success / trust proposed7/5:** rejected;
  no actual producer, wrong span prediction and unaddressed installation hazard.
- **Custom getter/instrumented Frida:** not currently justified as necessary;
  changes artifact/method and can certify only the changed build. Requires a
  separate approval and new validation if the read-only path is falsified.
- **Require future removal evidence before first install:** no meaningful
  prospective test; distinguish a proven readback capability from its later result.
- **New debugger/proxy/full-frame logger or pixel fit:** outside scope; no evidence
  here establishes a safer/cheaper substitute.

The recommendation is falsified if actual installed code/engine layout cannot be
joined to a reviewed pin, the memory chain cannot be bounded/read safely, source
semantics fail an independent reconstruction, or a later authorized receipt disagrees.
Any mismatch stays failure/unknown, not an expanded parser policy or automatic retry.
