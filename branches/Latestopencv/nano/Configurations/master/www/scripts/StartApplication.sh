

killall -KILL PushButton

cd /home/root;touch Eyelock.run
ssh root@192.168.40.2 'cd /home/root;touch Eyelock.run'
sync

ssh root@192.168.40.2 'sync'

sleep 5
./PushButton &
i2cset -y 3 0x2e 4 7

sleep 1

i2cset -y 3 0x2e 4 7

sleep 1

i2cset -y 3 0x2e 4 7

sleep 1

i2cset -y 3 0x2e 4 7