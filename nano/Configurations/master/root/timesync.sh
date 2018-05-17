#!/bin/bash

TIMESERVER=`grep -e "GRI\.InternetTimeAddr\=.*" Eyelock.ini | cut -f2 -d'='`
SYNCENABLE=`grep -e "GRI\.InternetTimeSync\=.*" Eyelock.ini | cut -f2 -d'='`
echo $TIMESERVER
echo $SYNCENABLE
while true
do
sleep 86400
echo time sync started
if [ $SYNCENABLE == true ]
then
	NOW=$(date -u +"%Y-%m-%d %T, %Z")
	echo "$NOW > Eyelock NXT Clock Sync" >> /home/root/nxtEvent.log

	NOW=$(date +"%Y-%m-%d, %T.000")
	echo "$NOW, INFO , [Eyelock], - Eyelock NXT Clock Sync" >> /home/root/nxtLog.log
	
	/bin/ntpclient -s -c 1 -i 3 -h $TIMESERVER
fi
echo time sync ended
done

