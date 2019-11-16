#!/bin/bash

echo "starting lighttpd webserver"
rm /tmp/php*
rm /tmp/sess*
echo " launching webserver "
chmod a+x /home/php
cp /home/www/php.ini /lib
cp /home/php /usr/bin
#we need permissions on home for www for upgrade
/bin/chown -R www:www /home
LD_LIBRARY_PATH=/home/www/ssllibs
export LD_LIBRARY_PATH
/home/root/lighttpd -D -f /home/www/lighttpd.conf -m /home/root/lighttpdlib&

echo "******************************webserver script finished ************************"

