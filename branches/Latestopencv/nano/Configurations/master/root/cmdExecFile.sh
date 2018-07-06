
 sed -i -e "s/\(GRITrigger.F2FEnable\)=\(.*\)/\1=false/g" /home/root/Eyelock.ini 
retval=$?
if [ $retval -eq 0 ]; then
# return success
echo "2" > /home/root/executedCmdFile
cp /home/root/Eyelock.ini  /home/eyelock/Eyelock.ini
echo 0
else
# return failure
echo "2" > /home/root/executedCmdFile
echo -1
fi
 sed -i -e "s/\(GRITrigger.RelayEnable\)=\(.*\)/\1=true/g" /home/root/Eyelock.ini 
retval=$?
if [ $retval -eq 0 ]; then
# return success
echo "2" > /home/root/executedCmdFile
cp /home/root/Eyelock.ini  /home/eyelock/Eyelock.ini
echo 0
else
# return failure
echo "2" > /home/root/executedCmdFile
echo -1
fi
