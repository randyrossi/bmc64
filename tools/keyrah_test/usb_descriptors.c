// usb_descriptors.c - keyrah_test
//
// Device/config/string/report descriptors. See usb_descriptors.h for the
// interface layout and its correspondence to the real Keyrah V3.

#include <string.h>
#include "usb_descriptors.h"

//--------------------------------------------------------------------+
// Device Descriptor
//--------------------------------------------------------------------+

tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,

    // Composite device, class/subclass/protocol declared per-interface -
    // matches the real Keyrah (bDeviceClass=0 in keyrah-lsusb-corrected.txt).
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    // Deliberately NOT the real Keyrah's VID:PID (0x18D8:0x0003). Circle
    // binds these interfaces purely by class/subclass/protocol - see
    // lib/usb/usbdevicefactory.cpp - so matching the real IDs is not needed,
    // and using a different one makes it obvious in any log/lsusb dump that
    // you're looking at the mock, not real Keyrah hardware.
    .idVendor           = 0xCafe,   // TinyUSB's shared test/example VID
    .idProduct          = 0x4B52,   // arbitrary ("KR")
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01,
};

uint8_t const *tud_descriptor_device_cb(void) {
  return (uint8_t const *) &desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptors
//--------------------------------------------------------------------+

uint8_t const desc_hid_report_kbd[] = {
  TUD_HID_REPORT_DESC_KEYBOARD()
};

uint8_t const desc_hid_report_joy[] = {
  HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP ),
  HID_USAGE      ( HID_USAGE_DESKTOP_GAMEPAD ),
  HID_COLLECTION ( HID_COLLECTION_APPLICATION ),
    HID_USAGE_PAGE     ( HID_USAGE_PAGE_DESKTOP ),
    HID_USAGE          ( HID_USAGE_DESKTOP_X ),
    HID_USAGE          ( HID_USAGE_DESKTOP_Y ),
    HID_USAGE          ( HID_USAGE_DESKTOP_RX ),
    HID_USAGE          ( HID_USAGE_DESKTOP_RY ),
    HID_LOGICAL_MIN    ( 0x00 ),
    HID_LOGICAL_MAX_N  ( 0x00ff, 2 ),
    HID_PHYSICAL_MIN   ( 0x00 ),
    HID_PHYSICAL_MAX_N ( 0x00ff, 2 ),
    HID_REPORT_COUNT   ( 4 ),
    HID_REPORT_SIZE    ( 8 ),
    HID_INPUT          ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),

    HID_USAGE_PAGE     ( HID_USAGE_PAGE_DESKTOP ),
    HID_USAGE          ( HID_USAGE_DESKTOP_HAT_SWITCH ),
    HID_LOGICAL_MIN    ( 1 ),
    HID_LOGICAL_MAX    ( 8 ),
    HID_PHYSICAL_MIN   ( 0x00 ),
    HID_PHYSICAL_MAX_N ( 0x013b, 2 ),
    HID_REPORT_COUNT   ( 1 ),
    HID_REPORT_SIZE    ( 8 ),
    HID_INPUT          ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),

    HID_USAGE_PAGE     ( HID_USAGE_PAGE_BUTTON ),
    HID_USAGE_MIN      ( 1 ),
    HID_USAGE_MAX      ( 3 ),
    HID_LOGICAL_MIN    ( 0x00 ),
    HID_LOGICAL_MAX    ( 0x01 ),
    HID_REPORT_COUNT   ( 8 ),
    HID_REPORT_SIZE    ( 1 ),
    HID_INPUT          ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),
  HID_COLLECTION_END
};

// Consumer Control (volume/media keys) and System Control (power/sleep/wake)
// usages - the real Keyrah's "Multimedia Control" and "ACPI Power Control"
// interfaces. Neither is driven by anything in this mock; what matters is
// that Circle sees the same interface shapes as the real device rather than
// two extra gamepads.
uint8_t const desc_hid_report_consumer[] = {
  TUD_HID_REPORT_DESC_CONSUMER()
};

