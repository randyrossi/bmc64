#!/bin/sh

X64_STATUS_MESSAGE="x64 compiles correctly and runs correctly."
X64SC_STATUS_MESSAGE="x64sc compiles correctly and runs correctly albeit slow, only meant for high end android devices."
X64DTV_STATUS_MESSAGE="x64dtv compiles correctly and runs correctly."
XSCPU64_STATUS_MESSAGE="xscpu64 compiles correctly and runs correctly albeit slow, only meant for high end android devices."
X128_STATUS_MESSAGE="x128 compiles correctly and runs correctly, vdc display is untested and might need work."
XCBM2_STATUS_MESSAGE="xcbm2 compiles correctly and runs correctly, needs a device which can handle 640x200 screen size."
XCBM5X0_STATUS_MESSAGE="xcbm5x0 compiles correctly and runs correctly."
XPET_STATUS_MESSAGE="xcbm2 compiles correctly and runs correctly, needs a device which can handle 640x200 screen size."
XPLUS4_STATUS_MESSAGE="xplus4 compiles correctly and runs correctly."
XVIC_STATUS_MESSAGE="xvic compiles correctly and runs correctly, screen dimensions needs some work."

MACHINE_X64="0"
MACHINE_X64SC="1"
MACHINE_XSCPU64="2"
MACHINE_X64DTV="3"
MACHINE_X128="4"
MACHINE_XCBM2="5"
MACHINE_XCBM5X0="6"
MACHINE_XPET="7"
MACHINE_XPLUS4="8"
MACHINE_XVIC="9"
MACHINE_ALL="100"

C64_ROMS="kernal basic chargen sdl_sym.vkm"
C64DTV_ROMS="kernal basic chargen dtvrom.bin sdl_sym.vkm"
SCPU64_ROMS="chargen scpu64 sdl_sym.vkm"
CBM5X0_ROMS="kernal.500 basic.500 chargen.500 sdl_sym.vkm"
CBM2_ROMS="kernal basic.128 chargen.600 sdl_sym.vkm"
C128_ROMS="kernal kernal64 kernalch kernalde kernalfi kernalfr kernalit kernalno kernalse chargch chargde chargen chargfr chargse basiclo basichi basic64 kernal64 sdl_pos.vkm"
PET_ROMS="kernal4 basic4 chargen edit4b80 sdl_sym.vkm"
PLUS4_ROMS="kernal basic 3plus1lo 3plus1hi sdl_pos.vkm"
VIC20_ROMS="kernal basic chargen sdl_sym.vkm"
DRIVE_ROMS="d1541II d1571cr dos1001 dos1540 dos1541 dos1551 dos1570 dos1571 dos2031 dos2040 dos3040 dos4040"

ROMS_EXTERNAL="0"
ROMS_PUSHED="1"
ROMS_ASSET="2"
ROMS_EMBEDDED="3"
ROMS_INVALID="-1"

# see if we are in the top of the tree
if [ ! -f configure.proto ]; then
  cd ../..
  if [ ! -f configure.proto ]; then
    echo "please run this script from the base of the VICE directory"
    exit 1
  fi
fi

VICEVERSION=`${SHELL} src/vice-version.sh`

curdir=`pwd`

# set all cpu builds to no
armbuild=no
arm7abuild=no
arm64build=no
mipsbuild=no
mips64build=no
x86build=no
x86_64build=no

showusage=no

buildrelease=no
builddevrelease=no
builddebug=no

buildemulators=0
romhandling=""
options=""

# no options, go into interactive mode
if test x"$1" = "x"; then
  echo "No parameters give, entering interactive mode."
  echo ""

  echo "Is this an official release build ? [n]"
  read answer
  if test x"$answer" != "x" -a x"$answer" != "n"; then
    options="$options release"
  fi

  echo "What cpu(s) to compile for ? (armeabi, armeabi-v7a, arm64-v8a, mips, mips64, x86, x86_64, all) [armeabi]"
  read answer
  if test x"$answer" = "xall"; then
    options="$options all-cpu"
  else
    if test x"$answer" != "x"; then
      options="$options $answer"
    fi
  fi

  echo "What emulator to build ?"
  echo "(x64, x64sc, xscpu64, x64dtv, x128, xcbm2, xcbm5x0, xpet, xplus4, xvic, all) [x64]"
  read answer
  if test x"$answer" = "xall"; then
    options="$options all-emu"
  else
    if test x"$answer" != "x"; then
      options="$options $answer"
    fi
  fi

  echo "What type of rom handling to use ? (external, pushed, asset, embedded) [asset]"
  echo "  external - use external roms that the user has to put in place"
  echo "  pushed   - use external roms put in place by the push.sh script"
  echo "  asset    - roms will be inside the .apk and put into place automatically"
  echo "  embedded - roms will be inside the shared object\(s\) in the .apk"
  read answer
  if test x"$answer" != "x"; then
    options="$options "$answer"roms"
  fi
else
  options=$*
fi

