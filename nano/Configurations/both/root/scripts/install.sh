#!/bin/sh

# keep the tar file in /etc/skel
# start running the install script ,with untar the file in /etc/skel
# run the script by moving the necessary file to corresponding file location

SKEL_DIR_PATH="/home/root"
cd $SKEL_DIR_PATH

#if [ -e *.tar.gz ] ; then  # -e : exist, -x : execute permission
#echo "file: " *.tar.gz
#tar -zxvf *.tar.gz
#else
#echo "file tar.gz not available"
#exit
#fi

/bin/sh "$SKEL_DIR_PATH/makeEyelkUsrFile.sh"
echo "moved the required files to required dir location"
#/bin/sh /etc/init.d/createEyelockUsrAc.sh
