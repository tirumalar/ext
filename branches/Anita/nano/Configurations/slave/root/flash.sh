#!/bin/bash
if [ -f /home/id.txt ]
then
    if [ -f /home/id.txt ] && [ -f /home/MAC.txt ]
    then
	ln -sf /usr/var/run/dbus /var/run/dbus
	ID=`cat /home/id.txt`
	MAC=`cat /home/MAC.txt `
	echo $MAC
	INDEX=`grep -e "GRI\.cameraID\=.*" Eyelock.ini | cut -f2 -d'='` 
	if [ -n "$ID" ] && [ -n "$MAC" ]
	then
	    echo "nanonxt${ID}-${INDEX}" | tr -d ':' > /etc/hostname
            sed -i "s/\(hwaddress ether\) *\(..:..:..:..:..:..\)/\1 ${MAC}/g" /etc/network/interfaces
	    sed -i "s/>\(.*_.*_.*_\).*_.*</>\1${ID}_${MAC}</g" /etc/avahi/services/udip.service
	    /etc/init.d/hostname.sh restart
            /home/root/reloadinterfaces.sh  
	    /etc/init.d/networking restart
#            /etc/init.d/avahi-daemon restart
	fi 
    fi
cp /home/id.txt /home/root/
cp /home/MAC.txt /home/root/
rm -rf /home/id.txt
rm -rf /home/MAC.txt
fi
if [ -f /root/.ssh/authorized_keys ]
then
	chmod 600 /root/.ssh/authorized_keys
	echo "ssh keys already present"
else
	echo "copying the ssh keys"
	mkdir -p /root/.ssh
	cp /home/root/authorized_keys /root/.ssh/
	chmod 600 /root/.ssh/authorized_keys
fi
