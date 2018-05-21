#!/bin/bash

export ARM_INC="/opt/gcc-linaro-aarch64-linux-gnu-4.9-2014.09_linux/aarch64-linux-gnu/libc/usr/include"
export ARM_LIBS="/opt/gcc-linaro-aarch64-linux-gnu-4.9-2014.09_linux/aarch64-linux-gnu/libc/usr/lib/aarch64-linux-gnu"

export PATH=~/odroid_cross_bin:$PATH
echo $PATH

export PKG_PATH="${EYELOCK_WS_EXT}/linux/pkg"
export TUT_INC=${PKG_PATH}/tut-190-trunk/include
export ANT_HOME=${HOME}/cruisecontrol-bin-2.8.4/apache-ant-1.8.4

export STATIC_LIBS=${PKG_PATH}/lib/odroidc2_Release
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

# for MultiChannelLogger
export POCO_BASE=${PKG_PATH}/poco-1.9.0
export OPENSSL_102N_BASE=${PKG_PATH}/openssl-1.0.2n

#for test_aruc
export FACE_INC=${EYELOCK_WS_EXT}/FaceTracker/facetracker/include
export CLM_EIGEN_INC=${EYELOCK_WS_EXT}/FaceTracker/CLM/Eigen
export CLM_INC=${EYELOCK_WS_EXT}/FaceTracker/CLM/include
export OPENCV_NEW_INC=${PKG_PATH}/opencv3.4/include
export OPENCV_NEW_LIBS=${PKG_PATH}/opencv3.4/lib

export ARCH_PROCESSOR="_ARM_"
