$ErrorActionPreference = 'Stop'
$projectDir = $PSScriptRoot
$repoRoot = Split-Path -Parent $projectDir
$sourceHeader = Join-Path $repoRoot 'Sequential\stb.h'
$targetHeader = Join-Path $projectDir 'stb.h'

if (-not (Test-Path $sourceHeader)) {
    Write-Error "Cannot find '$sourceHeader'. Extract the OpenMP folder beside the repository's existing Sequential folder, then run this script again."
}

Copy-Item -Path $sourceHeader -Destination $targetHeader -Force
Write-Host "Copied stb.h into the OpenMP project folder." -ForegroundColor Green
