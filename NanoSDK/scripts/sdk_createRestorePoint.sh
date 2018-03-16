set -e
# $1 = restore point archive name ( e.g. root_20150101_000000_0.00.000.tgz)

touch /home/createrestorepoint.txt

mkdir -p /home/firmware/nano/restorepoints

if [ -e /home/root/eyestartup ]
then
   cd /home; 
   if [ -e /home/eyelock/data ] 
   then
       cp -R /home/eyelock/data /home/root/
       sync;
   fi
   tar cf /home/firmware/nano/restorepoints/$1 root www 
else
   cd /home; 
   tar cf /home/firmware/nano/restorepoints/$1 root
fi

# SLAVE
ssh root@192.168.40.2 'mkdir -p /home/firmware/nano/restorepoints'
ssh root@192.168.40.2 "cd /home; tar cf /home/firmware/nano/restorepoints/$1 root"

rm /home/createrestorepoint.txt

echo "success"
