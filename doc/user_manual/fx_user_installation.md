Installation {#installation}
===================

The easiest way to install is to use the installers from the [releases page](https://gitlab.sintef.no/custusx/fraxinus/-/releases). Check out the \ref supported_platforms.

Windows installation
--------------------

The Windows installer includes optional components for the inference engines used by Fraxinus:

- **Raidionics** — deep learning segmentation of airways, lungs, lymph nodes, and other structures.
- **TotalSegmentator** — additional anatomical segmentation models.
- **Elastix** — image registration used for PET-to-CT alignment.

These components can be selected during installation. An internet connection is required to download model weights during setup.

Troubleshooting
---------------
- Fraxinus uses OpenCL. If you get errors about OpenCL, e.g. "Missing OpenCL.dll", try installing the latest driver for your graphics card.
- On Windows laptops with both integrated and dedicated graphics, Fraxinus must be set to run on the integrated graphics card to avoid crashes. See the in-app help for details.
