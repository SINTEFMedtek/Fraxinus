# Installer for Elastix, downloaded from the GitHub release ZIP

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "FraxinusInstallHelpers.ps1")
$logPath = Start-FraxinusInstallLog -Name "Elastix"

try {
    # Pinned: also read by CMake (org.custusx.fraxinus/CMakeLists.txt,
    # cxsetup_extract_ps1_version) to fill in the NSIS installer's
    # version-check marker below. Keep this variable name and quoting style
    # if it moves.
    $ElastixVersion = "5.3.0"
    $ZipUrl     = "https://github.com/SuperElastix/elastix/releases/download/$ElastixVersion/elastix-$ElastixVersion-windows.zip"
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

    # Read by the NSIS installer (Function .onInit in NSIS.template.in) to
    # decide whether to pre-uncheck this component's checkbox next install.
    Set-Content -Path (Join-Path $InstallDir 'installed_version.txt') -Value $ElastixVersion -NoNewline

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
