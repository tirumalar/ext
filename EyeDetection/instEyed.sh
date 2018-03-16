#!/bin/sh

host=192.168.10.3
app=EyeDetection
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

chmod a+x *.so*
fi

cd

if [ $skip_data ]
then
	echo Skipping data update
else
	mkdir data
	echo getting images
	for i in ir0_070703092824_890_match_Juan_Pablo_0_285599 ir0_070703122457_390_match_David_0_268222 ir1_070623155542_859_match_Ella_0_181898 real_eye fake_eye; do echo $i; tftp -g -r data/$i.pgm $host; done
#	for i in ir0_070622145827_968_match_Ramsay_0_264785 ir0_070622145811_062_match_Ramsay_0_189005 ir0_070622145915_796_match_Ramsay_0_172097 ir0_070622145828_031_match_Ramsay_0_293570; do echo $i; tftp -g -r data/$i.pgm $host; done
	
#	for i in 0 1 2 3 4 5 6 7 8 9; do for j in 0 1 2 3 4 5 6 7 8 9; do for k in 0 1; do echo eye0$i$j\_$k.pgm; tftp -g -r data/eye0$i$j\_$k.pgm $host; done; done; done
	for i in adaboostClassifier.txt; do echo $i; tftp -g -r data/$i $host; done
fi