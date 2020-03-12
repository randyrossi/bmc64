# JS1=None, JS2=None
 * Test: Cursor keys and space work in emulator
 * Test: Cursor keys work in menu
 * Test: Gamepad does nothing
 * Test: GPIO pins do nothing
 * Test: Num keys do nothing
# JS1=CURS+SPACE, JS1 = None
 * Test: Cursor keys are joystick in emulator
 * Test: Cursor keys work in menu
# JS1=None, JS2 = CURS+SPACE
 * Test: Cursor keys are joystick in emulator
 * Test: Cursor keys work in menu
# JS1=NUMS1, JS2=None
 * Test: Numpad are joystick in emulator
 * Test: Numpad navigates in menu
# JS1=None, JS2=NUMS1
 * Test: Numpad are joystick in emulator
 * Test: Numpad navigates in menu
# JS1=NUMS2, JS2=None
 * Test: Numpad are joystick in emulator
 * Test: Numpad navigates in menu
# JS1=None, JS2=NUMS2
 * Test: Numpad are joystick in emulator
 * Test: Numpad navigates in menu
# JS1=USB1, JS2=None
 * Test: Gamepad is joystick in emulator
 * Test: Gamepad navigates in menu
 * Test: Gamepad fire/menu button assignments work
# JS1=None, JS2=USB1
 * Test: Gamepad is joystick in emulator
 * Test: Gamepad navigates in menu
 * Test: Gamepad fire/menu button assignments work
# JS1=GPIO1, JS2=None
 * Test: GPIO bank1 is joystick in emulator
 * Test: GPIO bank1 navigates in menu
 * Test: Gamepad fire/menu button assignments work
# JS1=None, JS2=GPIO1
 * Test: GPIO bank1 is joystick in emulator
 * Test: GPIO bank1 navigates in menu
 * Test: Gamepad fire/menu button assignments work
# JS1=GPIO2, JS2=None
 * Test: GPIO bank2 is joystick in emulator
 * Test: GPIO bank2 navigates in menu
# JS1=None, JS2=GPIO2
 * Test: GPIO bank2 is joystick in emulator
 * Test: GPIO bank2 navigates in menu
# WARP=On
 * Test: emulated machine has no sound and no delay
# IEC 8=On
 * Test: LOAD "$",8 does not take forever or halt machine
# Comaland
 * Test: horizontal scrolling has no jitter/tearing
 * Test: audio does not pop on 1st disk/last sequence
# Ghostbusters:
 * Test: bouncing ball sync with text
 * Test: audio does not pop
# Keyrah:
 * Test: All keys work, including =
 * Test: Commodore + F7 = Menu
# Save Settings:
 * Test: values are restored properly
# Save Snapshot:
 * With tape attached
 * Load it (creates tmp file in /tmp)
 * With disk attached
 * Load it
# Menu nav repeat
 * Hold up/down/left/right and check acceleration
# Missing expected dir
 * Should fallback to root
# Navigate 3-4 levels deep
# Navigate back up
# Navigate to root from default dir
# All files in one partition
# Boot files in partition 1, Emulator files in partition 2
# Commodore+F7 + simpsons.crt reset bug
# Load kernal, basic or chargen
# Vice settings persist to vice.ini
# Boot with Resid with key/joy delaytest attached as cart
 * Delay should still be ~80ms (was bug with rel < 1.6)
* Custom keysets , test pot x works with SMB
* Test button assignments for key bindings honor up/down
* Button assignments for dirs should work in ui too
* Dolphon DOS works
* Keyboard works in CP/M mode on C128
* PET scanlines on/off appears right