# check options
for i in $options
do
  validoption=no
  if test x"$i" = "xarmeabi"; then
    armbuild=yes
    validoption=yes
  fi
  if test x"$i" = "xarmeabi-v7a"; then
    arm7abuild=yes
    validoption=yes
  fi
  if test x"$i" = "xarm64-v8a"; then
    arm64build=yes
    validoption=yes
  fi
  if test x"$i" = "xmips"; then
    mipsbuild=yes
    validoption=yes
  fi
  if test x"$i" = "xmips64"; then
    mips64build=yes
    validoption=yes
  fi
  if test x"$i" = "xx86"; then
    x86build=yes
    validoption=yes
  fi
  if test x"$i" = "xx86_64"; then
    x86_64build=yes
    validoption=yes
  fi
  if test x"$i" = "xall-cpu"; then
    armbuild=yes
    arm7abuild=yes
    arm64build=yes
    mipsbuild=yes
    mips64build=yes
    x86build=yes
    x86_64build=yes
    validoption=yes
  fi
  if test x"$i" = "xhelp"; then
    showusage=yes
    validoption=yes
  fi
  if test x"$i" = "xrelease"; then
    buildrelease=yes
    validoption=yes
  fi
  if test x"$i" = "xx64"; then
    buildemulators=`expr $buildemulators + 1`
    emulator="x64"
    emulib="libx64.so"
    emuname="AnVICE_x64"
    STATUS_MESSAGE=$X64_STATUS_MESSAGE
    MACHINE=$MACHINE_X64
    validoption=yes
  fi
  if test x"$i" = "xx64sc"; then
    buildemulators=`expr $buildemulators + 1`
    emulator="x64sc"
    emulib="libx64sc.so"
    emuname="AnVICE_x64sc"
    STATUS_MESSAGE=$X64SC_STATUS_MESSAGE
    MACHINE=$MACHINE_X64SC
    validoption=yes
  fi
  if test x"$i" = "xx64dtv"; then
    buildemulators=`expr $buildemulators + 1`
    emulator="x64dtv"
    emulib="libx64dtv.so"
    emuname="AnVICE_x64dtv"
    STATUS_MESSAGE=$X64DTV_STATUS_MESSAGE
    MACHINE=$MACHINE_X64DTV
    validoption=yes
  fi
  if test x"$i" = "xxscpu64"; then
    buildemulators=`expr $buildemulators + 1`
    emulator="xscpu64"
    emulib="libxscpu64.so"
    emuname="AnVICE_xscpu64"
    STATUS_MESSAGE=$XSCPU64_STATUS_MESSAGE
    MACHINE=$MACHINE_XSCPU64
    validoption=yes
  fi
  if test x"$i" = "xx128"; then
    buildemulators=`expr $buildemulators + 1`
    emulator="x128"
    emulib="libx128.so"
    emuname="AnVICE_x128"
    STATUS_MESSAGE=$X128_STATUS_MESSAGE
    MACHINE=$MACHINE_X128
    validoption=yes
  fi
  if test x"$i" = "xxcbm2"; then
    buildemulators=`expr $buildemulators + 1`
    emulator="xcbm2"
    emulib="libxcbm2.so"
    emuname="AnVICE_xcbm2"
    STATUS_MESSAGE=$XCBM2_STATUS_MESSAGE
    MACHINE=$MACHINE_XCBM2
    validoption=yes
  fi
  if test x"$i" = "xxcbm5x0"; then
    buildemulators=`expr $buildemulators + 1`
    emulator="xcbm5x0"
    emulib="libxcbm5x0.so"
    emuname="AnVICE_xcbm5x0"
    STATUS_MESSAGE=$XCBM5X0_STATUS_MESSAGE
    MACHINE=$MACHINE_XCBM5X0
    validoption=yes
  fi
  if test x"$i" = "xxpet"; then
    buildemulators=`expr $buildemulators + 1`
    emulator="xpet"
    emulib="libxpet.so"
    emuname="AnVICE_xpet"
    STATUS_MESSAGE=$XPET_STATUS_MESSAGE
    MACHINE=$MACHINE_XPET
    validoption=yes
  fi
  if test x"$i" = "xxplus4"; then
    buildemulators=`expr $buildemulators + 1`
    emulator="xplus4"
    emulib="libxplus4.so"
    emuname="AnVICE_xplus4"
    STATUS_MESSAGE=$XPLUS4_STATUS_MESSAGE
    MACHINE=$MACHINE_XPLUS4
    validoption=yes
  fi
  if test x"$i" = "xxvic"; then
    buildemulators=`expr $buildemulators + 1`
    emulator="xvic"
    emulib="libxvic.so"
    emuname="AnVICE_xvic"
    STATUS_MESSAGE=$XVIC_STATUS_MESSAGE
    MACHINE=$MACHINE_XVIC
    validoption=yes
  fi
  if test x"$i" = "xall-emu"; then
    buildemulators=`expr $buildemulators + 1`
    emulator="all emulators"
    emulib="libvice.so"
    emuname="AnVICE"
    MACHINE=$MACHINE_ALL
    validoption=yes
  fi
  if test x"$i" = "xexternalroms"; then
    if test x"$romhandling" = "x"; then
      romhandling=$ROMS_EXTERNAL
    else
      romhandling=$ROMS_INVALID
    fi
    validoption=yes
  fi
  if test x"$i" = "xpushedroms"; then
    if test x"$romhandling" = "x"; then
      romhandling=$ROMS_PUSHED
    else
      romhandling=$ROMS_INVALID
    fi
    validoption=yes
  fi
  if test x"$i" = "xassetroms"; then
    if test x"$romhandling" = "x"; then
      romhandling=$ROMS_ASSET
    else
      romhandling=$ROMS_INVALID
    fi
    validoption=yes
  fi
  if test x"$i" = "xembeddedroms"; then
    if test x"$romhandling" = "x"; then
      romhandling=$ROMS_EMBEDDED
    else
      romhandling=$ROMS_INVALID
    fi
    validoption=yes
  fi
  if test x"$validoption" != "xyes"; then
    echo "unknown option: $i"
    exit 1
  fi
done

if test x"$showusage" = "xyes"; then
  echo "Usage: $0 [<options>]"
  echo "  release      - build an official release version"
  echo "cpu-types:"
  echo "  armeabi      - build for soft-fpu arm device"
  echo "  armeabi-v7a  - build for hw-fpu arm device"
  echo "  arm64-v8a    - build for arm64 device"
  echo "  mips         - build for mips device"
  echo "  mips64       - build for mips64 device"
  echo "  x86          - build for x86 device"
  echo "  x86_64       - build for x86_64 device"
  echo "  all-cpu      - build for all cpu devices"
  echo "emulators:"
  echo "  x64          - build x64     \(AnVICE_x64\)     only"
  echo "  x64sc        - build x64sc   \(AnVICE_x64sc\)   only"
  echo "  xscpu64      - build xscpu64 \(AnVICE_xscpu64\) only"
  echo "  x64dtv       - build x64dtv  \(AnVICE_x64dtv\)  only"
  echo "  x128         - build x128    \(AnVICE_x128\)    only"
  echo "  xcbm2        - build xcbm2   \(AnVICE_xcbm2\)   only"
  echo "  xcbm5x0      - build xcbm5x0 \(AnVICE_xcvm5x0\) only"
  echo "  xpet         - build xpet    \(AnVICE_xpet\)    only"
  echo "  xplus4       - build xplus4  \(AnVICE_xplus4\)  only"
  echo "  xvic         - build xvic    \(AnVICE_xvic\)    only"
  echo "  all-emu      - build all     \(AnVICE\)"
  echo "roms:"
  echo "  externalroms - use external roms that the user has to put in place"
  echo "  pushedroms   - use external roms put in place by the push.sh script"
  echo "  assetroms    - roms will be inside the .apk and put int place automatically"
  echo "  embeddedroms - roms will be inside the shared object\(s\) in the .apk"
  exit 1
