# BMC64

BMC64 is a bare metal C64 emulator for the Raspberry Pi with true 50hz/60hz smooth scrolling and low latency between input & video/audio. Four other Commodore machines are available as well; C128, Vic20, Plus/4 and PET.

# BMC64 Features
  * Quick boot time (C64 in 4.1 seconds over composite!)
  * Frames are timed to vsync for true 50/60 hz smooth scrolling (no horizontal tearing!)
  * Low latency between input & audio/video
  * No shutdown sequence required, just power off
  * High C64 compatibility thanks to VICE
  * High Plus/4 compatibility thanks to Plus4Emu (Rpi3 Only)
  * Easily wire real Commodore/Atari Joysticks and nav buttons via GPIO using jumpers (GPIO Config 1)
  * Can use a real Commodore Keyboard and Joysticks via PCB (GPIO Config 2)
  * Can use a Waveshare Game HAT (Pi2 or 3 Only) (GPIO Config 3)
  * Also works with the Keyrah and TheC64 'Maxi' case

# Limitations
  * USB gamepad support is limited. Not all gamepads will work (especially wireless).
  * There is no network support.

This project uses VICE for emulation without any O/S (Linux) distribution installed on the Pi.  VICE (Versatile Commodore Emulator) platform dependencies are satisfied using circle-stdlib.

For Plus/4 emulation on the Rasbperry Pi 3, a more accurate emulator using Plus4Emu is also available.

# Known Issues

  * There is no hot plug in/out support for USB devices.  All devices must be plugged in before the device is booted and never removed.  Attempting to remove them will halt the emulator or make it slow down considerably.

  * Some USB gamepads will require manual tweaking of settings from the defaults.

# Precompiled Images

  NOTE: Since V3.0, all machines are now bundled into one release for all Pi models. You can switch from the 'Machine' menu.

  * https://accentual.com/bmc64

# Github Link

  * https://github.com/randyrossi/bmc64

# Machine Selection

The default machine is a C64.  You can switch to VIC20, C128, Plus/4 or PET from the 'Machines->Switch' menu option.  These configurations are defined in machines.txt. There you will find configurations for each machine type for NTSC/PAL over HDMI/Composite combinations.  Most video modes are 720p but you can change this (see below).

# machines.txt (Video & Timing)

The emulated machine is timed by the video mode you select. The default config provided uses 720p PAL 50hz on HDMI.  This is a 'safe' mode that should work on all monitors.  A 50.125hz custom mode is provided for the C64/Vic20/C128 machines that will match the timing of the real machine.  However, this mode may not work on all monitors.

Inside machines.txt, you can change or add new machine configurations. These will show up in the Machines->Switch menu.

Here is an example of a machine entry:

    [C64/PAL/HDMI/VICE 720p@50Hz]
    disable_overscan=1
    sdtv_mode=18
    hdmi_group=1
    hdmi_mode=19
    machine_timing=pal-hdmi
    scaling_params=384,240,1152,720

NOTE: Even though a config is intended to be used for HDMI or Composite (never both), you should always define both composite and hdmi parameters.

Valid machine names are C64, C128, VIC20, PET, PLUS4 and PLUS4EMU (Rpi3 only)
Valid video standards are NTSC, PAL
Valid video output types are HDMI, Composite, DPI

For HDMI, you should choose either a 50hz or 60hz mode.
See below for notes on using DPI.

If you want to use composite out, you MUST set machine_timing parameter to ntsc-composite or pal-composite and set the corresponding sdtv_mode. Otherwise, you will have audio synchronization issues.

Raspberry Pi Video Mode     | machine_timing | cycles_per_second
----------------------------|----------------|-------------------
hdmi_group=1,hdmi_mode=19   | pal-hdmi       | not required
hdmi_group=1,hdmi_mode=4    | ntsc-hdmi      | not required
sdtv_mode=18                | pal-composite  | not required
sdtv_mode=16                | ntsc-composite | not required
hdmi_group=2,hdmi_mode=87   | pal-custom or ntsc-custom | see below

