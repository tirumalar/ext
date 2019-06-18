#!/bin/bash

MAC_FILE='/home/MAC.txt'
ID_FILE='/home/id.txt'

# replace the existing MAC in interfaces file with the correct one
if [[ -f ${MAC_FILE} ]]
then
	CUR_MAC=$(cat ${MAC_FILE} | tr -d '[:space:]')
	INTERFACES_FILE='/home/www-internal/interfaces'
	sed -i "s/hwaddress ether .*$/hwaddress ether ${CUR_MAC}/" "${INTERFACES_FILE}.default"
	cp "${INTERFACES_FILE}"{.default,}
	rm "${MAC_FILE}"
	
fi

# replace the existing ID file in /home/root
# create factory hostname file and set factory hostname
if [[ -f ${ID_FILE} ]]
then
	ID_FILE_DEST='/home/root/id.txt'
	mv "${ID_FILE}" "${ID_FILE_DEST}"
	FACTORY_HOSTNAME="nanoext$(cat ${ID_FILE_DEST} | tr -d '[:space:]')" 
	echo "${FACTORY_HOSTNAME}" > '/home/www-internal/FactoryHostname'	

	hostnamectl set-hostname "${FACTORY_HOSTNAME}"
	awk -v factoryHostname="${FACTORY_HOSTNAME}" ' BEGIN { OFS = "\t" } ($1 == "127.0.1.1") { $2=factoryHostname; } { print } ' '/etc/hosts' > '/home/www-internal/hosts'
	mv '/home/www-internal/hosts' '/etc/hosts'
fi


