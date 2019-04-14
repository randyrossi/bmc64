__IMPORTANT PSA: BMC64 v1.0.6 through v1.4 were not properly putting the other 3 (unused) cores to a low powered mode and was causing CPU temperatures to rise close to or beyond automatic throttling limits. I'm very sorry about this. I don't believe this pushed the devices beyond their limits, it was just a waste of heat. The Pi will automatically throttle itself if CPU temperature goes beyond a certain limit. If you saw thermometer icons in the top right corner of your screen, this is probably why. V1.5+ fixes this.__

__If you are using an older version, I strongly recommend you update to v1.5__

# BMC64

BMC64 is a bare metal C64 emulator for the Raspberry Pi with true 50hz/60hz smooth scrolling and low latency between input & video/audio.

# Features
  * Quick boot time (5 seconds!)
  * Frames are timed to vsync for true 50/60 hz smooth scrolling (no horizontal tearing!)
  * Low latency between input & audio/video
  * No shutdown sequence required, just power off
  * High compatibility thanks to VICE
  * Can wire real Commodore/Atari joysticks via GPIO
  * Keyrah friendly

# Limitations
  * USB gamepad support is limited

This project uses VICE for emulation without any O/S (Linux) distribution installed on the Pi.  VICE (Versatile Commodore Emulator) platform dependencies are satisfied using circle-stdlib.

# Known Issues

  * There is no hot plug in/out support for USB devices.  All devices must be plugged in before the device is booted and never removed.  Attempting to remove them will halt the emulator or make it slow down considerably.

  * Some USB gamepads will require manual tweaking of settings from the defaults. 

# Precompiled Images

  * https://accentual.com/bmc64

# Github Link

  * https://github.com/randyrossi/bmc64

# Timing

The machine config provided defaults to PAL 50hz for HDMI.  If you want to use composite out, you MUST change the machine_timing parameter in cmdline.txt to 'pal-composite'.  Otherwise, you will have audio synchronization issues.  You can change the machine to be NTSC if you want (see below).

# FileSystem/Drives

By default, the first partition of the SDcard is mounted and is where BMC64 will search for emulator files. To change this, add "disk_partition=#" to cmdline.txt where # is the partition number you want to mount (1-4).

Mounting extended partitions is possible but requires the start sector to be known. (The fatfs library used is not capable of finding extended partitions by number or id). To find the start sector of the extended partition you want to use, use fdisk:

Ex: sudo fdisk /dev/sdb

    Command (m for help): p
    Disk /dev/sdb: 29.9 GiB, 32036093952 bytes, 62570496 sectors
    Units: sectors of 1 * 512 = 512 bytes
    Sector size (logical/physical): 512 bytes / 512 bytes
    I/O size (minimum/optimal): 512 bytes / 512 bytes
    Disklabel type: dos
    Disk identifier: 0xf57fa762

    Device     Boot   Start      End  Sectors  Size Id Type
    /dev/sdb1          2048   204799   202752   99M  b W95 FAT32
    /dev/sdb2        204800 62570495 62365696 29.8G  5 Extended
    /dev/sdb5        206848  1230847  1024000  500M 83 Linux
    /dev/sdb6       1232896  2256895  1024000  500M 83 Linux
    /dev/sdb7       2258944  3282943  1024000  500M 83 Linux
    /dev/sdb8       3284992  4308991  1024000  500M 83 Linux

To use the extended partition /dev/sdb7, for example, you would set disk_partition=2258944 in cmdline.txt (Any value > 4 is assumed to be a start sector for a fatfs partition)

IMPORTANT: The files the Raspbery Pi itself needs to boot BMC64 must still reside in the first partition. They are:

    bootcode.bin
    start.elf
    config.txt
    kernel*.img
    cmdline.txt

Directories and long filenames are supported as of v1.0.10. Previous versions, required all disks, tapes, cartridges, rom files etc to reside in the root directory.  This is no longer the case.  If you have an existing image, it is recommended you move your files to the following directory structure:

    C64/  <- for kernal, basic, chargen, disk roms, etc.
    snapshots/
    disks/
    tapes/
    carts/
    tmp/

