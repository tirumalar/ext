#!/bin/bash
cd /home/root

#Turning watchdog off
i2cset -y 3 0x2e 4 6
rm Eyelock.run

#Exiting Eyelock Process
killall -KILL Eyelock

#Running ICM_Communicator to get ICM Version
./icm_communicator -v

#Restarting the Watchdogs
touch Eyelock.run
i2cset -y 3 0x2e 4 7


