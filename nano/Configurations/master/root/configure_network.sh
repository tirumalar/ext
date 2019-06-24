#!/bin/bash
NUMBER_REGEX='^[0-9]+$'

configureStatic(){
	ifconfig "${IFACE}" down
	sleep 1
	ifconfig "${IFACE}" hw ether "${MAC}"
	sleep 1
	ifconfig "${IFACE}" inet "${IP}" netmask "${MASK}" up
		
	if [[ ${ENABLED_8021X} == 'true' ]]
	then
		# in NXT it is /home/wpa_supplicant
		killall -KILL wpa_supplicant
		echo "$(date +'%Y-%m-%d, %T.000'), INFO , [NetworkConfiguration], - 802.1x is enabled, starting wpa_supplicant" >> /home/root/nxtLog.log
		wpa_supplicant -c /home/www-internal/802.1XCerts/wpa_supplicant-wired.conf -D wired -i "${IFACE}" -ddd -t >> /home/root/wpa_supplicant.log &
	fi
	#sleep "${WPA_TIMEOUT}"

	route add default gw "${GW}"
}

configureDhcp(){
	killall -KILL dhclient
	ifconfig "${IFACE}" down
	sleep 1
	ifconfig "${IFACE}" hw ether "${MAC}"
	sleep 1
	ifconfig "${IFACE}" up
	sleep 1

	if [[ ${ENABLED_8021X} == 'true' ]]
	then
		# in NXT it is /home/wpa_supplicant
		killall -KILL wpa_supplicant
		echo "$(date +'%Y-%m-%d, %T.000'), INFO , [NetworkConfiguration], - 802.1x is enabled, starting wpa_supplicant" >> /home/root/nxtLog.log
		wpa_supplicant -c /home/www-internal/802.1XCerts/wpa_supplicant-wired.conf -D wired -i "${IFACE}" -ddd -t >> /home/root/wpa_supplicant.log &
		sleep "${WPA_TIMEOUT}"
	fi
	
	bash -c "dhclient -1 -d -v -4 ${IFACE}" &
}

sleep 5
echo "$(date +'%Y-%m-%d, %T.000'), INFO , [NetworkConfiguration], - USB-ethernet adapter connected" >> /home/root/nxtLog.log

INTERFACES_FILE='/home/www-internal/interfaces'
INTERFACES_FILE_6='/home/www-internal/interfaces6'
RESOLV_FILE='/etc/resolvconf/resolv.conf.d/tail'
IFACE='usbnet0'

DHCP_TIMEOUT="$(grep '^#dhcp_timeout' ${INTERFACES_FILE} | cut -d' ' -f2)"
if ! [[ ${DHCP_TIMEOUT} =~ ${NUMBER_REGEX} ]]
then
   DHCP_TIMEOUT=15
fi

DHCP_RETRIES="$(grep '^#dhcp_retries' ${INTERFACES_FILE} | cut -d' ' -f2)"
if ! [[ ${DHCP_RETRIES} =~ ${NUMBER_REGEX} ]]
then
   DHCP_RETRIES=3
fi

DHCP_RETRY_DELAY="$(grep '^#dhcp_retry_delay' ${INTERFACES_FILE} | cut -d' ' -f2)"
if ! [[ ${DHCP_RETRY_DELAY} =~ ${NUMBER_REGEX} ]]
then
   DHCP_RETRY_DELAY=1
fi

DHCP6_TIMEOUT=120
WPA_TIMEOUT=60

MAC="$(grep '^hwaddress ether' ${INTERFACES_FILE} | cut -d' ' -f3)"
echo "$(date +'%Y-%m-%d, %T.000'), INFO , [NetworkConfiguration], - MAC address: ${MAC}" >> /home/root/nxtLog.log

# 802.1X
if grep -q "^#EnableIEEE8021X=true" /home/www-internal/802.1XCerts/wpa_supplicant-wired.conf
then
	ENABLED_8021X='true'
else
	ENABLED_8021X='false'
