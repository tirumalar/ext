#!/bin/bash

configureStatic(){
	ifconfig "${IFACE}" down
	sleep 1
	ifconfig "${IFACE}" hw ether "${MAC}"
	sleep 1
	ifconfig "${IFACE}" inet "${IP}" hw ether "${MAC}" netmask "${MASK}" broadcast "${BRDCAST}" up
	route add default gw "${GW}"
}

configureDhcp(){
	killall -KILL dhclient
	ifconfig "${IFACE}" down
	sleep 1
	ifconfig "${IFACE}" hw ether "${MAC}" up
	sleep 2
	bash -c "dhclient -1 -d -v -4 ${IFACE}" &
}

sleep 5
echo "$(date +'%Y-%m-%d, %T.000'), INFO , [NetworkConfiguration], - USB-ethernet adapter connected" >> /home/root/nxtLog.log

INTERFACES_FILE='/home/www-internal/interfaces'
IFACE='usbnet0'
DHCP_RETRIES=3
DHCP_TIMEOUT=15

MAC="$(grep '^hwaddress ether' ${INTERFACES_FILE} | cut -d' ' -f3)"
echo "$(date +'%Y-%m-%d, %T.000'), INFO , [NetworkConfiguration], - MAC address: ${MAC}" >> /home/root/nxtLog.log

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

