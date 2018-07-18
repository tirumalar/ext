#!/bin/sh

host=192.168.10.100
app=FocusEyeSelector
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
tftp -g -r libcxcore.so.2 $host
tftp -g -r libcv.so.2 $host
tftp -g -r libhighgui.so.2 $host
tftp -g -r libz.so.1 $host
#tftp -g -r libEyeSegmentationLib.so $host
#tftp -g -r libBiOmega.so $host

chmod a+x *.so*
fi

cd

if [ $skip_data ]
then
	echo Skipping data update
else
	mkdir -p data/Input
	mkdir -p data/Output
	echo getting data
	for i in 44 45 46 47 48 49 50 100 101 102 103; do echo $i; tftp -g -r data_focus/EyeSwipeMini/LeftCamera/eye_$i.pgm -l data/Input/eye_$i.pgm $host; done
	#for i in 0 1; do echo $i; tftp -g -r data_eyeSeg/test/Test$i.pgm -l data/test/Test$i.pgm $host; done
	#tftp -g -r data_eyeSeg/database/IrisCodeDatabase.bin -l data/database/IrisCodeDatabase.bin $host
fi