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
# Retry a download a few times before giving up -- a single transient
# network/DNS hiccup shouldn't require rerunning the whole install script.
# ---------------------------------------------------------------------------
download_with_retry() {
    local attempt
    for attempt in 1 2 3 4 5; do
        if wget "$@"; then
            return 0
        fi
        if [ "$attempt" -lt 5 ]; then
            echo "Download attempt $attempt failed, retrying in 5s..."
            sleep 5
        fi
    done
    return 1
}

# ---------------------------------------------------------------------------
# Version — set by CI for each release; empty when run from a local checkout
# ---------------------------------------------------------------------------
FRAXINUS_VERSION=""

# GitLab project API base for package downloads
GITLAB_PROJECT_URL="https://gitlab.sintef.no/api/v4/projects/custusx%2Ffraxinus"

# ---------------------------------------------------------------------------
# Detect Ubuntu version
# ---------------------------------------------------------------------------
UBUNTU_VERSION=$(lsb_release -rs 2>/dev/null || echo "unknown")
echo "Detected Ubuntu version: $UBUNTU_VERSION"

# Ubuntu 22.04+ ships a suitable Python natively. 20.04 is no longer supported:
# its python3 (3.8) is too old, and the deadsnakes PPA no longer publishes the
# python3.10 we used to install from there (CustusX#51).
case "$UBUNTU_VERSION" in
    20.04)
        echo "ERROR: Ubuntu 20.04 is no longer supported (CustusX#51). Please use Ubuntu 22.04 or 24.04."
        exit 1
        ;;
    22.04) OS="Ubuntu2204" ;;
    24.04) OS="Ubuntu2404" ;;
    *)
        echo "ERROR: Unsupported Ubuntu version: $UBUNTU_VERSION"
        echo "Supported versions: 22.04, 24.04"
        exit 1
        ;;
esac
PYTHON_CMD="python3"

# ---------------------------------------------------------------------------
# Install system packages
#
# libglew-dev: Fraxinus deliberately does not bundle GLEW into the release
# package (see gp_resolved_file_type_override() in CMake/cxInstallUtilities.cmake)
# since it must match the system's own OpenGL/driver stack, so it has to come
# from the system instead.
# ---------------------------------------------------------------------------
sudo apt-get -y update
sudo apt-get -y install libglew-dev libpcre2-16-0 libdouble-conversion3 git wget unzip

sudo apt-get -y install python3-venv

# ---------------------------------------------------------------------------
# Find or download the Fraxinus release tarball
# ---------------------------------------------------------------------------
if [ -n "$FRAXINUS_VERSION" ]; then
    TARBALL="Fraxinus-${OS}.tar.gz"
    DOWNLOAD_URL="${GITLAB_PROJECT_URL}/packages/generic/Fraxinus/${FRAXINUS_VERSION}/${OS}/${TARBALL}"

    echo "Downloading Fraxinus ${FRAXINUS_VERSION} for Ubuntu ${UBUNTU_VERSION}..."
    WGET_ARGS=()
    if [ -n "$GITLAB_TOKEN" ]; then
        WGET_ARGS+=(--header "PRIVATE-TOKEN: $GITLAB_TOKEN")
    fi
    if ! download_with_retry "${WGET_ARGS[@]}" -O "$TARBALL" "$DOWNLOAD_URL"; then
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
    # -t: if more than one matching tarball is sitting here (e.g. an old one
    # left over from before an OS upgrade, or from a previous manual
    # download), prefer the most recently modified one over an arbitrary
    # alphabetical pick.
    TARBALL=$(ls -t Fraxinus*.tar.gz 2>/dev/null | head -1)
    if [ -z "$TARBALL" ]; then
        echo "ERROR: No Fraxinus*.tar.gz found in the current directory."
        echo "Download the versioned installer from the releases page:"
        echo "  https://gitlab.sintef.no/custusx/fraxinus/-/releases"
        exit 1
    fi
    # Local dev/CI builds encode the OS as e.g. "_Ubuntu22.04" (with a dot);
    # tagged releases encode it as "-Ubuntu2204" (no dot, matching $OS above).
    # Refuse a tarball built for a different Ubuntu version outright -- used
    # silently, it installs fine but fails at runtime with a confusing
    # missing-.so error (e.g. a 20.04 build's libGLEW.so.2.1 vs 22.04's
    # libGLEW.so.2.2), long after a clear error here would have helped.
    case "$TARBALL" in
        *"$OS"*|*"Ubuntu${UBUNTU_VERSION}"*) ;;
        *)
            echo "ERROR: $TARBALL does not look like it was built for Ubuntu $UBUNTU_VERSION."
            echo "Remove it and place a Fraxinus*${OS}*.tar.gz build here instead, then re-run."
            exit 1
            ;;
    esac
    echo "Using local tarball: $TARBALL"
