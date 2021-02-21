#!/bin/bash


# Set directory variables
SRC_DIR=`pwd`
CIRCLE_HOME="$SRC_DIR/third_party/circle-stdlib"
COMMON_HOME="$SRC_DIR/third_party/common"
if [ ! -n "$ARM_HOME" ]; then
	ARM_HOME="$HOME/gcc-arm-none-eabi-7-2018-q2-update"
fi
if [ ! -n "$ARM_VERSION" ]; then
	ARM_VERSION="7.3.1"
fi


BOARD=$1

if [ "$BOARD" = "pi2" ]
then
echo Making for pi2
elif [ "$BOARD" = "pi3" ]
then
echo Making for pi3
elif [ "$BOARD" = "pi4" ]
then
echo Making for pi4
elif [ "$BOARD" = "pi0" ]
then
echo Making for pi0
else
echo Need arg [pi0|pi2|pi3|pi4]
exit
fi

if [ -f sdcard/config.txt ]
then
echo Making everything...
else
echo Must be run from BMC64 root dir.
exit
fi



cd $SRC_DIR/third_party/circle-stdlib

find . -name 'config.cache' -exec rm {} \;

echo ==============================================================
echo APPLY PATCHES
echo ==============================================================

cd $SRC_DIR/third_party/circle-stdlib/libs/circle-newlib
patch -p1 < ../../../../circle_newlib_patch.diff

cd $SRC_DIR/third_party/circle-stdlib/libs/circle

if [ "$BOARD" = "pi0" ]
then
cat ../../../../circle_patch.diff | sed 's@+#define ARM_ALLOW_MULTI_CORE@+//#define ARM_ALLOW_MULTI_CORE@' | patch -p1
perl -pi -e 's@#define USE_PHYSICAL_COUNTER@//#define USE_PHYSICAL_COUNTER@' ./include/circle/sysconfig.h
perl -pi -e 's@//#define SAVE_VFP_REGS_ON_IRQ@#define SAVE_VFP_REGS_ON_IRQ@' ./include/circle/sysconfig.h
else
patch -p1 < ../../../../circle_patch.diff
fi

echo ==============================================================
echo BUILD CIRCLE-STDLIB
echo $PATH
echo ==============================================================

cd $SRC_DIR/third_party/circle-stdlib

if [ "$BOARD" = "pi2" ]
then
cat ../../circle_stdlib_patch.diff  | sed 's/-std=c++14//' | patch -p1
./configure --raspberrypi=2
elif [ "$BOARD" = "pi0" ]
then
cat ../../circle_stdlib_patch.diff | patch -p1
./configure --raspberrypi=1
elif [ "$BOARD" = "pi3" ]
then
cat ../../circle_stdlib_patch.diff  | patch -p1
./configure --raspberrypi=3
elif [ "$BOARD" = "pi4" ]
then
cat ../../circle_stdlib_patch.diff  | patch -p1
./configure --raspberrypi=4
else
echo "I don't know what to do for $BOARD"
exit
fi

# For pi0, we turn on our HID report throttle
if [ "$BOARD" = "pi0" ]
then
CFLAGS=-DBMC64_REPORT_THROTTLE make -j4
else
make -j4
if [ "$?" != "0" ]
then
       exit
fi
fi

echo ==============================================================
echo BUILD ADDONS
echo ==============================================================

cd $SRC_DIR/third_party/circle-stdlib/libs/circle/addon/fatfs
make clean
make
if [ "$?" != "0" ]
then
       exit
fi

cd $SRC_DIR/third_party/circle-stdlib/libs/circle/addon/linux
make clean
make
if [ "$?" != "0" ]
then
       exit
fi

cd $SRC_DIR/third_party/circle-stdlib/libs/circle/addon/vc4/vchiq
make clean
make 
if [ "$?" != "0" ]
then
       exit
fi

cd $SRC_DIR/third_party/circle-stdlib/libs/circle/addon/vc4/interface/bcm_host
make
if [ "$?" != "0" ]
then
       exit
fi

cd $SRC_DIR/third_party/circle-stdlib/libs/circle/addon/vc4/interface/khronos
make
if [ "$?" != "0" ]
then
       exit
fi

cd $SRC_DIR/third_party/circle-stdlib/libs/circle/addon/vc4/interface/vmcs_host
make
if [ "$?" != "0" ]
then
       exit
fi

cd $SRC_DIR/third_party/circle-stdlib/libs/circle/addon/vc4/interface/vcos
make
if [ "$?" != "0" ]
then
       exit
fi

# Common
cd $SRC_DIR/third_party/common
BOARD=$BOARD make

# Plus4Emu
cd $SRC_DIR/third_party/plus4emu
make
if [ "$?" != "0" ]
then
       exit
fi

# Vice
cd $SRC_DIR/third_party/vice-3.3

if [ "$BOARD" = "pi0" ]
then
# We have to configure resid even though we don't link it for pi0 to
# keep top level make happy. Too much of a hassle to fix configure.
DIRS="src/resid src/teensy-resid"
for d in $DIRS
do
cd $d
CXXFLAGS="-std=c++11 -funsafe-math-optimizations -fno-exceptions -fno-rtti -nostdinc++ -ffreestanding -nostdlib -DAARCH=32 -march=armv6k -mtune=arm1176jzf-s -marm -mfpu=vfp -mfloat-abi=hard --specs=nosys.specs -O3 -I$CIRCLE_HOME/install/arm-none-circle/include/ -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include-fixed" LDFLAGS="-L$CIRCLE_HOME/install/arm-none-circle/lib" ./configure --host=arm-none-eabi
cd ../..
done

