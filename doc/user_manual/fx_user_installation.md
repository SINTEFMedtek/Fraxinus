Installation {#installation}
===================

The easiest way to install is to use the installers from the [releases page](https://gitlab.sintef.no/custusx/fraxinus/-/releases). Check out the \ref supported_platforms.

Ubuntu installation
-------------------

Download `installFraxinus.sh` from the
[releases page](https://gitlab.sintef.no/custusx/fraxinus/-/releases) and run:

    chmod +x installFraxinus.sh
    ./installFraxinus.sh

Alternatively, install directly via `curl`:

    sudo apt install curl -y  # optional, if curl is not already installed
    curl -fsSL https://gitlab.sintef.no/custusx/fraxinus/-/releases/permalink/latest/downloads/installFraxinus.sh | bash

The script downloads the Fraxinus release tarball automatically.
If the package registry requires authentication, set your GitLab personal access token first:

    export GITLAB_TOKEN=your_personal_access_token
    ./installFraxinus.sh

The script installs the required system packages and sets up the inference engines
used for segmentation and registration:

- **Raidionics** — deep learning segmentation of airways, lungs, lymph nodes, and other structures.
- **TotalSegmentator** — additional anatomical segmentation models.
- **Elastix** — image registration used for PET-to-CT alignment.

Windows installation
--------------------

Download the Windows installer from the
[releases page](https://gitlab.sintef.no/custusx/fraxinus/-/releases) and run it.
Fraxinus can also be built from source on Windows; see the developer documentation for instructions.
