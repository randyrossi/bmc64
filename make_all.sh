#!/bin/bash


# Set directory variables
SRC_DIR=`pwd`
CIRCLE_HOME="$SRC_DIR/third_party/circle-stdlib"
COMMON_HOME="$SRC_DIR/third_party/common"

# Check for the Arm GNU Toolchain and install it if necessary
source "$SRC_DIR/get_gnu_toolchain"

CIRCLE_PUBLIC_INCLUDES="-I$CIRCLE_HOME/include -I$CIRCLE_HOME/libs/circle/include -I$CIRCLE_HOME/libs/circle/addon"


BOARD=""
SKIP_PATCHES=0

for arg in "$@"
do
case "$arg" in
       pi0|pi2|pi3|pi4)
              BOARD="$arg"
              ;;
       --skip-patches)
              SKIP_PATCHES=1
              ;;
       *)
              echo "Need arg [pi0|pi2|pi3|pi4] [--skip-patches]"
              exit 1
              ;;
esac
done

if [ -z "$BOARD" ]
then
echo "Need arg [pi0|pi2|pi3|pi4] [--skip-patches]"
exit 1
fi

echo "Making for $BOARD"

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

apply_patch_file()
{
       patch_file="$1"
       sed_expression="$2"

       if [ ! -f "$patch_file" ]
       then
              echo "Required patch file is missing: $patch_file" >&2
              exit 1
       fi

       if [ -n "$sed_expression" ]
       then
              sed "$sed_expression" "$patch_file" | patch -p1
              patch_status=("${PIPESTATUS[@]}")
              if [ "${patch_status[0]}" != "0" ] || [ "${patch_status[1]}" != "0" ]
              then
                     exit 1
              fi
       else
              patch -p1 < "$patch_file"
              if [ "$?" != "0" ]
              then
                     exit 1
              fi
       fi
}

if [ "$SKIP_PATCHES" = "1" ]
then
echo "Skipping patches"
else
cd $SRC_DIR/third_party/circle-stdlib/libs/circle-newlib
git reset --hard HEAD
git clean -fd
apply_patch_file "$SRC_DIR/circle_newlib_patch.diff"

cd $SRC_DIR/third_party/circle-stdlib/libs/circle
git reset --hard HEAD
git clean -fd

circle_patch_file="$SRC_DIR/circle_patch.diff"
if [ "$BOARD" = "pi0" ]
then
       apply_patch_file "$circle_patch_file" 's@+#define ARM_ALLOW_MULTI_CORE@+//#define ARM_ALLOW_MULTI_CORE@'
else
       apply_patch_file "$circle_patch_file"
fi

apply_patch_file "$SRC_DIR/circle_8bitdo_keyboard_patch.diff"
apply_patch_file "$SRC_DIR/circle_8bitdo_gamepad_patch.diff"
apply_patch_file "$SRC_DIR/circle_usb_descriptor_patch.diff"
apply_patch_file "$SRC_DIR/circle_xbox360_gamepad_patch.diff"
fi

echo ==============================================================
echo BUILD CIRCLE-STDLIB
echo $PATH
echo ==============================================================

cd $SRC_DIR/third_party/circle-stdlib

if [ "$BOARD" = "pi2" ]
then
#cat ../../circle_stdlib_patch.diff  | sed 's/-std=c++14//' | patch -p1
./configure --raspberrypi=2 --kernel-max-size 16
elif [ "$BOARD" = "pi0" ]
then
#cat ../../circle_stdlib_patch.diff | patch -p1
./configure --raspberrypi=1 --kernel-max-size 16
elif [ "$BOARD" = "pi3" ]
then
#cat ../../circle_stdlib_patch.diff  | patch -p1
./configure --raspberrypi=3 --kernel-max-size 16
elif [ "$BOARD" = "pi4" ]
then
#cat ../../circle_stdlib_patch.diff  | patch -p1
./configure --raspberrypi=4 --kernel-max-size 16
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
make clean
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

# Run autogen to ensure build system is up-to-date
echo "Running autogen for VICE..."
./autogen.sh
if [ "$?" != "0" ]; then
    echo "autogen failed!"
    exit 1
fi

