## 1.0.1
  * Initial release

## 1.0.2
  * GPIO joystick was broken in 1.0.1 - FIXED

## 1.0.3
  * Add hide_console=0 option in cmdline.txt to diagnose startup issues

## 1.0.4
  * Fixed save/restore joystick port selection issue
  * Pause emulation when ui is activated
  * Added tape controls (for .tap only)
  * Switch to soft usb irq for better gamepad support

## 1.0.5
  * Added configuration of usb button functions (fire or menu, more later)
  * Added drive sound emulation and volume to menu
  * Made menu larger
  * Moved USB config into a sub menu
  * Fixed hanging issue with gamepad when # analog axes > 4
  * Added raw usb monitor to help with gamepad config

## 1.0.6
  * Added Commodore + F7 to trigger menu for real keyboard users
  * Added ReSid
     * Added Fast/ReSid options (default to ReSid)
     * Added Sid model options 6581/8580 (default 6581)
     * Added Sid filter option (default On)
  * Had to increase kernel max to 32MB due to huge uninitialized
    data section added by libresid.a
  * Replaced inefficient bss clear routine with hand rolled asm
  * Placed main emulator loop on core 1 while core 0 services interrupts
  * Fixed bug with DB9 joysticks ports backwards for menu control

## 1.0.7
  * Fixed emulator lockup issue
    * Queues key events from ISR to main loop
  * Fixed keyboard issue with Commodore + F7 combo
    * Emulator would get stuck commodore key

## 1.0.8
  * Support read/write mode for files
    * Save to D64 files
    * Create snapshots
  * Better menu navigation with auto repeat and acceleration during hold
  * Added error/info dialogs for errors or confirmation of actions
  * Fixed wierd behavior with shift+down/right mapping to up/left for keyrah
  * Fixed audio cut out after sound engine restarted 8 times

## 1.0.9
  * Fix lockup issue caused by joystick value changes called from ISRs
  * Default PAL composite to progressive mode (18)
  * Fixed gpio16 not bringing up menu (still need to debounce)
  * Improved GPIO joystick response time (~45ms avg)

## 1.1
  * Added directory support
  * Long filename support added
  * Added Pepto-Pal color palette to prefs
  * Fixed audio sync bug for ntsc timing
  * Created tools dir for delay test programs
  * Made mounted partition configurable
  * Fixed unshiftable keys bug
  * Added temperature notes to README.md
  * gpu_mem changed to 64mb in mem config and config.txt
  * changed mem to 512mb in circle mem config
  * fixed issue with 0 key on snapshot file names crashing
  * debounced GPIO16 for menu
  * fixed issue with resid restart causing slowness

## 1.2
  * Fixed issue where defaults were not taking effect when
    settings file was missing
  * Fixed issue where info dialogs were triggering save
    snapshot dialog
  * Fixed IEC mode not able to change dirs using
    open 1,8,15,"cd:newdir":close 1
  * Added ability to mount extended partitions by start sector
  * Only flush RDWR in-memory files back to disk if written to
  * Fix issue with filenames > 32 chars not accessible

## 1.3
  * Boot time reduced to 5 seconds for both SID engine types.
  * Composite video refresh rate was discovered to be a tiny fraction higher than 50/60 hz and resulted in audio data accumulating in the audio buffer over time (causing audio latency). Added pal-composite and ntsc-composite machine_timing parameters to set it properly.
  * Changed main release format to a zip file with flat files rather than an img to flash. This will let people chose the size of their SD card partitions and make updating easier. The .img file is still available with a 300Mb partition now.

## 1.4
  * Moved to VICE 3.3
  * Added status bar in Prefs to show disk/tape counter/controls/motor etc
    at bottom of the screen.  It can be turned on permanently or triggered
    by activity and disappears after a timeout.
  * Fixed CRT attach issue that was leaking/clobbering memory
  * Fixed CBM File Browser not going into subdirectories (or up to parent)
  * Added missing fixup.dat so pi will report actual memory available
  * Added LeftControl + Cursor keys as joystick option
  * Got drives 9,10,11 working
  * Added 1541, 1571 and 1581 drive models with option to change

