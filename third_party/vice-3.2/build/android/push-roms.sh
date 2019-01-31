#!/bin/sh

# see if we are in the top of the tree
if [ ! -f configure.proto ]; then
  cd ../..
  if [ ! -f configure.proto ]; then
    echo "please run this script from the base of the VICE directory"
    exit 1
  fi
fi

curdir=`pwd`

if test x"$1" = "xlist"; then
  adb devices
  exit 1
fi

if test x"$1" = "xhelp" -o x"$1" = "x"; then
  echo "Usage: $0 [[list | help] | <emulator> <path> [<device>]]"
  echo "list: list all devices attached to the computer"
  echo "help: What you are looking at now."
  echo "emulator: x64, x64sc, xscpu64, x64dtv, x128, xcbm2, xcbm5x0, xpet, xplus4, xvic (or all for all emulators)."
  echo "path: Where to place the files, use '/' for internal storage and something like '/sdcard' for external storage."
  echo "device: the name of the device to send the files to."
  echo ""
  echo "No matter what emulator is given the drive roms will always be copied."
  echo ""
  echo "Be aware that this script erases the previous vice directory before copying the roms."
  exit 1
fi

if test x"$1" = "x" -o x"$2" = "x"; then
  echo "too few parameters, please use $0 help"
  exit 1
fi

emulator=$1

if test x"$2" = "x/"; then
  android_path=""
else
  android_path=$2
fi

if test x"$3" != "x"; then
  android_device="-s $3"
else
  android_device=""
fi

C64_FILES="basic c64hq.vpl c64s.vpl ccs64.vpl chargen community-colors.vpl deekay.vpl default.vrs edkernal frodo.vpl godot.vpl gskernal jpchrgen jpkernal kernal pc64.vpl pepto-ntsc.vpl pepto-ntsc-sony.vpl pepto-pal.vpl pepto-palold.vpl ptoing.vpl rgb.vpl sdl_pos.vkm sdl_pos_da.vkm sdl_pos_de.vkm sdl_pos_fi.vkm sdl_pos_no.vkm sdl_sym.vkm sxkernal vice.vpl"
SCPU64_FILES="c64hq.vpl c64s.vpl ccs64.vpl chargen community-colors.vpl deekay.vpl default.vrs frodo.vpl godot.vpl jpchrgen pc64.vpl pepto-ntsc.vpl pepto-ntsc-sony.vpl pepto-pal.vpl pepto-palold.vpl ptoing.vpl rgb.vpl scpu64 sdl_pos.vkm sdl_pos_da.vkm sdl_pos_de.vkm sdl_pos_fi.vkm sdl_pos_no.vkm sdl_sym.vkm vice.vpl"
C64DTV_FILES="basic chargen default.vrs dtvrom.bin kernal sdl_pos.vkm sdl_pos_da.vkm sdl_pos_de.vkm sdl_pos_fi.vkm sdl_pos_no.vkm sdl_sym.vkm spiff.vpl"
C128_FILES="basic64 basichi basiclo c64hq.vpl c64s.vpl ccs64.vpl chargch chargde chargen chargfr chargse community-colors.vpl deekay.vpl default.vrs frodo.vpl godot.vpl kernal kernal64 kernalch kernalde kernalfi kernalfr kernalit kernalno kernalse pc64.vpl pepto-ntsc.vpl pepto-ntsc-sony.vpl pepto-pal.vpl pepto-palold.vpl ptoing.vpl rgb.vpl sdl_pos.vkm vdc_comp.vpl vdc_deft.vpl vice.vpl"
CBM2_FILES="amber.vpl basic.128 basic.256 basic.500 c64hq.vpl c64s.vpl ccs64.vpl chargen.500 chargen.600 chargen.700 community-colors.vpl deekay.vpl frodo.vpl godot.vpl green.vpl kernal kernal.500 pc64.vpl pepto-ntsc.vpl pepto-ntsc-sony.vpl pepto-pal.vpl pepto-palold.vpl ptoing.vpl rgb.vpl rom128h.vrs rom128l.vrs rom256h.vrs rom256l.vrs rom500.vrs sdl_sym.vkm vice.vpl white.vpl"
PET_FILES="amber.vpl basic1 basic2 basic4 characters.901640-01.bin chargen chargen.de edit1g edit2b edit2g edit4b40 edit4b80 edit4g40 green.vpl hre.vrs hre-9000.324992-02.bin hre-a000.324993-02.bin kernal1 kernal2 kernal4 rom1g.vrs rom2b.vrs rom2g.vrs rom4b40.vrs rom4b80.vrs rom4g40.vrs romsuperpet.vrs sdl_buuk_sym.vkm sdl_grus_sym.vkm sdl_sym.vkm waterloo-a000.901898-01.bin waterloo-b000.901898-02.bin waterloo-c000.901898-03.bin waterloo-d000.901898-04.bin waterloo-e000.901897-01.bin waterloo-f000.901898-05.bin white.vpl"
PLUS4_FILES="3plus1hi 3plus1lo basic c2lo.364 default.vrs kernal kernal.005 kernal.232 kernal.364 sdl_pos.vkm vice.vpl"
VIC20_FILES="basic chargen default.vrs kernal mike-ntsc.vpl mike-pal.vpl sdl_pos.vkm sdl_pos_fi.vkm vice.vpl"
DRIVE_FILES="d1541II d1571cr dos1001 dos1540 dos1541 dos1551 dos1570 dos1571 dos1581 dos2031 dos2040 dos3040 dos4040"

