param(
  [Parameter(Mandatory = $true)]
  [string]$TargetFile,

  [string]$OutFile = ""
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path -LiteralPath $TargetFile)) {
  throw "Target file not found: $TargetFile"
}

$resolvedTarget = (Resolve-Path -LiteralPath $TargetFile).Path
$hash = Get-FileHash -LiteralPath $resolvedTarget -Algorithm SHA256

$line = "{0}  {1}" -f $hash.Hash.ToLowerInvariant(), [System.IO.Path]::GetFileName($resolvedTarget)

if ([string]::IsNullOrWhiteSpace($OutFile)) {
  $outDir = Split-Path -Parent $resolvedTarget
  $baseName = [System.IO.Path]::GetFileNameWithoutExtension($resolvedTarget)
  $OutFile = Join-Path $outDir ("{0}.sha256" -f $baseName)
}

$OutFile = [System.IO.Path]::GetFullPath($OutFile)
$line | Set-Content -LiteralPath $OutFile -Encoding UTF8

Write-Host "[Hash] Wrote: $OutFile"
Write-Host "[Hash] $line"
