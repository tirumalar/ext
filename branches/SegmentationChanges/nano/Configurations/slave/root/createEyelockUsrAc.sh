#!/bin/sh

#exec 9>&1
#exec 9>&2
#exec > /home/eyeuserlogfile

copyFiles()
{
	cp -R /home/root/* /home/eyelock/
	chown -R eyelock:eyelock /home/eyelock/*
	sync
}

if [ ! -e "/etc/rc5.d/S95eyestartup" ] ; then
# eyelock app to run as root user
echo "no /home/root/eyestartup file"
	if [ -e "/etc/init.d/eyestartup" -o ! -e "/etc/init.d/eyestartup" ] ; then
		rm -rf /etc/init.d/eyestartup
		rm -rf /etc/rc5.d/S95eyestartup
		rm -rf /etc/init.d/s95startup > /dev/null 2>&1
		ln -sf /home/root/startup.sh /etc/rc5.d/S95Startup
		sync
	fi
else
	#eyelock app to run as eyelock user
	#check if user eyelock available by checking in /etc/passwd file
	chkUserName=`cat /etc/passwd | grep eyelock | cut -f6 -d:`
	usermod -s /usr/sbin/nologin eyelock

	if [ "$chkUserName" == "/home/eyelock" ] ; then
		echo "user: eyelock existing ,so username eyelock is created"
		usermod -s /usr/sbin/nologin eyelock
		copyFiles
	else
		#user - eyelock is created and with password as "eyelock"
		#if need to change need to change here in this file
		adduse -D eyelock
		echo "username \"eyelock\" is added into the system"
		echo "user account : eyelock password is changed"

		sync
		sleep 2
		copyFiles

		#to delete username, and deleting the user's home dir files
		# userdel <username> , for eg., userdel eyelock
		# rm -rf /home<username> , for eg., rm -rf /home/eyelock

	fi
fi # for /home/root/eyestartup chk

#exec 1>9& 9>&-
#exec 2>9& 9>&-