## 1.5
  * Properly idle cores (3 cores were spinning causing CPU temperature
    to be hotter that required)
  * Added option to not reset tape on machine reset
  * Added missing option to reset just counter (not tape)

## 1.6
  * OMG! Boot time down to 3.5 seconds!!!
  * Added brightness, gamma, tint and contrast controls
  * Fixed status bar showing at boot even though show never was selected
  * Fixed sid re-initialization not using correct frag/buf sizes
  * Re-designed disk drive model selection. More like a popup.  Must go
    back to emulator after each selection.
  * Save and restore drive models properly.
  * Added warp and joystick swap indicators to status bar
  * In addition to Fire/Menu, USB buttons can now be mapped to
    Warp, Status Bar Toggle and Joy Port Swap
  * Added error message if emulator won't start (possibly no roms)
  * Added ability to save default cartridge
  * Sid/DriveEmulation save/restore moved to vice.ini
  * Added load kernal, basic, chargen roms and save as defaults
  * Fixed audio sync bug at boot where audio was behind until a cart was
    attached or user spent some time in menu. Fixed with a hack to re-init
    sid engine after boot.

## 1.7
  * Internal prep for building BMCV20 and BMC128
  * Fixed issue where space or cntrl was not functional if curs+space or
    curs+lcontrol was selected for joystick
  * Added custom HDMI timing tool to calculate actual fps from custom modes
    Use to prevent audio sync / popping issues on modes that aren't exactly
    50 / 60 hz
  * Added audio_out cmdline.txt option (hdmi, analog, auto) for cases where
    auto doesn't do the right thing.
  * Added hot key functions for Keyrah users. Can map Commodore + F1, F3, F5, F7
    to one of the available functions (Menu, Warp, Toggle Status, Swap Joy, etc)
  * Added Autostart Disk/Tape/Prg/Cart menu option
  * Added Tape OSD to control tape using USB or HotKey
  * Added Cart OSD to control common cartridge functions using USB or HotKey
  * Added EasyFlash Save Now feature (also available in Cart OSD)
  * Added Cartridge Freeze feature (also available in Cart OSD)
  * Fixed bug where drive models would show up even though ROM was not available
  * Added threshold % for analog gamepad sticks

## 1.8
  * Added 1351 Mouse support via USB Mouse
  * Sorted files in directory listings
  * Added Reset Soft/Hard button/hotkey functions
  * Fixed 'half' brightness issue with color palette.
    Colors were half intensity of what they should have been.

## 1.9
  * First release of BMVIC20 available
  * Updates BMC128 available with a few fixes
  * cmdline.txt has changed and should be udpated:

    PSA:

    If you are in the habit of just updating the kernel file, best to update
    cmdline.txt with this release.  canvas_height,canvas_width parameters
    have been renamed to vicii_canvas_height, vicii_canvas_width to
    distinguish between C64 and VIC20 (or other future) video chips that need
    different sized frame buffers.  Also, I changed the way the emulated
    display is drawn to fix some unused black border issues on both C64/C128
    and VIC20 so the C64/C128 frame buffers are now a different size.  If
    you find your video output is squished after this update,
    best to confirm you are using the new cmdline.txt settings. This change
    'zooms' the video in a bit so more of the screen is used.  It also looks
    better on composite out.

  * Moved bootstat.txt into machine dirs so they can be different for machines
  * Fixed C128 kernal/basic/chargen ROM load menu options
  * Fixed bug where POT Y Up/Down values could not be set
  * Fixed bug with some 'empty' USB button function menu options
  * Added menu item to create empty disks
  * Reorg some menu items
  * Added Home/PageUp/PageDown/End handling in menu (also F1/F3/F5/F7)
    for real keyboard users
  * Added first letter search in menus (press a letter and cursor will jump
    to next occurrence of an item that starts with that letter).

