Fraxinus
========
### An Open Source Platform for Image-Guided Bronchoscopy

**[Fraxinus home page](https://custusx.pages.sintef.no/Fraxinus/)** &mdash;
documentation, user guide, downloads, and screenshots.

Fraxinus is a pre-procedure planning and simulation platform for bronchoscopy research,
developed at SINTEF Medical Technology in collaboration with St. Olavs Hospital,
Trondheim University Hospital, Norway.

Lung cancer is the leading cause of cancer-related death worldwide. Pre-procedural
planning, computing a route through the airway tree to the target lesion and rehearsing
the procedure virtually, is a key step in improving diagnostic outcomes for peripheral
pulmonary lesions.

Fraxinus provides a complete, open-source pipeline in four guided steps:

1. **Patient** — import a chest CT and run automatic segmentation of airways, lungs,
   lymph nodes, vessels, tumors, and more using AI-based tools.
2. **Set Target** — click in the CT to place a bronchoscopy target; the optimal route
   through the airway tree is computed automatically.
3. **Virtual Bronchoscopy** — navigate the segmented airway tree in a fly-through view
   synchronized with CT cross-sections, including an EBUS simulator.
4. **Procedure Planning** — review all segmented structures in combined 2D/3D views
   before entering the bronchoscopy suite.

Fraxinus is built on [CustusX](https://gitlab.sintef.no/custusx/CustusX) and uses
[Raidionics](https://github.com/raidionics) and
[TotalSegmentator](https://github.com/wasserth/TotalSegmentator) for AI-based
segmentation, and [Elastix](https://elastix.lumc.nl/) for image registration.

> **DISCLAIMER**: Fraxinus is a research tool. It is not intended for routine clinical
> use and is neither FDA nor CE approved.

## Getting Started

Pre-built installers for Ubuntu 20.04, 22.04, and 24.04 are available on the
[releases page](https://gitlab.sintef.no/custusx/fraxinus/-/releases).
Download `installFraxinus.sh` and run:

```bash
chmod +x installFraxinus.sh
./installFraxinus.sh
```

The script downloads Fraxinus and installs all required inference engines and AI models
(Raidionics, TotalSegmentator, Elastix) automatically.

## Build from Source

Full [build instructions](https://custusx.pages.sintef.no/Fraxinus/developer_doc/build_instructions.html)
are in the developer documentation. The basic steps are:

```bash
mkdir fx
cd fx
git clone https://gitlab.sintef.no/custusx/fraxinus.git FX/FX
cd FX/FX
./script/cxFraxinusInstaller.py --full --all --build_type Release --user_doc
```

Run `./script/cxFraxinusInstaller.py -h` for more options.

## Structure

Fraxinus is written in C++ using CMake, Qt, VTK, ITK, and other libraries, built on the
CustusX platform using the CTK OSGi plugin framework. It consists of three main plugins:

- **org.custusx.fraxinus** — application entry point and setup logic for external AI tools
- **org.custusx.fraxinus.core.state** — finite state machine driving the four-step workflow
  and orchestrating the AI segmentation pipeline
- **org.custusx.fraxinus.widgets** — all user-facing interface components

## Citation

If you use Fraxinus in your research, please cite:

> Janne Beate Lervik Bakeng, Erlend Fagertun Hofstad, Ole Vegard Solberg, Jon Eiesland,
> Geir Arne Tangen, Tore Amundsen, Thomas Langø, Ingerid Reinertsen, Tormod Selbekk,
> Håkon Olav Leira.
> *Using the CustusX toolkit to create an image guided bronchoscopy application: Fraxinus*,
> PLOS ONE, 02/2019.
> DOI: [10.1371/journal.pone.0211772](https://doi.org/10.1371/journal.pone.0211772)

## Contributing

Bug reports and feature requests are welcome on
[GitHub Issues](https://github.com/SINTEFMedTek/Fraxinus/issues).
The source code is mirrored on [GitHub](https://github.com/SINTEFMedTek/Fraxinus).

## Contributors

Fraxinus is developed by
[SINTEF Medical Technology](https://www.sintef.no/en/digital/departments/department-of-health-research/medical-technology/)
for [St. Olavs Hospital, Trondheim University Hospital](https://stolav.no/en/),
in cooperation with the
[Norwegian National Research Center for Minimally Invasive and Image-Guided Diagnostics and Therapy](https://www.stolav.no/avdelinger/sentral-stab/utviklingsavdelingen/senter-for-innovasjon-medisinsk-utstyr-og-teknologi/midt/)
([SINTEF](https://www.sintef.no/en/), [NTNU](http://www.ntnu.edu/)).
