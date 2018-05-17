#!/bin/sh

# sh this-script-name case-switch-scenario options-like-ipaddr-gateway-broadcast-netmask
# eg
# sh sed-ipconfig.sh ips  p_NewIpOfBoard  p_Netmask  p_Broadcast  p_Gateway
# sh sed-ipconfig.sh ips  192.168.10.224  255.255.255.0  192.168.10.255  192.168.10.19
# 

ROOT_DIR_PATH="/home/root"
EYELOCK_INI_FNAME=$ROOT_DIR_PATH"/Eyelock.ini"
CMDFILE_2EXEC=$ROOT_DIR_PATH"/cmdExecFile.sh"
EXECUTEDFILE=$ROOT_DIR_PATH"/executedCmdFile"


scriptOpt="$1"
touch $EXECUTEDFILE
touch $CMDFILE_2EXEC

grepVal="`cat /home/root/Eyelock.ini | grep NwListenerSecure`"
grepSemiColonVal="`cat /home/root/Eyelock.ini | grep NwListenerSecure | grep \";\"`"

print_return_val()
{
	echo "retval=\$?" >> $CMDFILE_2EXEC
	echo "if [ \$retval -eq 0 ]; then" >> $CMDFILE_2EXEC
		echo "# return success" >> $CMDFILE_2EXEC
		echo "echo \"2\" > $EXECUTEDFILE" >> $CMDFILE_2EXEC
		echo "cp /home/root/Eyelock.ini  /home/eyelock/Eyelock.ini" >> $CMDFILE_2EXEC
		echo "echo 0" >> $CMDFILE_2EXEC
	echo "else" >> $CMDFILE_2EXEC
		echo "# return failure" >> $CMDFILE_2EXEC
		echo "echo \"2\" > $EXECUTEDFILE" >> $CMDFILE_2EXEC
		echo "echo -1" >> $CMDFILE_2EXEC
	echo "fi" >> $CMDFILE_2EXEC
}

chkFile2Wr()
{
	sleep 1
	if [ ! -e "$EXECUTEDFILE" ]; then
		touch $EXECUTEDFILE
		echo "0" > $EXECUTEDFILE
	fi
	sleep 1
	execVal="`cat $EXECUTEDFILE`"
	if [ "$execVal" == "2" ] ; then	# 1-written,2-executed,0-not-written-not-executed-yet
		echo > $EXECUTEDFILE	#deleted the contents
		echo > $CMDFILE_2EXEC	#deleted the contents
		echo "0" > $EXECUTEDFILE
	fi
}

exec_sed_cmd()
{
	echo " sed -i -e \"s/\($1\)=\(.*\)\(:[0-9]*\)/\1=$2/g\"  $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
	print_return_val
}

exec_sed_cmd_1()
{
	echo " sed -i -e \"s/\($1\)=\(.*\)/\1=$2/g\" $EYELOCK_INI_FNAME "  >> $CMDFILE_2EXEC
	print_return_val
}

exec_sed_cmd_2()
{
	secVal="$2"

	case "$grepVal" in
		"")
			case "$secVal" in
				"secure")
					echo "echo \"$1=$2\" >> $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
					print_return_val
					;;
				*)
					echo "echo \""should not be reachable"\" "  >> $CMDFILE_2EXEC
					;;
			esac

			;;
		*)
			case "$grepSemiColonVal" in
				"")
					case "$secVal" in
						"nonsecure")
							echo " sed -i -e \"s/\($1\)=\(.*\)/;\1=$2/g\" $EYELOCK_INI_FNAME "  >> $CMDFILE_2EXEC
							print_return_val
							;;
						"secure")
							echo "echo \""should not be reachable"\" " >> $CMDFILE_2EXEC
							;;
					esac
					;;
				*)
					case "$secVal" in
						"secure")
							echo " sed -i -e \"s/;\($1\)=\(.*\)/\1=$2/g\" $EYELOCK_INI_FNAME "  >> $CMDFILE_2EXEC
							print_return_val
							;;
						"nonsecure")
							echo "echo \""should not be reachable "\" " >> $CMDFILE_2EXEC
							;;
					esac
					;;
			esac

			;;
	esac

}

