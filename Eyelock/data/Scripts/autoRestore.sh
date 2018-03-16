#!/bin/bash
echo "run autoRestore.sh ..."
if [ -f /home/icmupdate.txt ]
then
    touch /home/updateInProgress.txt
fi
if [ -f /home/firmwareUpdate.txt ]
then
    touch /home/updateInProgress.txt
fi
if [ -f /home/untarpackage.txt ]
then
    touch /home/updateInProgress.txt
fi
if [ -f /home/untarpackage.txt ]
then
    touch /home/updateInProgress.txt
fi
if [[ -f /home/slaveUpdating.txt || -f /home/slaveUpdated.txt ]]
then
    touch /home/updateInProgress.txt
fi

if [ ! -f /home/updateInProgress.txt ]
then
    exit
fi


echo "Software upgrade failed ..."

if [ -f /home/createrestorepoint.txt ]
then
    echo "create restorepoint failed ..."
    rm /home/createrestorepoint.txt /home/updateInProgress.txt
    exit
fi

echo "now restoring the previous release ..."
cd /home/firmware/nano/restorepoints
filename=$(ls -t * | head -1)
echo "start backup restore - $filename"

# stop master
cd /home/root
i2cset -y 3 0x2e 4 6
rm Eyelock.run
killall -KILL Eyelock
sleep 2 

# install on master
#rm -rf default firmware root user www
#cd /home
#mv /home/root /home/upgradeTemp/root_old
#mv /home/upgradeTemp/root /home/root
#mv /home/upgradeTemp/root_old /home/upgradeTemp/root

#mv /home/www /home/upgradeTemp/www_old
#mv /home/upgradeTemp/www /home/www
#mv /home/upgradeTemp/www_old /home/upgradeTemp/www
#sleep 2

# install on master
#rm -rf default firmware root user www
cd /home
cp /home/firmware/nano/restorepoints/$filename .
sleep 3
tar -xvf $filename
sleep 3
chmod 755 /home/root/* /home/root/scripts/*
sleep 2

# install on slave
if [[ -f /home/slaveUpdating.txt || -f /home/slaveUpdated.txt ]]
then
    ping -q -c2 192.168.40.2 > /dev/null
    if [ $? -eq 0 ] 
    then
        echo "Slave is Pingable"
        # stop slave
        ssh root@192.168.40.2 "cd /home/root; killall -KILL Eyelock"
        ssh root@192.168.40.2 "cd /home; rm *.tar *.tgz; cp /home/firmware/nano/restorepoints/$filename ."
	#scp EyelockNxt_*_Slave.tar.gz root@192.168.40.2:/home
	#ssh root@192.168.40.2 "cd /home; rm *tar; gunzip EyelockNxt_*_Slave.tar.gz"
	#sleep 5
	#ssh root@192.168.40.2 "cd /home; tar -xvf EyelockNxt_*_Slave.tar"
	sleep 5
        ssh root@192.168.40.2 "cd /home; tar -xvf $filename"
        sleep 5
	ssh root@192.168.40.2 "cd /home/root; chmod 755 *; chmod 755 /home/root/scripts/*"
	sleep 1

	rm /home/slaveUpdating.txt /home/slaveUpdated.txt 
    else
	echo "Slave not pingable No point"
    fi
fi

if [ -f /home/icmupdate.txt ]
then
    echo "Restore ICM ..."
    cd /home/root
    ./icm_communicator -p nanoNxt_ICM_*.cyacd
    rm /home/icmupdate.txt 
fi
sleep 5

# cleanup
rm /home/root_*.tgz
rm /home/createrestorepoint.txt /home/updateInProgress.txt
rm /home/firmwareUpdate.txt /home/untarpackage.txt 

# log
touch /home/restoreSoftware.txt
NOW=$(date +"%Y-%m-%d, %T, 000")
echo "$NOW, INFO , [Eyelock], - SW Restore: $filename" > /home/root/nxtEvent.log
echo "$NOW, INFO , [Eyelock], - SW Restore: $filename" > /home/root/nxtLog.log

sleep 25
sync
sleep 5
i2cset -y 3 0x2e 4 7
sleep 5
i2cset -y 3 0x2e 4 8
reboot

