# Fraxinus release notes — v26.08

_Changes since v22.09_

## New Features

| Key | Summary | Release Note |
|---|---|---|
| [Fraxinus#42](https://gitlab.sintef.no/custusx/fraxinus/-/issues/42) | Make self-extracting Fraxinus Ubuntu installer | Added a single-file Ubuntu installer script that downloads and sets up Fraxinus automatically. |
| [Fraxinus#41](https://gitlab.sintef.no/custusx/fraxinus/-/issues/41) | Update public documentation | Updated the public Fraxinus website with current workflow description and screenshots. |
| [Fraxinus#40](https://gitlab.sintef.no/custusx/fraxinus/-/issues/40) | Fraxinus public version | Fraxinus is now available as an open-source public version, with tracking-related code moved to a separate private component. |
| [Fraxinus#37](https://gitlab.sintef.no/custusx/fraxinus/-/issues/37) | Layout option in Fraxinus VB | Added a virtual bronchoscopy layout option with larger VB + ACS + 3D views, and made the screenshot/video toolbar visible by default. |
| [Fraxinus#26](https://gitlab.sintef.no/custusx/fraxinus/-/issues/26) | EBUS simulator add abdominal view | Added an abdominal 2D view to the EBUS simulator, to help identify lymph nodes in CT. |
| [Fraxinus#13](https://gitlab.sintef.no/custusx/fraxinus/-/issues/13) | MDT widget | Added an MDT (multidisciplinary team meeting) widget and workflow, showing clinical variables and object colors read from a CSV file. |
| [Fraxinus#21](https://gitlab.sintef.no/custusx/fraxinus/-/issues/21) | Improve data import flow and info in Fraxinus | Added informational dialogs when a patient is created, data is imported, and segmentation is completed, to make the import flow clearer. |
| [Fraxinus#39](https://gitlab.sintef.no/custusx/fraxinus/-/issues/39) | Improvements in Fraxinus | Various Fraxinus usability improvements: square virtual bronchoscopy view, full centerline tree shown in VB, automatic CT/PET modality assignment on import, and a streamlined patient loading and segmentation workflow with inline progress. |
| [Fraxinus#38](https://gitlab.sintef.no/custusx/fraxinus/-/issues/38) | Make simpler virtual camera rotation | Simplified automatic camera rotation in virtual bronchoscopy, based on the target's lobe location. |
| [Fraxinus#34](https://gitlab.sintef.no/custusx/fraxinus/-/issues/34) | Simpler CT import in Fraxinus | Simplified the CT import and patient creation flow in Fraxinus with a streamlined dialog. |
| [Fraxinus#35](https://gitlab.sintef.no/custusx/fraxinus/-/issues/35) | Make Fraxinus segmentations run on Windows | Raidionics and TotalSegmentator segmentation now also work on Windows. |
| [Fraxinus#32](https://gitlab.sintef.no/custusx/fraxinus/-/issues/32) | Compile Fraxinus on Windows | Fraxinus can now be built and installed on Windows. |
| [Fraxinus#24](https://gitlab.sintef.no/custusx/fraxinus/-/issues/24) | Smoothing of meshes in Fraxinus | Added configurable smoothing of segmented structure meshes, for smoother-looking and smaller models. |
| [Fraxinus#20](https://gitlab.sintef.no/custusx/fraxinus/-/issues/20) | Integrate TotalSegmentator | Integrated TotalSegmentator for segmenting lung blood vessels and lung lobes. |
| [Fraxinus#19](https://gitlab.sintef.no/custusx/fraxinus/-/issues/19) | Interactive tumor assessment | Added interactive review of segmented tumors and lung nodules: each detected lesion can be confirmed or rejected individually. |
| [Fraxinus#18](https://gitlab.sintef.no/custusx/fraxinus/-/issues/18) | New tumor segmentation with Raidionics | Replaced the previous tumor segmentation (LungTumorMask) with a new AI model running in Raidionics. |
| [Fraxinus#12](https://gitlab.sintef.no/custusx/fraxinus/-/issues/12) | EBUS simulator | Added an EBUS (endobronchial ultrasound) simulator, showing a simulated ultrasound sector from CT during virtual bronchoscopy. |
| [Fraxinus#8](https://gitlab.sintef.no/custusx/fraxinus/-/issues/8) | Fraxinus fixes | Several virtual bronchoscopy improvements: show target and via points during navigation, display segmented structures in 2D view (previously 3D only), and switch to the post-processed airway model for smoother visualization. |
| [Fraxinus#7](https://gitlab.sintef.no/custusx/fraxinus/-/issues/7) | Navigate VB with keys | Added keyboard shortcuts to navigate and control virtual bronchoscopy, including up/down angle adjustment. |

## Bugfixes

| Key | Summary | Release Note |
|---|---|---|
| [Fraxinus#27](https://gitlab.sintef.no/custusx/fraxinus/-/issues/27) | Downsample CT image to 512x512 | Fixed AI segmentation failures on high-resolution (1024x1024) CT images by automatically downsampling to 512x512 before segmentation. |
| [Fraxinus#36](https://gitlab.sintef.no/custusx/fraxinus/-/issues/36) | Centerline generation in separate thread | Fixed the application freezing for about a minute during airway centerline generation, by moving the processing to a background thread. |
| [Fraxinus#33](https://gitlab.sintef.no/custusx/fraxinus/-/issues/33) | Udate raidionics | Updated to a newer Raidionics AI segmentation version and fixed related batch-size and segmentation-order issues. |
| [Fraxinus#25](https://gitlab.sintef.no/custusx/fraxinus/-/issues/25) | Correct export of centerlines | Fixed an incorrect coordinate system on exported airway centerlines, which caused misalignment when importing Fraxinus data into other systems. |
| [Fraxinus#23](https://gitlab.sintef.no/custusx/fraxinus/-/issues/23) | Use nodule segmentation from TotalSegmentator in Fraxinus | Replaced the previous lung-nodule segmentation method with TotalSegmentator, removing a dependency-troubled component. |
| [Fraxinus#16](https://gitlab.sintef.no/custusx/fraxinus/-/issues/16) | Limit automatic bronchoscope rotation | Limited automatic bronchoscope camera rotation to the first airway generations, since rotation further out was more distracting than helpful. |
| [Fraxinus#6](https://gitlab.sintef.no/custusx/fraxinus/-/issues/6) | Disable center to tool in 2D | Disabled "center to tool" in 2D views for the Pinpoint and Procedure Planning workflows, since it caused 2D slices to jump distractingly when selecting a new position. |
| [Fraxinus#5](https://gitlab.sintef.no/custusx/fraxinus/-/issues/5) | PET elastix registration fails | Fixed a mismatch between CT and PET-CT after PET/Elastix registration by tuning registration parameters. |
| [Fraxinus#3](https://gitlab.sintef.no/custusx/fraxinus/-/issues/3) | Fix bug in LungTumorMask | Fixed a GPU-related failure in lung tumor segmentation (LungTumorMask) by forcing it to run on CPU. |
| [CustusX#20](https://gitlab.sintef.no/custusx/custusx/-/issues/20) | GenericScriptFilter issues | Fixed script configuration file path handling for GenericScriptFilter in CustusS and Fraxinus. |

<details>
<summary>Internal changes not included above (CI/build/tooling)</summary>

- CustusX#14 — Make release builds

</details>
