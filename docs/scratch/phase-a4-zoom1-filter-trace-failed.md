# A4ab: the instrumented run crashed; no actor-bound filter trace was obtained

Queue `a4ab-zoom1-filter-requests`, worker session
`ses_ea5abd2afffe0z72Fc8T5sP3ol`. One owned stock start, followed by an offline
packet-only correction with **no additional launches**. Corrected packet:
`build/tmp/a4ab-zoom1-filter-requests/round-note.md`; original overclaims remain
in `round-note-initial.md`. Raw capture and hook script are unchanged.

## Verdict

**Failed experiment, no filter answer.** Zero of five requested actor-bound
draw windows and zero request tuples were retained. The owned process died
with an access violation in the D3D wrapper during the hook-load interval.
The specific cause was not isolated. No POINT/LINEAR, effective ALPHAREF,
actor-pointer equality or explanation of A4z's pixel failure is accepted.

The one permitted stock start is consumed. Do not retry this five-hook plan,
or assume a reduced hook set is safe. Offline harness validation is next;
another stock run requires a separately reviewed brief.

## What the raw records establish

- Stock pid **49944** entered `Compiled\MM_CLIMB_002.lvl` and reached paused
  ui_state 1. No actor was bound by the trace; the filtered pre-hook actor
  snapshot is empty. No zoom1-at-draw observation was achieved.
- Five **pre-hook** 16-byte live windows match the authority file, at
  `0x47A75D`, `0x47AB7F`, `0x470F29`, `0x470F86` and `0x470FA3`.
  The authority digest is unchanged. These are preimages, not post-hook safety
  or restoration evidence.
- `frida-events.jsonl` contains five HOOK_INSTALL records. Script inspection
  shows all five announcements occur **before** Interceptor.attach. The REA
  status sidecar has a ready message, supporting script execution reaching
  its end, not individual hook safety or successful target callbacks.
- Fresh facilitator query of Windows Application Error event **1000** matches
  the same process id `0xC318` / 49944, executable path and report id. Fault:
  `D3DImm.DLL` **4.7.1.3000**, exception **0xC0000005**, module offset
  **0x409D0**. This is crash metadata, not a stock-executable virtual address.

Recorded ordering (epoch milliseconds from retained records):

| event | time |
| --- | --- |
| owned launch | 1792517862205 |
| hook announcements | 1792517964883 |
| Application Error record | 1792517965018 |
| capture go step, followed by failed foreground acquisition | 1792517976869 |

The native fault record precedes the failed acquire call, so that later input
error is not evidence that the acquisition helper caused the fault. These
timestamps do not isolate a hook, relocation, callback or renderer mechanism.
The initial claim that D3DImm is entered only by the two hooked call sites was
unsupported and withdrawn.

## Why an empty trace is not a clean callback negative

Setter/request callbacks buffered records in an in-process ring and persisted
them only after target binding. No target binding/draw record survives. Thus
no retained tuples **does not prove the callbacks never executed**. Nor does
the absence of a BIND error show that the binder path was never reached: an
exception before emitting one would also leave no such record.

The sampled `[0x58D53C]==0` was between draws. A4aa already established that
this frame gate is cleared; it is not a readback proving control zoom zero.
This round therefore cannot be described as a completed zoom1 trial.

## Confirmed dormant harness defects (not native-crash diagnoses)

1. `hooks_a4ab.js` u32 returns a JavaScript Number. snapshotActors reads the
   actor-array address with it, then calls **arr.add** on that Number. The
   facilitator's separate `binder-defect.cjs` runs the unchanged script against
   fake Frida objects and a nonempty synthetic list: it reproduces
   **TypeError: arr.add is not a function**. No real process attachment, memory
   or File access is used. This proves an offline harness defect, not whether
   it executed during the stock crash or caused that native fault.
2. At the mid-call TSS sites the script labels esp[0] as retaddr. Existing
   original-byte reads show esp[0] is then the pushed **device argument**;
   function-entry stack conventions differ. No captured tuple is reinterpreted
   because none was retained.

The worker's unmeasured subobject formula on an indirect actor path is not a
replacement for A4s's retained actor+0x128 binding. No new live equality was
observed here; that historical binding is not retracted.

## Restoration, resources and unmet gate

This run retains both **280-byte** level-slot write records at `0x4E35D4`:
before, temporary and post-read payloads, plus source record/group/slot. The
temporary self-copy and final restore match the initial bytes. That closes
this run's entry-helper ledger only; it does not supply A4z's historical missing
bytes.

**Hook unload and live restored-code readback were not achieved.** The target
died, Frida status was detached and no hook-process-bytes.json exists. Clean
disk hashes and process death do not turn this unmet gate into a pass. There
is no surviving owned game to restore, but this was not a successful unload
verification.

Current exe/save/config/shim/DDraw/D3DImm file hashes match all six retained
before/after digests; save/config backups match too. Covered ten source/harness
hashes match the independently checked A4aa baseline. Separate fresh PowerShell
check finds **zero game/webolution processes and zero LemWeb tasks**.

The reported Frida session has no open session when independently queried.
Worker left its read-only Ghidra stock target open; facilitator observed the
authority identity via list_documents and closed that analysis session after
worker termination. No further instrumentation or stock launch occurred.

## Facilitator check (2026-10-20)

`build/tmp/a4ab-facilitator/verify.py` independently parses capture, preimages,
entry writes, Frida status, announcement stream and WER record, verifies current
file hashes and checks prediction predates launch. Results: `verified.json`.
It does not import the worker capture script or start/attach a game.

`checks.py` records the independent binder-defect reproduction, fresh Windows
event-log query, cleanup query and whole build/test invocation. Build needs no
work; **20/20 CTest pass**. These source tests do not validate the instrumentation
or supply the missing native request evidence. No production mutation or pixel
acceptance applies; A4z's full-scale failure and A2/A3/A5m limits remain.

## Next bounded action

Repair and unit-test the actor binder **offline** against typed mock pointers,
with a regression that fails on the original Number.add code. Keep the failed
hook script immutable. Hook relocation/callback safety and trace persistence
remain separate prerequisites before any future stock attempt. Do not promote
function-entry/onLeave values as if they were observed pre-call request tuples.
