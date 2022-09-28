#!/bin/bash

JAVA_HOME_BASH="/snap/vlc/2344/usr/lib/jvm/java-8-openjdk-amd64"
NATIVE_ENABLE_BASH=ON

if [ -z $ROOT_PATH ]; then
    ROOT_PATH=$(cd `dirname $0`; pwd)/..
    echo ${ROOT_PATH}
fi
rm -rf ${ROOT_PATH}/build-linux
rm -rf ${ROOT_PATH}/release-linux
mkdir ${ROOT_PATH}/build-linux
mkdir ${ROOT_PATH}/release-linux
cd ${ROOT_PATH}/build-linux

cmake ${ROOT_PATH}\
    -DLINUX=ON\
    -DNATIVE_ENABLE=${NATIVE_ENABLE_BASH}\
    -DJAVA_HOME=${JAVA_HOME_BASH}

make -j6

if [ "${NATIVE_ENABLE_BASH}" = "ON" ]; then
  cp ${ROOT_PATH}/build-linux/libfaceManager-jni.so ${ROOT_PATH}/release-linux
else
  cp ${ROOT_PATH}/build-linux/libfaceManager.so ${ROOT_PATH}/release-linux
  cp -r ${ROOT_PATH}/include ${ROOT_PATH}/release-linux
fi



JAVA_CODE_PATH=${ROOT_PATH}/modules/faceManager-jni