fi

# ---------------------------------------------------------------------------
# Unpack to ~/Fraxinus/Fraxinus (~/Fraxinus is the shared family folder --
# venvs/models/Patients live there too, alongside FraxinusExcelsior's own
# ~/Fraxinus/FraxinusExcelsior app folder when that's installed)
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
# The new version is extracted above, so it's safe to now wipe any old
# install: ~/Fraxinus/Fraxinus is pure install payload for this app (no user
# state -- venvs/models/Patients live at the ~/Fraxinus family-folder level,
# shared with FraxinusExcelsior's own ~/Fraxinus/FraxinusExcelsior, and must
# not be touched here). A full wipe of just this app's subfolder avoids stale
# files from a previous version (e.g. an old/renamed plugin .so) lingering
# and getting loaded alongside the new set.
rm -rf ~/Fraxinus/Fraxinus
mkdir -p ~/Fraxinus/Fraxinus
cp -r "$FRAXINUS_ROOT"/* ~/Fraxinus/Fraxinus/
rm -rf Fraxinus_temp

cd ~/Fraxinus/Fraxinus

# ---------------------------------------------------------------------------
# Install Elastix (shared across the Fraxinus family, like the virtual
# environments/models below)
# ---------------------------------------------------------------------------
ELASTIX_VERSION="5.3.0"
if command -v elastix > /dev/null 2>&1; then
    echo "Elastix is already installed, skipping."
else
    echo "Installing Elastix $ELASTIX_VERSION..."
    mkdir -p ~/Fraxinus
    cd ~/Fraxinus
    download_with_retry "https://github.com/SuperElastix/elastix/releases/download/${ELASTIX_VERSION}/elastix-${ELASTIX_VERSION}-ubuntu.zip"
    unzip -o "elastix-${ELASTIX_VERSION}-ubuntu.zip" -d elastix
    chmod +x elastix/bin/elastix elastix/bin/transformix
    cp elastix/lib/libANNlib* elastix/bin/ 2>/dev/null || true
    rm "elastix-${ELASTIX_VERSION}-ubuntu.zip"
    echo '' >> ~/.bashrc
    echo '# Path to Elastix installation' >> ~/.bashrc
    echo 'export PATH=$HOME/Fraxinus/elastix/bin:$PATH' >> ~/.bashrc
    echo 'export LD_LIBRARY_PATH=$HOME/Fraxinus/elastix/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
    source ~/.bashrc
    cd ~/Fraxinus/Fraxinus
fi

# ---------------------------------------------------------------------------
# Download Raidionics AI models (shared across the Fraxinus family)
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

mkdir -p ~/Fraxinus/models/raidionics_models
cd ~/Fraxinus/models/raidionics_models/

for MODEL in "${RAIDIONICS_MODELS[@]}"; do
    echo "Downloading $MODEL..."
    if download_with_retry -N "${RAIDIONICS_MODELS_URL}${MODEL}"; then
        unzip -o "$MODEL"
    else
        echo "WARNING: Failed to download $MODEL."
    fi
done

# ---------------------------------------------------------------------------
# Create virtual Python environments (shared across the Fraxinus family)
# ---------------------------------------------------------------------------
mkdir -p ~/Fraxinus/virtualEnvironments
cd ~/Fraxinus/virtualEnvironments

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
# Pinned: TotalSegmentator depends on fury<2, which depends on dipy without a
# version pin of its own. dipy dropped prebuilt wheels for cp310 as of 1.12.0
# (source-only there), which forces a from-source build that needs Cython/meson
# and Python dev headers -- headers deadsnakes no longer ships for focal at all,
# so that build can't succeed on Ubuntu 20.04. Installing dipy first pins it to
# 1.11.0, the newest release with prebuilt wheels for cp310 (22.04 native) and
# cp312 (24.04 native), so the plain TotalSegmentator install below (no
# --upgrade) leaves this already-satisfied version alone.
pip install "dipy==1.11.0"
# Pinned: TotalSegmentator has changed its CLI between releases (e.g. the
# weights downloader moved from `python -m totalsegmentator.download_weights`
# to the totalseg_download_weights console script), which silently broke the
# Windows installer. Bump this deliberately, and re-check the
# totalseg_download_weights invocation below, when updating.
pip install "TotalSegmentator==2.18.0"
totalseg_download_weights -t total
totalseg_download_weights -t total_fast
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
#
# xdg-user-dirs localizes the Desktop folder's name (e.g. ~/Skrivebord on a
# Norwegian install), so ~/Desktop doesn't reliably exist -- ask xdg-user-dir
# for the real path instead of hardcoding it, falling back to ~/Desktop if
# xdg-user-dirs isn't set up at all.
# ---------------------------------------------------------------------------
DESKTOP_DIR="$(xdg-user-dir DESKTOP 2>/dev/null || true)"
if [ -z "$DESKTOP_DIR" ]; then
    DESKTOP_DIR="$HOME/Desktop"
fi

cd ~/Fraxinus/Fraxinus
if [ -f "Fraxinus.desktop" ]; then
    EXEC_PATH="$HOME/Fraxinus/Fraxinus/bin/Fraxinus"
    ICON_PATH="$HOME/Fraxinus/Fraxinus/icons/Fraxinus.png"
    sed -i "s|Path=.*|Path=$HOME/Fraxinus/Fraxinus/bin|g" Fraxinus.desktop
    sed -i "s|Exec=.*|Exec=$EXEC_PATH|g" Fraxinus.desktop
    sed -i "s|Icon=.*|Icon=$ICON_PATH|g" Fraxinus.desktop
    if [ -d "$DESKTOP_DIR" ]; then
        cp Fraxinus.desktop "$DESKTOP_DIR/"
        # chmod before gio set: GNOME's desktop trust check only takes the
        # metadata::trusted flag into account for a file that's already
        # executable, so setting it first (against a not-yet-executable
        # freshly-copied file) doesn't stick -- Nautilus then renders it as
        # an untrusted/invalid launcher (broken icon, raw filename as label).
        chmod +x "$DESKTOP_DIR/Fraxinus.desktop"
        gio set "$DESKTOP_DIR/Fraxinus.desktop" metadata::trusted true 2>/dev/null || true
    else
        echo "NOTE: no Desktop folder found at $DESKTOP_DIR -- skipping desktop launcher shortcut."
    fi
fi

# ---------------------------------------------------------------------------
# Desktop shortcut to the (shared, family-level) Patients folder
#
# Type=Application + an absolute Exec path, not Type=Link -- Ubuntu's GNOME
# Shell desktop-icons extension (which renders desktop icons, not Nautilus
# itself) rejects Type=Link entries outright ("Broken Desktop File") and also
# rejects a bare command name in Exec= (e.g. "xdg-open", relying on $PATH)
# with the same error, needing the executable's absolute path instead.
# ---------------------------------------------------------------------------
mkdir -p ~/Fraxinus/Patients
XDG_OPEN_PATH="$(command -v xdg-open || echo /usr/bin/xdg-open)"
if [ -d "$DESKTOP_DIR" ]; then
    cat > "$DESKTOP_DIR/Fraxinus_Patients.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Fraxinus Patients
Icon=folder
Exec=$XDG_OPEN_PATH $HOME/Fraxinus/Patients
Terminal=false
EOF
    # chmod before gio set -- see the comment on the app shortcut above.
    chmod +x "$DESKTOP_DIR/Fraxinus_Patients.desktop"
    gio set "$DESKTOP_DIR/Fraxinus_Patients.desktop" metadata::trusted true 2>/dev/null || true
fi

echo ""
echo "---------- Fraxinus installation complete ----------"
echo "Launch Fraxinus from the desktop shortcut or run:"
echo "  cd $HOME/Fraxinus/Fraxinus/bin && ./Fraxinus"
