#!/bin/sh

host=192.168.10.3
app=EyeSeg	


echo getting latest executable
tftp -g -r $app $host
#tftp -g -r $app.ini $host
chmod a+x ./$app
skip_setup=
skip_data=
if [ $skip_setup ]
then
	echo skipping setup
else
cd /usr/lib
#tftp -g -r libMamigoBaseIPL.so $host
tftp -g -r libcxcore.so.2 $host
tftp -g -r libcv.so.2 $host
tftp -g -r libhighgui.so.2 $host
tftp -g -r libz.so.1 $host

chmod a+x *.so*
fi

cd

if [ $skip_data ]
then
	echo skipping data update
else
	echo getting  images
	for i in input_1 mask_1 input_2 mask_2 input_3 mask_3; do tftp -g -r $i.ppm $host; done
fi