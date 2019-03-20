#!/bin/bash

configureStatic(){
	ifconfig "${IFACE}" down
	sleep 1
	ifconfig "${IFACE}" hw ether "${MAC}"
	sleep 1
	ifconfig "${IFACE}" inet "${IP}" netmask "${MASK}" broadcast "${BRDCAST}" up
	
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
IFACE='usbnet0'
DHCP_RETRIES=3
DHCP_TIMEOUT=15
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

if grep -q "${IFACE} inet static" "${INTERFACES_FILE}"
then
	IP="$(grep '^address' ${INTERFACES_FILE} | cut -d' ' -f2)"
	MASK="$(grep '^netmask' ${INTERFACES_FILE} | cut -d' ' -f2)"
	BRDCAST="$(grep '^broadcast' ${INTERFACES_FILE} | cut -d' ' -f2)"
	GW="$(grep '^gateway' ${INTERFACES_FILE} | cut -d' ' -f2)"

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
	done

	killall -KILL dhclient
	
	if ! ifconfig "${IFACE}" | grep -q 'inet addr'
	then
		echo "$(date +'%Y-%m-%d, %T.000'), INFO , [NetworkConfiguration], - configuring DHCP failed. Falling back to the default IP" >> /home/root/nxtLog.log
		ifconfig "${IFACE}" down
		sleep 1
		ifconfig "${IFACE}" hw ether "${MAC}"
		sleep 1
		ifconfig "${IFACE}" inet "169.254.1.1" netmask "255.255.0.0" broadcast "169.254.255.255" up
	fi
fi

