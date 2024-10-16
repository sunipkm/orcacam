#!/usr/bin/env bash

# Settings
MAINDIR=$(cd $(dirname $0) && pwd)
UNINSTALL_DIR="/usr/local/hamamatsu_dcam/api"

if [ ! -d $UNINSTALL_DIR ]; then
	echo "Warnig : \"$UNINSTALL_DIR\" is not installed."	
	exit 0
fi

FLG_FORCE=0
if [ $FLG_FORCE == 0 ]; then
	echo "Do you want to remove \"$UNINSTALL_DIR\"? (Y/n)"
	read ans
	if [ "x$ans" == "xn" ]; then
		exit 0
	fi
fi

######################################
# module uninstall                   #
######################################
if [ -e /etc/ld.so.conf.d/hamamatsu_dcam.conf ]; then
	sudo rm -f /etc/ld.so.conf.d/hamamatsu_dcam.conf
fi
if [ -e /etc/ld.so.conf.d/dcam.conf ]; then
	sudo rm -f /etc/ld.so.conf.d/dcam.conf
fi
if [ -e /usr/local/lib/libdcamapi.so ]; then
	sudo rm -f /usr/local/lib/libdcamapi.so*
fi

######################################
# directory uninstall                #
######################################
sudo rm -f $UNINSTALL_DIR/$(basename $0)
#sudo rm -rf $UNINSTALL_DIR.$DCAM_VER
UNINSTALL_ARRAY=($(find $UNINSTALL_DIR -follow -maxdepth 1 -name "uninstall_*.sh"))
if [ "x$UNINSTALL_ARRAY" == "x" ]; then
	sudo rm -rf $UNINSTALL_DIR
fi

UNINSTALL_MAIN="/usr/local/hamamatsu_dcam"
DIR_COUNT=$(ls -l $UNINSTALL_MAIN | grep ^d | wc -l)
if [ $DIR_COUNT -eq 0 ]; then
	sudo rm -rf $UNINSTALL_MAIN
fi

sudo ldconfig

exit 0