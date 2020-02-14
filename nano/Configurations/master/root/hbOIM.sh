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
	EyelockLog "pinging OIM ${OIM_IP}..." 'TRACE'
	if ping -c 5 "${OIM_IP}"
	then
		EyelockLog "OIM ${OIM_IP} is pingable, sleeping ${TIMEOUT} seconds" 'TRACE'
#		touch "${FACETRACKER_RUN_FILE}"
		touch "${EYELOCK_RUN_FILE}"
		COUNTER=0	
		break	
	else
		EyelockLog "OIM ${OIM_IP} is NOT pingable!" 'ERROR'

		if [[ ${COUNTER} < ${MAX_SEQUENTIAL_RECOVERIES} ]]
		then

			if [[ -f ${FLAG} ]]
			then
				TIME_SINCE_LAST_RECOVERY=$(($(date +%s) - $(date +%s -r "${FLAG}")))
				EyelockLog "time since last recovery: $TIME_SINCE_LAST_RECOVERY" 'DEBUG'
			fi

			if [[ ${TIME_SINCE_LAST_RECOVERY} -ge ${RECOVERY_MIN_INTERVAL} ]] 
			then
				EyelockLog "recovering..." 'DEBUG'
				rm "${EYELOCK_RUN_FILE}"				
				#rm "${FACETRACKER_RUN_FILE}"

				#FCTKR_PID=$(ps -ef | grep '/FaceTracker 8194 1$' | awk '{print $2}')
				#if [[ ! -z ${FCTKR_PID} ]]
				#then
				#	EyelockLog "killing FaceTracker, PID: $FCTKR_PID"
				#	kill -9 "${FCTKR_PID}"
				#fi

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
				/home/root/i2cHandler -r1
				sleep 30
				ifdown eth0
				sleep 5
				ifup eth0
				sleep 5
				/sbin/ethtool -s eth0 speed 1000 autoneg off duplex full

				touch "${FLAG}"
				((COUNTER++))

				sleep 10
			else
				EyelockLog "too soon to recover, skipping..." 'DEBUG'
			fi
		else
			EyelockLog "${COUNTER} sequential recoveries. Device must be powercycled" 'ERROR'
			/home/root/i2cHandler -l 6
			exit 1
		fi
	fi
	sleep "${TIMEOUT}"	
done



