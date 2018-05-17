#!/bin/sh

#exec 10>&1
#exec 10>&2
#exec > /home/makeeyelockuserlog

STARTUP_DIR_PATH="/etc/rc5.d"
INIT_STARTUP_DIR="/etc/init.d"
SKEL_DIR_PATH="/home/root"

cd $SKEL_DIR_PATH

if [ ! -e /home/root/eyestartup ] ; then
sh $SKEL_DIR_PATH/createEyelockUsrAc.sh

else
chmod a+x eyestartup
chmod a+x run_eyelock_serv
cp  eyestartup  /etc/init.d/
sync
cd $STARTUP_DIR_PATH

chmod a+x /etc/init.d/eyestartup

ln -sf  ../init.d/eyestartup  "$STARTUP_DIR_PATH/S95eyestartup"
sync

if [ -e "/etc/rc5.d/S95Startup" ] ; then
rm -rf "/etc/rc5.d/S95Startup"
rm -rf /etc/init.d/startup.sh > /dev/null 2>&1
fi

sh $SKEL_DIR_PATH/createEyelockUsrAc.sh
fi

#exec 1>10& 10>&-
#exec 2>10& 10>&-
