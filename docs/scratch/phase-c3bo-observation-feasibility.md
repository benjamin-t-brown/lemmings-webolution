# C3bo — conditional no-redirect observation proposal

2026-10-26 local. **Concrete documentary candidate, NOT launch-ready or approved.**
Use an established WinDbg **processor execution breakpoint**, size1, at the retained
call instruction`0x0046D5F3`, rather than installing a Gum code redirect. This
addresses the particular concurrent first-redirect hazard by **not installing that
redirect at all**. It does not certify the entire debugger setup/exit protocol,
native source attribution, non-interference or renderer fidelity.

Fresh proposal-only directive SHA256
`04043694a907e2afaa0e6560b203ffd741b99bb14be7664327494bb6f76d4125`.
**C3bc operational refusal remains unchanged.** No implementation, debugger,
start/attach/injection, target/thread manipulation, host task, installation,
surrogate run or game/install/save write occurred or is authorized by this result.
Orientation remains parked; no correction preapproval triggered or exercised.

Worker packet:`build/tmp/c3bo-observation-feasibility/round-note.md` and
`evidence-index.md`; hypothesis/falsifier persisted in`prediction.txt`.
Own independent review:`build/tmp/c3-observation-feasibility-pass/independent-docs.md`,
`check-files.py`, `independent-files.json`. The proposal is landed **separately**;
historical`RENDER_CAPTURE_PROPOSAL.md` and its unrelated dirty edits stay untouched.

## Prospective mechanism — more than an API name

Microsoft's [processor-breakpoint documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/processor-breakpoints---ba-breakpoints-)
(revision`81d06fd79dd36cdd74ec0b3aeed6c4e52c2e2fc3`) states:

- “Processor breakpoints are stored in the processor's debug registers.”
- Execute type triggers when the processor retrieves the instruction at the address.
- Software breakpoints, in contrast, temporarily replace the location with a
  break instruction. The documented alternative for unmodifiable executable code
  is the processor execute breakpoint.

