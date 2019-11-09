# BMC64

BMC64 is a bare metal C64 emulator for the Raspberry Pi with true 50hz/60hz smooth scrolling and low latency between input & video/audio. Three other Commodore machines are available as well; C128, Vic20 and Plus/4.

# BMC64 Features
  * Quick boot time (C64 in 4.1 seconds!)
  * Frames are timed to vsync for true 50/60 hz smooth scrolling (no horizontal tearing!)
  * Low latency between input & audio/video
  * No shutdown sequence required, just power off
  * High compatibility thanks to VICE
  * Easily wire real Commodore/Atari Joysticks and nav buttons via GPIO using jumpers (GPIO Config 1)
  * Can use a real Commodore Keyboard and Joysticks via PCB (GPIO Config 2)
  * Can use a Waveshare Game HAT (Pi2 or 3 Only) (GPIO Config 3)
  * Also works with the Keyrah

# Limitations
  * USB gamepad support is limited. Not all gamepads will work.
  * There is no network support.

This project uses VICE for emulation without any O/S (Linux) distribution installed on the Pi.  VICE (Versatile Commodore Emulator) platform dependencies are satisfied using circle-stdlib.

# Known Issues

  * There is no hot plug in/out support for USB devices.  All devices must be plugged in before the device is booted and never removed.  Attempting to remove them will halt the emulator or make it slow down considerably.

  * Some USB gamepads will require manual tweaking of settings from the defaults.

# Precompiled Images

  NOTE: Since V3.0, all machines are now bundled into one release for all Pi models. You can switch from the 'Machine' menu.

  * https://accentual.com/bmc64

# Github Link

  * https://github.com/randyrossi/bmc64

# Machine Selection

The default machine is a C64.  You can switch to VIC20, C128 and Plus/4 from the 'Machines->Switch' menu option.  A reboot is required after a switch. These configurations are defined in machines.txt. There you will find configurations for each machine type for NTSC/PAL over HDMI/Composite combinations.  Most video modes are 720p but you can change this (see below).

# machines.txt (Video & Timing)

The emulated machine is timed by the video mode you select. The default config provided uses 720p PAL 50hz on HDMI.  This is a 'safe' mode that should work on all monitors.  A 50.125hz custom mode is provided for the C64/Vic20/C128 machines that will match the timing of the real machine.  However, this mode may not work on all monitors.

Inside machines.txt, you can change or add new machine configurations. These will show up in the Machines->Switch menu.

Here is an example of a machine entry:

    [C64/NTSC/HDMI/720p@60Hz]
    disable_overscan=1
    sdtv_mode=16
    hdmi_group=1
    hdmi_mode=19
    machine_timing=ntsc-hdmi

NOTE: Even though a config is intended to be used for HDMI or Composite (never both), you should always define both composite and hdmi parameters.

For HDMI, you should choose either a 50hz or 60hz mode.

If you want to use composite out, you MUST set machine_timing parameter to ntsc-composite or pal-composite and set the corresponding sdtv_mode. Otherwise, you will have audio synchronization issues.

Raspberry Pi Video Mode     | machine_timing | cycles_per_second
----------------------------|----------------|-------------------
hdmi_group=1,hdmi_mode=19   | pal-hdmi       | not required
hdmi_group=1,hdmi_mode=4    | ntsc-hdmi      | not required
sdtv_mode=18                | pal-composite  | not required
sdtv_mode=16                | ntsc-composite | not required
hdmi_group=2,hdmi_mode=87   | pal-custom or ntsc-custom | see below

## How to add your own custom HDMI mode

You are free to experiment with different modes. It may be advantageous to set the video mode to match the native resolution of your monitor.  That way, it may have less processing to do and _may_ save on latency (not confirmed).  That can be accomplished with either a different hdmi_mode or a custom mode.

If you plan to use a custom HDMI mode, you will have to alter the machine's 'cycles_per_second' value to match the actual fps that mode outputs.  Custom HDMI modes may not be exactly 50 hz or 60 hz and that can cause audio sync issues if you use the default value.  A tool to calculate this number is provided under the 'Video' menu.  The test will take 10 minutes and will let you know what values you should add to cmdline.txt for machine_timing and cycles_per_second.  You only need to run the test once for that mode.

Example: Making a custom 1360x768 50Hz HDMI Mode

