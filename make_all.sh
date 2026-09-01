#!/bin/bash


# Set directory variables
SRC_DIR=`pwd`
CIRCLE_HOME="$SRC_DIR/third_party/circle-stdlib"
COMMON_HOME="$SRC_DIR/third_party/common"

# Check for the Arm GNU Toolchain and install it if necessary
if ! source "$SRC_DIR/get_gnu_toolchain.sh"
then
       echo "Arm GNU Toolchain setup failed." >&2
       exit 1
fi

CIRCLE_PUBLIC_INCLUDES="-I$CIRCLE_HOME/include -I$CIRCLE_HOME/libs/circle/include -I$CIRCLE_HOME/libs/circle/addon"

if ! command -v flex >/dev/null 2>&1
then
       echo "Missing required build dependency: flex" >&2
       echo "Install it with: sudo apt-get install flex" >&2
       exit 1
fi

# VICE's nested reSID configure scripts otherwise select the obsolete
# automake-1.15 binary instead of the installed compatible Automake.
export AUTOMAKE=automake

# AC_PROG_LEX probes libfl by linking an executable. That cannot work with the
# bare-metal toolchain, even though flex itself is available for code generation.
configure_vice()
{
       LEX=flex LEXLIB= ac_cv_lib_lex='none needed' ac_cv_search_yywrap='none required' "$@"
}


BOARD=""
SKIP_PATCHES=0
KASAN=0
IO_STATS=0

for arg in "$@"
do
case "$arg" in
       pi0|pi2|pi3|pi4)
              BOARD="$arg"
              ;;
       --skip-patches)
              SKIP_PATCHES=1
              ;;
       --kasan)
              KASAN=1
              ;;
       --io-stats)
              IO_STATS=1
              ;;
       *)
              echo "Need arg [pi0|pi2|pi3|pi4] [--skip-patches] [--kasan] [--io-stats]"
              exit 1
              ;;
esac
done

if [ -z "$BOARD" ]
then
echo "Need arg [pi0|pi2|pi3|pi4] [--skip-patches] [--kasan] [--io-stats]"
exit 1
fi

# Opt-in storage I/O instrumentation (SD/FatFs counters, "I/O Statistics" menu
# screen, raw benchmark). BMC64_IO_STATS is picked up by the common and kernel
# Makefiles; IO_STATS also gates the diskio stats patch below. See docs/BUILDING.md.
if [ "$IO_STATS" = "1" ]
then
       export BMC64_IO_STATS=1
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
apply_patch_file "$SRC_DIR/src/patches/circle_newlib_patch.diff"

cd $SRC_DIR/third_party/circle-stdlib/libs/circle
git reset --hard HEAD
git clean -fd

circle_patch_file="$SRC_DIR/src/patches/circle_patch.diff"
if [ "$BOARD" = "pi0" ]
then
       apply_patch_file "$circle_patch_file" 's@+#define ARM_ALLOW_MULTI_CORE@+//#define ARM_ALLOW_MULTI_CORE@'
else
       apply_patch_file "$circle_patch_file"
fi

apply_patch_file "$SRC_DIR/src/patches/circle_8bitdo_keyboard_patch.diff"
apply_patch_file "$SRC_DIR/src/patches/circle_8bitdo_gamepad_patch.diff"
apply_patch_file "$SRC_DIR/src/patches/circle_usb_descriptor_patch.diff"
apply_patch_file "$SRC_DIR/src/patches/circle_xbox360_gamepad_patch.diff"
apply_patch_file "$SRC_DIR/src/patches/circle_tcpconnection_patch.diff"
apply_patch_file "$SRC_DIR/src/patches/circle_ethernet_patch.diff"
apply_patch_file "$SRC_DIR/src/patches/circle_wlan_patch.diff"
if [ "$IO_STATS" = "1" ]
then
       apply_patch_file "$SRC_DIR/src/patches/circle_diskio_stats_patch.diff"
fi
if [ "$KASAN" = "1" ]
then
       apply_patch_file "$SRC_DIR/src/patches/circle_kasan_patch.diff"