if [ "$BOARD" = "pi0" ]
then
# We have to configure resid even though we don't link it for pi0 to
# keep top level make happy. Too much of a hassle to fix configure.
DIRS="src/resid src/teensy-resid"
for d in $DIRS
do
cd $d
if ! CXXFLAGS="-std=c++11 -funsafe-math-optimizations -DAARCH=32 -march=armv6k -mtune=arm1176jzf-s -marm -mfpu=vfp -mfloat-abi=hard -O3 -I$CIRCLE_HOME/install/arm-none-circle/include/ $CIRCLE_PUBLIC_INCLUDES -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include-fixed" LDFLAGS="-nostdlib -Wl,-e,main" ./configure --host=arm-none-eabi
then
       echo "Failed to configure $d"
       exit 1
fi
cd ../..
done

LDFLAGS="-L$CIRCLE_HOME/install/arm-none-circle/lib" CXXFLAGS="-std=c++11 -O3 -ffreestanding -DAARCH=32 -march=armv6k -mtune=arm1176jzf-s -marm -mfpu=vfp -mfloat-abi=hard -fno-exceptions -fno-rtti -nostdinc++ --specs=nosys.specs $CIRCLE_PUBLIC_INCLUDES" CFLAGS="-O3 -I$COMMON_HOME -I$CIRCLE_HOME/install/arm-none-circle/include/ $CIRCLE_PUBLIC_INCLUDES -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include-fixed -I$CIRCLE_HOME/libs/circle/addon/fatfs -fno-exceptions --specs=nosys.specs -mfloat-abi=hard -ffreestanding -nostdlib -march=armv6k -mtune=arm1176jzf-s -marm -mfpu=vfp -nostdinc" ./configure --host=arm-none-eabi --disable-realdevice --disable-ipv6 --disable-ssi2001 --disable-catweasel --disable-hardsid --disable-parsid --disable-portaudio --disable-ahi --disable-bundle --disable-lame --disable-rs232 --disable-midi --disable-hidmgr --disable-hidutils --without-oss --without-alsa --without-pulse --without-zlib --disable-sdlui --disable-sdlui2 --enable-raspiui --enable-raspilite
elif [ "$BOARD" = "pi2" ]
then
cd src/resid
CXXFLAGS="-std=c++11 -funsafe-math-optimizations -mfloat-abi=hard -march=armv7-a -marm -mfpu=neon-vfpv4 -O3 -I$CIRCLE_HOME/install/arm-none-circle/include/ $CIRCLE_PUBLIC_INCLUDES -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include-fixed" LDFLAGS="-L$CIRCLE_HOME/install/arm-none-circle/lib" ./configure --host=arm-none-eabi
cd ../..

LDFLAGS="-L$CIRCLE_HOME/install/arm-none-circle/lib" CXXFLAGS="-std=c++11 -O3 -mfloat-abi=hard -ffreestanding -march=armv7-a -marm -mfpu=neon-vfpv4 -fno-exceptions -fno-rtti -nostdinc++ --specs=nosys.specs $CIRCLE_PUBLIC_INCLUDES" CFLAGS="-O3 -I$COMMON_HOME -I$CIRCLE_HOME/install/arm-none-circle/include/ $CIRCLE_PUBLIC_INCLUDES -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include-fixed -I$CIRCLE_HOME/libs/circle/addon/fatfs -fno-exceptions --specs=nosys.specs -mfloat-abi=hard -ffreestanding -nostdlib -march=armv7-a -marm -mfpu=neon-vfpv4 -nostdinc" ./configure --host=arm-none-eabi --disable-realdevice --disable-ipv6 --disable-ssi2001 --disable-catweasel --disable-hardsid --disable-parsid --disable-portaudio --disable-ahi --disable-bundle --disable-lame --disable-rs232 --disable-midi --disable-hidmgr --disable-hidutils --without-oss --without-alsa --without-pulse --without-zlib --disable-sdlui --disable-sdlui2 --enable-raspiui
elif [ "$BOARD" = "pi3" ]
then
cd src/resid
CXXFLAGS="-O3 -std=c++11 -march=armv8-a -mtune=cortex-a53 -marm -mfpu=neon-fp-armv8 -mfloat-abi=hard $CIRCLE_PUBLIC_INCLUDES" ./configure --host=arm-none-eabi
cd ../..

