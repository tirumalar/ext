#!/bin/bash
#
# exit codes:
# 1 - "common" error
# 2 - network connection error
# 3 - checksum mismatch
# TODO?: implement autorestore(), retrieveRestorePoints(), deleteRestorePoint() to be called with specific command line option? to have all the functionality in one place

configureLogger(){
	if [[ -e ${logger} ]]
	then
		chmod 777 ${logger}
		if [[ -n "${loggerDestIp}" && -n "${loggerDestPort}" && -n "${loggerDestSecure}" ]]	
		then
			${logger} -t1 -d${loggerDestIp} -p${loggerDestPort} -s${loggerDestSecure} -S
		else
			echo "TCP callbacks destination not specified"
		fi
	else
		echo "Error: logger not found"
		exit 1
	fi
}

validateCheckSum(){
	targetFile=$1
	# check the tar file consistency
	if [ -f $targetFile ] && [ -f "$targetFile".md5 ]
	then
		MD5A="`md5sum $targetFile | awk '{print $1}'`"
		MD5B="`awk '{print $1}' $targetFile.md5`"
		if [ "$MD5A" == "$MD5B" ] 
		then
			return 0
		else
			return 1
		fi
	else
	  return 2
	fi
}

checkDeviceState(){
# perform all checks to ensure that device can be upgraded

	# is slave pingable?
	ping -q -c2 192.168.40.2 > /dev/null
	if [[ $? -eq 0 ]]
	then
		${logger} -L"Slave is pingable"
	else
		${logger} -L"Error: Slave is not pingable"
		return 1	
	fi

	# is there enough free disk space on both boards?	
	diskSpaceLimit=70 # maximum value of used disk space	
	
	# check for free space in /home
	# 	the command searches for the correct line, prints the value in 5th column, then removes the '%'
	# 1. for read-only FS, /home is mounted on the separate partition, so regex matching for ^/home$ in 6th column:
	#	Filesystem                Size      Used Available Use% Mounted on
	#	/dev/root               160.0M    120.6M     39.4M  75% /
	#	/dev/mtdblock9          781.3M     68.0M    713.2M   9% /home
	# 2. for not read-only FS, /home is mounted on /dev/root. 
	#	Filesystem                Size      Used Available Use% Mounted on
	#	/dev/root               996.5M    607.0M    389.5M  61% /
	
	if [[ ${MASTER_UNAME} == ${UNAME_READONLY_FS} ]]
	then
		masterUsedSpace=$(df -h | awk '{if ($6 ~ /^\/home$/) print $5}' | sed -n "s/\(.*\)%/\1/p")
	else
		# Can be simplified. Leaving command as is to reduce the impact on existing functionality
		masterUsedSpace=$(df -h | grep '/dev/root' | awk '{print $5}' | sed -n "s/\(.*\)%/\1/p")
	fi
	
	if [[ ${masterUsedSpace} -lt ${diskSpaceLimit} ]]
	then
		${logger} -L"Free disk space check passed on Master"
	else
		${logger} -L"Error: not enough free disk space on Master. Remove restore points to free it"
		return 1
	fi

	# logic is the same as for master
	if [[ ${SLAVE_UNAME} == ${UNAME_READONLY_FS} ]]
	then
		slaveUsedSpace=$(ssh root@192.168.40.2 'df -h' | awk '{if ($6 ~ /^\/home$/) print $5}' | sed -n "s/\(.*\)%/\1/p")
	else
		# Can be simplified. Leaving command as is to reduce the impact on existing functionality
		slaveUsedSpace=$(ssh root@192.168.40.2 'df -h' | grep '/dev/root' | awk '{print $5}' | sed -n "s/\(.*\)%/\1/p")
	fi
	if [[ ${slaveUsedSpace} -lt ${diskSpaceLimit} ]]
	then
		${logger} -L"Free disk space check passed on Slave"
	else
		${logger} -L"Error: not enough free disk space on Slave. Remove restore points to free it"
		return 1
	fi

	# TODO: can ICM be programmed with current icm_communicator? 

	return 0
}

repairDevice()
{
	# remove excess restore point files
	# algorithm:
	# create list of restore point files on the slave
	# iterate slave's list
		# if file is not present on the master, remove it on the slave

	# create list of restore point files on the master	
	# iterate master's list
		# if file is not present on the slave, remove it on the master

	# no messages if everything is OK

	slaveRestorePoints=($(ssh root@192.168.40.2 'ls /home/firmware/nano/restorepoints'))
	for i in "${slaveRestorePoints[@]}"
	do
		test -f /home/firmware/nano/restorepoints/$i 
		if [[ $? -ne 0 ]]
		then
			ssh root@192.168.40.2 "rm /home/firmware/nano/restorepoints/$i"
			${logger} -L"no $i on master so deleted on slave"
		fi
	done

	masterRestorePoints=($(ls /home/firmware/nano/restorepoints))
	for i in "${masterRestorePoints[@]}"
	do
		ssh root@192.168.40.2 "test -f /home/firmware/nano/restorepoints/$i"
		if [[ $? -ne 0 ]]
		then
			rm /home/firmware/nano/restorepoints/$i
			${logger} -L"no $i on slave so deleted on master"
		fi
	done
}


