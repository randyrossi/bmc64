#!/bin/bash

if [ -f sdcard/config.txt ]
then
echo Cleaning...
else
echo Must be run from BMC64 root dir.
exit
fi

SRC_DIR=`pwd`

cd $SRC_DIR/third_party/vice-3.3
make clean
git status | grep 'deleted:.*doc' | sed 's/deleted:/git checkout /' | sh

cd $SRC_DIR/third_party/common
make clean

cd $SRC_DIR/third_party/plus4emu
make clean

cd $SRC_DIR/third_party/circle-stdlib
find . -name 'config.cache' -exec rm -f {} \;
rm -rf build
make clean
make mrproper

cd $SRC_DIR/third_party/circle-stdlib/libs/circle/addon

cd fatfs
make clean
cd ..

cd linux
make clean
cd ..

cd vc4/vchiq
make clean
cd ..

cd interface/bcm_host
make clean
cd ..

cd khronos
make clean
cd ..

cd vcos
make clean
cd ..

cd vmcs_host
make clean
cd ..

cd $SRC_DIR/third_party/circle-stdlib
rm -rf build
git reset --hard

cd libs/circle-newlib
git reset --hard

cd ../circle
git reset --hard

cd ../mbedtls
git reset --hard

cd $SRC_DIR
make clean

rm kernel*img.*
