#!/bin/bash

##########################################################################################################
#
# Fraxinus install script
#
# The Fraxinus.tar.xz file needs to be downloaded and placed together with this script file
#
# The installer must be made executable from the command line:
# chmod +x installFraxinus.sh
#
# Then run like this:
# ./installFraxinus.sh
#
# For the installation of apt packages a user password needs to be entered as well.
#
##########################################################################################################

#Install apt packages needed to run Fraxinus
sudo apt-get -y update
sudo apt-get -y upgrade
sudo apt install -y python3.8-venv
sudo apt install -y libpcre2-16-0
sudo apt install -y libdouble-conversion3
sudo apt install -y git

#Unpack compressed archive to /home/username/Fraxinus
tar -xf Fraxinus.tar.xz -C ~

#Place AI networks in correct position
mkdir ~/Fraxinus_settings
mkdir ~/Fraxinus_settings/models
mkdir ~/Fraxinus_settings/models/raidionics_models
mv models/raidionics_models/CT_Airways ~/Fraxinus_settings/models/raidionics_models/
mv models/raidionics_models/CT_Lungs ~/Fraxinus_settings/models/raidionics_models/
cd ~/Fraxinus

# Update desktop launcher with correct paths
mv Fraxinus22.04.desktop Fraxinus22.04.desktop-bak
sed -e "s,Icon=.*,Icon=$HOME/Fraxinus/Icon/Fraxinus.icns,g" Fraxinus22.04.desktop-bak > Fraxinus22.04.desktop
mv Fraxinus22.04.desktop Fraxinus22.04.desktop-bak
sed -e "s,Path=.*,Path=$HOME/Fraxinus/Fraxinus_2023.05.22-dev+develop.5d0b80_Linux-5.11.0-25-generic/Fraxinus,g" Fraxinus22.04.desktop-bak > Fraxinus22.04.desktop
rm Fraxinus22.04.desktop-bak

#Copy desktop launcher, and make it executable
cp Fraxinus22.04.desktop ~/Desktop
gio set ~/Desktop/Fraxinus22.04.desktop metadata::trusted true
chmod a+x ~/Desktop/Fraxinus22.04.desktop


#Create virtual python environments

python3 -m venv raidionicsVenv
source raidionicsVenv/bin/activate
pip install --upgrade pip
pip install git+https://github.com/dbouget/raidionics-rads-lib.git
deactivate

cd ~/Fraxinus/medtekAI/medtekAI/Docker-DeepSintef
python3 -m venv venv
source venv/bin/activate
pip install --upgrade pip
python -m pip install -r requirements.txt
deactivate