fi
# 1 if string not found in file
# 2 if file not found
if [[ ${ENABLED_8021X} == 'false' ]]
then
	echo "$(date +'%Y-%m-%d, %T.000'), INFO , [NetworkConfiguration], - 802.1x is disabled" >> /home/root/nxtLog.log
else
	echo "$(date +'%Y-%m-%d, %T.000'), INFO , [NetworkConfiguration], - 802.1x is enabled, wpa_supplicant will be started" >> /home/root/nxtLog.log
		
	#log rotation
	bash -c "while true; do /home/root/rotateLogs.sh /home/root/wpa_supplicant.log 1000; sleep 10; done" &
fi

# IPv6
if grep -q "^EnableIPv6=true" "${INTERFACES_FILE_6}"
then
	ENABLED_IPV6='true'
else
	ENABLED_IPV6='false'
fi

DHCP_MODE_6_STR="$(grep '^DhcpMode' ${INTERFACES_FILE_6} | cut -d'=' -f2)"
if [[ ${DHCP_MODE_6_STR} == 'auto' ]]
then
	#RECEIVED_ADVERTISEMENTS=$(timeout 15 radvdump -d4) 
	RADVDUMP_OUT="/tmp/radvdump_out"
	timeout 15 radvdump -d4 > "${RADVDUMP_OUT}" &
fi

if grep -q "^Address" "${INTERFACES_FILE_6}"
then
	IP6="$(grep '^Address' ${INTERFACES_FILE_6} | cut -d'=' -f2)"
	MASK6="$(grep '^SubnetPrefixLength' ${INTERFACES_FILE_6} | cut -d'=' -f2)"
	GW6="$(grep '^Gateway' ${INTERFACES_FILE_6} | cut -d'=' -f2)"
	DNS16="$(grep '^Dns1' ${INTERFACES_FILE_6} | cut -d'=' -f2)"
	DNS26="$(grep '^Dns2' ${INTERFACES_FILE_6} | cut -d'=' -f2)"
fi

if [[ ${ENABLED_IPV6} == 'false' ]]
then
	sysctl -w net.ipv6.conf.usbnet0.disable_ipv6=1
else
	sysctl -w net.ipv6.conf.usbnet0.disable_ipv6=0
	if grep -q "^AcceptRouterAdvertisements=true" "${INTERFACES_FILE_6}"
	then
		sysctl -w net.ipv6.conf.usbnet0.accept_ra=1
	else
		sysctl -w net.ipv6.conf.usbnet0.accept_ra=0
	fi
fi

echo '' > "${RESOLV_FILE}"
	
if grep -q "${IFACE} inet static" "${INTERFACES_FILE}"
then
	IP="$(grep '^address' ${INTERFACES_FILE} | cut -d' ' -f2)"
	MASK="$(grep '^netmask' ${INTERFACES_FILE} | cut -d' ' -f2)"
	BRDCAST="$(grep '^broadcast' ${INTERFACES_FILE} | cut -d' ' -f2)"
	GW="$(grep '^gateway' ${INTERFACES_FILE} | cut -d' ' -f2)"
	DNS1="$(grep '^dns-nameservers' ${INTERFACES_FILE} | cut -d' ' -f2)"
	DNS2="$(grep '^dns-nameservers' ${INTERFACES_FILE} | cut -d' ' -f3)"
	
	for (( c=1; c<=3; c++ ))
	do
		echo "$(date +'%Y-%m-%d, %T.000'), INFO , [NetworkConfiguration], - configuring static IP ${IP}, netmask ${MASK}, broadcast ${BRDCAST}, gateway ${GW} (attempt ${c})" >> /home/root/nxtLog.log
		configureStatic
		sleep 3
		if ifconfig "${IFACE}" | grep -q 'RUNNING'
		then
			echo "$(date +'%Y-%m-%d, %T.000'), INFO , [NetworkConfiguration], - static IP configured" >> /home/root/nxtLog.log
			break
		fi
	done
	echo "nameserver ${DNS1}" >> "${RESOLV_FILE}"
	echo "nameserver ${DNS2}" >> "${RESOLV_FILE}"

