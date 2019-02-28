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
