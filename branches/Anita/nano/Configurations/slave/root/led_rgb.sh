#!/bin/sh

chkRetVal()
{
if [ $1 -eq 0 ] ; then
	return 0
else
	return -1
fi
}

opt="$1"
case "$opt" in
	"killEyelockApp")
		# sh <this-script-name> killEyelockApp
		# sh /home/root/led_rgb.sh killEyelockApp
		nameDir=`dirname $0`
		cd /home/root
		"./"identityNkillEyelock.sh
		;;

"rgb")
	chmod a+x $3
	# give in c# code as given below religiously
	# sh <this-script-name> rgb looptime p_ScriptFilename
	# sh /home/root/led_rgb.sh rgb looptime nano_led.sh
	looptime=$2
	while [ $looptime -gt 0 ]
	do
		#10 0 0 0
		./$3 10 0 0 0 # red
		rv=$?
		if [ $rv -eq 0 ]; then
			echo ""
		fi
		sleep 1

		./$3 0 10 0 0 # green
		rv=$?
		if [ $rv -eq 0 ]; then
			echo ""
		fi
		sleep 1

		./$3 0 0 10 0 # blue
		rv=$?
		if [ $rv -eq 0 ]; then
			echo ""
		fi
		sleep 1

		looptime=$(($looptime - 1 ))
		echo "looptime = $looptime"

	done
;;

"last")
	# sh <this-script-name> last p_ScriptFilename p_AppName
	# sh /home/root/led_rgb.sh last nano_led.sh Eyelock

	#./nano_led 1 1 1 255
	chmod a+x $2
	./$2 1 1 1 255
sleep 1
;;

"runInfo")
cd /home/root
ls -la | grep Eyelock.run
;;

*)
	echo "pass option as [killEyelockApp | rgb | last | runInfo]"
	return -1
	;;

esac
