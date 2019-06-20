#!/bin/bash

export EYELOCK_LOGFILE='/home/root/nxtLog.log'
export EYELOCK_LOGCFG='/home/root/nxtlog.cfg'

export EYELOCK_LOGLABEL='EyelockShellScripts' # can be overwritten

convertEyelockLogLevel(){
	LEVEL_STR=$1
	case "$LEVEL_STR" in
	    ALL)
		result=0
		;;
	    TRACE)
		result=0
		;;
	    DEBUG)
		result=1
		;;
	    INFO)
		result=2
		;;
	    WARN)
		result=3
		;;
	    ERROR)
		result=4
		;;
	    FATAL)
		result=5
		;;
	    NONE)
		result=6
		;;
	esac
	
	echo $result
#	return $result
}

EyelockLog(){
	local MSG=$1
	local LEVELSTR=$2

	if [[ -z ${LEVELSTR} ]]
	then
		LEVELSTR='INFO'
	fi
	
	local LEVEL=$(convertEyelockLogLevel "${LEVELSTR}")
	if (( ${LEVEL} >= ${EYELOCK_LOGLEVEL} ))
	then
		NOW=$(date +"%Y-%m-%d, %T.000")
		echo "$NOW, ${LEVELSTR} , [${EYELOCK_LOGLABEL}], - $MSG" >> "${EYELOCK_LOGFILE}"
	fi
}


if [[ -f ${EYELOCK_LOGCFG} ]]
then
	EYELOCK_LOGLEVELSTR=$(grep 'log4j.category.nxtlog' "${EYELOCK_LOGCFG}" | head -n1 | cut -d'=' -f2 | cut -d',' -f1)
else
	EYELOCK_LOGLEVELSTR='INFO' # default
fi

export EYELOCK_LOGLEVEL=$(convertEyelockLogLevel "${EYELOCK_LOGLEVELSTR}")

