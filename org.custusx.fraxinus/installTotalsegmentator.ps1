# Creates a TotalSegmentator venv with GPU-enabled PyTorch

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "FraxinusInstallHelpers.ps1")
$logPath = Start-FraxinusInstallLog -Name "TotalSegmentator"

try {
    # ---------------------------
    # 1) Paths
    # ---------------------------
    $rootBase  = Join-Path $HOME "Fraxinus\virtualEnvironments"
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
    # For CUDA 11.8 wheels (works on most Windows GPUs, safe default).
    # torchvision must be installed together with torch from this same index:
    # TotalSegmentator (via timm) pulls in torchvision as a transitive
    # dependency, and if it isn't already present, pip resolves it (and can
    # silently upgrade torch along with it) from the default PyPI index
    # instead, producing a torch/torchvision pair that don't share a build -
    # which fails at runtime with "RuntimeError: operator torchvision::nms
    # does not exist" as soon as a segmentation is actually run.
    Invoke-FraxinusStep -Description "Installing CUDA-enabled PyTorch (GPU support)" -Action {
        & $VenvPython -m pip install torch torchvision --index-url https://download.pytorch.org/whl/cu118
    }

    # ---------------------------
    # 7) Install TotalSegmentator
    # ---------------------------
    # Pinned: TotalSegmentator has changed its CLI between releases (e.g. the
    # weights downloader moved from `python -m totalsegmentator.download_weights`
    # to the totalseg_download_weights console script), which silently broke
    # this installer. Bump this deliberately, and re-check the download-weights
    # invocation below, when updating.
    $TotalSegmentatorVersion = "2.18.0"
    Invoke-FraxinusStep -Description "Installing TotalSegmentator" -Action {
        & $VenvPython -m pip install "TotalSegmentator==$TotalSegmentatorVersion"
    }

    # ---------------------------
    # 8) Download weights
    # ---------------------------
    # `python -m totalsegmentator.download_weights` was removed in newer
    # TotalSegmentator releases; the weights downloader is now the
    # totalseg_download_weights console script installed into the venv.
    $DownloadWeights = Join-Path $venvDir "Scripts\totalseg_download_weights.exe"
    foreach ($task in @('total', 'lung_vessels', 'lung_nodules')) {
        Invoke-FraxinusStep -Description "Downloading TotalSegmentator weights ($task)" -Action {
            & $DownloadWeights -t $task
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
