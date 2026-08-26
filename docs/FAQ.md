# BMC64 FAQ <!-- omit in toc -->


## Contents <!-- omit in toc -->

- [Can I switch the machine to NTSC?](#can-i-switch-the-machine-to-ntsc)
- [Why does the video look soft/stretched/dark?](#why-does-the-video-look-softstretcheddark)
- [Can I change option 'X' in VICE?](#can-i-change-option-x-in-vice)
- [Hey, isn't the real thing running at 50.125Hz?](#hey-isnt-the-real-thing-running-at-50125hz)
- [Why is audio not coming out of HDMI or the analog jack?](#why-is-audio-not-coming-out-of-hdmi-or-the-analog-jack)
- [How do I get Dolphin DOS to work?](#how-do-i-get-dolphin-dos-to-work)
- [Why won't PetRescue attach in Plus4Emu?](#why-wont-petrescue-attach-in-plus4emu)
- [What do I put on the SD card?](#what-do-i-put-on-the-sd-card)

## Can I switch the machine to NTSC?

**Answer:** Yes, in V3.0 or higher, do this from the Machine->Switch menu.

   Be aware that some demos/games will not run if the machine is NTSC.  If you mix those two settings, your machine will likely run fast/slow, not look good or have audio issues (or all of the above).

## Why does the video look soft/stretched/dark?

**Answer:** By default, the scaling_kernel option softens the scaled image.  You can change this (see above).  Also, some monitors seem to stretch the canvas to full width rather than keep the aspect ratio.  You can change the hdmi mode and frame buffer aspect ratio in config.txt to suit your needs. Just keep in mind that NTSC machine timing must have a 60hz mode and PAL machine timing must have a 50hz mode.  This isn't something you normally care about in an emulator but because the machine is actually timed to the vertical blank signal of the video device, it matters.  I also recommend changing the mode to match your monitor/TV's native resolution.  That should avoid any unnecessary extra processing your monitor/TV might do which can add lag.  Also, place your Monitor/TV into 'Game' mode if possible.

Things you can fiddle with for video:

   hdmi_mode in config.txt
   framebuffer_aspect in config.txt (i.e. framebuffer_aspect=0x00070009 for 7:9)
   scaling_kernel (google for available algorithms)

## Can I change option 'X' in VICE?

**Answer:** Most video options are fixed right now to make sure video is rendered properly. VICE settings are read from 'vice.ini' located in the root of the emulator's partition. If there is no UI support for the option you are looking for, you can try setting it there using VICE documentation.

## Hey, isn't the real thing running at 50.125Hz?

**Answer:** Yes, the original machine ran at 50.125Hz for PAL and 59.826Hz for NTSC. So, yeah, you'll be about 0.25% off in terms of timing.  If you really want 50.125Hz, you can try this custom HDMI mode (only applies to HDMI):

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

## Why is audio not coming out of HDMI or the analog jack?

**Answer:** Sometimes the code that auto detects whether audio should be piped through HDMI vs analog jack doesn't work.  You can force audio to where you want it with a audio_out=hdmi or audio_out=analog parameter.  The default is audio_out=auto.  This ends up in cmdline.txt but you should set it in every machine config in machines.txt you want it to show up for.

## How do I get Dolphin DOS to work?

**Answer:** Follow these steps.

From: http://rr.pokefinder.org/wiki/Dolphin_DOS

From the menu:

    Machine -> ROMS -> Change Kernal ROM -> kernel-20-1.rom
    Drives -> Change ROM -> 1541 -> c1541-20-6.rom
    Drive 8
       Change Model -> 1541
       Options -> Parallel Cable -> Standard
       Options -> RAM 6000 On

## Why won't PetRescue attach in Plus4Emu?

**Answer:** Plus4Emu is picky about the size of the disk images it attaches.  PetRescue has extra sectors but for some reason, the d64 image is 197376 bytes which doesn't complete a full track. To get it to attach, you will have to pad it with extra 0's. The size after padding should be 200960.

On Linux, this command will work:

truncate -s +3584 petrescue.d64

## What do I put on the SD card?

**Answer:** See [INSTALLATION.md](INSTALLATION.md) for the required ROMs. The SD-card layout is shown below:

    C64/
        kernal
        basic
        chargen
        d1541II
        dos1541  (optional)
        dos1571  (optional)
        dos1581  (optional)
        dosCMDHD (optional)
        dos2000  (optional)
        dos4000  (optional)
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