fi

if test x"$buildemulators" = "x0"; then
  emulator="x64"
  emulib="libx64.so"
  emuname="AnVICE_x64"
  STATUS_MESSAGE=$X64_STATUS_MESSAGE
  MACHINE=$MACHINE_X64
else
  if test x"$buildemulators" != "x1"; then
    echo "Only one emulator option can be specified"
    exit 1
  fi
fi

if test x"$romhandling" = "x$ROMS_INVALID"; then
  echo "Only 1 type of rom handling can be specified"
  exit 1
fi

if test x"$romhandling" = "x"; then
  romhandling=$ROMS_ASSET
fi

if test x"$emulator" = "xall emulators"; then
  if test x"$romhandling" = "x$ROMS_EXTERNAL"; then
    echo "Cannot use external roms for all emulators"
    exit 1
  fi
fi

if test x"$ANDROID_HOME" = "x"; then
  echo "Please set \$ANDROID_HOME"
  exit 1
fi

CPUS=""

if test x"$armbuild" = "xyes"; then
  CPUS="armeabi"
fi

if test x"$arm7abuild" = "xyes"; then
  if test x"$CPUS" = "x"; then
    CPUS="armeabi-v7a"
  else
    CPUS="$CPUS armeabi-v7a"
  fi
fi

if test x"$arm64build" = "xyes"; then
  if test x"$CPUS" = "x"; then
    CPUS="arm64-v8a"
  else
    CPUS="$CPUS arm64-v8a"
  fi
fi

if test x"$mipsbuild" = "xyes"; then
  if test x"$CPUS" = "x"; then
    CPUS="mips"
  else
    CPUS="$CPUS mips"
  fi
fi

if test x"$mips64build" = "xyes"; then
  if test x"$CPUS" = "x"; then
    CPUS="mips64"
  else
    CPUS="$CPUS mips64"
  fi
fi

if test x"$x86build" = "xyes"; then
  if test x"$CPUS" = "x"; then
    CPUS="x86"
  else
    CPUS="$CPUS x86"
  fi
fi

if test x"$x86_64build" = "xyes"; then
  if test x"$CPUS" = "x"; then
    CPUS="x86_64"
  else
    CPUS="$CPUS x86_64"
  fi
fi

if test x"$CPUS" = "x"; then
  CPUS="armeabi"
fi

if test x"$CPUS" = "xarmeabi armeabi-v7a arm64-v8a mips mips64 x86 x86_64"; then
  CPULABEL="all"
else
  CPULABEL=$CPUS
fi

if test x"$buildrelease" = "xyes"; then
  if [ ! -f vice-release.keystore ]; then
    echo "vice-release.keystore not found, will fallback on a debug build"
    buildrelease=no
    builddebug=yes
  fi
else
  if [ ! -f vice-dev.keystore ]; then
    echo "vice-dev.keystore not found, will use a debug key instead"
    builddebug=yes
  else
    builddebug=no
    builddevrelease=yes
  fi
fi

cd src

cp arch/android/AnVICE/debug.h.proto ./debug.h

echo generating src/translate_table.h
${SHELL} buildtools/gentranslatetable.sh <translate.txt >translate_table.h

echo generating src/translate.h
${SHELL} buildtools/gentranslate_h.sh <translate.txt >translate.h

echo generating src/infocontrib.h${SHELL} buildtools/geninfocontrib_h.sh infocontrib.h <../doc/vice.texi | sed -f buildtools/infocontrib.sed >infocontrib.h

cd arch/android/AnVICE

echo generating config.android.h and versions info
sed "s|\@VERSION\@|\"$VICEVERSION\"|g" <config.android.h.proto >config.android.h

echo generating local.properties
sed "s|\@ANDROID_HOME\@|$ANDROID_HOME|g" <local.properties.proto >local.properties

cd jni

echo generating Application.mk
cp Application.mk.proto Application.mk
echo >>Application.mk "APP_ABI := $CPUS"

echo clearing out all Android.mk files
for i in `find . -name "Android.mk"`
do
  rm -f $i
done

echo generating Android.mk files for $emulator

if test x"$emulator" = "xx64"; then
  cp Android.mk.proto Android.mk
  cp locnet_x64/Android.mk.proto locnet_x64/Android.mk
  cp locnet_al/Android.mk.proto locnet_al/Android.mk
  cp vice_x64/Android.mk.proto vice_x64/Android.mk
  cp vice_x64_128_5x0/Android.mk.proto vice_x64_128_5x0/Android.mk
  cp vice_x64_dtv_128_5x0/Android.mk.proto vice_x64_dtv_128_5x0/Android.mk
  cp vice_x64_sc/Android.mk.proto vice_x64_sc/Android.mk
  cp vice_x64_sc_128/Android.mk.proto vice_x64_sc_128/Android.mk
  cp vice_x64_sc_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv/Android.mk.proto vice_x64_sc_dtv/Android.mk
  cp vice_x64_sc_dtv_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_dtv_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu/Android.mk.proto vice_x64_sc_dtv_scpu/Android.mk
  cp vice_x64_sc_dtv_scpu_128/Android.mk.proto vice_x64_sc_dtv_scpu_128/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_pet_4/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_pet_4/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_pet_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_20/Android.mk
  cp vice_x64_sc_scpu/Android.mk.proto vice_x64_sc_scpu/Android.mk
  cp vice_x64_sc_scpu_128/Android.mk.proto vice_x64_sc_scpu_128/Android.mk
  cp vice_x64_sc_scpu_128_2/Android.mk.proto vice_x64_sc_scpu_128_2/Android.mk
  cp vice_x64_sc_scpu_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_scpu_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_scpu_128_2_pet_4_20/Android.mk.proto vice_x64_sc_scpu_128_2_pet_4_20/Android.mk
  cp vice_x64_sc_scpu_128_2_pet_20/Android.mk.proto vice_x64_sc_scpu_128_2_pet_20/Android.mk
  cp vice_x64_sc_scpu_128_4/Android.mk.proto vice_x64_sc_scpu_128_4/Android.mk
  cp vice_x64_sc_scpu_128_20/Android.mk.proto vice_x64_sc_scpu_128_20/Android.mk
  cp vice_x64_sc_scpu_128_5x0_4_20/Android.mk.proto vice_x64_sc_scpu_128_5x0_4_20/Android.mk
fi

