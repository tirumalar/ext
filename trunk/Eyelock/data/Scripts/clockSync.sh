#!/bin/bash

#ping -q -c2 $1 > /dev/null
ping -q -c2 $1
if [[ $? -ne 0 ]]
then
        echo "Host $1 not reachable"
        echo "2" > /home/nxtwResult
        exit
fi

rdate -s $1 2>&1
if [ $? -ne 0 ]
then
        echo "could not synchronize with host $1"
        echo "3" > /home/nxtwResult
else
        echo "time set successfully"
        echo "0" > /home/nxtwResult
fi

