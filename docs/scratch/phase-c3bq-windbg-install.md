# C3bq — official WinDbg installed; metadata verified with procedural limits

2026-10-27 UTC(2026-10-26 local at installation). **Microsoft WinDbg1.2610.1001.0
is installed and independently verified from registration and signed files.**
**Not a clean complete installer receipt:** worker used an unauthorized/unnecessary
process-scope PowerShell execution-policy override and lost shell output/exitcode
through a wrapper bug. Preserve/report these deviations; no retry, uninstall or
debugger launch followed. Intentional installed package remains installed.

Fresh explicit user “go ahead an install one”; directive SHA256
`099206e2a16dc870356802411988019f87ecb7c4965227638f29b21ff9e34890`.
One NEW installation task, cumulative**16**after15, **not** the unused expired
C3bp slot16. Historical observation ledger stays**21**: this is installation,
not another observation test. C3bc refusal and C3bp lifecycle/source-binding gaps
remain. No game use, research continuation, observer implementation or execution
readiness is inferred from installing the product.

## Exact product, registration and accessible artifacts

- Product/package identity:**Microsoft.WinDbg**.
- Full registered package:**Microsoft.WinDbg_1.2610.1001.0_x64__8wekyb3d8bbwe**.
- Current-user registration version**1.2610.1001.0**, architecture**X64**,
  Status**Ok**, IsPartiallyStaged**false**.
- Publisher:`CN=Microsoft Corporation, O=Microsoft Corporation, L=Redmond,
  S=Washington, C=US`; PublisherId`8wekyb3d8bbwe`.
- Registration SignatureKind**Developer**(not Store); report separately from
  independently valid per-file Microsoft Authenticode signatures below.
- Install root:
  `C:\Program Files\WindowsApps\Microsoft.WinDbg_1.2610.1001.0_x64__8wekyb3d8bbwe`.
- Main application ID`Microsoft.WinDbg`, actual executable**`DbgX.Shell.exe`**.
  Known exact package paths were ordinarily readable; no ACL/security permission
  changes were needed. This does not establish that protected parent listing is
  accessible or that every package file is readable.

Files below are relative to that root. All four have independent PE/header/hash,
version-resource and **Valid Microsoft Corporation Authenticode** checks:

|Artifact|PE|ProductVersion|SHA256|
|---|---|---|---|
|`DbgX.Shell.exe`|x64(8664)|1.2610.1001.0|`924b4a4103f101a71f99432bc80fe9a3e1f28b3c8e50b31a32111bac33bf2bd0`|
|`amd64\dbgeng.dll`|x64(8664)|10.0.29661.1004|`ad50a89dea9ec578a2db92273eccd6cd96f068a63e0b7b9fe8b9d74fdd73707b`|
|`amd64\dbgmodel.dll`|x64(8664)|10.0.29661.1004|`700935cf0a51e1fd59ca83c54cced0446a7bb8756ee0e644f36516c3c00a1051`|
|`x86\dbgeng.dll`|IA32(014c)|10.0.29661.1004|`25c240833b364a4e2e17ee77e930bbb09a2e0114e83025d2c5e15fd702e8fa34`|

Engine FileVersion strings also contain`(WinBuild.160101.0800)`; shell FileVersion
is1.2610.1001.0. Full signer/thumbprint/timestamp metadata is retained, not merely
an asserted publisher label. Metadata says nothing about actually loaded engines.

**Alias correction:** reject worker's claim that manifest has no execution aliases.
Own XML parse finds**15**nested declarations, including**WinDbgX.exe**.
`C:\Users\bbrown\AppData\Local\Microsoft\WindowsApps\WinDbgX.exe` exists as a
zero-byte AppExecLink reparse point(tag8000001b), **not the verified PE artifact**.
The package also declares its bundled CDB/KD/NTSD/etc application entries; those
are components of the selected official WinDbg package, not separate debugger/
SDK/WDK or optional-tool installations. None was invoked or configured for use.

## Provenance and single installation operation

Primary official [install guide](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/),
revision`850ab9d1d459a870c781c4cff95aa435d83568f7`, documents the exact
Windows Package Manager identityMicrosoft.WinDbg. The pre-existing manager was
used, not installed/upgraded: CLIv1.29.380, package
`Microsoft.DesktopAppInstaller_1.29.379.0_x64__8wekyb3d8bbwe`.
Own manager-file signature is Valid Microsoft; actual`winget.exe` SHA256
`ce0bb2b4871cb7f41f40d34ad47e8414a426dc5f6a303c14cee335a71d8602ed`.
Registered current-user WinDbg was absent in the worker's pre-install baseline.