LDFLAGS="-L$CIRCLE_HOME/install/arm-none-circle/lib" CXXFLAGS="-std=c++11 -O3 -ffreestanding -DAARCH=32 -march=armv6k -mtune=arm1176jzf-s -marm -mfpu=vfp -mfloat-abi=hard -fno-exceptions -fno-rtti -nostdinc++ --specs=nosys.specs" CFLAGS="-O3 -I$COMMON_HOME -I$CIRCLE_HOME/install/arm-none-circle/include/ -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include-fixed -I$CIRCLE_HOME/libs/circle/addon/fatfs -fno-exceptions --specs=nosys.specs -mfloat-abi=hard -ffreestanding -nostdlib -march=armv6k -mtune=arm1176jzf-s -marm -mfpu=vfp -nostdinc" ./configure --host=arm-none-eabi --disable-realdevice --disable-ipv6 --disable-ssi2001 --disable-catweasel --disable-hardsid --disable-parsid --disable-portaudio --disable-ahi --disable-bundle --disable-lame --disable-rs232 --disable-midi --disable-hidmgr --disable-hidutils --without-oss --without-alsa --without-pulse --without-zlib --disable-sdlui --disable-sdlui2 --enable-raspiui --enable-raspilite
elif [ "$BOARD" = "pi2" ]
then
cd src/resid
CXXFLAGS="-std=c++11 -funsafe-math-optimizations -fno-exceptions -fno-rtti -nostdinc++ -mfloat-abi=hard -ffreestanding -nostdlib -march=armv7-a -marm -mfpu=neon-vfpv4 --specs=nosys.specs -O3 -I$CIRCLE_HOME/install/arm-none-circle/include/ -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include-fixed" LDFLAGS="-L$CIRCLE_HOME/install/arm-none-circle/lib" ./configure --host=arm-none-eabi
cd ../..

LDFLAGS="-L$CIRCLE_HOME/install/arm-none-circle/lib" CXXFLAGS="-std=c++11 -O3 -mfloat-abi=hard -ffreestanding -march=armv7-a -marm -mfpu=neon-vfpv4 -fno-exceptions -fno-rtti -nostdinc++ --specs=nosys.specs" CFLAGS="-O3 -I$COMMON_HOME -I$CIRCLE_HOME/install/arm-none-circle/include/ -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include-fixed -I$CIRCLE_HOME/libs/circle/addon/fatfs -fno-exceptions --specs=nosys.specs -mfloat-abi=hard -ffreestanding -nostdlib -march=armv7-a -marm -mfpu=neon-vfpv4 -nostdinc" ./configure --host=arm-none-eabi --disable-realdevice --disable-ipv6 --disable-ssi2001 --disable-catweasel --disable-hardsid --disable-parsid --disable-portaudio --disable-ahi --disable-bundle --disable-lame --disable-rs232 --disable-midi --disable-hidmgr --disable-hidutils --without-oss --without-alsa --without-pulse --without-zlib --disable-sdlui --disable-sdlui2 --enable-raspiui
elif [ "$BOARD" = "pi3" ]
then
LDFLAGS="-L$CIRCLE_HOME/install/arm-none-circle/lib" CXXFLAGS="-O3 -std=c++11 -fno-exceptions -march=armv8-a -mtune=cortex-a53 -marm -mfpu=neon-fp-armv8 -mfloat-abi=hard -ffreestanding -nostdlib" CFLAGS="-O3 -I$COMMON_HOME -I$CIRCLE_HOME/install/arm-none-circle/include/ -I$CIRCLE_HOME/libs/circle/addon/fatfs -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include-fixed -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include -fno-exceptions -march=armv8-a -mtune=cortex-a53 -marm -mfpu=neon-fp-armv8 -mfloat-abi=hard -ffreestanding -nostdlib" ./configure --host=arm-none-eabi --disable-realdevice --disable-ipv6 --disable-ssi2001 --disable-catweasel --disable-hardsid --disable-parsid --disable-portaudio --disable-ahi --disable-bundle --disable-lame --disable-rs232 --disable-midi --disable-hidmgr --disable-hidutils --without-oss --without-alsa --without-pulse --without-zlib --disable-sdlui --disable-sdlui2 --enable-raspiui
elif [ "$BOARD" = "pi4" ]
then
LDFLAGS="-L$CIRCLE_HOME/install/arm-none-circle/lib" CXXFLAGS="-O3 -std=c++11 -fno-exceptions -march=armv8-a -mtune=cortex-a72 -marm -mfpu=neon-fp-armv8 -mfloat-abi=hard -ffreestanding -nostdlib" CFLAGS="-O3 -I$COMMON_HOME -I$CIRCLE_HOME/install/arm-none-circle/include/ -I$CIRCLE_HOME/libs/circle/addon/fatfs -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include-fixed -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include -fno-exceptions -march=armv8-a -mtune=cortex-a72 -marm -mfpu=neon-fp-armv8 -mfloat-abi=hard -ffreestanding -nostdlib" ./configure --host=arm-none-eabi --disable-realdevice --disable-ipv6 --disable-ssi2001 --disable-catweasel --disable-hardsid --disable-parsid --disable-portaudio --disable-ahi --disable-bundle --disable-lame --disable-rs232 --disable-midi --disable-hidmgr --disable-hidutils --without-oss --without-alsa --without-pulse --without-zlib --disable-sdlui --disable-sdlui2 --enable-raspiui
else
echo "I don't know what to do for $BOARD"
exit
fi

cd src
make libarchdep
if [ "$?" != "0" ]
then
       exit
fi
make libhvsc
if [ "$?" != "0" ]
then
       exit
fi
cd ..

# These will fail
make x64
make x128
make xvic
make xplus4
make xpet

echo ==============================================================
echo Link errors above are expected
echo ==============================================================

cd $SRC_DIR
make clean
BOARD=$BOARD make -f Makefile-C64