if test x"$emulator" = "xx64sc"; then
  cp Android.mk.proto Android.mk
  cp locnet_x64sc/Android.mk.proto locnet_x64sc/Android.mk
  cp locnet_al/Android.mk.proto locnet_al/Android.mk
  cp vice_x64_sc/Android.mk.proto vice_x64_sc/Android.mk
  cp vice_x64_sc_128/Android.mk.proto vice_x64_sc_128/Android.mk
  cp vice_x64_sc_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv/Android.mk.proto vice_x64_sc_dtv/Android.mk
  cp vice_x64_sc_dtv_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_dtv_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu/Android.mk.proto vice_x64_sc_dtv_scpu/Android.mk
  cp vice_x64_sc_dtv_scpu_128/Android.mk.proto vice_x64_sc_dtv_scpu_128/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_pet_4/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_pet_4/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_pet_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_20/Android.mk
  cp vice_x64_sc_scpu/Android.mk.proto vice_x64_sc_scpu/Android.mk
  cp vice_x64_sc_scpu_128/Android.mk.proto vice_x64_sc_scpu_128/Android.mk
  cp vice_x64_sc_scpu_128_2/Android.mk.proto vice_x64_sc_scpu_128_2/Android.mk
  cp vice_x64_sc_scpu_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_scpu_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_scpu_128_2_pet_4_20/Android.mk.proto vice_x64_sc_scpu_128_2_pet_4_20/Android.mk
  cp vice_x64_sc_scpu_128_2_pet_20/Android.mk.proto vice_x64_sc_scpu_128_2_pet_20/Android.mk
  cp vice_x64_sc_scpu_128_4/Android.mk.proto vice_x64_sc_scpu_128_4/Android.mk
  cp vice_x64_sc_scpu_128_20/Android.mk.proto vice_x64_sc_scpu_128_20/Android.mk
  cp vice_x64_sc_scpu_128_5x0_4_20/Android.mk.proto vice_x64_sc_scpu_128_5x0_4_20/Android.mk
  cp vice_x64sc/Android.mk.proto vice_x64sc/Android.mk
  cp vice_x64sc_scpu/Android.mk.proto vice_x64sc_scpu/Android.mk
  cp vice_x64sc_scpu_128_2_pet_4_20/Android.mk.proto vice_x64sc_scpu_128_2_pet_4_20/Android.mk
fi

if test x"$emulator" = "xx64dtv"; then
  cp Android.mk.proto Android.mk
  cp locnet_x64dtv/Android.mk.proto locnet_x64dtv/Android.mk
  cp locnet_al/Android.mk.proto locnet_al/Android.mk
  cp vice_x64_dtv_128_5x0/Android.mk.proto vice_x64_dtv_128_5x0/Android.mk
  cp vice_x64_sc_dtv/Android.mk.proto vice_x64_sc_dtv/Android.mk
  cp vice_x64_sc_dtv_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_dtv_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu/Android.mk.proto vice_x64_sc_dtv_scpu/Android.mk
  cp vice_x64_sc_dtv_scpu_128/Android.mk.proto vice_x64_sc_dtv_scpu_128/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_pet_4/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_pet_4/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_pet_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_20/Android.mk
  cp vice_x64dtv/Android.mk.proto vice_x64dtv/Android.mk
fi

if test x"$emulator" = "xxscpu64"; then
  cp Android.mk.proto Android.mk
  cp locnet_xscpu64/Android.mk.proto locnet_xscpu64/Android.mk
  cp locnet_al/Android.mk.proto locnet_al/Android.mk
  cp vice_x64_sc_dtv_scpu/Android.mk.proto vice_x64_sc_dtv_scpu/Android.mk
  cp vice_x64_sc_dtv_scpu_128/Android.mk.proto vice_x64_sc_dtv_scpu_128/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_pet_4/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_pet_4/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_pet_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_20/Android.mk
  cp vice_x64_sc_scpu/Android.mk.proto vice_x64_sc_scpu/Android.mk
  cp vice_x64_sc_scpu_128/Android.mk.proto vice_x64_sc_scpu_128/Android.mk
  cp vice_x64_sc_scpu_128_2/Android.mk.proto vice_x64_sc_scpu_128_2/Android.mk
  cp vice_x64_sc_scpu_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_scpu_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_scpu_128_2_pet_4_20/Android.mk.proto vice_x64_sc_scpu_128_2_pet_4_20/Android.mk
  cp vice_x64_sc_scpu_128_2_pet_20/Android.mk.proto vice_x64_sc_scpu_128_2_pet_20/Android.mk
  cp vice_x64_sc_scpu_128_4/Android.mk.proto vice_x64_sc_scpu_128_4/Android.mk
  cp vice_x64_sc_scpu_128_20/Android.mk.proto vice_x64_sc_scpu_128_20/Android.mk
  cp vice_x64_sc_scpu_128_5x0_4_20/Android.mk.proto vice_x64_sc_scpu_128_5x0_4_20/Android.mk
  cp vice_x64sc_scpu/Android.mk.proto vice_x64sc_scpu/Android.mk
  cp vice_x64sc_scpu_128_2_pet_4_20/Android.mk.proto vice_x64sc_scpu_128_2_pet_4_20/Android.mk
  cp vice_xscpu64/Android.mk.proto vice_xscpu64/Android.mk
fi

if test x"$emulator" = "xxvic"; then
  cp Android.mk.proto Android.mk
  cp locnet_xvic/Android.mk.proto locnet_xvic/Android.mk
  cp locnet_al/Android.mk.proto locnet_al/Android.mk
  cp vice_x64_sc_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_dtv_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_pet_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_20/Android.mk
  cp vice_x64_sc_scpu_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_scpu_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_scpu_128_2_pet_4_20/Android.mk.proto vice_x64_sc_scpu_128_2_pet_4_20/Android.mk
  cp vice_x64_sc_scpu_128_2_pet_20/Android.mk.proto vice_x64_sc_scpu_128_2_pet_20/Android.mk
  cp vice_x64_sc_scpu_128_20/Android.mk.proto vice_x64_sc_scpu_128_20/Android.mk
  cp vice_x64_sc_scpu_128_5x0_4_20/Android.mk.proto vice_x64_sc_scpu_128_5x0_4_20/Android.mk
  cp vice_x64sc_scpu_128_2_pet_4_20/Android.mk.proto vice_x64sc_scpu_128_2_pet_4_20/Android.mk
  cp vice_xvic/Android.mk.proto vice_xvic/Android.mk
