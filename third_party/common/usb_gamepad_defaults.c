#include "circle.h"
#include "joy.h"
#include "usb_gamepad_defaults.h"

#include <string.h>

struct usb_gamepad_vendor_default_profile {
  const char *vendor;
  unsigned profile;
};

static const struct usb_gamepad_vendor_default_profile
    usb_gamepad_vendor_default_profiles[] = {
  { "ven45e-28e", USB_GAMEPAD_DEFAULT_PROFILE_XBOX360 },
  { "ven45e-28f", USB_GAMEPAD_DEFAULT_PROFILE_XBOX360 },
  { "ven45e-719", USB_GAMEPAD_DEFAULT_PROFILE_XBOX360 },
  { "ven45e-2a9", USB_GAMEPAD_DEFAULT_PROFILE_XBOX360 },
  { "ven2dc8-3106", USB_GAMEPAD_DEFAULT_PROFILE_XBOX360 },
  { "ven2dc8-310b", USB_GAMEPAD_DEFAULT_PROFILE_XBOX360 },
  { "venca3-24", USB_GAMEPAD_DEFAULT_PROFILE_8BITDO_M30_2_4G },
};

#define USB_GAMEPAD_DISPLAY_NAME_MAX 32

static unsigned usb_gamepad_mapping_profiles[MAX_USB_DEVICES];
static char usb_gamepad_display_names[MAX_USB_DEVICES]
                                           [USB_GAMEPAD_DISPLAY_NAME_MAX + 1];

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

static const struct usb_gamepad_button_default usb_gamepad_xbox360_button_defaults[] = {
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

static const struct usb_gamepad_button_default usb_gamepad_8bitdo_m30_2_4g_button_defaults[] = {
  { 0, BTN_ASSIGN_FIRE },
  { 1, BTN_ASSIGN_FIRE },
  { 2, BTN_ASSIGN_FIRE },
  { 3, BTN_ASSIGN_FIRE },
  { 6, BTN_ASSIGN_RUN_STOP_BACK },
  { 8, BTN_ASSIGN_MENU },
};

static const struct usb_gamepad_default_profile usb_gamepad_default_profiles[] = {
  {
    USB_GAMEPAD_DEFAULT_PROFILE_XBOX360,
    "Xbox 360-Compatible",
    USB_PREF_ANALOG,
    0,
    1,
    .50,
    .50,
    usb_gamepad_xbox360_button_defaults,
    sizeof(usb_gamepad_xbox360_button_defaults) /
      sizeof(usb_gamepad_xbox360_button_defaults[0]),
  },
  {
    USB_GAMEPAD_DEFAULT_PROFILE_8BITDO_M30_2_4G,
    "8bitdo M30 2.4g Controller",
    USB_PREF_ANALOG,
    3,
    4,
    .50,
    .50,
    usb_gamepad_8bitdo_m30_2_4g_button_defaults,
    sizeof(usb_gamepad_8bitdo_m30_2_4g_button_defaults) /
      sizeof(usb_gamepad_8bitdo_m30_2_4g_button_defaults[0]),
  },
};

void emu_set_usb_gamepad_mapping_profile(int device, unsigned profile) {
  if (device >= 0 && device < MAX_USB_DEVICES) {
    usb_gamepad_mapping_profiles[device] = profile;
  }
}

void emu_set_usb_gamepad_display_name(int device, const char *display_name) {
  if (device < 0 || device >= MAX_USB_DEVICES) {
    return;
  }

  if (display_name == 0) {
    usb_gamepad_display_names[device][0] = '\0';
    return;
  }

  size_t display_length = strlen(display_name);
  if (display_length > USB_GAMEPAD_DISPLAY_NAME_MAX) {
    display_length = USB_GAMEPAD_DISPLAY_NAME_MAX;
    while (display_length > 0 && display_name[display_length] != ' ') {
      display_length--;
    }
    if (display_length == 0) {
      display_length = USB_GAMEPAD_DISPLAY_NAME_MAX;
    }
  }

  memcpy(usb_gamepad_display_names[device], display_name, display_length);
  usb_gamepad_display_names[device][display_length] = '\0';
}

unsigned usb_gamepad_default_profile_for_vendor(const char *vendor) {
  if (vendor == 0) {
    return USB_GAMEPAD_DEFAULT_PROFILE_NONE;
  }

  for (unsigned i = 0;
       i < sizeof(usb_gamepad_vendor_default_profiles) /
               sizeof(usb_gamepad_vendor_default_profiles[0]);
       i++) {
    if (strcmp(vendor, usb_gamepad_vendor_default_profiles[i].vendor) == 0) {
      return usb_gamepad_vendor_default_profiles[i].profile;
    }
  }

  return USB_GAMEPAD_DEFAULT_PROFILE_NONE;
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
  if (device >= 0 && device < MAX_USB_DEVICES &&
      usb_gamepad_display_names[device][0] != '\0') {
    return usb_gamepad_display_names[device];
  }

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
