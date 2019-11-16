i2cset -y 3 0x2e 4 6
sleep 1
touch temperature
scp temperature root@192.168.40.2:/home/root
sleep 1
rm temperature
