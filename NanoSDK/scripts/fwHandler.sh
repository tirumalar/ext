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
	# kill OIM heartbeat script
	OIM_HB_PID=$(ps -ef | grep '/hbOIM.sh$' | awk '{print $2}')
    if [[ ! -z ${OIM_HB_PID} ]]
    then
            kill -9 "${OIM_HB_PID}"
    fi

	rm /home/root/Eyelock.run
	rm /home/root/FaceTracker.run
	killall -KILL Eyelock;
	killall -KILL FaceTracker;
}

checkApplicationTermination()
{
	ps ax | grep -q "Eyelock$"
	if [[ $? -eq 0 ]]
	then
		${logger} -L"Warn: Eyelock app was not terminated"
	fi

	ps ax | grep -q "FaceTracker$"
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
	
	cp /home/root/rootCert/certs/*.crt /home/upgradeTemp/root/rootCert/certs
	cp /home/root/rootCert/certs/*.key /home/upgradeTemp/root/rootCert/certs
	
	mv /home/root/*.log /home/upgradeTemp/root

	cp /home/root/MAC.txt /home/upgradeTemp/root/MAC.txt
	cp /home/root/id.txt /home/upgradeTemp/root/id.txt

	# copy current settings file. It will be corrected later
	cp /home/root/Eyelock.ini /home/upgradeTemp/root/Eyelock.ini 

	# preserving FaceTracker config file, because device calibration data is stored there
	#cp /home/root/data/calibration/faceConfig.ini /home/upgradeTemp/root/data/calibration/faceConfig.ini
	cp /home/root/data/calibration/Face.ini /home/upgradeTemp/root/data/calibration/Face.ini
	
	#cp /home/root/nxtlog.cfg /home/upgradeTemp/root/nxtlog.cfg
	cp /home/root/SDKRegisterIPs.txt /home/upgradeTemp/root/SDKRegisterIPs.txt
	
	# fixing log timestamp format
	#sed -i -e 's/log4j.appender.Rlog.layout.ConversionPattern=%d{yyyy-MM-dd}, %d{HH:mm:ss.SSS}, %-5p, [%c], - %m%n/log4j.appender.Rlog.layout.ConversionPattern=%d{yyyy-MM-dd}{UTC}, %d{HH:mm:ss.SSS}{UTC}, %-5p, [%c], - %m%n/g' /home/upgradeTemp/root/nxtlog.cfg
	#sed -i -e 's/log4j.appender.R.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss}, UTC > %m%n/log4j.appender.R.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss}{UTC}, UTC > %m%n/g' /home/upgradeTemp/root/nxtevent.cfg
	
	systemctl disable ntp
		
	mv /home/root/interfaces /home/upgradeTemp/root/interfaces
	mv /home/root/interfaces.md5 /home/upgradeTemp/root/interfaces.md5

	mv /home/root/CalRect.ini /home/upgradeTemp/root/CalRect.ini
	
	${logger} -L"Backup done."

	${logger} -L"Testing..."
	testMasterFw
	if [[ $? -ne 0 ]]
	then
		${logger} -L"Error: FW test failed"
		
		# moving back the original items to keep the existing configuration
		mv /home/upgradeTemp/root/test.db /home/root 
		mv /home/upgradeTemp/root/keys.db /home/root
		mv /home/upgradeTemp/root/*.log /home/root	
		mv /home/upgradeTemp/root/interfaces /home/root/interfaces
		mv /home/upgradeTemp/root/interfaces.md5 /home/root/interfaces.md5
		mv /home/upgradeTemp/root/CalRect.ini /home/root/CalRect.ini
		
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
	
	# TODO: copy for restore?
	#cp ${fpgaFileName} /home/root
	#cp ${fixedBrdFileName} /home/root
	#cp ${camBrdFileName} /home/root
	#cp ${firmwareDir}/fwHandler/NanoEXTVersionInfo.xml /home/root

	## copy configuration file for WebConfig
	#cp /home/upgradeTemp/www/lighttpd.conf /home/www/lighttpd.conf

	# taking new WebConfig configuration file and add settings for TLS if enabled
	#cat /home/root/Eyelock.ini | grep -iq "^Eyelock.TLSEnable=true" && sed -i -e "$ a ssl.use-sslv2 = \"disable\"" /home/www/lighttpd.conf && sed -i -e "$ a ssl.use-sslv3 = \"disable\"" /home/www/lighttpd.conf

	# ssl.use-sslv2 and other are present by default. Commenting them out if TLS is disabled explicitly
	grep -iq "^Eyelock.TLSEnable=false" /home/root/Eyelock.ini && sed -i s/'ssl.use-sslv2'/'#ssl.use-sslv2'/ '/home/root/lighttpd.conf' && sed -i s/'ssl.use-sslv3'/'#ssl.use-sslv3'/ '/home/root/lighttpd.conf'  && sed -i s/'ssl.cipher-list'/'#ssl.cipher-list'/ '/home/root/lighttpd.conf'


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

	mv /home/www/nxtW /home
	chmod +x /home/nxtW

	mv /home/root/OIMftp.sh /home
        chmod +x /home/OIMftp.sh
		
	# changing udev rule if needed
	CUR_CYP_RULE='/etc/udev/rules.d/85-cypress_rule.rules'
	NEW_CYP_RULE='/home/root/85-cypress_rule.rules'
	if ! diff -q "${CUR_CYP_RULE}" "${NEW_CYP_RULE}"
	then 
		mv "${NEW_CYP_RULE}" "${CUR_CYP_RULE}" 
		udevadm control --reload-rules
	#else 
	#	echo 'Cypress connected udev rule is the same, no need to update'
	fi
	rm "${NEW_CYP_RULE}"
	
	# changing usb network adapter systemd rule if needed
	CUR_USBNET_SYSTEMD_RULE='/lib/systemd/system/eyelock-ext-usbnet.service'
	NEW_USBNET_SYSTEMD_RULE='/home/root/eyelock-ext-usbnet.service'
	if ! diff -q "${CUR_USBNET_SYSTEMD_RULE}" "${NEW_USBNET_SYSTEMD_RULE}"
	then 
		mv "${NEW_USBNET_SYSTEMD_RULE}" "${CUR_USBNET_SYSTEMD_RULE}" 
	fi
	rm "${NEW_USBNET_SYSTEMD_RULE}"
	
	# changing dhcp exit hook if needed
	CUR_DHCP_HOOK='/etc/dhcp/dhclient-exit-hooks.d/eyelock_dhcp_exit_hook'
	NEW_DHCP_HOOK='/home/root/eyelock_dhcp_exit_hook'
	if ! diff -q "${CUR_DHCP_HOOK}" "${NEW_DHCP_HOOK}"
	then 
		mv "${NEW_DHCP_HOOK}" "${CUR_DHCP_HOOK}" 
		chmod +x "${CUR_DHCP_HOOK}" 
	else
		rm "${NEW_DHCP_HOOK}"
	fi

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

	if [[ -e $1 ]]
	then 
		if [[ -e $2 ]]
		then
			${logger} -L"PIM programming..."			
			
			# logging to local file added for DEBUG purposes
			icmProgrammingLog=/home/debugIcmProgrammingOut.log
			date -u >> ${icmProgrammingLog}
			echo "$1 -d /dev/ttyACM0 -s 115200 $2" >> ${icmProgrammingLog}
			
			sleep 3
			programmingOut=$($1 -d /dev/ttyACM0 -s 115200 $2)
			sleep 3
			
			echo "returned:" >> ${icmProgrammingLog}
			echo ${programmingOut} >> ${icmProgrammingLog}

			# can we rely on this output? 
			#echo ${programmingOut} | grep -q "Successfully"
			#if [[ $? -ne 0 ]]
			#then
			#	${logger} -L"Error: PIM programming failed"
			#	return 2	
			#fi
			
			# check version with current icm_communicator
			installedIcmVer=$(/home/root/i2cHandler -v | grep 'ICM software version' | cut -d':' -f2 | tr -d ' ')
			
			echo "installed: ${installedIcmVer}" >> ${icmProgrammingLog}
			echo "" >> ${icmProgrammingLog}

			sync

			if [[ ${installedIcmVer} == ${bobVersion} ]]
			then
				return 0
			else	
				${logger} -L"Error: PIM test failed"
				return 3
			fi

			return 0
		else
			${logger} -L"Error: PIM file not found"
			return 1
		fi
	else
		${logger} -L"Error: icm_communicator not found"
		return 1
	fi
}

cleanup(){
	echo "Cleanup..."
	cd ${firmwareDir}; rm -r "fwHandler"; rm *.*; rm ${logger}
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
	
	cd /home/root

	NOW=$(date +"%Y-%m-%d, %T.000")
	echo "$NOW, INFO , [Eyelock], - Rebooting OIM" >> /home/root/nxtLog.log

	# reboot OIM
	./i2cHandler -r0
	sleep 3
	./i2cHandler -r1
	sleep 7

	sync
	
	NOW=$(date +"%Y-%m-%d, %T.000")
	echo "$NOW, INFO , [Eyelock], - Rebooting COTS" >> /home/root/nxtLog.log
	
	reboot
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

oimctl(){
	local cmd=$1
	local quitAfter=$2
	if [[ -z ${quitAfter} ]]
	then
		local quitAfter=3
	fi
	#OIMADDR='192.168.4.172'
	#OIMPORT='30'
	#printf 'set_cam_mode(0x0,0)\n' | nc -q 1 192.168.4.172 30
	printf "${cmd}\n" | nc -q "${quitAfter}" -w 10 192.168.4.172 30
	return $?
}

oimSendFile(){
	local file=$1
	local status=0
	if [[ -f ${file} ]]
	then
		#cat "${file}" | pv -L 25K | nc -q 15 -O 512 192.168.4.172 35
		# TODO: timeout for nc?
		nc -q 15 -O 512 192.168.4.172 35 < "${file}"
		local status=$?
	fi
	return ${status}
}

upgradeOim(){

	# DEBUG
	#echo "New fixed board FW version: ${fixedBrdVer}"
	#echo "New fixed board FW file: ${fixedBrdFileName}"
	
	if ! ping -q -c 5 192.168.4.172
	then
		${logger} -L"Error: failed to ping OIM"
		${logger} -L"STATUS:UNSUCCESSFUL"
		return 1
	fi
	
	# check telnet connection with empty command
	if ! oimctl ''
	then
		${logger} -L"Error: failed to control OIM"
		${logger} -L"STATUS:UNSUCCESSFUL"
		return 1
	fi
		
	# check version and only update if version is different
	# retrieve hex files versions from XML
	allVer=$(oimctl 'ver(0)')
	if [[ -z ${allVer} ]]
	then
		${logger} -L"Error: failed to retrieve OIM chips versions"
		${logger} -L"STATUS:UNSUCCESSFUL"
		return 1
	fi
		
	# TODO: no option for specific version separately?
	# if this text is changed in OIM side, we will loose backward compatibility
	curFpgaVer=$(echo "${allVer}" | grep 'FPGA VERSION' | cut -d':' -f2)
	curFixedBrdVer=$(echo "${allVer}" | grep 'Fixed board Verson' | cut -d':' -f2)
	curCamBrdVer=$(echo "${allVer}" | grep 'Cam Psoc Version' | cut -d':' -f2)
	
	# DEBUG
	#echo
	#echo "curFpgaVer: ${curFpgaVer}"
	#echo "curFixedBrdVer: ${curFixedBrdVer}"
	#echo "curCamBrdVer: ${curCamBrdVer}"
	
	# if at least one of the chips needs to be programmed, then turn off all the cameras
	if [[ ${curFpgaVer} != ${fpgaVer} || ${curFixedBrdVer} != ${fixedBrdVer} || ${curCamBrdVer} != ${camBrdVer} ]]
	then
		${logger} -L"Turning off the cameras for upgrading OIM"
		oimctl 'set_cam_mode(0x0,0)' 
	fi
	
	# which one should go first? 
	# assuming, FPGA, because: 1) most risky: if fails, no need to continue 2) may contain improvements for upgrading PSoCs
	# any chance of loosing backward compatiblity if one fails?
	if [[ ${curFpgaVer} != ${fpgaVer} ]]
	then
		${logger} -L"Programming FPGA"
		oimctl 'data_store_set(4)'
		if ! oimSendFile "${fpgaFileName}"
		then
			# TODO: check hash sum? resend if failed? num attempts?
			${logger} -L"Error: failed to send FPGA file"
			${logger} -L"STATUS:UNSUCCESSFUL"
			return 1
		fi
		# TODO: check hash sum? resend if failed? num attempts?
		
		sleep 2
		# extra "enter" after sending the file
		oimctl ''		
		fpgaProgOut=$(oimctl 'flash_prog')
		# TODO: need to get status. Check output? check version after flashing? retry if failed? num retries?
		#fpgaProgStatus=$(echo "${fpgaProgOut}" | grep ... ?
		NOW=$(date -u)
		changeIniValue /home/root/Eyelock.ini "Eyelock.SoftwareUpdateDateFpga" "${NOW}"		
		${logger} -L"Programming FPGA done"
	fi
	
	# if [[ ${curFixedBrdVer} != ${fixedBrdVer} ]]
	# then
		# ${logger} -L"Programming fixed board"
		# oimctl 'data_store_set(4)'
		# if ! oimSendFile "${fixedBrdFileName}"
		# then
			# # TODO: check hash sum? resend if failed? num attempts?
			# ${logger} -L"Error: failed to send fixed board file"
			# ${logger} -L"STATUS:UNSUCCESSFUL"
			# return 1
		# fi
		# fixedBrdProgOut=$(oimctl 'psoc_prog(1)')
		# printf "\nFixed board prog out: %s" "${fixedBrdProgOut}"
		# # TODO: need to get status. Check output? check version after flashing? retry if failed? num retries?
		# #fixedBrdProgStatus=$(echo "${fixedBrdProgOut}" | grep ... ?
		
		# NOW=$(date -u)
		# changeIniValue /home/root/Eyelock.ini "Eyelock.SoftwareUpdateDateFixedBoard" "${NOW}"		
		# ${logger} -L"Programming fixed board done"
	# fi
	# # TODO: revert back the first chip if this failed?
	
	# if [[ ${curCamBrdVer} != ${camBrdVer} ]]
	# then
		# ${logger} -L"Programming camera board"
		# oimctl 'data_store_set(4)'
		# if ! oimSendFile "${camBrdFileName}"
		# then
			# # TODO: check hash sum? resend if failed? num attempts?
			# ${logger} -L"Error: failed to send camera board file"
			# ${logger} -L"STATUS:UNSUCCESSFUL"
			# return 1
		# fi
		# camBrdProgOut=$(oimctl 'psoc_prog(2)')
		# # TODO: need to get status. Check output? check version after flashing? retry if failed? num retries?
		# #camBrdProgStatus=$(echo "${camBrdProgOut}" | grep ... ?
		# NOW=$(date -u)
		# changeIniValue /home/root/Eyelock.ini "Eyelock.SoftwareUpdateDateCameraBoard" "${NOW}"
		# ${logger} -L"Programming camera board done"
	# fi
	# # TODO: revert back the first and the second chips if this failed?
}

upgrade_partial(){
	firmwareDir=/home/firmware
	
	masterFileName=$(getXmlTag ${firmwareDir}/fwHandler/NanoEXTVersionInfo.xml nanofilename)
		
	mv ${firmwareDir}/fwHandler/MultiChannelLogger ${firmwareDir}
	mv ${firmwareDir}/fwHandler/MultiChannelLoggerSettings.xml ${firmwareDir}
		
	logger=${firmwareDir}/MultiChannelLogger
	loggerDestIp=$1
	loggerDestPort=$2
	loggerDestSecure=$3

	# working directory: /home/firmware
	cd ${firmwareDir}

	configureLogger

	${logger} -L"UPGRADE_RUNNING"
	${logger} -L"Upgrade process started."

	if [[ -n "${masterFileName}" && -e ${masterFileName} ]]
	then
		${logger} -L"Decrypting ..."
		/home/root/KeyMgr -d -i ${firmwareDir}/"${masterFileName}" -o ${firmwareDir}/out.tar.gz > /dev/null
		if [[ $? -ne 0 ]]
		then
			${logger} -L"Error: Master: decrypting failed."
			${logger} -L"STATUS:UNSUCCESSFUL"
			cleanup
			# TODO: change led color to white. Or reboot?
			exit 1
		fi
		mv ${firmwareDir}/out.tar.gz ${firmwareDir}/"${masterFileName}"
		${logger} -L"Decrypting done."

		${logger} -L"Extracting..."
		if [[ -d /home/upgradeTemp ]]
		then
			rm -r /home/upgradeTemp
		fi
		mkdir -p /home/upgradeTemp/
		tar -xvzf ${firmwareDir}/"${masterFileName}" -C /home/upgradeTemp > /dev/null
		if [[ $? -ne 0 ]]
		then
			${logger} -L"Error: extracting failed."
			return 1
		fi
		${logger} -L"Extracting done."
		
		mv /home/upgradeTemp/www/scripts/EventHandlers.js /home/www/scripts/EventHandlers.js 
		mv /home/upgradeTemp/www/scripts/logdownload.php /home/www/scripts/logdownload.php
		mv /home/upgradeTemp/root/timesync.sh /home/root/timesync.sh
		
		${logger} -L"FW upgrade done."
		
		${logger} -L"STATUS:SUCCESS"
		${logger} -L"Device will be rebooted."

		cleanup
		rebootDevice
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
	bobVersion=$(getXmlTag ${firmwareDir}/fwHandler/NanoEXTVersionInfo.xml bobversion)
	nanoVersion=$(getXmlTag ${firmwareDir}/fwHandler/NanoEXTVersionInfo.xml nanoversion)
	bobFileName=$(getXmlTag ${firmwareDir}/fwHandler/NanoEXTVersionInfo.xml bobfilename)
	masterFileName=$(getXmlTag ${firmwareDir}/fwHandler/NanoEXTVersionInfo.xml nanofilename)
	
	fpgaVer=$(getXmlTag ${firmwareDir}/fwHandler/NanoEXTVersionInfo.xml fpgaversion)
	fixedBrdVer=$(getXmlTag ${firmwareDir}/fwHandler/NanoEXTVersionInfo.xml fixedbrdversion)
	camBrdVer=$(getXmlTag ${firmwareDir}/fwHandler/NanoEXTVersionInfo.xml cambrdversion)

	fpgaFileName=$(getXmlTag ${firmwareDir}/fwHandler/NanoEXTVersionInfo.xml fpgafilename)
	fixedBrdFileName=$(getXmlTag ${firmwareDir}/fwHandler/NanoEXTVersionInfo.xml fixedbrdfilename)
	camBrdFileName=$(getXmlTag ${firmwareDir}/fwHandler/NanoEXTVersionInfo.xml cambrdfilename)

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

		printf 'fixed_set_rgb(100,80,0)\n' | nc -q 1 192.168.4.172 50
	
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
	
	# TODO: uncomment ICM upgrade when ICM communicator will be ready
	#if [[ -n "${bobFileName}" && -e ${bobFileName} ]]
	#then
	#	checkIcmVersion
	#	if [[ $? -ne 0 ]]
	#	then
	#		${logger} -L"New PIM version matches with current."
	#	else
	#		${logger} -L"Upgrading PIM..."
	#		touch /home/icmupdate.txt
	#		rm /home/bobupdate.txt
	#	
	#		icmCommunicator="/home/root/icm_communicator"
	#	
	#		chmod 777 ${icmCommunicator}
	#	
	#		programIcm ${icmCommunicator} ${bobFileName}
	#		upgradeIcmStatus=$?
	#		if [[ ${upgradeIcmStatus} -ne 0 ]]
	#		then 
	#			${logger} -L"Error: PIM upgrade failed."
	#			${logger} -L"STATUS:UNSUCCESSFUL"
	#			${logger} -L"Device will be rebooted."
	#			cleanup
	#			rebootDevice
	#			exit 6
	#		fi
	#	
	#		# time format is different from WebConfig. Is it OK?	
	#		NOW=$(date -u)
	#		changeIniValue /home/root/Eyelock.ini "Eyelock.SoftwareUpdateDateBob" "${NOW}"
	#		date -u > /home/bobupdate.txt
	#	
	#		rm /home/icmupdate.txt
	#		${logger} -L"PIM upgrade done."
	#	fi
	#fi
	
	#upgradeOim

	# adding upgrade event to logs
	NOW=$(date -u +"%Y-%m-%d %T, %Z")
	echo "$NOW > SW Upgrade: AppVer: ${nanoVersion}; ICM FW: ${bobVersion}" >> /home/root/nxtEvent.log
	NOW=$(date +"%Y-%m-%d, %T.000")
	echo "$NOW, INFO , [Eyelock], - SW Upgrade: AppVer: ${nanoVersion}; ICM FW: ${bobVersion}" >> /home/root/nxtLog.log

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

	# TODO: uncomment ICM restoring when ICM communicator will be ready	
	# ICM
	# ===============================================================================
	#bobFileName=$(getXmlTag /home/root/NanoEXTVersionInfo.xml bobfilename)

	#bobVersion=$(getXmlTag /home/root/NanoEXTVersionInfo.xml bobversion)
	#
	#${logger} -L"Restoring ICM (${bobFileName})..."
	#
	## using .cyacd and icm_communicator from the restore point
	#icmCommunicator="/home/restoreTemp/root/icm_communicator"
	#
	#chmod 777 ${icmCommunicator}
	#
	#programIcm ${icmCommunicator} ${bobFileName}
	#restoreIcmStatus=$?
	#if [[ ${restoreIcmStatus} -ne 0 ]]
	#then 
	#	${logger} -L"Error: PIM restore failed."
	#	${logger} -L"STATUS:UNSUCCESSFUL"
	#	cleanupRestore
	#	exit 1
	#fi
	#${logger} -L"Restoring PIM done."
	# ===============================================================================
	
	# OIM
	# ===============================================================================
	# fpgaVer=$(getXmlTag /home/root/NanoEXTVersionInfo.xml fpgaversion)
	# fixedBrdVer=$(getXmlTag /home/root/NanoEXTVersionInfo.xml fixedbrdversion)
	# camBrdVer=$(getXmlTag /home/root/NanoEXTVersionInfo.xml cambrdversion)

	# fpgaFileName=$(getXmlTag /home/root/NanoEXTVersionInfo.xml fpgafilename)
	# fixedBrdFileName=$(getXmlTag /home/root/NanoEXTVersionInfo.xml fixedbrdfilename)
	# camBrdFileName=$(getXmlTag /home/root/NanoEXTVersionInfo.xml cambrdfilename)
	
	# upgradeOim	
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




