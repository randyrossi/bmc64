#include "keyboard_layout.h"

#if BMC64_NEW_KEYBOARD_INPUT
#include "../../third_party/common/emux_api.h"

// One row per choice. Rows with the same layout are the "Keyboard Mapping"
// choices for that "Keyboard Layout"; the first one is the layout's default.
static const KeyboardPreset c64_presets[] = {
   {KEYBOARD_PHYSICAL_MACHINE, "Commodore", KEYBOARD_MAP_POSITIONAL, "Positional", "rpi_pos.vkm", KEYBOARD_LAYOUT_C64},
   {KEYBOARD_PHYSICAL_MACHINE, "Commodore", KEYBOARD_MAP_MAXI, "TheC64 Maxi", "rpi_maxi_pos.vkm", KEYBOARD_LAYOUT_MAXI},
   {KEYBOARD_PHYSICAL_MACHINE, "Commodore", KEYBOARD_MAP_KEYRAH, "Keyrah V3", "rpi_keyrah_v3_pos.vkm", KEYBOARD_LAYOUT_C64},
   {KEYBOARD_PHYSICAL_US, "US English", KEYBOARD_MAP_SYMBOLIC, "Symbolic", "rpi_sym.vkm", KEYBOARD_LAYOUT_US},
   {KEYBOARD_PHYSICAL_US, "US English", KEYBOARD_MAP_POSITIONAL, "Positional", "rpi_pos.vkm", KEYBOARD_LAYOUT_US},
   {KEYBOARD_PHYSICAL_UK, "UK English", KEYBOARD_MAP_SYMBOLIC, "Symbolic", "rpi_sym_uk.vkm", KEYBOARD_LAYOUT_UK},
   {KEYBOARD_PHYSICAL_UK, "UK English", KEYBOARD_MAP_POSITIONAL, "Positional", "rpi_pos.vkm", KEYBOARD_LAYOUT_UK},
   {KEYBOARD_PHYSICAL_DE, "German", KEYBOARD_MAP_SYMBOLIC, "Symbolic", "rpi_sym_de.vkm", KEYBOARD_LAYOUT_DE},
   {KEYBOARD_PHYSICAL_DE, "German", KEYBOARD_MAP_POSITIONAL, "Positional", "rpi_pos.vkm", KEYBOARD_LAYOUT_DE},
   {KEYBOARD_PHYSICAL_FR, "French", KEYBOARD_MAP_SYMBOLIC, "Symbolic", "rpi_sym_fr.vkm", KEYBOARD_LAYOUT_FR},
   {KEYBOARD_PHYSICAL_FR, "French", KEYBOARD_MAP_POSITIONAL, "Positional", "rpi_pos.vkm", KEYBOARD_LAYOUT_FR},
   {KEYBOARD_PHYSICAL_NO, "Norwegian", KEYBOARD_MAP_SYMBOLIC, "Symbolic", "rpi_sym_no.vkm", KEYBOARD_LAYOUT_NO},
   {KEYBOARD_PHYSICAL_NO, "Norwegian", KEYBOARD_MAP_POSITIONAL, "Positional", "rpi_pos.vkm", KEYBOARD_LAYOUT_NO},
   {KEYBOARD_PHYSICAL_PETSCIIBOARD, "PETSCIIBOARD", KEYBOARD_MAP_SYMBOLIC, "Symbolic", "rpi_petsciiboard_sym.vkm", KEYBOARD_LAYOUT_US},
};

static const KeyboardPreset c128_presets[] = {
   {KEYBOARD_PHYSICAL_MACHINE, "Commodore", KEYBOARD_MAP_POSITIONAL, "Positional", "rpi_pos.vkm", KEYBOARD_LAYOUT_C64},
   {KEYBOARD_PHYSICAL_MACHINE, "Commodore", KEYBOARD_MAP_MAXI, "TheC64 Maxi", "rpi_maxi_pos.vkm", KEYBOARD_LAYOUT_MAXI},
   {KEYBOARD_PHYSICAL_US, "US English", KEYBOARD_MAP_SYMBOLIC, "Symbolic", "rpi_sym.vkm", KEYBOARD_LAYOUT_US},
};

