Installation {#installation}
===================

The easiest way to install is to use the installers from the [releases page](https://gitlab.sintef.no/custusx/fraxinus/-/releases). Check out the \ref supported_platforms.

Ubuntu installation
-------------------

Download `installFraxinus.sh` from the
[releases page](https://gitlab.sintef.no/custusx/fraxinus/-/releases) and run:

    chmod +x installFraxinus.sh
    ./installFraxinus.sh

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

Troubleshooting
---------------
- Fraxinus uses OpenCL. If you get errors about OpenCL, e.g. "Missing OpenCL.dll", try installing the latest driver for your graphics card.
- On Windows laptops with both integrated and dedicated graphics, Fraxinus must be set to run on the integrated graphics card to avoid crashes. See the in-app help for details.