fi
fi

echo ==============================================================
echo BUILD CIRCLE-STDLIB
echo $PATH
echo ==============================================================

cd $SRC_DIR/third_party/circle-stdlib

CIRCLE_CONFIGURE_ARGS=(--kernel-max-size 16)
if [ "$KASAN" = "1" ]
then
       CIRCLE_CONFIGURE_ARGS+=(--kasan)
fi

if [ "$BOARD" = "pi2" ]
then
#cat ../../src/patches/circle_stdlib_patch.diff  | sed 's/-std=c++14//' | patch -p1
./configure --raspberrypi=2 "${CIRCLE_CONFIGURE_ARGS[@]}"
elif [ "$BOARD" = "pi0" ]
then
#cat ../../src/patches/circle_stdlib_patch.diff | patch -p1
./configure --raspberrypi=1 "${CIRCLE_CONFIGURE_ARGS[@]}"
elif [ "$BOARD" = "pi3" ]
then
#cat ../../src/patches/circle_stdlib_patch.diff  | patch -p1
./configure --raspberrypi=3 "${CIRCLE_CONFIGURE_ARGS[@]}"
elif [ "$BOARD" = "pi4" ]
then
#cat ../../circle_stdlib_patch.diff  | patch -p1
./configure --raspberrypi=4 "${CIRCLE_CONFIGURE_ARGS[@]}"
else
echo "I don't know what to do for $BOARD"
exit
fi

VICE_KASAN_CFLAGS=""
if [ "$KASAN" = "1" ]
then
       KASAN_SHADOW_MAPPING_OFFSET=$(awk '/^KASAN_SHADOW_MAPPING_OFFSET = / { print $3; exit }' \
              "$CIRCLE_HOME/libs/circle/Config.mk")
       if [ -z "$KASAN_SHADOW_MAPPING_OFFSET" ]
       then
              echo "Circle did not generate a KASAN shadow mapping offset" >&2
              exit 1
       fi
       VICE_KASAN_CFLAGS="-fsanitize=kernel-address -fno-builtin -fasan-shadow-offset=$KASAN_SHADOW_MAPPING_OFFSET --param asan-globals=1 --param asan-stack=1 --param asan-instrumentation-with-call-threshold=0 -fno-omit-frame-pointer"
fi

cd $SRC_DIR/third_party/circle-stdlib/libs/circle
./makeall --nosample clean
if [ "$?" != "0" ]
then
       exit
fi

cd $SRC_DIR/third_party/circle-stdlib

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

cd $SRC_DIR/third_party/circle-stdlib/libs/circle/addon/wlan
make clean
make
if [ "$?" != "0" ]
then
       exit
fi

cd $SRC_DIR/third_party/circle-stdlib/libs/circle/addon/wlan/hostap/wpa_supplicant
make -f Makefile.circle clean
make -f Makefile.circle
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
if ! BOARD=$BOARD make
then
       echo "Failed to build BMC64 common library" >&2
       exit 1
fi

# Plus4Emu
cd $SRC_DIR/third_party/plus4emu
make
if [ "$?" != "0" ]
then
       exit
fi

# Vice
cd $SRC_DIR/third_party/vice-3.3

echo "Running autogen for VICE..."
if ! ./autogen.sh
then
       echo "VICE autogen failed!" >&2
       exit 1
fi

# autogen.sh does not force these legacy sub-builds to refresh. Regenerate them
# so their aclocal macros and Makefile.in files match the active Automake.
for resid_dir in src/resid src/teensy-resid
do
       if ! (cd "$resid_dir" && aclocal && autoconf -f && automake -a -c -f)
       then
              echo "Failed to regenerate VICE $resid_dir build files" >&2
              exit 1
       fi
done

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