killApplication(){
	# disabling watchdog
	# this command is supposed to be safe, so can be repeated with no harm
	# probably, sometimes this command fails, so repeating it
	i2cset -y 3 0x2e 4 6
	sleep 1	
	i2cset -y 3 0x2e 4 6
	sleep 1
	i2cset -y 3 0x2e 4 6

	killall -KILL PushButton

	rm /home/root/Eyelock.run
	ssh root@192.168.40.2 'cd /home/root;rm Eyelock.run'
	sync
	ssh root@192.168.40.2 'sync'
	killall -s SIGKILL Eyelock
	ssh root@192.168.40.2 'killall -s SIGKILL Eyelock'

	# probably, Eyelock app running can block i2c bus and prevent disabling the watchdog, so repeating the command again when the process is killed	
	sleep 5
	i2cset -y 3 0x2e 4 6
	sleep 1	
	i2cset -y 3 0x2e 4 6
}

checkApplicationTermination()
{
	ps ax | grep -q "Eyelock$"
	if [[ $? -eq 0 ]]
	then
		${logger} -L"Warn: Eyelock app was not terminated on Master"
	fi

	ps ax | grep -q "PushButton$"
	if [[ $? -eq 0 ]]
	then
		${logger} -L"Warn: Some of FW processes were not terminated on Master"
	fi

	ssh root@192.168.40.2 'ps ax | grep -q "Eyelock$"'
	if [[ $? -eq 0 ]]
	then
		${logger} -L"Warn: Eyelock app was not terminated on Slave"
	fi
}

createRestorePoint(){
	restorePointsNumber=$(ls /home/firmware/nano/restorepoints | grep -c root_.*tgz)
	${logger} -L"Restore points number: ${restorePointsNumber}"
	if [[ ${restorePointsNumber} -gt 1 ]]
	then
		toBeRemoved=$(ls -tr /home/firmware/nano/restorepoints | head -1)
		if [[ -n ${toBeRemoved} ]]
		then
			${logger} -L"Deleting ${toBeRemoved}"
			ssh root@192.168.40.2 "rm /home/firmware/nano/restorepoints/${toBeRemoved}" 
			rm /home/firmware/nano/restorepoints/${toBeRemoved}
		fi
	fi

	now=$(date -u +"%Y%m%d_%H%M%S")
	version=$(/home/root/Eyelock -v | sed -n "s/^\sVersion(AES)\s\(.*\)\s/\1/p")
	name="root_${now}_${version}.tgz"
	# actually it is not .gz

	${logger} -L"New restore point name: (${name})"
	
	# MASTER
	mkdir -p /home/firmware/nano/restorepoints

	# do we need the second case?
	if [[ -e /home/root/eyestartup ]]
	then
	   cd /home; 
	   if [[ -e /home/eyelock/data ]]
	   then
		   cp -R /home/eyelock/data /home/root/
		   sync;
	   fi
	   tarStatus=$(tar --exclude='./root/test.db' -cf /home/firmware/nano/restorepoints/${name} ./root ./www ./default ./ext_libs ./factory)
	else
	   cd /home;
	   tarStatus=$(tar cf /home/firmware/nano/restorepoints/${name} root)
	fi
	sync
	cd ${firmwareDir} 
	
	if [[ ${tarStatus} -ne 0 ]]
	then
		${logger} -L"Error: archive creation on master failed."
		rm /home/firmware/nano/restorepoints/${name}
		sync
		return 1
	fi
	${logger} -L"Restore point creation on master: done."
	
	# SLAVE

	# removing database file on the slave board to exclude it from the restore point
	# assuming that it is restored from the master after device reboot
	ssh root@192.168.40.2 "rm /home/root/data/PDB10.bin; sync"
	
	ssh root@192.168.40.2 "mkdir -p /home/firmware/nano/restorepoints; cd /home; tar cf /home/firmware/nano/restorepoints/${name} root && sync"
	if [[ $? -ne 0 ]]
	then
		${logger} -L"Error: archive creation on slave failed."
		rm /home/firmware/nano/restorepoints/${name}
		sync
		ssh root@192.168.40.2 "rm /home/firmware/nano/restorepoints/${name};sync"
		return 1
	fi
	${logger} -L"Restore point creation on slave: done."
	
	return 0
}

testMasterFw(){
	chmod 777 /home/upgradeTemp/root/Eyelock
	/home/upgradeTemp/root/Eyelock -v | grep -q "Eyelock"
	# should we check version number?
	if [[ $? -ne 0 ]]
	then 
		${logger} -L"Error: Master: Eyelock app: test run failed"
		return 1
	fi

	chmod 777 /home/upgradeTemp/root/KeyMgr
	/home/upgradeTemp/root/KeyMgr | grep -q "Start Key Management"
	if [[ $? -ne 0 ]]
	then 
		${logger} -L"Error: Master: FW test run failed (1)"
		return 1
	fi

	# swap ext_libs directory which contain libraries for avahi, check avahi restart output, return to original (swap again) in case of error
	mv /home/upgradeTemp/ext_libs /home/upgradeTemp/ext_libs0 && mv /home/ext_libs /home/upgradeTemp/ext_libs && mv /home/upgradeTemp/ext_libs0 /home/ext_libs
	ldconfig
	/etc/init.d/avahi-daemon restart | grep -q "done"
	if [[ $? -ne 0 ]]
	then 
		${logger} -L"Error: Master: FW test run failed (2)"
		mv /home/upgradeTemp/ext_libs /home/upgradeTemp/ext_libs0 && mv /home/ext_libs /home/upgradeTemp/ext_libs && mv /home/upgradeTemp/ext_libs0 /home/ext_libs
		return 1
	fi

	return 0
}

