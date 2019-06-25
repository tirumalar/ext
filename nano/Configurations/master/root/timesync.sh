#!/bin/bash

source '/home/root/EyelockLog.sh'
EYELOCK_LOGLABEL='TIMESYNC'

while true
do
TIMESERVER=`grep -e "GRI\.InternetTimeAddr\=.*" /home/root/Eyelock.ini | cut -f2 -d'='`
SYNCENABLE=`grep -e "GRI\.InternetTimeSync\=.*" /home/root/Eyelock.ini | cut -f2 -d'='`
echo "${TIMESERVER}"
echo "${SYNCENABLE}"
sleep 86400
echo time sync started
if [ $SYNCENABLE == true ]
then
	NOW=$(date -u +"%Y-%m-%d %T, %Z")
	echo "$NOW > Eyelock EXT Clock Sync" >> /home/root/nxtEvent.log

	NOW=$(date +"%Y-%m-%d, %T.000")
	#echo "$NOW, INFO , [Eyelock], - Eyelock EXT Clock Sync" >> /home/root/nxtLog.log
	EyelockLog "Eyelock EXT Clock Sync" 'INFO'
	
	if /home/root/ntpdate -s -t 3 "$TIMESERVER"
	then
		#echo "$NOW, INFO , [Eyelock], - daily sync: synchronized successfully with $TIMESERVER" >> /home/root/nxtLog.log
		EyelockLog "daily sync: synchronized successfully with $TIMESERVER" 'INFO'
		RESULT=$(printf "TIMESYNC_MSG;\n" | nc -w 10 127.0.0.1 8085)
		NOW=$(date +"%Y-%m-%d, %T.000")
		if [[ ${RESULT} == 'RTCSUCCESS;' ]]
		then
			#echo "$NOW, INFO , [Eyelock], - daily sync: RTC synchronized successfully" >> /home/root/nxtLog.log
			EyelockLog "daily sync: RTC synchronized successfully" 'INFO'
		else
			#echo "$NOW, ERROR , [Eyelock], - daily sync: failed to synchronize RTC" >> /home/root/nxtLog.log
			EyelockLog "daily sync: failed to synchronize RTC" 'ERROR'
		fi
	else
		#echo "$NOW, ERROR , [Eyelock], - daily sync: failed to synchronize clock with $TIMESERVER" >> /home/root/nxtLog.log
		EyelockLog "daily sync: failed to synchronize clock with $TIMESERVER" 'ERROR'
	fi
fi
echo time sync ended
done