fi

if test x"$emulator" = "xxplus4"; then
  cp Android.mk.proto Android.mk
  cp locnet_xplus4/Android.mk.proto locnet_xplus4/Android.mk
  cp locnet_al/Android.mk.proto locnet_al/Android.mk
  cp vice_x64_sc_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_dtv_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_pet_4/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_pet_4/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_pet_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_4_20/Android.mk
  cp vice_x64_sc_scpu_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_scpu_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_scpu_128_2_pet_4_20/Android.mk.proto vice_x64_sc_scpu_128_2_pet_4_20/Android.mk
  cp vice_x64_sc_scpu_128_4/Android.mk.proto vice_x64_sc_scpu_128_4/Android.mk
  cp vice_x64_sc_scpu_128_5x0_4_20/Android.mk.proto vice_x64_sc_scpu_128_5x0_4_20/Android.mk
  cp vice_x64sc_scpu_128_2_pet_4_20/Android.mk.proto vice_x64sc_scpu_128_2_pet_4_20/Android.mk
  cp vice_xplus4/Android.mk.proto vice_xplus4/Android.mk
fi

if test x"$emulator" = "xxcbm5x0"; then
  cp Android.mk.proto Android.mk
  cp locnet_xcbm5x0/Android.mk.proto locnet_xcbm5x0/Android.mk
  cp locnet_al/Android.mk.proto locnet_al/Android.mk
  cp vice_x64_128_5x0/Android.mk.proto vice_x64_128_5x0/Android.mk
  cp vice_x64_dtv_128_5x0/Android.mk.proto vice_x64_dtv_128_5x0/Android.mk
  cp vice_x64_sc_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_dtv_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_pet_4/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_pet_4/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_scpu_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_scpu_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_scpu_128_5x0_4_20/Android.mk.proto vice_x64_sc_scpu_128_5x0_4_20/Android.mk
  cp vice_x128_2_5x0_pet/Android.mk.proto vice_x128_2_5x0_pet/Android.mk
  cp vice_xcbm2_5x0/Android.mk.proto vice_xcbm2_5x0/Android.mk
  cp vice_xcbm5x0/Android.mk.proto vice_xcbm5x0/Android.mk
fi

if test x"$emulator" = "xx128"; then
  cp Android.mk.proto Android.mk
  cp locnet_x128/Android.mk.proto locnet_x128/Android.mk
  cp locnet_al/Android.mk.proto locnet_al/Android.mk
  cp vice_x64_128_5x0/Android.mk.proto vice_x64_128_5x0/Android.mk
  cp vice_x64_dtv_128_5x0/Android.mk.proto vice_x64_dtv_128_5x0/Android.mk
  cp vice_x64_sc_128/Android.mk.proto vice_x64_sc_128/Android.mk
  cp vice_x64_sc_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_pet_4/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_pet_4/Android.mk
  cp vice_x64_sc_dtv_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_dtv_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128/Android.mk.proto vice_x64_sc_dtv_scpu_128/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_pet_4/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_pet_4/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_pet_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_20/Android.mk
  cp vice_x64_sc_scpu_128/Android.mk.proto vice_x64_sc_scpu_128/Android.mk
  cp vice_x64_sc_scpu_128_2/Android.mk.proto vice_x64_sc_scpu_128_2/Android.mk
  cp vice_x64_sc_scpu_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_scpu_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_scpu_128_2_pet_4_20/Android.mk.proto vice_x64_sc_scpu_128_2_pet_4_20/Android.mk
  cp vice_x64_sc_scpu_128_2_pet_20/Android.mk.proto vice_x64_sc_scpu_128_2_pet_20/Android.mk
  cp vice_x64_sc_scpu_128_4/Android.mk.proto vice_x64_sc_scpu_128_4/Android.mk
  cp vice_x64_sc_scpu_128_20/Android.mk.proto vice_x64_sc_scpu_128_20/Android.mk
  cp vice_x64_sc_scpu_128_5x0_4_20/Android.mk.proto vice_x64_sc_scpu_128_5x0_4_20/Android.mk
  cp vice_x64sc_scpu_128_2_pet_4_20/Android.mk.proto vice_x64sc_scpu_128_2_pet_4_20/Android.mk
  cp vice_x128/Android.mk.proto vice_x128/Android.mk
  cp vice_x128_2_5x0_pet/Android.mk.proto vice_x128_2_5x0_pet/Android.mk
fi

if test x"$emulator" = "xxcbm2"; then
  cp Android.mk.proto Android.mk
  cp locnet_xcbm2/Android.mk.proto locnet_xcbm2/Android.mk
  cp locnet_al/Android.mk.proto locnet_al/Android.mk
  cp vice_x64_sc_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_dtv_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_pet_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_pet_4_20/Android.mk
  cp vice_x64_sc_scpu_128_2/Android.mk.proto vice_x64_sc_scpu_128_2/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_pet_4/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_pet_4/Android.mk
  cp vice_x64_sc_scpu_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_scpu_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_scpu_128_2_pet_4_20/Android.mk.proto vice_x64_sc_scpu_128_2_pet_4_20/Android.mk
  cp vice_x64_sc_scpu_128_2_pet_20/Android.mk.proto vice_x64_sc_scpu_128_2_pet_20/Android.mk
  cp vice_x64sc_scpu_128_2_pet_4_20/Android.mk.proto vice_x64sc_scpu_128_2_pet_4_20/Android.mk
  cp vice_x128_2_5x0_pet/Android.mk.proto vice_x128_2_5x0_pet/Android.mk
  cp vice_xcbm2/Android.mk.proto vice_xcbm2/Android.mk
  cp vice_xcbm2_5x0/Android.mk.proto vice_xcbm2_5x0/Android.mk
  cp vice_xcbm2_pet/Android.mk.proto vice_xcbm2_pet/Android.mk
fi

if test x"$emulator" = "xxpet"; then
  cp Android.mk.proto Android.mk
  cp locnet_xpet/Android.mk.proto locnet_xpet/Android.mk
  cp locnet_al/Android.mk.proto locnet_al/Android.mk
  cp vice_x64_sc_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_dtv_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_pet_4/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_pet_4/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_pet_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_pet_4_20/Android.mk
  cp vice_x64_sc_scpu_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_scpu_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_scpu_128_2_pet_4_20/Android.mk.proto vice_x64_sc_scpu_128_2_pet_4_20/Android.mk
  cp vice_x64_sc_scpu_128_2_pet_20/Android.mk.proto vice_x64_sc_scpu_128_2_pet_20/Android.mk
  cp vice_x64sc_scpu_128_2_pet_4_20/Android.mk.proto vice_x64sc_scpu_128_2_pet_4_20/Android.mk
  cp vice_x128_2_5x0_pet/Android.mk.proto vice_x128_2_5x0_pet/Android.mk
  cp vice_xcbm2_pet/Android.mk.proto vice_xcbm2_pet/Android.mk
  cp vice_xpet/Android.mk.proto vice_xpet/Android.mk