You can make drive 8 an IECDevice for the root directory of the SDcard. However, I don't recommend loading programs this way. The SDcard has slow access times and this will cause audio/video lag (but only during the load). This is because any native file access effectively blocks VICE's emulation routines.  It's fine to load a .PRG this way but don't try running something that needs frequent disk access.  IEC mode does not support all disk operations anyway.  It's mostly used for testing purposes.

# Sound

The default Sid engine is 'ReSid' which more accurately reproduces the sound chip. However, it consumes more CPU which is sometimes too much for the Raspberry Pi 2 @ 900Mhz.  Some high intensity demos may start to 'stagger' on the Pi 2. A good example of this is the end of Disk 1 of Comaland.  For most games, however, this won't be a problem.  If you run into this issue, however, you can switch the Sid engine to 'Fast'. It's lower quality but won't stutter.  The Pi 3 Model B @ 1.2 Ghz doesn't appear to have this problem.

You can switch between 6581 and 8580 models as well as turn on/off the filter.  For ReSid, only fast interpolation method is currently supported.

# Keyboards

Use F12 to bring up the menu and ESC/RUNSTOP to exit.

If you use a real commodore keyboard, you can use Commodore Key + F7 in place of F12. You can also wire a button between GPIO16 and GND to activate the menu or use gamepads with buttons configured for the menu.  Gamepads that have had a button configured to bring up the menu don't have to be assigned to a port to do that.  So even if you have DB9 joysticks wired, you should still be able to plug in a wired/wireless usb gamepad to trigger the menu if you want.

For the Keyrah, if you find your '=' key doesn't work.  Try switching the keyboard type to 'UK'.  You must save and restart for this to take effect.

# Gamepad config

As mentioned, gamepad support is limited.  Some gamepads advertise their dpads as analog sticks so if your dpad setting doesn't work and you want to use a dpad, try switching to analog.  Also, if the analog setting doesn't work, you may have to do some work to find the right axis # for both X and Y.  Usually, axis 0 and 1 are left stick X and Y axes but not always.  My cheap 'Kiwitata' gamepads are 3 & 4.  If your gamepad has two sticks, try 2 & 3 for right X/Y.

In v1.0.5+, there is a configuration sub-menu that will help you configure your usb gamepads.  You can monitor the raw usb values using 'Monitor raw USB data' men option.  The only way to escape from this menu is ESC/RUNSTOP.

# Menu Navigation

Since v1.0.8, you can hold down keys or gamepad/joystick directions and the navigation action will auto-repeat.  This accelerates the longer you hold in the same direction.

# GPIO Joystick Banks

DO NOT ATTEMPT THIS IF YOU ARE NOT COMFORTABLE WITH WIRING THINGS UP TO YOUR PI
I TAKE NO RESPONSIBILITY IF YOU WIRE THINGS INCORRECTLY OR DAMAGE YOUR DEVICE

It's possible to wire a real Commodore or Atari joystick directly to the Pi. The switches inside the joystick will ground the pins like they would on a real C64.  Wiring is as follows:

GPIO BANK 1   | GPIO BANK 2 | C64 JOY PIN
--------------|-------------|-------------
GPIO17        |GPIO5        | 1 (Up)
GPIO18        |GPIO6        | 2 (Down)
GPIO27        |GPIO12       | 3 (Left)
GPIO22        |GPIO13       | 4 (Right)
GPIO23        |GPIO19       | 6 (Fire)
GND           |GND          | 8 (GND)

In the menu, select either GPIO1 or GPIO2 and assign it to one of the emulated ports.

# CPU Temperature

The CPU temperature on a RPi 3 Model B clocked at @1.2Ghz can easily hit 70 degrees without a heat sync.  This measurement was made running the last sequence of Comaland mentioned above. With a good heat sync, however, it can drop to as low as 55. If you are going to put your Pi inside a case, I recommend you implement sufficient cooling measures to ensure your Pi does not overheat.

CPU temperature on a RPi 2 clocked at @900Mhz hit 46 degrees without a heat sync.  With a heat sync, it dropped to 40.  This is well below the default maximum of 85.  There's no reason to overclock your Pi so please don't do it.  I know of no game that won't run smoothly on the RPi 2 at its default clock rate.

