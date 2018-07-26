#!/bin/bash
# this script removes the existing Eclipse workspace and creates the new one

echo "Removing the existing workspace"
rm -rf "${EYELOCK_WS_EXTLINUX}/.metadata"

projects=(
AppBase
BiOmega
EdgeImage
EyeDetection
Eyelock
EyelockConfigurationLib
EyeSegmentationLib
EyeTracking
FocusEyeSelector
MamigoBaseIPL
mamigoShared
NanoSDK
NwHDMatcher
nxtW
KeyMgr
icm_communicator_EXT
MultiChannelLogger
EXTFaceDetection/CLM
EXTFaceDetection/facedetect
EXTFaceDetection/FaceTracker
i2cHandler
)

echo "Importing projects"
for i in "${projects[@]}"
do
	echo **Importing ${i}
	"${ECLIPSE_CDT}/eclipse" --launcher.suppressErrors -data "${EYELOCK_WS_EXTLINUX}" -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -import "${EYELOCK_WS_EXTLINUX}/${i}"
	echo ${i} import returned $?
done

