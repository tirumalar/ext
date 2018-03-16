echo "start factory reset"

# specify the correct path for file with factory MAC and id for devices with read-only file system
FACTORY_MAC_ID_FILE='/usr/nxt/factory_MAC_and_id.txt'

UNAME=$(uname -r)
UNAME_READONLY_FS='3.0.0-BSP-dm37x-2.4-2Eyelock_NXT_6.0'

cd /home/root
i2cset -y 3 0x2e 4 6
rm Eyelock.run
killall -KILL Eyelock;
sleep 2 
cp /home/default/Eyelock.ini .
#chown eyelock:eyelock Eyelock.ini
cp /home/default/reloadinterfaces.sh .
chmod 755 reloadinterfaces.sh
#chown eyelock:eyelock reloadinterfaces.sh
#required in Master
cp /home/default/test.db .
cp /home/default/keys.db .

if [ -f interfaces ]
then
    rm interfaces
fi
if [ -f interfaces.md5 ]
then
    rm interfaces.md5
fi
if [ -d /home/802.1XCerts ]
then
    rm /home/802.1XCerts/*
fi
if [ -f SDKRegisterIPs.txt ]
then
    rm SDKRegisterIPs.txt
fi

if [[ ${UNAME} == ${UNAME_READONLY_FS} ]]
then
	grep -oa '^[0-9]\{0,10\}' "${FACTORY_MAC_ID_FILE}" | head -n 1 > "/home/id.txt"
	grep -oa '\([0-9ABCDEFabcdef]\{2\}:\)\{5\}[0-9ABCDEFabcdef]\{2\}' "${FACTORY_MAC_ID_FILE}" | head -n 1 > "/home/MAC.txt"
	rm /tmp/etc/rc.conf
	rm /home/shadow # if no /home/shadow, web config and SDK will use /etc/shadow
					# assuming the correct default passwords are specified in /etc/shadow
else
	if [ -f /etc/FactoryHostname ]
	then
		cp /etc/FactoryHostname /etc/hostname
		# why? flash.sh will overwrite it anyway
	fi 
	
	cp /home/backup/id.txt /home/id.txt
	cp /home/backup/MAC.txt /home/MAC.txt
	
	cp /home/default/rc.conf /etc/rc.d/rc.conf	
	echo installer:installer | chpasswd -m
	echo SiteAdmin:SiteAdmin | chpasswd -m
	echo admin:admin | chpasswd -m
fi	

rm wpa_supplicant.log*

rm nxtEvent*.log*
NOW=$(date -u +"%Y-%m-%d %T, %Z")
echo "$NOW > Factory Reset" > nxtEvent.log

rm nxtLog*.log*
NOW=$(date +"%Y-%m-%d, %T.000")
echo "$NOW, INFO , [Eyelock], - Factory Reset" > nxtLog.log

ping -q -c2 192.168.40.2 > /dev/null
if [ $? -eq 0 ] 
then
	echo "Slave is Pingable"
	ssh root@192.168.40.2 cp /home/default/Eyelock.ini /home/root/
else
	echo "Slave not pingable No point"
fi

sleep 1
touch Eyelock.run
reboot

