# C3br — one actual IA32 CDB trial: failed validation, no retry

2026-10-27. **FAIL:** later-thread hardware marker hits were observed, but the
predicted initial-thread marker hits were not. The harness also continued past
the actual cleanup stop without disarming, invalidating clean-shutdown acceptance.
This is one surrogate result, **not** original-game evidence or a universal
main-thread debugger defect. No retry, repair dispatch or original-game use follows.

Directive SHA256`4ab7cff897bd14a5266efdb2102815fb6d75bfadd2050cd4cab3fc9b82b517d8`;
fresh cumulative task**17**after16. Actual debugger sessions**1**, newly built
surrogate starts**1**, stock starts/attaches/injection**0**. Preserve historical
observation/feasibility**21**separately; append this single surrogate validation,
not another stock observation. C3bc refusal and source-binding/readiness parking remain.

## Identities and the sole launch

Official installed package`Microsoft.WinDbg_1.2610.1001.0_x64__8wekyb3d8bbwe`,
version1.2610.1001.0, independently still StatusOk/nonpartial. Selected **bundled
x86 CDB**, not the WinDbg GUI, another family, custom engine or frontend retry.
Root:`C:\Program Files\WindowsApps\Microsoft.WinDbg_1.2610.1001.0_x64__8wekyb3d8bbwe`.

|Relative artifact|PE|ProductVersion|SHA256|
|---|---|---|---|
|`x86\cdb.exe`|014c|10.0.29661.1004|`238826866df36bc2e7f35d2369bab675b9ae4f027f2e7f2342d2df2496cbd3aa`|
|`x86\dbgeng.dll`|014c|10.0.29661.1004|`25c240833b364a4e2e17ee77e930bbb09a2e0114e83025d2c5e15fd702e8fa34`|

Both actual files independently read/hashed, versioned and **Valid Microsoft
Corporation Authenticode**; manifest has the exact CdbX86 application path.
Banner`Windows Debugger Version 10.0.29661.1004 X86`agrees. **Actual loaded engine
module path was not enumerated**; adjacent file authentication is not that claim.

Surrogate`build/tmp/c3br-windbg-surrogate/surrogate.exe`SHA256
`b334796aa00d5ce3e737251ea7d8237c6fa874cad52016e6be9833f3d4b649db`.
Independent fresh PE parser: Machine014c, PE32, base00400000, DYNAMIC_BASE absent;
export`marker`RVA15b0/VA004015b0,`cleanup_marker`RVA1610/VA00401610. Actual first
byte at both entries is55(`push ebp`), matching debugger output, not patchedCC.
Source SHA256`7878fff8d4d410b24f7269b0604ad53be60f67f08f4626112d6c4d8de1e119fd`.
Complete51-line source read: main calls marker three times, creates later thread
with three calls, joins, then calls cleanup marker and logs return intent0.
Existing mingw32 GCC16.1.0 was used via documented wrapper, no compiler install.
Failed compile attempts are retained; compiling did not execute the target.

Single launch at**06:38:38.401032Z**, repository-root working directory:

```text
<package-root>\x86\cdb.exe -logo <scratch>\debugger.log -cf <scratch>\initial.dbgcmd -sins <scratch>\surrogate.exe
```

`<scratch>`is`C:\progs\lemmings-webolution\build\tmp\c3br-windbg-surrogate`.
Receipt saves the complete absolute argv and debits allowance before Popen.
Debugger PID**37312**, creation FILETIME**134375567184265661**; target PID**46328**
(b4f8), main TID**42232**(a4f8), later TID**22736**(58d0).
Command file is exactly`ba e1 004015b0`, `ba e1 00401610`, `bl`: two processor
execution observations, no user software breakpoint/injection/manual context writes.

## Prediction versus raw observations

Saved prediction/preflight precede launch according to retained filesystem times
(not an independent trusted prelaunch timestamp). Predict six marker stops, three
per intended thread, then cleanup; disarm/list while stopped, continue, observe
actual target exit and debugger return. Limits1session/1start,2slots,7expected
hardware hits,10events,90s live window; unexpected stops/errors should abort.
Primary Microsoft startup/ba/CDB/bc/exit documentation revisions are in preflight.
Prior B1u/A1 failures remain, not explanations of this run.

|Phase|Target durable body executions|Hardware stops recorded|
|---|---|---|
|main marker|3 on TIDa4f8|**0**|
|later marker|3 on TID58d0|**3**, breakpoint0/EIP004015b0|
|post-join cleanup|1 on main TIDa4f8|**1**, breakpoint1/EIP00401610|

