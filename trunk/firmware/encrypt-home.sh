#!/bin/bash

# Parse device (default is /dev/mmcblk0p3)
if [ $# -ge 1 ]; then
	printf -v device "/dev/%s" "$1"
else
	device=/dev/mmcblk0p3
fi

if [ ! -e $device ]; then
	printf "Cannot find device under %s\n" "$device" >&2
	exit 1
fi

# Parse board ID (default is taken from /sys/class/efuse/usid)
if [ $# -ge 2 ]; then
	passphrase=$2
elif [ ! -e /sys/class/efuse/usid ]; then
	printf "Cannot get board ID from /sys/class/efuse/usid\n" >&2
	exit 1
else
	passphrase=$(</sys/class/efuse/usid)
fi

# Format new partition for encryption
if ! echo "$passphrase"|cryptsetup --batch-mode luksFormat $device; then
	printf "Cannot format partition in LUKS mode\n" >&2
	exit 1
fi

# Open encrypted partition in LUKS mode
if ! echo "$passphrase"|cryptsetup luksOpen $device cryptohome; then
	printf "Cannot open partition in LUKS mode\n" >&2
	exit 1
fi

# Create ext4 file system on mapper device
if ! mkfs -t ext4 /dev/mapper/cryptohome; then
	printf "Cannot create file system on mapper device\n" >&2
	exit 1
fi

# Ensure /media/home exists
if [ ! -d /media/home ]; then
	if ! mkdir /media/home
	then
		printf "Cannot create directory /media/home\n" >&2
		exit 1
	fi
fi

# Mount mapper device to /media/home
if ! mount -t ext4 /dev/mapper/cryptohome /media/home; then
	printf "Cannot mount /dev/mapper/cryptohome to /media/home\n" >&2
	exit 1
fi

# Replace the existing MAC in interfaces file with the correct one,
# replace the existing ID file in /home/root,
# create factory hostname file and set factory hostname
MAC_FILE='/media/boot/MAC.txt'
ID_FILE='/media/boot/id.txt'
if [[ -f ${MAC_FILE} ]]
then
	CUR_MAC=$(cat ${MAC_FILE} | tr -d '[:space:]')
	INTERFACES_FILE='/home/www-internal/interfaces'
	sed -i "s/hwaddress ether .*$/hwaddress ether ${CUR_MAC}/" "${INTERFACES_FILE}.default"
	cp "${INTERFACES_FILE}"{.default,}
	rm "${MAC_FILE}"
	
fi
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

# Copy /home to encrypted volume
if ! cp -Rp /home/* /media/home; then
	printf "Cannot copy content of /home to encrypted volume\n" >&2
	exit 1
fi

# Create systemd units for proper auto-mounting encrypted partition
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
if ! cp -f -t /etc/systemd/system $DIR/{home.mount,home.automount,systemd-cryptsetup@cryptohome.service}; then
	printf "Cannot copy systemd units to /etc/systemd/system\n" >&2
	exit 1
fi

# Enable home.automount systemd unit
if ! systemctl enable home.automount; then
	printf "Cannot enable home.automount systemd unit\n" >&2
	exit 1
fi

if ! service network-manager stop; then
	printf "Cannot stop network-manager service\n" >&2
	exit 1
fi

# Move plain /home folder to a backup location /home_plain
if ! mv /home /home_plain ; then
	printf "Cannot move /home folder\n" >&2
	exit 1
fi

# Create /home mount point
if ! mkdir /home ; then
	printf "Cannot create /home mount point\n" >&2
	exit 1
fi

# Disable systemd unit calling this script
if ! systemctl disable eyelock-encrypt-home; then
	printf "Cannot disable eyelock-encrypt-home systemd unit\n" >&2
	exit 1
fi

# Go to /
if ! cd /; then
	printf "Cannot change directory to /\n" >&2
	exit 1
fi

# Remove directory with this script and all of its content
if ! rm -rf $DIR; then
	printf "Cannot remove directory with the script\n" >&2
	exit 1
fi

# Reboot
if ! reboot; then
	printf "Cannot reboot the device\n" >&2
	exit 1
fi
