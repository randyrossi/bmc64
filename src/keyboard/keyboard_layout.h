#ifndef BMC64_KEYBOARD_LAYOUT_H
#define BMC64_KEYBOARD_LAYOUT_H

#include "keyboard_feature.h"

#if BMC64_NEW_KEYBOARD_INPUT

typedef enum {
   KEYBOARD_LAYOUT_C64 = 1,
   KEYBOARD_LAYOUT_US,
   KEYBOARD_LAYOUT_NO,
   KEYBOARD_LAYOUT_FR,
   KEYBOARD_LAYOUT_POSITIONAL,
   KEYBOARD_LAYOUT_MAXI,
} MenuKeyboardLayout;

typedef enum {
   KEYBOARD_PRESET_US_USB = 1,
   KEYBOARD_PRESET_NORWEGIAN_USB,
   KEYBOARD_PRESET_FRENCH_USB,
   KEYBOARD_PRESET_C64_GPIO,
   KEYBOARD_PRESET_C64_KEYRAH_V3,
   KEYBOARD_PRESET_C64_MAXI,
   KEYBOARD_PRESET_PETSCIIBOARD,
   KEYBOARD_PRESET_C16_KEYRAH,
   KEYBOARD_PRESET_PET_GRAPHICS_POS,
   KEYBOARD_PRESET_PET_BUSINESS_SYM,
   KEYBOARD_PRESET_PET_BUSINESS_POS,
} MenuKeyboardPreset;

typedef struct {
   MenuKeyboardPreset preset;
   const char *label;
   const char *vkm_file;
   MenuKeyboardLayout layout;
} KeyboardPreset;

typedef struct {
   long key;
   unsigned int normal;
   unsigned int shifted;
   unsigned int altgr;
   unsigned int shifted_altgr;
} KeyboardLayoutKey;

#ifdef __cplusplus
extern "C" {
#endif

int keyboard_preset_count(int machine_class);
const KeyboardPreset *keyboard_preset_at(int machine_class, int index);
const KeyboardPreset *keyboard_preset_find(int machine_class, int preset);
int keyboard_preset_index(int machine_class, int preset);
unsigned int keyboard_layout_key_to_codepoint(MenuKeyboardLayout layout, long key, int shifted, int altgr);
int keyboard_layout_has_altgr(MenuKeyboardLayout layout);
int keyboard_layout_effective_shift(int shifted, int caps_lock, int altgr);
unsigned int keyboard_layout_lookup(const KeyboardLayoutKey *keys, unsigned count,
                                    long key, int shifted, int altgr);
char keyboard_layout_us_char(long key, int shifted);
unsigned int keyboard_layout_no_char(long key, int shifted, int altgr);
unsigned int keyboard_layout_fr_char(long key, int shifted, int altgr);
char keyboard_layout_c64_char(long key, int shifted);
char keyboard_layout_positional_char(long key, int shifted);
char keyboard_layout_maxi_char(long key, int shifted);

#ifdef __cplusplus
}
#endif

#endif
#endif