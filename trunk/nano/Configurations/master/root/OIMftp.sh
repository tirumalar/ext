#!/bin/bash

#OIM ftp Credentials
host='192.168.4.172'
USER='guest'
PASSWD='guest'

#Audio files to upload to OIM ftp
AUTHFILE="auth.raw"
REJFILE="rej.raw"
TAMPERFILE="tamper1.raw"

#Calibration File to Download from OIM ftp
CALRECTFILE="CalRect.ini"

ftp -n -v $host << EOT
quote USER $USER
quote PASS $PASSWD
prompt
ls
quit
EOT

#Upload Audio Files to ftp
cd /home/root/tones
wput -B -t 2 $AUTHFILE ftp://$USER:$PASSWD@$host
wput -B -t 2 $REJFILE ftp://$USER:$PASSWD@$host
wput -B -t 2 $TAMPERFILE ftp://$USER:$PASSWD@$host

#Download CalRect File from ftp
cd ../
if ! wget -t 2 --user="$USER" --password="$PASSWD" "ftp://$host/$CALRECTFILE"
then
    echo "Error downloading file; CalRect.ini is not available"
    exit
else
    echo "CalRect.ini file download Successful"
fi


echo "*************************** Extftp script finished ***********************"
