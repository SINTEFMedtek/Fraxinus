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
sudo apt install software-properties-common -y #Needed for Python 3.10 on Ubuntu20.04
sudo add-apt-repository ppa:deadsnakes/ppa -y #Needed for Python 3.10 on Ubuntu20.04
sudo apt install -y python3.8-venv
sudo apt install -y python3.10-venv
sudo apt install -y libpcre2-16-0
sudo apt install -y libdouble-conversion3
sudo apt install -y git
sudo apt install wget

#Unpack compressed archive to /home/username/Fraxinus
if [ -d "Fraxinus_temp" ]; then
  rm -rf Fraxinus_temp #Remove if it exists
fi
mkdir Fraxinus_temp
tar -xf Fraxinus.tar.xz -C Fraxinus_temp
cd Fraxinus_temp/Fraxinus
FRAXINUS_PATH=$(ls -d Fraxinus_*)
mkdir -p ~/Fraxinus #Make dir if it does not exist
cd ../..
cp -r Fraxinus_temp/Fraxinus/* ~/Fraxinus #Copy and replace
rm -rf Fraxinus_temp


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
cp -r ~/Fraxinus/models/raidionics_models/CT_Tumor ~/Fraxinus_settings/models/raidionics_models/

#install elastix
if command -v elastix > /dev/null 2>&1; then
  echo "Elastix is already installed"
else
  echo "Installing Elastix"
  wget https://github.com/SuperElastix/elastix/releases/download/5.1.0/elastix-5.1.0-linux.zip
  unzip -o elastix-5.1.0-linux.zip -d elastix
  chmod +x elastix/bin/elastix
  chmod +x elastix/bin/transformix
  cp elastix/lib/libANNlib* elastix/bin/
  echo "Installing Elastix paths in .bashrc"
  echo '' >> ~/.bashrc
  echo '#Path to Elastix installation' >> ~/.bashrc
  echo 'export PATH=$HOME/Fraxinus/elastix/bin:$PATH' >> ~/.bashrc
  echo 'export LD_LIBRARY_PATH=$HOME/Fraxinus/elastix/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
  source ~/.bashrc
fi

# Update desktop launcher with correct paths
mv Fraxinus.desktop Fraxinus.desktop-bak
sed -e "s,Icon=.*,Icon=$HOME/Fraxinus/Icon/Fraxinus.icns,g" Fraxinus.desktop-bak > Fraxinus.desktop
mv Fraxinus.desktop Fraxinus.desktop-bak
sed -e "s,Path=.*,Path=$HOME/Fraxinus/$FRAXINUS_PATH/Fraxinus,g" Fraxinus.desktop-bak > Fraxinus.desktop
rm Fraxinus.desktop-bak

#Copy desktop launcher, and make it executable
cp Fraxinus.desktop ~/Desktop
gio set ~/Desktop/Fraxinus.desktop metadata::trusted true
chmod +x ~/Desktop/Fraxinus.desktop


#Create virtual python environments
rm -R raidionicsVenv
python3.8 -m venv raidionicsVenv
source raidionicsVenv/bin/activate
pip install --upgrade pip
pip install git+https://github.com/dbouget/raidionics-rads-lib.git
deactivate


mkdir TotalSegmentator
cd TotalSegmentator
mkdir segmentations
python3.10 -m venv venv
source venv/bin/activate
pip install --upgrade pip
pip install TotalSegmentator
totalseg_download_weights -t total
totalseg_download_weights -t lung_vessels
totalseg_download_weights -t lung_nodules
deactivate


#Add more swap space for AI networks if it does not exist
if ! grep -q '/swapfile swap swap defaults' /etc/fstab; then
	sudo fallocate -l 10G /swapfile
	sudo chmod 600 /swapfile
	sudo mkswap /swapfile
	sudo swapon /swapfile
	echo '/swapfile swap swap defaults 0 0' | sudo tee -a /etc/fstab
fi


echo '----------Fraxinus installation completed-------------'
