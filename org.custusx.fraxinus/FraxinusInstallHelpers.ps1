# ===============================================
# FraxinusInstallHelpers.ps1
# Shared helpers for the Fraxinus post-install scripts
# (installRaidionics.ps1, installTotalsegmentator.ps1, installElastix.ps1)
# ===============================================

# Writes all Write-Host output from the calling script to a log file, so the
# install details survive even though the NSIS-spawned console window closes
# as soon as the script exits.
function Start-FraxinusInstallLog {
    param([Parameter(Mandatory)][string]$Name)
    # Shared family folder (see CX_FAMILY_FOLDER_NAME) -- these venv/model
    # installs are shared between Fraxinus and FraxinusExcelsior, so their
    # setup logs live at the family level too, not under either app's own
    # per-app settings folder.
    $logDir = Join-Path $env:USERPROFILE 'Fraxinus\install_logs'
    New-Item -ItemType Directory -Force -Path $logDir | Out-Null
    $logPath = Join-Path $logDir "$Name.log"
    Start-Transcript -Path $logPath -Append | Out-Null
    return $logPath
}

# Runs $Action, treating both a thrown exception and a non-zero $LASTEXITCODE
# (set by native commands such as pip/python) as failure, and retries
# transient failures (e.g. network hiccups during downloads) before giving up.
function Invoke-FraxinusStep {
    param(
        [Parameter(Mandatory)][string]$Description,
        [Parameter(Mandatory)][scriptblock]$Action,
        [int]$MaxAttempts = 3,
        [int]$RetryDelaySeconds = 10
    )

    for ($attempt = 1; $attempt -le $MaxAttempts; $attempt++) {
        Write-Host "$Description (attempt $attempt/$MaxAttempts)..."
        $global:LASTEXITCODE = 0
        try {
            & $Action
            if ($LASTEXITCODE -ne 0) {
                throw "exit code $LASTEXITCODE"
            }
            return
        } catch {
            Write-Host "  -> FAILED: $($_.Exception.Message)"
            if ($attempt -ge $MaxAttempts) {
                throw "$Description failed after $MaxAttempts attempt(s): $($_.Exception.Message)"
            }
            Write-Host "  -> Retrying in $RetryDelaySeconds s..."
            Start-Sleep -Seconds $RetryDelaySeconds
        }
    }
}
