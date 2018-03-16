 cd $1
rm -r txtFile
mkdir txtFile
cp *log*  txtFile/
cp /var/log/messages*  txtFile/
cd txtFile
tar -cf masterArchieve.tar *
gzip -9 masterArchieve.tar
chmod  a+x *