The [ba command contract](https://learn.microsoft.com/en-us/windows-hardware/drivers/debuggercmds/ba--break-on-access-)
(revision`e618b55a431869daca39a8c0fc5598e89956f8ae`) expressly includes **user mode**,
x86, execute type and **Size1**. Thus generic user-mode/x86 support is documentary
evidence, not an unresolved API-name inference. **No exact installed debugger
version/configuration or IA32-on-WOW64 operation was inspected or tested here.**
Manual editing of debug registers is discouraged by Microsoft and is not the
proposed implementation; use only an explicitly reviewed established debugger.

[Windows debugging events](https://learn.microsoft.com/en-us/windows/win32/debug/debugging-events)
(revision`3aee48ac3f48e187d17bdfd56951741737331749`) says notification suspends **all
threads in the affected process** until ContinueDebugEvent. Its CREATE_PROCESS
event precedes process user-mode execution; CREATE_THREAD precedes the new
thread's user-mode execution. Conditional protocol premise: configure at a real
OS debug-event stop, account for existing/new threads before continuing, observe
at the hardware event, and use the debugger's established continuation/disarm path.
No claim that enumerating/suspending running threads is atomic is needed or made.

Under those conditions, observation at the selected site needs **no inline
instruction write, trampoline or displaced-instruction relocation**. A thread
cannot fetch a half-written observation redirect there because none is written.
This does **not** repair or approve the old Frida installation path, nor imply
that debugger operation has no other writes, traps, timing effects or failure modes.

## Facilitator narrowing: startup, source binding and exit remain separate

Independent primary-page review confirms the mechanism but adds limits omitted
or insufficiently explicit in the worker recommendation:

1. **Initial-stop caveat.** Microsoft's processor page says the initial user-mode
   breakpoint cannot be a processor breakpoint; mouse-created source/disassembly
   breakpoints are software breakpoints. Do not claim hardware-only setup just
   from a`ba` observation. Prefer a **prospectively reviewed launch-under-debugger
   CREATE_PROCESS event** as the documented setup-stop premise, not an assumed
   hardware initial-main breakpoint. Whether the chosen WinDbg exposes/handles
   that stop/configuration, its default startup traps and any incidental changes
   still need artifact-specific review. No launch, attach/breakin or software
   breakpoint fallback is authorized here. This is not a claim that every initial
   debugger breakpoint patches game code.
2. **New site is not old PRE.** Fresh read of the authority's65-byte call window
   confirms`46D5F3 ff5170` and the device push at`46D5F2`. At this stop six stack
   words start at EBP-0x2C: device/topology/FVF/vertices/count/flags. Old AW PRE had
   five words at EBP-0x28. Reusing its five-word decoder unchanged would be wrong.
   ECX is the vtable, not a thiscall argument. Original stack/source facts remain
   conditional static evidence, not observed debugger context/ABI.
3. **Source identity is unfinished.** [C3aw](phase-c3aw-site-preflight.md) supplies
   a ten-frame primary kind5/7 descriptor/node/ordinal discriminator; it explicitly
   **does not map to durable LVL asset/file offsets or repair4 root membership**.
   A future minimum sandy-qualified sample needs a prospectively reviewed unique
   loaded-asset/source binding to6b72b/596c7, not just a matching vertex, screen
   location, material, runtime pointer or ordinal. No such mapping is implemented
   or certified here. An unqualified first-call sample alone would **not answer
   the sandy-terrain question**.
4. **Entry is not draw success.** One hardware stop before the indirect call can
   record an attempted submission request/CPU-path reachability, not successful
   API execution, HRESULT, GPU admission or effective device state. A paired return
   observation is additional scope; it is not silently part of this one-site proposal.
5. **Exit policy matters.** [DebugSetProcessKillOnExit](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-debugsetprocesskillonexit)
   (revision`9267262487657894a8af112d7165006fed5035a7`) documents default TRUE
   terminates debuggees on debugger-thread exit; FALSE detaches, and the API requires
   a debug connection already established. This is **not evidence of WinDbg's
   actual policy**. Exact tool timeout/disconnect/exit behavior and permitted target
   termination must be reviewed/explicitly decided before any future launch.
   “Close debugger” is not proven graceful cleanup; do not implement a new policy
   or context-writing engine from these API descriptions.

## Minimum useful evidence and conditions — not an execution recipe

Gate served: [C3bn](phase-c3bn-terrain-image-link.md) links the right bank to software
record6b72b/node596c7, indices218/216/219, but not an original native counterpart.
The useful target is **at most one source-qualified native call-boundary fan**,
with raw source-chain/descriptor/node/ordinal data and a unique reviewed asset
binding, call arguments and complete bounded vertex bytes. This could distinguish
whether a corresponding native CPU path reaches the boundary and what coordinates
it requests. It cannot alone identify original visible pixels or fix C3.

|Evidence stage|Required distinction / condition|
|---|---|
|Prospective safety|Exact established debugger/version and supported Windows/IA32/WOW64 mode, documented hardware execute observation with no code-redirect fallback, setup/debug-event handling, debug-register slots/conflicts, all relevant thread lifecycle coverage and bounded exit policy. No assumption that processor coverage means thread coverage.|
|Source/layout/artifact|Authority hash/base/call bytes, actual mapped image and architecture; exact call-site stack/context layout and bounded readable caller chain; unique loaded-asset/source binding to selected record. A64-bit CONTEXT definition is not the IA32 layout. No blanket reproducible-build requirement.|
|Deferred observation|Actual hit/thread/IP, context and source association, payload completeness/length/hash, unknown/failed/filter/drop counts, actual event continuation outcome. Real installed/observed values cannot be demanded as inputs to this offline proposal.|
|Deferred cleanup|Disarm debugger-owned state, release owned stop/event, independently reconcile code/context changes and process/session outcome under the approved policy. Hardware breakpoint readback is not retroactive non-interference proof.|

Reuse AW's bounded-payload concept, not its decoder unchanged: proposed ceiling128
vertices/4096payload bytes, total8192read bytes per candidate; treat these as
design bounds, not measured safe debugger rates. A future brief must predeclare
stop/filter/time/payload limits and accept at most one qualified record. If source
association or coverage fails within that allowance, return unknown and stop—no
full-frame census, unbounded event loop, alternate engine or automatic retry.
Raw specular/history, resources/versions, preceding state, actual camera/frame match
and wrapper translation remain unknown unless specifically observed. The boundary
is original game→wrapper arguments, not downstream D3D11 output.

Each stop can perturb timers, rendering cadence, exception handling, scheduling,
thread races and device behavior. Hardware debug state can conflict with other
users. Single-site sampling omits other source kinds/routes and return status;
misses do not prove absence. Matched off/on non-interference is a separate future
experiment, **not a newly invented prerequisite to writing this proposal**.

## Exact future permissions — none granted now

Before any execution request, review the specific debugger artifact/mode, startup/
exit handling, changed call-site layout and durable source mapping. Then a **new
explicit authorization** would have to name one exact stock target/hash, one
debugger launch versus attach method (launch-event premise preferred here), bounded
session/start count, processor execution site/type, process/thread/debug-event
control, debugger-managed debug-register changes and context/memory reads, chosen
frame/navigation/input scope, payload/event/time caps, allowed cleanup including
termination risk, and no-retry stop criteria. Configuration failure, wrong bytes/
mode, lost thread coverage, unreadable/ambiguous source or payload, unexpected code
change, target exception/crash, timeout or inability to release stops all fail closed.

No approval for installation/update, injected observer/hook/proxy, software code
breakpoint fallback, custom engine, task operation, game/install/save or level-slot
write, reference replacement, broader capture or additional target follows. Any
such necessity requires **separate explicit scope**; do not revive stale allowances.

## Independent checks, C3bc positives and historical accounting

Freshly read primary pages/revisions and rechecked the retained C3bc source pins:
Windows writable remap FALSE, direct RWX apply branch has no thread suspension,
FXSAVE/FXRSTOR remains present. Source-conditional safety gap, **not observed crash**.
Local package/tag/source identity still does not attest an injected agent/private
ABI. That old route's binding debt is retained, not imposed automatically on a
different debugger route; this candidate has its **own** context/artifact/source debts.
Postinstall redirect/relocation receipts and postdetach restoration remain deferred
for any future inline-hook protocol, not prospective first-write guarantees.

Own file-backed IA32/PE/site read and source assertions pass; no native API, target,
candidate executable or surrogate was executed. No build/CTest, refusal-RPC rerun,
mutation or render is claimed for this document-only pass. Framework lint passes.
Two guessed Microsoft URLs returned404; the needed clauses were obtained from
the cited actual primary pages, no broader search. Initial snapshot/lint/dispatch
shell produced all completion outputs before an outer10s timeout; no reset/retry
of ownership/dispatch occurred, and exactly one worker was launched.

Cumulative **14**, prior13 retained. Separately retain post-pivot historical
observation/feasibility work: AT/AU/AV6attempts, AW/AX3, AY/AZ5, BA/BB3,
BC initial+repair2 = **19 prior**; this proposal is the20th in that historical
chain, not a reset. Earlier mixed C3 runs16/3 and all older archive entries stay
intact, without relabeling every mixed round as observation. Full unchanged
history snapshot:`build/tmp/c3-observation-feasibility-pass/historical-C3-runs.json`.
No unused authorization/start allowance carries over.

## Retained gates, preservation, resources and stop

No new render or rescore: retained C3**0.06354817708333334 FAIL** against0.02;
C1 zoom1/elev_down**0.0160863988750699/0.016121214378294987 PASS**. No gate,
reference/mask/exclusion/camera/floor change.368fixed/source/install inputs,
2105selected prior evidence files and9unrelated dirty paths preserved outside own
isolated PLAN status/link and Phase C append; historical capture proposal exact.

Worker`ses_e844a2f61ffe3UrVDkTJXdTxnc` confirmed succeeded/idle on configured
Luna/high, no override/fallback. **Zero target/candidate execution or owned native/
debugger/process/thread/task resources**; no installation or write to restore.
No host process/task inventory was performed; this is ownership/operation evidence,
not a claim to have inspected every host process. Start04:50:16Z, dispatch
04:54:15.426358Z; actual API idle/verification/landing/end clocks in queue/archive.
Unchanged overall deadline05:35:16Z; one attempt, no repair/follow-on. **Stop at
conditional proposal; C3bc refusal intact, no end-to-end native readiness claimed.**
