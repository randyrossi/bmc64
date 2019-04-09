#!/bin/bash

if [ "$1" = "pi2" ]
then
echo Making for pi2
elif [ "$1" = "pi3" ]
then
echo Making for pi3
else
echo Need arg pi2 or pi3
exit
fi

if [ -f config.txt ]
then
echo Making everything...
else
echo Must be run from BMC64 root dir.
fi

SRC_DIR=`pwd`

cd third_party/circle-stdlib

find . -name 'config.cache' -exec rm {} \;

echo ==============================================================
echo APPLY PATCHES
echo ==============================================================

cd libs/circle-newlib
patch -p1 < ../../../../circle_newlib_patch.diff

cd ../circle
patch -p1 < ../../../../circle_patch.diff


if [ "$1" = "pi2" ]
then
cd ../..
cat ../../circle_stdlib_patch.diff  | sed 's/-std=c++14//' | patch -p1
./configure -n --raspberrypi=2
elif [ "$1" = "pi3" ]
then
cd ../..
cat ../../circle_stdlib_patch.diff  | patch -p1
./configure -n --raspberrypi=3 --prefix=arm-linux-gnueabihf-
else
echo "I don't know what to do for $1"
exit
fi

make -j4

echo ==============================================================
echo BUILD ADDONS
echo ==============================================================

cd libs/circle/addon

cd fatfs
make clean
make
cd ..

cd linux
make clean
make
cd ..

cd vc4/vchiq
make clean
make 

cd ../../../../../../..
cd third_party/vice-3.3

if [ "$1" = "pi2" ]
then
cd src/resid
CIRCLE_HOME="$SRC/third_party/circle-stdlib" ARM_HOME="$HOME/gcc-arm-none-eabi-7-2018-q2-update" CXXFLAGS="-std=c++11 -funsafe-math-optimizations -fno-exceptions -fno-rtti -nostdinc++ -mfloat-abi=hard -ffreestanding -nostdlib -march=armv7-a -marm -mfpu=neon-vfpv4 --specs=nosys.specs -O2 -I$CIRCLE_HOME/install/arm-none-circle/include/ -I$ARM_HOME/lib/gcc/arm-none-eabi/7.3.1/include -I$ARM_HOME/lib/gcc/arm-none-eabi/7.3.1/include-fixed" LDFLAGS="-L$CIRCLE_HOME/install/arm-none-circle/lib" ./configure --host=arm-none-eabi
cd ../..

CIRCLE_HOME="$SRC_DIR/third_party/circle-stdlib" ARM_HOME="$HOME/gcc-arm-none-eabi-7-2018-q2-update" LDFLAGS="-L$CIRCLE_HOME/install/arm-none-circle/lib" CXXFLAGS="-std=c++11 -O3 -mfloat-abi=hard -ffreestanding -march=armv7-a -marm -mfpu=neon-vfpv4 -fno-exceptions -fno-rtti -nostdinc++ --specs=nosys.specs" CFLAGS="-O3 -I$CIRCLE_HOME/install/arm-none-circle/include/ -I$ARM_HOME/lib/gcc/arm-none-eabi/7.3.1/include -I$ARM_HOME/lib/gcc/arm-none-eabi/7.3.1/include-fixed -I$CIRCLE_HOME/libs/circle/addon/fatfs -fno-exceptions --specs=nosys.specs -mfloat-abi=hard -ffreestanding -nostdlib -march=armv7-a -marm -mfpu=neon-vfpv4 -nostdinc" ./configure --host=arm-none-eabi --disable-realdevice --disable-ipv6 --disable-ssi2001 --disable-catweasel --disable-hardsid --disable-parsid --disable-portaudio --disable-ahi --disable-bundle --disable-lame --disable-rs232 --disable-midi --disable-hidmgr --disable-hidutils --without-oss --without-alsa --without-pulse --without-zlib --disable-sdlui --disable-sdlui2 --enable-raspiui
else
CIRCLE_HOME="$SRC_DIR/third_party/circle-stdlib" ARM_HOME="$HOME/rpi-tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf" LDFLAGS="-L$CIRCLE_HOME/install/arm-none-circle/lib" CXXFLAGS="-O3 -std=c++11 -fno-exceptions -march=armv8-a -mtune=cortex-a53 -marm -mfpu=neon-fp-armv8 -mfloat-abi=hard -ffreestanding -nostdlib" CFLAGS="-O3 -I$CIRCLE_HOME/install/arm-none-circle/include/ -I$CIRCLE_HOME/libs/circle/addon/fatfs -I$ARM_HOME/lib/gcc/arm-linux-gnueabihf/4.9.3/include -I$ARM_HOME/lib/gcc/arm-linux-gnueabihf/4.9.3/include-fixed -fno-exceptions -march=armv8-a -mtune=cortex-a53 -marm -mfpu=neon-fp-armv8 -mfloat-abi=hard -ffreestanding -nostdlib" ./configure --host=arm-linux-gnueabihf --disable-realdevice --disable-ipv6 --disable-ssi2001 --disable-catweasel --disable-hardsid --disable-parsid --disable-portaudio --disable-ahi --disable-bundle --disable-lame --disable-rs232 --disable-midi --disable-hidmgr --disable-hidutils --without-oss --without-alsa --without-pulse --without-zlib --disable-sdlui --disable-sdlui2 --enable-raspiui
fi

cd src
make libarchdep
make libhvsc
cd ..
make x64
cd ../..
echo ==============================================================
echo Link errors above are expected
echo ==============================================================

make clean
make
