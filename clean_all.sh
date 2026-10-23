#!/bin/bash
#
# clean_all.sh
#
# BMC64 - A bare metal Commodore emulator for Raspberry Pi
# Copyright (C) 2019 Randy Rossi <randy.rossi@gmail.com>
# Copyright (C) 2026 aminch <https://github.com/aminch>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

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
# The configured VICE clean target can stop at an unavailable optional
# subdirectory, leaving objects from a prior Pi target behind.
find . -type f \( -name '*.o' -o -name '*.a' -o -name '*.lo' \) -delete
find . -type d \( -name '.deps' -o -name '.libs' \) -prune -exec rm -rf {} +
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

rm -f kernel*img.*
