#!/bin/bash
# ---------------------------------------------------------------------------------------
source "/home/root/var.sh"
# ---------------------------------------------------------------------------------------

getIniValue(){
	local INIFILE=$1
	local KEY=$2

	if [[ ! -f "${INIFILE}" ]]
	then
		return 1
	fi
		
	awk -F '=' -v key="${KEY}" '$0 !~ /^;.*/ && $1 == key { print $2 }' "${INIFILE}"
}


getEyelockIniValue(){
	local KEY=$1
	
	getIniValue "${FW_ROOT_DIR}Eyelock.ini" "${KEY}"
}
