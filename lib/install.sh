#!/usr/bin/env bash

# Settings
MAINDIR=$(cd $(dirname $0) && pwd)
PROCTYPE=$(uname -m)
if [ $PROCTYPE == "i486" -o $PROCTYPE == "i586" -o $PROCTYPE == "i686" -o $PROCTYPE == "k6" -o $PROCTYPE == "athlon" ]; then
	PROCTYPE="i386"
fi
DCAM_VERSION=$(cd $MAINDIR/$PROCTYPE/core && ls libdcamdig.so.* | cut -d "." -f 3-5)

# Prerequisites
dist=`cat /etc/issue`
if [[ $dist = Ubuntu* ]]; then
	dist=Debian
fi

if [[ $dist = Debian* ]]; then
    dpkg -l libusb-dev > libusbdever;
    if grep -q '0.1.12-32' ./libusbdever; then
        echo "libusb-dev Version 0.1.12-32 found" 
    else
        echo "Purging and installing latest libusb-dev. Should be 0.1.12-32 or higher."
        sudo apt --assume-yes purge libusb-dev
        sudo apt --assume-yes autoremove
        sudo apt --assume-yes install libusb-dev
    fi
    rm libusbdever;

else
    sudo yum install -y libusb-devel
fi

# Directories
ORIGINAL_DCAM_API_DIR="$MAINDIR"
ORIGINAL_DCAM_API_PROC_DIR="$MAINDIR/$PROCTYPE"
ORIGINAL_DCAM_API_CORE_DIR="$MAINDIR/$PROCTYPE/core"
ORIGINAL_DCAM_API_USB3_DIR="$MAINDIR/$PROCTYPE/usb3"
ORIGINAL_DCAM_API_LDSO_DIR="$MAINDIR/etc/ld.so.conf.d"

INSTALL_DCAM_MAIN_DIR="/usr/local/hamamatsu_dcam"
INSTALL_DCAM_API_DIR="$INSTALL_DCAM_MAIN_DIR/api"
INSTALL_DCAM_API_ETC_DIR="$INSTALL_DCAM_API_DIR/etc"
INSTALL_DCAM_API_ETC_MOD_DIR="$INSTALL_DCAM_API_DIR/etc/modules"
INSTALL_DCAM_API_MOD_DIR="$INSTALL_DCAM_API_DIR/modules"

INSTALL_DCAM_LIB_DIR="/usr/local/lib"
INSTALL_LDSO_DIR="/etc/ld.so.conf.d"


# Install
if [ ! -d $INSTALL_DCAM_MAIN_DIR ]; then
	sudo mkdir $INSTALL_DCAM_MAIN_DIR
fi

if [ ! -d $INSTALL_DCAM_API_DIR ]; then
	sudo mkdir $INSTALL_DCAM_API_DIR
fi

if [ ! -d $INSTALL_DCAM_API_ETC_DIR ]; then
	sudo mkdir $INSTALL_DCAM_API_ETC_DIR
fi

if [ ! -d $INSTALL_DCAM_API_ETC_MOD_DIR ]; then
	sudo mkdir $INSTALL_DCAM_API_ETC_MOD_DIR
fi

if [ ! -d $INSTALL_DCAM_API_MOD_DIR ]; then
	sudo mkdir $INSTALL_DCAM_API_MOD_DIR
fi

if [ ! -e $INSTALL_DCAM_API_DIR/libdcamapi.so.$DCAM_VERSION ]; then
	sudo rm -rf $INSTALL_DCAM_API_DIR/libdcamapi.so.*
	sudo cp -f $ORIGINAL_DCAM_API_CORE_DIR/libdcamapi.so.$DCAM_VERSION $INSTALL_DCAM_API_DIR
fi

if [ ! -e $INSTALL_DCAM_API_MOD_DIR/libdcamdig.so.$DCAM_VERSION ]; then
	sudo rm -rf $INSTALL_DCAM_API_MOD_DIR/libdcamdig.so*
	sudo cp -f $ORIGINAL_DCAM_API_CORE_DIR/libdcamdig.so.$DCAM_VERSION $INSTALL_DCAM_API_MOD_DIR
fi

sudo cp -f $ORIGINAL_DCAM_API_CORE_DIR/dcamlog.conf $INSTALL_DCAM_API_ETC_DIR

if [ -e $ORIGINAL_DCAM_API_LDSO_DIR/hamamatsu_dcam.conf ]; then
	sudo cp -f $ORIGINAL_DCAM_API_LDSO_DIR/hamamatsu_dcam.conf $INSTALL_LDSO_DIR
elif [ -e $ORIGINAL_DCAM_API_LDSO_DIR/dcam.conf ]; then
	sudo cp -f $ORIGINAL_DCAM_API_LDSO_DIR/dcam.conf $INSTALL_LDSO_DIR
else
	echo "Not Found Conf File of DCAM-API."
fi

sudo cp -f $ORIGINAL_DCAM_API_CORE_DIR/dcamdig.conf $INSTALL_DCAM_API_ETC_MOD_DIR

sudo sed -i -e "/^module.version/d" $INSTALL_DCAM_API_ETC_MOD_DIR/dcamdig.conf
sudo sed -i -e "/^END_OF_FILE$/i module.version\t$DCAM_VERSION" $INSTALL_DCAM_API_ETC_MOD_DIR/dcamdig.conf

if [ ! -e $INSTALL_DCAM_API_MOD_DIR/libfgusb3.so.$DCAM_VERISON ]; then
    sudo rm -rf $INSTALL_DCAM_API_MOD_DIR/libfgusb3.so*
    sudo cp -f $ORIGINAL_DCAM_API_USB3_DIR/libfgusb3.so.$DCAM_VERSION $INSTALL_DCAM_API_MOD_DIR
fi
sudo cp -f $ORIGINAL_DCAM_API_USB3_DIR/fgusb3.conf $INSTALL_DCAM_API_ETC_MOD_DIR
sudo sed -i -e "/^module.version/d" $INSTALL_DCAM_API_ETC_MOD_DIR/fgusb3.conf
sudo sed -i -e "/^END_OF_FILE$/i module.version\t$DCAM_VERSION" $INSTALL_DCAM_API_ETC_MOD_DIR/fgusb3.conf
echo "USB3 Module installed."


sudo rm -f $INSTALL_DCAM_LIB_DIR/libdcamapi.so
cd $INSTALL_DCAM_LIB_DIR
sudo ln -s $INSTALL_DCAM_API_DIR/libdcamapi.so.$DCAM_VERSION $INSTALL_DCAM_LIB_DIR/libdcamapi.so

cd $INSTALL_DCAM_API_MOD_DIR
SO_FILE_ARRAY=($(cd $INSTALL_DCAM_API_MOD_DIR && ls -1))
for SO_FILE_ARRAY in ${SO_FILE_ARRAY[@]}
do
	if [ ! -e $(echo "$SO_FILE_ARRAY" | cut -f1 -d'.').so ]; then
		sudo ln -s $SO_FILE_ARRAY $(echo "$SO_FILE_ARRAY" | cut -f1 -d'.').so
	fi
done

sudo cp $ORIGINAL_DCAM_API_DIR/uninstall* $INSTALL_DCAM_API_DIR
sudo /sbin/ldconfig

sudo chmod -R 0755 $INSTALL_DCAM_API_DIR

exit 0