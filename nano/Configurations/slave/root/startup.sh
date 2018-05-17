#!/bin/sh
ldconfig
cd /home/root
chmod a+x *
chmod a+x ./scripts/*

sleep 5
echo 24 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio24/direction
echo low > /sys/class/gpio/gpio24/direction
echo 4 > /debug/omap_mux/gpmc_ncs4
echo 4 > /debug/omap_mux/gpmc_ncs5
echo 4 > /debug/omap_mux/gpmc_ncs6
echo 4 > /debug/omap_mux/gpmc_ncs7

echo 0x104 > /debug/omap_mux/dss_data1

echo 0x04 > /debug/omap_mux/etk_d10

insmod mt9p001.ko master_mode=2

ifconfig usb0 192.168.40.2



./flash.sh
./reloadinterfaces.sh
#/etc/init.d/avahi-daemon restart

ID=`cat /home/root/id.txt`
sed -i "s/nano.*-1.local/nanonxt${ID}-1.local/g" /home/root/Eyelock.ini
sed -i "s/nano.*-0.local/nanonxt${ID}.local/g" /home/root/Eyelock.ini

# Only run Eyelock when Eyelock.run exists
chmod a+x ./Eyelock
touch Eyelock.run
bash -c "while true; do if [ -f Eyelock.run ]; then ./Eyelock; fi; sleep 4; done" &

rm /home/root/*.tmp