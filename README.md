Fraxinus
========
### An Open Source Platform for Image-Guided Bronchoscopy

Fraxinus is a pre-procedure planning and simulation platform for bronchoscopy research. The intended audience is clinical and technological researchers in the fields of pulmonology and minimally invasive diagnostics. The platform guides users through CT import, automatic airway segmentation, route planning to a target lesion, and virtual bronchoscopy simulation including EBUS.

Fraxinus is built on [CustusX](https://gitlab.sintef.no/custusx/CustusX).

See [Fraxinus home page](https://custusx.pages.sintef.no/Fraxinus/) for more.

## Usage

DISCLAIMER: Fraxinus is a research tool: It is not intended for normal clinical use, and is not FDA nor CE approved.

The code is free to download and use under a BSD-3 license.

### Build instructions

[Build instructions available here.](https://custusx.pages.sintef.no/Fraxinus/developer_doc/build_instructions.html)

The basic steps are:

        mkdir fx
        cd fx
        git clone https://gitlab.sintef.no/custusx/fraxinus.git FX/FX
        cd FX/FX
        ./script/cxFraxinusInstaller.py --full --all --build_type Release --user_doc

Run `cxFraxinusInstaller.py -h` for more options.

## Structure

Fraxinus is written in C++ using CMake, Qt, VTK, ITK, and other libraries, and is built on the CustusX platform. It uses the OSGi plugin framework implemented by CTK. Inference-based segmentation relies on Raidionics and TotalSegmentator; image registration uses Elastix.

## Contributors

Fraxinus is developed by [SINTEF Medical Technology](https://www.sintef.no/en/digital/departments/department-of-health-research/medical-technology/) for [St. Olavs Hospital, Trondheim University Hospital](https://stolav.no/en/), in cooperation with the [Norwegian National Research Center for Minimally Invasive and Image-Guided Diagnostics and Therapy](https://www.stolav.no/avdelinger/sentral-stab/utviklingsavdelingen/senter-for-innovasjon-medisinsk-utstyr-og-teknologi/midt/) ([SINTEF](https://www.sintef.no/en/), [NTNU](http://www.ntnu.edu/)).