## How to add your own custom HDMI mode (VICE emulators only)

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

    (see below for chosing scaling_params)

And this option will show up in the Machine->Switch menu.

Custom HDMI modes are not supported for plus4emu yet.

## DPI (Parallel Display Interface)

BMC64 v3.3 and higher supports video output via DPI. DPI is a (up to) 24-bit parallel RGB interface.  A DPI capable display device or RGB adapter board is required. You will have to match the output format to your device.  See https://www.raspberrypi.org/documentation/hardware/raspberrypi/dpi/README.md for more details.

DPI disables HDMI and Composite video and is different for every device. For this reason, these configurations are not provided by default and must be added manually to machines.txt. I suggest once you have a working configuration, you add the machines.txt entries for all the machines you want to use DPI with.

Here are a couple examples you can add to machines.txt that will work with the VGA666 adapter board:

    [C64/NTSC/DPI/VGA666:720p@60hz]
    enable_dpi=true
    machine_timing=ntsc-custom
    cycles_per_second=1025643
    enable_dpi_lcd=1
    display_default_lcd=1
    dpi_group=1
    dpi_mode=4
    scaling_params=384,240,1152,720

    [C64/PAL/DPI/VGA666:720p@50hz]
    enable_dpi=true
    machine_timing=pal-custom
    cycles_per_second=982734
    enable_dpi_lcd=1
    display_default_lcd=1
    dpi_group=1
    dpi_mode=19
    scaling_params=384,240,1152,720

* It appears these modes are not exactly 50hz/60hz like HDMI. It's likely the case that all DPI modes will require custom timing.  See steps mentioned above for how to find the correct cycles_per_second value for your DPI mode.

** DPI uses almost all the GPIO pins. GPIO configs for things like joysticks/keyboards/buttons are disabled when enable_dpi is present in cmdline.txt

