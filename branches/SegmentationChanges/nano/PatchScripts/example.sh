#!/bin/bash
echo this is an example to generate a patch file for device.
chmod a+x ./createPatch.sh
echo create adummy folder  testfolder/ABC/123
mkdir -p testfolder/ABC/123

echo encryption key is THISISTHEKEYTOENCRYPT
./createPatch.sh THISISTHEKEYTOENCRYPT testfolder

# PatchGenerated.tar will be created and use it to upload on device using web config tool.
# /etc/patchworking will be created on device(both master and slave)
# testfolder will be coped in hoe folder.