uint8_t const desc_hid_report_syscontrol[] = {
  TUD_HID_REPORT_DESC_SYSTEM_CONTROL()
};

uint8_t const *tud_hid_descriptor_report_cb(uint8_t itf) {
  switch (itf) {
    case ITF_KBD1:
    case ITF_KBD2:     return desc_hid_report_kbd;
    case ITF_CONSUMER: return desc_hid_report_consumer;
    case ITF_SYSCTRL:  return desc_hid_report_syscontrol;
    default:           return desc_hid_report_joy;   // ITF_JOY1/ITF_JOY2
  }
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

#define EPNUM_KBD1       0x81
#define EPNUM_KBD2       0x82
#define EPNUM_JOY1       0x83
#define EPNUM_JOY2       0x84
#define EPNUM_CONSUMER   0x85
#define EPNUM_SYSCTRL    0x86
#define EPNUM_CDC_NOTIF  0x87
#define EPNUM_CDC_OUT    0x02
#define EPNUM_CDC_IN     0x88

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN * ITF_NUM_HID_TOTAL + TUD_CDC_DESC_LEN)

uint8_t const desc_configuration[] = {
  // Config number, interface count, string index, total length, attribute, power in mA
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

  //                    itf,      str, boot_protocol,             report_desc,                 ep,         size,        interval
  TUD_HID_DESCRIPTOR(ITF_KBD1, 0, HID_ITF_PROTOCOL_KEYBOARD, sizeof(desc_hid_report_kbd), EPNUM_KBD1, HID_EP_SIZE, KEYRAH_KBD_POLL_INTERVAL_MS),
  TUD_HID_DESCRIPTOR(ITF_KBD2, 0, HID_ITF_PROTOCOL_KEYBOARD, sizeof(desc_hid_report_kbd), EPNUM_KBD2, HID_EP_SIZE, KEYRAH_KBD_POLL_INTERVAL_MS),
  TUD_HID_DESCRIPTOR(ITF_JOY1, 0, HID_ITF_PROTOCOL_NONE,     sizeof(desc_hid_report_joy), EPNUM_JOY1, HID_EP_SIZE, KEYRAH_PAD_POLL_INTERVAL_MS),
  TUD_HID_DESCRIPTOR(ITF_JOY2, 0, HID_ITF_PROTOCOL_NONE,     sizeof(desc_hid_report_joy), EPNUM_JOY2, HID_EP_SIZE, KEYRAH_PAD_POLL_INTERVAL_MS),
  TUD_HID_DESCRIPTOR(ITF_CONSUMER, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report_consumer),   EPNUM_CONSUMER, HID_EP_SIZE, KEYRAH_PAD_POLL_INTERVAL_MS),
  TUD_HID_DESCRIPTOR(ITF_SYSCTRL,  0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report_syscontrol), EPNUM_SYSCTRL,  HID_EP_SIZE, KEYRAH_PAD_POLL_INTERVAL_MS),

  // Interface number, string index, EP notification address & size, EP data address (out, in), size
  TUD_CDC_DESCRIPTOR(ITF_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),
};

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
  (void) index;
  return desc_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

char const *string_desc_arr[] = {
  (const char[]) { 0x09, 0x04 },      // 0: supported language = English (0x0409)
  "BMC64",                             // 1: Manufacturer
  "Keyrah V3 mock (keyrah_test)",     // 2: Product
  "KEYRAHTEST0001",                   // 3: Serial - fixed so it's easy to spot in lsusb
  "keyrah_test console",              // 4: CDC interface name
};

static uint16_t _desc_str[32];

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  (void) langid;
  size_t chr_count;

  if (index == 0) {
    memcpy(&_desc_str[1], string_desc_arr[0], 2);
    chr_count = 1;
  } else {
    if (index >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) return NULL;

    const char *str = string_desc_arr[index];
    chr_count = strlen(str);
    if (chr_count > 31) chr_count = 31;

    for (size_t i = 0; i < chr_count; i++) {
      _desc_str[1 + i] = str[i];
    }
  }

  _desc_str[0] = (uint16_t) ((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));
  return _desc_str;
}
