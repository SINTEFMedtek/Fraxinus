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
# For the installation of apt packages a user password needs to be entered as well
#
##########################################################################################################

#Install apt packages needed to run Fraxinus
sudo apt-get -y update
sudo apt-get -y upgrade
sudo apt install -y python3.8-venv
sudo apt install -y libpcre2-16-0
sudo apt install -y libdouble-conversion3
sudo apt install -y git
sudo apt install wget

#Unpack compressed archive to /home/username/Fraxinus
tar -xf Fraxinus.tar.xz -C ~

#Place Raidionics AI networks in correct position
mkdir ~/Fraxinus_settings
mkdir ~/Fraxinus_settings/models
mkdir ~/Fraxinus_settings/models/raidionics_models
cd ~/Fraxinus
cp -r ~/Fraxinus/models/raidionics_models/CT_Airways ~/Fraxinus_settings/models/raidionics_models/
cp -r ~/Fraxinus/models/raidionics_models/CT_Lungs ~/Fraxinus_settings/models/raidionics_models/
cp -r ~/Fraxinus/models/raidionics_models/CT_LymphNodes ~/Fraxinus_settings/models/raidionics_models/
cp -r ~/Fraxinus/models/raidionics_models/CT_MediumOrgansMediastinum ~/Fraxinus_settings/models/raidionics_models/
cp -r ~/Fraxinus/models/raidionics_models/CT_PulmSystHeart ~/Fraxinus_settings/models/raidionics_models/
cp -r ~/Fraxinus/models/raidionics_models/CT_SmallOrgansMediastinum ~/Fraxinus_settings/models/raidionics_models/

#install elastix
if command -v elastix > /dev/null 2>&1; then
  echo "Elastix is already installed"
else
  echo "Installing Elastix"
  wget https://github.com/SuperElastix/elastix/releases/download/5.1.0/elastix-5.1.0-linux.zip
  unzip elastix-5.1.0-linux.zip -d elastix
  chmod +x elastix/bin/elastix
  chmod +x elastix/bin/transformix
  echo "Installing Elastix paths in .bashrc"
  echo '' >> ~/.bashrc
  echo '#Path to Elastix installation' >> ~/.bashrc
  echo 'export PATH=$HOME/Fraxinus/elastix/bin:$PATH' >> ~/.bashrc
  echo 'export LD_LIBRARY_PATH=$HOME/Fraxinus/elastix/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
  source ~/.bashrc
fi

FRAXINUS_PATH=$(ls -d Fraxinus_*)

# Update desktop launcher with correct paths
mv Fraxinus22.04.desktop Fraxinus22.04.desktop-bak
sed -e "s,Icon=.*,Icon=$HOME/Fraxinus/Icon/Fraxinus.icns,g" Fraxinus22.04.desktop-bak > Fraxinus22.04.desktop
mv Fraxinus22.04.desktop Fraxinus22.04.desktop-bak
sed -e "s,Path=.*,Path=$HOME/Fraxinus/$FRAXINUS_PATH/Fraxinus,g" Fraxinus22.04.desktop-bak > Fraxinus22.04.desktop
rm Fraxinus22.04.desktop-bak

#Copy desktop launcher, and make it executable
cp Fraxinus22.04.desktop ~/Desktop
gio set ~/Desktop/Fraxinus22.04.desktop metadata::trusted true
chmod +x ~/Desktop/Fraxinus22.04.desktop


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

python3 -m venv venvLungTumorMask
source venvLungTumorMask/bin/activate
pip install --upgrade pip
pip install https://github.com/VemundFredriksen/LungTumorMask/releases/download/v1.2.1/lungtumormask-1.2.1-py2.py3-none-any.whl
deactivate

#Add more swap space for AI networks
sudo fallocate -l 10G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile
