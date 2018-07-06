#!/bin/sh

level="0"
if [ "$1" == "2" ]
then
    level="2"
fi

sed -i "s/\(GRI\.TestImageLevel\)=.*/\1=${level}/g" GRIDemo.ini
grep TestImageLevel GRIDemo.ini