LDFLAGS="-L$CIRCLE_HOME/install/arm-none-circle/lib" CXXFLAGS="-O3 -std=c++11 -fno-exceptions -march=armv8-a -mtune=cortex-a53 -marm -mfpu=neon-fp-armv8 -mfloat-abi=hard -ffreestanding -nostdlib $CIRCLE_PUBLIC_INCLUDES" CFLAGS="-O3 -I$COMMON_HOME -I$CIRCLE_HOME/install/arm-none-circle/include/ $CIRCLE_PUBLIC_INCLUDES -I$CIRCLE_HOME/libs/circle/addon/fatfs -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include-fixed -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include -fno-exceptions -march=armv8-a -mtune=cortex-a53 -marm -mfpu=neon-fp-armv8 -mfloat-abi=hard -ffreestanding -nostdlib" ./configure --host=arm-none-eabi --disable-realdevice --disable-ipv6 --disable-ssi2001 --disable-catweasel --disable-hardsid --disable-parsid --disable-portaudio --disable-ahi --disable-bundle --disable-lame --disable-rs232 --disable-midi --disable-hidmgr --disable-hidutils --without-oss --without-alsa --without-pulse --without-zlib --disable-sdlui --disable-sdlui2 --enable-raspiui
elif [ "$BOARD" = "pi4" ]
then
cd src/resid
CXXFLAGS="-O3 -std=c++11 -march=armv8-a -mtune=cortex-a72 -marm -mfpu=neon-fp-armv8 -mfloat-abi=hard $CIRCLE_PUBLIC_INCLUDES" ./configure --host=arm-none-eabi
cd ../..

LDFLAGS="-L$CIRCLE_HOME/install/arm-none-circle/lib" CXXFLAGS="-O3 -std=c++11 -fno-exceptions -march=armv8-a -mtune=cortex-a72 -marm -mfpu=neon-fp-armv8 -mfloat-abi=hard -ffreestanding -nostdlib $CIRCLE_PUBLIC_INCLUDES" CFLAGS="-O3 -I$COMMON_HOME -I$CIRCLE_HOME/install/arm-none-circle/include/ $CIRCLE_PUBLIC_INCLUDES -I$CIRCLE_HOME/libs/circle/addon/fatfs -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include-fixed -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include -fno-exceptions -march=armv8-a -mtune=cortex-a72 -marm -mfpu=neon-fp-armv8 -mfloat-abi=hard -ffreestanding -nostdlib" ./configure --host=arm-none-eabi --disable-realdevice --disable-ipv6 --disable-ssi2001 --disable-catweasel --disable-hardsid --disable-parsid --disable-portaudio --disable-ahi --disable-bundle --disable-lame --disable-rs232 --disable-midi --disable-hidmgr --disable-hidutils --without-oss --without-alsa --without-pulse --without-zlib --disable-sdlui --disable-sdlui2 --enable-raspiui
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

# Build teensy-resid for pi0
if [ "$BOARD" = "pi0" ]
then
cd teensy-resid
make libresid.a
if [ "$?" != "0" ]
then
       exit
fi
cd ..
fi

cd ..

# These will fail
make x64
make x128
make xvic
make xplus4
make xpet

VICE_LINK_ARTIFACTS=(
       src/core/libcore.a
       src/userport/libuserport.a
       src/rtc/librtc.a
       src/usleep.o
       src/imagecontents/libimagecontents.a
)

missing_vice_artifacts=()
for artifact in "${VICE_LINK_ARTIFACTS[@]}"
do
       if [ ! -f "$artifact" ]
       then
              missing_vice_artifacts+=("$artifact")
       fi
done

if [ "${#missing_vice_artifacts[@]}" -ne 0 ]
then
       echo "VICE did not produce required BMC64 link artifacts:" >&2
       printf '  %s\n' "${missing_vice_artifacts[@]}" >&2
       exit 1
fi

echo ==============================================================
echo Link errors above are expected
echo ==============================================================

cd $SRC_DIR
make clean
BOARD=$BOARD make -f Makefile-C64