fi

if test x"$emulator" = "xall emulators"; then
  cp Android.mk.proto Android.mk
  cp locnet_x64/Android.mk.proto locnet_x64/Android.mk
  cp locnet_x64sc/Android.mk.proto locnet_x64sc/Android.mk
  cp locnet_x64dtv/Android.mk.proto locnet_x64dtv/Android.mk
  cp locnet_x128/Android.mk.proto locnet_x128/Android.mk
  cp locnet_xcbm2/Android.mk.proto locnet_xcbm2/Android.mk
  cp locnet_xcbm5x0/Android.mk.proto locnet_xcbm5x0/Android.mk
  cp locnet_xpet/Android.mk.proto locnet_xpet/Android.mk
  cp locnet_xplus4/Android.mk.proto locnet_xplus4/Android.mk
  cp locnet_xscpu64/Android.mk.proto locnet_xscpu64/Android.mk
  cp locnet_xvic/Android.mk.proto locnet_xvic/Android.mk
  cp locnet_al/Android.mk.proto locnet_al/Android.mk
  cp vice_x64/Android.mk.proto vice_x64/Android.mk
  cp vice_x64_128_5x0/Android.mk.proto vice_x64_128_5x0/Android.mk
  cp vice_x64_dtv_128_5x0/Android.mk.proto vice_x64_dtv_128_5x0/Android.mk
  cp vice_x64_sc/Android.mk.proto vice_x64_sc/Android.mk
  cp vice_x64_sc_128/Android.mk.proto vice_x64_sc_128/Android.mk
  cp vice_x64_sc_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv/Android.mk.proto vice_x64_sc_dtv/Android.mk
  cp vice_x64_sc_dtv_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_dtv_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu/Android.mk.proto vice_x64_sc_dtv_scpu/Android.mk
  cp vice_x64_sc_dtv_scpu_128/Android.mk.proto vice_x64_sc_dtv_scpu_128/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_2_pet_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_2_pet_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_4_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_4_20/Android.mk
  cp vice_x64_sc_dtv_scpu_128_20/Android.mk.proto vice_x64_sc_dtv_scpu_128_20/Android.mk
  cp vice_x64_sc_scpu/Android.mk.proto vice_x64_sc_scpu/Android.mk
  cp vice_x64_sc_scpu_128/Android.mk.proto vice_x64_sc_scpu_128/Android.mk
  cp vice_x64_sc_scpu_128_2/Android.mk.proto vice_x64_sc_scpu_128_2/Android.mk
  cp vice_x64_sc_scpu_128_2_5x0_pet_4_20/Android.mk.proto vice_x64_sc_scpu_128_2_5x0_pet_4_20/Android.mk
  cp vice_x64_sc_scpu_128_2_pet_4_20/Android.mk.proto vice_x64_sc_scpu_128_2_pet_4_20/Android.mk
  cp vice_x64_sc_scpu_128_2_pet_20/Android.mk.proto vice_x64_sc_scpu_128_2_pet_20/Android.mk
  cp vice_x64_sc_scpu_128_4/Android.mk.proto vice_x64_sc_scpu_128_4/Android.mk
  cp vice_x64_sc_scpu_128_20/Android.mk.proto vice_x64_sc_scpu_128_20/Android.mk
  cp vice_x64_sc_scpu_128_5x0_4_20/Android.mk.proto vice_x64_sc_scpu_128_5x0_4_20/Android.mk
  cp vice_x64dtv/Android.mk.proto vice_x64dtv/Android.mk
  cp vice_x64sc/Android.mk.proto vice_x64sc/Android.mk
  cp vice_x64sc_scpu/Android.mk.proto vice_x64sc_scpu/Android.mk
  cp vice_x64sc_scpu_128_2_pet_4_20/Android.mk.proto vice_x64sc_scpu_128_2_pet_4_20/Android.mk
  cp vice_x128/Android.mk.proto vice_x128/Android.mk
  cp vice_x128_2_5x0_pet/Android.mk.proto vice_x128_2_5x0_pet/Android.mk
  cp vice_xcbm2/Android.mk.proto vice_xcbm2/Android.mk
  cp vice_xcbm2_5x0/Android.mk.proto vice_xcbm2_5x0/Android.mk
  cp vice_xcbm2_pet/Android.mk.proto vice_xcbm2_pet/Android.mk
  cp vice_xcbm5x0/Android.mk.proto vice_xcbm5x0/Android.mk
  cp vice_xpet/Android.mk.proto vice_xpet/Android.mk
  cp vice_xplus4/Android.mk.proto vice_xplus4/Android.mk
  cp vice_xscpu64/Android.mk.proto vice_xscpu64/Android.mk
  cp vice_xvic/Android.mk.proto vice_xvic/Android.mk
fi

echo building $emulib
cd ..

rm -f -r assets
mkdir assets
cp assets-proto/gpl.txt assets

if test x"$emulator" = "xx64"; then
  sed -e 's/@VICE@/AnVICE_x64/g' -e 's/@VICE_ROM@/C64 ROM \(KERNAL\)/g' <res-proto/values/strings.xml >res/values/strings.xml
  cp assets-proto/sdl-vicerc-x64 assets/sdl-vicerc
  if test x"$romhandling" = "x$ROMS_EXTERNAL"; then
    cp ../../../../data/C64/sdl_sym.vkm assets/sdl_sym.vkm
  fi
fi

if test x"$emulator" = "xx64sc"; then
  sed -e 's/@VICE@/AnVICE_x64sc/g' -e 's/@VICE_ROM@/C64SC ROM \(KERNAL\)/g' <res-proto/values/strings.xml >res/values/strings.xml
  cp assets-proto/sdl-vicerc-x64sc assets/sdl-vicerc
  if test x"$romhandling" = "x$ROMS_EXTERNAL"; then
    cp ../../../../data/C64/sdl_sym.vkm assets/sdl_sym.vkm
  fi
