# On mouv over SSH: open a visible Windows Terminal in desktop session 1 running a script.
# SSH commands run in session 0, so this goes through a one-shot scheduled task.
# Usage: powershell -File scripts\overlord\Start-Visible.ps1 -Script <path.ps1> -Title <name>
param([Parameter(Mandatory)][string]$Script, [string]$Title = 'overlord')
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$path = (Resolve-Path $Script).Path
$name = 'LemWeb-overlord-' + $Title + '-' + (Get-Date -Format 'HHmmss')
if (Get-ScheduledTask -TaskName $name -ErrorAction SilentlyContinue) { throw "$name exists" }
if (-not (Get-Process explorer -ErrorAction SilentlyContinue | Where-Object SessionId -eq 1)) { throw 'no desktop session 1' }
$action = New-ScheduledTaskAction -Execute "$env:LOCALAPPDATA\Microsoft\WindowsApps\wt.exe" `
    -Argument "-w new --title $Title powershell -NoProfile -ExecutionPolicy Bypass -File `"$path`"" `
    -WorkingDirectory $root
$principal = New-ScheduledTaskPrincipal -UserId ([Security.Principal.WindowsIdentity]::GetCurrent().Name) `
    -LogonType Interactive -RunLevel Limited
$settings = New-ScheduledTaskSettingsSet -ExecutionTimeLimit (New-TimeSpan -Minutes 5)
try {
    Register-ScheduledTask -TaskName $name -Action $action -Principal $principal -Settings $settings | Out-Null
    Start-ScheduledTask -TaskName $name
    Start-Sleep -Seconds 5
    "launched $name"
} finally {
    # Removing the task does not stop the window it started.
    Unregister-ScheduledTask -TaskName $name -Confirm:$false -ErrorAction SilentlyContinue
}
