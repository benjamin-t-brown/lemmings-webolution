# Play the optimized clone build against a separate playtest save.
# Usage: .\scripts\Play-Clone.ps1            (frontend)
#        .\scripts\Play-Clone.ps1 -Rebuild   (rebuild the release binary first)
#
# The clone saves under %LOCALAPPDATA%\LemmingsWebolution. Registered play tests
# assert that the normal save is untouched while they run, so playing on it during
# verification fails them. This points the clone at build/playtest/appdata instead;
# the first run copies the existing normal save there so progress carries over.
param([switch]$Rebuild)

$Root = Split-Path -Parent $PSScriptRoot
$PlayData = Join-Path $Root 'build\playtest\appdata'
$PlaySave = Join-Path $PlayData 'LemmingsWebolution\SavedGame.dat'
$NormalSave = Join-Path $env:LOCALAPPDATA 'LemmingsWebolution\SavedGame.dat'

if (-not (Test-Path -LiteralPath $PlaySave)) {
    New-Item -ItemType Directory -Force (Split-Path -Parent $PlaySave) | Out-Null
    if (Test-Path -LiteralPath $NormalSave) { Copy-Item -LiteralPath $NormalSave -Destination $PlaySave }
}

Push-Location $Root
try {
    if ($Rebuild) {
        & (Join-Path $PSScriptRoot 'Invoke-Ucrt64.ps1') 'cmake --preset gcc-release -DBUILD_TESTING=OFF > /dev/null && cmake --build build/cmake/gcc-release --target webolution -j 16'
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    }
    $env:LOCALAPPDATA = $PlayData
    & (Join-Path $PSScriptRoot 'Invoke-Ucrt64.ps1') "build/cmake/gcc-release/webolution.exe --game-dir 'Lemmings Revolution'"
} finally {
    $env:LOCALAPPDATA = Split-Path -Parent (Split-Path -Parent $NormalSave)
    Pop-Location
}
