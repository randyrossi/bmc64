# Installation

## Prepare the SD Card

1. Download the release ZIP and extract its contents to the root of a freshly formatted FAT32 SD card.
2. If using a release image instead, write it to the SD card with Raspberry Pi Imager, balenaEtcher, or a similar tool.
3. Insert the SD card into a supported Raspberry Pi and power it on.

For upgrades, back up your existing configuration and ROM files first, then replace the release files on the SD card.

## ROM Files

You must supply the required ROM files from a legal source. BMC64 cannot start without, at minimum, the C64 KERNAL, BASIC, character ROM, and 1541-II drive ROM.

ROM archives often use different filenames. Rename each ROM to the exact filename expected by BMC64. For example, `characters.901225-01.bin` must be renamed to `chargen`. Filenames are case-sensitive.

## Required ROM Filenames

Copy the ROMs for the machine you intend to use into its corresponding directory. The text after each filename identifies the ROM's purpose; PET entries also list the original Commodore ROM image names documented by VICE 3.3.

```text
C64/
    kernal                         C64 KERNAL ROM
    basic                          C64 BASIC ROM
    chargen                        C64 character generator ROM
    d1541II                        1541-II drive ROM: dos1541ii-251968-03.bin
C128/
    kernal                         C128 KERNAL ROM
    basichi                        C128 BASIC high ROM
    basiclo                        C128 BASIC low ROM
    charg64                        C64-mode character generator ROM
    chargen                        C128 character generator ROM
    kernal64                       C64-mode KERNAL ROM
    basic64                        C64-mode BASIC ROM
    d1541II                        1541-II drive ROM: dos1541ii-251968-03.bin
    dos1571                        1571 drive ROM: dos1571-310654-05.bin
VIC20/
    basic                          VIC-20 BASIC ROM
    chargen                        VIC-20 character generator ROM
    kernal                         VIC-20 KERNAL ROM
    d1541II                        1541-II drive ROM: dos1541ii-251968-03.bin
PLUS4/
    kernal                         Plus/4 KERNAL ROM
    kernal.005                     Plus/4 KERNAL revision .005
    kernal.232                     Plus/4 KERNAL revision .232
    3plus1hi                       3-Plus-1 high ROM
    3plus1lo                       3-Plus-1 low ROM
    basic                          Plus/4 BASIC ROM
    c2lo.364                       Plus/4 character ROM
PLUS4EMU/ (Pi 3 only)
    p4kernal.rom                   Plus/4 KERNAL ROM
    p4_ntsc.rom                    Plus/4 NTSC KERNAL ROM
    p4_basic.rom                   Plus/4 BASIC ROM
    3plus1.rom                     3-Plus-1 ROM
    dos1551.rom                    1551 drive ROM
PET/
    basic1                         PET2001 BASIC 1: 901447-09, -02, -03, -04
    basic2                         PET3032 BASIC 2: 901465-01 (two ROMs)
    basic4                         PET4032/8032 BASIC 4: 901465-23, -20, -21
    characters.901640-01.bin       SuperPET character generator: 901640-01
    chargen                        PET character generator: 901447-10
    edit1g                         PET2001 graphics editor: 901447-05
    edit2b                         PET3032 business editor: 901474-01
    edit2g                         PET3032 graphics editor: 901447-24
    edit4b40                       PET4032 business 40-column editor: 901474-02
    edit4b80                       PET8032 business 80-column editor: 901474-04
    edit4g40                       PET4032 graphics 40-column editor: 901498-01
    hre-9000.324992-02.bin         HiRes Emulator ROM: 324992-02
    hre-a000.324993-02.bin         HiRes BASIC ROM: 324993-02
    kernal1                        PET2001 KERNAL: 901447-06 and -07
    kernal2                        PET3032 KERNAL: 901465-03
    kernal4                        PET4032/8032 KERNAL: 901465-22
    waterloo-a000.901898-01.bin    SuperPET Waterloo ROM: 901898-01
    waterloo-b000.901898-02.bin    SuperPET Waterloo ROM: 901898-02
    waterloo-c000.901898-03.bin    SuperPET Waterloo ROM: 901898-03
    waterloo-d000.901897-01.bin    SuperPET Waterloo ROM: 901897-01
    waterloo-e000.901897-01.bin    SuperPET Waterloo ROM: 901897-01
    waterloo-f000.901898-05.bin    SuperPET Waterloo ROM: 901898-05
```

These files provide the initial ROM set only. You can later select replacement or custom ROMs, such as JiffyDOS, from the BMC64 menus.

## First Boot

Press `F12` to open the BMC64 menu. When using a compatible real C64 keyboard, `Commodore` + `F7` can be used instead when enabled.

USB devices can be connected, disconnected, or replaced while BMC64 is running. Some gamepads may require configuration from the USB gamepad menu.