static const KeyboardPreset vic20_presets[] = {
   {KEYBOARD_PHYSICAL_MACHINE, "Commodore", KEYBOARD_MAP_POSITIONAL, "Positional", "rpi_pos.vkm", KEYBOARD_LAYOUT_C64},
   {KEYBOARD_PHYSICAL_MACHINE, "Commodore", KEYBOARD_MAP_MAXI, "TheC64 Maxi", "rpi_maxi_pos.vkm", KEYBOARD_LAYOUT_MAXI},
   {KEYBOARD_PHYSICAL_US, "US English", KEYBOARD_MAP_SYMBOLIC, "Symbolic", "rpi_sym.vkm", KEYBOARD_LAYOUT_US},
};

static const KeyboardPreset plus4_presets[] = {
   {KEYBOARD_PHYSICAL_MACHINE, "Commodore", KEYBOARD_MAP_POSITIONAL, "Positional", "rpi_pos.vkm", KEYBOARD_LAYOUT_C64},
   {KEYBOARD_PHYSICAL_MACHINE, "Commodore", KEYBOARD_MAP_MAXI, "TheC64 Maxi", "rpi_maxi_pos.vkm", KEYBOARD_LAYOUT_MAXI},
   {KEYBOARD_PHYSICAL_US, "US English", KEYBOARD_MAP_SYMBOLIC, "Symbolic", "rpi_sym.vkm", KEYBOARD_LAYOUT_US},
};

static const KeyboardPreset plus4emu_presets[] = {
   {KEYBOARD_PHYSICAL_MACHINE, "Commodore", KEYBOARD_MAP_POSITIONAL, "Positional", "rpi_pos.vkm", KEYBOARD_LAYOUT_US},
   {KEYBOARD_PHYSICAL_MACHINE, "Commodore", KEYBOARD_MAP_MAXI, "TheC64 Maxi", "rpi_maxi_pos.vkm", KEYBOARD_LAYOUT_MAXI},
   {KEYBOARD_PHYSICAL_MACHINE, "Commodore", KEYBOARD_MAP_KEYRAH, "Keyrah V2 (C16)", "rpi_c16_keyrah_pos.vkm", KEYBOARD_LAYOUT_POSITIONAL},
};

// The PET's keyboard (Graphics or Business) follows the PET model, so the
// emulator picks rpi_grus_* or rpi_buus_* itself. The menu hides Keyboard
// Layout (see machine_tables), and menu text entry is always US.
static const KeyboardPreset pet_presets[] = {
   {KEYBOARD_PHYSICAL_MACHINE, "Commodore", KEYBOARD_MAP_SYMBOLIC, "Symbolic", NULL, KEYBOARD_LAYOUT_US},
   {KEYBOARD_PHYSICAL_MACHINE, "Commodore", KEYBOARD_MAP_POSITIONAL, "Positional", NULL, KEYBOARD_LAYOUT_US},
};

typedef struct {
   const KeyboardPreset *presets;
   int count;
   int hide_layout; // show only "Keyboard Mapping" in the menu
} KeyboardPresetTable;

static const KeyboardPresetTable machine_tables[] = {
   [BMC64_MACHINE_CLASS_C64] = {c64_presets, sizeof(c64_presets) / sizeof(c64_presets[0])},
   [BMC64_MACHINE_CLASS_C128] = {c128_presets, sizeof(c128_presets) / sizeof(c128_presets[0])},
   [BMC64_MACHINE_CLASS_VIC20] = {vic20_presets, sizeof(vic20_presets) / sizeof(vic20_presets[0])},
   [BMC64_MACHINE_CLASS_PLUS4] = {plus4_presets, sizeof(plus4_presets) / sizeof(plus4_presets[0])},
   [BMC64_MACHINE_CLASS_PLUS4EMU] = {plus4emu_presets, sizeof(plus4emu_presets) / sizeof(plus4emu_presets[0])},
   [BMC64_MACHINE_CLASS_PET] = {pet_presets, sizeof(pet_presets) / sizeof(pet_presets[0]), 1},
};

static const KeyboardPreset *machine_presets(int machine_class, int *count) {
   if (machine_class < 0 || machine_class >= (int)(sizeof(machine_tables) / sizeof(machine_tables[0]))) {
      *count = 0;
      return 0;
   }
   *count = machine_tables[machine_class].count;
   return machine_tables[machine_class].presets;
}

int keyboard_preset_layout_hidden(int machine_class) {
   int count;
   return machine_presets(machine_class, &count) != 0 && machine_tables[machine_class].hide_layout;
}

int keyboard_preset_count(int machine_class) {
   int count;
   machine_presets(machine_class, &count);
   return count;
}

const KeyboardPreset *keyboard_preset_at(int machine_class, int index) {
   int count;
   const KeyboardPreset *presets = machine_presets(machine_class, &count);
   return index >= 0 && index < count ? &presets[index] : 0;
}

