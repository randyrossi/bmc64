#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circle.h"
#include "joy.h"
#include "kbd.h"
#include "keycodes.h"
#include "emux_api.h"
#include "ui.h"
#include "../../src/keyboard/keyboard_router.h"
#include "../../src/keyboard/keyboard_layout.h"

struct joydev_config joydevs[MAX_JOY_PORTS];
volatile int ui_enabled;
int ui_toggle_pending;
raw_keycode_func_t raw_keycode_func;
signed long commodore_key_sym = KEYCODE_LeftSuper;
signed long ctrl_key_sym = KEYCODE_LeftControl;

static int emulator_events;
static int menu_events;
static int quick_action;
static int captured_key;
static int joystick_events;
static long last_key;
static int last_pressed;
static int last_mod;
static unsigned long ticks;
static int safe_video_calls;
static int reboot_calls;

void assertion_failed(const char *expression, const char *file, unsigned line) {
  fprintf(stderr, "%s:%u: %s\n", file, line, expression);
  exit(1);
}

void emux_key_interrupt(long key, int pressed) {
  emulator_events++;
  last_key = key;
  last_pressed = pressed;
}

void emux_key_interrupt_mod(long key, int pressed, int mod) {
  emux_key_interrupt(key, pressed);
  last_mod = mod;
}

void emu_ui_key_interrupt(long key, int pressed) {
  menu_events++;
  last_key = key;
  last_pressed = pressed;
}

void emu_quick_func_interrupt(int action) {
  quick_action = action;
}

void circle_lock_acquire(void) {}
void circle_lock_release(void) {}
unsigned long circle_get_ticks(void) { return ticks; }
void switch_safe(void) { safe_video_calls++; }
void reboot(void) { reboot_calls++; }

int joy_key_down(unsigned int port, int key) {
  (void)port;
  if (key != KEYCODE_j) {
    return 0;
  }
  joystick_events++;
  return 1;
}

int joy_key_up(unsigned int port, int key) {
  return joy_key_down(port, key);
}

static void capture_key(long key) {
  captured_key = key;
}

