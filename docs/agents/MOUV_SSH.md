# Running the original on mouv over SSH

Use this when the agent runs on a non-Windows host (this Mac) and a task needs the stock
executable, a Windows build, or a Windows-only harness script. The evidence standard,
live-game ownership and restoration rules are unchanged; this page only covers getting
there. Facts below were checked on 2026-10-19; re-check anything that fails.

## Host

| Item | Value |
| --- | --- |
| SSH alias | `mouv` (in `~/.ssh/config`, key `~/.ssh/id_ed25519_mouv`) |
| Machine / user | `DESKTOP-MOUVNOP`, `bbrown` (Administrator), Windows 11 |
| Remote shell | `cmd.exe` |
| Repo checkout | `C:\progs\lemmings-webolution`; on the Mac this same tree is `~/mouv-progs/lemmings-webolution` (sshfs) |
| Stock game | `C:\progs\Lemmings Revolution\Lemmings Revolution.exe` (the path harness scripts hard-code; shim `dinput.dll` is installed beside it) |
| Scratch | `C:\progs\lemmings-webolution\build\tmp` (gitignored) |
| Python | `python` = 3.13, has `capstone` 5.0.7 and `frida` 17.24.3 (user site-packages, c1an; the 2026-10-24 directive keeps it) |
| MSYS2 | `C:\progs\msys2` (`ucrt64` for the project, `mingw32` for `tools/dinput-shim`) |
| REA | `rea` on PATH (npm global); Ghidra 12.1.4 under `C:\progs\ghidra` |

Key setup and REA background: `~/lemweblocal/NOTES.md`. Never write the password anywhere.

## Commands

```bash
ssh -o BatchMode=yes mouv "hostname && whoami"
ssh mouv 'cd /d C:\progs\lemmings-webolution && git status --short'
ssh mouv 'cd /d C:\progs\lemmings-webolution && powershell -NoProfile -File scripts\Invoke-Ucrt64.ps1 "cmake --build --preset dev-debug"'
ssh mouv 'cd /d C:\progs\lemmings-webolution && python tools\harness\read_steppers.py --at 0x00430BA1 --len 0x90'
```

- Use `-o BatchMode=yes` and a `timeout` so a prompt or a hang fails instead of blocking.
- Single-quote the remote command in zsh so `$` reaches PowerShell untouched. Anything
  longer than one line: write a `.ps1`/`.py` into `build/tmp` through the mount, run it with
  `powershell -NoProfile -ExecutionPolicy Bypass -File ...`, and delete it afterwards.
- The remote shell is `cmd`: use `cd /d`, backslashes, `&&`. `query`/`quser` are absent.
- Screenshots and logs written under the repo are readable on the Mac through the mount.
- The PATH traps in the [build rules](../../.claude/rules/windows-msys2-build.md) still apply on
  mouv: harness Python must not see UCRT64 `bin`.

## The session-0 trap

An SSH command runs in **session 0**, which has no desktop. The logged-in desktop is
session 1 (`explorer.exe`). Offline work — byte reads, REA, builds, `ctest`, level dumps —
is fine from SSH. Anything that launches the game, owns a window, sends input, takes
foreground or captures frames must run **in session 1**. Started from SSH it either fails
or runs invisibly, which can look like a real negative.

Launch into session 1 with a one-shot interactive scheduled task, from a script in scratch. Use a unique task name, refuse to replace an existing one, write results to a
file and poll for it, and unregister in `finally`:

```powershell
$name = 'LemWeb-<round>-<run>'
if (Get-ScheduledTask -TaskName $name -ErrorAction SilentlyContinue) { throw "$name exists" }
$action = New-ScheduledTaskAction -Execute 'python.exe' `
    -Argument 'tools\harness\<script>.py --out build\tmp\<run>.json' `
    -WorkingDirectory 'C:\progs\lemmings-webolution'
$principal = New-ScheduledTaskPrincipal -UserId ([Security.Principal.WindowsIdentity]::GetCurrent().Name) `
    -LogonType Interactive -RunLevel Limited