Full196-line`debugger.log`: startup lines49–54 show ordinary
`ntdll!LdrpDoDebuggerBreak`, exception80000003,`int 3`at777991a8. **Not** a user
hardware stop or a software-breakpoint fallback. Hardware hits lines72/90/108
each show EIP004015b0 and selected native TID58d0; cleanup hit126 shows EIP00401610
and main TIDa4f8 at133. Main cleanup hit precludes a blanket “hardware breakpoints
never work on main” claim. Initial marker misses have **no independently established
cause**. Arguments/stack values were not captured: source/log establish phase/order,
not direct per-stop argument inspection. An image-load diagnostic at58 is retained.

## Cleanup failure and narrowed resource result

Launcher blindly loops seven prompts and clears only at`number==7`, rather than
classifying stop address/thread or aborting mismatches. The actual cleanup was
the **fourth hardware stop**. Enabled listings remain at140–141, and`g`at142
continues without disarm. It then reports NtTerminateProcess,`No runnable
debuggees`at158, and repeats`g`errors. `bc *`is only at182, followed by empty`bl`
at183. **Late empty listing cannot prove live-stopped disarm.** The predeclared
unexpected-event/error stop contract was not implemented; do not blame this
controller defect on the debugger or claim clean exit/detach acceptance.

Raw target log ends`EXIT_INTENT code=0`; debugger has **no actual exit-code
report**. Receipt retains`process_returncode=null`,`target_exit_code=null`and
timeout error. Stale`status="process created; session active"`is historical,
not current liveness. 90s is a declared controller window, **not** verified exact
target/debugger lifetime; receipt end-state check is06:40:31.847285Z(113.446s
after launch). File-backed stdout/stderr exist(stderr empty); exit receipts are
missing. Normal exit, graceful detach, continued post-detach execution and actual
exit mechanisms/codes remain **unknown/untested**, not promoted from process absence.

Worker reports no termination command. Own read-only exact-PID query at
**06:48:54.8739167Z**finds37312/46328 absent, independently confirming no remaining
original owned processes at that check. No process control was needed; no reused
PID was treated as owned or touched. Worker API confirms succeeded/idle on its
configured pin. This resource-absence result is **not** graceful-cleanup proof.

Startup prints gallery defaults`AllowNugetExeUpdate:true`and permission for
credential-provider installation; UserExtensions0/LocalInstalled46 are logged.
These flags are not proof an update/install occurred. No explicit update/install
command is in saved harness/command transcript; no full before/after per-user
extension inventory exists, so do not certify a complete host-state delta.
No ExecutionPolicy override/security/ACL/elevation/host-task/reboot operation was
reported or present in the saved scripts; the earlier install override is not precedent.

## Facilitator check and stop

Own bounded offline checker reads source/controller/command/receipt/raw target
and debugger output, parses all hardware hit/TID pairs and cleanup ordering, and
independently parses PE headers/export addresses/bytes. Separate ordinary
no-policy-override metadata query authenticates exact package/frontend/adjacent
engine files. **Never executes target, debugger or worker launcher again.**
No repair, second session, original-executable research/launch/input/attach/write,
source-binding continuation, observer/renderer/framework change or other research.
No production change; mutation/rebuild/render/rescore would not verify this failed
surrogate trial and were not run. Existing gate/reference/mask/exclusion/camera/
floor remain: C3**0.06354817708333334 FAIL**; C1 zoom1/elev_down
**0.0160863988750699 / 0.016121214378294987 PASS**.

Worker raw packet:`build/tmp/c3br-windbg-surrogate/`; facilitator independent
review/identity/resource/preservation manifests:`build/tmp/c3-windbg-surrogate-pass/`.
Raw evidence remains in shared ignored scratch, not implied to sync through Git.
Verified raw SHA256:

|File|SHA256|
|---|---|
|debugger.log|`1fad8336aaf19d058a448ca7b8e655980bc20af7f5ccb764c03ab1f0a7a9f9ac`|
|target.log|`b0e99dbcea317f910eb3f65540179b2f424e4e9db64a11495708aa64870407b2`|
|launcher.py|`b3e215675d8ef64a72c96314e002970620aa08e088430e81e5fb5ffb0d726a96`|
|launch-receipt.json|`1797a9486e137c4882cf36bcc530f740a715745f34f0e2048bda2a5042ad3137`|

Queue/archive hold actual clocks/counts/terminal state. Start**06:22:02Z**;
dispatch**06:28:16.772171Z**; original overall deadline**07:07:02Z**unchanged.
Protected package/game/prior evidence and all unrelated dirty deltas preserved
by baseline manifests; selective staging excludes those unrelated changes.
**Stop with failed validation and retained parking; authorization consumed.**