fi

if test x"$emulator" = "xx64dtv"; then
  sed -e 's/@VICE@/AnVICE_x64dtv/g' -e 's/@VICE_ROM@/C64DTV ROM \(KERNAL\)/g' <res-proto/values/strings.xml >res/values/strings.xml
  cp assets-proto/sdl-vicerc-x64dtv assets/sdl-vicerc
  if test x"$romhandling" = "x$ROMS_EXTERNAL"; then
    cp ../../../../data/C64DTV/sdl_sym.vkm assets/sdl_sym.vkm
  fi
fi

if test x"$emulator" = "xxscpu64"; then
  sed -e 's/@VICE@/AnVICE_xscpu64/g' -e 's/@VICE_ROM@/SCPU64 ROM \(SCPU64\)/g' <res-proto/values/strings.xml >res/values/strings.xml
  cp assets-proto/sdl-vicerc-xscpu64 assets/sdl-vicerc
  if test x"$romhandling" = "x$ROMS_EXTERNAL"; then
    cp ../../../../data/SCPU64/sdl_sym.vkm assets/sdl_sym.vkm
  fi
fi

if test x"$emulator" = "xxvic"; then
  sed -e 's/@VICE@/AnVICE_xvic/g' -e 's/@VICE_ROM@/VIC20 ROM \(KERNAL\)/g' <res-proto/values/strings.xml >res/values/strings.xml
  cp assets-proto/sdl-vicerc-xvic assets/sdl-vicerc
  if test x"$romhandling" = "x$ROMS_EXTERNAL"; then
    cp ../../../../data/VIC20/sdl_sym.vkm assets/sdl_sym.vkm
  fi
fi

if test x"$emulator" = "xxplus4"; then
  sed -e 's/@VICE@/AnVICE_xplus4/g' -e 's/@VICE_ROM@/PLUS4 ROM \(KERNAL\)/g' <res-proto/values/strings.xml >res/values/strings.xml
  cp assets-proto/sdl-vicerc-xplus4 assets/sdl-vicerc
  if test x"$romhandling" = "x$ROMS_EXTERNAL"; then
    cp ../../../../data/PLUS4/sdl_pos.vkm assets/sdl_pos.vkm
  fi
fi

if test x"$emulator" = "xxcbm5x0"; then
  sed -e 's/@VICE@/AnVICE_xcbm5x0/g' -e 's/@VICE_ROM@/CBM5X0 ROM \(KERNAL.500\)/g' <res-proto/values/strings.xml >res/values/strings.xml
  cp assets-proto/sdl-vicerc-xcbm5x0 assets/sdl-vicerc
  if test x"$romhandling" = "x$ROMS_EXTERNAL"; then
    cp ../../../../data/CBM-II/sdl_sym.vkm assets/sdl_sym.vkm
  fi
fi

if test x"$emulator" = "xx128"; then
  sed -e 's/@VICE@/AnVICE_x128/g' -e 's/@VICE_ROM@/C128 ROM \(KERNAL\)/g' <res-proto/values/strings.xml >res/values/strings.xml
  cp assets-proto/sdl-vicerc-x128 assets/sdl-vicerc
  if test x"$romhandling" = "x$ROMS_EXTERNAL"; then
    cp ../../../../data/C128/sdl_sym.vkm assets/sdl_sym.vkm
  fi
fi

if test x"$emulator" = "xxcbm2"; then
  sed -e 's/@VICE@/AnVICE_xcbm2/g' -e 's/@VICE_ROM@/CBM2 ROM \(KERNAL\)/g' <res-proto/values/strings.xml >res/values/strings.xml
  cp assets-proto/sdl-vicerc-xcbm2 assets/sdl-vicerc
  if test x"$romhandling" = "x$ROMS_EXTERNAL"; then
    cp ../../../../data/CBM-II/sdl_sym.vkm assets/sdl_sym.vkm
  fi
fi

if test x"$emulator" = "xxpet"; then
  sed -e 's/@VICE@/AnVICE_xpet/g' -e 's/@VICE_ROM@/PET ROM \(KERNAL4\)/g' <res-proto/values/strings.xml >res/values/strings.xml
  cp assets-proto/sdl-vicerc-xpet assets/sdl-vicerc
  if test x"$romhandling" = "x$ROMS_EXTERNAL"; then
    cp ../../../../data/CBM-II/sdl_sym.vkm assets/sdl_sym.vkm
  fi
fi

if test x"$emulator" = "xall emulators"; then
  sed -e 's/@VICE@/AnVICE/g' -e 's/@VICE_ROM@/No ROM/g' <res-proto/values/strings.xml >res/values/strings.xml
  cat assets-proto/sdl-vicerc-x64 assets-proto/sdl-vicerc-x64sc assets-proto/sdl-vicerc-x64dtv assets-proto/sdl-vicerc-xscpu64 assets-proto/sdl-vicerc-x128 assets-proto/sdl-vicerc-xcbm2 assets-proto/sdl-vicerc-xcbm5x0 assets-proto/sdl-vicerc-xpet assets-proto/sdl-vicerc-xplus4 assets-proto/sdl-vicerc-xvic >assets/sdl-vicerc
  cp res-proto/layout/prefs-allemus.xml res/layout/prefs.xml
else
  if test x"$romhandling" = "x$ROMS_EXTERNAL"; then
    cp res-proto/layout/prefs-externalroms.xml res/layout/prefs.xml
  else
    cp res-proto/layout/prefs-viceroms.xml res/layout/prefs.xml
  fi
fi