## 2.0
  * Fixed default values for bootstat for each machine
  * Added Menu Back/Run Stop usb button for easier menu nav from gamepad
  * Handled real keyboard scan into ui
  * Add custom keysets for joysticks
  * Add cuttom key bindings for usb buttons
  * Remember last cursor position in file lists
  * Fixed badling named cycles_per_refresh to cycles_per_second
    (but old name still works)
  * Put canvas height back to 288 for C64

## 2.1
  * Major overhaul of how video is handled
    * Added C128 VDC support with VIC/VDC toggle as well as
      PIP and SIDE-BY-SIDE display options
    * C128 40/80 Column Key
    * Added separate layers for VIC, VDC, UI and STATUS bar using dispmanx API
    * Can adjust video canvas border trim and aspect ratio in real time now
      (all *_canvas_width, *_canvas_height params removed from cmdline.txt)
    * Used VICE buffer alloc/free/clear callbacks to have VICE draw directly
      into fb2's frame buffer
    * Added transparency support to UI layer
    * UI is made transparent while video adjustments made to show user what
      is changing
    * Added status bar padding to adjust location of status bar

## 2.2
  * Minor regression on audio latency fixed. Badly placed previous fix was
    causing extra frame delay.

## 2.3
  * Add virtual keyboard
  * Added vertical/horizontal positioning menu options
  * Applied video settings to menu so it is made visible same as VIC display
  * Fixed navigation bug inside C128 rom files list, couldn't navigate out
  * Removed 2nd LED for drive status lights (never seemed to activate)
  * Added bmc64-lite build for PiZero
  * Made palettes independent for each layer

## 2.3.1
  * Fixed regression with status bar, showing old/default values
  * Fixed HOME/DEL/Pound keys for Vic20 Virtual Keyboard
  * Show PETSCII chars when virtual keyboard shift/commodore held

## 2.4
  * Activated real keyboard and joystick scan for PCB
  * Added GPIO Config PCB or No-PCB options + documentation
  * Added CTRL + F[1,3,5,7] Programmable Hotkeys
  * Added ability to mount up to 3 USB thumb drives
       * Default drive is SD: but USB1, USB2, USB3 can be selected
         by selecting the first item in any files dialog.
       * Only the first partition will be mounted
  * Added ability to set IEC mode on all drives, not just 8
  * Added selection of IEC dir
  * Paint menus stacked on top of each other like they used to be
  * Fixed tape counter not updating properly (again)

## 2.5
  * Added Plus/4
  * Added WaveShare Game HAT GPIO support
  * Navigate virtual keyboard with GP even if not set in a port
  * Navigate virtual keyboard with GP when GPIO banks 1 or 2 set to any port
  * Menu reorg : +Machine, moved ROMs under
  * Add drive ROM change menu item
  * Add parallel cable option on drives
  * Fix GPIO virtual keyboard navigation too fast
  * Added Menu navigation button options to GPIO config 1
  * Added Volume control
  * Fixed wrong keycode for Alt_L
  * Cleaned up keycodes in rpi_sym.kym files

## 2.5.1
  * Fixed Vic20 cartridge menu
  * Fixed Plus/4 cartridge menu
  * Real keyboard matrix for Plus/4 to match C16 positional layout

## 3.0
  * Unified C64, VIC20, C128 and PLUS/4 into one release
  * Fixed cycles_per_second bug causing custom hdmi to not work
  * Look for carts, tapes, disks etc in machine subdir first

## 3.1
  * Added Plus4Emu option (more accurate PLUS/4 but Pi3 only)
  * Reboot after machine switch
  * Fix mouse issue with Final Cart III
  * Fix datasette not available after snapshot load
  * Fix ntsc-custom timing mode sound sync issue
  * Added memory config option for plus/4
  * Added drive memory options for VICE emus
  * Lots of internal src reorg, new layers to plugin different emulators

## 3.2
  * Fixed VIC20 crash on Port 1 menu option change to 1351 Mouse
  * Fix 1551 and 1581 ROMs not loading in plus4emu
  * Added Symbolic and Postional keyboard options

    Defaults to Symbolic which is different from what was released up
    to now. Named the keymap file incorrectly.

  * Added disabled option to gpio config
  * Moved up to v40 of circle in prep for Pi4

