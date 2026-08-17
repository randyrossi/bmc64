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
MACHINE=""
KASAN=0

while [ "$#" -gt 0 ]
do
    argument=$1
    case "$argument" in
        pi0|pi2|pi3|pi4)
            BOARDS+=("$argument")
            ;;
        --machine)
            shift
            if [ "$#" -eq 0 ]
            then
                echo "--machine requires one of: c64 c128 vic20 plus4 plus4emu pet" >&2
                exit 1
            fi
            MACHINE=$1
            ;;
        --machine=c64|--machine=c128|--machine=vic20|--machine=plus4|--machine=plus4emu|--machine=pet)
            MACHINE=${argument#--machine=}
            ;;
        --skip-patches)
            SKIP_PATCHES=1
            ;;
        --kasan)
            KASAN=1
            ;;
        *)
            echo "Arguments must be board names, --machine MACHINE, or --skip-patches" >&2
            exit 1
            ;;
    esac
    shift
done

if [ "${#BOARDS[@]}" -eq 0 ]
then
    BOARDS=(pi0 pi2 pi3)
fi

if [ -n "$MACHINE" ]
then
    case "$MACHINE" in
        c64|c128|vic20|plus4|plus4emu|pet) ;;
        *)
            echo "Machine must be one of: c64 c128 vic20 plus4 plus4emu pet" >&2
            exit 1
            ;;
    esac
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

makefile_for_machine()
{
    case "$1" in
        c64) echo "C64" ;;
        c128) echo "C128" ;;
        vic20) echo "VIC20" ;;
        plus4) echo "Plus4" ;;
        plus4emu) echo "Plus4Emu" ;;
        pet) echo "PET" ;;
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
    if [ "$KASAN" -eq 1 ]
    then
        make_all_arguments+=(--kasan)
    fi
    ./make_all.sh "${make_all_arguments[@]}"
    kernel=$(kernel_for_board "$board")

    if [ -n "$MACHINE" ]
    then
        machine_name=$(makefile_for_machine "$MACHINE")
        echo "Building $machine_name -> $kernel.$MACHINE"
        make clean
        BOARD=$board make -f "Makefile-$machine_name"

        image_artifact="$SRC_DIR/$kernel"
        if [ ! -f "$image_artifact" ]
        then
            echo "Expected artifact was not created: $image_artifact" >&2
            exit 1
        fi
        cp "$image_artifact" "$image_artifact.$MACHINE"

        artifact_base=${kernel%.img}
        for artifact in elf lst map
        do
            source_artifact="$SRC_DIR/$artifact_base.$artifact"
            if [ ! -f "$source_artifact" ]
            then
                echo "Expected artifact was not created: $source_artifact" >&2
                exit 1
            fi
            cp "$source_artifact" "$source_artifact.$MACHINE"
        done

        cp "$SRC_DIR/$kernel.$MACHINE" "$STAGING_DIR/$kernel.$MACHINE"
        printf '\nkernel=%s.%s\n' "$kernel" "$MACHINE" >> "$STAGING_DIR/config.txt"
    else
        ./make_machines.sh "$board"
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
    fi
done

if [ -z "$MACHINE" ]
then
    check_release_arguments=("$STAGING_DIR")
    if [ "${#BOARDS[@]}" -eq 1 ] && [[ "${BOARDS[0]}" =~ ^pi[023]$ ]]
    then
        check_release_arguments+=("${BOARDS[0]}")
    fi
    "$SRC_DIR/check_release_files.sh" "${check_release_arguments[@]}"
else
    echo "Skipping full-release check for selected machine: $MACHINE"
fi

echo "Staged boot partition files in $STAGING_DIR"