#!/bin/bash
#sudo chmod a+x /home/root/cvPreview
sudo touch cvPreview.run
while true; do if [ -f /home/root/cvPreview.run ]; then sudo /home/root/cvPreview; fi; sleep 6; done