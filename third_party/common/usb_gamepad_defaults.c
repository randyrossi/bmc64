#include "circle.h"
#include "joy.h"
#include "usb_gamepad_defaults.h"

static unsigned usb_gamepad_mapping_profiles[MAX_USB_DEVICES];

extern int usb_pref[MAX_USB_DEVICES];
extern int usb_x_axis[MAX_USB_DEVICES];
extern int usb_y_axis[MAX_USB_DEVICES];
extern float usb_x_thresh[MAX_USB_DEVICES];
extern float usb_y_thresh[MAX_USB_DEVICES];
extern int usb_button_assignments[MAX_USB_DEVICES][MAX_USB_BUTTONS];

static const struct usb_gamepad_button_default usb_gamepad_generic_button_defaults[] = {
  { 0, BTN_ASSIGN_FIRE },
  { 1, BTN_ASSIGN_FIRE },
  { 2, BTN_ASSIGN_FIRE },
  { 3, BTN_ASSIGN_FIRE },
  { 4, BTN_ASSIGN_RUN_STOP_BACK },
  { 8, BTN_ASSIGN_MENU },
};

static const struct usb_gamepad_button_default usb_gamepad_8bitdo_ultimate_c_button_defaults[] = {
  { 5, BTN_ASSIGN_RUN_STOP_BACK },
  { 7, BTN_ASSIGN_FIRE },
  { 8, BTN_ASSIGN_FIRE },
  { 9, BTN_ASSIGN_FIRE },
  { 10, BTN_ASSIGN_FIRE },
  { 11, BTN_ASSIGN_MENU },
  { 15, BTN_ASSIGN_UP },
  { 16, BTN_ASSIGN_RIGHT },
  { 17, BTN_ASSIGN_DOWN },
  { 18, BTN_ASSIGN_LEFT },
};

static const struct usb_gamepad_default_profile usb_gamepad_default_profiles[] = {
  {
    USB_GAMEPAD_DEFAULT_PROFILE_8BITDO_ULTIMATE_C,
    "8BitDo Ultimate C 2.4G",
    USB_PREF_ANALOG,
    0,
    1,
    .50,
    .50,
    usb_gamepad_8bitdo_ultimate_c_button_defaults,
    sizeof(usb_gamepad_8bitdo_ultimate_c_button_defaults) /
        sizeof(usb_gamepad_8bitdo_ultimate_c_button_defaults[0]),
  },
};

void emu_set_usb_gamepad_mapping_profile(int device, unsigned profile) {
  if (device >= 0 && device < MAX_USB_DEVICES) {
    usb_gamepad_mapping_profiles[device] = profile;
  }
}

const struct usb_gamepad_default_profile *usb_gamepad_default_profile_for_device(
    int device) {
  if (device < 0 || device >= MAX_USB_DEVICES ||
      usb_gamepad_mapping_profiles[device] == USB_GAMEPAD_DEFAULT_PROFILE_NONE) {
    return 0;
  }

  for (unsigned i = 0;
       i < sizeof(usb_gamepad_default_profiles) / sizeof(usb_gamepad_default_profiles[0]);
       i++) {
    if (usb_gamepad_mapping_profiles[device] ==
      usb_gamepad_default_profiles[i].id) {
      return &usb_gamepad_default_profiles[i];
    }
  }

  return 0;
}

const char *usb_gamepad_default_profile_display_name_for_device(int device) {
  const struct usb_gamepad_default_profile *profile =
      usb_gamepad_default_profile_for_device(device);
  return profile != 0 ? profile->display_name : 0;
}

void usb_gamepad_reset_to_defaults(int device) {
  if (device < 0 || device >= MAX_USB_DEVICES) {
    return;
  }

  usb_pref[device] = USB_PREF_ANALOG;
  usb_x_axis[device] = 0;
  usb_y_axis[device] = 1;
  usb_x_thresh[device] = .50;
  usb_y_thresh[device] = .50;
  for (unsigned button = 0; button < MAX_USB_BUTTONS; button++) {
    usb_button_assignments[device][button] = BTN_ASSIGN_UNDEF;
  }

  const struct usb_gamepad_default_profile *profile =
      usb_gamepad_default_profile_for_device(device);
  if (profile == 0) {
    for (unsigned i = 0;
         i < sizeof(usb_gamepad_generic_button_defaults) /
                 sizeof(usb_gamepad_generic_button_defaults[0]);
         i++) {
      unsigned button = usb_gamepad_generic_button_defaults[i].button;
      usb_button_assignments[device][button] =
          usb_gamepad_generic_button_defaults[i].assignment;
    }
    return;
  }

  usb_pref[device] = profile->direction_mode;
  usb_x_axis[device] = profile->x_axis;
  usb_y_axis[device] = profile->y_axis;
  usb_x_thresh[device] = profile->x_threshold;
  usb_y_thresh[device] = profile->y_threshold;
  for (unsigned i = 0; i < profile->button_default_count; i++) {
    unsigned button = profile->button_defaults[i].button;
    if (button < MAX_USB_BUTTONS) {
      usb_button_assignments[device][button] =
          profile->button_defaults[i].assignment;
    }
  }
}
