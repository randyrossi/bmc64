#!/bin/bash

if [ -f config.txt ]
then
echo Cleaning...
else
echo Must be run from BMC64 root dir.
fi

SRC_DIR=`pwd`

cd $SRC_DIR/third_party/vice-3.2
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

git reset --hard
cd libs/circle-newlib
git reset --hard
cd ../circle
git reset --hard

cd $SRC_DIR
make clean
