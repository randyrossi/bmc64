#!/bin/bash

set -euo pipefail

SRC_DIR=$(cd "$(dirname "$0")" && pwd)
BUILD_DIR="$SRC_DIR/build"
STAGING_DIR="$BUILD_DIR/sdcard"

if ! source "$SRC_DIR/get_gnu_toolchain.sh"
then
    echo "Arm GNU Toolchain setup failed." >&2
    exit 1
fi

BOARDS=()
SKIP_PATCHES=0

for argument in "$@"
do
    case "$argument" in
        pi0|pi2|pi3|pi4)
            BOARDS+=("$argument")
            ;;
        --skip-patches)
            SKIP_PATCHES=1
            ;;
        *)
            echo "Arguments must be board names or --skip-patches: pi0 pi2 pi3 pi4" >&2
            exit 1
            ;;
    esac
done

if [ "${#BOARDS[@]}" -eq 0 ]
then
    BOARDS=(pi0 pi2 pi3)
fi

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

if ! "$SRC_DIR/make_wifi_firmware.sh" "$STAGING_DIR"
then
exit 1
fi

cp "$SRC_DIR/sdcard/config.txt" "$SRC_DIR/sdcard/cmdline.txt" \
    "$SRC_DIR/sdcard/machines.txt" "$STAGING_DIR/"
cp "$SRC_DIR/sdcard/wpa_supplicant.conf.example" \
    "$STAGING_DIR/wpa_supplicant.conf"
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

for machine in c64 c128 vic20 plus4 plus4emu pet
do
    release_path="$SRC_DIR/release/${machine}_release_files"
    machine_dir=${machine^^}

    for release_entry in "$release_path"/*
    do
        if [ "$release_entry" = "$release_path/carts" ]
        then
            mkdir -p "$STAGING_DIR/carts/$machine_dir"
            cp -a "$release_entry/." "$STAGING_DIR/carts/$machine_dir/"
        else
            cp -a "$release_entry" "$STAGING_DIR/"
        fi
    done
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

mkdir -p "$STAGING_DIR/DRIVES" "$STAGING_DIR/prg" "$STAGING_DIR/tmp"

for board in "${BOARDS[@]}"
do
    echo "=============================================================="
    echo "BUILDING $board"
    echo "=============================================================="
    if [ "$SKIP_PATCHES" -eq 1 ]
    then
        echo "Skipping clean_all.sh"
    else
        ./clean_all.sh
    fi
    make_all_arguments=("$board")
    if [ "$SKIP_PATCHES" -eq 1 ]
    then
        make_all_arguments+=(--skip-patches)
    fi
    ./make_all.sh "${make_all_arguments[@]}"
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

check_release_arguments=("$STAGING_DIR")
if [ "${#BOARDS[@]}" -eq 1 ] && [[ "${BOARDS[0]}" =~ ^pi[023]$ ]]
then
    check_release_arguments+=("${BOARDS[0]}")
fi
"$SRC_DIR/check_release_files.sh" "${check_release_arguments[@]}"

echo "Staged boot partition files in $STAGING_DIR"