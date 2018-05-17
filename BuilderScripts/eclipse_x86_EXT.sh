#!/bin/bash -e
echo HOMEDIR  $HOME
export WORKSPACE="${HOME}/HBox/EXT/trunk" 
if [ -f "${HOME}/eclipse/eclipse" ];
then
   export ECLIPSEPATH="${HOME}/eclipse"
   echo "Luna PRESENT"
else
	if [ -f "${HOME}/eclipse_Kepler/eclipse" ];
	then
	   export ECLIPSEPATH="${HOME}/eclipse_Kepler"
	   echo "KEPLER PRESENT"
	else
	   export ECLIPSEPATH="${HOME}/HBox/eclipse"
	   echo "JUNO PRESENT"
	fi
fi
echo $PATH


export PKG_PATH="${WORKSPACE}/linux/pkg"
export TUT_INC=${PKG_PATH}/tut-190-trunk/include
export ANT_HOME=${HOME}/cruisecontrol-bin-2.8.4/apache-ant-1.8.4

export STATIC_LIBS=${PKG_PATH}/lib/x86
export COMMON_INC=${PKG_PATH}/common/include

export EYELOCK_INC=${WORKSPACE}/linux/lib/Include
export EYELOCK_LIB=${WORKSPACE}/linux/lib/x86

export OPENSSL_INC=${PKG_PATH}/openssl-1.0.1i/include
export OPENSSL_ASN_INC=${PKG_PATH}/openssl-1.0.1i/crypto/asn1
export OPENCV_INC=${PKG_PATH}/cv/include
export CXCORE_INC=${PKG_PATH}/cxcore/include
export HIGHGUI_INC=${PKG_PATH}/highgui/include
export APR_INC=${PKG_PATH}/apr-1.4.8/include
export APR_UTIL_INC=${PKG_PATH}/apr-util-1.5.2/include
export LOG4CXX_INC=${PKG_PATH}/apache-log4cxx-0.10.0/src/main/include

export THRIFT_BASE=${PKG_PATH}/thrift-0.9.1
export SQLITE_BASE=${PKG_PATH}/sqlite3
export SOCI_BASE=${PKG_PATH}/soci-3.2.1
export BOOST=${PKG_PATH}/boost_1_53_0
export LIBEVENT=${PKG_PATH}/libevent-2.0.21-stable
export ZLIB=${PKG_PATH}/zlib-1.2.8
export FLYCAPTURE_INC=/usr/include/flycapture
export FLYCAPTURE_LIBS=/usr/lib
export COOLRUNNER_INC=${WORKSPACE}/SafeFlight/include

#test_aruc
export FACE_INC=${WORKSPACE}/FaceTracker/facetracker/include
export CLM_EIGEN_INC=${WORKSPACE}/FaceTracker/CLM/Eigen
export CLM_INC=${WORKSPACE}/FaceTracker/CLM/include
export OPENCV_NEW_INC=${PKG_PATH}/opencv3.4/include
export OPENCV_NEW_LIBS=${PKG_PATH}/opencv3.4/lib

export ARCH_PROCESSOR="x86"


${ECLIPSEPATH}/eclipse -Xmx2048M -data ${WORKSPACE}&
