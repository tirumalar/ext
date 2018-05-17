#!/bin/bash

export PKG_PATH="${EYELOCK_WS_EXT}/linux/pkg"
export TUT_INC=${PKG_PATH}/tut-190-trunk/include
export ANT_HOME=${HOME}/cruisecontrol-bin-2.8.4/apache-ant-1.8.4

export STATIC_LIBS=${PKG_PATH}/lib/x86
export COMMON_INC=${PKG_PATH}/common/include

export EYELOCK_INC=${EYELOCK_WS_EXT}/linux/lib/Include
export EYELOCK_LIB=${EYELOCK_WS_EXT}/linux/lib/x86

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
export BOOST="${EYELOCK_3DPARTY}/boost/boost_1_53_0"
export LIBEVENT=${PKG_PATH}/libevent-2.0.21-stable
export ZLIB=${PKG_PATH}/zlib-1.2.8
#export FLYCAPTURE_INC=/usr/include/flycapture
#export FLYCAPTURE_LIBS=/usr/lib
#export CURL=${PKG_PATH}/libcurl-7.58.0

# for MultiChannelLogger
export POCO_BASE=${PKG_PATH}/poco-1.9.0
export OPENSSL_102N_BASE=${PKG_PATH}/openssl-1.0.2n

export ARCH_PROCESSOR="x86"

projects=(
BiOmega
EdgeImage
EyeDetection
MamigoBaseIPL
EyeSegmentationLib
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
icm_communicator
MultiChannelLogger
)

NANOSDK_PATH="${EYELOCK_WS_EXT}/NanoSDK"
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

"${EYELOCK_WS_EXT}/BuilderScripts/setVersion.sh"

for i in  ${projects[*]}
do
echo "Building ${i}/Release"
	"${ECLIPSE_CDT}/eclipse" --launcher.suppressErrors -data "${EYELOCK_WS_EXT}" -nosplash -application  org.eclipse.cdt.managedbuilder.core.headlessbuild -cleanBuild "${i}/Release"
done

