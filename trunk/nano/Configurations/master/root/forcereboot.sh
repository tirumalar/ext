#!/bin/bash

OIM_IP='192.168.4.172'
TIMEOUT='5' # seconds

FLAG='/home/root/hbOIM_failure'
RECOVERY_MIN_INTERVAL=30 # seconds
TIME_SINCE_LAST_RECOVERY=30 # initialize to 
COUNTER=0
MAX_SEQUENTIAL_RECOVERIES=3
FACETRACKER_RUN_FILE='/home/root/FaceTracker.run'
EYELOCK_RUN_FILE='/home/root/Eyelock.run'
EYELOCK_OIM_TAMPER_FILE='/home/root/OIMtamper'
EYELOCK_TAMPER_FILE='/home/root/tamper'

cd /home/root/
rm "${FLAG}"

rm "${FACETRACKER_RUN_FILE}"
rm "${EYELOCK_RUN_FILE}"
rm "${EYELOCK_OIM_TAMPER_FILE}"
rm "${EYELOCK_TAMPER_FILE}"

source '/home/root/EyelockLog.sh'
EYELOCK_LOGLABEL='HBOIM'

while true
do
	EyelockLog "restarting EXT from script" 'TRACE'

	ELK_PID=$(ps -ef | grep '/Eyelock$' | awk '{print $2}')
	if [[ ! -z ${ELK_PID} ]]
	then
		EyelockLog "killing Eyelock, PID: $ELK_PID" 'DEBUG'
		kill -9 "${ELK_PID}"
		killall -KILL Eyelock
	fi

	sleep 1
	/home/root/i2cHandler -r0
	sleep 5
	reboot
done


