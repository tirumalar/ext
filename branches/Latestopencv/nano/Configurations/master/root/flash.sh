#!/bin/bash
if [ -f /home/xxxxid.txt ]
then
    echo "setting default time to Jan 1st 2015"
    /bin/date -s 2015-01-01-00:00:01
    /sbin/hwclock -w
    if [ -f /home/id.txt ] && [ -f /home/MAC.txt ]
    then
	ln -sf /usr/var/run/dbus /var/run/dbus
	ID=`cat /home/id.txt`
	MAC=`cat /home/MAC.txt`
#	MAC=`cat /sys/class/net/eth0/address`
	INDEX=`grep -e "GRI\.cameraID\=.*" Eyelock.ini | cut -f2 -d'='` 
	if [ -n "$ID" ] && [ -n "$MAC" ]
	then
	    echo "nanonxt${ID}" | tr -d ':' > /etc/hostname
            sed -i "s/\(hwaddress ether\) *\(..:..:..:..:..:..\)/\1 ${MAC}/g" /etc/network/interfaces
	    sed -i "s/>\(.*_.*_.*_\).*_.*</>\1${ID}_${MAC}</g" /etc/avahi/services/udip.service
	    /etc/init.d/hostname.sh restart
            /home/root/reloadinterfaces.sh  
	    /etc/init.d/networking restart
            /etc/init.d/avahi-daemon restart
	fi 
    fi
cp /home/id.txt /home/root/
cp /home/MAC.txt /home/root/
rm -rf /home/id.txt
rm -rf /home/MAC.txt
fi

#if [ -f /hbox/.ssh/id_rsa ]
#then
#	chmod 600 /root/.ssh/id_rsa
#	echo "ssh keys already present"
#else
#	echo "copying the ssh keys"
#	mkdir -p /hbox/.ssh
#	cp /home/root/id_rsa /hbox/.ssh/
#	chmod 600 /hbox/.ssh/id_rsa
#fi
#sed -i '/PasswordAuthentication/d' /etc/ssh/sshd_config; sed -i -e "$ a PasswordAuthentication no" /etc/ssh/sshd_config
#killall -KILL sshd;/usr/sbin/sshd

# running the user commands all the time .. to avoid not resetting the password on older devices

ret=false
getent passwd installer >/dev/null 2>&1 && ret=true
if $ret; then
	echo "www-data users exists"
else
	/usr/sbin/adduser --no-create-home installer
	echo installer:installer | /usr/sbin/chpasswd -m
	/usr/sbin/adduser --no-create-home admin
	echo admin:admin | /usr/sbin/chpasswd -m
	#echo root:GQQE-UEwhB8v}THr9Eyel0ck | /usr/sbin/chpasswd -m
	/usr/sbin/usermod -L admin
	/usr/sbin/usermod -L installer
cp /etc/shadow /home/
chmod 666 /home/shadow
fi
