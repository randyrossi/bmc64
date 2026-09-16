// usb_descriptors.h - keyrah_test
//
// Interface layout matches the real Keyrah V3: 2 boot-protocol keyboard
// interfaces, 2 joystick-shaped generic-HID interfaces, and 2 more
// generic-HID interfaces carrying "Multimedia Control" and "ACPI Power
// Control" usages. A CDC interface is added on top for this mock's own test
// console - Circle binds interfaces purely by class/subclass/protocol, so
// the extra interface doesn't change how the 6 HID ones are seen.

#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#include "tusb.h"

enum {
  ITF_KBD1 = 0,   // boot keyboard -> Circle ukbd1
  ITF_KBD2,       // boot keyboard -> Circle ukbd2
  ITF_JOY1,       // generic HID   -> Circle upad1
  ITF_JOY2,       // generic HID   -> Circle upad2
  ITF_CONSUMER,   // generic HID, Consumer Control usage
  ITF_SYSCTRL,    // generic HID, System Control usage
  ITF_NUM_HID_TOTAL,
};

enum {
  ITF_CDC = ITF_NUM_HID_TOTAL,  // uses ITF_CDC and ITF_CDC+1 internally (TUD_CDC_DESCRIPTOR)
  ITF_NUM_TOTAL = ITF_CDC + 2,
};

// bInterval (ms, full-speed), set separately for the keyboard and
// gamepad-shaped interfaces so they can be tuned independently. Set via
// CMakeLists.txt target_compile_definitions; both default to 1 (1000Hz,
// matching the real Keyrah's descriptors) if not overridden.
#ifndef KEYRAH_KBD_POLL_INTERVAL_MS
#define KEYRAH_KBD_POLL_INTERVAL_MS   1
#endif
#ifndef KEYRAH_PAD_POLL_INTERVAL_MS
#define KEYRAH_PAD_POLL_INTERVAL_MS   1
#endif

typedef struct TU_ATTR_PACKED {
  uint8_t x, y, rx, ry;   // 0-255
  uint8_t hat;            // 1-8 (see above)
  uint8_t buttons;        // bit0-2 = buttons 1-3, bits 3-7 unused
} keyrah_joy_report_t;

#define HID_EP_SIZE               48

#endif