upgradeMaster(){
# $1 - master FW file

	# backup default settings file. It will be used for merging later
	cp /home/default/Eyelock.ini /home/Eyelock.default 
	
	${logger} -L"Master: Decrypting ..."
	/home/root/KeyMgr -d -i ${firmwareDir}/$1 -o ${firmwareDir}/out.tar.gz;
	mv ${firmwareDir}/out.tar.gz ${firmwareDir}/$1
	${logger} -L"Master: Decrypting done."

	${logger} -L"Master: Extracting..."
	touch /home/untarpackage.txt
	if [[ -d /home/upgradeTemp ]]
	then
		rm -r /home/upgradeTemp
	fi
	mkdir -p /home/upgradeTemp/root/rootCert/certs
	tar -xvzf ${firmwareDir}/$1 -C /home/upgradeTemp > /dev/null
	if [[ $? -ne 0 ]]
	then
		${logger} -L"Error: Master: extracting failed."
		return 1
	fi
	${logger} -L"Master: Extracting done."
	rm /home/untarpackage.txt

	if [[ -f /home/upgradeTemp/root/patch.sh ]]
	then
		${logger} -L"Master: Patching..."
		touch /home/runPatch.txt		
		chmod 777 /home/upgradeTemp/root/patch.sh
		/home/upgradeTemp/root/patch.sh
		rm /home/runPatch.txt
		${logger} -L"Master: Patching done."
	else
		${logger} -L"Master: No patch files in this FW"
	fi

	${logger} -L"Master: Backup..."
	# TODO: check
	# .key .crt and id.txt must be in /home/root for Logger needs (otherwise reconfigure)
	mv /home/root/test.db /home/upgradeTemp/root/
	mv /home/root/keys.db /home/upgradeTemp/root/
	cp /home/root/rootCert/certs/nanoNXTDefault.crt /home/upgradeTemp/root/rootCert/certs
	cp /home/root/rootCert/certs/nanoNXTDefault.key /home/upgradeTemp/root/rootCert/certs
	cp /home/root/rootCert/certs/nanoNXTDefault.pem /home/upgradeTemp/root/rootCert/certs
	
	mv /home/root/*.log /home/upgradeTemp/root

	cp /home/root/MAC.txt /home/upgradeTemp/root/MAC.txt
	cp /home/root/id.txt /home/upgradeTemp/root/id.txt

	# copy current settings file. It will be corrected later
	cp /home/root/Eyelock.ini /home/upgradeTemp/root/Eyelock.ini 
	
	cp /home/root/nxtlog.cfg /home/upgradeTemp/root/nxtlog.cfg
	cp /home/root/SDKRegisterIPs.txt /home/upgradeTemp/root/SDKRegisterIPs.txt

	
	
	mv /home/root/interfaces /home/upgradeTemp/root/interfaces
	mv /home/root/interfaces.md5 /home/upgradeTemp/root/interfaces.md5

	mv /home/root/Calibration.ini /home/upgradeTemp/root/Calibration.ini
	
	${logger} -L"Master: Backup done."

	${logger} -L"Master: Testing..."
	testMasterFw
	if [[ $? -ne 0 ]]
	then
		${logger} -L"Error: Master: master FW test failed"
		return 1
	fi
	${logger} -L"Master: Test passed."

	${logger} -L"Master: Applying changes..."
	mv /home/upgradeTemp/root /home/upgradeTemp/root0 && mv /home/root /home/upgradeTemp/root && mv /home/upgradeTemp/root0 /home/root
	mv /home/upgradeTemp/www /home/upgradeTemp/www0 && mv /home/www /home/upgradeTemp/www && mv /home/upgradeTemp/www0 /home/www
	mv /home/upgradeTemp/default /home/upgradeTemp/default0 && mv /home/default /home/upgradeTemp/default && mv /home/upgradeTemp/default0 /home/default
	mv /home/upgradeTemp/factory /home/upgradeTemp/factory0 && mv /home/factory /home/upgradeTemp/factory && mv /home/upgradeTemp/factory0 /home/factory

	# using ";" instead "&&" to proceed even if a file doesn't exist now
	mv /home/upgradeTemp/wpa_supplicant /home/upgradeTemp/wpa_supplicant0; mv /home/wpa_supplicant /home/upgradeTemp/wpa_supplicant; mv /home/upgradeTemp/wpa_supplicant0 /home/wpa_supplicant

	# copy ICM file even if ICM programming is not needed to have it in restore point
	cp ${bobFileName} /home/root

	## copy configuration file for WebConfig
	#cp /home/upgradeTemp/www/lighttpd.conf /home/www/lighttpd.conf

	# taking new WebConfig configuration file and add settings for TLS if enabled
	cat /home/root/Eyelock.ini | grep -iq "^Eyelock.TLSEnable=true" && sed -i -e "$ a ssl.use-sslv2 = \"disable\"" /home/www/lighttpd.conf && sed -i -e "$ a ssl.use-sslv3 = \"disable\"" /home/www/lighttpd.conf

	# copy this script, Logger and Logger settings for restore point functionality
	cp ${firmwareDir}/fwHandler/fwHandler.sh /home/root	
	cp ${logger} /home/root
	cp "${logger}Settings.xml" /home/root

	
	
	chmod 777 /home/root/nano_led
	chmod 777 /home/root/Eyelock
	chmod 777 /home/root/KeyMgr
	chmod 777 /home/root/PushButton
	chmod 777 /home/root/icm_communicator
	chmod 777 /home/root/*.sh
	chmod 777 /home/root/scripts/*.sh
	chmod 777 /home/default/reloadinterfaces.sh
	${logger} -L"Master: Applying done."

	${logger} -L"Master: Settings merging..."
	/home/root/scripts/mergeINI.sh
	${logger} -L"Master: Settings merging done."

	sync

	${logger} -L"Master: Done."

	return 0
}

testSlaveFw(){
	ssh root@192.168.40.2 "chmod 777 /home/upgradeTemp/root/Eyelock"
	ssh root@192.168.40.2 "/home/upgradeTemp/root/Eyelock -v | grep -q 'Eyelock'"
	# should we check version number?
	if [[ $? -ne 0 ]]
	then 
		${logger} -L"Error: Slave: Eyelock app: incorrect output when test run"
		return 1
	fi

	ssh root@192.168.40.2 "chmod 777 /home/upgradeTemp/root/KeyMgr"
	ssh root@192.168.40.2 "/home/upgradeTemp/root/KeyMgr | grep -q 'Start Key Management'"
	if [[ $? -ne 0 ]]
	then 
		${logger} -L"Error: Slave: FW test run failed (1)"
		return 1
	fi

	# TODO: test libs in ext_libs directory on Slave. How?

	return 0
}

upgradeSlave(){
# $1 - slave FW file
	${logger} -L"Slave: Decrypting ..."
	ssh root@192.168.40.2 "chmod 777 /home/root/KeyMgr"
	ssh root@192.168.40.2 "/home/root/KeyMgr -d -i ${firmwareDir}/$1 -o ${firmwareDir}/out.tar.gz;mv ${firmwareDir}/out.tar.gz ${firmwareDir}/$1"
	${logger} -L"Slave: Decrypting done."

	ssh root@192.168.40.2 "if [[ -d /home/upgradeTemp ]]; then rm -r /home/upgradeTemp; fi; mkdir -p /home/upgradeTemp/root/rootCert/certs"

	# slave board has different version of tar which doesn't support -z option
	${logger} -L"Slave: Extracting..."
	ssh root@192.168.40.2 "gzip -d < ${firmwareDir}/$1 | tar xvf - -C /home/upgradeTemp > /dev/null"
	if [[ $? -ne 0 ]]
	then
		${logger} -L"Error: Slave: extracting failed."
		return 1
	fi
	${logger} -L"Slave: Extracting done."

	${logger} -L"Slave: Testing..."
	testSlaveFw
	if [[ $? -ne 0 ]]
	then
		${logger} -L"Error: Slave FW test failed"
		return 1
	fi
	${logger} -L"Slave: Test passed."

	${logger} -L"Slave: Applying changes..."
	ssh root@192.168.40.2 "cp /home/root/Eyelock.ini /home/upgradeTemp/root/EyelockBak.ini"

	ssh root@192.168.40.2 "mv /home/upgradeTemp/root /home/upgradeTemp/root0 && mv /home/root /home/upgradeTemp/root && mv /home/upgradeTemp/root0 /home/root"
	ssh root@192.168.40.2 "mv /home/upgradeTemp/default /home/upgradeTemp/default0 && mv /home/default /home/upgradeTemp/default && mv /home/upgradeTemp/default0 /home/default"
	ssh root@192.168.40.2 "mv /home/upgradeTemp/factory /home/upgradeTemp/factory0 && mv /home/factory /home/upgradeTemp/factory && mv /home/upgradeTemp/factory0 /home/factory"
	ssh root@192.168.40.2 "mv /home/upgradeTemp/ext_libs /home/upgradeTemp/ext_libs0 && mv /home/ext_libs /home/upgradeTemp/ext_libs && mv /home/upgradeTemp/ext_libs0 /home/ext_libs"
	${logger} -L"Slave: Applying done."

	${logger} -L"Slave: Settings merging..."
	ssh root@192.168.40.2 "chmod 777 /home/root/merge.sh && /home/root/merge.sh"
	${logger} -L"Slave: Settings merging done."	

	ssh root@192.168.40.2 "sync"

	${logger} -L"Slave: Done."

	return 0
}

checkIcmVersion(){
	${logger} -L"Current ICM version: (${currentIcmVer})"
	${logger} -L"New ICM version: (${bobVersion})"
	if [[ ${currentIcmVer} == ${bobVersion} ]]
	then
		return 1
	fi
	# 0 if upgrade needed
	return 0
}

programIcm(){
# $1 - icm_communicator utility file
# $2 - <ICM FileName>
# $3 - <Time (in ms)>

	if [[ -e $1 ]]
	then 
		if [[ -e $2 ]]
		then
			${logger} -L"ICM programming with -t $3..."			
			
			# logging to local file added for DEBUG purposes
			icmProgrammingLog=/home/debugIcmProgrammingOut.log
			date -u >> ${icmProgrammingLog}
			echo "$1 -p $2 -t $3" >> ${icmProgrammingLog}
			
			sleep 3
			programmingOut=$($1 -t $3 -p $2)
			sleep 3
			
			echo "returned:" >> ${icmProgrammingLog}
			echo ${programmingOut} >> ${icmProgrammingLog}

			# can we rely on this output? 
			echo ${programmingOut} | grep -q "Successfully"
			if [[ $? -ne 0 ]]
			then
				${logger} -L"Error: ICM programming failed"
				return 2	
			fi
			
			# check version with current icm_communicator
			installedIcmVer=$(/home/root/icm_communicator -v | sed -n "s/^nanoNXT\sICM\sSoftware\sVersion:\s\(.*\)\s/\1/p")
			
			echo "installed: ${installedIcmVer}" >> ${icmProgrammingLog}
			echo "" >> ${icmProgrammingLog}

			sync

			if [[ ${installedIcmVer} == ${bobVersion} ]]
			then
				return 0
			else	
				${logger} -L"Error: ICM test failed"
				return 3
			fi

			return 0
		else
			${logger} -L"Error: ICM file not found"
			return 1
		fi
	else
		${logger} -L"Error: icm_communicator not found"
		return 1
	fi
}

readXml(){
	# no need to save initial IFS value. Tested on NXT and Ubuntu 14.04
	# The following can be used to print IFS:
	# printf %q "$IFS" 
	local IFS=\>
	
    read -d \< ENTITY CONTENT
    local ret=$?
    TAG_NAME=${ENTITY%% *}
    ATTRIBUTES=${ENTITY#* }
    return $ret
}

validateCheckSumArg(){
	TARGETFILE=$1
	TARGETFILEMD5=$2
	BOARD=$3
	TARGETFILEPRINT=$4
	
	if [[ "${BOARD}" != 'Master' && "${BOARD}" != 'Slave' ]]
	then
		${logger} -L"Error: ${TARGETFILEPRINT} checksum validation failed on ${BOARD} (1)"
		return
	fi	
	
	if [[ -z "${TARGETFILEMD5}" ]]
	then
		${logger} -L"Error: ${TARGETFILEPRINT} checksum validation failed on ${BOARD} (2)"
		return
	fi	
	
	if [[ ${BOARD} = 'Master' ]]
	then
		test -f "${TARGETFILE}"
	else
		ssh -n root@192.168.40.2 "cd ${DEVICE_BASE_PATH}; test -f \"${TARGETFILE}\""
	fi
	if [[ $? -ne 0 ]]
	then
		${logger} -L"Error: ${TARGETFILEPRINT} file is missed on ${BOARD}"
		return
	fi
	
	if [[ ${BOARD} = 'Master' ]]
	then
		MD5=$(md5sum "${TARGETFILE}" | awk '{print $1}')
	else
		MD5=$(ssh -n root@192.168.40.2 "cd ${DEVICE_BASE_PATH}; md5sum \"${TARGETFILE}\"" | awk '{print $1}')
	fi
	if [ "$MD5" == "${TARGETFILEMD5}" ] 
	then
		# disabling reporting about success checks to reduce messages quantity
		echo "${TARGETFILEPRINT} check passed on ${BOARD}"
	else
		${logger} -L"Error: ${TARGETFILEPRINT} checksum mismatch on ${BOARD}"
	fi
}

checkFwElement(){
    if [[ $TAG_NAME = "md5" ]]
	then
		eval local $ATTRIBUTES
		if [[ -z $CONTENT || -z $path ]]
		then
			${logger} -L"Error: $printableName checksum validation error (4)"
		else
			if [[ $toBeCheckedOnMaster = "true" ]]
			then
				validateCheckSumArg "$path" $CONTENT "Master" "$printableName"
			fi		
			if [[ $toBeCheckedOnSlave = "true" ]]
			then
				validateCheckSumArg "$path" $CONTENT "Slave" "$printableName"
			fi
		fi

	elif [[ $TAG_NAME = "existence" ]]
	then
		eval local $ATTRIBUTES
		
		if [[ $toBeCheckedOnMaster = "true" ]]
		then
			test -f "${path}" && EXIST='true' || EXIST='false'
			if [[ $EXIST = $CONTENT ]]
			then
				# disabling reporting about success checks to reduce messages quantity
				echo "$printableName test passed on Master"
			else
				${logger} -L"Error: $printableName test failed on Master"
			fi
		fi
		
		if [[ $toBeCheckedOnSlave = "true" ]]
		then		
			# ssh reads from standard input, therefore it eats all remaining lines, so redirecting standard input
			ssh -n root@192.168.40.2 "cd ${DEVICE_BASE_PATH}; test -f \"${path}\"" && EXIST='true' || EXIST='false'
			if [[ $EXIST = $CONTENT ]]
			then
				# disabling reporting about success checks to reduce messages quantity
				echo "$printableName test passed on Slave"
			else
				${logger} -L"Error: $printableName test failed on Slave"
			fi
		fi	
    fi
}

checkFwIntegrity(){
	cp ${firmwareDir}/MultiChannelLoggerSettings.xml ${DEVICE_BASE_PATH}/MultiChannelLoggerSettings.xml
	pushd ${DEVICE_BASE_PATH}
	while readXml; do
		checkFwElement
	done < $1
	popd
	rm ${DEVICE_BASE_PATH}/MultiChannelLoggerSettings.xml
}

cleanup(){
	echo "Cleanup..."
	ssh root@192.168.40.2 "cd ${firmwareDir}; rm *.*"
	cd ${firmwareDir}; rm -r "fwHandler"; rm *.*; rm ${logger}
	# should upgradeTemp be deleted? WebConfig doesn't
	sync
}

cleanupRestore(){
	echo "Cleanup..."
	ssh root@192.168.40.2 "rm -r /home/restoreTemp"
	rm -r /home/restoreTemp
	sync
}

rebootDevice(){
	echo "Rebooting..."

	sync
	ssh root@192.168.40.2 'sync'	

	# enabling watchdog
	i2cset -y 3 0x2e 4 7 
	sleep 1
	i2cset -y 3 0x2e 4 7
	sleep 1
	i2cset -y 3 0x2e 4 7
	sleep 10

	ssh root@192.168.40.2 'reboot'
	sleep 10
	reboot

	# reset command to motherboard if the above command fails
	sleep 5
	i2cset -y 3 0x2e 4 8
}

getXmlTag()
{
	xmlfile=$1
	tag=$2
	value=$(cat $1 | sed -n "s/<${tag}>\(.*\)<\/${tag}>/\1/p")
	echo ${value}
}

changeIniValue()
{
	inifile=$1
	key=$2
	value=$3
	grep -q "^${key}" ${inifile}
	if [[ $? -ne 0 ]]
	then
		sed -i -e "$ a ${key}=${value}" ${inifile}
	else
		sed -i "s/^${key}=.*$/${key}=${value}/" ${inifile}
	fi
}

# performed by WebConfig/SDK themselves:
# *******************************************************************************
#echo "Upgrading..."

### should updateInProgress flag be created before any changes? seems that autorestore will restore old version if error before new restore point creation
##touch /home/updateInProgress.txt 

### is slaveNotUpdatedYet flag necessary? not used in autorestore.sh
## touch /home/slaveNotUpdatedYet.txt 
#
#rm /home/restoreSoftware.txt
#
#mkdir -p /home/firmware # not "from factory"?
#
#mv tempUploadedFile ${firmwareDir}/${tarFileName}
#cd ${firmwareDir};tar xf tarfilename
#
#running this script:
#chmod 777 fwHandler.sh 

# *******************************************************************************

upgrade()
{
	firmwareDir=/home/firmware
	bobVersion=$(getXmlTag ${firmwareDir}/fwHandler/NanoNXTVersionInfo.xml bobversion)
	nanoVersion=$(getXmlTag ${firmwareDir}/fwHandler/NanoNXTVersionInfo.xml nanoversion)
	bobFileName=$(getXmlTag ${firmwareDir}/fwHandler/NanoNXTVersionInfo.xml bobfilename)
	masterFileName=$(getXmlTag ${firmwareDir}/fwHandler/NanoNXTVersionInfo.xml nanofilename)
	slaveFileName=$(echo ${masterFileName} | sed -n "s/Master/Slave/p")

	mv ${firmwareDir}/fwHandler/MultiChannelLogger ${firmwareDir}
	mv ${firmwareDir}/fwHandler/MultiChannelLoggerSettings.xml ${firmwareDir}
	mv ${firmwareDir}/fwHandler/fwIntegrityValidationList.xml ${firmwareDir}
	
	logger=${firmwareDir}/MultiChannelLogger
	loggerDestIp=$1
	loggerDestPort=$2
	loggerDestSecure=$3

	# working directory: /home/firmware
	cd ${firmwareDir}

	configureLogger

	${logger} -L"UPGRADE_RUNNING"
	
	# set global variables to be used in other functions
	UNAME_READONLY_FS='3.0.0-BSP-dm37x-2.4-2Eyelock_NXT_6.0'
	MASTER_UNAME=$(uname -r)
	SLAVE_UNAME=$(ssh root@192.168.40.2 'uname -r')

	${logger} -L"Device testing..."
	checkDeviceState
	checkDeviceStateStatus=$?
	if [[ ${checkDeviceStateStatus} -ne 0 ]]
	then
		${logger} -L"Error: device cannot be upgraded"
		${logger} -L"STATUS:UNSUCCESSFUL"
		cleanup
		exit 1
	fi
	${logger} -L"Device testing completed."
	
	repairDevice

	# must be done before swapping directories
	# change to "icm_communicator -v ..."?
	currentIcmVer=$(cat /home/root/BobVersion | sed -n "s/^ICM\ssoftware\sversion:\s\(.*\)$/\1/p")

	${logger} -L"Upgrade process started."

	if [[ -n "${masterFileName}" && -e ${masterFileName} && -e ${slaveFileName} ]]
	then
		${logger} -L"FW validation..."
		validateCheckSum ${masterFileName}
		if [[ $? -ne 0 ]]
		then
			${logger} -L"Error: master FW file checksum mismatch"
			${logger} -L"STATUS:UNSUCCESSFUL"
			cleanup
			exit 3
		fi
		validateCheckSum ${slaveFileName}
		if [[ $? -ne 0 ]]
		then
			${logger} -L"Error: slave FW file checksum mismatch"
			${logger} -L"STATUS:SLAVE_UNSUCCESSFUL"
			cleanup
			exit 3	
		fi
		${logger} -L"FW validation done."
	
		${logger} -L"Preparing for upgrade..."
		/home/root/nano_led 100 80 0 1
	
		${logger} -L"Transferring slave FW file..."
		# Device state check was performed. Move file transfer to upgradeSlave?
		ssh root@192.168.40.2 "mkdir /home/firmware"
		scp ${slaveFileName} root@192.168.40.2:/home/firmware 
		if [[ $? -ne 0 ]]
		then
			${logger} -L"Error: slave FW file transfer failed"
			${logger} -L"STATUS:SLAVE_CONNECTION_FLOW_ERROR"
			cleanup
			# TODO: change led color to white. /home/root/nano_led args?
			exit 1
		fi
		${logger} -L"Slave FW file transferred."
	
		${logger} -L"Restore point creation..."
		touch /home/createrestorepoint.txt
		createRestorePoint
		createRestorePointStatus=$?
		if [[ ${createRestorePointStatus} -ne 0 ]]
		then
			${logger} -L"Error: restore point creation failed"
			${logger} -L"STATUS:UNSUCCESSFUL"
			cleanup
			# TODO: change led color to white. Or reboot?
			exit 1
		fi
		rm /home/createrestorepoint.txt
		${logger} -L"Restore point creation done."
	
		${logger} -L"Upgrading FW..."
		touch /home/firmwareUpdate.txt
		rm /home/restoreSoftware.txt
		rm /home/nanoupdate.txt

		${logger} -L"Terminating current FW processes..."
		killApplication
		${logger} -L"Terminating done."
		checkApplicationTermination

		${logger} -L"Upgrading master..."
		upgradeMaster ${masterFileName}
		upgradeMasterStatus=$?
		if [[ ${upgradeMasterStatus} -ne 0 ]]
		then
			${logger} -L"Error: master upgrade failed."
			${logger} -L"STATUS:UNSUCCESSFUL"
			${logger} -L"Device will be rebooted."
			sleep 5
			cleanup
			rebootDevice
		fi
		${logger} -L"Master upgrade done."
	
		${logger} -L"Upgrading slave..."
		touch /home/slaveUpdating.txt
		upgradeSlave ${slaveFileName}
		upgradeSlaveStatus=$?
		if [[ ${upgradeSlaveStatus} -ne 0 ]]
		then
			${logger} -L"Error: slave upgrade failed."
			${logger} -L"STATUS:SLAVE_UNSUCCESSFUL"
			${logger} -L"Device will be rebooted."
			cleanup
			rebootDevice
		fi
		rm /home/slaveUpdating.txt
		touch /home/slaveUpdated.txt
		${logger} -L"Slave upgrade done."

		# format is different from WebConfig. Is it OK?
		NOW=$(date -u)
		changeIniValue /home/root/Eyelock.ini "Eyelock.SoftwareUpdateDateNano" "${NOW}"
		date -u > /home/nanoupdate.txt
	
		rm /home/firmwareUpdate.txt

		${logger} -L"FW upgrade done."
	else
		${logger} -L"FW master/slave files not found."
	fi
	
	if [[ -n "${bobFileName}" && -e ${bobFileName} ]]
	then
		checkIcmVersion
		if [[ $? -ne 0 ]]
		then
			${logger} -L"New ICM version matches with current."
		else
			${logger} -L"Upgrading ICM..."
			touch /home/icmupdate.txt
			rm /home/bobupdate.txt
		
			# dependence on existing icm_communicator. It'd be better to take it from the FW distribution
			icmCommunicator="/home/upgradeTemp/root/icm_communicator"
		
			chmod 777 ${icmCommunicator}
		
			programIcm ${icmCommunicator} ${bobFileName} 20 || programIcm ${icmCommunicator} ${bobFileName} 30 || programIcm ${icmCommunicator} ${bobFileName} 40
			upgradeIcmStatus=$?
			if [[ ${upgradeIcmStatus} -ne 0 ]]
			then 
				${logger} -L"Error: ICM upgrade failed."
				${logger} -L"STATUS:UNSUCCESSFUL"
				${logger} -L"Device will be rebooted."
				cleanup
				rebootDevice
			fi
		
			# time format is different from WebConfig. Is it OK?	
			NOW=$(date -u)
			changeIniValue /home/root/Eyelock.ini "Eyelock.SoftwareUpdateDateBob" "${NOW}"
			date -u > /home/bobupdate.txt
		
			rm /home/icmupdate.txt
			${logger} -L"ICM upgrade done."
		fi
	fi

	# adding upgrade event to logs
	NOW=$(date -u +"%Y-%m-%d %T, %Z")
	echo "$NOW > SW Upgrade: AppVer: ${nanoVersion}; ICM FW: ${bobVersion}" >> /home/root/nxtEvent.log
	NOW=$(date +"%Y-%m-%d, %T.000")
	echo "$NOW, INFO , [Eyelock], - SW Upgrade: AppVer: ${nanoVersion}; ICM FW: ${bobVersion}" >> /home/root/nxtLog.log

	DEVICE_BASE_PATH='/home'
	${logger} -L"Checking FW integrity..."
	checkFwIntegrity ${firmwareDir}/fwIntegrityValidationList.xml
	${logger} -L"FW integrity check done"
	
	touch /home/root/Eyelock.run
	ssh root@192.168.40.2 "touch /home/root/Eyelock.run"

	${logger} -L"Upgrade done."
	${logger} -L"STATUS:SUCCESS"
	${logger} -L"Device will be rebooted."

	rm /home/slaveUpdated.txt
	rm /home/updateInProgress.txt

	cleanup

	rebootDevice
}

restore(){
	# working directory: /home/firmware
	firmwareDir=/home/firmware
	cd ${firmwareDir}

	restorePointName=$1
	restorePoint="${firmwareDir}/nano/restorepoints/${restorePointName}"
	
	loggerDestIp=$2
	loggerDestPort=$3
	loggerDestSecure=$4
	# copy Logger to another directory to preserve from overwriting
	cp /home/root/MultiChannelLogger /home/firmware
	cp /home/root/MultiChannelLoggerSettings.xml /home/firmware
	logger=/home/firmware/MultiChannelLogger 
	configureLogger

	test -e ${restorePoint}
	if [[ $? -ne 0 ]]
	then
		${logger} -L"Error: restore point not found: ${restorePoint}."
		${logger} -L"STATUS:UNSUCCESSFUL"
		cleanupRestore
		exit 1		
	else
		${logger} -L"Master: archive exists"
	fi

	ssh root@192.168.40.2 "test -e ${restorePoint}"
	if [[ $? -ne 0 ]]
	then
		${logger} -L"Error: restore point ${restorePoint} incomplete: slave part not found."
		${logger} -L"STATUS:UNSUCCESSFUL"
		cleanupRestore
		exit 1	
	else
		${logger} -L"Slave: archive exists"
	fi

	${logger} -L"RESTORE_RUNNING"
	/home/root/nano_led 100 80 0 1

	${logger} -L"Terminating current FW processes..."
	killApplication
	${logger} -L"Terminating done."
	checkApplicationTermination	

	# SLAVE
	# ===============================================================================
	${logger} -L"Slave: extracting ${restorePoint}"
	ssh root@192.168.40.2 "test -e /home/restoreTemp" && ssh root@192.168.40.2 "rm -r /home/restoreTemp"
	ssh root@192.168.40.2 "mkdir -p /home/restoreTemp/root"
	ssh root@192.168.40.2 "tar -xvf ${restorePoint} -C /home/restoreTemp > /dev/null"
	if [[ $? -ne 0 ]]
	then
		${logger} -L"Error: Slave restore failed."		
		${logger} -L"STATUS:SLAVE_UNSUCCESSFUL"
		cleanupRestore
		return 1
	fi
	${logger} -L"Slave: extracting done."
	# ===============================================================================

	# MASTER
	# ===============================================================================
	if [[ -d /home/restoreTemp ]]
	then
		rm -r /home/restoreTemp
	fi
	mkdir -p /home/restoreTemp/root/rootCert/certs
	${logger} -L"Master: extracting ${restorePoint}"
	tar -xvf ${restorePoint} -C /home/restoreTemp > /dev/null
	if [[ $? -ne 0 ]]
	then
		${logger} -L"Error: Master restore failed."
		${logger} -L"STATUS:UNSUCCESSFUL"
		cleanupRestore
		return 1
	fi
	${logger} -L"Master: extracting done."
	# ===============================================================================

	# ICM
	# ===============================================================================
	bobFileName=$(find /home/restoreTemp/root -type f -name '*.cyacd' | sort | sed q)
	if [[ -z ${bobFileName} ]]
	then
		${logger} -L"Error: ICM file not found."		
		${logger} -L"STATUS:UNSUCCESSFUL"
		cleanupRestore
		exit 1
	fi

	bobVersion=$(echo ${bobFileName} | sed -n "s/\(.*\)v\(.*\)\.cyacd/\2/p")

	${logger} -L"Restoring ICM (${bobFileName})..."

	# using .cyacd and icm_communicator from the restore point
	icmCommunicator="/home/restoreTemp/root/icm_communicator"

	chmod 777 ${icmCommunicator}

	programIcm ${icmCommunicator} ${bobFileName} 20 || programIcm ${icmCommunicator} ${bobFileName} 30 || programIcm ${icmCommunicator} ${bobFileName} 40
	restoreIcmStatus=$?
	if [[ ${restoreIcmStatus} -ne 0 ]]
	then 
		${logger} -L"Error: ICM restore failed."
		${logger} -L"STATUS:UNSUCCESSFUL"
		cleanupRestore
		exit 1
	fi
	${logger} -L"Restoring ICM done."
	# ===============================================================================

	${logger} -L"Master: applying changes..."

	# keep existing database
	mv /home/root/test.db /home/restoreTemp/root/test.db

	mv /home/restoreTemp/root /home/restoreTemp/root0 && mv /home/root /home/restoreTemp/root && mv /home/restoreTemp/root0 /home/root
	mv /home/restoreTemp/www /home/restoreTemp/www0 && mv /home/www /home/restoreTemp/www && mv /home/restoreTemp/www0 /home/www
	mv /home/restoreTemp/default /home/restoreTemp/default0 && mv /home/default /home/restoreTemp/default && mv /home/restoreTemp/default0 /home/default
	mv /home/restoreTemp/ext_libs /home/restoreTemp/ext_libs0 && mv /home/ext_libs /home/restoreTemp/ext_libs && mv /home/restoreTemp/ext_libs0 /home/ext_libs
	mv /home/restoreTemp/factory /home/restoreTemp/factory0 && mv /home/factory /home/restoreTemp/factory && mv /home/restoreTemp/factory0 /home/factory

	chmod 777 /home/root/nano_led
	chmod 777 /home/root/Eyelock
	chmod 777 /home/root/KeyMgr
	chmod 777 /home/root/PushButton
	chmod 777 /home/root/icm_communicator
	chmod 777 /home/root/*.sh
	chmod 777 /home/root/scripts/*.sh
	chmod 777 /home/default/reloadinterfaces.sh
	${logger} -L"Master: applying done."

	${logger} -L"Slave: applying changes..."
	ssh root@192.168.40.2 "mv /home/restoreTemp/root /home/restoreTemp/root0 && mv /home/root /home/restoreTemp/root && mv /home/restoreTemp/root0 /home/root"
	ssh root@192.168.40.2 "mv /home/restoreTemp/default /home/restoreTemp/default0 && mv /home/default /home/restoreTemp/default && mv /home/restoreTemp/default0 /home/default"

	ssh root@192.168.40.2 "chmod 777 /home/root/Eyelock"
	ssh root@192.168.40.2 "chmod 777 /home/root/KeyMgr"
	${logger} -L"Slave: applying done."

	# adding restore event to logs
	NOW=$(date -u +"%Y-%m-%d %T, %Z")
	echo "$NOW > SW Restore: ${restorePointName}" >> /home/root/nxtEvent.log
	NOW=$(date +"%Y-%m-%d, %T.000")
	echo "$NOW, INFO , [Eyelock], - Restore: ${restorePointName}" >> /home/root/nxtLog.log

	sync

	${logger} -L"Restore done."
	${logger} -L"STATUS:SUCCESS"
	${logger} -L"Device will be rebooted."

	rm ${logger}
	rm "${logger}Settings.xml"

	cleanupRestore
	rebootDevice
}


echo "fwHandler is running"

operation=$1

case "${operation}" in

	"upgrade" )
	upgrade $2 $3 $4

	;;

	"restore" )
	restore $2 $3 $4 $5
	
	;;

	"reboot" )
	killApplication
	rebootDevice
	
	;;

esac




