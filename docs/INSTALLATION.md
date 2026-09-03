# Installation

## Prepare the SD Card

1. Download the release ZIP and extract its contents to the root of a freshly formatted FAT32 SD card.
2. If using a release image instead, write it to the SD card with Raspberry Pi Imager, balenaEtcher, or a similar tool.
3. Insert the SD card into a supported Raspberry Pi and power it on.

For upgrades, back up your existing configuration and ROM files first, then replace the release files on the SD card.

## ROM Files

You must supply the required ROM files from a legal source. BMC64 cannot start without, at minimum, the C64 KERNAL, BASIC, character ROM, and 1541-II drive ROM.

ROM archives often use different filenames. Rename each ROM to the exact filename expected by BMC64. For example, `characters.901225-01.bin` must be renamed to `chargen`.

Copy the ROMs for the machine you intend to use into its corresponding directory. The tables list BMC64 filenames, the corresponding VICE 3.10 filenames, and their purpose. Plus4Emu uses its own ROM format, rather than VICE ROM names.

### C64

| BMC ROM name | VICE 3.10 ROM name | Description |
| --- | --- | --- |
| `kernal` | `kernal-901227-03.bin` | C64 KERNAL ROM |
| `basic` | `basic-901226-01.bin` | C64 BASIC ROM |
| `chargen` | `chargen-901225-01.bin` | C64 character generator ROM |
| `d1541II` | `dos1541ii-251968-03.bin` | 1541-II drive ROM |
| **Optional ROMs** |  |  |
| `dos1541` | `dos1541-325302-01+901229-05.bin` | 1541 drive ROM |
| `dos1571` | `dos1571-310654-05.bin` | 1571 drive ROM |
| `dos1581` | `dos1581-318045-02.bin` | 1581 drive ROM |
| `dos2000` | `dos2000-cs-33cc6f.bin` | CMD FD-2000 drive ROM |
| `dos4000` | `dos4000-fd-350022.bin` | CMD FD-4000 drive ROM |
| `dosCMDHD` | `bootromCMDHD-v2-80.bin` | CMD HD boot ROM |

### C128

| BMC ROM name | VICE 3.10 ROM name | Description |
| --- | --- | --- |
| `kernal` | `kernal-318020-05.bin` | C128 KERNAL ROM |
| `basichi` | `basichi-318019-04.bin` | C128 BASIC high ROM |
| `basiclo` | `basiclo-318018-04.bin` | C128 BASIC low ROM |
| `chargen` | `chargen-390059-01.bin` | C128 character generator ROM |
| `kernal64` | `kernal64-901227-03.bin` | C64-mode KERNAL ROM |
| `basic64` | `basic64-901226-01.bin` | C64-mode BASIC ROM |
| `charg64` | `chargen-901225-01.bin` | C64-mode character generator ROM |
| `d1541II` | `dos1541ii-251968-03.bin` | 1541-II drive ROM |
| **Optional ROMs** |  |  |
| `dos1541` | `dos1541-325302-01+901229-05.bin` | 1541 drive ROM |
| `dos1571` | `dos1571-310654-05.bin` | 1571 drive ROM |
| `dos1581` | `dos1581-318045-02.bin` | 1581 drive ROM |
| `dos2000` | `dos2000-cs-33cc6f.bin` | CMD FD-2000 drive ROM |
| `dos4000` | `dos4000-fd-350022.bin` | CMD FD-4000 drive ROM |
| `dosCMDHD` | `bootromCMDHD-v2-80.bin` | CMD HD boot ROM |

### VIC20

| BMC ROM name | VICE 3.10 ROM name | Description |
| --- | --- | --- |
| `kernal` | `kernal.901486-07.bin` | VIC-20 KERNAL ROM |
| `basic` | `basic-901486-01.bin` | VIC-20 BASIC ROM |
| `chargen` | `chargen-901460-03.bin` | VIC-20 character generator ROM |
| `d1541II` | `dos1541ii-251968-03.bin` | 1541-II drive ROM |
| **Optional ROMs** |  |  |
| `dos1541` | `dos1541-325302-01+901229-05.bin` | 1541 drive ROM |
| `dos1571` | `dos1571-310654-05.bin` | 1571 drive ROM |
| `dos1581` | `dos1581-318045-02.bin` | 1581 drive ROM |
| `dos2000` | `dos2000-cs-33cc6f.bin` | CMD FD-2000 drive ROM |
| `dos4000` | `dos4000-fd-350022.bin` | CMD FD-4000 drive ROM |
| `dosCMDHD` | `bootromCMDHD-v2-80.bin` | CMD HD boot ROM |

### PLUS4

| BMC ROM name | VICE 3.10 ROM name | Description |
| --- | --- | --- |
| `kernal` | `kernal-318004-05.bin` | Plus/4 KERNAL ROM |
| `kernal.005` | `kernal-318005-05.bin` | Plus/4 KERNAL revision .005 |
| `kernal.232` | `kernal-318004-01.bin` | Plus/4 KERNAL revision .232 |
| `kernal.364` | `kernal-364.bin` | Plus/4 KERNAL revision .364 |
| `basic` | `basic-318006-01.bin` | Plus/4 BASIC ROM |
| `3plus1hi` | `3plus1-317054-01.bin` | 3-Plus-1 high ROM |
| `3plus1lo` | `3plus1-317053-01.bin` | 3-Plus-1 low ROM |
| `c2lo.364` | `c2lo-364.bin` | Plus/4 character ROM |
| `d1541II` | `dos1541ii-251968-03.bin` | 1541-II drive ROM |
| `dos1551` | `dos1551-318008-01.bin` | 1551 drive ROM |
| **Optional ROMs** |  |  |
| `dos1541` | `dos1541-325302-01+901229-05.bin` | 1541 drive ROM |
| `dos1571` | `dos1571-310654-05.bin` | 1571 drive ROM |
| `dos1581` | `dos1581-318045-02.bin` | 1581 drive ROM |
| `dos2000` | `dos2000-cs-33cc6f.bin` | CMD FD-2000 drive ROM |
| `dos4000` | `dos4000-fd-350022.bin` | CMD FD-4000 drive ROM |
| `dosCMDHD` | `bootromCMDHD-v2-80.bin` | CMD HD boot ROM |

