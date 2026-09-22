# Fraxinus release notes — v26.09

_Changes since v26.08_

## New Features

| Key | Summary | Release Note |
|---|---|---|
| [CustusX#44](https://gitlab.sintef.no/custusx/custusx/-/issues/44) | Windows installer component descriptions | Hovering over an optional component (Raidionics, TotalSegmentator, Elastix) in the Windows installer now shows its description, and the installer pre-checks/unchecks components based on whether they're already installed. The uninstaller can now optionally remove these external tools' virtual environments too. |

## Bugfixes

| Key | Summary | Release Note |
|---|---|---|
| [CustusX#46](https://gitlab.sintef.no/custusx/custusx/-/issues/46) | Missing glew dependency | The Ubuntu install script now installs `libglew-dev` and other missing dependencies, fixing a "missing glew library" error on a fresh install. |
| [CustusX#46](https://gitlab.sintef.no/custusx/custusx/-/issues/46) | False "DICOM data was not found" warning | This warning no longer appears incorrectly when importing a non-DICOM file (e.g. an mhd file). |
| [CustusX#46](https://gitlab.sintef.no/custusx/custusx/-/issues/46) | DICOM import false rejection | Relaxed gantry tilt tolerance during DICOM import so valid series with a near-zero tilt are no longer incorrectly rejected. |
| [CustusX#46](https://gitlab.sintef.no/custusx/custusx/-/issues/46) | Scripted filter reliability | Fixed the TotalSegmentator/Raidionics-based lung segmentation filters sometimes reporting Stop as success, a race condition, and a rare freeze when finishing. |
| [CustusX#46](https://gitlab.sintef.no/custusx/custusx/-/issues/46) | Unresponsive during DICOM import | Importing DICOM data (e.g. from a USB drive) no longer makes Fraxinus appear "not responding" during long scans; the progress dialog's Cancel button now also works while a folder is being scanned. |
| [CustusX#46](https://gitlab.sintef.no/custusx/custusx/-/issues/46) | Scrolling a view only worked over its scrollbar | Scrolling a view now works from anywhere inside it, not just directly on the scrollbar — for example the structures-selection widget shown after a segmentation finishes, which previously only responded to the mouse wheel exactly over its own scrollbar. Scrolling over a dropdown/spin box/slider/tab bar still changes its value (or switches tabs) as before once you've clicked into it. |
| [CustusX#46](https://gitlab.sintef.no/custusx/custusx/-/issues/46) | "Load existing patient" opened the wrong folder | The "Load existing patient" list now correctly shows patients from the current `~/Fraxinus/Patients` folder for anyone using an old Fraxinus_settings, instead of getting stuck on the old `~/Patients` location (which this dialog has no way to browse away from). |

## Known Issues

| Key | Summary | Release Note |
|---|---|---|
| [CustusX#30](https://gitlab.sintef.no/custusx/custusx/-/issues/30) | Tracking on Ubuntu 24.04 | Tracking hardware support is not available in the open-source Fraxinus build on Ubuntu 24.04. |
| [CustusX#42](https://gitlab.sintef.no/custusx/custusx/-/issues/42) | Windows testing coverage | The Windows build has not been as thoroughly tested and verified as the Ubuntu builds. Ubuntu remains the primary, best-tested platform. |

<details>
<summary>Internal changes not included above (CI/build/tooling)</summary>

- CustusX#43 — Windows build object-file paths were close to Windows' 260-char MAX_PATH limit; shortened the default Windows release build root to restore margin
- CustusX#44 — Fraxinus and FraxinusExcelsior now share venvs/models/Patients under a common family folder on both Linux and Windows
- CustusX#46 — Various Python release-tooling fixes and test coverage (git sync robustness, release tagging, CI); `syncToGitRef()` no longer fails merging on a reused CI runner with no git identity configured
- CustusX#50 — Reworked external-libs CI caching into independently-published, per-library packages instead of one all-or-nothing combined package, with cleanup of orphaned/old packages and separated plain/igstk cache keys to fix a wrong-VTK_DIR CMake error

</details>
