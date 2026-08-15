#!/bin/bash

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)

# Extract version from the menu.c file
SOURCE_VERSION=$(sed -n 's/^#define VERSION_STRING "\(.*\)"$/\1/p' pi3/third_party/common/menu.c)
VERSION="v${SOURCE_VERSION// /-}"

if [ -z "$SOURCE_VERSION" ]
then
echo "Could not read VERSION_STRING from pi3/third_party/common/menu.c" >&2
exit 1
fi

SRC_PI3=pi3
SRC_PI2=pi2
SRC_PI0=pi0

COUNT2=`ls -l $SRC_PI2/*.img.* | wc -l`
COUNT3=`ls -l $SRC_PI3/*.img.* | wc -l`
COUNT0=`ls -l $SRC_PI0/*.img.* | wc -l`

if [ "$COUNT2" != "6" ]
then
exit -1
fi

if [ "$COUNT3" != "6" ]
then
exit -1
fi

if [ "$COUNT0" != "6" ]
then
exit -1
fi

export MACHINE_C64_L=c64
export MACHINE_C64_U=C64
export MACHINE_C128_L=c128
export MACHINE_C128_U=C128
export MACHINE_VIC20_L=vic20
export MACHINE_VIC20_U=VIC20
export MACHINE_PLUS4_L=plus4
export MACHINE_PLUS4EMU_L=plus4emu
export MACHINE_PLUS4_U=PLUS4
export MACHINE_PLUS4EMU_U=PLUS4EMU
export MACHINE_PET_L=pet
export MACHINE_PET_U=PET

rm -rf stage_dir

# Create the staging directory structure
mkdir -p stage_dir
for data_dir in disks tapes snapshots
do
	for machine_dir in "$MACHINE_C64_U" "$MACHINE_C128_U" "$MACHINE_VIC20_U" "$MACHINE_PLUS4_U" "$MACHINE_PET_U"
	do
		mkdir -p "stage_dir/$data_dir/$machine_dir"
	done
done

for machine_dir in "$MACHINE_C64_U" "$MACHINE_C128_U" "$MACHINE_VIC20_U" "$MACHINE_PLUS4_U"
do
	mkdir -p "stage_dir/carts/$machine_dir"
done
mkdir -p stage_dir/DRIVES stage_dir/prg stage_dir/tmp


cp -R ${SRC_PI3}/release/common_release_files/* stage_dir/
cp -R ${SRC_PI3}/release/${MACHINE_C64_L}_release_files/C64 stage_dir/
cp ${SRC_PI3}/release/${MACHINE_C64_L}_release_files/carts/* stage_dir/carts/$MACHINE_C64_U/
cp -R ${SRC_PI3}/release/${MACHINE_C128_L}_release_files/* stage_dir/
cp -R ${SRC_PI3}/release/${MACHINE_VIC20_L}_release_files/* stage_dir/
cp -R ${SRC_PI3}/release/${MACHINE_PLUS4_L}_release_files/* stage_dir/
cp -R ${SRC_PI3}/release/${MACHINE_PLUS4EMU_L}_release_files/* stage_dir/
cp -R ${SRC_PI3}/release/${MACHINE_PET_L}_release_files/* stage_dir/

cp ${SRC_PI0}/kernel.img.${MACHINE_C64_L} stage_dir/kernel.img
cp ${SRC_PI0}/kernel.img.${MACHINE_C128_L} stage_dir
cp ${SRC_PI0}/kernel.img.${MACHINE_VIC20_L} stage_dir
cp ${SRC_PI0}/kernel.img.${MACHINE_PLUS4_L} stage_dir
cp ${SRC_PI0}/kernel.img.${MACHINE_PET_L} stage_dir

cp ${SRC_PI2}/kernel7.img.${MACHINE_C64_L} stage_dir/kernel7.img
cp ${SRC_PI2}/kernel7.img.${MACHINE_C128_L} stage_dir
cp ${SRC_PI2}/kernel7.img.${MACHINE_VIC20_L} stage_dir
cp ${SRC_PI2}/kernel7.img.${MACHINE_PLUS4_L} stage_dir
cp ${SRC_PI2}/kernel7.img.${MACHINE_PET_L} stage_dir

cp ${SRC_PI3}/kernel8-32.img.${MACHINE_C64_L} stage_dir/kernel8-32.img
cp ${SRC_PI3}/kernel8-32.img.${MACHINE_C128_L} stage_dir
cp ${SRC_PI3}/kernel8-32.img.${MACHINE_VIC20_L} stage_dir
cp ${SRC_PI3}/kernel8-32.img.${MACHINE_PLUS4_L} stage_dir
cp ${SRC_PI3}/kernel8-32.img.${MACHINE_PLUS4EMU_L} stage_dir
cp ${SRC_PI3}/kernel8-32.img.${MACHINE_PET_L} stage_dir

cp ${SRC_PI3}/sdcard/$MACHINE_C64_L/* stage_dir/$MACHINE_C64_U
cp ${SRC_PI3}/sdcard/$MACHINE_C128_L/* stage_dir/$MACHINE_C128_U
cp ${SRC_PI3}/sdcard/$MACHINE_VIC20_L/* stage_dir/$MACHINE_VIC20_U
cp ${SRC_PI3}/sdcard/$MACHINE_PLUS4_L/* stage_dir/$MACHINE_PLUS4_U
cp ${SRC_PI3}/sdcard/$MACHINE_PLUS4EMU_L/* stage_dir/$MACHINE_PLUS4EMU_U
cp ${SRC_PI3}/sdcard/$MACHINE_PET_L/* stage_dir/$MACHINE_PET_U

cp ${SRC_PI3}/sdcard/machines.txt stage_dir
cp ${SRC_PI3}/sdcard/config.txt stage_dir
cp ${SRC_PI3}/sdcard/cmdline.txt stage_dir

cp ${SRC_PI3}/LICENSE stage_dir
cp ${SRC_PI3}/README.md stage_dir

if ! "$SCRIPT_DIR/check_release_files.sh" stage_dir
then
exit 1
fi

cd stage_dir
zip -r ../bmc64-${VERSION}.files.zip .
cd ..
rm -rf stage_dir
