 #!/bin/bash -e
retval=0
CheckLogic(){
if [ "$1" -ne 0 ]
then
retval=$1
fi
}

#sed -r \"/.*Eyelock.TestImageLevel.\*$/d\" -i /mnt/mmc/Eyelock.ini && sed -r \"$ a\Eyelock.TestImageLevel=0\" -i /mnt/mmc/Eyelock.ini\n");
SetIniKey(){
	echo $1 should execute
	$1
	CheckLogic $?
	sync
}
SetIniKey $1

if [ "$retval" == 0 ]
then
echo "SUCCESS"
else
echo "FAILURE"
fi


