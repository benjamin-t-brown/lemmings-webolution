# C3bc initial/repair1 — source-supported installation-safety blocker

2026-10-26. Complete negative packet; **no accepted verifier or integration**.
Initial worker note is preserved at `build/tmp/c3bc-verifier/round-note.md`;
repair evidence is `build/tmp/c3bc-verifier/repair1/addendum.md`. Two attempts
including repair exhaust this directive's allowance. No native activity followed.

## Finding and limits

The pinned Gum source exposes a concrete prospective installation-safety gap,
not merely the previously missing public getter. Under reviewed Windows IA32
source conditions, the normal non-code-signing-required transaction reaches
the direct RWX update path:

- `gum/guminterceptor.c:577-644,1705-1840`: attach batches changes under the
  interceptor mutex and transaction-end dispatches page updates. This mutex is
  not a guarantee that other target threads cannot execute original code.
- `gum/gummemory.c:344-360,465-502`: Windows source reports full RWX support.
  The **earlier remap branch must also be checked**: Windows implementation
  `gum/backend-windows/gummemory-windows.c:126-130` returns `FALSE` for
  `gum_memory_can_remap_writable`, resolving the branch omitted by the worker.
- `gum/gummemory.c:674-756`: the selected path enables writable executable
  pages, invokes the update callback at their original addresses, restores
  protection and clears the cache; it does not enumerate/suspend other threads.
  The later fallback's suspension at `817-820` does not protect this branch.
- `gum/backend-x86/guminterceptor-x86.c:386-429` writes the redirect and
  padding, or restores saved bytes. Neither transaction batching nor a later
  matching readback establishes safe concurrent first execution during this write.

This is a **source-conditional unresolved hazard**, not an observed torn patch,
crash diagnosis, claim about every Frida build/platform, or proof no safer
protocol exists. No target-specific quiescence or independently supported atomic
installation guarantee was established. The code-signing-required update arm
also does not supply explicit thread quiescence in the checked function, but its
actual use is unknown; it is not evidence of a safer alternate strategy.

The full x86 callback prolog/epilog **does emit FXSAVE/FXRSTOR**, flags and
general-register save/restore (`guminterceptor-x86.c:534-692`). The initial
worker's isolated `pushax` comment is not a basis for an FPU-preservation defect.
This positive source evidence is not a complete loaded-agent/context attestation.

The source/private-record route remains possible in principle. Local package
and native-binding hashes identify inspected artifacts, not the particular
injected payload or its private ABI. Core's Windows source has both embedded
architecture-specific blobs and a non-embedded agent path. Exact supported
artifact/source/layout binding remains incomplete; **no blanket reproducible-build
requirement is introduced**. A loaded-instance identity check would belong to a
later separately authorized stage, not an instruction to attach now.

Actual redirect/displacement/saved-original/relocation receipts are deliberately
**postinstall-before-collection** evidence under the approved staged protocol.
They are not a prerequisite to writing or synthetic-testing an offline decoder.
Likewise actual restoration is postdetach evidence. Initial packet language
suggesting their absence alone prevents an offline parser is not accepted.
The demonstrated prospective safety gap, and the directive's stop condition,
are why no speculative parser or integration was pursued here.

## Facilitator check

Returned the initial shallow prerequisite assessment for one bounded repair,
then independently read the concrete attach/transaction/page-update/backend
paths, complete context-save thunks, private context declaration, and Core's
agent-resource construction/materialization paths. Authenticated the worker's
six additional-source hashes, installed package/binding/declarations and both
clean pinned checkouts. Independently checked the missing Windows remap predicate
above, so the RWX conclusion does not ignore a preceding strategy choice.

`build/tmp/c3-offline-verifier-pass/check_bc.py` records the bounded independent
source/file assertions in `independent-bc.json`. Re-read the executable's whole
65-byte guard into `site-read.txt`: PRE `0x46D5EC` is
`8b15b8cd5800`, POST `0x46D5F6` is `b940cb5800`; default-source prediction
remains **6/5**, not observed installed spans. The checker's first run failed on
Windows default text decoding; the UTF-8 retry passed. Both logs are retained.

Re-executed the unchanged actual observer RPC under the existing independent
Node VM trap checker: handshake not ready, three setup refusals, **zero native
API accesses**, original observer SHA unchanged. This is offline refusal testing,
not Frida execution. No behavioral code was changed; mutate-one-field testing
is inapplicable to this source-review negative result.

Fresh authoritative wrapper build and **44/44 CTest** pass; **25/25 framework
tests** and brief lint pass. Seven install hashes match the earlier accepted
baseline. Independent read-only host check finds no Lemmings process or LemWeb
task. All sixteen initial dirty files were checked byte-identical before our
document landing; unrelated edits are excluded from the round commit.

C3 **14934330/235008000 = 0.06354817708333334 FAIL**; C1 zoom1
**285083/17721990 = 0.0160863988750699**, elev_down
**285700/17721990 = 0.016121214378294987 PASS** are retained baselines, not
rerenders. No renderer, gate, mask, reference, comparison schema or floor change.
Zero starts/attaches/injection/host-task execution/install writes/package installs.
No installation occurred, so no live restoration claim is made.

## Stop and resume condition

Park this route; do not dispatch integration, a third attempt or a live follow-on.
Recommend retaining refusal until a separately scoped direction establishes a
reviewed artifact-specific installation/synchronization guarantee and supported
source/layout binding. A prospective safety case must precede the first redirect;
later receipts cannot repair it. Do not silently accept risk, add a new hook
method, build a custom engine, or require matched off/on testing as an invented
prelaunch condition. Any native start/attach or host-task drycheck requires fresh
explicit authorization. Exact pass/dispatch/repair/landing clocks are in the queue.
