#!/bin/bash

OIM_IP='192.168.4.172'
#INTERVAL='0.3' # seconds

LOG='/home/root/oim_debug.log'
LOGMAXSIZE=1000

while true
do
	NOW=$(date)
	if ping -c1 "${OIM_IP}"
	then
		echo "${NOW} OIM ${OIM_IP} is pingable" >> ${LOG}
	else
		echo "${NOW} OIM ${OIM_IP} is NOT pingable" >> ${LOG}
	fi
	netstat -putan >> ${LOG}	
	FILE_SIZE_KB=$(du -m "${LOG}" | cut -f1)
	if [[ ${FILE_SIZE_KB} -ge ${LOGMAXSIZE} ]]
	then
		mv ${LOG} "${LOG}.1"
	fi
done


