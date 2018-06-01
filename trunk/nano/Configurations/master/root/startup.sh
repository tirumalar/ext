#!/bin/bash

#cp /home/www/nxtW /home/

NOW=$(date -u +"%Y-%m-%d %T, %Z")
echo "$NOW > Eyelock NXT Start" >> /home/root/nxtEvent.log

NOW=$(date +"%Y-%m-%d, %T.000")
echo "$NOW, INFO , [Eyelock], - Eyelock NXT Start" >> /home/root/nxtLog.log

cd /home/
touch nxtW.run
chmod 777 nxtW
bash -c "while true; do if [ -f /home/nxtW.run ]; then /home/nxtW; fi; sleep 4; done" &
#./nxtW &
cd /home/root
rm *.bin
chmod -R 755 root/
chmod a+x *
chmod a+x ./scripts/*
chmod 755 /home/root/scripts/*.sh
chmod 755 /home/www/scripts/*.sh
chmod 755 /home/*.sh

if [[ ! -d /home/www-internal ]]
then
	mkdir /home/www-internal
fi
chown -R www-data:www-data /home/root
chown -R www-data:www-data /home/www
chown -R www-data:www-data /home/www-internal
chown -R www-data:www-data /home/default
# on NXT: launchwebserver.sh: /bin/chown -R www:www /home
# need to be reviewed from security perspective

touch ./rootCert/CA/index.text.attr

echo ******************************************
echo running from startup.sh
echo ******************************************


#./flash.sh 
#./reloadinterfaces.sh
#/etc/init.d/avahi-daemon restart


ID=`cat /home/root/id.txt`
sed -i "s/nano.*-1.local/nanonxt${ID}-1.local/g" /home/root/Eyelock.ini
sed -i "s/nano.*-0.local/nanonxt${ID}.local/g" /home/root/Eyelock.ini

export NTP_SERVER=`grep -e "GRI\.InternetTimeAddr\=.*" Eyelock.ini | cut -f2 -d'='`

# Only run Eyelock when Eyelock.run exists
chmod a+x ./icm_communicator
chmod a+x ./Eyelock
touch Eyelock.run
#bash -c "while true; do if [ -f /home/root/cvPreview.run ]; then /home/root/cvPreview; fi; sleep 6; done" &
bash -c "while true; do if [ -f /home/root/Eyelock.run ]; then /home/root/Eyelock; fi; sleep 6; done" &
#chmod a+x knockd
#/usr/sbin/knockd -d -c /home/root/knockd.conf 
#add firewall rules
#chmod a+x firewall.sh
#bash -c "while true; do sleep 1; ./firewall.sh; break; done" &
echo "******************************script finished ************************"


