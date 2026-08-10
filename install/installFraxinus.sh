#!/bin/bash

##########################################################################################################
#
# Fraxinus install script
#
# Download this script from the releases page:
#   https://gitlab.sintef.no/custusx/fraxinus/-/releases
#
# Then run:
#   chmod +x installFraxinus.sh
#   ./installFraxinus.sh
#
# The script downloads the Fraxinus release tarball automatically.
# If the package registry requires authentication, set your GitLab token first:
#   export GITLAB_TOKEN=your_personal_access_token
#
# A user password is required for installing system packages.
#
##########################################################################################################

set -e

# ---------------------------------------------------------------------------
# Version — set by CI for each release; empty when run from a local checkout
# ---------------------------------------------------------------------------
FRAXINUS_VERSION=""

# GitLab project API base for package downloads
GITLAB_PROJECT_URL="https://gitlab.sintef.no/api/v4/projects/custusx%2Ffraxinus"

# ---------------------------------------------------------------------------
# Detect Ubuntu version and select the correct Python
# ---------------------------------------------------------------------------
UBUNTU_VERSION=$(lsb_release -rs 2>/dev/null || echo "unknown")
echo "Detected Ubuntu version: $UBUNTU_VERSION"

case "$UBUNTU_VERSION" in
    20.04)
        # Ubuntu 20.04 ships Python 3.8; install 3.10 from deadsnakes
        PYTHON_CMD="python3.10"
        NEED_DEADSNAKES=true
        ;;
    *)
        # Ubuntu 22.04+ ships a suitable Python natively
        PYTHON_CMD="python3"
        NEED_DEADSNAKES=false
        ;;
esac

# ---------------------------------------------------------------------------
# Install system packages
# ---------------------------------------------------------------------------
sudo apt-get -y update
sudo apt-get -y install libpcre2-16-0 libdouble-conversion3 git wget unzip

if [ "$NEED_DEADSNAKES" = true ]; then
    sudo apt-get -y install software-properties-common
    sudo add-apt-repository ppa:deadsnakes/ppa -y
    sudo apt-get -y update
    sudo apt-get -y install python3.10-venv
else
    sudo apt-get -y install python3-venv
fi

# ---------------------------------------------------------------------------
# Find or download the Fraxinus release tarball
# ---------------------------------------------------------------------------
if [ -n "$FRAXINUS_VERSION" ]; then
    case "$UBUNTU_VERSION" in
        20.04) OS="Ubuntu2004" ;;
        22.04) OS="Ubuntu2204" ;;
        24.04) OS="Ubuntu2404" ;;
        *)
            echo "ERROR: Unsupported Ubuntu version: $UBUNTU_VERSION"
            echo "Supported versions: 20.04, 22.04, 24.04"
            exit 1
            ;;
    esac

    TARBALL="Fraxinus-${OS}.tar.gz"
    DOWNLOAD_URL="${GITLAB_PROJECT_URL}/packages/generic/Fraxinus/${FRAXINUS_VERSION}/${OS}/${TARBALL}"

    echo "Downloading Fraxinus ${FRAXINUS_VERSION} for Ubuntu ${UBUNTU_VERSION}..."
    WGET_ARGS=()
    if [ -n "$GITLAB_TOKEN" ]; then
        WGET_ARGS+=(--header "PRIVATE-TOKEN: $GITLAB_TOKEN")
    fi
    if ! wget "${WGET_ARGS[@]}" -O "$TARBALL" "$DOWNLOAD_URL"; then
        echo ""
        echo "ERROR: Download failed. URL: $DOWNLOAD_URL"
        if [ -z "$GITLAB_TOKEN" ]; then
            echo "If the package registry requires authentication, set your token first:"
            echo "  export GITLAB_TOKEN=your_personal_access_token"
            echo "  ./installFraxinus.sh"
        fi
        rm -f "$TARBALL"
        exit 1
    fi
else
    TARBALL=$(ls Fraxinus*.tar.gz 2>/dev/null | head -1)
    if [ -z "$TARBALL" ]; then
        echo "ERROR: No Fraxinus*.tar.gz found in the current directory."
        echo "Download the versioned installer from the releases page:"
        echo "  https://gitlab.sintef.no/custusx/fraxinus/-/releases"
        exit 1
    fi
    echo "Using local tarball: $TARBALL"
fi

# ---------------------------------------------------------------------------
# Unpack to ~/Fraxinus
# ---------------------------------------------------------------------------
if [ -d "Fraxinus_temp" ]; then
    rm -rf Fraxinus_temp
fi
mkdir Fraxinus_temp
tar -xzf "$TARBALL" -C Fraxinus_temp
# CPack's TGZ generator wraps the installed tree in a top-level directory named
# after the package (e.g. Fraxinus_26.08-rc3_Ubuntu24.04/), so the Fraxinus/
# folder isn't always directly under Fraxinus_temp/ -- search for it instead
# of assuming a fixed depth.
FRAXINUS_ROOT=$(find Fraxinus_temp -mindepth 1 -maxdepth 2 -type d -name Fraxinus | head -1)
if [ -z "$FRAXINUS_ROOT" ]; then
    echo "ERROR: Could not find a Fraxinus folder inside the extracted tarball."
    exit 1
