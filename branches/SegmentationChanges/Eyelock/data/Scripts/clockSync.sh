#!/bin/bash
RESULT=1
TIMESERVER=$1

source '/home/root/EyelockLog.sh'
EYELOCK_LOGLABEL='TIMESYNC'

NOW=$(date +"%Y-%m-%d, %T.000")
#echo "$NOW, INFO , [Eyelock], - time sync: sync with ${TIMESERVER}" >> /home/root/nxtLog.log
EyelockLog "time sync: sync with ${TIMESERVER}" 'INFO'
#ping -q -c2 $1 > /dev/null
#ping -q -c2 $1
#if [[ $? -ne 0 ]]
#then
#        echo "Host $1 not reachable">> /home/root/nxtLog.log 
#        echo "2" > /home/nxtwResult
#        exit
#fi

#rdate -s $1 2>&1

if /home/root/ntpdate -s -t 3 "$TIMESERVER"
then
	NOW=$(date +"%Y-%m-%d, %T.000")
	#echo "$NOW, INFO , [Eyelock], - time sync: synchronized successfully with $TIMESERVER" >> /home/root/nxtLog.log
	EyelockLog "time sync: synchronized successfully with $TIMESERVER" 'INFO'
	RTC_RESULT=$(printf "TIMESYNC_MSG;\n" | nc -w 10 127.0.0.1 8085)
	NOW=$(date +"%Y-%m-%d, %T.000")
	if [[ ${RTC_RESULT} == 'RTCSUCCESS;' ]]
	then
		#echo "$NOW, INFO , [Eyelock], - time sync: RTC synchronized successfully" >> /home/root/nxtLog.log
		EyelockLog "time sync: RTC synchronized successfully" 'INFO'
		RESULT=0
	else
		#echo "$NOW, ERROR , [Eyelock], - time sync: failed to synchronize RTC" >> /home/root/nxtLog.log
		EyelockLog "time sync: failed to synchronize RTC" 'ERROR'
		RESULT=3
	fi
else
	#echo "$NOW, ERROR , [Eyelock], - time sync: failed to synchronize clock with $TIMESERVER" >> /home/root/nxtLog.log
	EyelockLog "time sync: failed to synchronize clock with $TIMESERVER" 'ERROR'
	RESULT=4
fi

echo "${RESULT}" > /home/clockSyncResult

