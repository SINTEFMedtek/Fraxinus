# Fraxinus release notes — v26.08

_Changes since v22.09_

## New Features

| Key | Summary | Release Note |
|---|---|---|
| [CustusX#30](https://gitlab.sintef.no/custusx/custusx/-/issues/30) | Ubuntu 24.04 support | Fraxinus now builds and runs on Ubuntu 24.04 (in addition to Ubuntu 20.04 and 22.04). VTK is updated to 9.6.1 and ITK to 5.4.5. |
| [Fraxinus#42](https://gitlab.sintef.no/custusx/fraxinus/-/issues/42) | Single-file Ubuntu installer | Added a single-file Ubuntu installer script (installFraxinus.sh) that downloads and sets up Fraxinus automatically. |
| [Fraxinus#40](https://gitlab.sintef.no/custusx/fraxinus/-/issues/40) | Fraxinus public version | Fraxinus is now available as an open-source public version, with tracking-related code moved to a separate private component. |
| [Fraxinus#39](https://gitlab.sintef.no/custusx/fraxinus/-/issues/39) | Usability improvements | Various Fraxinus usability improvements: square virtual bronchoscopy view, full centerline tree shown in VB, automatic CT/PET modality assignment on import, and a streamlined patient loading and segmentation workflow with inline progress. |
| [Fraxinus#38](https://gitlab.sintef.no/custusx/fraxinus/-/issues/38) | Simplified virtual camera rotation | Simplified automatic camera rotation in virtual bronchoscopy, based on the target's lobe location. |
| [Fraxinus#37](https://gitlab.sintef.no/custusx/fraxinus/-/issues/37) | New virtual bronchoscopy layout | Added a virtual bronchoscopy layout option with larger VB + ACS + 3D views, and made the screenshot/video toolbar visible by default. |
| [Fraxinus#35](https://gitlab.sintef.no/custusx/fraxinus/-/issues/35) | AI segmentations on Windows | Raidionics and TotalSegmentator AI segmentations now also work on Windows. |
| [Fraxinus#34](https://gitlab.sintef.no/custusx/fraxinus/-/issues/34) | Simplified CT import | Simplified the CT import and patient creation flow in Fraxinus with a streamlined dialog. |
| [Fraxinus#32](https://gitlab.sintef.no/custusx/fraxinus/-/issues/32) | Windows support | Fraxinus can now be built and installed on Windows. |
| [Fraxinus#26](https://gitlab.sintef.no/custusx/fraxinus/-/issues/26) | EBUS simulator abdominal view | Added an abdominal 2D view to the EBUS simulator, to help identify lymph nodes in CT. |
| [Fraxinus#24](https://gitlab.sintef.no/custusx/fraxinus/-/issues/24) | Smoothed segmentation meshes | Added configurable smoothing of segmented structure meshes, for smoother-looking and smaller models. |
| [Fraxinus#22](https://gitlab.sintef.no/custusx/fraxinus/-/issues/22) | Optional gray application style | Added an optional gray application style for all applications, activated with `-style gray` on the command line. |
| [Fraxinus#21](https://gitlab.sintef.no/custusx/fraxinus/-/issues/21) | Improved workflow guidance | Added informational dialogs when a patient is created, data is imported, and segmentation is completed, to make the import flow clearer. |
| [Fraxinus#20](https://gitlab.sintef.no/custusx/fraxinus/-/issues/20) | TotalSegmentator integration | Integrated TotalSegmentator for segmenting lung blood vessels and lung lobes. |
| [Fraxinus#19](https://gitlab.sintef.no/custusx/fraxinus/-/issues/19) | Interactive tumor assessment | Added interactive review of segmented tumors and lung nodules: each detected lesion can be confirmed or rejected individually. |
| [Fraxinus#18](https://gitlab.sintef.no/custusx/fraxinus/-/issues/18) | Updated tumor segmentation (Raidionics) | Replaced the previous tumor segmentation (LungTumorMask) with a new AI model running in Raidionics. |
| [Fraxinus#17](https://gitlab.sintef.no/custusx/fraxinus/-/issues/17) | Research data acquisition workflow | Added a single-button workflow for recording synchronized bronchoscope video and tracking data for research purposes. |
| [Fraxinus#12](https://gitlab.sintef.no/custusx/fraxinus/-/issues/12) | EBUS simulator | Added an EBUS (endobronchial ultrasound) simulator, showing a simulated ultrasound sector from CT during virtual bronchoscopy. |
| [Fraxinus#9](https://gitlab.sintef.no/custusx/fraxinus/-/issues/9) | Extra via points | Added the option to insert additional via points between the segmented airways and the target. |
| [Fraxinus#8](https://gitlab.sintef.no/custusx/fraxinus/-/issues/8) | Virtual bronchoscopy improvements | Several virtual bronchoscopy improvements: show target and via points during navigation, display segmented structures in 2D view (previously 3D only), and switch to the post-processed airway model for smoother visualization. |
| [Fraxinus#7](https://gitlab.sintef.no/custusx/fraxinus/-/issues/7) | Keyboard navigation in virtual bronchoscopy | Added keyboard shortcuts to navigate and control virtual bronchoscopy, including up/down angle adjustment. |

## Bugfixes

| Key | Summary | Release Note |
|---|---|---|
| [Fraxinus#36](https://gitlab.sintef.no/custusx/fraxinus/-/issues/36) | Centerline generation in separate thread | Fixed the application freezing for about a minute during airway centerline generation, by moving the processing to a background thread. |
| [Fraxinus#33](https://gitlab.sintef.no/custusx/fraxinus/-/issues/33) | Updated Raidionics version | Updated to a newer Raidionics AI segmentation version and fixed related batch-size and segmentation-order issues. |
| [Fraxinus#27](https://gitlab.sintef.no/custusx/fraxinus/-/issues/27) | High-resolution CT segmentation failure | Fixed AI segmentation failures on high-resolution (1024×1024) CT images by automatically downsampling to 512×512 before segmentation. |
| [Fraxinus#25](https://gitlab.sintef.no/custusx/fraxinus/-/issues/25) | Correct export of centerlines | Fixed an incorrect coordinate system on exported airway centerlines, which caused misalignment when importing Fraxinus data into other systems. |
| [Fraxinus#23](https://gitlab.sintef.no/custusx/fraxinus/-/issues/23) | Improved nodule segmentation | Replaced the previous lung-nodule segmentation method with TotalSegmentator, removing a dependency-troubled component. |
| [Fraxinus#16](https://gitlab.sintef.no/custusx/fraxinus/-/issues/16) | Limit automatic bronchoscope rotation | Limited automatic bronchoscope camera rotation to the first airway generations, since rotation further out was more distracting than helpful. |
| [Fraxinus#6](https://gitlab.sintef.no/custusx/fraxinus/-/issues/6) | 2D view jumping during target selection | Disabled "center to tool" in 2D views for the Pinpoint and Procedure Planning workflows, since it caused 2D slices to jump distractingly when selecting a new position. |
| [Fraxinus#5](https://gitlab.sintef.no/custusx/fraxinus/-/issues/5) | PET elastix registration misalignment | Fixed a mismatch between CT and PET-CT after PET/Elastix registration by tuning registration parameters. |
| [Fraxinus#3](https://gitlab.sintef.no/custusx/fraxinus/-/issues/3) | Tumor segmentation failure on new GPU hardware | Fixed a GPU-related failure in lung tumor segmentation (LungTumorMask) by forcing it to run on CPU. |
| [CustusX#20](https://gitlab.sintef.no/custusx/custusx/-/issues/20) | GenericScriptFilter path handling | Fixed script configuration file path handling for GenericScriptFilter in CustusS and Fraxinus. |

## Known Issues

| Key | Summary | Release Note |
|---|---|---|
| [CustusX#30](https://gitlab.sintef.no/custusx/custusx/-/issues/30) | Tracking on Ubuntu 24.04 | Tracking hardware support is not available in the open-source Fraxinus build on Ubuntu 24.04. |

<details>
<summary>Internal changes not included above (CI/build/tooling)</summary>

- Fraxinus#41 — Update public documentation
- CustusX#14 — Make release builds

</details>
