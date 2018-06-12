#!/bin/bash

OIM_IP='192.168.4.172'
TIMEOUT='5' # seconds

FLAG='/home/root/hbOIM_failure'
RECOVERY_MIN_INTERVAL=30 # seconds
TIME_SINCE_LAST_RECOVERY=30 # initialize to 
COUNTER=0
MAX_SEQUENTIAL_RECOVERIES=3

rm "${FLAG}"

while true
do
	echo "pinging OIM ${OIM_IP}..."
	if ping -c 1 "${OIM_IP}"
	then
		echo "OIM ${OIM_IP} is pingable, sleeping ${TIMEOUT} seconds"
		COUNTER=0		
	else
		echo "OIM ${OIM_IP} is NOT pingable!"

		if [[ ${COUNTER} < ${MAX_SEQUENTIAL_RECOVERIES} ]]
		then

			if [[ -f ${FLAG} ]]
			then
				TIME_SINCE_LAST_RECOVERY=$(($(date +%s) - $(date +%s -r "${FLAG}")))
			fi

			if [[ ${TIME_SINCE_LAST_RECOVERY} -ge ${RECOVERY_MIN_INTERVAL} ]] 
			then
				echo "recovering..."

				ifdown eth0
				sleep 1
				/home/root/i2cHandler -r0
				sleep 5
				/home/root/i2cHandler -r1
				sleep 5
				ifup eth0

				touch "${FLAG}"
				((COUNTER++))
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


