#!/bin/bash -e
echo HOMEDIR  $HOME
export WORKSPACE="${HOME}/TESTWorkspace/trunk/FaceTracker" 
if [ "${HOME}/workspace/eclipse" ];
then
   export ECLIPSEPATH="${HOME}/workspace/eclipse"
   echo "NEON PRESENT"
else
	if [ -f "${HOME}/workspace/eclipse" ];
	then
	   export ECLIPSEPATH="${HOME}/workspace/eclipse"
	   echo "NEON PRESENT"
	else
	   export ECLIPSEPATH="${HOME}/workspace/eclipse_oxygen"
	   echo "OXYGEN PRESENT"
	fi
fi
echo $PATH

export PKG_PATH="${HOME}/TESTWorkspace/trunk/linux/pkg"
export OPENCV_INC=${PKG_PATH}/opencv/include
export FACE_INC=${WORKSPACE}/facetracker/include
export CLM_EIGEN_INC=${WORKSPACE}/CLM/Eigen
export CLM_INC=${WORKSPACE}/CLM/include

export OPENCV_LIBS=${PKG_PATH}/opencv/lib
export TRACKER_LIBS=${WORKSPACE}/lib

export ARCH_PROCESSOR="x86"


${ECLIPSEPATH}/eclipse -Xmx2048M -data ${WORKSPACE}&
