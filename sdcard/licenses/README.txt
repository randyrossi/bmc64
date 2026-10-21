Licenses and notices
===================

BMC64's main license is the GNU General Public License version 3.
The complete text is in LICENSE at the root of the card and GPL-3.0.txt
in this folder. The OSD's License screen also displays the full license
and points to the licenses/ and firmware/ notice folders.

This folder contains license texts and notices for included software.

  GPL-3.0.txt            BMC64's main license; also used by Circle.
  GPL-2.0.txt            GPL version 2 for upstream notices, including
                        VICE, Plus4Emu, reSID, crt-pi, the Linux code in
                        the Raspberry Pi 4 and 5 GPU driver and the Pi 4
                        and 5 device trees. Their applicable "or later"
                        grants remain in the source notices.
  VICE.txt              VICE 3.3's README, copyright and author credits.
  Plus4Emu.txt           Plus4Emu's README, copyright and author credits.
                        It also describes desktop features/dependencies
                        that are not included in this bare-metal build.
  Apache-2.0.txt         Apache License 2.0 for BMC64 integration files
                        carrying Apache 2.0 headers.
  LGPL-2.1.txt           Full LGPL version 2.1 text.
  dotconf.txt            Plus4Emu's bundled dot.conf parser notice.
  libsndfile-interface.txt
                        Notice for Plus4Emu's libsndfile interface.
                        BMC64 uses a local stub, not full libsndfile.
  zlib.txt              zlib decompression code used by the updater.
  crt-pi.txt            davej's CRT shader notice.
  Linux-V3D.txt         Notices of the Linux V3D and BCM2835 power driver
                        files the Raspberry Pi 4 and 5 GPU driver adapts.
  Mesa-MIT.txt          Notices and MIT license of the Mesa files the
                        Raspberry Pi 4 and 5 GPU driver adapts.
  Bescii-CC0.txt         CC0 text for Damian Vila's Bescii Mono glyphs.
  newlib.txt            Aggregate newlib C/math library notices.
  libgloss.txt          Aggregate libgloss runtime-adaptation notices.
  GCC-Runtime.txt       GCC license section supplied with Arm GNU
                        Toolchain 15.2.rel1, including the GCC Runtime
                        Library Exception 3.1. Covers runtime licensing;
                        some listed GCC components are not linked.
  RaspberryPi-Firmware.txt
                        Broadcom/Raspberry Pi redistribution terms for
                        bootcode.bin, start.elf and fixup.dat, and the
                        Raspberry Pi 4's start4.elf and fixup4.dat.

Circle and VICE are upstream components, not newly licensed by BMC64.
Their original per-file notices remain in the corresponding source.
Circle's main sources declare GPL version 3 or later; VICE's main
sources declare GPL version 2 or later. Individual exceptions exist.

Wi-Fi vendor license extracts are also supplied in firmware/:
  LICENCE.broadcom_bcm43xx
  LICENCE.cypress
  LICENCE.synaptics

Source
------
https://github.com/randyrossi/bmc64

Third-party software information:
https://github.com/randyrossi/bmc64/blob/master/docs/THIRD_PARTY.md

License texts alone do not replace GPL/LGPL corresponding-source
obligations. Obtain the source, dependency revisions and patches
matching the particular release.
