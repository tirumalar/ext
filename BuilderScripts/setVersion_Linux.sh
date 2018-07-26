#!/bin/bash

# changes version in EyelockMain.cpp and echoes it to standard output
# Major version and minor version are retrieved from ${EYELOCK_WS_EXTLINUX}/Eyelock/build.properties
# EYELOCK_WS_EXTLINUX is assumed to be set as environment variable
# revision number is retrieved from svn status output

FW_VER_MAJ_MIN=$(grep -Po 'majorVersion=\K.*' "${EYELOCK_WS_EXTLINUX}/Eyelock/build.properties")
REVISION=$(cd "${EYELOCK_WS_EXTLINUX}"; svn info | grep "Revision" | awk '{print $2}')
FW_VER="${FW_VER_MAJ_MIN}.${REVISION}"
awk -v ver="${FW_VER}" ' $0 ~ "^#define EYELOCK_VERSION \".*\"" { printf("#define EYELOCK_VERSION \"%s\"\n", ver); $0 = ""; } { print $0 }' "${EYELOCK_WS_EXTLINUX}/Eyelock/EyeLockMain.cpp" > "${EYELOCK_WS_EXTLINUX}/Eyelock/EyeLockMain.cpp.new"
mv "${EYELOCK_WS_EXTLINUX}/Eyelock/EyeLockMain.cpp.new" "${EYELOCK_WS_EXTLINUX}/Eyelock/EyeLockMain.cpp"
