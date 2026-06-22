---
title: 'Fraxinus: An Open-Source Navigation System for Bronchoscopy Research'
tags:
  - bronchoscopy
  - image-guided navigation
  - airway segmentation
  - deep learning
  - virtual bronchoscopy
  - C++
  - Qt
authors:
  - name: Erlend Fagertun Hofstad
    orcid: 0009-0007-5927-7468
    affiliation: 1
  - name: Ole Vegard Solberg
    orcid: 0009-0004-9488-3621
    affiliation: 1
  - name: Thomas Langø
    orcid: 0000-0002-2824-6120
    affiliation: 1, 2
  - name: Håkon Olav Leira
    orcid: 0000-0003-0845-3891
    affiliation: "3, 4"
affiliations:
  - name: SINTEF Digital, Department of Health Research, 7030 Trondheim, Norway
    index: 1
  - name: National Research Center for Minimally Invasive and Image-Guided Diagnostics and Therapy, St. Olavs Hospital, 7030 Trondheim, Norway
    index: 2 
  - name: Department of Thoracic Medicine, St. Olavs Hospital, 7030 Trondheim, Norway
    index: 3
  - name: Department of Circulation and Medical Imaging, Faculty of Medicine and Health Sciences, Norwegian University of Science and Technology, 7030 Trondheim, Norway
  - index: 4
date: 22 June 2026
bibliography: paper.bib
---

# Summary

Fraxinus is an open-source, workflow-driven software system for bronchoscopy planning, developed at SINTEF Medical Technology. Built on the CustusX image-guided therapy platform [@askeland2016custusx], Fraxinus provides a complete pipeline for pre-procedural bronchoscopy planning. The software orchestrates automatic multi-organ segmentation from CT images using deep learning tools, computes routes through the airway tree to pulmonary lesions, and supports virtual bronchoscopy visualization in both fly-through and cut-plane modes. The software is written in C++ with Qt for the user interface and runs on Windows and Ubuntu.

An earlier version of the Fraxinus architecture was described in [@bakeng2019fraxinus]. The present paper describes the substantially evolved open-source codebase, which now integrates AI-driven multi-organ segmentation and deformable image registration.

# Statement of Need

Lung cancer is the leading cause of cancer-related death worldwide [@sung2021cancer]. Flexible bronchoscopy is a primary diagnostic tool for pulmonary lesions, but the diagnostic yield for peripherally located, bronchoscopically non-visible tumors is as low as 15% with conventional bronchoscopy [@chen2007yield], compared with around 80% for centrally visible lesions. Pre-procedural planning — computing routes through the airway tree to the target lesion and rehearsing the procedure with virtual bronchoscopy — is a key step in improving outcomes for peripheral lesions [@pritchett2017nav].

Open-source, full-pipeline tools for bronchoscopy planning are largely absent from the literature and from public repositories.

Fraxinus addresses this gap by providing researchers and clinicians with a freely available, feature-complete research platform that runs on commodity hardware. It is intended for expert personnel in clinical research settings and is not approved for routine clinical use.

# State of the Field

Several open-source medical imaging platforms support partial bronchoscopy workflows. 3D Slicer [@fedorov2012slicer] is widely used for airway segmentation (via extensions such as SlicerAirwaySegmentation) and general virtual endoscopy, but it is a general-purpose framework without a dedicated bronchoscopy workflow or intraoperative navigation capability. MITK [@wolf2005mitk] similarly offers broad imaging functionality without a bronchoscopy-specific pipeline. Commercial platforms, including LungVision (Body Vision Medical) and Auris Health's Monarch Platform, integrate robotic control but are proprietary.

Fraxinus is unique in combining in a single open-source package: (1) automated AI-based segmentation of multiple anatomical structures from CT; (2) a guided, step-by-step workflow tailored to bronchoscopy procedure planning; and (3) virtual bronchoscopy visualization in both fly-through and cut-plane modes.

# Software Design

Fraxinus is implemented in C++ (C++14) using Qt 5 for the GUI, VTK for 3D visualization, and ITK for image processing. It uses the CTK OSGi plugin framework [@seibert2010ctk], inherited from CustusX, to separate concerns across three public plugins:

**org.custusx.fraxinus** provides the application entry point, installer configuration, and setup logic for external AI tools (downloading models, configuring Python virtual environments).

**org.custusx.fraxinus.core.state** implements a finite state machine that guides users through a sequential workflow: from new patient creation and CT import, through automated segmentation and route planning, to virtual bronchoscopy and procedure planning. Each state configures the application layout, available controls, and automated actions appropriate to that step. This plugin also orchestrates the multi-step segmentation pipeline, invoking Python-based AI tools as subprocesses and importing results back into the patient session.

**org.custusx.fraxinus.widgets** contains all user-facing interface components. Key widgets handle patient loading, DICOM import, segmented structure visibility, target pinpointing in multi-planar views, virtual bronchoscopy visualization, and procedure planning.

The segmentation pipeline automatically extracts airways, lungs, lung lobes, lymph nodes, heart, major pulmonary vessels, and tumor candidates from a chest CT scan. Two deep learning backends are supported: Raidionics [@bouget2023raidionics; @bouget2019lymphnodes; @bouget2022mediastinal; @stoverud2024aeropath], which provides organ-specific models (CT_Airways, CT_Lungs, CT_LymphNodes, CT_Tumor, and others), and TotalSegmentator [@wasserthal2023totalsegmentator], which can segment over 100 anatomical structures. Deformable image registration via Elastix [@shamonin2014elastix] enables fusion of PET and CT volumes for combined metabolic and anatomical visualization.

Once the airway centerline is extracted, a route-to-target algorithm traces the shortest path from the trachea to a clinician-specified lesion through the airway tree.

The system is built and distributed via a Python-based superbuild script (`cxFraxinusInstaller.py`) that manages all C++ library dependencies (Qt, VTK, ITK, Eigen, OpenCV, OpenIGTLink, CTK, Boost) and Python tool setup. Pre-built binary installers are provided for Windows and Ubuntu.

# Research Impact

Fraxinus has been tested and used in clinical research at totally eight hospitals in Norway, mainly at St. Olavs Hospital (Trondheim, Norway), supporting studies in bronchoscopy planning. An early description of the software appeared in [@bakeng2019fraxinus]; the system has since been extended with AI-based segmentation and multimodal registration.

The software is available at [https://github.com/SINTEFMedTek/Fraxinus](https://github.com/SINTEFMedTek/Fraxinus) under the BSD 3-Clause License. Binary installers and sample phantom datasets are provided at [https://custusx.pages.sintef.no/fraxinus/](https://custusx.pages.sintef.no/fraxinus/). The codebase serves as a reference implementation for bronchoscopy planning research.

# AI Usage Disclosure

Generative AI tools were used to assist in drafting this manuscript. All AI-assisted content has been reviewed, edited, and validated by the human authors, who take full responsibility for the accuracy and completeness of the paper.

# Acknowledgements

Development of Fraxinus has been supported by the Research Council of Norway, the South-Eastern Norway Regional Health Authority, the Central Norway Regional Health Authority, and SINTEF. We thank clinical staff at the Department of Thoracic Medicine, St. Olavs Hospital, for procedural expertise and iterative testing throughout development.

# References
