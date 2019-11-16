# use SED to delete the lines we added

UNAME=$(uname -r)
UNAME_READONLY_FS='3.0.0-BSP-dm37x-2.4-2Eyelock_NXT_6.0'

if [[ ${UNAME} == ${UNAME_READONLY_FS} ]]
then
	rm /tmp/etc/rc.conf
else
	sed -i -e "/export IPADDR0/ d" /etc/rc.d/rc.conf
	sed -i -e "/export NETMASK0/ d" /etc/rc.d/rc.conf
	sed -i -e "/export GATEWAY0/ d" /etc/rc.d/rc.conf
	sed -i -e "/export NAMESERVER0/ d" /etc/rc.d/rc.conf
	sed -i -e "/export NAMESERVER1/ d" /etc/rc.d/rc.conf

	#put the ip address line back as dhcp
	cat /etc/rc.d/rc.conf >/etc/rc.d/rcnew.conf
	echo -e "export IPADDR0=\"dhcp\"" >>/etc/rc.d/rcnew.conf

	cp /etc/rc.d/rcnew.conf /etc/rc.d/rc.conf
fi
	