#!/bin/bash

eval `scramv1 ru -sh`

COLLECTOR_NODE=$1

HOSTNAME=$(echo `/bin/hostname` | sed 's/\//\\\//g')

TEST_PATH=$(echo "${PWD}" | sed 's/\//\\\//g')

MWC_LIB1="${LOCALRT}/lib/${SCRAM_ARCH}/pluginDQMSiPixelMonitorClient.so"
if [ ! -f $MWC_LIB1 ]; then
    echo "Library not found! Will pick it up from the release area..."
    
MWC_LIB1="${CMSSW_RELEASE_BASE}/lib/${SCRAM_ARCH}/pluginDQMSiPixelMonitorClient.so"
else 
    echo "Library found!"
fi

MWC_LIB=$(echo "$MWC_LIB1" | sed 's/\//\\\//g')
echo $MWC_LIB1

SERVED_DIR="http://${HOSTNAME}:1972/temporary"

if [ -e profile.xml ]; then
    rm -f profile.xml
fi 
if [ -e monClient.xml ]; then
    rm -f SiPixelWebClient.xml
fi
if [ -e startMonitorClient ]; then
    rm -f startMonitorClient
fi

sed -e "s/.portn/1972/g" -e "s/.host/${HOSTNAME}/g" -e "s/.pwd/${TEST_PATH}/g" -e "s/.libpath/${MWC_LIB}/g" .profile.xml > profile.xml
sed -e "s/.portn/1972/g" -e "s/.host/${HOSTNAME}/g" -e "s/.pwd/${TEST_PATH}/g" -e "s/.libpath/${MWC_LIB}/g" -e "s/.collector/${COLLECTOR_NODE}/g" .SiPixelWebClient.xml > SiPixelWebClient.xml 
sed -e "s/.portn/1972/g" -e "s/.host/${HOSTNAME}/g" -e "s/.pwd/${TEST_PATH}/g" -e "s/.libpath/${MWC_LIB}/g" .startMonitorClient > startMonitorClient

sed -e "s@SERVED_DIRECTORY_URL@${SERVED_DIR}@g" .WebLib.js > WebLib.js
sed -e "s@.host@${HOSTNAME}@g" .trackermap.txt > trackermap.txt

chmod 751 profile.xml
chmod 751 SiPixelWebClient.xml
chmod 751 startMonitorClient



