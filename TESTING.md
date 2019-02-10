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
