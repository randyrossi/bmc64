#ifndef BMC64_KEYBOARD_FEATURE_H
#define BMC64_KEYBOARD_FEATURE_H

// Keep the legacy keyboard path as the development default.
#define BMC64_NEW_KEYBOARD_INPUT 1

#if BMC64_NEW_KEYBOARD_INPUT != 0 && BMC64_NEW_KEYBOARD_INPUT != 1
#error "BMC64_NEW_KEYBOARD_INPUT must be 0 or 1"
#endif

#endif