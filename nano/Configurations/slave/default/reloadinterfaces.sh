#!/bin/bash

INTERFACES=/etc/network/interfaces

# If the network interface isn't up we have to reboot
ifconfig eth0 
if [ $? != 0 ] 
then
fi

#If default IP address aren't configured, we configure them 
grep 169.254.1 /etc/avahi/avahi-autoipd.action
if [ $? != 0 ]
then
    grep "\-0" /etc/hostname
	if [ $? != 0 ]	
	then
		sed -i -e 's/"$3"/169.254.1.2/g' /etc/avahi/avahi-autoipd.action
	else
		sed -i -e 's/"$3"/169.254.1.1/g' /etc/avahi/avahi-autoipd.action
	fi
fi


# Reconfigure the network using our local preferences
if [ -f /home/root/interfaces ] && [ -f /home/root/interfaces.md5 ]
then
    INTERFACES=/home/root/interfaces
    MD5A=`md5sum /home/root/interfaces | awk '{print $1}'`
    MD5B=`awk '{print $1}' /home/root/interfaces.md5`
    if [ "$MD5A" = "$MD5B" ]
    then
	ifdown -f eth0
	ifup -a -i ${INTERFACES}
    fi
fi

# Reconfigure the hostname and avahi broadcasting
/etc/init.d/hostname.sh restart
/etc/init.d/avahi-daemon restart

# If interfaces file exists and we don't have an IP address then reboot
if [ -f ${INTERFACES} ]
then

    # If we are using DHCP and don't have an IP, then try to renew lease via pump
    grep -e ".* eth0 .* dhcp" ${INTERFACES}
    if [ $? == 0 ]
    then
	for ((i=0; i<2; i++))
	do
	    ifconfig eth0 | grep -e "inet addr"
	    if [ "$?" != 0 ]
	    then
		udhcpc -i eth0 -b -T 10 -A 5 -S
		sleep 5
	    else
		break
	    fi
	done
    fi
	
    # HACK!!! Network is comin up in between two consecutive calls to dnsdomainname -i
    for ((i=0; i<10; i++)); do
	ifconfig eth0 | grep -e "inet addr"
	status=$?
	if [ $status == 0 ]; then
	    break
	fi
	sleep 1
    done
fi
