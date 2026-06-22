Installation {#installation}
===================

The easiest way to install is to use the installers from the [releases page](https://gitlab.sintef.no/custusx/fraxinus/-/releases). Check out the \ref supported_platforms.

Ubuntu installation
-------------------

A shell installer script is available alongside the release tarball on the
[releases page](https://gitlab.sintef.no/custusx/fraxinus/-/releases).
Download both files, place them in the same folder, and run:

    chmod +x installFraxinus.sh
    ./installFraxinus.sh

The script installs the required system packages and sets up the inference engines
used for segmentation and registration:

- **Raidionics** — deep learning segmentation of airways, lungs, lymph nodes, and other structures.
- **TotalSegmentator** — additional anatomical segmentation models.
- **Elastix** — image registration used for PET-to-CT alignment.

Windows installation
--------------------

A Windows installer is planned but not yet published. Fraxinus can be built from source on Windows;
see the developer documentation for instructions.

Troubleshooting
---------------
- Fraxinus uses OpenCL. If you get errors about OpenCL, e.g. "Missing OpenCL.dll", try installing the latest driver for your graphics card.
- On Windows laptops with both integrated and dedicated graphics, Fraxinus must be set to run on the integrated graphics card to avoid crashes. See the in-app help for details.