int main(void) {
  static const char *machine_dirs[] = {
    NULL, "vic20", "c64", "c128", "plus4", "plus4emu", "pet"
  };
  static const int expected_counts[] = {0, 3, 8, 3, 3, 3, 2};
  for (int machine = BMC64_MACHINE_CLASS_VIC20; machine <= BMC64_MACHINE_CLASS_PET; machine++) {
    assert(keyboard_preset_count(machine) == expected_counts[machine]);
    for (int index = 0; index < keyboard_preset_count(machine); index++) {
      const KeyboardPreset *preset = keyboard_preset_at(machine, index);
      assert(keyboard_preset_find(machine, preset->preset) == preset);
      assert(keyboard_preset_index(machine, preset->preset) == index);
      char path[128];
      assert(keyboard_preset_file(preset, preset->default_mode) != NULL);
      for (int mode = KEYBOARD_MODE_SYMBOLIC; mode <= KEYBOARD_MODE_POSITIONAL; mode++) {
        const char *vkm_file = keyboard_preset_file(preset, mode);
        if (!vkm_file) continue;
        snprintf(path, sizeof(path), "sdcard/%s/%s", machine_dirs[machine], vkm_file);
        FILE *file = fopen(path, "r");
        assert(file != NULL);
        fclose(file);
      }
    }
    char directory[64];
    snprintf(directory, sizeof(directory), "sdcard/%s", machine_dirs[machine]);
    DIR *maps = opendir(directory);
    assert(maps != NULL);
    struct dirent *map;
    while ((map = readdir(maps)) != NULL) {
      size_t length = strlen(map->d_name);
      if (length < 4 || strncmp(map->d_name, "rpi_", 4) != 0 ||
          strcmp(map->d_name + length - 4, ".vkm") != 0) continue;
      int found = 0;
      for (int index = 0; index < keyboard_preset_count(machine); index++) {
        const KeyboardPreset *preset = keyboard_preset_at(machine, index);
        if ((preset->symbolic_vkm_file && strcmp(preset->symbolic_vkm_file, map->d_name) == 0) ||
          (preset->positional_vkm_file && strcmp(preset->positional_vkm_file, map->d_name) == 0)) found++;
      }
      assert(found > 0);
    }
    closedir(maps);
  }
  assert(keyboard_preset_count(BMC64_MACHINE_CLASS_C64) == 8);
  assert(keyboard_preset_count(BMC64_MACHINE_CLASS_PET) == 2);
  assert(keyboard_preset_index(BMC64_MACHINE_CLASS_PET, KEYBOARD_PRESET_PET_BUSINESS) == 1);
  assert(keyboard_preset_find(BMC64_MACHINE_CLASS_PET, KEYBOARD_PRESET_PET_BUSINESS)->default_mode == KEYBOARD_MODE_SYMBOLIC);
  assert(keyboard_preset_layout(keyboard_preset_at(BMC64_MACHINE_CLASS_PET, 0), KEYBOARD_MODE_POSITIONAL) == KEYBOARD_LAYOUT_POSITIONAL);
  assert(keyboard_preset_layout(keyboard_preset_at(BMC64_MACHINE_CLASS_PET, 1), KEYBOARD_MODE_SYMBOLIC) == KEYBOARD_LAYOUT_US);
  for (int index = 0; index < keyboard_preset_count(BMC64_MACHINE_CLASS_C64); index++) {
    const KeyboardPreset *preset = keyboard_preset_at(BMC64_MACHINE_CLASS_C64, index);
    assert(preset && (int)preset->preset == index + KEYBOARD_PRESET_US_USB);
    assert(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, preset->preset) == preset);
    assert(keyboard_preset_index(BMC64_MACHINE_CLASS_C64, preset->preset) == index);
  }
  assert(keyboard_preset_at(BMC64_MACHINE_CLASS_C64, -1) == NULL);
  assert(keyboard_preset_at(BMC64_MACHINE_CLASS_C64, 8) == NULL);
  assert(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, 0) == NULL);
  assert(keyboard_preset_index(BMC64_MACHINE_CLASS_C64, 0) == -1);
  assert(keyboard_preset_count(BMC64_MACHINE_CLASS_UNKNOWN) == 0);
  assert(keyboard_preset_at(BMC64_MACHINE_CLASS_UNKNOWN, 0) == NULL);
  for (int machine = BMC64_MACHINE_CLASS_VIC20; machine <= BMC64_MACHINE_CLASS_PET; machine++) {
    if (machine == BMC64_MACHINE_CLASS_C64) continue;
    const KeyboardPreset *preset = keyboard_preset_at(machine, 0);
    assert(preset && preset->preset == KEYBOARD_PRESET_US_USB);
    assert(preset->layout == KEYBOARD_LAYOUT_US);
    assert(keyboard_preset_find(machine, KEYBOARD_PRESET_US_USB) == preset);
    assert(keyboard_preset_index(machine, KEYBOARD_PRESET_US_USB) == 0);
    assert(keyboard_preset_at(machine, expected_counts[machine]) == NULL);
    assert(strcmp(keyboard_preset_file(preset, preset->default_mode), machine == BMC64_MACHINE_CLASS_PET
        ? "rpi_grus_sym.vkm" : machine == BMC64_MACHINE_CLASS_PLUS4EMU
        ? "rpi_pos.vkm" : "rpi_sym.vkm") == 0);
  }
  assert(strcmp(keyboard_preset_file(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, KEYBOARD_PRESET_US_USB), KEYBOARD_MODE_SYMBOLIC), "rpi_sym.vkm") == 0);
  assert(strcmp(keyboard_preset_file(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, KEYBOARD_PRESET_US_USB), KEYBOARD_MODE_POSITIONAL), "rpi_pos.vkm") == 0);
  assert(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, KEYBOARD_PRESET_US_USB)->default_mode == KEYBOARD_MODE_SYMBOLIC);
  assert(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, KEYBOARD_PRESET_NORWEGIAN_USB)->layout == KEYBOARD_LAYOUT_NO);
  assert(strcmp(keyboard_preset_file(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, KEYBOARD_PRESET_NORWEGIAN_USB), KEYBOARD_MODE_SYMBOLIC), "rpi_sym_no.vkm") == 0);
  assert(keyboard_preset_file(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, KEYBOARD_PRESET_NORWEGIAN_USB), KEYBOARD_MODE_POSITIONAL) == NULL);
  assert(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, KEYBOARD_PRESET_FRENCH_USB)->layout == KEYBOARD_LAYOUT_FR);
  assert(strcmp(keyboard_preset_file(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, KEYBOARD_PRESET_FRENCH_USB), KEYBOARD_MODE_SYMBOLIC), "rpi_sym_fr.vkm") == 0);
  assert(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, KEYBOARD_PRESET_GERMAN_USB)->layout == KEYBOARD_LAYOUT_DE);
  assert(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, KEYBOARD_PRESET_GERMAN_USB)->default_mode == KEYBOARD_MODE_SYMBOLIC);
  assert(strcmp(keyboard_preset_file(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, KEYBOARD_PRESET_GERMAN_USB), KEYBOARD_MODE_SYMBOLIC), "rpi_sym_de.vkm") == 0);
  assert(keyboard_preset_file(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, KEYBOARD_PRESET_GERMAN_USB), KEYBOARD_MODE_POSITIONAL) == NULL);
  assert(keyboard_preset_find(BMC64_MACHINE_CLASS_C128, KEYBOARD_PRESET_GERMAN_USB) == NULL);
  assert(strcmp(keyboard_preset_file(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, KEYBOARD_PRESET_C64_GPIO), KEYBOARD_MODE_POSITIONAL), "rpi_pos.vkm") == 0);
  assert(keyboard_preset_file(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, KEYBOARD_PRESET_C64_GPIO), KEYBOARD_MODE_SYMBOLIC) == NULL);
  assert(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, KEYBOARD_PRESET_C64_GPIO)->default_mode == KEYBOARD_MODE_POSITIONAL);
  assert(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, KEYBOARD_PRESET_C64_KEYRAH_V3)->layout == KEYBOARD_LAYOUT_C64);
  assert(strcmp(keyboard_preset_file(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, KEYBOARD_PRESET_C64_KEYRAH_V3), KEYBOARD_MODE_POSITIONAL), "rpi_keyrah_v3_pos.vkm") == 0);
  assert(strcmp(keyboard_preset_file(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, KEYBOARD_PRESET_C64_MAXI), KEYBOARD_MODE_POSITIONAL), "rpi_maxi_pos.vkm") == 0);
  assert(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, KEYBOARD_PRESET_C64_MAXI)->layout == KEYBOARD_LAYOUT_MAXI);
  assert(strcmp(keyboard_preset_file(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, KEYBOARD_PRESET_PETSCIIBOARD), KEYBOARD_MODE_SYMBOLIC), "rpi_petsciiboard_sym.vkm") == 0);
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_US, KEYCODE_2, 1, 0) == '@');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_NO, KEYCODE_2, 1, 0) == '"');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_NO, KEYCODE_4, 1, 0) == 0xA4);
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_NO, KEYCODE_LeftBracket, 0, 0) == 0xE5);
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_NO, KEYCODE_SemiColon, 1, 0) == 0xD8);
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_NO, KEYCODE_SingleQuote, 0, 0) == 0xE6);
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_NO, KEYCODE_RightBracket, 0, 0) == 0xA8);
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_NO, KEYCODE_NonUSBackSlash, 1, 0) == '>');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_FR, KEYCODE_2, 0, 0) == 0xE9);
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_DE, KEYCODE_y, 0, 0) == 'z');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_DE, KEYCODE_z, 0, 0) == 'y');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_DE, KEYCODE_7, 1, 0) == '/');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_DE, KEYCODE_Dash, 0, 0) == 0xDF);
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_DE, KEYCODE_LeftBracket, 0, 0) == 0xFC);
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_DE, KEYCODE_SemiColon, 0, 0) == 0xF6);
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_DE, KEYCODE_SingleQuote, 0, 0) == 0xE4);
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_DE, KEYCODE_RightBracket, 0, 1) == '~');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_DE, KEYCODE_Dash, 0, 1) == '\\');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_DE, KEYCODE_8, 0, 1) == '[');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_DE, KEYCODE_9, 0, 1) == ']');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_DE, KEYCODE_q, 0, 1) == '@');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_DE, KEYCODE_e, 0, 1) == 0x20AC);
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_DE, KEYCODE_NonUSBackSlash, 0, 1) == '|');
  assert(keyboard_layout_has_altgr(KEYBOARD_LAYOUT_DE));
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_FR, KEYCODE_2, 1, 0) == '2');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_FR, KEYCODE_7, 0, 0) == 0xE8);
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_FR, KEYCODE_9, 0, 0) == 0xE7);
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_FR, KEYCODE_q, 0, 0) == 'a');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_FR, KEYCODE_z, 1, 0) == 'W');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_FR, KEYCODE_m, 0, 0) == ',');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_FR, KEYCODE_SemiColon, 1, 0) == 'M');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_FR, KEYCODE_LeftBracket, 1, 0) == 0xA8);
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_NO, KEYCODE_2, 0, 1) == '@');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_NO, KEYCODE_3, 0, 1) == 0xA3);
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_NO, KEYCODE_4, 0, 1) == '$');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_NO, KEYCODE_4, 1, 1) == 0);
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_NO, KEYCODE_7, 0, 1) == '{');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_NO, KEYCODE_Dash, 0, 1) == '\\');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_FR, KEYCODE_0, 0, 1) == '@');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_FR, KEYCODE_4, 1, 1) == '$');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_FR, KEYCODE_8, 0, 1) == '\\');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_FR, KEYCODE_2, 0, 1) == '~');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_FR, KEYCODE_1, 0, 1) == 0);
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_US, KEYCODE_2, 0, 1) == '2');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_C64, KEYCODE_2, 1, 0) == '"');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_POSITIONAL, KEYCODE_Dash, 0, 0) == '+');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_MAXI, KEYCODE_KP_Add, 0, 0) == '+');
  assert(keyboard_layout_key_to_codepoint(0, KEYCODE_2, 1, 0) == 0);
  assert(keyboard_layout_has_altgr(KEYBOARD_LAYOUT_NO));
  assert(keyboard_layout_has_altgr(KEYBOARD_LAYOUT_FR));
  assert(!keyboard_layout_has_altgr(KEYBOARD_LAYOUT_US));

    assert(keyboard_layout_effective_shift(0, 0, 0) == 0);
    assert(keyboard_layout_effective_shift(1, 0, 0) == 1);
    assert(keyboard_layout_effective_shift(0, 1, 0) == 1);
    assert(keyboard_layout_effective_shift(1, 1, 0) == 1);
    assert(keyboard_layout_effective_shift(0, 1, 1) == 0);
    assert(keyboard_layout_effective_shift(1, 1, 1) == 1);
    assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_US, KEYCODE_2,
      keyboard_layout_effective_shift(0, 0, 0), 0) == '2');
    assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_US, KEYCODE_2,
      keyboard_layout_effective_shift(1, 0, 0), 0) == '@');
    assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_US, KEYCODE_2,
      keyboard_layout_effective_shift(0, 1, 0), 0) == '@');
    assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_NO, KEYCODE_2,
      keyboard_layout_effective_shift(0, 1, 0), 0) == '"');
    assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_FR, KEYCODE_2,
      keyboard_layout_effective_shift(0, 1, 0), 0) == '2');
    assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_NO, KEYCODE_4,
      keyboard_layout_effective_shift(0, 1, 1), 1) == '$');
    assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_FR, KEYCODE_q,
      keyboard_layout_effective_shift(0, 1, 0), 0) == 'A');

  emu_key_pressed(KEYCODE_a);
  emu_key_released(KEYCODE_a);
  assert(emulator_events == 2 && last_key == KEYCODE_a && !last_pressed);

  ui_enabled = 1;
  emu_key_pressed(KEYCODE_b);
  emu_key_released(KEYCODE_b);
  assert(menu_events == 2 && last_key == KEYCODE_b && !last_pressed);
  emu_key_pressed(KEYCODE_RightAlt);
  emu_key_released(KEYCODE_RightAlt);
  assert(menu_events == 4 && last_key == KEYCODE_RightAlt && !last_pressed);
  emu_key_pressed(KEYCODE_LeftShift);
  assert(menu_events == 5 && last_key == KEYCODE_LeftShift && last_pressed);
  emu_key_pressed(KEYCODE_2);
  emu_key_released(KEYCODE_2);
  emu_key_released(KEYCODE_LeftShift);
  assert(menu_events == 8 && last_key == KEYCODE_LeftShift && !last_pressed);
  ui_enabled = 0;

  emu_key_pressed(KEYCODE_F12);
  emu_key_released(KEYCODE_F12);
  assert(ui_toggle_pending == 2 && emulator_events == 4);
  ui_toggle_pending = 0;

  kbd_set_hotkey_function(0, KEYCODE_f, BTN_ASSIGN_WARP);
  emu_key_pressed(commodore_key_sym);
  emu_key_pressed(KEYCODE_f);
  emu_key_released(KEYCODE_f);
  emu_key_released(commodore_key_sym);
  assert(quick_action == BTN_ASSIGN_WARP && emulator_events == 6);

  kbd_set_hotkey_function(0, KEYCODE_m, BTN_ASSIGN_MENU);
  emu_key_pressed(commodore_key_sym);
  emu_key_pressed(KEYCODE_m);
  emu_key_released(KEYCODE_m);
  emu_key_released(commodore_key_sym);
  assert(ui_toggle_pending == 2 && emulator_events == 8);

  raw_keycode_func = capture_key;
  emu_key_pressed(KEYCODE_c);
  emu_key_released(KEYCODE_c);
  assert(captured_key == KEYCODE_c && emulator_events == 8);
  raw_keycode_func = 0;

  joydevs[0].device = JOYDEV_KEYSET1;
  emu_key_pressed(KEYCODE_j);
  emu_key_released(KEYCODE_j);
  assert(joystick_events == 2 && emulator_events == 8);

  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_d, KEYCODE_d, 1);
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_d, KEYCODE_d, 1);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 12, KEYCODE_e, 1);
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_d, KEYCODE_x, 0);
  assert(last_key == KEYCODE_d && !last_pressed);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 12, KEYCODE_y, 0);
  assert(last_key == KEYCODE_e && emulator_events == 12);

  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_t, KEYCODE_t, 1);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 15, KEYCODE_t, 1);
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_t, KEYCODE_t, 0);
  assert(emulator_events == 13);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 15, KEYCODE_t, 0);
  assert(emulator_events == 14 && last_key == KEYCODE_t && !last_pressed);

  kbd_set_hotkey_function(0, KEYCODE_n, BTN_ASSIGN_RESET_SOFT);
  emu_key_pressed(commodore_key_sym);
  emu_key_pressed(KEYCODE_n);
  emu_key_released(commodore_key_sym);
  assert(quick_action == BTN_ASSIGN_RESET_SOFT && emulator_events == 16);
  emu_key_released(KEYCODE_n);
  assert(emulator_events == 16);

  emu_key_pressed(commodore_key_sym);
  emu_key_pressed(KEYCODE_F7);
  ticks = 5000001UL;
  emu_key_released(KEYCODE_F7);
  emu_key_released(commodore_key_sym);
  assert(safe_video_calls == 1 && reboot_calls == 1);

  int emulator_before = emulator_events;
  int menu_before = menu_events;
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_v, KEYCODE_v, 1);
  ui_enabled = 1;
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_v, KEYCODE_v, 0);
  assert(emulator_events == emulator_before + 2 && menu_events == menu_before);

  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 22, KEYCODE_w, 1);
  ui_enabled = 0;
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 22, KEYCODE_w, 0);
  assert(menu_events == menu_before + 2 && emulator_events == emulator_before + 2);

  emulator_before = emulator_events;
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_a, KEYCODE_a, 1);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 9, KEYCODE_w, 1);
  assert(emulator_events == emulator_before + 2 && last_key == KEYCODE_w && last_pressed);
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_a, KEYCODE_a, 0);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 9, KEYCODE_w, 0);
  assert(emulator_events == emulator_before + 4 && last_key == KEYCODE_w && !last_pressed);

  ui_enabled = 1;
  menu_before = menu_events;
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_Return, KEYCODE_Return, 1);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 8, KEYCODE_3, 1);
  assert(menu_events == menu_before + 2 && last_key == KEYCODE_3 && last_pressed);
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_Return, KEYCODE_Return, 0);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 8, KEYCODE_3, 0);
  assert(menu_events == menu_before + 4 && last_key == KEYCODE_3 && !last_pressed);

  ui_enabled = 0;
  emulator_before = emulator_events;
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_LeftShift, KEYCODE_LeftShift, 1);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 15, KEYCODE_LeftShift, 1);
  assert(emulator_events == emulator_before + 1 && last_key == KEYCODE_LeftShift && last_pressed);
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_LeftShift, KEYCODE_LeftShift, 0);
  assert(emulator_events == emulator_before + 1);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 15, KEYCODE_LeftShift, 0);
  assert(emulator_events == emulator_before + 2 && last_key == KEYCODE_LeftShift && !last_pressed);

  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 64, KEYCODE_PageUp, 1);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 64, KEYCODE_PageUp, 0);
  assert(emulator_events == emulator_before + 4 && last_key == KEYCODE_PageUp && !last_pressed);

  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_RightAlt, KEYCODE_RightAlt, 1);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 65, KEYCODE_2, 1);
  assert(last_key == KEYCODE_2 && last_pressed && (last_mod & EMUX_KEY_MOD_RALT));
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 65, KEYCODE_2, 0);
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_RightAlt, KEYCODE_RightAlt, 0);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 65, KEYCODE_2, 1);
  assert(last_key == KEYCODE_2 && last_pressed && !(last_mod & EMUX_KEY_MOD_RALT));
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 65, KEYCODE_2, 0);

  return 0;
}