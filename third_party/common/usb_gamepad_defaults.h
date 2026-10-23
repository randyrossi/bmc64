//
// usb_gamepad_defaults.h
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

#ifndef BMC64_USB_GAMEPAD_DEFAULTS_H
#define BMC64_USB_GAMEPAD_DEFAULTS_H

#define USB_GAMEPAD_DEFAULT_PROFILE_NONE 0
#define USB_GAMEPAD_DEFAULT_PROFILE_XBOX360 1
#define USB_GAMEPAD_DEFAULT_PROFILE_8BITDO_M30_2_4G 2

struct usb_gamepad_button_default {
  unsigned button;
  int assignment;
};

struct usb_gamepad_default_profile {
  unsigned id;
  const char *display_name;
  int direction_mode;
  int x_axis;
  int y_axis;
  float x_threshold;
  float y_threshold;
  const struct usb_gamepad_button_default *button_defaults;
  unsigned button_default_count;
};

void emu_set_usb_gamepad_mapping_profile(int device, unsigned profile);
void emu_set_usb_gamepad_display_name(int device, const char *display_name);

unsigned usb_gamepad_default_profile_for_vendor(const char *vendor);
const struct usb_gamepad_default_profile *usb_gamepad_default_profile_for_device(
    int device);
const char *usb_gamepad_default_profile_display_name_for_device(int device);
void usb_gamepad_reset_to_defaults(int device);

#endif
