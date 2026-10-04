// Tests for src/profiles. Rule numbers ([P2], [F1], ...) refer to the
// behaviour reference in docs/PROFILES.md.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "profiles.h"

static int failures;

#define CHECK(expr)                                                     \
   do {                                                                 \
      if (!(expr)) {                                                    \
         fprintf(stderr, "%s:%d: FAILED: %s\n", __FILE__, __LINE__, #expr); \
         failures++;                                                    \
      }                                                                 \
   } while (0)

static void check_label(const char *machine, const char *expected) {
   char label[16];
   profiles_machine_label(machine, label, sizeof(label));
   if (strcmp(label, expected) != 0) {
      fprintf(stderr, "machine label for \"%s\": got \"%s\", expected \"%s\"\n",
              machine, label, expected);
      failures++;
   }
}

// With no profiles, Main runs ([P1]).
static void test_main_runs(void) {
   CHECK(profiles_running_is_main());
   CHECK(strcmp(profiles_running()->id, PROFILES_MAIN_ID) == 0);
   CHECK(strcmp(profiles_running()->name, "Main") == 0);
   CHECK(profiles_running()->machine[0] == '\0');
   CHECK(strcmp(profiles_autostart(), "") == 0);
}

// Main is always in the list, first ([P2]).
static void test_list_has_main(void) {
   int count = profiles_list_open();
   CHECK(count >= 1);
   CHECK(profiles_list_at(0) != NULL);
   CHECK(strcmp(profiles_list_at(0)->id, PROFILES_MAIN_ID) == 0);
   CHECK(profiles_list_at(count) == NULL);
   CHECK(profiles_list_at(-1) == NULL);
   profiles_list_close();
}

// The shared settings from docs/PROFILES.md, and some that belong to a profile.
static void test_shared_settings(void) {
   static const char *const shared[] = {
      "network_device", "timezone_offset_minutes", "network_modem_address",
      "webui_enabled", "webui_pin", "hotkey_cf1", "hotkey_cf3", "hotkey_cf5",
      "hotkey_cf7", "hotkey_tf1", "hotkey_tf3", "hotkey_tf5", "hotkey_tf7",
      "volume", "overlay", "overlay_padding", "vkbd_trans", "reset_confirm",
      "dir_convention", "drive_flush",
   };
   static const char *const per_profile[] = {
      "port_1", "palette", "gpio_config", "custom_gpio", "keyboard_layout",
      "keyboard_layout_mapping", "drive_type_8", "usb_0", "key_binding_1",
      "h_center_0", "tapereset", "s_scanlines", "",
   };
   for (unsigned i = 0; i < sizeof(shared) / sizeof(shared[0]); i++) {
      CHECK(profiles_is_shared_setting(shared[i]));
   }
   for (unsigned i = 0; i < sizeof(per_profile) / sizeof(per_profile[0]); i++) {
      CHECK(!profiles_is_shared_setting(per_profile[i]));
   }
   CHECK(!profiles_is_shared_setting(NULL));
}

static void test_machine_labels(void) {
   check_label("C64/PAL/HDMI/VICE 720p@50Hz", "C64");
   check_label("C128/NTSC/HDMI/VICE 720p@60Hz", "C128");
   check_label("VIC20/PAL/HDMI/VICE 720p@50Hz", "VIC-20");
   check_label("Plus4/PAL/HDMI/VICE 720p@50Hz", "Plus/4");
   check_label("Plus4Emu/PAL/HDMI/720p@50Hz", "Plus/4");
   check_label("Pet/NTSC/HDMI/VICE 720p@60Hz", "PET");
   check_label("C64", "C64");
   check_label("", "");
   check_label(NULL, "");

   // A label never overruns the buffer.
   char small[4];
   profiles_machine_label("VIC20/PAL", small, sizeof(small));
   CHECK(strcmp(small, "VIC") == 0);
}

int main(void) {
   test_main_runs();
   test_list_has_main();
   test_shared_settings();
   test_machine_labels();

   if (failures) {
      fprintf(stderr, "profiles tests: %d failure(s)\n", failures);
      return 1;
   }
   printf("profiles tests passed\n");
   return 0;
}