*** Special thanks goes out to github user K-rnivoro (https://github.com/K-rnivoro) for suggesting this feature.

# Recovering from a Blank Screen

If you are experimenting with a video mode and are not getting a picture, you can press Cntrl followed by F7 and hold both for 5 seconds after a boot, then release the F7 key.  This will reset the emulator and switch back to C64 with a 'safe' HDMI 720p video mode. (This will also work over composite but you should follow up with a switch to one of the composite modes for correct audio sync.)

# Video + Timing (VIC20, C128, Plus/4, PET)

All of the above re: timing applies to the other machines as well.  However, in my opinion, the VIC20 machine is better configured to be an NTSC machine.  Most cartridges were made for NTSC and you will notice they position their screens poorly when inserted into a PAL machine.  Most games gave the option of moving it using cursor keys or joystick but this is annoying.

# Dimensions and Scaling

The virtual display dimensions can be adjusted dynamically from the menu. Under 'Video', you will find Horizontal Border (px), Vertical Border (px), and H/V Stretch controls for each virtual display available. Displays are scaled as follows:

      1. The main graphics area is trimmed or padded by border level adjustments.
      2. The resulting image is scaled according to stretch factors (1.0 is full vertical height, horizontal is a scalar of the display height)
      3. The scaled image is then centered within the display resolution.

Using the settings, you should be able to customize the display to your liking. However, if you plan on using scaling_kernel=8 (nearest neighbor) there are benefits to chosing dimensions that result in integer multipliers of the frame buffer (see below).

## Integer Scaling

If you want to get better picture quality on your CRT monitor or if you prefer the 'pixel perfect' look on HDMI, integer scaling is what you want. Integer scaling along with scaling_kernel=8 will eliminate scaling artifacts (i.e. variation in thickness of scaled up pixels).  However, you may have to sacrifice some border area and/or not get the exact aspect ratio you want.

When you change the border values, you will see three dimensions displayed; the display dimensions, the frame buffer dimensions (FB) and the scaled frame buffer (SFB) dimensions. When the scaled frame buffer dimensions are an integer multiple of the frame buffer dimensions, they will turn green.

To make this easier, there are video options named 'Next H Integer Scale' and 'Next V Integer Scale'.  They will bump up the horizontal and vertical stretch to the next nearest integer multiple of the frame buffer's dimensions. Keep pressing return to cycle through all integer multipls that will fit.  If the next integer multiple doesn't fill the screen, you will have to adjust the border amount so that it does and try again (not always possible).

Once you have values that work for you, edit the machines.txt file and add or change the scaling_params for the mode you are running:

    scaling_params=fb_width,fb_height,sfb_width,sfb_height

    (For C128 and PET use scaling_params2 for the 80 column displays.)

When sfb_width = fb_width * N, where N is some integer, the width will be integer scaled.

When sfb_height = fb_height * N, where N is some integer, height will be integer scaled.

Using scaling_kernel=8 without integer scaling can cause irregular looking characters. Notice the two O's and S's look odd.

![alt text](https://raw.githubusercontent.com/randyrossi/bmc64/master/images/not_integer_scaled.png)

Using scaling_kernel=8 with integer scaling eliminates irregularities.

![alt text](https://raw.githubusercontent.com/randyrossi/bmc64/master/images/integer_scaled.png)

## Perfect DPI -> (Real) CRT Scanlines

For those using DPI connected to CRTs (via VGA666 for example), you can try a custom resolution like this:

    dpi_timings=1920 1 56 176 208 282 1 5 2 23 0 0 0 50 0 36908040 1

This gives a 1920 x 282 display res.  Then set FB to 384x282.  Then set SFB to 1920x282.  That gives a 5x horizontal scale and 1x vertical.  So the CRT will trace each line of the frame buffer on one scanline.  Once your resolution looks good, follow the custom timing tool instructions to get your cycles_per_second correct.  The above mode was tested on a Sony Trinitron CRT.  Here is the machines.txt entry:

    [C64/PAL/DPI/VGA666:1920x282@50.125hz]
    enable_dpi=true
    machine_timing=pal-custom
    cycles_per_second=985257
    enable_dpi_lcd=1
    display_default_lcd=1
    dpi_group=2
    dpi_mode=87
    dpi_timings=1920 1 56 176 208 282 1 5 2 23 0 0 0 50 0 36908040 1
    scaling_params=384,282,1920,282

NOTE: This mode has a 15khz horizontal refresh rate which many monitors don't support.  You may have to experiment with other resolutions (not necessarily 282 lines) to get something working.

Many thanks goes out to Alessio Scanderebech and Andrea Mazzoleni for their assistance with getting this working.

# VGA Display without 15khz hsync support

If you have a VGA monitor that doesn't support 15khz hsync, you can try using a 1920x1080 resolution and add raster_skip=true to the machine config.  The 'raster_skip' option doubles the vertical height of the frame buffer and draws the emulated display into every other line.  When this is integer scaled up to 1080, you can achieve more or less the same effect you would get with a monitor that could do 15khz.  That is, there will be gaps between each 'scanline' as though there were only ~270 lines.  This looks pretty decent on a Trinitron tube because there are no horizontal gaps in the phosphor bars and the 2 real scanlines that resulted from doubling the one line from the frame buffer looks like one solid scanline (...at least that's what I think is happening)

    [C64/PAL/DPI/VGA666:1920x1080@50hz]
    enable_dpi=true
    machine_timing=pal-custom
    cycles_per_second=985257
    enable_dpi_lcd=1
    display_default_lcd=1
    dpi_group=1
    dpi_mode=31
    raster_skip=true
    scaling_params=1920,540,1920,1080

# Video Scaling Algorithm

The emulated resolutions are small and must be scaled up to the video mode's resolution.  You can tell the Pi to change the way it scales the video using the 'scaling_kernel' option in config.txt:

  * (Soft Look) This is what the default scaling_kernel option (absent from config.txt) will look like:

![alt text](https://raw.githubusercontent.com/randyrossi/bmc64/master/images/scaling_kernel_default.jpg)

  * (Pixel Perfect Look) This is what scaling_kernel=8 option will look like:

![alt text](https://raw.githubusercontent.com/randyrossi/bmc64/master/images/scaling_kernel_8.jpg)

  NOTE: For scaling_kernel=8, it's best to adjust your FB so that it can be scaled by an integer and fill most of the screen.  See section on Integer Scaling above.

# (Real) CRT Scanline alignment

If you are using a CRT, you may want to use the 'V Stretch Factor' to better align the video to your monitr's real scanlines.  A mis-alignment can make characters look fuzzy.

  * Here's what a bad alignment can look like:

![alt text](https://raw.githubusercontent.com/randyrossi/bmc64/master/images/bad_crt_align.jpg)

  * Here's what a better alignment can look like:

![alt text](https://raw.githubusercontent.com/randyrossi/bmc64/master/images/good_crt_align.jpg)

NOTE: It may not be possible for EVERY scanline to line up from top to bottom but you should be able to get most of the display to look less 'fuzzy'.

# CRT Filter

For HDMI displays, you can enable davej's shader which emulates a CRT. To use the shader properly, you will need to use (at the very least) integer scaling (see above).  I recommend at least 3x in both dimensions.  This will produce acceptable results on just about any resolution. However, if you want every scanline and every 'phosphor' (if using a mask) to be precicely the same height/width, you will also need to use a resolution that matches your monitor's native resolution.  That's because unless you have a resolution that matches your monitor's native resolution, your monitor will end up scaling the image itself.

  * Here is the CRT shader using a 1280x720 display resolution while the monitor is 1600x900.  Notice every few scanlines are slightly thicker.

![alt text](https://raw.githubusercontent.com/randyrossi/bmc64/master/images/shader_nonative.png)

  * Here is the same CRT shader using the monitor's 1600x900 native resolution.  Notice every scan line height is identical.

![alt text](https://raw.githubusercontent.com/randyrossi/bmc64/master/images/shader_native.png)

The shader is not available if the mode is marked for composite. (There's no reason to put fake scanlines underneath real scanlines.)  For DPI output, using the shader depends on your resolution.  If you are able to use a (near) 240p resolution, don't enable the shader. However, if you are using a VGA666 adapter and a high display resolution on a very sharp display (i.e. Trinitron), you might want to use the shader for scanlines.  The shader should work up to 1080p resolution at 60fps but some options might affect performance (i.e. curvature).

Vertical centering will be reset to 0 if the shader is enabled.

## CRT Filter + Performance

Some shader parameter combinations may cause stuttering or frame drops on the Pi0/2. Curvature + MultiSample + Filter might be too much (especially for the Pi0), but two out of three may work. If you see bad performance, try disabling the shader altogether to determine if that's the cause.  Then try disabling different features to see if you can isolate which option pushes your system over the limit. It's likely most games won't experience any problems but some demos might.  I have not tested the shader's performance or how it impacts emulation at all resolutions. Your mileage may vary.

The shader may crash the Pi on higher resolutions/fps. (My Pi0 crashes @ 1600x900 @60fps consistently.) 720p and 1080p @ 50fps seem to operate okay.

# Commodore 128 VDC raster_skip2 flag

The shader does not apply to the Commodore 128's VDC display.  However, you can get a raster line effect by adding raster_skip2=true to cmdline.txt (or the corresponding machines.txt entry.)  This will skip every other frame buffer line when rendering the display giving (something close to) a raster lines effect. Results will look better with an integer multiple of the native vertical resolution for the frame buffer height.  However, non-integer values should work too.

NOTE: raster_skip=true (no 2) can also be used for the main display on other emulators (i.e. VIC-II on the C64) but is not recommended since the shader produces much better results.  Do not use both raster_skip=true AND enable the shader at the same time or the results will look terrible.

![alt text](https://raw.githubusercontent.com/randyrossi/bmc64/master/images/raster_skip.jpg)

# Files Organization

File browsers will by default look in directories off the SD card using this convention:

   /disks/C64
   /carts/C64
   /tapes/C64
   /snapshots/C64

   (Autostart starts off in "/")

However, if you prefer to organize your files like this:

   /C64/disks
   /C64/carts
   /C64/tapes
   /C64/snapshots

   (Autostart starts off in "/C64")

Then change the 'Files Location Convention' in the Prefs menu accordingly.

* ROMS must always be located in "/C64".
** Replace "C64" above with the appropriate emulator sub dir for different emulators.

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

BMC64 cannot mount some USB sticks (especially newer/larger models). If this happens, try a different (possibly lower capacity) stick.

## IEC Mode (C64/C128 Only)

You can make any drive an IECDevice and select the directory you want to mount from the 'Disks' menu. However, I don't recommend loading programs this way. The SDcard has slow access times and this will cause audio/video lag (but only during the load). This is because any native file access effectively blocks VICE's emulation routines.  It's fine to load a .PRG this way but don't try running something that needs frequent disk access.  IEC mode does not support all disk operations anyway.  It's mostly used for testing purposes.

NOTE: If you select a directory off a USB device (not the SD card), then the IEC drive will not be accessible until you navigate to the USB device in any file dialog at least once.  This is because all USB drives are lazy mounted.  No USB drives are mounted at boot time, only the SD card.

# CMDHD Support

CMDHD drive support was added in v4.1.  You can switch a drive model to CMDHD and mount .DHD images.  However, there are some limitations to be aware of:

1. .DHD files are very slow to attach and parse from SDCards.  If possible, use a USB thumbstick instead. Performance will be much better.
2. Whenever you attach an image, you must RESET the CMDHD drive by using the Options CMDHD Mode.  Cycle through the options until you reach 'Normal' again (even if you started on Normal).  Please be aware that every time you RESET a CMDHD drive, the emulator will freeze for a period of time.  This makes it looks like it has crashed but it is actually just processing the RESET.  When using SDCard mounted .DHD files, this pause can be quite long.  Again, use a USB thumbstick for more reasonable wait times.
3. Auto-expanding .DHD files are not supported. You may come across instructions on the web telling you to create a 0 byte .DHD file and then create partitions. This may work on VICE desktop builds, but this will not work with BMC64. Instead, you must pre-allocate the storage space you need. Do this with the dd command like this:

    dd if=/dev/zero of=my.dhd bs=1M count=16

This will create a 16Mb image.  You can then use CREATE SYS and HD-TOOLS.64 to create a partition.  To change the drive mode to 'Initialization' or 'Configuration', use the Options menu. Although creating partitions is possible from BMC64, it is better to prepare your CMDHD disk images using a desktop/laptop VICE installation and then make them available to BMC64 on a USB thumbstick.
4. Saving snapshots with .DHD disks attached does not work. The emulator will likely crash.

NOTE: Adding CMDHD support is on a best-effort basis as this feature was added in later revisions of VICE and back-ported to BMC64. Not all functions may work as expected.

# C64OS Support

C64OS will work with BMC64 from 4.1 onward but only with JiffyDOS kernal replacement installed.  For some reason, the regular kernal hangs on application launches.  This happens on desktop versions of VICE as well so I don't think it's a problem with BMC64 specifically.  If you are able to use C64OS without JiffyDOS on VICE, let me know.

# Sound

The default Sid engine is 'ReSid' which more accurately reproduces the sound chip.

You can switch between 6581 and 8580 models as well as turn on/off the filter.

On the Pi0/Pi2, only 'Fast' sampling method is enabled.

On the Pi3, 'Fast', 'Interpolation' and 'Fast Resampling" sampling methods have been enabled.  'Fast Resampling' produces the highest quality but consumes the most CPU.

On the Pi3 model B with a 1.2Ghz clock, 'Fast Resampling' passband is set to 13230hz.
On the Pi3 model B+ with a 1.4Ghz clock, 'Fast Resampling' passband is set to ~~19845hz~~ 13230hz (Temporary until I can reliably detect Pi3B at boot).

Fast Resampling+6581 can possibly cause stuttering on complex demos with a lot of multi colored expanded sprites.  However, my two worst test cases (Comaland 100% and Star Wars) appear to just squeak by with approx 2ms to spare at the most complex points in the demos.

Caution: Fast Resampling in combination with other high CPU intensive tasks (like true drive emulation) can push the emulator 'over the edge' and cause stuttering.  A known case is the Epyx FastLoad cartridge + Interpoliation or Fast Resampling will cause slow downs on the Pi3.  If you experience this, use Fast sampling instead.

NOTE: For resampling, parameters like Gain and Passband frequency must remain at the default values and cannot be changed (even in vice.ini).

## Dual SIDs

You can enable dual SIDs in v3.5+ for the C64 to get 6 voices and stereo sound. For the second SID to work, your C64 program and the base address for the second SID must match.  If you change the base address, it's best to reset the machine to clear out the registers.

## Pseudo Stereo Effect

BMC64 has modified VICE code to allow the same base address ($d400) for both SIDs as well allowing different SID models on each channel.  You can get a 'pseudo' stereo effect this way due to the different characteristics of the different SID chips.  (Try listening to 'Edge of Disgrace' with headphones and select 6581 for SID1 and 8850 for SID2.)  This will work for any software you run, not just programs that can use a second SID.

Thanks goes out to to github.com user boras-pl (https://github.com/boras-pl) for suggesting this. This simulates this real life mod:

(https://www.youtube.com/watch?v=2Qlqeaxkp14)
(http://blog.tynemouthsoftware.co.uk/2015/11/commodore-64-pseudo-stereo-dual-sid.html)

# Keyboards

Use F12 to bring up the menu and ESC/RUNSTOP to exit.

If you use a real commodore keyboard (either Keyrah or GPIO via PCB), you can use Commodore Key + F7 in place of F12 (This key combination is configurable). You can also use gamepads with buttons configured for the menu.  Gamepads that have had a button configured to bring up the menu don't have to be assigned to a port to do that.  So even if you have DB9 joysticks wired, you should still be able to plug in a wired/wireless usb gamepad to trigger the menu if you want.

What keyboard mapping to use?

Usage         | Keyboard Mapping
--------------|----------------
USB           | Positional or Symbolic
GPIO          | Positional
Keyrah        | Positional
TheC64 (Maxi) | Maxi Positional

* Make sure you have the rpi_*.vkm files located in each machine subdir.

* Special thanks goes out to ody81(github) / ody ody (youtube) user who figured out the Maxi keyboard mapping file.  You can watch his tutorial on how to put BMC64 inside a Maxi case here: https://www.youtube.com/channel/UCrXCNM2oXmIA7sTUwXrumiw or read his Reddit post here: https://www.reddit.com/r/Commodore/comments/ejtggy/the_c64_maxi_to_bmc64_conversion_rough_tutorial

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

'Regular' Joyports (2 for C64/C128, 1 for VIC20) can be configured to use the devices below.  The VICE emulators also support userport adapters that can add another 2 joyports (for games that support them).  Extra userport joysticks must be activated in the Joyports->Userport Joysticks sub-menu.

Device | Description
-------|------------
None | No device active for the port
USB Gamepad 1 | First USB gamepad detected
USB Gamepad 2 | Second USB gamepad detected
USB Gamepad 3 | Third USB gamepad detected
USB Gamepad 4 | Fourth USB gamepad detected
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
Up | Equivalent to Joystick Up | USB, GPIO
Down | Equivalent to Joystick Down | USB, GPIO
Left | Equivalent to Joystick Left | USB, GPIO
Right | Equivalent to Joystick Right | USB, GPIO
Fire | Equivalent to Joystick Fire Button | USB, GPIO
POT X | Equivalent to Joystick 2nd Button (for some games) | USB, GPIO
POT Y | Equivalent to Joystick 3rd Button (for some games) | USB, GPIO
Menu | Enter/Exit Menu | USB, GPIO, HotKey
Menu Back | Navigate to previous menu | USB, GPIO, HotKey
Warp | Toggle warp mode | USB, GPIO, HotKey
Status | Toggle status bar | USB, GPIO, HotKey
Swap Ports | Swap joystick ports | USB, GPIO, HotKey
Tape OSD | Brings up tape on screen display options | USB, GPIO, HotKey
Cart OSD | Brings up cartridge on screen display options | USB, GPIO, HotKey
Cart Freeze | Triggers cartridge freeze function (if installed) | USB, GPIO, HotKey
Hard Reset | Hard resets the emulated machine | USB, GPIO, HotKey
Soft Reset | Soft resets the emulated machine | USB, GPIO, HotKey
Key 1-6 | Custom defined keystroke  | USB, GPIO
Change Display | Switches VICII/VDC Displays | USB, GPIO, Hotkey (C128 Only)
Change PIP Location | Moves PIP Location | USB, GPIO, Hotkey (C128 Only)
Swap PIP | Swaps PIP Displays | USB, GPIO, Hotkey (C128 Only)
40/80 Column | Toggles 40/80 Column Key | USB, GPIO, Hotkey (C128 Only)
Virtual Kbd | Toggles Virtual Keyboard | USB, GPIO

Custom defined keys (1-6) can be defined in the usb gamepad configuration screen.
The Tape/Cart OSD functions display a dialog with access to some common functions without pausing emulation.

# GPIO Configurations

There are a number of GPIO configurations to chose from:

Config | Description
-------|----------------------------------
1      | Menu Nav Buttons + Real Joysticks (i.e. CPIO-64 PCB or jumpers)
2      | Real Keyboard + Real Joysticks (BMC64 PCB)
3      | Waveshare Game HAT
4      | Userport  + Real Joysticks
5      | Custom Defined

NOTE: The default config is 1.  If you are installing BMC64 for the first time and need a GPIO config other than 1 and don't have a USB keyboard to change the option, you will have to manually edit settiongs.txt and ensure a line with 'gpio_config=[0-2]' exists. The number given should be 1 less than the config number in the documentation above.  For example, if you are using a PCB with full keyboard connector, you would add to (or create) a settings.txt file with "gpio_config=1" for the selection of Config 2.  Since each emulator has its own settings file, you would need to do this for settings.txt (C64), settings-c128.txt , settings-vic20.txt, settings-plus4.txt and settings-plus4emu.txt

## GPIO Config 1 : Menu Nav Buttons and Joysticks

DO NOT ATTEMPT THIS IF YOU ARE NOT COMFORTABLE WITH WIRING THINGS UP TO YOUR PI
I TAKE NO RESPONSIBILITY IF YOU WIRE THINGS INCORRECTLY OR DAMAGE YOUR DEVICE

You can wire buttons between GPIO pins and GND to have a physical menu activation or navigation button.

GPIO   | Function
-------|----------
GPIO16 | Menu Toggle
GPIO4  | Menu Back
GPIO25 | Up
GPIO8  | Down
GPIO20 | Left
GPIO21 | Right
GPIO24 | Enter
GPIO26 | Virtual Keyboard

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

## GPIO Config 4 : Userport and Joysticks

This configuration will let you control 8 GPIO pins using the emulator userport.  This has been tested for the C64 and may work as is for the C128, VIC20 and PET.  It also provides access to two real joysticks similar to option 1.  Please keep in mind the highest resolution for changing or reading pins values will be that of the video vertical frequency of 50hz or 60hz.  This is because the emulator emulates CPU cycles in bursts as fast as it can to render the next 20 ms or 16 ms worth of activity each frame.  This is much lower a resolution than you would get on a real C64 (even in BASIC) so don't expect to control high speed devices this way.  This is for educational/fun purposes only.

***WARNING***: This configuration is capable of setting some pins to OUTPUT 3.3V. Do NOT use this while the keyboard/joystick PCB mentioned above is connected to your Pi.  This will make it possible for some OUTPUT pins to be connected to some INPUT pins simply by pressing keys on the C64 keyboard or using a joystick.  Since there is nothing limiting current, this WILL cause damage to the GPIO pins.  For this reason, this option is only available for selection from the menu by adding 'enable_gpio_outputs=true' to cmdline.txt  Do not use this option unless you are certain the connections to your GPIO header are safe for use with this configuration.

The joysticks are mapped as follows:

GPIO BANK 1   | GPIO BANK 2 | C64 JOY PIN
--------------|-------------|-------------
GPIO17        |GPIO5        | 1 (Up)
GPIO18        |GPIO6        | 2 (Down)
GPIO27        |GPIO12       | 3 (Left)
GPIO22        |GPIO13       | 4 (Right)
GPIO23        |GPIO19       | 6 (Fire)
GND           |GND          | 8 (GND)

The userport is available as follows:

GPIO   | Port Bit
-------|---------
GPIO4  | 0
GPIO24 | 1
GPIO25 | 2
GPIO8  | 3
GPIO16 | 4
GPIO26 | 5
GPIO20 | 6
GPIO21 | 7

When used as inputs, the pins use pull-up resistors (just like the real C64) so the value defaults to 1.  Grounding the pin will result in a 0 being read.

## GPIO Config 5 : Custom Defined

If the pre-defined GPIO configurations don't suit your needs, you can define your own GPIO mappings if you wish.  Use the 'Configure Custom GPIO' menu option under 'Prefs'.

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
    scaling_params=0,384,272,768,544

This mode will match the timing of the original machine (for the purists) but may not be compatible with all monitors:

For NTSC, this mode will match the real timing very closely.  But again, since it's not a standard resolution, it may not work on all monitors.

    [C64/NTSC/HDMI/VICE 768x525@59.825Hz]
    sdtv_mode=18
    hdmi_group=2
    hdmi_mode=87
    hdmi_timings=768 0 24 72 96 525 1 3 10 9 0 0 0 60 0 31415829 1
    machine_timing=ntsc-custom
    cycles_per_second=1022708
    scaling_params=0,384,246,768,492

Q: Audio is not coming out of HDMI/Analog jack when I expect it to. Why?

A. Sometimes the code that auto detects whether audio should be piped through HDMI vs analog jack doesn't work.  You can force audio to where you want it with a audio_out=hdmi or audio_out=analog parameter.  The default is audio_out=auto.  This ends up in cmdline.txt but you should set it in every machine config in machines.txt you want it to show up for.

Q: How do I get Dolphin DOS to work?

From: http://rr.pokefinder.org/wiki/Dolphin_DOS

From the menu:

    Machine -> ROMS -> Change Kernal ROM -> kernel-20-1.rom
    Drives -> Change ROM -> 1541 -> c1541-20-6.rom
    Drive 8
       Change Model -> 1541
       Options -> Parallel Cable -> Standard
       Options -> RAM 6000 On

Q: PetRescue won't attach in Plus4Emu. Why?

A: Plus4Emu is picky about the size of the disk images it attaches.  PetRescue has extra sectors but for some reason, the d64 image is 197376 bytes which doesn't complete a full track. To get it to attach, you will have to pad it with extra 0's. The size after padding should be 200960.

On Linux, this command will work:

truncate -s +3584 petrescue.d64

Q. What do I put on the SDcard?

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
        dos1541.rom (optional)
        dos1551.rom
        dos1581.rom (optional)
        p4fileio.rom (optional)
        p4fileio.rom (optional)
    /PET
        basic1
        basic2
        basic4
        characters.901640-01.bin
        chargen
        edit1g
        edit2b
        edit2g
        edit4b40
        edit4b80
        edit4g40
        rpi_buus_pos.vkm
        rpi_buus_sym.vkm
        rpi_grus_pos.vkm
        rpi_grus_sym.vkm
        hre-9000.324992-02.bin
        hre-a000.324993-02.bin
        kernal1
        kernal2
        kernal4
        waterloo-a000.901898-01.bin
        waterloo-b000.901898-02.bin
        waterloo-c000.901898-03.bin
        waterloo-d000.901898-04.bin
        waterloo-e000.901897-01.bin
        waterloo-f000.901898-05.bin
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
        PET/
    disks/
        C64/
        C128/
        VIC20/
        PLUS4/
        PET/
    tapes/
        C64/
        C128/
        VIC20/
        PLUS4/
        PET/
    carts/
        C64/
        C128/
        VIC20/
        PLUS4/
    tmp/
        (used by the emulator sometimes)

# Build Instructions

Refer to BUILDING.md for instructions.

# Performance numbers can be found at https://accentual.com/bmc64
