#ifndef BMC64_KEYBOARD_LAYOUT_H
#define BMC64_KEYBOARD_LAYOUT_H

#include "keyboard_feature.h"

#if BMC64_NEW_KEYBOARD_INPUT

typedef enum {
   KEYBOARD_LAYOUT_C64 = 1,
   KEYBOARD_LAYOUT_US,
   KEYBOARD_LAYOUT_NO,
   KEYBOARD_LAYOUT_FR,
   KEYBOARD_LAYOUT_DE,
   KEYBOARD_LAYOUT_POSITIONAL,
   KEYBOARD_LAYOUT_MAXI,
   KEYBOARD_LAYOUT_UK,
} MenuKeyboardLayout;

// The physical keyboard the user has. Saved in settings, so only append.
typedef enum {
   KEYBOARD_PHYSICAL_MACHINE = 1, // laid out like the emulated machine's own keyboard
   KEYBOARD_PHYSICAL_US,
   KEYBOARD_PHYSICAL_NO,
   KEYBOARD_PHYSICAL_FR,
   KEYBOARD_PHYSICAL_DE,
   KEYBOARD_PHYSICAL_PETSCIIBOARD,
   KEYBOARD_PHYSICAL_PET_GRAPHICS,
   KEYBOARD_PHYSICAL_PET_BUSINESS,
   KEYBOARD_PHYSICAL_UK,
} KeyboardPhysicalLayout;

// How that keyboard is mapped onto the machine. Saved in settings, so only append.
typedef enum {
   KEYBOARD_MAP_POSITIONAL = 1,
   KEYBOARD_MAP_SYMBOLIC,
   KEYBOARD_MAP_MAXI,
   KEYBOARD_MAP_KEYRAH,
} KeyboardMapType;

// One selectable (Keyboard Layout, Keyboard Mapping) pair for a machine.
typedef struct KeyboardPreset {
   KeyboardPhysicalLayout layout;
   const char *layout_label;
   KeyboardMapType mapping;
   const char *mapping_label;
   const char *vkm_file;
   MenuKeyboardLayout text_layout; // table used for menu text entry
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
const KeyboardPreset *keyboard_preset_find(int machine_class, int layout, int mapping);
const KeyboardPreset *keyboard_preset_default(int machine_class);

// Menu helpers. A machine's layouts are listed in table order, and so are
// the mappings of one layout; the first mapping is that layout's default.
int keyboard_preset_layout_count(int machine_class);
const KeyboardPreset *keyboard_preset_layout_at(int machine_class, int index);
int keyboard_preset_layout_index(int machine_class, int layout);
int keyboard_preset_mapping_count(int machine_class, int layout);
const KeyboardPreset *keyboard_preset_mapping_at(int machine_class, int layout, int index);

unsigned int keyboard_layout_key_to_codepoint(MenuKeyboardLayout layout, long key, int shifted, int altgr);
int keyboard_layout_has_altgr(MenuKeyboardLayout layout);
int keyboard_layout_effective_shift(int shifted, int caps_lock, int altgr);
unsigned int keyboard_layout_lookup(const KeyboardLayoutKey *keys, unsigned count,
                                    long key, int shifted, int altgr);
char keyboard_layout_us_char(long key, int shifted);
unsigned int keyboard_layout_uk_char(long key, int shifted);
unsigned int keyboard_layout_no_char(long key, int shifted, int altgr);
unsigned int keyboard_layout_fr_char(long key, int shifted, int altgr);
unsigned int keyboard_layout_de_char(long key, int shifted, int altgr);
char keyboard_layout_c64_char(long key, int shifted);
char keyboard_layout_positional_char(long key, int shifted);
char keyboard_layout_maxi_char(long key, int shifted);

#ifdef __cplusplus
}
#endif

#endif
#endif