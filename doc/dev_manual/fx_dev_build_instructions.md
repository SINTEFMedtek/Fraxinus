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

Ubuntu is the primary development and test platform. Windows builds are not part of
the continuous integration pipeline and may be less stable. That said, Fraxinus can
be built on Windows using the same Python build script as on Ubuntu.

Install the following prerequisites before building:

- **Visual Studio 2022** with the **Desktop development with C++** workload.
- **Qt 5.15.2 msvc2019\_64** — install via the Qt Online Installer to `D:\Qt` or `C:\Qt` (the default locations the build script searches).
- **Ninja** — install via `winget install Ninja-build.Ninja` or from [ninja-build.org](https://ninja-build.org).

The build script checks out CustusX automatically as its first step. Just run it directly —
it also detects and initializes the Visual Studio environment automatically:

    python .\script\cxFraxinusInstaller.py --full --all --build_type Release --user_doc

The CustusX repository includes a PowerShell setup script `Setup-CustusX.ps1` that installs
any missing tools (Ninja, GLEW) via winget and vcpkg. Run it **once** (as Administrator) after
the first build has checked out CustusX. If script execution is blocked, first run:

    Set-ExecutionPolicy RemoteSigned -Scope CurrentUser

Then:

    . ..\..\CX\CX\Setup-CustusX.ps1

If Qt is not in a default location, pass `--qt-path`:

    python .\script\cxFraxinusInstaller.py --full --all --build_type Release --user_doc --qt-path "C:\Qt\5.15.2\msvc2019_64"

After the build, use the generated `set_run_environment.bat` in the build folder
to launch *Qt Creator* or *Fraxinus* with the correct DLL paths:

    FX\build_Release\set_run_environment.bat Fraxinus.exe

### Practical information

To start writing code, open the file

    root_dir/CX/CX/CMakeLists.txt

in *Qt Creator.* Make sure that the build folder(s) for your selected configuration(s) (Debug/Release)
matches the build folder(s) in your build tree.

You might need to rerun the build script and CMake to get everything working.

## Running the tests

The test suite can be run through the *Catch* executable, which is built as part of the superbuild.
Run it with the `-h` argument to see the options. To run a specific test:

    ./CX/build_Release/bin/Catch "test name"

Note that the test name must be in quotes. Tests are tagged. To run all the unit tests:

    ./CX/build_Release/bin/Catch [unit]~[hide]~[unstable]~[not_linux]

See the CustusX developer documentation for a full description of the test suite.

## Inference engines

Fraxinus uses the following inference engines for segmentation and registration.
These are not built from source. On Ubuntu they are set up by `installFraxinus.sh`.
On Windows, the generated installer includes optional setup components for each
(using the PowerShell scripts in `org.custusx.fraxinus/`):

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
