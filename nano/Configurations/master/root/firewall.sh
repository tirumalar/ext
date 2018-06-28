#!/bin/bash

iptables -F
iptables -P INPUT DROP
iptables -A INPUT -i lo -p all -j ACCEPT
iptables -A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT

iptables -A INPUT -i usbnet0 -p tcp -m multiport --dports 22,80,443,8081,8090 -j ACCEPT
iptables -A INPUT -i eth0 -p udp -m multiport --dports 8192,8193,8194 -j ACCEPT

iptables -A INPUT -j DROP

