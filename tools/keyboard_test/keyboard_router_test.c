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
static int safe_mode_requests;

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
void emu_safe_mode_interrupt(void) { safe_mode_requests++; }
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
  static const int expected_counts[] = {0, 3, 14, 3, 3, 3, 2};
  static const int expected_layouts[] = {0, 2, 7, 2, 2, 1, 1};
  for (int machine = BMC64_MACHINE_CLASS_VIC20; machine <= BMC64_MACHINE_CLASS_PET; machine++) {
    assert(keyboard_preset_count(machine) == expected_counts[machine]);
    assert(keyboard_preset_at(machine, expected_counts[machine]) == NULL);
    assert(keyboard_preset_layout_count(machine) == expected_layouts[machine]);
    assert(keyboard_preset_layout_at(machine, expected_layouts[machine]) == NULL);
    assert(keyboard_preset_default(machine) != NULL);
    for (int index = 0; index < keyboard_preset_count(machine); index++) {
      const KeyboardPreset *preset = keyboard_preset_at(machine, index);
      // Each (layout, mapping) pair is unique and has labels that fit a menu choice.
      assert(keyboard_preset_find(machine, preset->layout, preset->mapping) == preset);
      assert(strlen(preset->layout_label) < 36 && strlen(preset->mapping_label) < 36);
      assert(keyboard_preset_layout_index(machine, preset->layout) >= 0);
      if (preset->vkm_file == NULL) continue; // the emulator picks the file
      char path[128];
      snprintf(path, sizeof(path), "sdcard/%s/%s", machine_dirs[machine], preset->vkm_file);
      FILE *file = fopen(path, "r");
      assert(file != NULL);
      fclose(file);
    }
    // Walking layouts then mappings visits every preset exactly once.
    int visited = 0;
    for (int layout_index = 0; layout_index < keyboard_preset_layout_count(machine); layout_index++) {
      const KeyboardPreset *layout = keyboard_preset_layout_at(machine, layout_index);
      assert(keyboard_preset_layout_index(machine, layout->layout) == layout_index);
      assert(keyboard_preset_mapping_at(machine, layout->layout, 0) == layout);
      int mappings = keyboard_preset_mapping_count(machine, layout->layout);
      assert(mappings > 0);
      assert(keyboard_preset_mapping_at(machine, layout->layout, mappings) == NULL);
      for (int index = 0; index < mappings; index++) {
        const KeyboardPreset *preset = keyboard_preset_mapping_at(machine, layout->layout, index);
        assert(preset->layout == layout->layout);
        assert(strcmp(preset->layout_label, layout->layout_label) == 0);
        visited++;
      }
    }
    assert(visited == keyboard_preset_count(machine));
    char directory[64];
    snprintf(directory, sizeof(directory), "sdcard/%s", machine_dirs[machine]);
    DIR *maps = opendir(directory);
    assert(maps != NULL);
    struct dirent *map;
    while ((map = readdir(maps)) != NULL) {
      size_t length = strlen(map->d_name);
      if (length < 4 || strncmp(map->d_name, "rpi_", 4) != 0 ||
          strcmp(map->d_name + length - 4, ".vkm") != 0) continue;
      // Used when a row names it, or when a row without a file lets the
      // emulator pick rpi_<keyboard type>_sym/pos.vkm for that mapping.
      int found = 0;
      for (int index = 0; index < keyboard_preset_count(machine); index++) {
        const KeyboardPreset *preset = keyboard_preset_at(machine, index);
        if (preset->vkm_file != NULL) {
          if (strcmp(preset->vkm_file, map->d_name) == 0) found++;
        } else if (length > 8 && strcmp(map->d_name + length - 8,
                   preset->mapping == KEYBOARD_MAP_POSITIONAL ? "_pos.vkm" : "_sym.vkm") == 0) {
          found++;
        }
      }
      assert(found > 0);
    }
    closedir(maps);
  }
  assert(keyboard_preset_count(BMC64_MACHINE_CLASS_UNKNOWN) == 0);
  assert(keyboard_preset_at(BMC64_MACHINE_CLASS_UNKNOWN, 0) == NULL);
  assert(keyboard_preset_default(BMC64_MACHINE_CLASS_UNKNOWN) == NULL);
  assert(keyboard_preset_layout_count(BMC64_MACHINE_CLASS_UNKNOWN) == 0);
  assert(keyboard_preset_at(BMC64_MACHINE_CLASS_C64, -1) == NULL);
  assert(keyboard_preset_find(BMC64_MACHINE_CLASS_C64, 0, 0) == NULL);
  assert(keyboard_preset_layout_at(BMC64_MACHINE_CLASS_C64, -1) == NULL);
  assert(keyboard_preset_layout_index(BMC64_MACHINE_CLASS_C64, KEYBOARD_PHYSICAL_PET_BUSINESS) == -1);
  assert(keyboard_preset_mapping_count(BMC64_MACHINE_CLASS_C64, KEYBOARD_PHYSICAL_PET_BUSINESS) == 0);
  assert(keyboard_preset_mapping_at(BMC64_MACHINE_CLASS_C64, KEYBOARD_PHYSICAL_US, -1) == NULL);

  // C64: the Commodore layout has three mappings; each locale has Symbolic
  // (default) and Positional, which keeps the locale's menu text table.
  const int c64 = BMC64_MACHINE_CLASS_C64;
  assert(strcmp(keyboard_preset_layout_at(c64, 0)->layout_label, "Commodore") == 0);
  assert(keyboard_preset_mapping_count(c64, KEYBOARD_PHYSICAL_MACHINE) == 3);
  assert(strcmp(keyboard_preset_mapping_at(c64, KEYBOARD_PHYSICAL_MACHINE, 0)->vkm_file, "rpi_pos.vkm") == 0);
  assert(strcmp(keyboard_preset_find(c64, KEYBOARD_PHYSICAL_MACHINE, KEYBOARD_MAP_MAXI)->vkm_file, "rpi_maxi_pos.vkm") == 0);
  assert(keyboard_preset_find(c64, KEYBOARD_PHYSICAL_MACHINE, KEYBOARD_MAP_MAXI)->text_layout == KEYBOARD_LAYOUT_MAXI);
  assert(strcmp(keyboard_preset_find(c64, KEYBOARD_PHYSICAL_MACHINE, KEYBOARD_MAP_KEYRAH)->vkm_file, "rpi_keyrah_v3_pos.vkm") == 0);
  assert(keyboard_preset_find(c64, KEYBOARD_PHYSICAL_MACHINE, KEYBOARD_MAP_KEYRAH)->text_layout == KEYBOARD_LAYOUT_C64);
  assert(keyboard_preset_find(c64, KEYBOARD_PHYSICAL_MACHINE, KEYBOARD_MAP_SYMBOLIC) == NULL);
  static const struct { int layout; const char *file; MenuKeyboardLayout text; } c64_locales[] = {
    {KEYBOARD_PHYSICAL_US, "rpi_sym.vkm", KEYBOARD_LAYOUT_US},
    {KEYBOARD_PHYSICAL_UK, "rpi_sym_uk.vkm", KEYBOARD_LAYOUT_UK},
    {KEYBOARD_PHYSICAL_DE, "rpi_sym_de.vkm", KEYBOARD_LAYOUT_DE},
    {KEYBOARD_PHYSICAL_FR, "rpi_sym_fr.vkm", KEYBOARD_LAYOUT_FR},
    {KEYBOARD_PHYSICAL_NO, "rpi_sym_no.vkm", KEYBOARD_LAYOUT_NO},
  };
  for (unsigned index = 0; index < sizeof(c64_locales) / sizeof(c64_locales[0]); index++) {
    int layout = c64_locales[index].layout;
    assert(keyboard_preset_mapping_count(c64, layout) == 2);
    const KeyboardPreset *symbolic = keyboard_preset_find(c64, layout, KEYBOARD_MAP_SYMBOLIC);
    assert(symbolic && keyboard_preset_mapping_at(c64, layout, 0) == symbolic);
    assert(strcmp(symbolic->vkm_file, c64_locales[index].file) == 0);
    assert(symbolic->text_layout == c64_locales[index].text);
    const KeyboardPreset *positional = keyboard_preset_find(c64, layout, KEYBOARD_MAP_POSITIONAL);
    assert(positional && strcmp(positional->vkm_file, "rpi_pos.vkm") == 0);
    assert(positional->text_layout == c64_locales[index].text);
  }
  const KeyboardPreset *petsciiboard = keyboard_preset_find(c64, KEYBOARD_PHYSICAL_PETSCIIBOARD, KEYBOARD_MAP_SYMBOLIC);
  assert(petsciiboard && keyboard_preset_mapping_count(c64, KEYBOARD_PHYSICAL_PETSCIIBOARD) == 1);
  assert(strcmp(petsciiboard->vkm_file, "rpi_petsciiboard_sym.vkm") == 0);
  assert(keyboard_preset_find(BMC64_MACHINE_CLASS_C128, KEYBOARD_PHYSICAL_DE, KEYBOARD_MAP_SYMBOLIC) == NULL);

  // Defaults: US Symbolic where it exists, otherwise the first row.
  for (int machine = BMC64_MACHINE_CLASS_VIC20; machine <= BMC64_MACHINE_CLASS_PLUS4; machine++) {
    const KeyboardPreset *preset = keyboard_preset_default(machine);
    assert(preset->layout == KEYBOARD_PHYSICAL_US && preset->mapping == KEYBOARD_MAP_SYMBOLIC);
    assert(strcmp(preset->vkm_file, "rpi_sym.vkm") == 0);
  }
  assert(strcmp(keyboard_preset_default(BMC64_MACHINE_CLASS_PLUS4EMU)->vkm_file, "rpi_pos.vkm") == 0);
  assert(keyboard_preset_default(BMC64_MACHINE_CLASS_PET)->mapping == KEYBOARD_MAP_SYMBOLIC);

  // PET: one layout, hidden in the menu so only Keyboard Mapping shows; Symbolic
  // (default) or Positional. The PET model picks the Graphics or Business
  // keymap, so no row names a file, and menu text entry is always US.
  const int pet = BMC64_MACHINE_CLASS_PET;
  assert(keyboard_preset_layout_hidden(pet));
  for (int machine = BMC64_MACHINE_CLASS_VIC20; machine < BMC64_MACHINE_CLASS_PET; machine++) {
    assert(!keyboard_preset_layout_hidden(machine));
  }
  assert(!keyboard_preset_layout_hidden(BMC64_MACHINE_CLASS_UNKNOWN));
  assert(keyboard_preset_mapping_count(pet, KEYBOARD_PHYSICAL_MACHINE) == 2);
  assert(keyboard_preset_mapping_at(pet, KEYBOARD_PHYSICAL_MACHINE, 0)->mapping == KEYBOARD_MAP_SYMBOLIC);
  assert(keyboard_preset_mapping_at(pet, KEYBOARD_PHYSICAL_MACHINE, 1)->mapping == KEYBOARD_MAP_POSITIONAL);
  for (int index = 0; index < keyboard_preset_count(pet); index++) {
    assert(keyboard_preset_at(pet, index)->vkm_file == NULL);
    assert(keyboard_preset_at(pet, index)->text_layout == KEYBOARD_LAYOUT_US);
  }
  static const char *pet_maps[] = {"rpi_grus_sym.vkm", "rpi_grus_pos.vkm", "rpi_buus_sym.vkm", "rpi_buus_pos.vkm"};
  for (unsigned index = 0; index < sizeof(pet_maps) / sizeof(pet_maps[0]); index++) {
    char path[64];
    snprintf(path, sizeof(path), "sdcard/pet/%s", pet_maps[index]);
    FILE *file = fopen(path, "r");
    assert(file != NULL);
    fclose(file);
  }
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_US, KEYCODE_2, 1, 0) == '@');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_UK, KEYCODE_2, 1, 0) == '"');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_UK, KEYCODE_3, 1, 0) == 0xA3);
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_UK, KEYCODE_SingleQuote, 0, 0) == '\'');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_UK, KEYCODE_SingleQuote, 1, 0) == '@');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_UK, KEYCODE_BackQuote, 1, 0) == 0xAC);
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_UK, KEYCODE_Pound, 0, 0) == '#');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_UK, KEYCODE_BackSlash, 1, 0) == '~');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_UK, KEYCODE_NonUSBackSlash, 0, 0) == '\\');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_UK, KEYCODE_NonUSBackSlash, 1, 0) == '|');
  assert(keyboard_layout_key_to_codepoint(KEYBOARD_LAYOUT_UK, KEYCODE_LeftBracket, 1, 0) == '{');
  assert(!keyboard_layout_has_altgr(KEYBOARD_LAYOUT_UK));
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
  // The key handler only asks; the main loop does the switch and reboot.
  assert(safe_mode_requests == 1 && safe_video_calls == 0 && reboot_calls == 0);

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