# Changelog

  * https://github.com/randyrossi/bmc64/blob/master/CHANGELOG.md

# Q&A

Q: Can I switch the machine to NTSC?

A: Yes, you must edit BOTH config.txt and cmdline.txt.

   In config.txt, select an hdmi_mode that is 60hz.
   In cmdline.txt, change machine_timing to ntsc or ntsc-hdmi for HDMI
   If using composite, machine_timing MUST be ntsc-composite.  Othersie you will get audio synchronization issues.

   Be aware that some demos/games will not run if the machine is NTSC.  If you mix those two settings, your machine will likely run fast/slow, not look good or have audio issues (or all of the above).

Q: Why does the video look soft/stretched/dark?

A: I currently don't have much control over how the emulated display is scaled. I'm relying on the Pi to scale a small canvas up to whatever resolution your hdmi mode is set to and I've noticed it looks 'soft' sometimes.  Also, some monitors seem to stretch the canvas to full width rather than keep the aspect ratio.  You can change the hdmi mode and frame buffer aspect ratio in config.txt to suit your needs. Just keep in mind that NTSC machine timing must have a 60hz mode and PAL machine timing must have a 50hz mode.  This isn't something you normally care about in an emulator but because the machine is actually timed to the vertical blank signal of the video device, it matters.  I also recommend changing the mode to match your monitor/TV's native resolution.  That should avoid any unnecessary extra processing your monitor/TV might do which can add lag.  Also, place your Monitor/TV into 'Game' mode if possible.

Things you can fiddle with for video:

   hdmi_mode in config.txt
   framebuffer_aspect in config.txt (i.e. framebuffer_aspec=0x00070009 for 7:9)
   canvas_width canvas_height in cmdline.txt

The default settings work fine for composite out.

Q: Can I change option 'X' in vice?

A: Most options are fixed right now to make sure video and audio are rendered properly. Other options may be added in the future.

Q: Hey, isn't the real thing running at 50.125Hz?

A: Yes, the original machine ran at 50.125Hz for PAL and 59.826Hz for NTSC. So, yeah, you'll be about 0.25% off in terms of timing.

# Build Instructions

Pre-reqs:

    sudo apt-get install xa65

First get this repo:

    git clone https://github.com/randyrossi/bmc64.git --recursive

From the top level dir:

./clean_all.sh - will clean everything
./make_all.sh [pi2|pi3] - build everything

That should make a kernel7.img for RPI2, kernel8-32.img for RPI3

NOTE: There is no incremental build and the make_all.sh script will likely fail if run without a clean_all.sh before.

What to put on the SDcard:

    C64/
        KERNAL
        BASIC
        CHARGEN
        d1541II
        dos1541 (optional)
        dos1571 (optional)
        dos1581 (optional)
        rpi_sym.vkm
    kernel7.img (for Pi2)
    kernel8-32.img (for Pi3)
    fixup.dat
    bootstat.txt
    config.txt
    cmdline.txt
    snapshots/
        (place snapshot files here)
    disks/
        (place .d64 or other disk files here)
    tapes/
        (place .tap files here)
    carts/
        (place .crt files here)
    tmp/
        (used by the emulator sometimes)

# Resources

    https://github.com/raspberrypi/tools
    https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads

# Dependencies

These are the git hashes for the repos under circle-stdlib this project is known to work with.
If the patches don't apply successfully, it's possible those projects have moved forward causing incompatibilities.
If that happens, reset these repos using "git reset HASH --hard" commands in the following directories:

    circle-stdlib ea6fd459838f40586e4c60adb7a0912222a227b5
    libs/circle a5c693117e31a567fafd3e68880071f2f8c64977
    libs/circle-newlib 8a313d45ad5d8c0e306ce8751b6e3d6d3ef46ab3
    libs/mbedtls 60fbd5bdf05c223b641677204469b53c2ff39d4e

# Todo
- [ ] Measure latency (in frames) between input and response (both audio/video)
- [ ] Add ability to filter long lists of files 
- [ ] Add more emulator options to UI (i.e. true drive emulation toggle)
- [ ] Most timings below were taken from https://www.youtube.com/watch?v=b73BONBBZR8

# Performance comparison can be found at https://accentual.com/bmc64
