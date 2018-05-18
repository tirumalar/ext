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

source "${EYELOCK_WS_EXT}/BuilderScripts/setEnv.sh"

${ECLIPSEPATH}/eclipse -Xmx2048M -data ${WORKSPACE}&