if test x"$romhandling" = "x$ROMS_ASSET"; then
  if test x"$emulator" = "xx64" -o x"$emulator" = "xx64sc" -o x"$emulator" = "xall emulators"; then
    for i in $C64_ROMS
    do
      cp ../../../../data/C64/$i assets/c64_$i
    done
  fi

  if test x"$emulator" = "xx64dtv" -o x"$emulator" = "xall emulators"; then
    for i in $C64DTV_ROMS
    do
      cp ../../../../data/C64DTV/$i assets/c64dtv_$i
    done
  fi

  if test x"$emulator" = "xxscpu64" -o x"$emulator" = "xall emulators"; then
    for i in $SCPU64_ROMS
    do
      cp ../../../../data/SCPU64/$i assets/scpu64_$i
    done
  fi

  if test x"$emulator" = "xx128" -o x"$emulator" = "xall emulators"; then
    for i in $C128_ROMS
    do
      cp ../../../../data/C128/$i assets/c128_$i
    done
  fi

  if test x"$emulator" = "xxcbm2" -o x"$emulator" = "xall emulators"; then
    for i in $CBM2_ROMS
    do
      cp ../../../../data/CBM-II/$i assets/cbm-ii_$i
    done
  fi

  if test x"$emulator" = "xxcbm5x0" -o x"$emulator" = "xall emulators"; then
    for i in $CBM5X0_ROMS
    do
      cp ../../../../data/CBM-II/$i assets/cbm-ii_$i
    done
  fi

  if test x"$emulator" = "xxpet" -o x"$emulator" = "xall emulators"; then
    for i in $PET_ROMS
    do
      cp ../../../../data/CBM-II/$i assets/pet_$i
    done
  fi

  if test x"$emulator" = "xxplus4" -o x"$emulator" = "xall emulators"; then
    for i in $PLUS4_ROMS
    do
      cp ../../../../data/PLUS4/$i assets/plus4_$i
    done
  fi

  if test x"$emulator" = "xxvic" -o x"$emulator" = "xall emulators"; then
    for i in $VIC20_ROMS
    do
      cp ../../../../data/VIC20/$i assets/vic20_$i
    done
  fi

  for i in $DRIVE_ROMS
  do
    cp ../../../../data/DRIVES/$i assets/drives_$i
  done
fi

ndk-build -j$(nproc)

echo generating needed java files

sed -e s/@VICE_MACHINE@/$MACHINE/g -e s/@VICE_ROMS@/$romhandling/g <src-proto/com/locnet/vice/PreConfig.java >src/com/locnet/vice/PreConfig.java

if test x"$emulator" = "xx64" -o x"$emulator" = "xall emulators"; then
  sed -e s/@VICE@/x64/g -e s/@VICE_DATA_PATH@/c64/g -e s/@VICE_DATA_FILE@/kernal/g <src-proto/com/locnet/vice/DosBoxLauncher.java >src/com/locnet/vice/DosBoxLauncher.java
fi

if test x"$emulator" = "xx64sc"; then
  sed -e s/@VICE@/x64sc/g -e s/@VICE_DATA_PATH@/c64/g -e s/@VICE_DATA_FILE@/kernal/g <src-proto/com/locnet/vice/DosBoxLauncher.java >src/com/locnet/vice/DosBoxLauncher.java
fi

if test x"$emulator" = "xx64dtv"; then
  sed -e s/@VICE@/x64dtv/g -e s/@VICE_DATA_PATH@/c64dtv/g -e s/@VICE_DATA_FILE@/kernal/g <src-proto/com/locnet/vice/DosBoxLauncher.java >src/com/locnet/vice/DosBoxLauncher.java
fi

if test x"$emulator" = "xxscpu64"; then
  sed -e s/@VICE@/xscpu64/g -e s/@VICE_DATA_PATH@/scpu64/g -e s/@VICE_DATA_FILE@/scpu64/g <src-proto/com/locnet/vice/DosBoxLauncher.java >src/com/locnet/vice/DosBoxLauncher.java
fi

if test x"$emulator" = "xxvic"; then
  sed -e s/@VICE@/xvic/g -e s/@VICE_DATA_PATH@/vic20/g -e s/@VICE_DATA_FILE@/kernal/g <src-proto/com/locnet/vice/DosBoxLauncher.java >src/com/locnet/vice/DosBoxLauncher.java
fi

if test x"$emulator" = "xxplus4"; then
  sed -e s/@VICE@/xplus4/g -e s/@VICE_DATA_PATH@/plus4/g -e s/@VICE_DATA_FILE@/kernal/g <src-proto/com/locnet/vice/DosBoxLauncher.java >src/com/locnet/vice/DosBoxLauncher.java
fi

if test x"$emulator" = "xxcbm5x0"; then
  sed -e s/@VICE@/xcbm5x0/g -e s/@VICE_DATA_PATH@/cbm-ii/g -e s/@VICE_DATA_FILE@/kernal.500/g <src-proto/com/locnet/vice/DosBoxLauncher.java >src/com/locnet/vice/DosBoxLauncher.java
fi

if test x"$emulator" = "xx128"; then
  sed -e s/@VICE@/x128/g -e s/@VICE_DATA_PATH@/c128/g -e s/@VICE_DATA_FILE@/kernal/g <src-proto/com/locnet/vice/DosBoxLauncher.java >src/com/locnet/vice/DosBoxLauncher.java
fi

if test x"$emulator" = "xxcbm2"; then
  sed -e s/@VICE@/xcbm2/g -e s/@VICE_DATA_PATH@/cbm-ii/g -e s/@VICE_DATA_FILE@/kernal/g <src-proto/com/locnet/vice/DosBoxLauncher.java >src/com/locnet/vice/DosBoxLauncher.java
fi

if test x"$emulator" = "xxpet"; then
  sed -e s/@VICE@/xpet/g -e s/@VICE_DATA_PATH@/pet/g -e s/@VICE_DATA_FILE@/kernal4/g <src-proto/com/locnet/vice/DosBoxLauncher.java >src/com/locnet/vice/DosBoxLauncher.java
fi

echo generating apk

if test x"$buildrelease" = "xyes"; then
  cp $curdir/vice-release.keystore ./
  echo >ant.properties "key.store=vice-release.keystore"
  echo >>ant.properties "key.alias=vice_release"
fi

if test x"$builddevrelease" = "xyes"; then
  cp $curdir/vice-dev.keystore ./
  echo >ant.properties "key.store=vice-dev.keystore"
  echo >>ant.properties "key.alias=vice_dev"
fi

if test x"$builddebug" = "xyes"; then
  rm -f ant.properties
  ant debug
  cd ../../../..
  mv src/arch/android/AnVICE/bin/PreConfig-debug.apk ./$emuname-\($CPULABEL\)-$VICEVERSION.apk
else
  ant release
  rm -f vice-*.keystore
  rm -f ant.properties
  cd ../../../..
  mv src/arch/android/AnVICE/bin/PreConfig-release.apk ./$emuname-\($CPULABEL\)-$VICEVERSION.apk
fi

if [ ! -f $emuname-\($CPULABEL\)-$VICEVERSION.apk ]; then
  echo build not completed, check for errors in the output
else
  echo Android port binary generated as $emuname-\($CPULABEL\)-$VICEVERSION.apk
  echo $STATUS_MESSAGE
fi
