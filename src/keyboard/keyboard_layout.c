#include "keyboard_layout.h"

#if BMC64_NEW_KEYBOARD_INPUT
#include "../../third_party/common/emux_api.h"

static const KeyboardPreset c64_presets[] = {
   {KEYBOARD_PRESET_US_USB, "USB keyboard - US", "rpi_sym.vkm", KEYBOARD_LAYOUT_US},
   {KEYBOARD_PRESET_NORWEGIAN_USB, "USB keyboard - Norwegian", "rpi_sym_no.vkm", KEYBOARD_LAYOUT_NO},
   {KEYBOARD_PRESET_FRENCH_USB, "USB keyboard - French", "rpi_sym.vkm", KEYBOARD_LAYOUT_FR},
   {KEYBOARD_PRESET_C64_GPIO, "C64 keyboard - GPIO / C64P", "rpi_pos.vkm", KEYBOARD_LAYOUT_C64},
   {KEYBOARD_PRESET_C64_KEYRAH_V3, "C64 keyboard - Keyrah V3", "rpi_keyrah_v3_pos.vkm", KEYBOARD_LAYOUT_C64},
   {KEYBOARD_PRESET_C64_MAXI, "TheC64 Maxi keyboard", "rpi_maxi_pos.vkm", KEYBOARD_LAYOUT_MAXI},
   {KEYBOARD_PRESET_PETSCIIBOARD, "PETSCIIBOARD keyboard", "rpi_petsciiboard_sym.vkm", KEYBOARD_LAYOUT_US},
};

static const KeyboardPreset c128_presets[] = {
   {KEYBOARD_PRESET_US_USB, "USB keyboard - US", "rpi_sym.vkm", KEYBOARD_LAYOUT_US},
   {KEYBOARD_PRESET_C64_GPIO, "C64 keyboard - GPIO / C64P", "rpi_pos.vkm", KEYBOARD_LAYOUT_C64},
   {KEYBOARD_PRESET_C64_MAXI, "TheC64 Maxi keyboard", "rpi_maxi_pos.vkm", KEYBOARD_LAYOUT_MAXI},
};

static const KeyboardPreset vic20_presets[] = {
   {KEYBOARD_PRESET_US_USB, "USB keyboard - US", "rpi_sym.vkm", KEYBOARD_LAYOUT_US},
   {KEYBOARD_PRESET_C64_GPIO, "C64 keyboard - GPIO / C64P", "rpi_pos.vkm", KEYBOARD_LAYOUT_C64},
   {KEYBOARD_PRESET_C64_MAXI, "TheC64 Maxi keyboard", "rpi_maxi_pos.vkm", KEYBOARD_LAYOUT_MAXI},
};

static const KeyboardPreset plus4_presets[] = {
   {KEYBOARD_PRESET_US_USB, "USB keyboard - US", "rpi_sym.vkm", KEYBOARD_LAYOUT_US},
   {KEYBOARD_PRESET_C64_GPIO, "C64 keyboard - GPIO / C64P", "rpi_pos.vkm", KEYBOARD_LAYOUT_C64},
   {KEYBOARD_PRESET_C64_MAXI, "TheC64 Maxi keyboard", "rpi_maxi_pos.vkm", KEYBOARD_LAYOUT_MAXI},
};

static const KeyboardPreset plus4emu_presets[] = {
   {KEYBOARD_PRESET_US_USB, "USB keyboard - US", "rpi_pos.vkm", KEYBOARD_LAYOUT_US},
   {KEYBOARD_PRESET_C64_MAXI, "TheC64 Maxi keyboard", "rpi_maxi_pos.vkm", KEYBOARD_LAYOUT_MAXI},
   {KEYBOARD_PRESET_C16_KEYRAH, "C16 keyboard - Keyrah V2", "rpi_c16_keyrah_pos.vkm", KEYBOARD_LAYOUT_POSITIONAL},
};

static const KeyboardPreset pet_presets[] = {
   {KEYBOARD_PRESET_US_USB, "PET Graphics - US", "rpi_grus_sym.vkm", KEYBOARD_LAYOUT_US},
   {KEYBOARD_PRESET_PET_GRAPHICS_POS, "PET Graphics - positional", "rpi_grus_pos.vkm", KEYBOARD_LAYOUT_POSITIONAL},
   {KEYBOARD_PRESET_PET_BUSINESS_SYM, "PET Business - symbolic", "rpi_buus_sym.vkm", KEYBOARD_LAYOUT_US},
   {KEYBOARD_PRESET_PET_BUSINESS_POS, "PET Business - positional", "rpi_buus_pos.vkm", KEYBOARD_LAYOUT_POSITIONAL},
};

typedef struct {
   const KeyboardPreset *presets;
   int count;
} KeyboardPresetTable;

static const KeyboardPresetTable machine_tables[] = {
   [BMC64_MACHINE_CLASS_C64] = {c64_presets, sizeof(c64_presets) / sizeof(c64_presets[0])},
   [BMC64_MACHINE_CLASS_C128] = {c128_presets, sizeof(c128_presets) / sizeof(c128_presets[0])},
   [BMC64_MACHINE_CLASS_VIC20] = {vic20_presets, sizeof(vic20_presets) / sizeof(vic20_presets[0])},
   [BMC64_MACHINE_CLASS_PLUS4] = {plus4_presets, sizeof(plus4_presets) / sizeof(plus4_presets[0])},
   [BMC64_MACHINE_CLASS_PLUS4EMU] = {plus4emu_presets, sizeof(plus4emu_presets) / sizeof(plus4emu_presets[0])},
   [BMC64_MACHINE_CLASS_PET] = {pet_presets, sizeof(pet_presets) / sizeof(pet_presets[0])},
};

static const KeyboardPreset *machine_presets(int machine_class, int *count) {
   if (machine_class < 0 || machine_class >= (int)(sizeof(machine_tables) / sizeof(machine_tables[0]))) {
      *count = 0;
      return 0;
   }
   *count = machine_tables[machine_class].count;
   return machine_tables[machine_class].presets;
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

const KeyboardPreset *keyboard_preset_find(int machine_class, int preset) {
   int count;
   const KeyboardPreset *presets = machine_presets(machine_class, &count);
   for (int index = 0; index < count; index++) {
      if ((int)presets[index].preset == preset) return &presets[index];
   }
   return 0;
}

int keyboard_preset_index(int machine_class, int preset) {
   for (int index = 0; index < keyboard_preset_count(machine_class); index++) {
      if ((int)keyboard_preset_at(machine_class, index)->preset == preset) return index;
   }
   return -1;
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
   case KEYBOARD_LAYOUT_NO: return keyboard_layout_no_char(key, shifted, altgr);
   case KEYBOARD_LAYOUT_FR: return keyboard_layout_fr_char(key, shifted, altgr);
   case KEYBOARD_LAYOUT_POSITIONAL: return keyboard_layout_positional_char(key, shifted);
   case KEYBOARD_LAYOUT_MAXI: return keyboard_layout_maxi_char(key, shifted);
   default: return '\0';
   }
}

int keyboard_layout_has_altgr(MenuKeyboardLayout layout) {
   return layout == KEYBOARD_LAYOUT_NO || layout == KEYBOARD_LAYOUT_FR;
}

int keyboard_layout_effective_shift(int shifted, int caps_lock, int altgr) {
   return shifted || (caps_lock && !altgr);
}

#endif