chkFile2Wr
case $scriptOpt in
	"begin_addr")
		#sh $0 $1 $2 $3 $4 $5
		#sh $0=<sed-ipconfig.sh>  $1=<begin_addr>  $2=<GRI_HDMatcher_0_Address>  $3=<GRI_SlaveAddressList>  $4=<GRI_HBDestAddress>  $5=<GRI_MatchResultDestAddr>
		#sh sed-ipconfig.sh begin_addr  GRI_HDMatcher_0_Address GRI_SlaveAddressList GRI_HBDestAddress GRI_MatchResultDestAddr

		GRI_HDMatcher_0_Address="$2"
		GRI_SlaveAddressList="$3"
		GRI_HBDestAddress="$4"
		GRI_MatchResultDestAddr="$5"

		echo "option passed are : \r \"$0\", \r\t  \"$1\", \r\t  GRI_HDMatcher_0_Address=\"$2\", \r\t GRI_SlaveAddressList=\"$3\", \r\t GRI_HBDestAddress=\"$4\", \r\t GRI_MatchResultDestAddr=\"$5\" "

		echo " sed -i -e \"s/\(GRI.HDMatcher.0.Address\)=\(.*\)\(:[0-9]*\)/\1="$GRI_HDMatcher_0_Address"/g\"  $EYELOCK_INI_FNAME "	>> $CMDFILE_2EXEC
		echo " sed -i -e \"s/\(GRI.SlaveAddressList\)=\(.*\)\(:[0-9]*\)/\1="$GRI_SlaveAddressList"/g\"  $EYELOCK_INI_FNAME "	>> $CMDFILE_2EXEC
		echo " sed -i -e \"s/\(GRI.HBDestAddress\)=\(.*\)\(:[0-9]*\)/\1="$GRI_HBDestAddress"/g\"  $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		echo " sed -i -e \"s/\(GRI.MatchResultDestAddr\)=\(.*\)\(:[0-9]*\)/\1="$GRI_MatchResultDestAddr"/g\"  $EYELOCK_INI_FNAME "	>> $CMDFILE_2EXEC
		print_return_val

		;;

	"FSFEnable_HasValue")
		GRITrigger_F2FEnable="$2"
		#		echo " sed -i -e \"s/\(GRITrigger.F2FEnable\)=\(.*\)/\1="$GRITrigger_F2FEnable"/g\"  $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		exec_sed_cmd_1  "GRITrigger.F2FEnable"  "$GRITrigger_F2FEnable"
		;;

	"GRITrigger_WeigandEnable")
		GRITrigger_WeigandEnable="$2"
		#		echo " sed -i -e \"s/\(GRITrigger.WeigandEnable\)=\(.*\)/\1="$GRITrigger_WeigandEnable"/g\"  $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		exec_sed_cmd_1  "GRITrigger.WeigandEnable"  "$GRITrigger_WeigandEnable"
		;;

	"GRITrigger_RelayEnable")
		GRITrigger_RelayEnable_HasValue="$2"
		#		echo " sed -i -e \"s/\(GRITrigger.RelayEnable\)=\(.*\)/\1="$GRITrigger_RelayEnable"/g\"  $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		exec_sed_cmd_1  "GRITrigger.RelayEnable"  "$GRITrigger_RelayEnable_HasValue"
		;;

	"last_tone")
		RelayTimeInMS="$2"
		AuthorizationToneFrequency="$3"
		AuthorizationToneVolume="$4"
		AuthorizationToneDurationSeconds="$5"
		LEDBrightness="$6"
		RepeatAuthorizationPeriod="$7"

		echo " sed -i -e \"s/\(GRITrigger.RelayTimeInMS\)=\(.*\)/\1="$RelayTimeInMS"/g\"  $EYELOCK_INI_FNAME " >>  $CMDFILE_2EXEC
		echo " sed -i -e \"s/\(GRI.AuthorizationToneFrequency\)=\(.*\)/\1="$AuthorizationToneFrequency"/g\"  $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		echo " sed -i -e \"s/\(GRI.AuthorizationToneVolume\)=\(.*\)/\1="$AuthorizationToneVolume"/g\"  $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		echo " sed -i -e \"s/\(GRI.AuthorizationToneDurationSeconds\)=\(.*\)/\1="$AuthorizationToneDurationSeconds"/g\"  $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		echo " sed -i -e \"s/\(GRI.LEDBrightness\)=\(.*\)/\1="$LEDBrightness"/g\"  $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		echo " sed -i -e \"s/\(GRI.RepeatAuthorizationPeriod\)=\(.*\)/\1="$RepeatAuthorizationPeriod"/g\"  $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC

		print_return_val
		;;

	"fan_speed")
		FAN_SPEED_LEFT="$2"
		FAN_SPEED_RIGHT="$3"
		# sed -i 's/\(pwr_bd_if f\) \(.*\)/\1 {0} {1}/g' startup.sh", l_startUpDetails.FAN_SPEED_LEFT,l_startUpDetails.FAN_SPEED_RIGHT)
		#sed -i \"s/\(pwr_bd_if f\) \(.*\)/\1 $FAN_SPEED_LEFT $FAN_SPEED_RIGHT/g\" $ROOT_DIR_PATH\"/startup.sh\"
		echo " sed -i -e \"s/\(pwr_bd_if f\) \(.*\)/\1 $FAN_SPEED_LEFT $FAN_SPEED_RIGHT/g\" /home/root/eyestartup " >> $CMDFILE_2EXEC
		echo " cp /home/root/eyestartup /etc/init.d/eyestartup " >> $CMDFILE_2EXEC
		echo " cp /etc/init.d/eyestartup /etc/rc5.d/S95eyestartup " >> $CMDFILE_2EXEC
		print_return_val
		;;

	"rdCfg_HDMtch")
		GRI_HDMatcher_0_Address_rdCfg="$2"
		# sed -i -e ""s/\\(GRI.HDMatcher.0.Address\\)=\\(.*\\)\\(:[0-9]*\\)/\\1={0}/g"" {1}{2}", l_ConfigIniDetails.GRI_HDMatcher_0_Address.ToString(), p_PathFolder, p_Filename))
		#		echo " sed -i -e \"s/\(GRI.HDMatcher.0.Address\)=\(.*\)\(:[0-9]*\)/\1=$GRI_HDMatcher_0_Address_rdCfg/g\"  $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		exec_sed_cmd  "GRI.HDMatcher.0.Address"  "$GRI_HDMatcher_0_Address_rdCfg"
		;;

	"rdCfg_SlaveAddr")
		GRI_SlaveAddressList_rdCfg="$2"
		# sed -i -e ""s/\\(GRI.SlaveAddressList\\)=\\(.*\\)\\(:[0-9]*\\)/\\1={0}/g"" {1}{2}", l_ConfigIniDetails.GRI_SlaveAddressList.ToString(), p_PathFolder, p_Filename))
		#		echo " sed -i -e \"s/\(GRI.SlaveAddressList\)=\(.*\)\(:[0-9]*\)/\1=$GRI_SlaveAddressList_rdCfg/g\" $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		exec_sed_cmd  "GRI.SlaveAddressList"  "$GRI_SlaveAddressList_rdCfg"
		;;

	"rdCfg_HBDstAddr")
		GRI_HBDestAddress_rdCfg="$2"
		# sed -i -e ""s/\\(GRI.HBDestAddress\\)=\\(.*\\)\\(:[0-9]*\\)/\\1={0}/g"" {1}{2}", l_ConfigIniDetails.GRI_HBDestAddress.ToString(), p_PathFolder, p_Filename))
		#		echo " sed -i -e \"s/\(GRI.HBDestAddress\)=\(.*\)\(:[0-9]*\)/\1=$GRI_HBDestAddress_rdCfg/g\" $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		exec_sed_cmd  "GRI.HBDestAddress"  "$GRI_HBDestAddress_rdCfg"
		;;

	"rdCfg_MtchDstAddr_inIf")
		GRI_MatchResultDestAddr_rdCfg="$2"
		# sed -i -e ""s/GRI.MatchResultDestAddr/;GRI.MatchResultDestAddr/""  {1}{2} ; sed -i -e ""s/;*GRI.MatchResultDestAddr/;GRI.MatchResultDestAddr/""  {1}{2}", l_ConfigIniDetails.GRI_MatchResultDestAddr.ToString(), p_PathFolder, p_Filename))
		echo " sed -i -e \"s/GRI.MatchResultDestAddr/;GRI.MatchResultDestAddr/\"  $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		echo " sed -i -e \"s/;*GRI.MatchResultDestAddr/;GRI.MatchResultDestAddr/\"  $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		print_return_val
		;;

	"rdCfg_MtchDstAddr_inElse")
		GRI_MatchResultDestAddr_rdCfg_else="$2"
		# sed -i -e ""s/;GRI.MatchResultDestAddr/GRI.MatchResultDestAddr/""  {1}{2}; sed -i -e ""s/\\(GRI.MatchResultDestAddr\\)=\\(.*\\)/\\1={0}/g""  {1}{2}", l_ConfigIniDetails.GRI_MatchResultDestAddr.ToString(), p_PathFolder, p_Filename))
		echo " sed -i -e \"s/;GRI.MatchResultDestAddr/GRI.MatchResultDestAddr/\"  $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		echo " sed -i -e \"s/\(GRI.MatchResultDestAddr\)=\(.*\)/\1=$GRI_MatchResultDestAddr_rdCfg_else/g\"  $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		print_return_val
		;;

	"rdCfg_TrigF2FEn")
		GRITrigger_F2FEnable_rdCfg="$2"
		# sed -i -e ""s/\\(GRITrigger.F2FEnable\\)=\\(.*\\)/\\1={0}/g"" {1}{2}", l_ConfigIniDetails.GRITrigger_F2FEnable.ToString().ToLower(), p_PathFolder, p_Filename))
		#		echo " sed -i -e \"s/\(GRITrigger.F2FEnable\)=\(.*\)/\1=$GRITrigger_F2FEnable_rdCfg/g\" $EYELOCK_INI_FNAME "  >> $CMDFILE_2EXEC
		exec_sed_cmd_1  "GRITrigger.F2FEnable"  "$GRITrigger_F2FEnable_rdCfg"
		;;

	"rdCfg_TrigWeigandEn")
		GRITrigger_WeigandEnable_rdCfg="$2"
		# sed -i -e ""s/\\(GRITrigger.WeigandEnable\\)=\\(.*\\)/\\1={0}/g"" {1}{2}", l_ConfigIniDetails.GRITrigger_WeigandEnable.ToString().ToLower(), p_PathFolder, p_Filename))
		#		echo " sed -i -e \"s/\(GRITrigger.WeigandEnable\)=\(.*\)/\1=$GRITrigger_WeigandEnable_rdCfg/g\" $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		exec_sed_cmd_1  "GRITrigger.WeigandEnable"  "$GRITrigger_WeigandEnable_rdCfg"
		;;

	"rdCfg_TrigRelayEn")
		GRITrigger_RelayEnable_rdCfg="$2"
		# sed -i -e ""s/\\(GRITrigger.RelayEnable\\)=\\(.*\\)/\\1={0}/g"" {1}{2}", l_ConfigIniDetails.GRITrigger_RelayEnable.ToString().ToLower(), p_PathFolder, p_Filename));
		#		echo " sed -i -e \"s/\(GRITrigger.RelayEnable\)=\(.*\)/\1=$GRITrigger_RelayEnable_rdCfg/g\" $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		exec_sed_cmd_1  "GRITrigger.RelayEnable"  "$GRITrigger_RelayEnable_rdCfg"
		;;

	"rdCfg_TrigRelayTime")
		GRITrigger_RelayTimeInMS_rdCfg="$2"
		# sed -i -e ""s/\\(GRITrigger.RelayTimeInMS\\)=\\(.*\\)/\\1={0}/g"" {1}{2}", l_ConfigIniDetails.GRITrigger_RelayTimeInMS.ToString(), p_PathFolder, p_Filename));
		#		echo " sed -i -e \"s/\(GRITrigger.RelayTimeInMS\)=\(.*\)/\1=$GRITrigger_RelayTimeInMS_rdCfg/g\" $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		exec_sed_cmd_1  "GRITrigger.RelayTimeInMS"  "$GRITrigger_RelayTimeInMS_rdCfg"
		;;

	"rdCfg_AuthToneFreq")
		GRI_AuthorizationToneFrequency_rdCfg="$2"
		# sed -i -e ""s/\\(GRI.AuthorizationToneFrequency\\)=\\(.*\\)/\\1={0}/g"" {1}{2}", l_ConfigIniDetails.GRI_AuthorizationToneFrequency.ToString(), p_PathFolder, p_Filename));
		#		echo " sed -i -e \"s/\(GRI.AuthorizationToneFrequency\)=\(.*\)/\1=$GRI_AuthorizationToneFrequency_rdCfg/g\" $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		exec_sed_cmd_1  "GRI.AuthorizationToneFrequency"  "$GRI_AuthorizationToneFrequency_rdCfg"
		;;

	"rdcfg_AuthToneVol")
		GRI_AuthorizationToneVolume_rdCfg="$2"
		# sed -i -e ""s/\\(GRI.AuthorizationToneVolume\\)=\\(.*\\)/\\1={0}/g"" {1}{2}", l_ConfigIniDetails.GRI_AuthorizationToneVolume.ToString(), p_PathFolder, p_Filename));
		#		echo " sed -i -e \"s/\(GRI.AuthorizationToneVolume\)=\(.*\)/\1=$GRI_AuthorizationToneVolume_rdCfg/g\" $EYELOCK_INI_FNAME " > $CMDFILE_2EXEC
		exec_sed_cmd_1  "GRI.AuthorizationToneVolume"  "$GRI_AuthorizationToneVolume_rdCfg"
		;;

	"rdCfg_AuthToneDurSec")
		GRI_AuthorizationToneDurationSeconds_rdCfg="$2"
		# sed -i -e ""s/\\(GRI.AuthorizationToneDurationSeconds\\)=\\(.*\\)/\\1={0}/g"" {1}{2}", l_ConfigIniDetails.GRI_AuthorizationToneDurationSeconds.ToString(), p_PathFolder, p_Filename));
		#		echo " sed -i -e \"s/\(GRI.AuthorizationToneDurationSeconds\)=\(.*\)/\1=$GRI_AuthorizationToneDurationSeconds_rdCfg/g\" $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		exec_sed_cmd_1  "GRI.AuthorizationToneDurationSeconds"  "$GRI_AuthorizationToneDurationSeconds_rdCfg"
		;;

	"rdCfg_LEDBrite")
		GRI_LEDBrightness_rdCfg="$2"
		# "sed -i -e ""s/\\(GRI.LEDBrightness\\)=\\(.*\\)/\\1={0}/g"" {1}{2}", l_ConfigIniDetails.GRI_LEDBrightness.ToString(), p_PathFolder, p_Filename));
		#		echo " sed -i -e \"s/\(GRI.LEDBrightness\)=\(.*\)/\1=$GRI_LEDBrightness_rdCfg/g\" $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		exec_sed_cmd_1  "GRI.LEDBrightness"  "$GRI_LEDBrightness_rdCfg"
		;;

	"rdCfg_RptAuthPrd")
		GRI_RepeatAuthorizationPeriod_rdCfg="$2"
		# "sed -i -e ""s/\\(GRI.RepeatAuthorizationPeriod\\)=\\(.*\\)/\\1={0}/g"" {1}{2}", l_ConfigIniDetails.GRI_RepeatAuthorizationPeriod.ToString(), p_PathFolder, p_Filename));
		#		echo " sed -i -e \"s/\(GRI.RepeatAuthorizationPeriod\)=\(.*\)/\1=$GRI_RepeatAuthorizationPeriod_rdCfg/g\" $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		exec_sed_cmd_1  "GRI.RepeatAuthorizationPeriod"  "$GRI_RepeatAuthorizationPeriod_rdCfg"
		;;

	"rdCfg_TSMstrDstAddr")
		Eyelock_TSMasterDestAddr_rdCfg="$2"
		# "sed -i -e ""s/\\(Eyelock.TSMasterDestAddr\\)=\\(.*\\)/\\1={0}/g"" {1}{2}", l_ConfigIniDetails.GRI_HDMatcher_0_Address.ToString(), p_PathFolder, p_Filename));
		#		echo " sed -i -e \"s/\(Eyelock.TSMasterDestAddr\)=\(.*\)/\1=$Eyelock_TSMasterDestAddr_rdCfg/g\" $EYELOCK_INI_FNAME " >> $CMDFILE_2EXEC
		exec_sed_cmd_1  "Eyelock.TSMasterDestAddr"  "$Eyelock_TSMasterDestAddr_rdCfg"
		;;

	"rdCfg_NwListSec")
		GRI_NwListenerSecure_rdCfg="$2"

		strSecVal="`cat /home/root/Eyelock.ini | grep \"\;Keys For secure Communication\"`"
                if [ "$strSecVal" == "" ] ; then
                        echo "echo \""\;Keys For secure Communication"\" >> $EYELOCK_INI_FNAME" >> $CMDFILE_2EXEC
                fi


		exec_sed_cmd_2  "GRI.NwListenerSecure"  "$GRI_NwListenerSecure_rdCfg"
		exec_sed_cmd_2  "GRI.HBDispatcherSecure"  "$GRI_NwListenerSecure_rdCfg"
		exec_sed_cmd_2  "GRI.NwDispatcherSecure"  "$GRI_NwListenerSecure_rdCfg"
		exec_sed_cmd_2  "Eyelock.PullDBSecure"  "$GRI_NwListenerSecure_rdCfg"
		exec_sed_cmd_2  "GRI.EyeDispatcherSecure"  "$GRI_NwListenerSecure_rdCfg"
		exec_sed_cmd_2  "Eyelock.MasterSlaveCommSecure"  "$GRI_NwListenerSecure_rdCfg"
		exec_sed_cmd_2  "Eyelock.SlaveMasterCommSecure"  "$GRI_NwListenerSecure_rdCfg"
		exec_sed_cmd_2  "Eyelock.WeigandDispatcherSecure"  "$GRI_NwListenerSecure_rdCfg"
		exec_sed_cmd_2  "Eyelock.F2FDispatcherSecure"  "$GRI_NwListenerSecure_rdCfg"
		exec_sed_cmd_2  "Eyelock.NwLEDDispatcherSecure"  "$GRI_NwListenerSecure_rdCfg"

	;;

	*)
		echo "no option been provided"
		return -1
		;;

esac

chmod a+x $CMDFILE_2EXEC
