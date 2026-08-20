# Installer for Elastix, downloaded from the GitHub release ZIP

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "FraxinusInstallHelpers.ps1")
$logPath = Start-FraxinusInstallLog -Name "Elastix"

try {
    $ZipUrl     = "https://github.com/SuperElastix/elastix/releases/download/5.3.0/elastix-5.3.0-windows.zip"
    $ZipPath    = Join-Path $env:TEMP "elastix.zip"
    $InstallDir = "C:\Elastix"

    Invoke-FraxinusStep -Description "Downloading Elastix" -Action {
        Invoke-WebRequest -Uri $ZipUrl -OutFile $ZipPath -UseBasicParsing
    }

    Write-Host "Extracting Elastix to $InstallDir..."
    Expand-Archive -Path $ZipPath -DestinationPath $InstallDir -Force

    $binPath  = $InstallDir
    $userPath = [System.Environment]::GetEnvironmentVariable("Path", "User")

    if ($userPath -notlike "*$binPath*") {
        $newPath = "$userPath;$binPath"
        [System.Environment]::SetEnvironmentVariable("Path", $newPath, "User")
        Write-Host "Added Elastix to USER PATH. Restart your terminal to pick it up."
    } else {
        Write-Host "Elastix already in USER PATH."
    }

    Write-Host ""
    Write-Host "==============================================="
    Write-Host "  Elastix installation complete"
    Write-Host "==============================================="
    exit 0
} catch {
    Write-Host ""
    Write-Host "==============================================="
    Write-Host "  Elastix setup FAILED"
    Write-Host "  $($_.Exception.Message)"
    Write-Host "  See log: $logPath"
    Write-Host "==============================================="
    exit 1
} finally {
    Stop-Transcript | Out-Null
}
