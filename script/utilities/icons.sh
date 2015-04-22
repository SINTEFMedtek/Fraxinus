#!/bin/bash

######
#
# Script for making icons on mac
#
# http://stackoverflow.com/questions/12306223/how-to-manually-create-icns-files-using-iconutil
#
######

clear
echo *** Making a set of icons for use on mac ***
mkdir Fraxinus.iconset
sips -z 16 16     Fraxinus.png --out Fraxinus.iconset/icon_16x16.png
sips -z 32 32     Fraxinus.png --out Fraxinus.iconset/icon_16x16@2x.png
sips -z 32 32     Fraxinus.png --out Fraxinus.iconset/icon_32x32.png
sips -z 64 64     Fraxinus.png --out Fraxinus.iconset/icon_32x32@2x.png
sips -z 128 128   Fraxinus.png --out Fraxinus.iconset/icon_128x128.png
sips -z 256 256   Fraxinus.png --out Fraxinus.iconset/icon_128x128@2x.png
sips -z 256 256   Fraxinus.png --out Fraxinus.iconset/icon_256x256.png
sips -z 512 512   Fraxinus.png --out Fraxinus.iconset/icon_256x256@2x.png
sips -z 512 512   Fraxinus.png --out Fraxinus.iconset/icon_512x512.png
cp Fraxinus.png Fraxinus.iconset/icon_512x512@2x.png
iconutil -c icns Fraxinus.iconset
rm -R Fraxinus.iconset