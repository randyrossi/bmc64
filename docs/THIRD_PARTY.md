# Third-Party Software

BMC64 incorporates software and other materials from third-party projects. Third-party components remain subject to their respective copyright and license terms.

The original copyright, license, and attribution notices supplied with these components are retained with their source where applicable.

A bundle of license texts and third-party notices is included in the SD card's
[`licenses/`](../sdcard/licenses/README.txt) folder.

## VICE

BMC64 incorporates source code from the [VICE project](https://vice-emu.sourceforge.io/), version 3.3.

The original VICE 3.3 source archive is available from [SourceForge](https://sourceforge.net/projects/vice-emu/files/releases/vice-3.3.tar.gz/download).

See [`third_party/vice-3.3/`](../third_party/vice-3.3/) for the source incorporated into BMC64 and its accompanying copyright and license information.

## circle-stdlib

BMC64 uses the [circle-stdlib project](https://github.com/smuehlst/circle-stdlib), v21.

See [`third_party/circle-stdlib/`](https://github.com/smuehlst/circle-stdlib/tree/v21) for the version used by BMC64 and its accompanying copyright and license information.

## Plus4Emu

BMC64 incorporates source code from the [Plus4Emu project](https://github.com/istvan-v/plus4emu).

See [`third_party/plus4emu/`](../third_party/plus4emu/) for the source incorporated into BMC64 and its accompanying copyright and license information.

## zlib

BMC64 incorporates source code from [zlib](https://github.com/madler/zlib), v1.3.2.

See [`third_party/zlib/`](../third_party/zlib/) for the source incorporated into BMC64 and its accompanying copyright and license information.

## Raspberry Pi firmware

The boot firmware is supplied by [raspberrypi/firmware](https://github.com/raspberrypi/firmware).
Its redistribution terms are included in
[`RaspberryPi-Firmware.txt`](../sdcard/licenses/RaspberryPi-Firmware.txt).

The Raspberry Pi 4 and 5 builds also include `start4.elf` and `fixup4.dat`,
under the same terms, and the Raspberry Pi 4 and 5 device trees, under the
GPL version 2 ([`GPL-2.0.txt`](../sdcard/licenses/GPL-2.0.txt)).

The Raspberry Pi 4 and 5 boot files in `release/pi45_release_files/` are the
firmware, device trees and overlay that Circle 51.1.1 uses (see its
`boot/Makefile`), from [raspberrypi/firmware](https://github.com/raspberrypi/firmware).
The device trees and overlay are unmodified builds of the device tree sources
in the [Raspberry Pi kernel](https://github.com/raspberrypi/linux) (Linux 6.12).

## GCC runtime

BMC64 links runtime code from the
[Arm GNU Toolchain](https://gitlab.arm.com/tooling/gnu-toolchains-for-arm).
The GCC license section supplied with version 15.2.rel1, including the GCC
Runtime Library Exception 3.1, is included in
[`GCC-Runtime.txt`](../sdcard/licenses/GCC-Runtime.txt).

## Raspberry Pi Pico SDK

[`tools/keyrah_test/pico_sdk_import.cmake`](../tools/keyrah_test/pico_sdk_import.cmake) is an unmodified copy of the [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk)'s `external/pico_sdk_import.cmake`, under its BSD 3-Clause licence; the Keyrah test tool it builds is not part of the SD card.

## BMA800XL

BMC64's Raspberry Pi 4 and 5 support (`src/pi45/`, `src/av/`, `src/kms/`, `src/v3d/`, `tools/v3dshader/`) is taken from [BMA800XL](https://github.com/aminch/bma800xl), under the GNU General Public License version 3. The sections below cover the third-party material that support brings. The license texts they name are in the SD card's [`licenses/`](../sdcard/licenses/README.txt) folder.

### crt-pi

BMC64 uses the crt-pi CRT shader by davej under the GNU General Public License version 2 or later: with OpenGL ES on the Raspberry Pi 0 to 3, and on the Raspberry Pi 4 and Pi 5 compiled for the 3D GPU from [`tools/v3dshader/crt.frag`](../tools/v3dshader/crt.frag). See [`sdcard/licenses/crt-pi.txt`](../sdcard/licenses/crt-pi.txt) and [`sdcard/licenses/GPL-2.0.txt`](../sdcard/licenses/GPL-2.0.txt).

### Mesa

BMC64 uses the [Mesa 3D graphics library](https://mesa3d.org/)'s Broadcom V3D drivers in three ways. The adapted files, with their copyright notices, and Mesa's MIT licence are in [`sdcard/licenses/Mesa-MIT.txt`](../sdcard/licenses/Mesa-MIT.txt).

- **Adapted into the Raspberry Pi 4 and Pi 5 kernels** ([`src/v3d/`](../src/v3d/)): control list packet layouts and packing rules, the rendering job, texture and tile layouts, and the GPU's device facts. The V3D 4.2 (Pi 4) material is from Mesa 26.2.4, the V3D 7.1 (Pi 5) material from Mesa 25.2.8.
- **Adapted into the shader compiler tool** ([`tools/v3dshader/`](../tools/v3dshader/)): the shader preparation that Mesa's drivers do before compiling. The tool is built on a PC against Mesa 25.2.8 and is not part of the SD card.
- **As the shader compiler:** the compiled shaders in `src/v3d/` (`shader_*.h`, `shader71_*.h`) are output of Mesa 25.2.8's V3D compiler. Compiling with Mesa doesn't make them Mesa code: the crt-pi shaders are under crt-pi's licence (above), the others are BMC64's.

### Linux

For the 3D GPU of the Raspberry Pi 4 and Pi 5, BMC64 adapts parts of the Linux kernel's V3D driver and BCM2835 power domain driver ([`src/v3d/`](../src/v3d/)), from the [Raspberry Pi kernel](https://github.com/raspberrypi/linux/tree/rpi-6.18.y) (`rpi-6.18.y`), under the GNU General Public License version 2 or later. See [`sdcard/licenses/Linux-V3D.txt`](../sdcard/licenses/Linux-V3D.txt) and [`sdcard/licenses/GPL-2.0.txt`](../sdcard/licenses/GPL-2.0.txt).

The Raspberry Pi 4 and Pi 5 display implementation ([`src/kms/pi4/`](../src/kms/pi4/), [`src/kms/pi5/`](../src/kms/pi5/) and the HDMI sound in [`src/av/`](../src/av/)) was independently written using hardware information documented in the Linux vc4 driver (`rpi-6.18.y`). No code from that driver was copied or incorporated.

### Circle ARM stub

For the Raspberry Pi 4, BMC64 includes `armstub8-rpi4.bin`, built from [Circle's `boot/armstub`](https://github.com/rsta2/circle/tree/master/boot/armstub) source, under Circle's [GNU General Public License version 3](https://github.com/rsta2/circle/blob/master/COPYING).

### Wi-Fi Firmware

BMC64 includes the Wi-Fi firmware for the Raspberry Pi's wireless chip from the [RPi-Distro firmware-nonfree repository](https://github.com/RPi-Distro/firmware-nonfree), downloaded when BMC64 is built.

See the `firmware/` folder on the SD card for the Broadcom, Cypress and Synaptics licenses that accompany it.

The Pi 5's files (`brcmfmac43455-sdio.raspberrypi,5-model-b.*`) are copies of the `brcmfmac43455-sdio.*` files under the name Circle looks for on the Pi 5, under the same Cypress license.

## Other Third-Party Components

Additional third-party components and supporting materials may be present in the BMC64 source tree. Where such material is accompanied by its own copyright, license, or attribution notices, those notices and terms continue to apply.