### PLUS4EMU (Pi 3 only)

| BMC ROM name | VICE 3.10 ROM name | Description |
| --- | --- | --- |
| `p4kernal.rom` | N/A | Plus/4 KERNAL ROM |
| `p4_ntsc.rom` | N/A | Plus/4 NTSC KERNAL ROM |
| `p4_basic.rom` | N/A | Plus/4 BASIC ROM |
| `3plus1.rom` | N/A | 3-Plus-1 ROM |
| `dos1551.rom` | N/A | 1551 drive ROM |
| `dos1541.rom` | N/A | 1541 drive ROM |
| **Optional ROMs** |  |  |
| `p4fileio.rom` | N/A | Plus4Emu file I/O cartridge ROM with tape turbo |

Plus4Emu is a separate emulator from VICE. See the original [Plus4Emu project](https://github.com/istvan-v/plus4emu) for information about its ROM requirements.

### PET

| BMC ROM name | VICE 3.10 ROM name | Description |
| --- | --- | --- |
| `basic1` | `basic-1.901439-09-05-02-06.bin` | PET2001 BASIC 1: 901447-09, -02, -03, -04 |
| `basic2` | `basic-2.901465-01-02.bin` | PET3032 BASIC 2: 901465-01 (two ROMs) |
| `basic4` | `basic-4.901465-23-20-21.bin` | PET4032/8032 BASIC 4: 901465-23, -20, -21 |
| `kernal1` | `kernal-1.901439-04-07.bin` | PET2001 KERNAL: 901447-06 and -07 |
| `kernal2` | `kernal-2.901465-03.bin` | PET3032 KERNAL: 901465-03 |
| `kernal4` | `kernal-4.901465-22.bin` | PET4032/8032 KERNAL: 901465-22 |
| `chargen` | `characters-2.901447-10.bin` | PET character generator: 901447-10 |
| `characters.901640-01.bin` | `characters.901640-01.bin` | SuperPET character generator: 901640-01 |
| `edit1g` | `edit-1-n.901439-03.bin` | PET2001 graphics editor: 901447-05 |
| `edit2b` | `edit-2-b.901474-01.bin` | PET3032 business editor: 901474-01 |
| `edit2g` | `edit-2-n.901447-24.bin` | PET3032 graphics editor: 901447-24 |
| `edit4b40` | `edit-4-40-b-50Hz.ts.bin` | PET4032 business 40-column editor: 901474-02 |
| `edit4b80` | `edit-4-80-b-50Hz.901474-04_.bin` | PET8032 business 80-column editor: 901474-04 |
| `edit4g40` | `edit-4-40-n-50Hz.901498-01.bin` | PET4032 graphics 40-column editor: 901498-01 |
| `hre-9000.324992-02.bin` | `hre-9000.324992-02.bin` | HiRes Emulator ROM: 324992-02 |
| `hre-a000.324993-02.bin` | `hre-a000.324993-02.bin` | HiRes BASIC ROM: 324993-02 |
| `waterloo-a000.901898-01.bin` | `waterloo-a000.901898-01.bin` | SuperPET Waterloo ROM: 901898-01 |
| `waterloo-b000.901898-02.bin` | `waterloo-b000.901898-02.bin` | SuperPET Waterloo ROM: 901898-02 |
| `waterloo-c000.901898-03.bin` | `waterloo-c000.901898-03.bin` | SuperPET Waterloo ROM: 901898-03 |
| `waterloo-d000.901898-04.bin` | `waterloo-d000.901898-04.bin` | SuperPET Waterloo ROM: 901898-04 |
| `waterloo-e000.901897-01.bin` | `waterloo-e000.901897-01.bin` | SuperPET Waterloo ROM: 901897-01 |
| `waterloo-f000.901898-05.bin` | `waterloo-f000.901898-05.bin` | SuperPET Waterloo ROM: 901898-05 |
| `dos2031` | `dos2031-901484-03+05.bin` | 2031 drive ROM |
| **Optional ROMs** |  |  |
| `dos2040` | `dos2040-901468-06+07.bin` | 2040 drive ROM |
| `dos3040` | `dos3040-901468-11-13.bin` | 3040 drive ROM |
| `dos4040` | `dos4040-901468-14-16.bin` | 4040 drive ROM |
| `dos1001` | `dos1001-901887+8-01.bin` | 1001/8050/8250 drive ROM |

These files provide the initial ROM set only. You can later select replacement or custom ROMs, such as JiffyDOS, from the BMC64 menus.

## First Boot

Press `F12` to open the BMC64 menu. When using a compatible real C64 keyboard, `Commodore` + `F7` can be used instead when enabled.

USB devices can be connected, disconnected, or replaced while BMC64 is running. Some gamepads may require configuration from the USB gamepad menu.