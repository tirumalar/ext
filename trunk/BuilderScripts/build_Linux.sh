#!/bin/bash

if [[ ${configuration} == "Release" ]]
then
	source "${EYELOCK_WS_EXTLINUX}/BuilderScripts/setEnv_Linux.sh"

elif [[ ${configuration} == "Release_OdroidC2" ]]
then

	source "${EYELOCK_WS_EXTLINUX}/BuilderScripts/setEnv_OdroidC2.sh"
fi

projects=(
log4cxx
EyelockLoggingLib
AusSegmentLib
BiOmega
EdgeImage
EyeDetection
MamigoBaseIPL
EyeSegmentationLib
EyeSortingLib
HttpPostSender
ISOBiometricCodecLib
EyeTracking
FocusEyeSelector
mamigoShared
AppBase
NwHDMatcher
EyelockConfigurationLib
NanoSDK
Eyelock
nxtW
KeyMgr
icm_communicator_EXT
MultiChannelLogger
FaceTracker
i2cHandler
)

NANOSDK_PATH="${EYELOCK_WS_EXTLINUX}/NanoSDK"
rm "${NANOSDK_PATH}/include/EyelockNanoDevice.h"
rm "${NANOSDK_PATH}/include/NanoDevice_constants.h"
rm "${NANOSDK_PATH}/include/NanoDevice_types.h"
rm "${NANOSDK_PATH}/EyelockNanoDevice.cpp"
rm "${NANOSDK_PATH}/NanoDevice_constants.cpp"
rm "${NANOSDK_PATH}/NanoDevice_types.cpp"

THRIFT="${THRIFT_BASE}/bin/thrift"
#chmod +x "${THRIFT}"
if "${THRIFT}" --gen cpp -out ${NANOSDK_PATH} "${NANOSDK_PATH}/NanoDevice.thrift"
then
	echo "thrift file compilation succesful"
	mv "${NANOSDK_PATH}/EyelockNanoDevice.h" "${NANOSDK_PATH}/include/"
	mv "${NANOSDK_PATH}/NanoDevice_constants.h" "${NANOSDK_PATH}/include/"
	mv "${NANOSDK_PATH}/NanoDevice_types.h" "${NANOSDK_PATH}/include/"
else
	echo "Error: thrift file compilation failed"
	exit 1
fi

"${EYELOCK_WS_EXTLINUX}/BuilderScripts/setVersion_Linux.sh"

for i in  ${projects[*]}
do
echo "Building ${i}/${configuration}"
	"${ECLIPSE_CDT}/eclipse" --launcher.suppressErrors -data "${EYELOCK_WS_EXTLINUX}" -nosplash -application  org.eclipse.cdt.managedbuilder.core.headlessbuild -cleanBuild "${i}/${configuration}" -vmargs -Xms1024M -Xmx2048M
done

