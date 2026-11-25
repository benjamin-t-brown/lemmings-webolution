# Play the optimized clone build against a separate playtest save.
# Usage: .\scripts\Play-Clone.ps1                       (frontend, software renderer)
#        .\scripts\Play-Clone.ps1 -Rebuild              (rebuild the release binary first)
#        .\scripts\Play-Clone.ps1 -Rebuild -Renderer gl (OpenGL renderer)
#
# The clone saves under %LOCALAPPDATA%\LemmingsWebolution. Registered play tests
# assert that the normal save is untouched while they run, so playing on it during
# verification fails them. This points the clone at build/playtest/appdata instead;
# the first run copies the existing normal save there so progress carries over.
param(
    [switch]$Rebuild,
    [ValidateSet('software', 'gl', 'auto')]
    [string]$Renderer = 'software'
)

$Root = Split-Path -Parent $PSScriptRoot
$PlayData = Join-Path $Root 'build\playtest\appdata'
$PlaySave = Join-Path $PlayData 'LemmingsWebolution\SavedGame.dat'
$NormalSave = Join-Path $env:LOCALAPPDATA 'LemmingsWebolution\SavedGame.dat'
$Exe = Join-Path $Root 'build\cmake\gcc-release\webolution.exe'

# A running clone locks the exe, so the link step fails with Permission denied.
$Running = Get-Process webolution -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq $Exe }
if ($Rebuild -and $Running) {
    Write-Error 'The clone is still running from build\cmake\gcc-release; close it and run again.'
    exit 1
}

if (-not (Test-Path -LiteralPath $PlaySave)) {
    New-Item -ItemType Directory -Force (Split-Path -Parent $PlaySave) | Out-Null
    if (Test-Path -LiteralPath $NormalSave) { Copy-Item -LiteralPath $NormalSave -Destination $PlaySave }
}

Push-Location $Root
try {
    if ($Rebuild) {
        & (Join-Path $PSScriptRoot 'Invoke-Ucrt64.ps1') 'cmake --preset gcc-release -DBUILD_TESTING=OFF > /dev/null && cmake --build build/cmake/gcc-release --target webolution -j 16'
        if ($LASTEXITCODE -ne 0) { Write-Error "Rebuild failed (exit $LASTEXITCODE)."; exit $LASTEXITCODE }
    }
    if (-not (Test-Path -LiteralPath $Exe)) {
        Write-Error 'No release build yet; run with -Rebuild.'
        exit 1
    }
    $env:LOCALAPPDATA = $PlayData
    # Every unfinished level stays enterable (yellow); only won levels show complete.
    $env:WEBOLUTION_UNLOCK_ALL = '1'
    & (Join-Path $PSScriptRoot 'Invoke-Ucrt64.ps1') "build/cmake/gcc-release/webolution.exe --game-dir 'Lemmings Revolution' --renderer $Renderer --record build/playtest/replays"
} finally {
    $env:LOCALAPPDATA = Split-Path -Parent (Split-Path -Parent $NormalSave)
    Remove-Item Env:WEBOLUTION_UNLOCK_ALL -ErrorAction SilentlyContinue
    Pop-Location
}
