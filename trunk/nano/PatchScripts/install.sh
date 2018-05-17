#!/bin/bash
echo Parameters $1 $2
export scriptdir="$( cd "$(dirname "$(readlink -f $0)")/" ; pwd -P )"
echo Loc $scriptdir

MasterUpdate(){
	echo provide your Master update steps
	echo MasterUpdate;
	echo Patch is working on master >> /etc/patchworking
	$date >> /etc/patchworking 
	cp -r testfolder /home
	echo SUCCESSINSTALLPATCH
}

SlaveUpdate(){
	echo provide your Slave update steps
	echo SlaveUpdate;
	echo Patch is working on slave >> /etc/patchworking
	$date >> /etc/patchworking
	cp -r testfolder /home
	echo SUCCESSINSTALLPATCH
}

MasterRestore(){
	echo provide your Master restore steps
	echo MasterRestore;
}

SlaveRestore(){
	echo provide your Slave restore steps
	echo SlaveRestore;
}


if [ $# -ne 2 ]; then
	echo Number Arguments not proper $#
else
	if [[ $1 = "master" && $2 = "update" ]] ;then
		MasterUpdate
	elif [[ $1 = "master" && $2 = "restore" ]] ;then
		MasterRestore
	elif [[ $1 = "slave" && $2 = "update" ]] ;then
		SlaveUpdate
	elif [[ $1 = "slave" && $2 = "restore" ]] ;then
		SlaveRestore
	else
		echo Not valid parameters
	fi
fi

