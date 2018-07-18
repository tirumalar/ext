set -e
# $1 = firmware directory ( e.g. /home/firmware)
# $2 = slave firmware archive name ( e.g. EyelockNxt_v0.00.000_Slave.tar.gz)

2>&1
exec 7>&1
exec > /home/slaveFirmwareUpgradeLog.txt

touch /home/slaveUpdating.txt

echo "backup ini file..."
cp /home/root/Eyelock.ini /home/root/EyelockBak.ini

echo "decrypting..."
/home/root/KeyMgr -d -i $1/$2 -o $1/out.tar.gz && mv $1/out.tar.gz $1/$2

echo "extracting..."
gzip -d < $1/$2 | tar xvf - -C /home

echo "settings merging..."
chmod 777 /home/root/merge.sh
/home/root/merge.sh

rm /home/slaveUpdating.txt
echo "slave upgrade completed."
exec 1>&7 7>&-

echo "success"
