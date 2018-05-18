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
	${logger} -L"Checking the device state..."
	# TODO. Do we need anything?
}


killApplication(){
	killall -KILL PushButton
	rm /home/root/Eyelock.run
	sync
	killall -s SIGKILL Eyelock
}

checkApplicationTermination()
{
	ps ax | grep -q "Eyelock$"
	if [[ $? -eq 0 ]]
	then
		${logger} -L"Warn: Eyelock app was not terminated"
	fi

	ps ax | grep -q "PushButton$"
	if [[ $? -eq 0 ]]
	then
		${logger} -L"Warn: Some of FW processes were not terminated on Master"
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
			rm /home/firmware/nano/restorepoints/${toBeRemoved}
		fi
	fi

	now=$(date -u +"%Y%m%d_%H%M%S")
	version=$(/home/root/Eyelock -v | sed -n "s/^\sVersion(AES)\s\(.*\)\s/\1/p")
	name="root_${now}_${version}.tgz"
	# actually it is not .gz

	${logger} -L"New restore point name: (${name})"
	
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
	
	return 0
}

testMasterFw(){
	chmod 777 /home/upgradeTemp/root/Eyelock
	/home/upgradeTemp/root/Eyelock -v | grep -q "Eyelock"
	# should we check version number?
	if [[ $? -ne 0 ]]
	then 
		${logger} -L"Error: Eyelock app: test run failed"
		return 1
	fi

	chmod 777 /home/upgradeTemp/root/KeyMgr
	/home/upgradeTemp/root/KeyMgr | grep -q "Start Key Management"
	if [[ $? -ne 0 ]]
	then 
		${logger} -L"Error: FW test run failed (1)"
		return 1
	fi

	return 0
}