$settings = New-ScheduledTaskSettingsSet -ExecutionTimeLimit (New-TimeSpan -Minutes 10)
try {
    Register-ScheduledTask -TaskName $name -Action $action -Principal $principal -Settings $settings | Out-Null
    Start-ScheduledTask -TaskName $name
    # poll for build\tmp\<run>.json, with a deadline
} finally {
    Unregister-ScheduledTask -TaskName $name -Confirm:$false -ErrorAction SilentlyContinue
}
```

This was checked live: the task reported session 1 and left no task behind. It needs
`bbrown` logged in at the desktop. If no `explorer` runs in session 1, stop and report it;
do not try to create a logon. Check the task's own exit and output file. "Task started"
is not "probe ran". Use `-RunLevel Highest` only when the probe needs elevation (for
example, the debugger).

## One tree, two views

- `~/mouv-progs` on the Mac is a macFUSE/sshfs mount of `mouv:C:/progs`. The repo there
  is the same working tree as `C:\progs\lemmings-webolution`, not a separate clone. An edit
  on the Mac is on mouv immediately, so nothing needs syncing or copying. Its uncommitted
  changes may belong to another session or to the user. Do not `checkout`, `reset` or
  `stash` without asking.
- Edit files through the mount. Run builds, `ctest`, harness scripts and heavy `git`
  commands on mouv over SSH. Over sshfs they are slow, and the Mac's toolchain is not the
  project's.
- macOS writes `._<name>` AppleDouble files beside files created through the mount. Delete
  the ones you create (`fd -H -I '^\._'`) before reporting. They are not project files,
  and Claude Code even lists `._*` command files as skills.
- Tools that write to a temp file and rename it into place (`sd`, some formatters) fail
  over the mount with "Operation not permitted" and leave `.tmp*` files behind. Edit with
  the editor tool or an in-place write, and delete any leftovers.
- If local commands fail with `ENXIO`, or reads report unresolvable paths, the mount has
  dropped. Check `mount | rg mouv-progs` and ask the user to remount. Do not treat the
  failure as missing files.

## Live-game etiquette on mouv

- Session 1 is the user's real desktop. Before launching, check
  `Get-Process | Where-Object { $_.ProcessName -like '*Lemming*' }` and the queue's
  live-game owner. One owner at a time; never kill a game you did not start.
- Automatic foreground acquisition is authorized (`acquire_game_foreground`). Keep it and
  the input batch in the same session-1 worker, as the evidence rules require.
- Verify cleanup from a separate SSH call afterwards. Check the game process state,
  hardware breakpoints, restored writes, the stock exe/`SavedGame.dat`/`dgVoodoo.conf`
  hashes the capture scripts record, and that no `LemWeb-*` scheduled task is left.
- Delete the scratch scripts you wrote. Leave result files only where the report cites them.

## Frida through the REA CLI (c1o, 2026-10-24)

REA bundles Frida (`rea@6.5.0`, provider Frida). This recipe does not need Python `frida`. **Updated 2026-10-24:** mouv's `python` imports `frida` 17.24.3 from the user site-packages (c1an). Launch-attach probes use that module. Do not uninstall it. **MEASURED:** `rea instrument-with-frida attach`
run from an ssh (session 0) worker attached to a session-1 process, the hook reported hits, rea detached itself
(`cleanup_error: null`) and the target stayed alive. The facilitator re-ran it on a fresh target (hook hits n=1..3, target
`Responding True` after detach, no `LemWeb-*` task, no Lemming process). So no session-1 task is needed for the attach itself.
Untested: elevation sensitivity, other integrity levels, spawn mode, and attaching to the stock game (never done).
Worker note: `build/tmp/c1o-frida-preflight/round-note.md`.

1. Preconditions: an `explorer` in session 1; no `LemWeb-*` task; no `*Lemming*` process.
2. Start a harmless session-1 target with the one-shot task above (`pythonw.exe`, so no console window): a loop calling
   `kernel32!Sleep(200)`, writing `pid session` to a file and exiting when a stop file exists.
3. Hook script (`hook.js`): `Process.getModuleByName('kernel32.dll').getExportByName('Sleep')`, `Interceptor.attach` with
   `onEnter` logging the first three hits, and a `HOOK-INSTALLED` line.
4. Attach from ssh, with the script path relative to the `cd /d` directory:
   `rea instrument-with-frida attach <PID> --script-file hook.js --duration-ms 3000 --format json`.
   `console.log` lines print as plain text before the JSON envelope; require `cleanup_error: null`.
5. From a separate ssh call, check `Get-Process -Id <PID>` is alive; then create the stop file and confirm the PID is gone.
   Verify no `LemWeb-*` task and no Lemming process.

A hook on the game is a different step: it needs its own brief, a start budget and the c1l restoration checks.
