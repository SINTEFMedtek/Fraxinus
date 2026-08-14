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
  - name: David Bouget
    orcid: 0000-0002-5669-9514
    affiliation: 1
  - name: Thomas Langø
    orcid: 0000-0002-2824-6120
    affiliation: 1, 2
  - name: Arne Kildahl-Andersen
    orcid: 0000-0002-3911-5222
    affiliation: "3, 4"    
  - name: Hanne Sorger
    orcid: 0000-0002-9968-3491
    affiliation: "4, 5"
  - name: Tore Amundsen
    orcid: 0000-0002-4714-1949
    affiliation: "3, 4"
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
    index: 4
  - name: Clinic of Medicine, Nord-Trøndelag Hospital Trust, Levanger Hospital, 7601 Levanger, Norway
    index: 5
date: 22 June 2026
bibliography: paper.bib
---

# Summary

Fraxinus is an open-source, workflow-driven software system for bronchoscopy planning, developed at SINTEF Digital and St. Olavs hospital, Trondheim, Norway. Built on the CustusX image-guided therapy platform [@askeland2016custusx], Fraxinus provides a complete pipeline for pre-procedural bronchoscopy planning, with a primary focus on lung cancer staging and diagnosis. Key capabilities include automatic multi-organ segmentation from CT images using deep learning, PET-CT fusion for metabolically active lymph node visualization, route computation through the airway tree to pulmonary lesions, virtual bronchoscopy, and an endobronchial ultrasound (EBUS) simulator for rehearsing navigated lymph node sampling. The software is written in C++ with Qt for the user interface and runs on Windows and Ubuntu. Fraxinus runs on commodity hardware and supports both CPU and GPU execution allowing it to run on a modern laptop while remaining compatible with GPU acceleration where available.

An earlier version of the Fraxinus architecture was described in [@bakeng2019fraxinus]. The present paper describes the substantially evolved open-source codebase, which now integrates AI-driven multi-organ segmentation and deformable image registration.

# Statement of Need

Lung cancer is the leading cause of cancer-related mortality worldwide, accounting for approximately 1.8 million deaths annually [@bray2024globalcancer]. Flexible bronchoscopy is the primary diagnostic tool for pulmonary lesions, but the diagnostic yield for peripherally located, bronchoscopically non-visible tumors is as low as 14–20% for lesions smaller than 2 cm in the outer third of the lung [@chen2007yield; @dhillon2017bronchoscopy], compared with around 80% for centrally visible lesions [@rivera2013diagnosis]. Pre-procedural planning by computing routes through the airway tree to the target lesion and rehearsing the procedure with virtual bronchoscopy (VB) raises the diagnostic yield substantially [@kops2023navigation].

Open-source, full-pipeline tools for bronchoscopy planning are largely absent from the literature and from public repositories. Fraxinus addresses this gap by providing researchers and clinicians with a freely available research platform that runs on commodity hardware. It is intended for expert personnel in clinical research settings and is not approved for routine clinical use.

# State of the Field

Several open-source medical imaging platforms support partial bronchoscopy workflows. 3D Slicer [@fedorov2012slicer] is widely used for airway segmentation (via extensions such as SlicerAirwaySegmentation) and general virtual endoscopy, but it is a general-purpose framework without a dedicated bronchoscopy workflow. MITK [@wolf2005mitk] similarly offers broad imaging functionality without a bronchoscopy-specific pipeline.

Commercial bronchoscopy navigation platforms (Medtronic superDimension, Intuitive Ion, J&J Monarch) offer strong mechanical reach through shape-sensing catheters and robotic actuation, with multicenter trials reporting high diagnostic yields for peripheral nodules [@murgu2025target; @ali2023robotic; @simoff2021ion]. However, they require proprietary hardware, cannot be independently inspected or extended, and are inaccessible to most research groups and resource-limited health systems.

Fraxinus is unique in combining components into an open-source package: (1) automated AI-based segmentation of multiple anatomical structures from CT; (2) a guided, step-by-step workflow tailored to bronchoscopy procedure planning; and (3) virtual bronchoscopy visualization.

# Software Design

Fraxinus is implemented in C++ using Qt for the GUI, VTK for 3D visualization and image processing. It uses the CTK OSGi plugin framework [@seibert2010ctk], inherited from CustusX, to separate concerns across three public plugins:

**org.custusx.fraxinus** provides the application entry point, installer configuration, and setup logic for external AI tools (downloading models, configuring Python virtual environments).

**org.custusx.fraxinus.core.state** implements a finite state machine that guides users through a sequential workflow: from new patient creation and CT import, through automated segmentation and route planning, to virtual bronchoscopy and procedure planning. Each state configures the application layout, available controls, and automated actions appropriate to that step. This plugin also orchestrates the multi-step segmentation pipeline, invoking Python-based AI tools as subprocesses and importing results back into the patient session.

