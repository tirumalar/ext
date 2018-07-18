#!/bin/bash
targetFile=$1
# check the tar file consistency
if [ -f $targetFile ] && [ -f "$targetFile".md5 ]
then
    MD5A="`md5sum $targetFile | awk '{print $1}'`"
    MD5B="`awk '{print $1}' $targetFile.md5`"
    if [ "$MD5A" == "$MD5B" ]
    then
        echo 0
    else
        echo 1
    fi
else
  echo 2
fi