LDFLAGS="-nostdlib -Wl,-e,main -L$CIRCLE_HOME/install/arm-none-circle/lib" CXXFLAGS="-std=c++11 -O3 -ffreestanding -DAARCH=32 -march=armv6k -mtune=arm1176jzf-s -marm -mfpu=vfp -mfloat-abi=hard -fno-exceptions -fno-rtti -nostdinc++ --specs=nosys.specs $CIRCLE_PUBLIC_INCLUDES" CFLAGS="-O3 -I$COMMON_HOME -I$CIRCLE_HOME/install/arm-none-circle/include/ $CIRCLE_PUBLIC_INCLUDES -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include-fixed -I$CIRCLE_HOME/libs/circle/addon/fatfs -fno-exceptions --specs=nosys.specs -mfloat-abi=hard -ffreestanding -nostdlib -march=armv6k -mtune=arm1176jzf-s -marm -mfpu=vfp -nostdinc $VICE_KASAN_CFLAGS" configure_vice ./configure --host=arm-none-eabi --disable-realdevice --disable-ipv6 --disable-ssi2001 --disable-catweasel --disable-hardsid --disable-parsid --disable-portaudio --disable-ahi --disable-bundle --disable-lame --disable-rs232 --disable-midi --disable-hidmgr --disable-hidutils --without-oss --without-alsa --without-pulse --without-zlib --disable-sdlui --disable-sdlui2 --enable-raspiui --enable-raspilite
if [ "$?" != "0" ]
then
       echo "Failed to configure VICE" >&2
       exit 1
fi
elif [ "$BOARD" = "pi2" ]
then
cd src/resid
CXXFLAGS="-std=c++11 -funsafe-math-optimizations -mfloat-abi=hard -march=armv7-a -marm -mfpu=neon-vfpv4 -O3 -I$CIRCLE_HOME/install/arm-none-circle/include/ $CIRCLE_PUBLIC_INCLUDES -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include-fixed" LDFLAGS="-L$CIRCLE_HOME/install/arm-none-circle/lib" ./configure --host=arm-none-eabi
cd ../..

LDFLAGS="-nostdlib -Wl,-e,main -L$CIRCLE_HOME/install/arm-none-circle/lib" CXXFLAGS="-std=c++11 -O3 -mfloat-abi=hard -ffreestanding -march=armv7-a -marm -mfpu=neon-vfpv4 -fno-exceptions -fno-rtti -nostdinc++ --specs=nosys.specs $CIRCLE_PUBLIC_INCLUDES" CFLAGS="-O3 -I$COMMON_HOME -I$CIRCLE_HOME/install/arm-none-circle/include/ $CIRCLE_PUBLIC_INCLUDES -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include-fixed -I$CIRCLE_HOME/libs/circle/addon/fatfs -fno-exceptions --specs=nosys.specs -mfloat-abi=hard -ffreestanding -nostdlib -march=armv7-a -marm -mfpu=neon-vfpv4 -nostdinc $VICE_KASAN_CFLAGS" configure_vice ./configure --host=arm-none-eabi --disable-realdevice --disable-ipv6 --disable-ssi2001 --disable-catweasel --disable-hardsid --disable-parsid --disable-portaudio --disable-ahi --disable-bundle --disable-lame --disable-rs232 --disable-midi --disable-hidmgr --disable-hidutils --without-oss --without-alsa --without-pulse --without-zlib --disable-sdlui --disable-sdlui2 --enable-raspiui
if [ "$?" != "0" ]
then
       echo "Failed to configure VICE" >&2
       exit 1
fi
elif [ "$BOARD" = "pi3" ]
then
cd src/resid
CXXFLAGS="-O3 -std=c++11 -march=armv8-a -mtune=cortex-a53 -marm -mfpu=neon-fp-armv8 -mfloat-abi=hard $CIRCLE_PUBLIC_INCLUDES" ./configure --host=arm-none-eabi
cd ../..

