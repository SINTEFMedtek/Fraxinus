# ===============================================
# installRaidionics.ps1 - NSIS-compatible version
# ===============================================

$ErrorActionPreference = 'Stop'
$ProgressPreference    = 'SilentlyContinue'
. (Join-Path $PSScriptRoot "FraxinusInstallHelpers.ps1")
$logPath = Start-FraxinusInstallLog -Name "Raidionics"

function Log { param($m) ; Write-Host $m }

try {

# -------------------- Resolve safe home folder --------------------
$UserHome = $env:USERPROFILE
if (-not $UserHome -or -not (Test-Path $UserHome)) {
    throw "USERPROFILE cannot be resolved."
}

$VenvRoot   = Join-Path $UserHome 'Fraxinus\virtualEnvironments'
$ModelsRoot = Join-Path $UserHome 'Fraxinus\models\raidionics_models'

New-Item -ItemType Directory -Force -Path $VenvRoot   | Out-Null
New-Item -ItemType Directory -Force -Path $ModelsRoot | Out-Null

# -------------------- Python detection --------------------
function Get-Python {
    $candidates = @(
        "$env:LOCALAPPDATA\Programs\Python\Python312\python.exe",
        "$env:LOCALAPPDATA\Programs\Python\Python311\python.exe",
        "$env:LOCALAPPDATA\Programs\Python\Python310\python.exe",
        "C:\Python312\python.exe","C:\Python311\python.exe",
        "C:\Python310\python.exe"
    )
    foreach ($p in $candidates) {
        if (Test-Path $p) { return $p }
    }
    try { return (Get-Command python.exe -ErrorAction Stop).Path } catch {}
    try { return "$((Get-Command py.exe -ErrorAction Stop).Path) -3" } catch {}
    throw "Python not found"
}

Log "Resolving Python..."
$Python = Get-Python
Log "Using Python: $Python"

# -------------------- Create venv --------------------
$VenvPath = Join-Path $VenvRoot 'raidionicsVenv'
if (Test-Path $VenvPath) {
    Log "Removing old venv..."
    Remove-Item -Recurse -Force $VenvPath -ErrorAction SilentlyContinue
}

Invoke-FraxinusStep -Description "Creating venv" -Action {
    if ($Python.EndsWith(" -3")) {
        & py.exe -3 -m venv "$VenvPath"
    } else {
        & "$Python" -m venv "$VenvPath"
    }
}

$VenvPython = Join-Path $VenvPath 'Scripts\python.exe'
if (-not (Test-Path $VenvPython)) { throw "Venv Python missing" }

# -------------------- pip installs --------------------
Invoke-FraxinusStep -Description "Upgrading pip" -Action {
    & "$VenvPython" -m pip install --upgrade pip setuptools wheel
}

Invoke-FraxinusStep -Description "Installing raidionics-rads-lib" -Action {
    & "$VenvPython" -m pip install git+https://github.com/dbouget/raidionics-rads-lib.git@v1.2.0
}

Invoke-FraxinusStep -Description "Installing onnxruntime-gpu" -Action {
    & "$VenvPython" -m pip install onnxruntime-gpu==1.23
}

# -------------------- ZIP extraction utilities --------------------
Add-Type -AssemblyName System.IO.Compression.FileSystem

function Is-ZipFile {
    param($Path)
    if (-not (Test-Path $Path)) { return $false }
    $fs = [System.IO.File]::OpenRead($Path)
    try {
        $buf = New-Object byte[] 4
        $fs.Read($buf, 0, 4) | Out-Null
        return ($buf[0] -eq 0x50 -and $buf[1] -eq 0x4B)
    } finally {
        $fs.Dispose()
    }
}

function Extract-ZipForce {
    param($ZipFile, $Dest)
    $zip = [System.IO.Compression.ZipFile]::OpenRead($ZipFile)
    try {
        foreach ($entry in $zip.Entries) {
            $target = Join-Path $Dest $entry.FullName
            if ([string]::IsNullOrEmpty($entry.Name)) {
                New-Item -ItemType Directory -Force -Path $target -ErrorAction SilentlyContinue | Out-Null
                continue
            }
            $dir = Split-Path $target -Parent
            New-Item -ItemType Directory -Force -Path $dir -ErrorAction SilentlyContinue | Out-Null
            [System.IO.Compression.ZipFileExtensions]::ExtractToFile($entry, $target, $true)
        }
    } finally {
        $zip.Dispose()
    }
}

# -------------------- Download & extract models --------------------
$Repo = 'https://github.com/raidionics/Raidionics-models/releases/download/v1.3.0-rc/'
$Models = @(
    'Raidionics-CT_Airways-v13.zip',
    'Raidionics-CT_Lungs-v13.zip',
    'Raidionics-CT_LymphNodes-v13.zip',
    'Raidionics-CT_MediumOrgansMediastinum-v13.zip',
    'Raidionics-CT_PulmSystHeart-v13.zip',
    'Raidionics-CT_SmallOrgansMediastinum-v13.zip',
    'Raidionics-CT_Tumor-v13.zip'
)

foreach ($m in $Models) {
    $url = $Repo + $m
    $zipFile = Join-Path $ModelsRoot $m

    Invoke-FraxinusStep -Description "Downloading $m" -Action {
        Invoke-WebRequest -Uri $url -OutFile $zipFile -UseBasicParsing
    }

    if (-not (Is-ZipFile $zipFile)) { throw "Invalid ZIP: $zipFile" }

    Log "Extracting $m ..."
    Extract-ZipForce -ZipFile $zipFile -Dest $ModelsRoot
	
    Log "Delete $m after extraction"
	Remove-Item -Force "$zipFile"
}

Log "==============================================="
Log " Raidionics installation complete"
Log "==============================================="

exit 0

} catch {
    Write-Host ""
    Write-Host "==============================================="
    Write-Host "  Raidionics setup FAILED"
    Write-Host "  $($_.Exception.Message)"
    Write-Host "  See log: $logPath"
    Write-Host "==============================================="
    exit 1
} finally {
    Stop-Transcript | Out-Null
}