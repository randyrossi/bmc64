#include "profiles_internal.h"

#include <string.h>

// Shared settings: the settings.txt keys kept in /profiles/system.txt for
// every profile instead of in each profile (docs/PROFILES.md, Shared
// settings). Wi-Fi (wpa_supplicant.conf) and logging (cmdline.txt) are
// already outside settings.txt, so they aren't listed.
//
// Not implemented yet: reading and writing system.txt.

static const char *const shared_keys[] = {
   "network_device",
   "timezone_offset_minutes",
   "network_modem_address",
   "webui_enabled",
   "webui_pin",
   "hotkey_cf1",
   "hotkey_cf3",
   "hotkey_cf5",
   "hotkey_cf7",
   "hotkey_tf1",
   "hotkey_tf3",
   "hotkey_tf5",
   "hotkey_tf7",
   "volume",
   "overlay",
   "overlay_padding",
   "vkbd_trans",
   "reset_confirm",
   "dir_convention",
   "drive_flush",
};

int profiles_is_shared_setting(const char *key) {
   if (key == 0) return 0;
   for (unsigned i = 0; i < sizeof(shared_keys) / sizeof(shared_keys[0]); i++) {
      if (strcmp(key, shared_keys[i]) == 0) return 1;
   }
   return 0;
}
