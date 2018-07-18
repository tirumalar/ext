#!/bin/sh
# Check if this file exists:
if test -e /home/www/msglog
then
echo "You have a config.file file"
else
echo "No such file config.file"
sudo touch /home/www/msglog
fi
echo "Finished with my 'if' test"
sudo chmod 666 /home/www/msglog
echo "script complete..."
