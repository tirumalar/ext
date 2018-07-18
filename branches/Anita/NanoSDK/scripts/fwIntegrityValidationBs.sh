#!/bin/bash

readXml(){
	# no need to save initial IFS value. Tested on NXT and Ubuntu 14.04
	# The following can be used to print IFS:
	# printf %q "$IFS" 
	local IFS=\>
	
    read -d \< ENTITY CONTENT
    local ret=$?
    TAG_NAME=${ENTITY%% *}
    ATTRIBUTES=${ENTITY#* }
    return $ret
}

setFileMd5ToXml(){
	INPUTXML=$1
	FILE=$2
	MD5=$(md5sum "${FILE}" | awk '{print $1}')
	sed -i "s~\(<md5.* path=\"${FILE}.*>\)[^<>]*\(<\/md5.*\)~\1${MD5}\2~" ${INPUTXML}
}

fillCheckInfo(){
	INPUTXML=$1
	pushd $2
	declare -a FILES
	while readXml
	do
		if [[ $TAG_NAME = "md5" ]]
		then
			eval local $ATTRIBUTES
			setFileMd5ToXml ${INPUTXML} "$path"
		fi
	done < ${INPUTXML}
	popd
}

# Globals
VALIDATION_LIST_FILE=$1
echo "Target XML path: ${VALIDATION_LIST_FILE}"
BS_BASE_PATH=$2
echo "Base path: ${BS_BASE_PATH}"

fillCheckInfo ${VALIDATION_LIST_FILE} ${BS_BASE_PATH}