upgradeMaster(){
# $1 - master FW file

	# backup default settings file. It will be used for merging later
	cp /home/default/Eyelock.ini /home/Eyelock.default 
	
	${logger} -L"Decrypting ..."
	/home/root/KeyMgr -d -i ${firmwareDir}/$1 -o ${firmwareDir}/out.tar.gz > /dev/null
	if [[ $? -ne 0 ]]
	then
		${logger} -L"Error: Master: decrypting failed."
		${logger} -L"STATUS:UNSUCCESSFUL"
		cleanup
		# TODO: change led color to white. Or reboot?
		exit 1
	fi
	mv ${firmwareDir}/out.tar.gz ${firmwareDir}/$1
	${logger} -L"Decrypting done."

	${logger} -L"Extracting..."
	touch /home/untarpackage.txt
	if [[ -d /home/upgradeTemp ]]
	then
		rm -r /home/upgradeTemp
	fi
	mkdir -p /home/upgradeTemp/root/rootCert/certs
	tar -xvzf ${firmwareDir}/$1 -C /home/upgradeTemp > /dev/null
	if [[ $? -ne 0 ]]
	then
		${logger} -L"Error: extracting failed."
		return 1
	fi
	${logger} -L"Extracting done."
	rm /home/untarpackage.txt

	if [[ -f /home/upgradeTemp/root/patch.sh ]]
	then
		${logger} -L"Patching..."
		touch /home/runPatch.txt		
		chmod 777 /home/upgradeTemp/root/patch.sh
		/home/upgradeTemp/root/patch.sh
		rm /home/runPatch.txt
		${logger} -L"Patching done."
	else
		${logger} -L"No patch files in this FW"
	fi

	${logger} -L"Backup..."
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
	
	${logger} -L"Backup done."

	${logger} -L"Testing..."
	testMasterFw
	if [[ $? -ne 0 ]]
	then
		${logger} -L"Error: FW test failed"
		return 1
	fi
	${logger} -L"Test passed."

	${logger} -L"Applying changes..."
	mv /home/upgradeTemp/root /home/upgradeTemp/root0 && mv /home/root /home/upgradeTemp/root && mv /home/upgradeTemp/root0 /home/root
	mv /home/upgradeTemp/www /home/upgradeTemp/www0 && mv /home/www /home/upgradeTemp/www && mv /home/upgradeTemp/www0 /home/www
	mv /home/upgradeTemp/default /home/upgradeTemp/default0 && mv /home/default /home/upgradeTemp/default && mv /home/upgradeTemp/default0 /home/default
	mv /home/upgradeTemp/factory /home/upgradeTemp/factory0 && mv /home/factory /home/upgradeTemp/factory && mv /home/upgradeTemp/factory0 /home/factory

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
	${logger} -L"Applying done."

	${logger} -L"Settings merging..."
	/home/root/scripts/mergeINI.sh
	${logger} -L"Settings merging done."

	sync

	${logger} -L"Done."

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

cleanup(){
	echo "Cleanup..."
	#cd ${firmwareDir}; rm -r "fwHandler"; rm *.*; rm ${logger}
	# should upgradeTemp be deleted? WebConfig doesn't
	sync
}

cleanupRestore(){
	echo "Cleanup..."
	rm -r /home/restoreTemp
	sync
}

rebootDevice(){
	echo "Rebooting..."

	sync
	sleep 10
	 
	 # is it OK?
	 /home/root/startup.sh &
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
	bobVersion=$(getXmlTag ${firmwareDir}/fwHandler/HBoxVersionInfo.xml bobversion)
	nanoVersion=$(getXmlTag ${firmwareDir}/fwHandler/HBoxVersionInfo.xml nanoversion)
	bobFileName=$(getXmlTag ${firmwareDir}/fwHandler/HBoxVersionInfo.xml bobfilename)
	masterFileName=$(getXmlTag ${firmwareDir}/fwHandler/HBoxVersionInfo.xml nanofilename)

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
	
	# must be done before swapping directories
	# change to "icm_communicator -v ..."?
	currentIcmVer=$(cat /home/root/BobVersion | sed -n "s/^ICM\ssoftware\sversion:\s\(.*\)$/\1/p")

	${logger} -L"Upgrade process started."

	if [[ -n "${masterFileName}" && -e ${masterFileName} ]]
	then
		${logger} -L"FW validation..."
		validateCheckSum ${masterFileName}
		if [[ $? -ne 0 ]]
		then
			${logger} -L"Error: FW file checksum mismatch"
			${logger} -L"STATUS:UNSUCCESSFUL"
			cleanup
			exit 3
		fi

		${logger} -L"FW validation done."
	
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
			exit 5
		fi
		${logger} -L"Master upgrade done."
	
		# format is different from WebConfig. Is it OK?
		NOW=$(date -u)
		changeIniValue /home/root/Eyelock.ini "Eyelock.SoftwareUpdateDateNano" "${NOW}"
		date -u > /home/nanoupdate.txt
	
		rm /home/firmwareUpdate.txt

		${logger} -L"FW upgrade done."
	else
		${logger} -L"FW files not found."
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
		
			icmCommunicator="/home/root/icm_communicator"
		
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
				exit 6
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

	touch /home/root/Eyelock.run

	${logger} -L"Upgrade done."
	${logger} -L"STATUS:SUCCESS"
	${logger} -L"Device will be rebooted."

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
		${logger} -L"Archive exists"
	fi

	${logger} -L"RESTORE_RUNNING"

	${logger} -L"Terminating current FW processes..."
	killApplication
	${logger} -L"Terminating done."
	checkApplicationTermination	

	# ===============================================================================
	if [[ -d /home/restoreTemp ]]
	then
		rm -r /home/restoreTemp
	fi
	mkdir -p /home/restoreTemp/root/rootCert/certs
	${logger} -L"Extracting ${restorePoint}"
	tar -xvf ${restorePoint} -C /home/restoreTemp > /dev/null
	if [[ $? -ne 0 ]]
	then
		${logger} -L"Error: restore failed."
		${logger} -L"STATUS:UNSUCCESSFUL"
		cleanupRestore
		return 1
	fi
	${logger} -L"Extracting done."
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
	${logger} -L"Applying done."

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




