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

rm "${FLAG}"

rm "${FACETRACKER_RUN_FILE}"
rm "${EYELOCK_RUN_FILE}"
rm "${EYELOCK_OIM_TAMPER_FILE}"
rm "${EYELOCK_TAMPER_FILE}"

while true
do
	echo "pinging OIM ${OIM_IP}..."
	if ping -c 5 "${OIM_IP}"
	then
		echo "OIM ${OIM_IP} is pingable, sleeping ${TIMEOUT} seconds"
		touch "${FACETRACKER_RUN_FILE}"
		COUNTER=0		
	else
		echo "OIM ${OIM_IP} is NOT pingable!"

		if [[ ${COUNTER} < ${MAX_SEQUENTIAL_RECOVERIES} ]]
		then

			if [[ -f ${FLAG} ]]
			then
				TIME_SINCE_LAST_RECOVERY=$(($(date +%s) - $(date +%s -r "${FLAG}")))
				echo "time since last recovery: $TIME_SINCE_LAST_RECOVERY"
			fi

			if [[ ${TIME_SINCE_LAST_RECOVERY} -ge ${RECOVERY_MIN_INTERVAL} ]] 
			then
				echo "recovering..."
				rm "${EYELOCK_RUN_FILE}"				
				rm "${FACETRACKER_RUN_FILE}"

				FCTKR_PID=$(ps -ef | grep '/FaceTracker 8194 1$' | awk '{print $2}')
				if [[ ! -z ${FCTKR_PID} ]]
				then
					echo "killing FaceTracker, PID: $FCTKR_PID"
					kill -9 "${FCTKR_PID}"
				fi

				ELK_PID=$(ps -ef | grep '/Eyelock$' | awk '{print $2}')
				if [[ ! -z ${ELK_PID} ]]
				then
					echo "killing Eyelock, PID: $ELK_PID"
					kill -9 "${ELK_PID}"
				fi

				sleep 1
				/home/root/i2cHandler -r0
				sleep 5
				/home/root/i2cHandler -r1
				sleep 30
				ifdown eth0
				sleep 5
				ifup eth0

				touch "${FLAG}"
				((COUNTER++))

				sleep 10
			else
				echo "too soon to recover, skipping..."
			fi
		else
			echo "${COUNTER} sequential recoveries. Device must be powercycled"
			/home/root/i2cHandler -l 6
			exit 1
		fi
	fi
	sleep "${TIMEOUT}"	
done


