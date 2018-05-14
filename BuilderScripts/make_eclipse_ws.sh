#!/bin/bash
# this script removes the existing Eclipse workspace and creates the new one

echo "Removing the existing workspace"
rm -rf "${EYELOCK_WS_EXT}/.metadata"

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
icm_communicator
)

echo "Importing projects"
for i in "${projects[@]}"
do
	echo **Importing ${i}
	"${ECLIPSE_CDT}/eclipse" --launcher.suppressErrors -data "${EYELOCK_WS_EXT}" -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -import "${EYELOCK_WS_EXT}/${i}"
	echo ${i} import returned $?
done

