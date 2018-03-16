sysctl -w net.core.rmem_default=3145704
sysctl -w net.core.rmem_max=3145704
ifconfig $1  hw ether f8:32:e4:9b:39:a2
ifconfig $1 192.168.4.170

