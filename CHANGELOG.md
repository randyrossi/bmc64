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
