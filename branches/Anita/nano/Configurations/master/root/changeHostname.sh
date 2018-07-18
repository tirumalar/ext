#!/bin/bash
# ---------------------------------------------------------------------------------------
source "/home/root/var.sh"
# ---------------------------------------------------------------------------------------

# requires root privileges 
# example of usage: 
# source /home/root/changeHostname.sh; changeHostname ubuntu-VirtualBox1

# $1 - new hostname
changeHostname(){

	source "${FW_SCRIPTS_DIR}logging.sh"
	
	log "Changing hostname"

	local NEW_HOSTNAME="$1"
	if [[ -z ${NEW_HOSTNAME} || ${#NEW_HOSTNAME} -ge 63 ]]
	then
		logError "Invalid new hostname specified"
		return 1
	fi
	
	source "${FW_SCRIPTS_DIR}identifyDeviceType.sh"
	identifyDeviceType
	
	log "Device type: ${DEVICE_TYPE}"
		
	if [[ ${DEVICE_TYPE} == 'NXT' || ${DEVICE_TYPE} == 'HBOX' ]]
	then
		HOSTNAME_FILE="/etc/hostname"
		HOSTS_FILE="/etc/hosts"
	elif [[ ${DEVICE_TYPE} == 'NXT_6_0' ]]
	then
		HOSTNAME_FILE="/tmp/etc/hostname"
		HOSTS_FILE="/tmp/etc/hosts"	
	else
		logError "Unknown device type"
		return 2
	fi	
	
	CUR_HOSTNAME=$(cat "${HOSTNAME_FILE}")
	
	#echo "current hostname: ${CUR_HOSTNAME}"
	#echo "new hostname: ${NEW_HOSTNAME}"
	
	awk -v curHn="${CUR_HOSTNAME}" -v newHn="${NEW_HOSTNAME}" '{ if ($2 == curHn) { printf("%s\t%s\n", $1, newHn); } else print; }' "${HOSTS_FILE}" > "${HOSTS_FILE}.tmp"
	mv "${HOSTS_FILE}.tmp" "${HOSTS_FILE}"
	
	if [[ ${DEVICE_TYPE} == 'HBOX' ]]
	then
		hostnamectl set-hostname "${NEW_HOSTNAME}"
		#systemctl restart systemd-logind.service
		service avahi-daemon restart
	elif [[ ${DEVICE_TYPE} == 'NXT' || ${DEVICE_TYPE} == 'NXT_6_0' ]]
		echo "${NEW_HOSTNAME}" > "${HOSTNAME_FILE}"
		/etc/init.d/hostname.sh restart
		/etc/init.d/avahi-daemon restart
	fi
	
	log "Hostname was successfully changed"
	# name of service (avahi) remains unchanged
}
