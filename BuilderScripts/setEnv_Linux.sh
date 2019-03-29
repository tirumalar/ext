#!/bin/bash



export PKG_PATH="${EYELOCK_WS_EXTLINUX}/linux/pkg"
export TUT_INC=${PKG_PATH}/tut-190-trunk/include
export ANT_HOME=${HOME}/cruisecontrol-bin-2.8.4/apache-ant-1.8.4

export STATIC_LIBS=${PKG_PATH}/lib/x86
export COMMON_INC=${PKG_PATH}/common/include

export EYELOCK_INC=${EYELOCK_WS_EXTLINUX}/linux/lib/Include
export EYELOCK_LIB=${EYELOCK_WS_EXTLINUX}/linux/lib/Release

export ALSA_INC=${STATIC_LIBS}
export ALSA_LIB=${STATIC_LIBS}/alsa/lib

export OPENSSL_INC=${PKG_PATH}/openssl-1.0.1i/include
export OPENSSL_ASN_INC=${PKG_PATH}/openssl-1.0.1i/crypto/asn1
#export OPENCV_INC=${PKG_PATH}/cv/include
#export CXCORE_INC=${PKG_PATH}/cxcore/include
#export HIGHGUI_INC=${PKG_PATH}/highgui/include
export APR_INC=${PKG_PATH}/apr-1.4.8/include
export APR_UTIL_INC=${PKG_PATH}/apr-util-1.5.2/include
export LOG4CXX_INC=${PKG_PATH}/apache-log4cxx-0.10.0/src/main/include

export THRIFT_BASE=${PKG_PATH}/thrift-0.9.1
export SQLITE_BASE=${PKG_PATH}/sqlite3
export SOCI_BASE=${PKG_PATH}/soci-3.2.1
export BOOST="${EYELOCK_3DPARTY}/boost/boost_1_53_0"
export LIBEVENT=${PKG_PATH}/libevent-2.0.21-stable
export ZLIB=${PKG_PATH}/zlib-1.2.8
export OPEN_JPEG_INC=${PKG_PATH}/openjpeg23/include

# for MultiChannelLogger
export POCO_BASE=${PKG_PATH}/poco-1.9.0
export OPENSSL_102N_BASE=${PKG_PATH}/openssl-1.0.2n

#for FaceTracker
#export FACE_INC=${EYELOCK_WS_EXTLINUX}/EXTFaceDetection/facedetect/include
#export CLM_EIGEN_INC=${EYELOCK_WS_EXTLINUX}/EXTFaceDetection/CLM/Eigen
#export CLM_INC=${EYELOCK_WS_EXTLINUX}/EXTFaceDetection/CLM/include
export OPENCV_NEW_INC=${PKG_PATH}/opencv3.4/include
export OPENCV_NEW_LIBS=${PKG_PATH}/opencv3.4/lib
export OPEN_JPEG_LIB=${PKG_PATH}/openjpeg23/lib

#for OIMCalibration
export ARUCO_INC=${PKG_PATH}/opencv3.4/aruco/include/aruco
export ARUCO_LIBS=${PKG_PATH}/opencv3.4/aruco/lib

export ARCH_PROCESSOR="x86"
