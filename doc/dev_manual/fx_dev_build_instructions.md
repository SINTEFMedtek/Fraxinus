Build instructions {#build_instructions}
===================

Recommended build procedure {#build_instructions2}
===================

Make sure you:
* Are on a \ref supported_platforms.
* Fulfill the \ref prerequisites.
* Have [Git](https://git-scm.com/) installed.

Choose a root folder for the project, e.g. `~/dev/fx`. It will be populated as described in \ref build_instructions_folder_structure.

Then run the following commands:

    mkdir dev
    mkdir dev/fx
    cd dev/fx
    git clone https://gitlab.sintef.no/custusx/fraxinus.git FX/FX
    cd FX/FX
    ./script/cxFraxinusInstaller.py --full --all --build_type Release --user_doc

Run `cxFraxinusInstaller.py -h` for a list of available components and options.
The `--full` argument is a combination of the following arguments:

 * `--checkout:` download the source code repositories of the selected components.
 * `--configure:` configure and run CMake to generate build files.
 * `--make:` build the selected components.

The `--all` argument selects all components for building. After having run the script successfully, you might want to
drop this argument and instead list the components you want to run commands on, primarily *Fraxinus*.


Prerequisites {#prerequisites}
------------------------

The following software must be installed prior to building *Fraxinus*.

Several other libraries are part of the \ref dev_superbuild,
and thus do not need to be installed separately.

For convenience, setup scripts for some platforms are available in the
repository. Look for your platform in
[script/cxsetup](https://gitlab.sintef.no/custusx/fraxinus/-/tree/develop/script/cxsetup).

### Linux

On Ubuntu, most required packages can be installed via the package manager.
See the platform setup scripts for the complete list.

**Python and Git**: Make sure that *Python* and *Git* are installed and available on the command line.

### Windows

The Windows build environment requires *Visual Studio* and several additional tools.
An updated build script for Windows is available under
[script/cxsetup](https://gitlab.sintef.no/custusx/fraxinus/-/tree/develop/script/cxsetup).
Follow the instructions in the README found there.

After a successful build, use the `set_run_environment.bat` script in the build folder to start
*Qt Creator* or launch *Fraxinus* directly:

    set_run_environment.bat Fraxinus.exe

Inference engines
-----------------

Fraxinus uses the following inference engines for segmentation and registration.
These are not built from source; the Windows installer includes optional setup components for each.
On Linux, they can be installed separately after building:

- **Raidionics** — deep learning segmentation (airways, lungs, lymph nodes, tumors, and more).
- **TotalSegmentator** — additional anatomical segmentation models.
- **Elastix** — image registration used for PET-to-CT alignment.

## Superbuild Folder Structure {#build_instructions_folder_structure}

The default Fraxinus folder structure follows the same pattern as CustusX.
All libraries and Fraxinus itself are placed within a root folder,
with source and build folders grouped by library.

|        |          |                |
| ------ | ----     | -------------- |
| root   | FX       | FX             |
|        |          | build_Release  |
|        | CX       | CX             |
|        |          | build_Release  |
|        | VTK      | VTK            |
|        |          | build_Release  |
