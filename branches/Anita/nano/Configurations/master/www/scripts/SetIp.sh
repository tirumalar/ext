# $1 = ip, $2 = subnet $3 = gateway $4 = dns1 $5 = dns2

#to set the IP address of the nano in rc.conf we must first copy the file to a backup.
#then we must SED it and replace ADDR0 with the ip address we want.
#!/bin/bash

INTERFACES_FILE="/etc/network/interfaces"
NEW_INTERFACES_FILE="/etc/network/interfaces.new"

INTERFACE="enp0s3"
if [ -f $NEW_INTERFACES_FILE ]
then
	echo "file found"
	rm -rf ${NEW_INTERFACES_FILE}
fi

echo  "auto ${INTERFACE}" >> ${NEW_INTERFACES_FILE}
echo  "iface ${INTERFACE} inet static" >> ${NEW_INTERFACES_FILE}
echo  "address $1" >> ${NEW_INTERFACES_FILE}
echo  "gateway $2" >> ${NEW_INTERFACES_FILE}
echo  "netmask $3" >> ${NEW_INTERFACES_FILE}
echo  "network $4" >> ${NEW_INTERFACES_FILE}
echo  "broadcast $5" >> ${NEW_INTERFACES_FILE}

mv "${NEW_INTERFACES_FILE}" "${INTERFACES_FILE}"

