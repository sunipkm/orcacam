#!/usr/bin/env bash

######################################
# SETTING                            #
######################################
MAINDIR=$(cd $(dirname $0) && pwd)
TARGET_RULES_DIR="/etc/udev/rules.d"

sudo cp -f $MAINDIR/udev/rules.d/55-hamamatsu_dcamusb.rules $TARGET_RULES_DIR

ID=$(whoami)
sudo gpasswd -a $ID plugdev 

echo "USB Driver installed."

exit 0