# Fraxinus
An open source, software only, navigation system for bronchoscopy.

Fraxinus is based on [CustusX](https://github.com/SINTEFMedtek/CustusX).

See [custusx.org](https://www.custusx.org/index.php/applications/fraxinus) for more.

### Build instructions

Build instructions are available [here](https://www.custusx.org/uploads/fraxinus/developer_doc/nightly/build_instructions.html).
However, the script to be run should be:

        mkdir fx
        cd fx 
        git clone git@github.com:SINTEFMedTek/Fraxinus.git FX/FX 
        ./FX/FX/script/cxFraxinusInstaller.py --full --all -t Release --user_doc

Run `cxPrivateInstaller.py -h` for more options.