fi
mkdir -p ~/Fraxinus
cp -r "$FRAXINUS_ROOT"/* ~/Fraxinus/
rm -rf Fraxinus_temp

cd ~/Fraxinus

# ---------------------------------------------------------------------------
# Install Elastix
# ---------------------------------------------------------------------------
ELASTIX_VERSION="5.3.0"
if command -v elastix > /dev/null 2>&1; then
    echo "Elastix is already installed, skipping."
else
    echo "Installing Elastix $ELASTIX_VERSION..."
    wget "https://github.com/SuperElastix/elastix/releases/download/${ELASTIX_VERSION}/elastix-${ELASTIX_VERSION}-linux.zip"
    unzip -o "elastix-${ELASTIX_VERSION}-linux.zip" -d elastix
    chmod +x elastix/bin/elastix elastix/bin/transformix
    cp elastix/lib/libANNlib* elastix/bin/ 2>/dev/null || true
    rm "elastix-${ELASTIX_VERSION}-linux.zip"
    echo '' >> ~/.bashrc
    echo '# Path to Elastix installation' >> ~/.bashrc
    echo 'export PATH=$HOME/Fraxinus/elastix/bin:$PATH' >> ~/.bashrc
    echo 'export LD_LIBRARY_PATH=$HOME/Fraxinus/elastix/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
    source ~/.bashrc
fi

# ---------------------------------------------------------------------------
# Download Raidionics AI models
# ---------------------------------------------------------------------------
RAIDIONICS_MODELS_URL="https://github.com/raidionics/Raidionics-models/releases/download/v1.3.0-rc/"
RAIDIONICS_MODELS=(
    "Raidionics-CT_Airways-v13.zip"
    "Raidionics-CT_Lungs-v13.zip"
    "Raidionics-CT_LymphNodes-v13.zip"
    "Raidionics-CT_MediumOrgansMediastinum-v13.zip"
    "Raidionics-CT_PulmSystHeart-v13.zip"
    "Raidionics-CT_SmallOrgansMediastinum-v13.zip"
    "Raidionics-CT_Tumor-v13.zip"
)

mkdir -p ~/Fraxinus_settings/models/raidionics_models
cd ~/Fraxinus_settings/models/raidionics_models/

for MODEL in "${RAIDIONICS_MODELS[@]}"; do
    echo "Downloading $MODEL..."
    if wget -N "${RAIDIONICS_MODELS_URL}${MODEL}"; then
        unzip -o "$MODEL"
    else
        echo "WARNING: Failed to download $MODEL."
    fi
done

# ---------------------------------------------------------------------------
# Create virtual Python environments
# ---------------------------------------------------------------------------
mkdir -p ~/Fraxinus_settings/virtualEnvironments
cd ~/Fraxinus_settings/virtualEnvironments

echo "Creating Raidionics virtual environment..."
rm -rf raidionicsVenv
$PYTHON_CMD -m venv raidionicsVenv
source raidionicsVenv/bin/activate
pip install --upgrade pip
pip install "git+https://github.com/dbouget/raidionics-rads-lib.git@v1.2.0"
deactivate

echo "Creating TotalSegmentator virtual environment..."
mkdir -p TotalSegmentator
cd TotalSegmentator
$PYTHON_CMD -m venv venv
source venv/bin/activate
pip install --upgrade pip
pip install TotalSegmentator
totalseg_download_weights -t total
totalseg_download_weights -t lung_vessels
totalseg_download_weights -t lung_nodules
deactivate
cd ..

# ---------------------------------------------------------------------------
# Add swap space for AI inference (if not already configured)
# ---------------------------------------------------------------------------
if ! grep -q '/swapfile swap swap defaults' /etc/fstab; then
    echo "Adding 10 GB swap space for AI inference..."
    sudo fallocate -l 10G /swapfile
    sudo chmod 600 /swapfile
    sudo mkswap /swapfile
    sudo swapon /swapfile
    echo '/swapfile swap swap defaults 0 0' | sudo tee -a /etc/fstab
fi

# ---------------------------------------------------------------------------
# Install desktop launcher
# ---------------------------------------------------------------------------
cd ~/Fraxinus
if [ -f "Fraxinus.desktop" ]; then
    EXEC_PATH="$HOME/Fraxinus/bin/Fraxinus"
    ICON_PATH="$HOME/Fraxinus/icons/Fraxinus.png"
    sed -i "s|Path=.*|Path=$HOME/Fraxinus/bin|g" Fraxinus.desktop
    sed -i "s|Exec=.*|Exec=$EXEC_PATH|g" Fraxinus.desktop
    sed -i "s|Icon=.*|Icon=$ICON_PATH|g" Fraxinus.desktop
    cp Fraxinus.desktop ~/Desktop/
    gio set ~/Desktop/Fraxinus.desktop metadata::trusted true 2>/dev/null || true
    chmod +x ~/Desktop/Fraxinus.desktop
fi

echo ""
echo "---------- Fraxinus installation complete ----------"
echo "Launch Fraxinus from the desktop shortcut or run:"
echo "  $HOME/Fraxinus/bin/Fraxinus"
