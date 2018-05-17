#!/bin/bash

HEIGHT=15
WIDTH=50
CHOICE_HEIGHT=6
BACKTITLE="EyeLock CamTools for HBOX"
TITLE="Preview Window Mode"
MENU="Choose one of the following options:"

OPTIONS=(1 "Focus Camera 0"
         2 "Focus Camera 1"
         3 "Focus Camera 2"
         4 "Launch HBOX Preview")

CHOICE=$(dialog --clear \
                --backtitle "$BACKTITLE" \
                --title "$TITLE" \
                --menu "$MENU" \
                $HEIGHT $WIDTH $CHOICE_HEIGHT \
                "${OPTIONS[@]}" \
                2>&1 >/dev/tty)

clear
case $CHOICE in
        1)
			sudo sed -i 's/^Eyelock.CameraIndex=0/Eyelock.CameraIndex=0/g' ./Preview.ini
			sudo sed -i 's/^Eyelock.CameraIndex=1/Eyelock.CameraIndex=0/g' ./Preview.ini
			sudo sed -i 's/^Eyelock.CameraIndex=2/Eyelock.CameraIndex=0/g' ./Preview.ini
			sudo sed -i 's/^Eyelock.Focus=0/Eyelock.Focus=1/g' ./Preview.ini
			# Now that our ini file is setup...  Launch cvPreview
			sudo killall -9 cvPreview
			sleep 3
			#sudo /home/root/identityNkillEyelock.sh
			sudo /home/root/cvPreview
            ;;
        2)
			sudo sed -i 's/^Eyelock.CameraIndex=0/Eyelock.CameraIndex=1/g' ./Preview.ini
			sudo sed -i 's/^Eyelock.CameraIndex=1/Eyelock.CameraIndex=1/g' ./Preview.ini
			sudo sed -i 's/^Eyelock.CameraIndex=2/Eyelock.CameraIndex=1/g' ./Preview.ini
			sudo sed -i 's/^Eyelock.Focus=0/Eyelock.Focus=1/g' ./Preview.ini
			sudo killall -9 cvPreview
			sleep 3
			#sudo /home/root/identityNkillEyelock.sh
			sudo /home/root/cvPreview
            ;;
        3)
			sudo sed -i 's/^Eyelock.CameraIndex=2/Eyelock.CameraIndex=2/g' ./Preview.ini
			sudo sed -i 's/^Eyelock.CameraIndex=0/Eyelock.CameraIndex=2/g' ./Preview.ini
			sudo sed -i 's/^Eyelock.CameraIndex=1/Eyelock.CameraIndex=2/g' ./Preview.ini
			sudo sed -i 's/^Eyelock.Focus=0/Eyelock.Focus=1/g' ./Preview.ini
			sudo killall -9 cvPreview
			sleep 3
			#sudo /home/root/identityNkillEyelock.sh
			sudo /home/root/cvPreview
            ;;
        4)
			sudo sed -i 's/^Eyelock.Focus=1/Eyelock.Focus=0/g' ./Preview.ini
			sudo killall -9 cvPreview
			#sudo bash /home/root/startup.sh
			#sudo /home/root/cvPreview &
            ;;
       	
esac