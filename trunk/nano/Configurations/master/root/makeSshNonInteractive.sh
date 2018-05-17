#!/bin/bash
#### this script is to make the ssh to slave non interactive ( for WebConfigTool)  ##########################
#### static ssh keys are added to the system, but it still requires to log-in using the password for the ####
#### firsttime. ############################################################################################# 
while true;
do
        cd /home/root/
        chmod a+x sshpass
        ./sshpass -p root ssh -o ConnectTimeout=10 -o BatchMode=yes -o StrictHostKeyChecking=no root@192.168.40.2 exit 10 
        if [ $? -eq 10 ]
        then
                echo "ssh connection poking success"
                logger "ssh connection poking success"
                break
        else
                echo "ssh connection poking failed..trying again"
                logger "ssh connection poking failed..trying again"
                sleep 30
        fi
done                                       
