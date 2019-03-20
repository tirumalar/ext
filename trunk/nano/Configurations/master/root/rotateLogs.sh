#!/bin/bash
#
# USAGE:
# ./rotateLogs.sh <log_file_path> <log_max_size_KB> 
# ./rotateLogs.sh app.log 1000
#
# Continuous check:
# bash -c "while true; do ./rotateLogs.sh app.log 1000; sleep 10; done" &

logfile=$1
logmaxsize=$2

echo "rotating ${logfile}, max size: ${logmaxsize}"

if [[ -f ${logfile} ]]
then
	file_size_kb=`du -k "${logfile}" | cut -f1`
	echo "${logfile} size: ${file_size_kb}"
	if [[ ${file_size_kb} -ge ${logmaxsize} ]]
	then
		mv ${logfile} "${logfile}.1"
	fi
	exit 0
else
	echo "log file not found"
	exit 1
fi