**org.custusx.fraxinus.widgets** contains all user-facing interface components. Key widgets handle patient loading, DICOM import, segmented structure visibility, target pinpointing in multi-planar views, virtual bronchoscopy visualization with an integrated EBUS simulator that renders a simulated ultrasound sector from the CT to rehearse navigated lymph node sampling, and procedure planning.

The segmentation pipeline automatically extracts airways, lungs, lung lobes, lymph nodes, heart, major pulmonary vessels, and tumor candidates from a chest CT scan. Two deep learning backends are supported: Raidionics [@bouget2023raidionics; @bouget2019lymphnodes; @bouget2022mediastinal; @stoverud2024aeropath], which provides organ-specific models (CT_Airways, CT_Lungs, CT_LymphNodes, CT_Tumor, and others), and TotalSegmentator [@wasserthal2023totalsegmentator], which can segment over 100 anatomical structures. Deformable image registration via Elastix [@klein2010elastix; @shamonin2014elastix] enables fusion of PET and CT volumes for combined metabolic and anatomical visualization.

Once the airway centerline is extracted, a route-to-target algorithm traces the optimal path from the trachea to a clinician-specified lesion through the airway tree. The resulting procedure planning view is shown in Figure 1.

The system is built and distributed via a Python-based superbuild script (`cxFraxinusInstaller.py`) that manages all C++ library dependencies (Qt, VTK, ITK, Eigen, OpenCV, OpenIGTLink, CTK, Boost) and Python tool setup. Pre-built binary installers are provided for Windows and Ubuntu.

![The Fraxinus procedure planning step. Left: Virtual bronchoscopy with the planned route overlaid on the airway surface and 2D CT sections with the target lesion highlighted. Right: 3D rendering of the segmented airway tree with lymph nodes (green) and tumours (yellow).](Fraxinus_1.png)

# Research Impact

The Fraxinus development originated from the doctoral thesis of Håkon Olav Leira in 2012 [@leira2012thesis], first presented as Fraxinus at IPCAI/CARS 2016 (Heidelberg, Germany) and formally published in 2019 [@bakeng2019fraxinus]. The system has since been extended with AI-based segmentation and multimodal registration. Fraxinus has been tested and used in clinical research at a total of eight hospitals in Norway, mainly at St. Olavs Hospital (Trondheim, Norway), supporting studies in bronchoscopy planning.

The planning module in Fraxinus has also been included and validated through Sorger et al. [@sorger2017EBUS], who demonstrated feasibility of navigated EBUS bronchoscopy in humans. Kildahl-Andersen et al. [@kildahl-andersen2024PETEBUS] subsequently validated PET-CT-fused navigation in a human cohort.

Perhaps the strongest indicator of the platform’s maturity is its adoption as the core navigation technology in two independent robotic bronchoscopy platforms. Chen et al. [@chen2023robotic] developed a teleoperated robotic bronchoscopy system with a variable-stiffness catheter, using CustusX as their multimodal navigation framework. Gruionu et al. [@gruionu2024robocath] developed the RoboCath robotic catheter platform within the IDEAR project (University of Craiova and SINTEF), using Fraxinus directly for procedural planning and guidance. This adoption demonstrates that the software architecture is sufficiently robust, documented, and generalizable to serve as infrastructure for third-party development.

The software is available at [https://github.com/SINTEFMedTek/Fraxinus](https://github.com/SINTEFMedTek/Fraxinus) under the BSD 3-Clause License. Binary installers and sample phantom datasets are provided at [https://custusx.pages.sintef.no/fraxinus/](https://custusx.pages.sintef.no/fraxinus/). The codebase serves as a reference implementation for bronchoscopy planning research.

# AI Usage Disclosure

Claude Sonnet 4.6 (Anthropic) was used to assist with drafting of manuscript text and compilation of the reference list. All AI-assisted content has been reviewed, edited, and validated by the human authors, who take full responsibility for the accuracy and completeness of the paper.

# Acknowledgements

The authors thank the patients, clinical staff, and research colleagues at the Department of Thoracic Medicine, St. Olavs hospital, Trondheim, and at SINTEF Digital, Department of Health Research, who have contributed to Fraxinus with procedural expertise, software development and iterative testing.
Development of Fraxinus has been supported by the Norwegian National Research Center for Minimally Invasive and Image-Guided Diagnostics and Therapy (MiDT) at Center for Innovation, Medical Devices and Technology (SIMUT) at St. Olavs hospital, SINTEF Digital, the Norwegian Research Council, the Norwegian Cancer Society, the Liaison Committee between Central Norway Regional Health Authority and NTNU, and the EEA project IDEAR.

# References
