#!/bin/bash

# EYELOCK_WS_EXT is assumed to be set as environment variable

# -----------------------------------------------------------------------------------------------------------
# initializations

TARGET_DIR="${EYELOCK_WS_EXT}/dist/EyelockExtFW"
if [[ -d ${TARGET_DIR} ]]
then
	rm -rf "${TARGET_DIR}"
fi
mkdir "${TARGET_DIR}"

# versions
FW_VER=$(awk ' $0 ~ "^#define EYELOCK_VERSION \".*\"" {split($0, parts, "\""); printf("%s\n",parts[2])} ' "${EYELOCK_WS_EXT}/Eyelock/EyeLockMain.cpp")
ICM_VER=$(cat "${EYELOCK_WS_EXT}/ICMBinary/icmversion.txt" | tr -d '\n\r\t ')

FPGA_VER=$(cat "${EYELOCK_WS_EXT}/OIMBinaries/versions.txt" | grep 'FPGA VERSION' | cut -d':' -f2)
FIXED_BRD_VER=$(cat "${EYELOCK_WS_EXT}/OIMBinaries/versions.txt" | grep 'Fixed board Verson' | cut -d':' -f2)
CAM_BRD_VER=$(cat "${EYELOCK_WS_EXT}/OIMBinaries/versions.txt" | grep 'Cam Psoc Version' | cut -d':' -f2)

ICM_FILE="nanoExt_ICM_v${ICM_VER}.cyacd"

FPGA_FILE="nanoExt_FPGA_v${FPGA_VER}.bin"
FIXED_BRD_FILE="nanoExt_FixedBoard_v${FIXED_BRD_VER}.cyacd"
CAM_BRD_FILE="nanoExt_CameraBoard_v${CAM_BRD_VER}.cyacd"

if [[ ${configuration} == "Release" ]]
then
	NAME_POSTFIX=""

elif [[ ${configuration} == "Release_OdroidC2" ]]
then

	NAME_POSTFIX="_Linux"
fi

FW_FILE="EyelockExt_v${FW_VER}_ICM_v${ICM_VER}${NAME_POSTFIX}.tar"

BOARD_FILE="EyelockExt_v${FW_VER}.tar.gz"

ENCRYPTER="/home/ubuntu/KeyMgr"
chmod +x "${ENCRYPTER}"

KEY_FILE='/home/ubuntu/BSkey.pem'

# -----------------------------------------------------------------------------------------------------------
# edit the VersionInfo.xml
XML_FILE='NanoEXTVersionInfo.xml'
XML_FILE_PATH="${TARGET_DIR}/${XML_FILE}"

cp "${EYELOCK_WS_EXT}/Eyelock/AppTemplate.xml" "${XML_FILE_PATH}"
NOW=$(date)
sed -i "s/@@date@@/${NOW}/" "${XML_FILE_PATH}"
sed -i "s/@@version@@/${FW_VER}/" "${XML_FILE_PATH}"
sed -i "s/@@bobversion@@/${ICM_VER}/" "${XML_FILE_PATH}"
sed -i "s/@@tarfilename@@/${FW_FILE}/" "${XML_FILE_PATH}"
sed -i "s/@@ICMFilename@@/${ICM_FILE}/" "${XML_FILE_PATH}"

sed -i "s/@@fpgaversion@@/${FPGA_VER}/" "${XML_FILE_PATH}"
sed -i "s/@@fixedbrdversion@@/${FIXED_BRD_VER}/" "${XML_FILE_PATH}"
sed -i "s/@@cambrdversion@@/${CAM_BRD_VER}/" "${XML_FILE_PATH}"

sed -i "s/@@fpgafilename@@/${FPGA_FILE}/" "${XML_FILE_PATH}"
sed -i "s/@@fixedbrdfilename@@/${FIXED_BRD_FILE}/" "${XML_FILE_PATH}"
sed -i "s/@@cambrdfilename@@/${CAM_BRD_FILE}/" "${XML_FILE_PATH}"

