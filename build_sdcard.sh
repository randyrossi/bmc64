#!/bin/bash

set -euo pipefail

SRC_DIR=$(cd "$(dirname "$0")" && pwd)
BUILD_DIR="$SRC_DIR/build"
STAGING_DIR="$BUILD_DIR/sdcard"
WLAN_FIRMWARE_DIR="$SRC_DIR/third_party/circle-stdlib/libs/circle/addon/wlan/firmware"
FIRMWARE_CACHE_DIR="${XDG_CACHE_HOME:-$HOME/.cache}/bmc64/wlan-firmware"

pi3_wlan_firmware=(
    brcmfmac43430-sdio.bin
    brcmfmac43430-sdio.txt
    brcmfmac43430-sdio.clm_blob
    brcmfmac43436-sdio.bin
    brcmfmac43436-sdio.txt
    brcmfmac43436-sdio.clm_blob
    brcmfmac43436s-sdio.bin
    brcmfmac43436s-sdio.txt
	brcmfmac43455-sdio.bin
	brcmfmac43455-sdio.clm_blob
	brcmfmac43455-sdio.txt    
)

if [ "$#" -eq 0 ]
then
    BOARDS=(pi0 pi2 pi3)
else
    BOARDS=("$@")
fi

for board in "${BOARDS[@]}"
do
    case "$board" in
        pi0|pi2|pi3|pi4)
            ;;
        *)
            echo "Board arguments must be: pi0 pi2 pi3 pi4" >&2
            exit 1
            ;;
    esac
done

kernel_for_board()
{
    case "$1" in
        pi0) echo "kernel.img" ;;
        pi2) echo "kernel7.img" ;;
        pi3) echo "kernel8-32.img" ;;
        pi4) echo "kernel7l.img" ;;
    esac
}

cd "$SRC_DIR"

rm -rf "$STAGING_DIR"
mkdir -p "$STAGING_DIR"

if [[ " ${BOARDS[*]} " == *" pi3 "* ]]
then
    firmware_revision=$(sed -n 's/^FIRMWARE[[:space:]]*?=[[:space:]]*//p' \
        "$WLAN_FIRMWARE_DIR/Makefile")
    firmware_marker="$FIRMWARE_CACHE_DIR/$firmware_revision.complete"
    firmware_missing=0

    for firmware in "${pi3_wlan_firmware[@]}"
    do
        if [ ! -s "$WLAN_FIRMWARE_DIR/$firmware" ]
        then
            firmware_missing=1
            break
        fi
    done

    if [ ! -f "$firmware_marker" ] || [ "$firmware_missing" -eq 1 ]
    then
        make -C "$WLAN_FIRMWARE_DIR" firmware
        mkdir -p "$FIRMWARE_CACHE_DIR"
        touch "$firmware_marker"
    fi
    mkdir -p "$STAGING_DIR/firmware"

    for firmware in "${pi3_wlan_firmware[@]}"
    do
        cp "$WLAN_FIRMWARE_DIR/$firmware" "$STAGING_DIR/firmware/"
    done
fi

cp "$SRC_DIR/sdcard/config.txt" "$SRC_DIR/sdcard/cmdline.txt" \
    "$SRC_DIR/sdcard/machines.txt" "$STAGING_DIR/"
cp -a "$SRC_DIR/release/common_release_files/." "$STAGING_DIR/"
cp "$SRC_DIR/LICENSE" "$SRC_DIR/README.md" "$STAGING_DIR/"

machine_dirs=(
    c64:C64
    c128:C128
    vic20:VIC20
    plus4:PLUS4
    plus4emu:PLUS4EMU
    pet:PET
)

for machine_dir in "${machine_dirs[@]}"
do
    source_dir=${machine_dir%%:*}
    target_dir=${machine_dir##*:}
    mkdir -p "$STAGING_DIR/$target_dir"
    cp -a "$SRC_DIR/sdcard/$source_dir/." "$STAGING_DIR/$target_dir/"
done

for release_dir in c64_release_files c128_release_files vic20_release_files plus4_release_files
do
    cp -a "$SRC_DIR/release/$release_dir/." "$STAGING_DIR/"
done

for data_dir in disks tapes snapshots
do
    for machine_dir in C64 C128 VIC20 PLUS4 PET
    do
        mkdir -p "$STAGING_DIR/$data_dir/$machine_dir"
    done
done

for machine_dir in C64 C128 VIC20 PLUS4
do
    mkdir -p "$STAGING_DIR/carts/$machine_dir"
done

mkdir -p "$STAGING_DIR/tmp"

for board in "${BOARDS[@]}"
do
    echo "=============================================================="
    echo "BUILDING $board"
    echo "=============================================================="
    ./clean_all.sh
    ./make_all.sh "$board"
    ./make_machines.sh "$board"
    kernel=$(kernel_for_board "$board")
    machines=(c64 c128 vic20 plus4 pet)

    if [ "$board" = "pi3" ]
    then
        machines+=(plus4emu)
    fi

    for machine in "${machines[@]}"
    do
        image="$SRC_DIR/$kernel.$machine"
        if [ ! -f "$image" ]
        then
            echo "Expected image was not created: $image" >&2
            exit 1
        fi
        cp "$image" "$STAGING_DIR/"
    done

    mv "$STAGING_DIR/$kernel.c64" "$STAGING_DIR/$kernel"
done

echo "Staged boot partition files in $STAGING_DIR"