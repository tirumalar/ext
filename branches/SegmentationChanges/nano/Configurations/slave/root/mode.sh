#!/bin/sh

mode="false"
if [ "$1" == "true" ]
then
    mode="true"
fi

sed -i "s/\(GRI\.shouldDetectEye\)=.*/\1=${mode}/g" GRIDemo.ini
grep shouldDetectEye GRIDemo.ini
