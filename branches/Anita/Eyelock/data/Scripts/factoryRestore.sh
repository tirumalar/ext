#!/bin/bash
echo "start factory restore"
cp /home/root/scripts/factoryRestore.sh /home/backup/.

UNAME=$(uname -r)
UNAME_READONLY_FS='3.0.0-BSP-dm37x-2.4-2Eyelock_NXT_6.0'

if [[ ${UNAME} == ${UNAME_READONLY_FS} ]]
then
	# implementation for devices with read-only filesystem
	export NXT_FACTORY_FW_DIR='/usr/nxt/'

	source "${NXT_FACTORY_FW_DIR}/var.sh"
	source "${NXT_FACTORY_FW_DIR}/util.sh"
	source "${NXT_FACTORY_FW_DIR}/logging.sh"
	source "${NXT_FACTORY_FW_DIR}/checkHw.sh"
	source "${NXT_FACTORY_FW_DIR}/checkFw.sh"
	source "${NXT_FACTORY_FW_DIR}/factoryRestore.sh"

	if checkHw
	then
		logError 'Current FW will be removed'
		removeFw
		installFactoryFw
		log "Rebooting..."
		commitLogs
		rebootDevice
	else
		logError 'There are HW issues preventing restoring factory FW' 
		logError 'Reboot the device or contact Eyelock Support Center'
	fi
	commitLogs

else
	# legacy implementation for devices with non read-only filesystem

	# stop master
	cd /home/root
	./nano_led 1 1 1 255
	sleep 2
	i2cset -y 3 0x2e 4 6
	sleep 2
	rm Eyelock.run
	sleep 2
	killall -KILL Eyelock
	sleep 2 

	# cleanup
	cd /home
	rm EyelockNxt_v*.tar
	rm *.cyacd *.gz *.md5
	rm *txt

	# get factory firmware EyelockNxt_v3.01.646_ICM_v3.1.13.tar
	FWVER=$(ls /home/backup | awk -F'[_]' '{print $2}')
	ICMVER=$(ls /home/backup | awk -F'[_]' '{print $4}')
	ICM=$(echo "${ICMVER%.*}")
	FW_NAME="EyelockNxt_""$FWVER""_ICM_$ICM.tar"
	echo $FW_NAME
	cp /home/backup/$FW_NAME .
	tar -xvf $FW_NAME

	# install on slave
	ping -q -c2 192.168.40.2 > /dev/null
	if [ $? -eq 0 ] 
	then
		# stop slave
		ssh root@192.168.40.2 "cd /home; killall -KILL Eyelock; rm -rf firmware/*"

		echo "Slave is Pingable"
		scp EyelockNxt_"$FWVER"_Slave.tar.gz root@192.168.40.2:/home
		ssh root@192.168.40.2 "cd /home; rm *tar; gunzip EyelockNxt_"$FWVER"_Slave.tar.gz"
		sleep 5
		ssh root@192.168.40.2 "cd /home; tar -xvf EyelockNxt_"$FWVER"_Slave.tar"
		sleep 5
		ssh root@192.168.40.2 "cd /home/root; chmod 755 *"
		sleep 1
	else
		echo "Slave not pingable No point"
	fi

	# install on master
	rm -rf default firmware root user www
	gunzip EyelockNxt_"$FWVER"_Master.tar.gz
		sleep 5
	tar -xvf EyelockNxt_"$FWVER"_Master.tar
		sleep 5
	sleep 2
	chmod 755 /home/root/* /home/root/scripts/*
	sleep 2
	cd /home
	/home/root/icm_communicator -p nanoNxt_ICM_"$ICM".cyacd
	sleep 2
	cp /home/default/test.db /home/root/.
	cp /home/default/keys.db /home/root/.
	cp /home/default/rc.conf /etc/rc.d/rc.conf
	cp /home/backup/id.txt /home/id.txt
	cp /home/backup/MAC.txt /home/MAC.txt
	cp /home/backup/id.txt /home/root/id.txt
	cp /home/backup/MAC.txt /home/root/MAC.txt

	# cleanup
	rm EyelockNxt_v*.tar
	rm *.cyacd *.gz *.md5

	# log
	NOW=$(date +"%Y-%m-%d %T, 000")
	echo "$NOW, UTC > Factory Restore SW Version: $FWVER, ICM Version: $ICM" > /home/root/nxtEvent.log
	NOW=$(date +"%Y-%m-%d, %T 000")
	echo "$NOW, INFO , [Eyelock], - Factory Restore FW Version: $FWVER, ICM Version: $ICM" > /home/root/nxtLog.log

	sleep 5
	reboot
fi

