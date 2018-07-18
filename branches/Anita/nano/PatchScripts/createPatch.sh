#!/bin/bash

function runcmd {
  echo running cmd $1
  $1
}

if [ $# -lt 1 ]; then
    echo Number Arguments not proper $#
#    echo Argument1 is the key to encrypt(max 20 caracter key)
#    echo Argument2 is file/folder to be tared with install.sh
else	
    #runcmd "rm Patch*"
    runcmd "tar -cvf  Patch.tar install.sh $2"
    runcmd "gzip Patch.tar"
    runcmd "./KeyMgr -e -i Patch.tar.gz -o Patch.tar.gz.e -k $1"
    runcmd "mv Patch.tar.gz.e Patch.tar.gz"
    md5=($(md5sum Patch.tar.gz))
    echo $md5 > Patch.tar.gz.md5 
    runcmd "tar -cvf PatchGenerated.tar Patch.tar.gz Patch.xml Patch.tar.gz.md5"
fi