# temporary workaround for upgrade via SDK (client side requires specific xml file in tar)
XML_FILE_LEGACY='NanoNXTVersionInfo.xml'
XML_FILE_PATH_LEGACY="${TARGET_DIR}/${XML_FILE_LEGACY}" 
cp "${XML_FILE_PATH}" "${XML_FILE_PATH_LEGACY}"
BOARD_FILE_LEGACY="EyelockExt_v${FW_VER}_Master.tar.gz"
sed -i "s/@@MasterFilename@@/${BOARD_FILE_LEGACY}/" "${XML_FILE_PATH_LEGACY}"

sed -i "s/@@MasterFilename@@/${BOARD_FILE}/" "${XML_FILE_PATH}"

# -----------------------------------------------------------------------------------------------------------
BOARD_DIR="${TARGET_DIR}/board" # analog of "master" and "slave" on the NXT for EXT (being single board device)
mkdir "${BOARD_DIR}"

# root
ROOT_DIR="${BOARD_DIR}/root" 
mkdir "${ROOT_DIR}"
rsync -r "${EYELOCK_WS_EXT}/nano/Configurations/master/root/" "${ROOT_DIR}"
rsync -r "${EYELOCK_WS_EXT}/nano/Configurations/both/root/" "${ROOT_DIR}"

mkdir "${ROOT_DIR}/scripts"
rsync -r  "${EYELOCK_WS_EXT}/Eyelock/data/Scripts/" "${ROOT_DIR}/scripts"
rsync -r --exclude='sdk_upgradeSlaveFirmware.sh' "${EYELOCK_WS_EXT}/NanoSDK/scripts/" "${ROOT_DIR}/scripts"

cp "${EYELOCK_WS_EXT}/Eyelock/data/keys.db" "${ROOT_DIR}"
cp "${EYELOCK_WS_EXT}/linux/bin/Release/Eyelock" "${ROOT_DIR}"
cp "${EYELOCK_WS_EXT}/linux/bin/Release/FaceTracker" "${ROOT_DIR}"
cp "${EYELOCK_WS_EXT}/linux/bin/Release/ExtCapture" "${ROOT_DIR}"

cp "${EYELOCK_WS_EXT}/linux/bin/Release/KeyMgr" "${ROOT_DIR}"
cp "${EYELOCK_WS_EXT}/icm_communicator_EXT/icm_communicator" "${ROOT_DIR}"
cp "${EYELOCK_WS_EXT}/linux/bin/Release/i2cHandler" "${ROOT_DIR}"

cp "${EYELOCK_WS_EXT}/NanoSDK/scripts/fwHandler.sh" "${ROOT_DIR}"

cp "${EYELOCK_WS_EXT}/MultiChannelLogger/Release/MultiChannelLogger" "${ROOT_DIR}"
cp "${EYELOCK_WS_EXT}/MultiChannelLogger/MultiChannelLoggerSettings.xml" "${ROOT_DIR}"

mkdir "${ROOT_DIR}/packages_updates"
readarray -t packages < <(sed '/^\s*$/d' "${EYELOCK_WS_EXT}/firmware/packages_to_update.txt")
for file in "${packages[@]}"
do
    cp "${EYELOCK_WS_EXT}/packages_updates/$file" "${ROOT_DIR}/packages_updates"
done

# default
rsync -r "${EYELOCK_WS_EXT}/nano/Configurations/master/default" "${BOARD_DIR}"
cp "${ROOT_DIR}/Eyelock.ini" "${BOARD_DIR}/default"
cp "${ROOT_DIR}/keys.db" "${BOARD_DIR}/default"

# www
WWW_DIR="${BOARD_DIR}/www"
mkdir "${WWW_DIR}"
WWW_REPO_PATH="${EYELOCK_WS_EXT}/nano/Configurations/master/www"
rsync -r --exclude="php_binary" --exclude="ssllibs" --exclude="nxtW" "${WWW_REPO_PATH}/" "${WWW_DIR}"
cp "${EYELOCK_WS_EXT}/nxtW/Release/nxtW" "${WWW_DIR}"

# compressing
tar -czf "${TARGET_DIR}/${BOARD_FILE}" -C "${BOARD_DIR}/" 'root' 'www' 'default'

