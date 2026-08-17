#!/bin/bash

# Required files from the known-good BMC64 4.2 release.
REQUIRED_FILES=(
    # Root support files
    "LICENSE"
    "README.md"
    "bootcode.bin"
    "cmdline.txt"
    "config.txt"
    "fixup.dat"
    "machines.txt"
    "start.elf"

    # Wi-Fi firmware and redistribution licences
    "firmware/brcmfmac43430-sdio.bin"
    "firmware/brcmfmac43430-sdio.clm_blob"
    "firmware/brcmfmac43430-sdio.txt"
    "firmware/brcmfmac43436-sdio.bin"
    "firmware/brcmfmac43436-sdio.clm_blob"
    "firmware/brcmfmac43436-sdio.txt"
    "firmware/brcmfmac43436s-sdio.bin"
    "firmware/brcmfmac43436s-sdio.txt"
    "firmware/brcmfmac43455-sdio.bin"
    "firmware/brcmfmac43455-sdio.clm_blob"
    "firmware/brcmfmac43455-sdio.txt"
    "firmware/LICENCE.broadcom_bcm43xx"
    "firmware/LICENCE.cypress"
    "firmware/LICENCE.synaptics"

    # C64 files
    "C64/PUT_ROMS_HERE"
    "C64/bootstat.txt"
    "C64/rpi_maxi_pos.vkm"
    "C64/rpi_petsciiboard_sym.vkm"
    "C64/rpi_pos.vkm"
    "C64/rpi_sym.vkm"

    # C128 files
    "C128/PUT_ROMS_HERE"
    "C128/bootstat.txt"
    "C128/rpi_maxi_pos.vkm"
    "C128/rpi_pos.vkm"
    "C128/rpi_sym.vkm"

    # VIC20 files
    "VIC20/PUT_ROMS_HERE"
    "VIC20/bootstat.txt"
    "VIC20/rpi_maxi_pos.vkm"
    "VIC20/rpi_pos.vkm"
    "VIC20/rpi_sym.vkm"

    # Plus/4 files
    "PLUS4/PUT_ROMS_HERE"
    "PLUS4/bootstat.txt"
    "PLUS4/rpi_maxi_pos.vkm"
    "PLUS4/rpi_pos.vkm"
    "PLUS4/rpi_sym.vkm"

    # Plus/4 emulator files
    "PLUS4EMU/PUT_ROMS_HERE"
    "PLUS4EMU/README.md"
    "PLUS4EMU/rpi_c16_keyrah_pos.vkm"
    "PLUS4EMU/rpi_maxi_pos.vkm"
    "PLUS4EMU/rpi_pos.vkm"

    # PET files
    "PET/PUT_ROMS_HERE"
    "PET/rpi_buus_pos.vkm"
    "PET/rpi_buus_sym.vkm"
    "PET/rpi_grus_pos.vkm"
    "PET/rpi_grus_sym.vkm"

    # Cartridge files
    "carts/C64/delayjoy.crt"
    "carts/C64/delaykey.crt"
)

PI0_KERNEL_FILES=(
    "kernel.img"
    "kernel.img.c128"
    "kernel.img.pet"
    "kernel.img.plus4"
    "kernel.img.vic20"
)

PI2_KERNEL_FILES=(
    "kernel7.img"
    "kernel7.img.c128"
    "kernel7.img.pet"
    "kernel7.img.plus4"
    "kernel7.img.vic20"
)

PI3_KERNEL_FILES=(
    "kernel8-32.img"
    "kernel8-32.img.c128"
    "kernel8-32.img.pet"
    "kernel8-32.img.plus4"
    "kernel8-32.img.plus4emu"
    "kernel8-32.img.vic20"
)

# Required directories from the known-good BMC64 4.2 release.
REQUIRED_DIRECTORIES=(
    # Machine directories
    "C64"
    "C128"
    "VIC20"
    "PLUS4"
    "PLUS4EMU"
    "PET"

    # Cartridge directories
    "carts"
    "carts/C64"
    "carts/C128"
    "carts/VIC20"
    "carts/PLUS4"

    # User media directories
    "disks"
    "disks/C64"
    "disks/C128"
    "disks/VIC20"
    "disks/PLUS4"
    "disks/PET"
    "tapes"
    "tapes/C64"
    "tapes/C128"
    "tapes/VIC20"
    "tapes/PLUS4"
    "tapes/PET"
    "prg"

    # Snapshot directories
    "snapshots"
    "snapshots/C64"
    "snapshots/C128"
    "snapshots/VIC20"
    "snapshots/PLUS4"
    "snapshots/PET"

    # Other empty release directories
    "DRIVES"
    "tmp"
)

if [ "$#" -lt 1 ] || [ "$#" -gt 2 ]
then
    echo "Usage: $0 RELEASE_DIRECTORY [pi0|pi2|pi3]" >&2
    exit 2
fi

RELEASE_DIRECTORY=${1%/}
PI_MODEL=${2:-}

if [ ! -d "$RELEASE_DIRECTORY" ]
then
    echo "Release directory does not exist: $RELEASE_DIRECTORY" >&2
    exit 2
fi

case "$PI_MODEL" in
    "")
        REQUIRED_FILES+=("${PI0_KERNEL_FILES[@]}" "${PI2_KERNEL_FILES[@]}" "${PI3_KERNEL_FILES[@]}")
        ;;
    pi0)
        REQUIRED_FILES+=("${PI0_KERNEL_FILES[@]}")
        ;;
    pi2)
        REQUIRED_FILES+=("${PI2_KERNEL_FILES[@]}")
        ;;
    pi3)
        REQUIRED_FILES+=("${PI3_KERNEL_FILES[@]}")
        ;;
    *)
        echo "Pi model must be pi0, pi2, or pi3: $PI_MODEL" >&2
        exit 2
        ;;
esac

missing_entries=0

for required_file in "${REQUIRED_FILES[@]}"
do
    if [ ! -f "$RELEASE_DIRECTORY/$required_file" ]
    then
        echo "Missing: $required_file"
        missing_entries=1
    fi
done

for required_directory in "${REQUIRED_DIRECTORIES[@]}"
do
    if [ ! -d "$RELEASE_DIRECTORY/$required_directory" ]
    then
        echo "Missing directory: $required_directory"
        missing_entries=1
    fi
done

if [ "$missing_entries" -ne 0 ]
then
    echo "Release check failed."
    exit 1
fi

echo "Release check passed: ${#REQUIRED_FILES[@]} required files and ${#REQUIRED_DIRECTORIES[@]} required directories found."