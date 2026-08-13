#!/bin/bash

set -euo pipefail

SRC_DIR=$(cd "$(dirname "$0")" && pwd)
BUILD_DIR="$SRC_DIR/build"
STAGING_DIR="$BUILD_DIR/sdcard"
WLAN_FIRMWARE_DIR="$SRC_DIR/third_party/circle-stdlib/libs/circle/addon/wlan/firmware"
FIRMWARE_CACHE_DIR="${XDG_CACHE_HOME:-$HOME/.cache}/bmc64/wlan-firmware"
FIRMWARE_LICENSE_URL_BASE="https://raw.githubusercontent.com/RPi-Distro/firmware-nonfree"

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

pi3_wlan_licenses=(
    LICENCE.broadcom_bcm43xx
    LICENCE.cypress
    LICENCE.synaptics
)

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

if true
then
    firmware_revision=$(sed -n 's/^FIRMWARE[[:space:]]*?=[[:space:]]*//p' \
        "$WLAN_FIRMWARE_DIR/Makefile")
    firmware_marker="$FIRMWARE_CACHE_DIR/$firmware_revision.complete"
    license_cache_dir="$FIRMWARE_CACHE_DIR/$firmware_revision/licenses"
    copyright_cache_file="$license_cache_dir/copyright"
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

    license_missing=0
    for license in "${pi3_wlan_licenses[@]}"
    do
        if [ ! -s "$license_cache_dir/$license" ]
        then
            license_missing=1
            break
        fi
    done

    if [ "$license_missing" -eq 1 ]
    then
        mkdir -p "$license_cache_dir"
        if [ ! -s "$copyright_cache_file" ]
        then
            curl -fsSL --user-agent "Mozilla/5.0 (Windows NT 10.0; Win64; x64)" \
                "$FIRMWARE_LICENSE_URL_BASE/$firmware_revision/debian/copyright" \
                -o "$copyright_cache_file"
        fi

        extract_license()
        {
            local license_name=$1
            local output_file=$2
            awk -v license_name="$license_name" '
                $0 == "License: " license_name {
                    printing = 1
                    license_text = $0 ORS
                    next
                }
                printing && /^License:/ { printing = 0 }
                printing { license_text = license_text $0 ORS }
                END { printf "%s", license_text }
            ' "$copyright_cache_file" > "$output_file"
            test -s "$output_file"
        }

        extract_license "binary-redist-Broadcom-wifi" \
            "$license_cache_dir/LICENCE.broadcom_bcm43xx"
        extract_license "binary-redist-Cypress" \
            "$license_cache_dir/LICENCE.cypress"
        extract_license "Synaptics" "$license_cache_dir/LICENCE.synaptics"
    fi

    mkdir -p "$STAGING_DIR/firmware"

    for firmware in "${pi3_wlan_firmware[@]}"
    do
        cp "$WLAN_FIRMWARE_DIR/$firmware" "$STAGING_DIR/firmware/"
    done
    for license in "${pi3_wlan_licenses[@]}"
    do
        cp "$license_cache_dir/$license" "$STAGING_DIR/firmware/"
    done
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

for machine in c64 c128 vic20 plus4
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

mkdir -p "$STAGING_DIR/tmp"

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

echo "Staged boot partition files in $STAGING_DIR"