## 3.3
  * Add PET emulator
  * Add DPI support
  * Add 2 more USB gamepads for user port joysticks
  * Renamed menu label 'Aspect Ratio' to 'H Stretch Factor'
  * Plus4Emu filter on PRG files for Load .PRG menu
  * Fixed Composite out due to bootcode.bin/start.elf/fixup.dat files
  * Removed custom HDMI timing from plus4emu until I can fix sound issues
  * Removed incorrect 50.125hz VIC20 mode from machines.txt
  * Menu shows actual fps for mode
  * Virtual keyboard for PET does not work

## 3.4
  * Added Resid Interpolation and Fast Resampling for Pi3
  * Added vertical stretch to better align with real scanlines on CRTs
  * Added saturation to video color menu
  * Added "Look for files in" option: /C64/{dir} or /{dir}/C64
  * Added (cheap) PET scanlines video option
  * Added configurable gpio
  * Fixed IEC mode not working after plus4emu attach/detach
  * Fixed PET color and video settings after boot not applied
  * Fixed bug with files list if /disks dir was missing
  * Fixed DRIVES drive rom discovery issue
  * Fixed PET vertical centering issue
  * Fixed issue with debounce routine on GPIO button functions
  * Removed drive roms from bootstat to not interfere with path lookup
  * Merged Mark Bush's Userport changes (See README.md on how to activate)

## 3.5
  * Allow same joystick GPIO mapping on multiple pins with custom GPIO
  * Added 'Maxi' keyboard support , new vkm files switchable from menu
    (Thanks to youtube user 'ody ody' for figuring out the keymap file! See
     README.md for tutorial link and Reddit post.)
  * Fixed CP/M keyboard not responsive regression
  * Fixed duplicate GPIO pins on Config #1 issue
  * Disable VDC video cache causing glitches on some games
  * Added Dual SID support (also made pseudo stereo effect possible)
  * Changed border trim menu options from % to absolute pixel values
  * Added support for negative border values (padding)
  * Added scaling_params to machines.txt for integer scaling when possible
  * Made finding integer scaling options easier from menu
  * Changed PET resolutions to 1080p in default machines.txt
  * Fixed bug with PET screen init after model switch
  * Added menu optionm to turn scaling interpolation off

## 3.6
  * Added davej's CRT shader
  * Removed 'cheap' scanlines from PET (replaced by shader)
  * Increased Pi0 audio sample rate to 44100
  * Added AutostartWarp switch
  * Added DriveTrueEmulation switch
  * Fixed stretch params not saving for VDC (C128)
  * Fixed bug with raster_skip (cmdline.txt)
  * Fixed incorrect scaling param in machines.txt NTSC C128 59.825hz
  * Fixed "Sound running too slow" problem on Pi0

## 3.7
  * Fixed RCNTRL key not recognized in keymap files
  * Provided keymap file for real C16 keyboards
  * Applied XBOX num buttons patch by rhester
  * Use gamepad analog axes as paddles (for custom built controllers)
  * Apply 8580 filter fix (rhester)
  * Fix transparency not restored properly after reboot
  * Fixed issue #163 with custom gpio preventing gamepad input
  * Enable dual sid menu for C128
  * Enabled sound drive emulation for Pi0
  * Added virtual devices menu option (#140)
  * Added disk drive flush function
  * Added PETSCIIBOARD keyboard mapping
  * Fix build issue due to -pthread being added

## 4.0
  * Boot to VDC if 80 column key is down (C128)
  * Fix CNTRL-F7 safety switch problem if roms not found
  * Add raster_skip2 to enable raster skip for VDC only

## 4.1
  * VIC20 symbolic keymap - F3 maps to C=
  * Add cmdhd support for C64OS (requires JiffyDOS)

## 4.2
  * Fix incorrect END key mapping
  * Add 256k and 1024k memory options to plus4emu
  * Support more recent Maxi keyboard

