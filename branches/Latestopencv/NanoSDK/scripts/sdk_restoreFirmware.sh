set -e
# $1 = restore point archive ( e.g. /home/firmware/nano/root_20150101_000000_0.00.000.tgz)

2>&1
exec 7>&1
exec > /home/restoreFirmwareLog.txt

touch /home/restoreSoftware.txt

echo "backup .db..."
mv /home/root/test.db /home/root/test.db.restore
mv /home/root/keys.db /home/root/keys.db.restore

echo "extracting..."
cd /home; tar xf /home/firmware/nano/restorepoints/$1

echo "restoring .db..."
mv /home/root/test.db.restore /home/root/test.db
mv /home/root/keys.db.restore /home/root/keys.db

# SLAVE
echo "performing operations with slave..."
ssh root@192.168.40.2 "cd /home; tar xf /home/firmware/nano/restorepoints/$1"

# ICM
echo "getting ICM filename..."
ICM_FIRMWARE_FILE=`find /home/root -type f -name '*.cyacd' | sort | sed q`

if [ "$ICM_FIRMWARE_FILE" = "" ]
then
	echo "ICM FW file not found"
else
	echo "ICM FW file: $ICM_FIRMWARE_FILE" 
fi

chmod 777 /home/root/icm_communicator
/home/root/icm_communicator -p $ICM_FIRMWARE_FILE

echo "restore completed."
exec 1>&7 7>&-

rm /home/restoreSoftware.txt

echo "success"
