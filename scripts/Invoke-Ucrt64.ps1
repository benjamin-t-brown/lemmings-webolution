# Run a command in MSYS2 UCRT64 from PowerShell.
# Usage: .\scripts\Invoke-Ucrt64.ps1 "make -C src test"
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$Command
)

function Test-Msys2Root {
    param([string]$Root)
    if (-not $Root) { return $false }
    return Test-Path -LiteralPath (Join-Path $Root 'msys2_shell.cmd')
}

function Resolve-Msys2Root {
    if (Test-Msys2Root $env:MSYS2_ROOT) {
        return $env:MSYS2_ROOT.TrimEnd('\', '/')
    }
    $shellOnPath = Get-Command msys2_shell.cmd -ErrorAction SilentlyContinue
    if ($shellOnPath) {
        return (Split-Path -Parent $shellOnPath.Source)
    }
    $candidates = @(
        'C:\progs\msys2', 'C:\msys64', 'C:\msys2', 'C:\tools\msys64',
        (Join-Path $env:USERPROFILE 'msys64'),
        (Join-Path $env:USERPROFILE 'msys2'),
        (Join-Path $env:USERPROFILE 'scoop\apps\msys2\current')
    )
    foreach ($candidate in $candidates) {
        if (Test-Msys2Root $candidate) {
            return $candidate.TrimEnd('\', '/')
        }
    }
    return $null
}

$Msys2Root = Resolve-Msys2Root
if (-not $Msys2Root) {
    Write-Error @"
MSYS2 not found. Install MSYS2 (https://www.msys2.org/) or set MSYS2_ROOT to your install directory.

Searched: MSYS2_ROOT, PATH (msys2_shell.cmd), C:\progs\msys2, C:\msys64, C:\msys2, C:\tools\msys64, ~\msys64, ~\msys2, ~\scoop\apps\msys2\current
"@
    exit 1
}

$env:MSYS2_ROOT = $Msys2Root

# Call bash.exe directly. msys2_shell.cmd -no-start still detaches into its
# own console: stdout never comes back and $LASTEXITCODE is always 0, so a
# build that never ran reports success. Verified 2026-10-14 --
# `Invoke-Ucrt64.ps1 "g++ --version"` printed nothing and exited 0.
$Bash = Join-Path $Msys2Root 'usr/bin/bash.exe'
if (-not (Test-Path -LiteralPath $Bash)) {
    Write-Error "MSYS2 found at $Msys2Root but $Bash is missing."
    exit 1
}
$env:MSYSTEM = 'UCRT64'
$env:CHERE_INVOKING = '1'
& $Bash -lc $Command
exit $LASTEXITCODE
