#!/bin/bash
#### this script is to close telnet,ssh and other ports  ##########################
while true;
do
	/sbin/iptables -L | grep ssh
        if [ $? -eq 1 ]
        then
                echo "no rules existing "
                logger "adding firewall rules"
                #close telnet and ssh ports
                /usr/sbin/iptables -A INPUT -p tcp --dport 79 -j DROP
                /usr/sbin/iptables -A INPUT -p tcp --dport 23 -j DROP
                /usr/sbin/iptables -A INPUT -p tcp --dport 22 -j DROP
                break
        else
                echo "ssh is already blocked"
                logger "ssh and other ports are already blocked "
                break;
        fi
done                                       
