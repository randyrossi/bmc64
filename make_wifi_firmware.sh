#!/bin/bash

set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
WLAN_FIRMWARE_DIR="$SCRIPT_DIR/third_party/circle-stdlib/libs/circle/addon/wlan/firmware"
FIRMWARE_CACHE_DIR="${XDG_CACHE_HOME:-$HOME/.cache}/bmc64/wlan-firmware"
FIRMWARE_LICENSE_URL_BASE="https://raw.githubusercontent.com/RPi-Distro/firmware-nonfree"

WLAN_FIRMWARE=(
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

WLAN_LICENSES=(
    LICENCE.broadcom_bcm43xx
    LICENCE.cypress
    LICENCE.synaptics
)

if [ "$#" -ne 1 ]
then
    echo "Usage: $0 OUTPUT_DIRECTORY" >&2
    exit 2
fi

OUTPUT_DIRECTORY=${1%/}
firmware_revision=$(sed -n 's/^FIRMWARE[[:space:]]*?=[[:space:]]*//p' \
    "$WLAN_FIRMWARE_DIR/Makefile")

if [ -z "$firmware_revision" ]
then
    echo "Could not determine the WLAN firmware revision." >&2
    exit 1
fi

firmware_marker="$FIRMWARE_CACHE_DIR/$firmware_revision.complete"
license_cache_dir="$FIRMWARE_CACHE_DIR/$firmware_revision/licenses"
copyright_cache_file="$license_cache_dir/copyright"
firmware_missing=0

for firmware in "${WLAN_FIRMWARE[@]}"
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
for license in "${WLAN_LICENSES[@]}"
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

mkdir -p "$OUTPUT_DIRECTORY/firmware"

for firmware in "${WLAN_FIRMWARE[@]}"
do
    cp "$WLAN_FIRMWARE_DIR/$firmware" "$OUTPUT_DIRECTORY/firmware/"
done

for license in "${WLAN_LICENSES[@]}"
do
    cp "$license_cache_dir/$license" "$OUTPUT_DIRECTORY/firmware/"
done