LDFLAGS="-nostdlib -Wl,-e,main -L$CIRCLE_HOME/install/arm-none-circle/lib" CXXFLAGS="-O3 -std=c++11 -fno-exceptions -march=armv8-a -mtune=cortex-a53 -marm -mfpu=neon-fp-armv8 -mfloat-abi=hard -ffreestanding -nostdlib -nostdinc++ $CIRCLE_PUBLIC_INCLUDES" CFLAGS="-O3 -I$COMMON_HOME -I$CIRCLE_HOME/install/arm-none-circle/include/ $CIRCLE_PUBLIC_INCLUDES -I$CIRCLE_HOME/libs/circle/addon/fatfs -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include-fixed -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include -fno-exceptions -march=armv8-a -mtune=cortex-a53 -marm -mfpu=neon-fp-armv8 -mfloat-abi=hard -ffreestanding -nostdlib $VICE_KASAN_CFLAGS" configure_vice ./configure --host=arm-none-eabi --disable-realdevice --disable-ipv6 --disable-ssi2001 --disable-catweasel --disable-hardsid --disable-parsid --disable-portaudio --disable-ahi --disable-bundle --disable-lame --disable-rs232 --disable-midi --disable-hidmgr --disable-hidutils --without-oss --without-alsa --without-pulse --without-zlib --disable-sdlui --disable-sdlui2 --enable-raspiui
if [ "$?" != "0" ]
then
       echo "Failed to configure VICE" >&2
       exit 1
fi
elif [ "$BOARD" = "pi4" ]
then
cd src/resid
CXXFLAGS="-O3 -std=c++11 -march=armv8-a -mtune=cortex-a72 -marm -mfpu=neon-fp-armv8 -mfloat-abi=hard $CIRCLE_PUBLIC_INCLUDES" ./configure --host=arm-none-eabi
cd ../..

LDFLAGS="-nostdlib -Wl,-e,main -L$CIRCLE_HOME/install/arm-none-circle/lib" CXXFLAGS="-O3 -std=c++11 -fno-exceptions -march=armv8-a -mtune=cortex-a72 -marm -mfpu=neon-fp-armv8 -mfloat-abi=hard -ffreestanding -nostdlib -nostdinc++ $CIRCLE_PUBLIC_INCLUDES" CFLAGS="-O3 -I$COMMON_HOME -I$CIRCLE_HOME/install/arm-none-circle/include/ $CIRCLE_PUBLIC_INCLUDES -I$CIRCLE_HOME/libs/circle/addon/fatfs -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include-fixed -I$ARM_HOME/lib/gcc/arm-none-eabi/$ARM_VERSION/include -fno-exceptions -march=armv8-a -mtune=cortex-a72 -marm -mfpu=neon-fp-armv8 -mfloat-abi=hard -ffreestanding -nostdlib $VICE_KASAN_CFLAGS" configure_vice ./configure --host=arm-none-eabi --disable-realdevice --disable-ipv6 --disable-ssi2001 --disable-catweasel --disable-hardsid --disable-parsid --disable-portaudio --disable-ahi --disable-bundle --disable-lame --disable-rs232 --disable-midi --disable-hidmgr --disable-hidutils --without-oss --without-alsa --without-pulse --without-zlib --disable-sdlui --disable-sdlui2 --enable-raspiui
if [ "$?" != "0" ]
then
       echo "Failed to configure VICE" >&2
       exit 1
fi
else
echo "I don't know what to do for $BOARD"
exit
fi

if ! grep -q '^#define HAVE_RS232BMC' src/config.h
then
echo "VICE configuration did not enable the BMC64 RS232 modem backend." >&2
exit 1
fi

echo "Rebuilding VICE after configuration"
make clean || true
find . -type f \( -name '*.o' -o -name '*.a' -o -name '*.lo' \) -delete
find . -type d \( -name '.deps' -o -name '.libs' \) -prune -exec rm -rf {} +

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

if [ "$BOARD" = "pi0" ]
then
       VICE_LINK_ARTIFACTS+=(src/teensy-resid/libresid.a)
else
       VICE_LINK_ARTIFACTS+=(src/resid/libresid.a)
fi

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

git restore --source=HEAD -- doc/vice.pdf ':(glob)doc/html/vice_*.html' \
       src/monitor/mon_parse.c src/monitor/mon_parse.h

echo ==============================================================
echo Link errors above are expected
echo ==============================================================

cd $SRC_DIR
make clean
BOARD=$BOARD make -f Makefile-C64
