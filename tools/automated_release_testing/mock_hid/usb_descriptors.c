//
// BMC64 - A bare metal Commodore emulator for Raspberry Pi
// Copyright (C) 2026 aminch <https://github.com/aminch>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include <string.h>
#include "tusb.h"

static const tusb_desc_device_t device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    /* Test-only IDs, following tools/keyrah_test; not production USB IDs. */
    .idVendor = 0xcafe,
    .idProduct = 0x424d,
    .bcdDevice = 0x0100,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1,
};

static const uint8_t keyboard[] = { TUD_HID_REPORT_DESC_KEYBOARD() };
static const uint8_t mouse[] = { TUD_HID_REPORT_DESC_MOUSE() };
static const uint8_t pad[] = {
    HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
    HID_USAGE(HID_USAGE_DESKTOP_GAMEPAD),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),
        HID_USAGE(HID_USAGE_DESKTOP_X),
        HID_USAGE(HID_USAGE_DESKTOP_Y),
        HID_LOGICAL_MIN(0),
        HID_LOGICAL_MAX_N(255, 2),
        HID_REPORT_COUNT(2),
        HID_REPORT_SIZE(8),
        HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
        HID_USAGE_PAGE(HID_USAGE_PAGE_BUTTON),
        HID_USAGE_MIN(1),
        HID_USAGE_MAX(8),
        HID_LOGICAL_MIN(0),
        HID_LOGICAL_MAX(1),
        HID_REPORT_COUNT(8),
        HID_REPORT_SIZE(1),
        HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
    HID_COLLECTION_END
};

static const uint8_t config[] = {
    TUD_CONFIG_DESCRIPTOR(1, 3, 0,
        TUD_CONFIG_DESC_LEN + 3 * TUD_HID_DESC_LEN, 0, 100),
    TUD_HID_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_KEYBOARD,
        sizeof(keyboard), 0x81, 16, 1),
    TUD_HID_DESCRIPTOR(1, 0, HID_ITF_PROTOCOL_MOUSE,
        sizeof(mouse), 0x82, 16, 1),
    TUD_HID_DESCRIPTOR(2, 0, HID_ITF_PROTOCOL_NONE,
        sizeof(pad), 0x83, 16, 1),
};

const uint8_t *tud_descriptor_device_cb(void) {
    return (const uint8_t *)&device;
}

const uint8_t *tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return config;
}

const uint8_t *tud_hid_descriptor_report_cb(uint8_t instance) {
    switch (instance) {
        case 0: return keyboard;
        case 1: return mouse;
        case 2: return pad;
        default: return NULL;
    }
}

const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    static uint16_t result[32];
    static const char *const strings[] = {
        "", "BMC64", "Release test mock HID", "BMC64MOCK0001"
    };
    if (index == 0) {
        result[0] = (TUSB_DESC_STRING << 8) | 4;
        result[1] = 0x0409;
        return result;
    }
    if (index >= sizeof(strings) / sizeof(strings[0])) return NULL;
    size_t length = strlen(strings[index]);
    if (length > 31) length = 31;
    result[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * length + 2));
    for (size_t i = 0; i < length; ++i)
        result[i + 1] = (uint8_t)strings[index][i];
    return result;
}