`adb $android_device shell rm -r $android_path/vice >/dev/null`
result=`adb $android_device shell mkdir $android_path/vice`

if test x"$result" != "x"; then
  echo "failed to make the vice directory"
  echo $result
  exit 1
fi

if test x"$emulator" = "xx64" -o x"$emulator" = "xx64sc" -o x"$emulator" = "xall"; then
  cd data/C64
  adb $android_device shell mkdir $android_path/vice/c64
  for i in $C64_FILES
  do
    adb $android_device push $i $android_path/vice/c64/$i
  done
  cd ../..
fi

if test x"$emulator" = "xxscpu64" -o x"$emulator" = "xall"; then
  cd data/SCPU64
  adb $android_device shell mkdir $android_path/vice/scpu64
  for i in $SCPU64_FILES
  do
    adb $android_device push $i $android_path/vice/scpu64/$i
  done
  cd ../..
fi

if test x"$emulator" = "xx64dtv" -o x"$emulator" = "xall"; then
  cd data/C64DTV
  adb $android_device shell mkdir $android_path/vice/c64dtv
  for i in $C64DTV_FILES
  do
    adb $android_device push $i $android_path/vice/c64dtv/$i
  done
  cd ../..
fi

if test x"$emulator" = "xx128" -o x"$emulator" = "xall"; then
  cd data/C128
  adb $android_device shell mkdir $android_path/vice/c128
  for i in $C128_FILES
  do
    adb $android_device push $i $android_path/vice/c128/$i
  done
  cd ../..
fi

if test x"$emulator" = "xxcbm2" -o x"$emulator" = "xxcbm5x0" -o x"$emulator" = "xall"; then
  cd data/CBM-II
  adb $android_device shell mkdir $android_path/vice/cbm-ii
  for i in $CBM2_FILES
  do
    adb $android_device push $i $android_path/vice/cbm-ii/$i
  done
  cd ../..
fi

if test x"$emulator" = "xxpet" -o x"$emulator" = "xall"; then
  cd data/PET
  adb $android_device shell mkdir $android_path/vice/pet
  for i in $PET_FILES
  do
    adb $android_device push $i $android_path/vice/pet/$i
  done
  cd ../..
fi

if test x"$emulator" = "xxplus4" -o x"$emulator" = "xall"; then
  cd data/PLUS4
  adb $android_device shell mkdir $android_path/vice/plus4
  for i in $PLUS4_FILES
  do
    adb $android_device push $i $android_path/vice/plus4/$i
  done
  cd ../..
fi

if test x"$emulator" = "xxvic" -o x"$emulator" = "xall"; then
  cd data/VIC20
  adb $android_device shell mkdir $android_path/vice/vic20
  for i in $VIC20_FILES
  do
    adb $android_device push $i $android_path/vice/vic20/$i
  done
  cd ../..
fi

cd data/DRIVES
adb $android_device shell mkdir $android_path/vice/drives
for i in $DRIVE_FILES
do
  adb $android_device push $i $android_path/vice/drives/$i
done
cd ../..

echo "Transfer complete."
