#!/bin/bash
# ---------------------------------------------------------------------------------------
source "/home/root/var.sh"
# ---------------------------------------------------------------------------------------

EYELOCK_LOG_FILE="${FW_ROOT_DIR}nxtLog.log"
EYELOCK_EVENTLOG_FILE="${FW_ROOT_DIR}nxtEvent.log"

# device log and eventlog date&time format 
EYELOCK_LOG_TIMESTAMP='$(date +"%Y-%m-%d, %T.000")'
EYELOCK_EVENT_TIMESTAMP='$(date -u +"%Y-%m-%d %T, %Z")'

EYELOCK_LOG_LABEL='SHELL'

# ---------------------------------------------------------------------------------------

log(){
	local MSG="$1"
	[[ -z $2 ]] && local TYPE='INFO' || local TYPE="$2"
	
	eval local NOW="${EYELOCK_LOG_TIMESTAMP}"
	echo "${NOW}, ${TYPE} , [${EYELOCK_LOG_LABEL}], - ${MSG}" >> "${EYELOCK_LOG_FILE}"
}

logError(){
	log "$1" 'ERROR'
}

eventLog(){
	local MSG="$1"
	
	eval local NOW="${EYELOCK_EVENT_TIMESTAMP}"
	echo "${NOW} > ${MSG}" >> "${EYELOCK_EVENTLOG_FILE}"
}

