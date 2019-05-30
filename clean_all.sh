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

cd $SRC_DIR/third_party/circle-stdlib
find . -name 'config.cache' -exec rm {} \;

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

cd $SRC_DIR/third_party/circle-stdlib

rm -rf build

git reset --hard
cd libs/circle-newlib
git reset --hard
cd ../circle
git reset --hard

cd $SRC_DIR
make clean
