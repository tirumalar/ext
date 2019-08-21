#!/bin/bash

OIM_IP='192.168.4.172'
TIMEOUT='5' # seconds

EYELOCK_RUN_FILE='/home/root/Eyelock.run'

cd /home/root/

rm "${EYELOCK_RUN_FILE}"

source '/home/root/EyelockLog.sh'
EYELOCK_LOGLABEL='HBOIM'

while true
do
	EyelockLog "pinging OIM ${OIM_IP}..." 'TRACE'
	if ping -c 5 "${OIM_IP}"
	then
		EyelockLog "OIM ${OIM_IP} is pingable, creating Eyelock run flag and exiting" 'TRACE'
		touch "${EYELOCK_RUN_FILE}"
		break
	else
		EyelockLog "OIM ${OIM_IP} is still NOT pingable, waiting ${TIMEOUT} seconds" 'TRACE'
	fi
	sleep "${TIMEOUT}"	
done

exit 0