const KeyboardPreset *keyboard_preset_find(int machine_class, int layout, int mapping) {
   int count;
   const KeyboardPreset *presets = machine_presets(machine_class, &count);
   for (int index = 0; index < count; index++) {
      if ((int)presets[index].layout == layout && (int)presets[index].mapping == mapping) return &presets[index];
   }
   return 0;
}

// US Symbolic where the machine has it, otherwise the first row.
const KeyboardPreset *keyboard_preset_default(int machine_class) {
   const KeyboardPreset *preset = keyboard_preset_find(machine_class, KEYBOARD_PHYSICAL_US, KEYBOARD_MAP_SYMBOLIC);
   return preset ? preset : keyboard_preset_at(machine_class, 0);
}

// True for the first row of each layout.
static int starts_layout(const KeyboardPreset *presets, int index) {
   for (int earlier = 0; earlier < index; earlier++) {
      if (presets[earlier].layout == presets[index].layout) return 0;
   }
   return 1;
}

int keyboard_preset_layout_count(int machine_class) {
   int count, layouts = 0;
   const KeyboardPreset *presets = machine_presets(machine_class, &count);
   for (int index = 0; index < count; index++) {
      if (starts_layout(presets, index)) layouts++;
   }
   return layouts;
}

const KeyboardPreset *keyboard_preset_layout_at(int machine_class, int index) {
   int count;
   const KeyboardPreset *presets = machine_presets(machine_class, &count);
   for (int row = 0; index >= 0 && row < count; row++) {
      if (starts_layout(presets, row) && index-- == 0) return &presets[row];
   }
   return 0;
}

int keyboard_preset_layout_index(int machine_class, int layout) {
   for (int index = 0; index < keyboard_preset_layout_count(machine_class); index++) {
      if ((int)keyboard_preset_layout_at(machine_class, index)->layout == layout) return index;
   }
   return -1;
}

int keyboard_preset_mapping_count(int machine_class, int layout) {
   int count, mappings = 0;
   const KeyboardPreset *presets = machine_presets(machine_class, &count);
   for (int index = 0; index < count; index++) {
      if ((int)presets[index].layout == layout) mappings++;
   }
   return mappings;
}

const KeyboardPreset *keyboard_preset_mapping_at(int machine_class, int layout, int index) {
   int count;
   const KeyboardPreset *presets = machine_presets(machine_class, &count);
   for (int row = 0; index >= 0 && row < count; row++) {
      if ((int)presets[row].layout == layout && index-- == 0) return &presets[row];
   }
   return 0;
}

unsigned int keyboard_layout_lookup(const KeyboardLayoutKey *keys, unsigned count,
                                    long key, int shifted, int altgr) {
   for (unsigned index = 0; index < count; index++) {
      if (keys[index].key == key) {
         if (altgr) return shifted ? keys[index].shifted_altgr : keys[index].altgr;
         return shifted ? keys[index].shifted : keys[index].normal;
      }
   }
   return '\0';
}

unsigned int keyboard_layout_key_to_codepoint(MenuKeyboardLayout layout, long key, int shifted, int altgr) {
   switch (layout) {
   case KEYBOARD_LAYOUT_C64: return keyboard_layout_c64_char(key, shifted);
   case KEYBOARD_LAYOUT_US: return keyboard_layout_us_char(key, shifted);
   case KEYBOARD_LAYOUT_UK: return keyboard_layout_uk_char(key, shifted);
   case KEYBOARD_LAYOUT_NO: return keyboard_layout_no_char(key, shifted, altgr);
   case KEYBOARD_LAYOUT_FR: return keyboard_layout_fr_char(key, shifted, altgr);
   case KEYBOARD_LAYOUT_DE: return keyboard_layout_de_char(key, shifted, altgr);
   case KEYBOARD_LAYOUT_POSITIONAL: return keyboard_layout_positional_char(key, shifted);
   case KEYBOARD_LAYOUT_MAXI: return keyboard_layout_maxi_char(key, shifted);
   default: return '\0';
   }
}

int keyboard_layout_has_altgr(MenuKeyboardLayout layout) {
   return layout == KEYBOARD_LAYOUT_NO || layout == KEYBOARD_LAYOUT_FR || layout == KEYBOARD_LAYOUT_DE;
}

int keyboard_layout_effective_shift(int shifted, int caps_lock, int altgr) {
   return shifted || (caps_lock && !altgr);
}

#endif