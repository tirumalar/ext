#!/bin/bash

INDEX=`grep -e "GRI\.cameraID\=.*" Eyelock.ini | cut -f2 -d'='`
if [ $INDEX == 0 ]
then
	/home/root/nano_led 100 80 0 1
	NOW=$(date +"%Y-%m-%d, %T.000")
	echo "$NOW, ERROR , [Eyelock], - mt9p001 driver resetting" >> /home/root/nxtLog.log
	cd /home/root
	rm Eyelock.run
	i2cset -y 3 0x2e 4 6
	sleep 1
	i2cset -y 3 0x2e 4 6
	sleep 1
	i2cset -y 3 0x2e 4 6
	sleep 1
	p=$(pidof Eyelock)
	kill -9 $p
	#wait for Eyelock to exit
	sleep 5
	rmmod mt9p001.ko
	sleep 2 
	insmod mt9p001.ko master_mode=1
	sleep 2
	touch Eyelock.run
	#wait for Eyelock to start
	sleep 60
	i2cset -y 3 0x2e 4 7
	sleep 1
	i2cset -y 3 0x2e 4 7
	sleep 1
fi