elif grep -q "${IFACE} inet dhcp" "${INTERFACES_FILE}"
then
	for (( c=1; c<="${DHCP_RETRIES}"; c++ ))
	do
		echo "$(date +'%Y-%m-%d, %T.000'), INFO , [NetworkConfiguration], - configuring DHCP attempt ${c} out of ${DHCP_RETRIES}, timeout ${DHCP_TIMEOUT}" >> /home/root/nxtLog.log

		configureDhcp
		sleep "${DHCP_TIMEOUT}"
		if ifconfig "${IFACE}" | grep -q 'RUNNING'
		then
			if ifconfig "${IFACE}" | grep -q 'inet addr'
			then
				echo "$(date +'%Y-%m-%d, %T.000'), INFO , [NetworkConfiguration], - DHCP configured" >> /home/root/nxtLog.log
				break
			fi
		fi
		sleep "${DHCP_RETRY_DELAY}"
	done

	killall -KILL dhclient
	
	if ! ifconfig "${IFACE}" | grep -q 'inet addr'
	then
		echo "$(date +'%Y-%m-%d, %T.000'), INFO , [NetworkConfiguration], - configuring DHCP failed. Falling back to the default IP" >> /home/root/nxtLog.log
		DEFAULT_IP_LABEL='default'
		ifconfig "${IFACE}:${DEFAULT_IP_LABEL}" hw ether "${MAC}"
		sleep 1
		ifconfig "${IFACE}:${DEFAULT_IP_LABEL}" inet "169.254.1.1" netmask "255.255.0.0" up
		configureDhcp
	fi
fi

if [[ ${DHCP_MODE_6_STR} == 'auto' ]]
then
#TODO: check if radvdump is ready
	RA_MANAGED_FLAG=$(cat "${RADVDUMP_OUT}" | grep 'AdvManagedFlag' | cut -d' ' -f2  | tr -d ';')
	RA_OTHER_FLAG=$(cat "${RADVDUMP_OUT}" | grep 'AdvOtherConfigFlag' | cut -d' ' -f2  | tr -d ';')
	rm "${RADVDUMP_OUT}"
	if [[ ${RA_MANAGED_FLAG} == 'on' ]] 
	then
		echo "$(date +'%Y-%m-%d, %T.000'), INFO , [NetworkConfiguration], - Router Advertisements: managed flag is active" >> /home/root/nxtLog.log
		timeout "${DHCP6_TIMEOUT}" bash -c "dhclient -1 -d -v -6 -N ${IFACE}"
	elif [[ ${RA_OTHER_FLAG} == 'on' ]] 
	then
		echo "$(date +'%Y-%m-%d, %T.000'), INFO , [NetworkConfiguration], - Router Advertisements: other flag is active" >> /home/root/nxtLog.log
		timeout "${DHCP6_TIMEOUT}" bash -c "dhclient -1 -d -v -6 -S ${IFACE}"
	fi
elif [[ ${DHCP_MODE_6_STR} == 'normal' ]]
then
	timeout "${DHCP6_TIMEOUT}" bash -c "dhclient -1 -d -v -6 -N ${IFACE}"
elif [[ ${DHCP_MODE_6_STR} == 'information-only' ]]
then 
	timeout "${DHCP6_TIMEOUT}" bash -c "dhclient -1 -d -v -6 -S ${IFACE}"
else 
	echo "$(date +'%Y-%m-%d, %T.000'), INFO , [NetworkConfiguration], - IPv6 is disabled" >> /home/root/nxtLog.log
	# none - IPv6 dhcp disabled
fi

if [[ -n ${IP6} ]]
then
	if [[ -z ${MASK6} ]]
	then
		MASK6='64'
	fi

	ifconfig "${IFACE}" inet6 add "${IP6}"/"${MASK6}"
fi	

if [[ -n ${GW6} ]]
then
	ip -6 route add "${GW6}" dev "${IFACE}"	
fi	

if [[ -n ${DNS16} ]]
then
	echo "nameserver ${DNS16}" >> "${RESOLV_FILE}"
fi	

if [[ -n ${DNS26} ]]
then
	echo "nameserver ${DNS26}" >> "${RESOLV_FILE}"
fi	

resolvconf -u


