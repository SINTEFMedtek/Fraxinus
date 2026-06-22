#!/bin/bash

##########################################################################################################
#
# Fraxinus install script
#
# Download the Fraxinus release tarball and this script from the releases page:
#   https://gitlab.sintef.no/custusx/Fraxinus/-/releases
#
# Place both files in the same folder, then run:
#   chmod +x installFraxinus.sh
#   ./installFraxinus.sh
#
# A user password is required for installing system packages.
#
##########################################################################################################

set -e

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
# Find the Fraxinus release tarball
# ---------------------------------------------------------------------------
TARBALL=$(ls Fraxinus*.tar.gz 2>/dev/null | head -1)
if [ -z "$TARBALL" ]; then
    echo "ERROR: No Fraxinus*.tar.gz found in the current directory."
    echo "Download the release tarball from:"
    echo "  https://gitlab.sintef.no/custusx/Fraxinus/-/releases"
    exit 1
fi
echo "Using tarball: $TARBALL"

# ---------------------------------------------------------------------------
# Unpack to ~/Fraxinus
# ---------------------------------------------------------------------------
if [ -d "Fraxinus_temp" ]; then
    rm -rf Fraxinus_temp
fi
mkdir Fraxinus_temp
tar -xzf "$TARBALL" -C Fraxinus_temp
FRAXINUS_PATH=$(ls Fraxinus_temp/Fraxinus/ | head -1)
mkdir -p ~/Fraxinus
cp -r Fraxinus_temp/Fraxinus/* ~/Fraxinus/
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
    EXEC_PATH="$HOME/Fraxinus/$FRAXINUS_PATH/Fraxinus"
    ICON_PATH="$HOME/Fraxinus/$FRAXINUS_PATH/Icon/Fraxinus.icns"
    sed -i "s|Icon=.*|Icon=$ICON_PATH|g" Fraxinus.desktop
    sed -i "s|Path=.*|Path=$HOME/Fraxinus/$FRAXINUS_PATH|g" Fraxinus.desktop
    sed -i "s|Exec=.*|Exec=$EXEC_PATH|g" Fraxinus.desktop
    cp Fraxinus.desktop ~/Desktop/
    gio set ~/Desktop/Fraxinus.desktop metadata::trusted true 2>/dev/null || true
    chmod +x ~/Desktop/Fraxinus.desktop
fi

echo ""
echo "---------- Fraxinus installation complete ----------"
echo "Launch Fraxinus from the desktop shortcut or run:"
echo "  $HOME/Fraxinus/$FRAXINUS_PATH/Fraxinus"
