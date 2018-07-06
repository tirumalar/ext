#!/bin/bash
# ---------------------------------------------------------------------------------------
source "/home/root/var.sh"
# ---------------------------------------------------------------------------------------

identifyDeviceType(){
	
	source "${FW_SCRIPTS_DIR}helpers.sh"
	
	local DEVICETYPE_INI_KEY='Eyelock.HardwareType'
	local DEVICETYPE_INI_VAL=$(getEyelockIniValue "${DEVICETYPE_INI_KEY}")
	
	case "${DEVICETYPE_INI_VAL}" in
		'0' )
		DEVICE_TYPE='NXT'
		;;
		'1' )
		DEVICE_TYPE='EXT'
		;;
		'2' )
		DEVICE_TYPE='HBOX"
		;;	
		'3' )
		DEVICE_TYPE='NXT_6_0'
		;;
		*)
		DEVICE_TYPE='UNDEFINED'
		;;
	esac

	echo "${DEVICE_TYPE}"
}
