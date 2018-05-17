#!/bin/bash

EYELOCK_RUN_FNAME="Eyelock.run"
HOME_PATH="/home"
ROOT_PATH=$HOME_PATH"/root"
EYELOCK_PATH=$HOME_PATH"/eyelock"

rm $ROOT_PATH/$EYELOCK_RUN_FNAME > /dev/null 2>&1
rm $EYELOCK_PATH/$EYELOCK_RUN_FNAME > /dev/null 2>&1
rm $HOME_PATH/$EYELOCK_RUN_FNAME > /dev/null 2>&1

appInfiPID=`ps -aef | grep "bash \-c" | grep "Eyelock" | awk '{print $2}'`
if [ "$appInfiPID" == "" ] ; then
exit
fi

kill -9 $appInfiPID

appOnlyName=`ps -aef | grep "/home/eyelock/Eyelock" | head -1 | awk '{print $8}'`
if [ "$appOnlyName" == "/home/eyelock/Eyelock" ]; then
	appOnlyUID=`ps -aef | grep $appOnlyName | head -1 | awk '{print $1}'`

	appOnlyPID=`ps -aef | grep $appOnlyName | head -1 | awk '{print $2}'`

	kill -9 $appOnlyPID
elif [ "$appOnlyName" == "./Eyelock" ] ; then
	kill -9 `ps -aef | grep "Eyelock" | head -1 | awk '{print $2}'`
fi

