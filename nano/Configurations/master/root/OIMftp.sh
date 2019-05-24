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
FACECONFIGFILE="Face.ini"

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
# Go to /home/root folder
cd ../

#Check if file exists and remove old file and download new file from OIM
if [ -f $CALRECTFILE ]
then
    echo $CALRECTFILE exists
    rm CalRect.ini
else
    echo $CALRECTFILE does not exist
fi

if ! wget -t 2 --user="$USER" --password="$PASSWD" "ftp://$host/$CALRECTFILE"
then
    echo "Error downloading file; CalRect.ini is not available"
    exit
else
    echo "CalRect.ini file download Successful"
fi

#Download Face.ini File from ftp
if [ -f $FACECONFIGFILE ]
then
    echo $FACECONFIGFILE exists
    rm Face.ini
else
    echo $FACECONFIGFILE does not exist
fi

if ! wget -t 2 --user="$USER" --password="$PASSWD" "ftp://$host/$FACECONFIGFILE"
then
    echo "Error downloading file; Face.ini is not available"
    exit
else
    echo "Face.ini file download Successful"
fi

#Copy the downloaded file to data calibration folder
mv data/calibration/Face.ini data/calibration/Face.ini.Bkup
cp Face.ini /home/root/data/calibration
echo "*************************** Extftp script finished ***********************"
