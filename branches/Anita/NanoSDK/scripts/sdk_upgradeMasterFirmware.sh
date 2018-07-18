set -e
# $1 = firmware directory ( e.g. /home/firmware)
# $2 = master firmware archive name ( e.g. EyelockNxt_v0.00.000_Master.tar.gz)

2>&1
exec 7>&1
exec > /home/masterFirmwareUpgradeLog.txt

echo "decrypting..."
/home/root/KeyMgr -d -i $1/$2 -o $1/out.tar.gz;
mv $1/out.tar.gz $1/$2

echo "creating upgrade temporary directory..."
if [ -d /home/upgradeTemp ] 
then
	rm -r /home/upgradeTemp
fi
mkdir -p /home/upgradeTemp/root/rootCert/certs

echo "backup ini file..."
cp /home/root/Eyelock.ini /home/upgradeTemp/root/Eyelock.ini.upgrade

echo "extracting..."
tar -xvzf $1/$2 -C /home/upgradeTemp 

echo "copying db, crt and log files to upgrade temporary directory..."
cp -f /home/root/test.db /home/upgradeTemp/root/
cp -f /home/root/rootCert/certs/nanoNXTDefault.crt /home/upgradeTemp/root/rootCert/certs/
cp -f /home/root/rootCert/certs/nanoNXTDefault.key /home/upgradeTemp/root/rootCert/certs/
cp -f /home/root/keys.db /home/upgradeTemp/root/
cp -f /home/root/*.log /home/upgradeTemp/root/

mv /home/root/id.txt /home/upgradeTemp/root/id.txt
mv /home/root/MAC.txt /home/upgradeTemp/root/MAC.txt

echo "swapping current and upgrade directories..."
mv /home/upgradeTemp/root /home/upgradeTemp/root0 && mv /home/root /home/upgradeTemp/root && mv /home/upgradeTemp/root0 /home/root
mv /home/upgradeTemp/www /home/upgradeTemp/www0 && mv /home/www /home/upgradeTemp/www && mv /home/upgradeTemp/www0 /home/www
mv /home/upgradeTemp/default /home/upgradeTemp/default0 && mv /home/default /home/upgradeTemp/default && mv /home/upgradeTemp/default0 /home/default
mv /home/upgradeTemp/ext_libs /home/upgradeTemp/ext_libs0 && mv /home/ext_libs /home/upgradeTemp/ext_libs && mv /home/upgradeTemp/ext_libs0 /home/ext_libs
mv /home/upgradeTemp/factory /home/upgradeTemp/factory0 && mv /home/factory /home/upgradeTemp/factory && mv /home/upgradeTemp/factory0 /home/factory

echo "changing permissions..."
chmod 777 /home/root/Eyelock
chmod 777 /home/root/*.sh
chmod 777 /home/root/scripts/*.sh

echo "touch Eyelock.run..."
touch /home/root/Eyelock.run; ssh root@192.168.40.2 'touch /home/root/Eyelock.run'

echo "master upgrade completed."
exec 1>&7 7>&-

echo "success"
