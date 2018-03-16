set -e
# $1 = restore point archive name ( e.g. root_20150101_000000_0.00.000.tgz)

cd /home/firmware/nano/restorepoints/; rm $1

# SLAVE
ssh root@192.168.40.2 "cd /home/firmware/nano/restorepoints/; rm $1"

echo "success"