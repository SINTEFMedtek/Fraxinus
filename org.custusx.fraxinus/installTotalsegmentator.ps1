# Creates a TotalSegmentator venv with GPU-enabled PyTorch

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "FraxinusInstallHelpers.ps1")
$logPath = Start-FraxinusInstallLog -Name "TotalSegmentator"

try {
    # ---------------------------
    # 1) Paths
    # ---------------------------
    $rootBase  = Join-Path $HOME "Fraxinus_settings\virtualEnvironments"
    $toolBase  = Join-Path $rootBase "TotalSegmentator"
    $segDir    = Join-Path $toolBase "segmentations"
    $venvDir   = Join-Path $toolBase "venv"

    # ---------------------------
    # 2) (Re)create directory tree
    # ---------------------------
    if (Test-Path $toolBase) {
        Write-Host "Removing existing TotalSegmentator folder..."
        Remove-Item -Recurse -Force $toolBase
    }
    New-Item -ItemType Directory -Force -Path $segDir | Out-Null

    # ---------------------------
    # 3) Detect Python
    # ---------------------------
    if (Get-Command python.exe -ErrorAction SilentlyContinue) {
        $Python = "python.exe"
    } elseif (Get-Command py.exe -ErrorAction SilentlyContinue) {
        $Python = "py.exe -3"
    } else {
        throw "Python not found on PATH"
    }
    Write-Host "Using Python: $Python"

    # ---------------------------
    # 4) Create venv
    # ---------------------------
    Invoke-FraxinusStep -Description "Creating virtual environment" -Action {
        if ($Python.EndsWith(" -3")) {
            & py.exe -3 -m venv $venvDir
        } else {
            & python.exe -m venv $venvDir
        }
    }

    $VenvPython = Join-Path $venvDir "Scripts\python.exe"
    if (-not (Test-Path $VenvPython)) { throw "Venv python.exe missing after venv creation" }

    # ---------------------------
    # 5) Upgrade pip
    # ---------------------------
    Invoke-FraxinusStep -Description "Upgrading pip" -Action {
        & $VenvPython -m pip install --upgrade pip
    }

    # ---------------------------
    # 6) Install CUDA-enabled PyTorch
    # ---------------------------
    # For CUDA 11.8 wheels (works on most Windows GPUs, safe default)
    Invoke-FraxinusStep -Description "Installing CUDA-enabled PyTorch (GPU support)" -Action {
        & $VenvPython -m pip install torch --index-url https://download.pytorch.org/whl/cu118
    }

    # ---------------------------
    # 7) Install TotalSegmentator
    # ---------------------------
    Invoke-FraxinusStep -Description "Installing TotalSegmentator" -Action {
        & $VenvPython -m pip install TotalSegmentator
    }

    # ---------------------------
    # 8) Download weights
    # ---------------------------
    foreach ($task in @('total', 'lung_vessels', 'lung_nodules')) {
        Invoke-FraxinusStep -Description "Downloading TotalSegmentator weights ($task)" -Action {
            & $VenvPython -m totalsegmentator.download_weights -t $task
        }
    }

    # ---------------------------
    # 9) Summary
    # ---------------------------
    Write-Host ""
    Write-Host "==============================================="
    Write-Host "  TotalSegmentator GPU environment ready"
    Write-Host "  Location:"
    Write-Host "    $toolBase"
    Write-Host "==============================================="
    exit 0
} catch {
    Write-Host ""
    Write-Host "==============================================="
    Write-Host "  TotalSegmentator setup FAILED"
    Write-Host "  $($_.Exception.Message)"
    Write-Host "  See log: $logPath"
    Write-Host "==============================================="
    exit 1
} finally {
    Stop-Transcript | Out-Null
}