First change config.txt to the custom mode you want

    disable_overscan=1
    sdtv_mode=18
    hdmi_group=2
    hdmi_mode=87
    hdmi_cvt=1360 768 50 3 0 0 0

In config.txt you would temporarily have:

    fast=true machine_timing=pal-hdmi (remove any cycles_per_second settings)

Boot, then run the test tool. The tool will tell you the actual frame rate for this mode is 49.89 and show you the machine_timing and cycles_per_second parameters to use:

    machine_timing=pal-custom cycles_per_second=980670

You would then define your machines.txt entry like this:

    [C64/PAL/HDMI/1360x768@49.89Hz]
    disable_overscan=1
    hdmi_cvt=1360 768 50 3 0 0 0
    sdtv_mode=18
    hdmi_group=2
    hdmi_mode=87
    machine_timing=pal-custom
    cycles_per_second=980670

And this option will show up in the Machine->Switch menu.

# Video + Timing (VIC20, C128, Plus/4)

All of the above re: timing applies to the other machines as well.  However, in my opinion, the VIC20 machine is better configured to be an NTSC machine.  Most cartridges were made for NTSC and you will notice they position their screens poorly when inserted into a PAL machine.  Most games gave the option of moving it using cursor keys or joystick but this is annoying.

# Canvas Dimensions

Since v2.1, the virtual display dimensions are adjusted dynamically from the menu. Under 'Video', you will find Horizontal Border Trim %, Vertical Border Trim % and Aspect Ratio controls for each virtual display available.  Displays are scaled as follows:

      1. The amount of border to trim is removed from top/botom and left/right edges.
      2. The resulting image is stretched vertically to fill the Y dimension.
      3. The width is then calculated according to the aspect ratio setting.

Using the three settings available, you should be able to customize how much border is available as well as the aspect ratio of the final image.  Reasonable defaults are provided.  

NOTE: v2.1 and onward will ignore any vic_canvas_* or vicii_canvas_* kernel parameters.  The scaling_kernel option is still applicable.  If you are using a version lower than v2.1, consult the old documentation on how video cropping/sizing works.

# Video Scaling Algorithm

The emulated resolutions are small and must be scaled up to the video mode's resolution.  You can tell the Pi to change the way it scales the video using the 'scaling_kernel' option in config.txt:

  * (Soft Look) This is what the default scaling_kernel option (absent from config.txt) will look like:

![alt text](https://raw.githubusercontent.com/randyrossi/bmc64/master/images/scaling_kernel_default.jpg)

  * (Pixel Perfect Look) This is what scaling_kernel=8 option will look like:

![alt text](https://raw.githubusercontent.com/randyrossi/bmc64/master/images/scaling_kernel_8.jpg)

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
    kernel*.img* <- all kernels must be in the root dir
    cmdline.txt
    fixup.dat
    machines.txt

NOTE: Machine switching is currently NOT supported if you use any partition other than the first partition on the SDCard.

See 'What to put on the SDCard' for the directory structure expected.

## USB Drives

Since v2.4, you can mount up to 3 USB thumb drives and load cartridges, tapes, disks etc from them.  However, keep in mind that if you save your settings with custom kernels or attached cartridges that were loaded from a USB drive, they will not load on boot.  That's because all USB drives are lazy mounted on the first access via any file dialog.  For custom kernels or saved cartridges, it's best to load those from the SD card.

## IEC Mode (C64/C128 Only)

You can make any drive an IECDevice and select the directory you want to mount from the 'Disks' menu. However, I don't recommend loading programs this way. The SDcard has slow access times and this will cause audio/video lag (but only during the load). This is because any native file access effectively blocks VICE's emulation routines.  It's fine to load a .PRG this way but don't try running something that needs frequent disk access.  IEC mode does not support all disk operations anyway.  It's mostly used for testing purposes.

NOTE: If you select a directory off a USB device (not the SD card), then the IEC drive will not be accessible until you navigate to the USB device in any file dialog at least once.  This is because all USB drives are lazy mounted.  No USB drives are mounted at boot time, only the SD card.

# Sound

The default Sid engine is 'ReSid' which more accurately reproduces the sound chip.

NOTE: I had a section here previously about the Pi 2 @900Mhz not being able to keep up with some high intensity demos (i.e. last sequence of disk 1 Comaland 100%).  This is, in fact, a bug that only shows up after a snapshot previously saved at that part of the demo is loaded.  I'm investigating this.  But it appears the Pi 2 does run that sequence okay as long as you load it from disk.

You can switch between 6581 and 8580 models as well as turn on/off the filter.  For ReSid, only fast interpolation method is currently supported.

# Keyboards

Use F12 to bring up the menu and ESC/RUNSTOP to exit.

If you use a real commodore keyboard (either Keyrah or GPIO via PCB), you can use Commodore Key + F7 in place of F12 (This key combination is configurable). You can also use gamepads with buttons configured for the menu.  Gamepads that have had a button configured to bring up the menu don't have to be assigned to a port to do that.  So even if you have DB9 joysticks wired, you should still be able to plug in a wired/wireless usb gamepad to trigger the menu if you want.

For the Keyrah, if you find your '=' key doesn't work.  Try switching the keyboard type to 'UK'.  You must save and restart for this to take effect.

## Plus/4 Keyboard Notes

If you use a real C64 keyboard via GPIO Config #2, please be aware that BMPLUS4 will use a positional mapping that will match the C16 layout.  So the labels of the keys you press will not always match what the emulator gets.  For example, the four separate cursor keys are located where the +, -, pound and CLR keys are on the C64.  A real C16 or Plus/4 keyboard is not supported using GPIO. How the provided USB mapping works (or doesn't) with a real C16 keyboard and Keyrah is not known.

## C128 Keyboard Notes

F11 is the 40/80 Column Key for C128

# Gamepad config

As mentioned, gamepad support is limited.  Some gamepads advertise their dpads as analog sticks so if your dpad setting doesn't work and you want to use a dpad, try switching to analog.  Also, if the analog setting doesn't work, you may have to do some work to find the right axis # for both X and Y.  Usually, axis 0 and 1 are left stick X and Y axes but not always.  My cheap 'Kiwitata' gamepads are 3 & 4.  If your gamepad has two sticks, try 2 & 3 for right X/Y.

There is a configuration sub-menu that will help you configure your usb gamepads.  You can monitor the raw usb values using 'Monitor raw USB data' men option.  The only way to escape from this menu is ESC/RUNSTOP.

# Menu Navigation

You can hold down keys or gamepad/joystick directions and the navigation action will auto-repeat.  This accelerates the longer you hold in the same direction.  The Home, PageUp, PageDown, and End keys are supported in the menu as well.  The equivalent C64 keys are F1, F3, F5 and F7 respectively.  Also, pressing a letter will find the first menu item that has text starting with that letter for quick navigation of large lists.

# Joyport Configuration

Joyports (2 for C64/C128, 1 for VIC20) can be configured to use the following devices:

Device | Description 
-------|------------
None | No device active for the port
USB Gamepad 1 | First USB gamepad detected
USB Gamepad 2 | Second USB gamepad detected
GPIO Bank 1 | GPIO Pins as described below (GPIO Config dependent)
GPIO Bank 2 | GPIO Pins as described below (GPIO Config dependent)
1351 Mouse | First USB mouse detected
Keyrah Keys 1 | Numeric keypad keys 64825 compatible with Keyrah
Keyrah Keys 2 | Numeric keypad keys 17930 compatible with Keyrah
Cursor + Space | Cursor keys for directions, space for fire
Cursor + LCtrl | Cursor keys for directions, left control for fire
Custom Keyset 1 | Custom keyset 1
Custom Ketset 2 | Custom keyset 1

You can define custom keysets in the menu under the Joyport sub-menu.

# USB Button / Hotkey Function Mapping

You can assign functions to USB buttons or HotKey Combinations. HotKey combinations are LeftControl(Commodore) + F1,3,5,7 or Tab(Cntrl) + F1,3,5,7. Not all functions are available as HotKey assignments.  The functions available are:

Function | Description | Availability
---------|-------------|-------------
Up | Equivalent to Joystick Up | USB
Down | Equivalent to Joystick Down | USB
Left | Equivalent to Joystick Left | USB
Right | Equivalent to Joystick Right | USB
Fire | Equivalent to Joystick Fire Button | USB
POT X | Equivalent to Joystick 2nd Button (for some games) | USB
POT Y | Equivalent to Joystick 3rd Button (for some games) | USB
Menu | Enter/Exit Menu | USB+HotKey
Menu Back | Navigate to previous menu | USB+HotKey
Warp | Toggle warp mode | USB+HotKey
Status | Toggle status bar | USB+HotKey
Swap Ports | Swap joystick ports | USB+HotKey
Tape OSD | Brings up tape on screen display options | USB+HotKey
Cart OSD | Brings up cartridge on screen display options | USB+HotKey
Cart Freeze | Triggers cartridge freeze function (if installed) | USB+HotKey
Hard Reset | Hard resets the emulated machine | USB+HotKey
Soft Reset | Soft resets the emulated machine | USB+HotKey
Key 1-6 | Custom defined keystroke  | USB
Change Display | Switches VICII/VDC Displays | USB+Hotkey (C128 Only)
Change PIP Location | Moves PIP Location | USB+Hotkey (C128 Only)
Swap PIP | Swaps PIP Displays | USB+Hotkey (C128 Only)
40/80 Column | Toggles 40/80 Column Key | USB+Hotkey (C128 Only)

Custom defined keys (1-6) can be defined in the usb gamepad configuration screen.
The Tape/Cart OSD functions display a dialog with access to some common functions without pausing emulation.

# GPIO Configurations

There are 3 GPIO configurations to chose from:

Config | Description
-------|----------------------------------
1      | Menu Nav Buttons + Real Joysticks (i.e. CPIO-64 PCB or jumpers)
2      | Real Keyboard + Real Joysticks (BMC64 PCB)
3      | Waveshare Game HAT

## GPIO Config 1 : Menu Nav Buttons and Joysticks

DO NOT ATTEMPT THIS IF YOU ARE NOT COMFORTABLE WITH WIRING THINGS UP TO YOUR PI
I TAKE NO RESPONSIBILITY IF YOU WIRE THINGS INCORRECTLY OR DAMAGE YOUR DEVICE

You can wire buttons between GPIO pins and GND to have a physical menu activation or navigation button.

GPIO   | Function
-------|----------
GPIO16 | Menu Toggle
GPIO4  | Menu Back
GPIO25 | Up
GPIO19 | Down
GPIO20 | Left
GPIO21 | Right
GPIO24 | Enter

It's possible to wire real Commodore or Atari joysticks directly to the Pi using this config. You can do it without a PCB if you want. Each joystick gets its own set of GPIO pins, making it easy to hook up using nothing but jumpers and DB9 connectors.  (See http://accentual.com/bmc64/joystick.html). Wiring is as follows:

GPIO BANK 1   | GPIO BANK 2 | C64 JOY PIN
--------------|-------------|-------------
GPIO17        |GPIO5        | 1 (Up)
GPIO18        |GPIO6        | 2 (Down)
GPIO27        |GPIO12       | 3 (Left)
GPIO22        |GPIO13       | 4 (Right)
GPIO23        |GPIO19       | 6 (Fire)
GND           |GND          | 8 (GND)

In Joyports the menu, select either GPIO1 or GPIO2 and assign it to one of the emulated ports.
NOTE: There are no analog inputs so paddles won't function.

## GPIO Config 2 : GPIO Keyboard and Joysticks

This GPIO config option enables real keyboard and joystick scanning code purely from GPIO connections.  It can be used with a PCB specifically designed for BMC64.  The PCB design is available at https://upverter.com/design/rrossi/bmc64  It is possible to breadboard these connections using jumpers but that would mean a mess of wires inside your C64 shell.  The PCB is meant to mount your DB9 joystick ports, a power switch and power connector in the right spots as well as provide power for the shell's LED.

The PIN configuration for joysticks is different than what was described above for the GPIO Config 1 option.  So if you turn this option on, the GPIO joysticks you wired directly to the PI's header will not work unless you re-wire them as described below.

GPIO | C64 JOY PIN  | KEYBOARD CONNECTOR
-----|--------------|-------------------
GND  |              | KBD1 (GND)
 04  |              | KDB3 (Restore)
 26  | 6 (J2_FIRE)  | KBD20
 20  | 1 (J2_UP)    | KBD19
 19  | 2 (J2_DOWN)  | KBD18
 16  | 3 (J2_LEFT)  | KBD17
 13  | 4 (J2_RIGHT) | KBD16
 06  |              | KBD15
 12  |              | KBD14
 05  |              | KDB13
 21  | GND (J2_SEL) | 
 08  |              | KBD12
 25  |              | KBD11
 24  |              | KBD10
 22  | 6 (J1_FIRE)  | KBD9
 23  | 1 (J1_UP)    | KBD8
 27  | 2 (J1_DOWN)  | KBD7
 17  | 3 (J1_LEFT)  | KBD6
 18  | 4 (J1_RIGHT) | KBD5
 07  | GND (J1_SEL) |

NOTE: There are no analog inputs so paddles won't function.

Both real VIC20 and C64 keyboards should work in all emulated machines.  However, the additional keys found on the C128's keyboard are not available.

(One cool side-effect of using a real keyboard is if you boot the C128 and hold down the commodore key, it will boot into 64 mode like the real thing! That's something I could never get to work with USB keyboards.)

## GPIO Config 3 : Waveshare Game HAT

This GPIO configuration will let you use a Waveshare Game HAT with BMC64. Set either port 1 or 2 on the virtual machine to GPIO Bank 1. GPIO Bank 2 has no function in this configuration.  But you can still plug in a USB gamepad and use that as a second joystick if you like.

https://www.waveshare.com/game-hat.htm

Buttons are mapped as follows:

GPIO   | WAVESHARE BUTTON | BMC64 Function
-------|------------------|---------------
GPIO5  | Up               | Up
GPIO6  | Down             | Down
GPIO13 | Left             | Left
GPIO19 | Right            | Right
GPIO21 | Start            | Menu Toggle
GPIO4  | Select           | Status Bar Toggle
GPIO26 | A                | POTX
GPIO12 | B                | Fire
GPIO23 | Top Right        | Warp Toggle
GPIO20 | Y                | POTY
GPIO16 | X                | Virtual Keyboard Toggle
GPIO18 | Top Left         | Menu Back

NOTE: These button assignments are not configurable but may be in the future.

# CPU Temperature

IMPORTANT : BMC64 v1.0.6 through v1.4 were not properly putting the other 3 (unused) cores to a low powered mode and was causing CPU temperatures to rise close to or beyond automatic throttling limits. The CPU temperature on a RPi 3 Model B clocked at @1.2Ghz hit 70 degrees without a heat sync.  V1.5+ should not have this issue.  If you are experiencing heat issues (thermometer icon in top right corner), please update your installation to the latest version.

# Changelog

  * https://github.com/randyrossi/bmc64/blob/master/CHANGELOG.md

# Q&A

Q: Can I switch the machine to NTSC?

A: Yes, in V3.0 or higher, do this from the Machine->Switch menu.

   Be aware that some demos/games will not run if the machine is NTSC.  If you mix those two settings, your machine will likely run fast/slow, not look good or have audio issues (or all of the above).

Q: Why does the video look soft/stretched/dark?

A: By default, the scaling_kernel option softens the scaled image.  You can change this (see above).  Also, some monitors seem to stretch the canvas to full width rather than keep the aspect ratio.  You can change the hdmi mode and frame buffer aspect ratio in config.txt to suit your needs. Just keep in mind that NTSC machine timing must have a 60hz mode and PAL machine timing must have a 50hz mode.  This isn't something you normally care about in an emulator but because the machine is actually timed to the vertical blank signal of the video device, it matters.  I also recommend changing the mode to match your monitor/TV's native resolution.  That should avoid any unnecessary extra processing your monitor/TV might do which can add lag.  Also, place your Monitor/TV into 'Game' mode if possible.

Things you can fiddle with for video:

   hdmi_mode in config.txt
   framebuffer_aspect in config.txt (i.e. framebuffer_aspect=0x00070009 for 7:9)
   scaling_kernel (google for available algorithms)

Q: Can I change option 'X' in vice?

A: Most video options are fixed right now to make sure video is rendered properly. VICE settings are read from 'vice.ini' located in the root of the emulator's partition. If there is no UI support for the option you are looking for, you can try setting it there using VICE documentation.

Q: Hey, isn't the real thing running at 50.125Hz?

A: Yes, the original machine ran at 50.125Hz for PAL and 59.826Hz for NTSC. So, yeah, you'll be about 0.25% off in terms of timing.  If you really want 50.125Hz, you can try this custom HDMI mode (only applies to HDMI):

    [C64/PAL/HDMI/768x544@50.125Hz]
    sdtv_mode=18
    hdmi_group=2
    hdmi_mode=87
    hdmi_timings=768 0 24 72 96 544 1 3 2 14 0 0 0 50 0 27092000 1
    machine_timing=pal-custom
    cycles_per_second=985257

    This mode will match the timing of the original machine (for the purists) but may not be compatible with all monitors:

Q: Audio is not coming out of HDMI/Analog jack when I expect it to. Why?

A. Sometimes the code that auto detects whether audio should be piped through HDMI vs analog jack doesn't work.  You can force audio to where you want it with a audio_out=hdmi or audio_out=analog parameter.  The default is audio_out=auto.  This ends up in cmdline.txt but you should set it in every machine config in machines.txt you want it to show up for.

Q: How do I get Dolphin DOS to work?

From: http://rr.pokefinder.org/wiki/Dolphin_DOS

In BMC64:

    Use model 1541 with c1541-20-6.rom
    Load kernel-20-1.rom
    Use Parallel Cable : Standard

In vice.ini:

    Drive8RAM6000=1

# Build Instructions

Pre-reqs:

    sudo apt-get install xa65

First get this repo:

    git clone https://github.com/randyrossi/bmc64.git --recursive

From the top level dir:

./clean_all.sh - will clean everything
./make_all.sh [pi0|pi2|pi3] - build third party libs (vice/circle) and a C64 image for a Pi model
./make_machines.sh [pi0|pi2|pi3] - build all machines for a Pil model (depends on make_all.sh having been run once)

The make_all.sh script will make kernel.img for RPI0, kernel7.img for RPI2, kernel8-32.img for RPI3
The make_machines.sh script will make kernels for each machine with a suffix (.c64, .c128, .vic20, etc)

NOTE: There is no incremental build and the make_all.sh script will likely fail if run without a clean_all.sh before.

What to put on the SDcard:

    C64/
        kernal
        basic
        chargen
        d1541II
        dos1541 (optional)
        dos1571 (optional)
        dos1581 (optional)
        rpi_sym.vkm
    /C128
        kernal
	basichi
	basiclo
	charg64
	chargen
	kernal64
	basic64
	z80bios (optional)
	d1541II (optional)
        dos1571 (recommended)
	rpi_sym.vkm
        bootstat.txt
    /VIC20
	basic
	chargen
	kernal
	d1541II
	rpi_sym.vkm
        bootstat.txt
    /PLUS4
	kernal
	kernal.005
	kernal.232
	3plus1hi
	3plus1lo
	basic
	c2lo.364
	d1541II (optional)
        dos1551 (recommended)
	rpi_sym.vkm
        bootstat.txt
    /PLUS4EMU (Available for Pi3 Only)
        p4kernal.rom
        p4_ntsc.rom
        p4_basic.rom
        3plus1.rom
        dos1541.rom
        dos1551.rom
        dos1581.rom (optional)
        p4fileio.rom (optional)
        p4fileio.rom (optional)
    kernel.img (C64 kernel for Pi0)
    kernel7.img (C64 kernel for Pi2)
    kernel8-32.img (C64 kernel for Pi3)
    kernel.img.vic20
    kernel7.img.vic20
    kernel8-32.img.vic20
    kernel.img.c128
    kernel7.img.c128
    kernel8-32.img.c128
    kernel.img.plus4
    kernel7.img.plus4
    kernel8-32.img.plus4
    kernel8-32.img.plus4emu (for Pi3 only)
    fixup.dat
    bootstat.txt
    config.txt
    cmdline.txt
    machines.txt
    snapshots/
        C64/
        C128/
        VIC20/
        PLUS4/
    disks/
        C64/
        C128/
        VIC20/
        PLUS4/
    tapes/
        C64/
        C128/
        VIC20/
        PLUS4/
    carts/
        C64/
        C128/
        VIC20/
        PLUS4/
    tmp/
        (used by the emulator sometimes)

# Resources

    https://github.com/raspberrypi/tools
    https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads

# Dependencies

These are the git hashes for the repos under circle-stdlib this project is known to work with.
If the patches don't apply successfully, it's possible those projects have moved forward causing incompatibilities.
If that happens, reset these repos using "git reset HASH --hard" commands in the following directories:

    circle-stdlib dda16112cdb5470240cd51fb33bf72b311634340
    libs/circle 641264ed56f0f621e4b681d5eefd6a19f8adfa5e
    libs/circle-newlib 7565e4314b5915a0dc2a7745bd18c8f7833b3f5c
    libs/mbedtls fb1972db23da39bd11d4f9c9ea6266eee665605b

# Performance numbers can be found at https://accentual.com/bmc64
