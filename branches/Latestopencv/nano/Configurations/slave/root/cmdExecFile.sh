 sed -i -e "s/\(GRI.HDMatcher.0.Address\)=\(.*\)\(:[0-9]*\)/\1=nano1365-0.local:8081/g"  /home/root/Eyelock.ini 
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
 sed -i -e "s/\(GRI.HBDestAddress\)=\(.*\)\(:[0-9]*\)/\1=nano1365-0.local:8081/g"  /home/root/Eyelock.ini 
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
 sed -i -e "s/GRI.MatchResultDestAddr/;GRI.MatchResultDestAddr/"  /home/root/Eyelock.ini 
 sed -i -e "s/;*GRI.MatchResultDestAddr/;GRI.MatchResultDestAddr/"  /home/root/Eyelock.ini 
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
 sed -i -e "s/\(Eyelock.TSMasterDestAddr\)=\(.*\)/\1=nano1365-0.local:8081/g" /home/root/Eyelock.ini 
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
echo ";Keys For secure Communication" >> /home/root/Eyelock.ini
echo "should not be reachable" 
echo "should not be reachable" 
echo "should not be reachable" 
echo "should not be reachable" 
echo "should not be reachable" 
echo "should not be reachable" 
echo "should not be reachable" 
echo "should not be reachable" 
echo "should not be reachable" 
echo "should not be reachable" 
