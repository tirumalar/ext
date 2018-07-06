#!/bin/bash
while true;
do
   count=`/usr/bin/lsusb | wc -l`
   if [ $count -gt 2 ]
   then
   	ping -q -c 2 192.168.40.2
   	if [ $? = 1 ]
   	then
   		echo "configuring the USB adapter"
   		ifconfig usb0 192.168.40.1
   	else
   		sleep 60	
   	fi
   else	
   	echo "USB gadget not connected,waiting..."
   	sleep 5
   fi	
done
                                                        