md5sum "${TARGET_DIR}/${BOARD_FILE}" | awk ' { print $1 } ' > "${TARGET_DIR}/${BOARD_FILE}.md5"

rm -r "${BOARD_DIR}"

# -----------------------------------------------------------------------------------------------------------
# fwHandler
FWHANDLER_FILE='fwHandler.tar.gz'

mkdir "${TARGET_DIR}/fwHandler"
cp "${EYELOCK_WS_EXT}/NanoSDK/scripts/fwHandler.sh" "${TARGET_DIR}/fwHandler/"
cp "${EYELOCK_WS_EXT}/MultiChannelLogger/Release/MultiChannelLogger" "${TARGET_DIR}/fwHandler/"
cp "${EYELOCK_WS_EXT}/MultiChannelLogger/MultiChannelLoggerSettings.xml" "${TARGET_DIR}/fwHandler/"

cp "${XML_FILE_PATH}" "${TARGET_DIR}/fwHandler"
# TODO: fill and add fwIntegrityValidationList.xml

tar -czf "${TARGET_DIR}/${FWHANDLER_FILE}" -C "${TARGET_DIR}/" 'fwHandler'

# signing
openssl dgst -sha256 -sign "${KEY_FILE}" -out "${TARGET_DIR}/${FWHANDLER_FILE}.sig" "${TARGET_DIR}/${FWHANDLER_FILE}"

# -----------------------------------------------------------------------------------------------------------
cp "${EYELOCK_WS_EXT}/ICMBinary/ext_pim.cyacd" "${TARGET_DIR}/${ICM_FILE}"

cp "${EYELOCK_WS_EXT}/OIMBinaries/output16_18.bin" "${TARGET_DIR}/${FPGA_FILE}"
cp "${EYELOCK_WS_EXT}/OIMBinaries/fixed.cyacd" "${TARGET_DIR}/${FIXED_BRD_FILE}"
cp "${EYELOCK_WS_EXT}/OIMBinaries/camera.cyacd" "${TARGET_DIR}/${CAM_BRD_FILE}"

tar -cf "${EYELOCK_WS_EXT}/dist/${FW_FILE}" -C "${TARGET_DIR}" "${BOARD_FILE}" "${BOARD_FILE}.md5" "${ICM_FILE}" "${FPGA_FILE}" "${FIXED_BRD_FILE}" "${CAM_BRD_FILE}" "${FWHANDLER_FILE}" "${FWHANDLER_FILE}.sig" "${XML_FILE}" "${XML_FILE_LEGACY}"

# -----------------------------------------------------------------------------------------------------------
# encrypted version

"${ENCRYPTER}" -e -i "${TARGET_DIR}/${BOARD_FILE}" -o "${TARGET_DIR}/tmp" -k "${FW_VER}"
mv "${TARGET_DIR}/tmp" "${TARGET_DIR}/${BOARD_FILE}"
md5sum "${TARGET_DIR}/${BOARD_FILE}" | awk ' { print $1 } ' > "${TARGET_DIR}/${BOARD_FILE}.md5"

"${ENCRYPTER}" -e -i "${TARGET_DIR}/${FWHANDLER_FILE}" -o "${TARGET_DIR}/tmp" -k "${FW_VER}"
mv "${TARGET_DIR}/tmp" "${TARGET_DIR}/${FWHANDLER_FILE}"
openssl dgst -sha256 -sign "${KEY_FILE}" -out "${TARGET_DIR}/${FWHANDLER_FILE}.sig" "${TARGET_DIR}/${FWHANDLER_FILE}"

tar -cf "${EYELOCK_WS_EXT}/dist/${FW_FILE}.enc" -C "${TARGET_DIR}" "${BOARD_FILE}" "${BOARD_FILE}.md5" "${ICM_FILE}" "${FWHANDLER_FILE}" "${FWHANDLER_FILE}.sig" "${XML_FILE}" "${XML_FILE_LEGACY}"

rm -r "${TARGET_DIR}"
# -----------------------------------------------------------------------------------------------------------