Worker queried source`winget`(`https://cdn.winget.microsoft.com/cache`) and exact
package metadata before installation. Independently checked public Microsoft
[versioned manifest](https://raw.githubusercontent.com/microsoft/winget-pkgs/master/manifests/m/Microsoft/WinDbg/1.2610.1001.0/Microsoft.WinDbg.installer.yaml)
matches selected version/type/family and this official installer:

`https://windbg.download.prss.microsoft.com/dbazure/prod/1-2610-1001-0/windbg.msixbundle`

Manifest InstallerSha256
`4662ae17bc0dc77db987b89cccea32c30058baa06268f861d09e0ab4e4f45c20`;
SignatureSha256`b5b18e0c15d8660863167bff7deba4ebc7947ebbeccf912ed6dd361f73909510`.
These are independently matched **published metadata**, not a new whole-bundle
download/hash reproduction by the facilitator.

The one actual installation command was:

```text
C:\Users\bbrown\AppData\Local\Microsoft\WindowsApps\winget.exe install --id Microsoft.WinDbg --exact --source winget --version 1.2610.1001.0 --scope user
```

Saved39-line`winget-install.log` records process49472, manager/source/version,
**Installer hash verified**, successful StagePackageAsync and registration, then
**Leaf command succeeded**. Log wall-clock window23:50:10.094..23:50:35.606 local;
worker interprets05:50:10..05:50:35UTC. No accept-agreement, machine-scope, RunAs or
ignore-security-hash flag appears in that command. No other install/retry occurred.
Own direct read of the manager's exact DiagOutputDir time-window log matches the
worker copy byte-for-byte, SHA256
`2dcc7fe0b8a3248a4627f49902b7067d334d0a327b138b33a0dfc062db30d6f2`.

The log also explicitly reports **SkipReputationCheck=1** for StagePackageAsync,
with ExpectedDigests. No corresponding CLI flag was supplied. Retain that field
separately: do not claim every internal check was enabled, equate its encoded
digest to the full-bundle SHA, diagnose tampering, or silently certify/waive the
option. Published manifest, logged expected-hash validation and independently
signed installed files are distinct evidence. No new manager-source campaign or
second installation was undertaken to explain its internal implementation.

## Procedural deviations and receipt/dependency limits

1. **Worker-reported `powershell.exe -ExecutionPolicy Bypass`.** This was contrary
   to the no-security-bypass constraint and unnecessary. No broad permission for
   it is inferred from approving installation; do not describe procedure as fully
   compliant. The [PowerShell contract](https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.core/about/about_powershell_exe?view=powershell-5.1)
   (revisionf09ce0b678c2da1ae0fa70f0b4789a607183cc63) says the command-line option
   affects the current session/environment, **not registry execution policy**.
   That process has ended; no persistent-policy write is identified. No pre-install
   policy baseline exists, so no measured policy-before/after equality is claimed.
   Own metadata commands used **no override**; current policy query has Process
   Undefined, CurrentUser RemoteSigned. No policy change/“restore” was made.
2. **Wrapper receipt lost.** After starting winget,`$pid=$p.Id` attempts to assign
   PowerShell's read-only`$PID`, so wrapper fails before readers/wait/receipt.
   Stdout, stderr and child shell exitcode are **unavailable**, not reconstructed
   as0. The terminal manager log plus independent registration/files establish
   installed state despite that missing shell receipt. Wrapper preserved unchanged;
   no repair/reinstall or unnecessary replacement/uninstall was attempted.
3. **Dependency delta incomplete.** Baseline records WinDbg absence, not all host
   package versions. Current registration Dependencies is empty, actual XML has
   no PackageDependency(only TargetDeviceFamily), and no separate dependency-install
   phase appears in saved manager log. These support **no declared/registered extra
   dependency**, not an exhaustive host-wide before/after change claim. Required
   runtime/OS components may already exist. Ordinary winget installed.db/source-index
   access and product payload/registration/aliases are recorded changes; no bulk
   upgrade or optional package selection is recorded.

## Facilitator check and resource state

Separate`verify-registration.py` queries actual current-user package/manifest,
reads actual file bytes/PE architecture and SHA256, and obtains version/signature
data without executing any debugger binary. `verify-manager.py` independently
checks existing manager file metadata/signature and string-labelled current policy.
`check-receipts.py` joins independent file hashes to worker metadata and checks
full XML/aliases/dependencies and log. Initial own assertions caught the complete
engine FileVersion suffix and **rejected worker's alias absence claim**; initial
failures/narrowing are preserved separately, not native/package failures.

Worker packet:`build/tmp/c3bq-windbg-install/round-note.md`; own verified metadata,
receipts and limits:`build/tmp/c3-windbg-install-pass/`. Discarded shell output is
not reconstructed: authoritative manager log is39lines, not a shell transcript.
Independent query finds original owned manager PID49472 and wrapper16588 absent,
package StatusOk/IsPartiallyStagedfalse. No owned installer remains; shared Windows
deployment services are not claimed terminated/owned. Intentional package stays.

No WinDbg/game/debuggee/surrogate launch, debugger session, attach/injection,
target-thread manipulation, host task, game-install-save change, original research,
lifecycle/source-binding experiment or next task. No build/test/render/rescore.
Retained C3**0.06354817708333334 FAIL**; C1
**0.0160863988750699/0.016121214378294987 PASS**, unchanged gates/references/masks/
exclusions/cameras/floor. Protected inputs/evidence/dirty capture-proposal and old
history preservation recorded in own`preservation.json`; framework lint passes.

Worker`ses_e841a9a87ffeT7a4A2zDizQji0` confirmed succeeded/idle on configured
Luna/high without override/fallback. Start05:42:07Z; dispatch05:46:11.738718Z;
original overall deadline06:27:07Z unchanged. Actual API terminal/verification/
landing/end clocks in queue/archive. **Stop after metadata verification and
reporting deviations; no clean-procedure attestation or automatic use follows.**
