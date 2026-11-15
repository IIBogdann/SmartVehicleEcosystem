$ErrorActionPreference = "Stop"

$repositoryRoot = Split-Path -Parent $PSScriptRoot

Write-Host "=== Git ===" -ForegroundColor Cyan
git -C $repositoryRoot status --short --branch

$githubCli = Get-Command gh -ErrorAction SilentlyContinue
if (-not $githubCli) {
    Write-Host "`nGitHub CLI nu este instalat. Starea Git locală este disponibilă, dar Issues/PR-uri nu pot fi citite." -ForegroundColor Yellow
    exit 0
}

gh auth status *> $null
if ($LASTEXITCODE -ne 0) {
    Write-Host "`nGitHub CLI nu este autentificat. Rulează: gh auth login" -ForegroundColor Yellow
    exit 0
}

Write-Host "`n=== Issues deschise ===" -ForegroundColor Cyan
gh issue list --repo IIBogdann/SmartVehicleEcosystem --state open --limit 30

Write-Host "`n=== Pull Request-uri deschise ===" -ForegroundColor Cyan
gh pr list --repo IIBogdann/SmartVehicleEcosystem --state open --limit 30
