//
// profile_summary.c
//
// BMC64 - A bare metal Commodore emulator for Raspberry Pi
// Copyright (C) 2026 aminch <https://github.com/aminch>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// An at-a-glance summary of a profile's setup: its drives, control ports
// and expansions, read from the profile's vice.ini and BMC64 settings file.
// The same summary as the WebUI's Profiles page
// (src/webui/assets/js/profile_summary.js); keep the two in step.
//

#include "profiles_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "../sdcard/sd_fs.h"

// The most that is read of a profile's vice.ini or settings file.
#define SUMMARY_MAX_FILE_LEN 32768

// ---- Values read from the files ----

// The vice.ini resources the summary uses. VICE ignores the case of
// resource names, so these are matched ignoring case.
enum {
   V_DRIVE_TYPE,                               // Drive8Type ... Drive11Type
   V_IEC_DEVICE = V_DRIVE_TYPE + PROFILES_NUM_DRIVES,   // IECDevice8 ...
   V_FS_DIR = V_IEC_DEVICE + PROFILES_NUM_DRIVES,       // FSDevice8Dir ...
   V_RAM_BLOCK = V_FS_DIR + PROFILES_NUM_DRIVES,        // RAMBlock0 ... 5
   V_REU = V_RAM_BLOCK + 6,
   V_REU_SIZE,
   V_REU_FILE,
   V_GEORAM,
   V_GEORAM_SIZE,
   V_GEORAM_FILE,
   V_CART,
   V_KERNAL,
   V_SID_MODEL,
   V_SID_STEREO,
   V_VIDEO_SIZE,
   V_COLUMN_KEY,
   V_COUNT
};

static const char *const VICE_KEYS[V_COUNT] = {
   "Drive8Type", "Drive9Type", "Drive10Type", "Drive11Type",
   "IECDevice8", "IECDevice9", "IECDevice10", "IECDevice11",
   "FSDevice8Dir", "FSDevice9Dir", "FSDevice10Dir", "FSDevice11Dir",
   "RAMBlock0", "RAMBlock1", "RAMBlock2", "RAMBlock3", "RAMBlock4",
   "RAMBlock5",
   "REU", "REUsize", "REUfilename",
   "GEORAM", "GEORAMsize", "GEORAMfilename",
   "CartridgeFile", "KernalName", "SidModel", "SidStereo", "VideoSize",
   "C128ColumnKey",
};

// The BMC64 settings file keys the summary uses.
enum {
   S_DRIVE_TYPE,                               // drive_type_8 ... 11
   S_PORT = S_DRIVE_TYPE + PROFILES_NUM_DRIVES, // port_1 ... port_4
   S_DRIVE_MODEL_8 = S_PORT + 4,
   S_RAM_SIZE,
   S_ROM,                                      // rom_c0_lo ... rom_c2_hi
   S_3PLUS1 = S_ROM + 6,
   S_DIGIBLASTER,
   S_COUNT
};

static const char *const SETTINGS_KEYS[S_COUNT] = {
   "drive_type_8", "drive_type_9", "drive_type_10", "drive_type_11",
   "port_1", "port_2", "port_3", "port_4",
   "drive_model_8", "ram_size",
   "rom_c0_lo", "rom_c0_hi", "rom_c1_lo", "rom_c1_hi", "rom_c2_lo",
   "rom_c2_hi",
   "attach_3plus1_roms", "sid_digiblaster",
};

// Values point into the file texts; NULL when the key isn't there. The
// last line with a key wins.
typedef struct {
   const char *const *keys;
   int count;
   int ignore_case;
   const char **values;
} Values;

static void value_found(void *ctx, const char *key, const char *value) {
   Values *v = (Values *)ctx;
   for (int i = 0; i < v->count; i++) {
      if (v->ignore_case ? strcasecmp(key, v->keys[i]) == 0
                         : strcmp(key, v->keys[i]) == 0) {
         v->values[i] = value;
         return;
      }
   }
}

// The vice.ini section of each machine; Plus4Emu has no vice.ini.
static const char *vice_section(const char *key) {
   if (strcmp(key, "c64") == 0) return "C64";
   if (strcmp(key, "c128") == 0) return "C128";
   if (strcmp(key, "vic20") == 0) return "VIC20";
   if (strcmp(key, "plus4") == 0) return "PLUS4";
   if (strcmp(key, "pet") == 0) return "PET";
   return NULL;
}

static int is_space(char c) {
   return c == ' ' || c == '\t' || c == '\r';
}

// Turns every line outside the machine's sections (and the section lines
// themselves) into comments, so pkv_parse only sees the machine's values.
static void keep_section(char *text, const char *section) {
   int inside = 0;
   char *line = text;
   while (line != NULL && *line != '\0') {
      char *end = strchr(line, '\n');
      char *stop = end ? end : line + strlen(line);
      char *first = line;
      while (first < stop && is_space(*first)) first++;
      char *last = stop;
      while (last > first && is_space(last[-1])) last--;
      if (last - first >= 2 && *first == '[' && last[-1] == ']') {
         char *name = first + 1;
         char *name_end = last - 1;
         while (name < name_end && is_space(*name)) name++;
         while (name_end > name && is_space(name_end[-1])) name_end--;
         size_t length = (size_t)(name_end - name);
         inside = section != NULL && length == strlen(section) &&
                  strncasecmp(name, section, length) == 0;
         *first = '#';
      } else if (!inside && first < stop) {
         *first = '#';
      }
      line = end ? end + 1 : NULL;
   }
}

// Takes the quotes off a vice.ini string value.
static const char *unquote(const char *value, char *out, int out_size) {
   size_t length = strlen(value);
   if (length >= 2 && value[0] == '"' && value[length - 1] == '"') {
      int n = (int)length - 2 < out_size - 1 ? (int)length - 2 : out_size - 1;
      memcpy(out, value + 1, (size_t)n);
      out[n] = '\0';
   } else {
      snprintf(out, (size_t)out_size, "%s", value);
   }
   return out;
}

// A number at the start of a value (as JavaScript's parseInt), or fallback.
static int int_value(const char *value, int fallback) {
   if (value == NULL) {
      return fallback;
   }
   char text[24];
   unquote(value, text, sizeof(text));
   char *end;
   long n = strtol(text, &end, 10);
   return end == text ? fallback : (int)n;
}

// ---- Labels ----

// Drive model labels, as the BMC64 menu's Change Model shows them
// (DRIVE_TYPE_* in vice-3.3/src/drive/drive.h).
static const struct {
   int type;
   const char *label;
} DRIVE_TYPES[] = {
   {1540, "1540"}, {1541, "1541"}, {1542, "1541II"}, {1551, "1551"},
   {1570, "1570"}, {1571, "1571"}, {1573, "1571CR"}, {1581, "1581"},
   {2000, "FD2000"}, {4000, "FD4000"}, {2031, "2031"}, {2040, "2040"},
   {3040, "3040"}, {4040, "4040"}, {1001, "1001"}, {8050, "8050"},
   {8250, "8250"}, {4844, "CMDHD"},
};

static const char *drive_label(int type) {
   for (size_t i = 0; i < sizeof(DRIVE_TYPES) / sizeof(DRIVE_TYPES[0]); i++) {
      if (DRIVE_TYPES[i].type == type) {
         return DRIVE_TYPES[i].label;
      }
   }
   return NULL;
}

// A drive type value, or fallback when it's missing or isn't a drive model.
static int drive_type(const char *value, int fallback) {
   int type = int_value(value, fallback);
   return type == 0 || drive_label(type) ? type : fallback;
}

// The menu's Port choices; settings files keep the choice's position.
static const char *const PORT_CHOICES[] = {
   "None", "USB Gamepad 1", "USB Gamepad 2", "GPIO Bank 1", "GPIO Bank 2",
   "CURS + SPACE", "NUMPAD 64825", "NUMPAD 17930", "CURS + LCTRL",
   "USB Mouse (1351)", "USB Mouse (MicroMys)", "Custom Keyset 1",
   "Custom Keyset 2", "USB Gamepad 3", "USB Gamepad 4",
};
#define NUM_PORT_CHOICES (int)(sizeof(PORT_CHOICES) / sizeof(PORT_CHOICES[0]))

static const char *base_name(const char *path) {
   const char *slash = strrchr(path, '/');
   return slash ? slash + 1 : path;
}

// A ROM or cartridge file's name without its folder and .bin / .rom.
static void file_label(const char *path, char *out, int out_size) {
   snprintf(out, (size_t)out_size, "%s", base_name(path));
   size_t length = strlen(out);
   if (length >= 4 && (strcasecmp(out + length - 4, ".bin") == 0 ||
                       strcasecmp(out + length - 4, ".rom") == 0)) {
      out[length - 4] = '\0';
   }
}

static void size_label(int kb, char *out, int out_size) {
   if (kb >= 1024 && kb % 1024 == 0) {
      snprintf(out, (size_t)out_size, "%d MB", kb / 1024);
   } else {
      snprintf(out, (size_t)out_size, "%d KB", kb);
   }
}

// ---- Summary ----

// "C64/PAL/HDMI" gives "c64".
static void machine_key(const char *machine, char *out, int out_size) {
   profiles_machine_name(machine, out, out_size);
   for (char *c = out; *c; c++) {
      if (*c >= 'A' && *c <= 'Z') *c = (char)(*c - 'A' + 'a');
   }
}

static ProfileSummaryExtra *add_extra(ProfileSummary *s, const char *key,
                                      const char *value) {
   if (s->num_extras >= PROFILES_SUMMARY_MAX_EXTRAS) {
      return NULL;
   }
   ProfileSummaryExtra *x = &s->extras[s->num_extras++];
   snprintf(x->key, sizeof(x->key), "%s", key);
   snprintf(x->value, sizeof(x->value), "%s", value);
   x->file[0] = '\0';
   return x;
}

static void add_extra_file(ProfileSummary *s, const char *key,
                           const char *value, const char *file) {
   ProfileSummaryExtra *x = add_extra(s, key, value);
   if (x != NULL) {
      pkv_copy(x->file, sizeof(x->file), file);
   }
}

// plus4emu's one drive: its own drive_model_8 (0 1541, 1 1551). Its
// drive_type_N lines are junk: plus4emu doesn't answer the menu's question
// for them.
static void plus4emu_summary(const char **set, const char *const disks[],
                             ProfileSummary *s) {
   ProfileSummaryDrive *d = &s->drives[s->num_drives++];
   d->unit = PROFILES_FIRST_DRIVE;
   d->type = int_value(set[S_DRIVE_MODEL_8], 0) == 1 ? "1551" : "1541";
   pkv_copy(d->disk, sizeof(d->disk), disks[0] ? disks[0] : "");

   // Its options are its own keys (emux_save_additional_settings in
   // third_party/plus4emu/main.c).
   char text[PROFILES_SUMMARY_VALUE_LEN + 1];
   int ram = int_value(set[S_RAM_SIZE], 64);
   if (ram != 64) {
      size_label(ram, text, sizeof(text));
      add_extra(s, "RAM", text);
   }
   // The ROM files of the menu's Cartridge folder. Attach 3Plus1 ROMs fills
   // the C0 ones with 3plus1.rom, so that shows as 3-plus-1 instead.
   const char *cart = NULL;
   for (int i = 0; i < 6 && cart == NULL; i++) {
      if (set[S_ROM + i] != NULL && set[S_ROM + i][0] != '\0') {
         cart = set[S_ROM + i];
      }
   }
   if (int_value(set[S_3PLUS1], 0)) {
      add_extra(s, "ROM", "3-plus-1");
   } else if (cart != NULL) {
      file_label(cart, text, sizeof(text));
      add_extra_file(s, "Cart", text, cart);
   }
   if (int_value(set[S_DIGIBLASTER], 0)) {
      add_extra(s, "SID", "Digiblaster");
   }
}

void profile_summary_parse(const char *machine, char *vice_text,
                           char *settings_text,
                           const char *const disks[PROFILES_NUM_DRIVES],
                           ProfileSummary *s) {
   char key[16];
   machine_key(machine, key, sizeof(key));

   const char *vice[V_COUNT] = {0};
   const char *set[S_COUNT] = {0};
   const char *section = vice_section(key);
   if (vice_text != NULL && section != NULL) {
      keep_section(vice_text, section);
      Values v = {VICE_KEYS, V_COUNT, 1, vice};
      pkv_parse(vice_text, value_found, &v);
   }
   if (settings_text != NULL) {
      Values v = {SETTINGS_KEYS, S_COUNT, 0, set};
      pkv_parse(settings_text, value_found, &v);
   }

   s->known = vice_text != NULL || settings_text != NULL;
   s->num_drives = 0;
   s->num_ports = 0;
   s->num_extras = 0;

   static const char *const no_disks[PROFILES_NUM_DRIVES] = {0};
   if (disks == NULL) {
      disks = no_disks;
   }

   if (strcmp(key, "plus4emu") == 0) {
      plus4emu_summary(set, disks, s);
      return;
   }

   int is_c64 = strcmp(key, "c64") == 0;
   int is_c128 = strcmp(key, "c128") == 0;
   int is_vic20 = strcmp(key, "vic20") == 0;
   int is_pet = strcmp(key, "pet") == 0;
   char text[PROFILES_MAX_PATH_LEN + 1];

   // Drives: the settings file first, then vice.ini, then what drive 8 is
   // when nothing says otherwise.
   int iec = !is_vic20 && !is_pet;
   for (int i = 0; i < PROFILES_NUM_DRIVES; i++) {
      ProfileSummaryDrive *d = &s->drives[s->num_drives++];
      d->unit = PROFILES_FIRST_DRIVE + i;
      int fallback = i == 0 ? (is_pet ? 2031 : 1541) : 0;
      int type = drive_type(set[S_DRIVE_TYPE + i],
                            drive_type(vice[V_DRIVE_TYPE + i], fallback));
      d->type = type ? drive_label(type) : "";
      pkv_copy(d->disk, sizeof(d->disk), disks[i] ? disks[i] : "");
      d->iec = iec && int_value(vice[V_IEC_DEVICE + i], 0) != 0;
      d->iec_dir[0] = '\0';
      if (d->iec && vice[V_FS_DIR + i] != NULL) {
         unquote(vice[V_FS_DIR + i], text, sizeof(text));
         pkv_copy(d->iec_dir, sizeof(d->iec_dir), text);
      }
   }

   // Ports 3 and 4 are userport adapters: only shown when in use.
   int machine_ports = is_pet ? 0 : is_vic20 ? 1 : 2;
   for (int port = 1; port <= 4; port++) {
      int choice = int_value(set[S_PORT + port - 1], 0);
      if (port > machine_ports && (port <= 2 || !choice)) {
         continue;
      }
      ProfileSummaryPort *p = &s->ports[s->num_ports++];
      p->port = port;
      p->none = !choice;
      if (choice >= 0 && choice < NUM_PORT_CHOICES) {
         snprintf(p->device, sizeof(p->device), "%s", PORT_CHOICES[choice]);
      } else {
         snprintf(p->device, sizeof(p->device), "Port choice %d", choice);
      }
   }

   // Expansions and other things that set a profile apart.
   char value[PROFILES_SUMMARY_VALUE_LEN + 1];
   if (is_c64 || is_c128) {
      if (int_value(vice[V_REU], 0)) {
         size_label(int_value(vice[V_REU_SIZE], 512), value, sizeof(value));
         add_extra_file(s, "REU", value,
                        vice[V_REU_FILE] ? unquote(vice[V_REU_FILE], text,
                                                   sizeof(text))
                                         : "");
      }
      if (int_value(vice[V_GEORAM], 0)) {
         size_label(int_value(vice[V_GEORAM_SIZE], 512), value, sizeof(value));
         add_extra_file(s, "GeoRAM", value,
                        vice[V_GEORAM_FILE] ? unquote(vice[V_GEORAM_FILE], text,
                                                      sizeof(text))
                                            : "");
      }
   }
   if (is_vic20) {
      static const int blocks[6] = {3, 8, 8, 8, 0, 8};
      int kb = 0;
      for (int i = 0; i < 6; i++) {
         kb += int_value(vice[V_RAM_BLOCK + i], 0) ? blocks[i] : 0;
      }
      if (kb) {
         snprintf(value, sizeof(value), "+%d KB", kb);
         add_extra(s, "RAM", value);
      }
   }
   if (vice[V_CART] != NULL) {
      unquote(vice[V_CART], text, sizeof(text));
      if (text[0]) {
         file_label(text, value, sizeof(value));
         add_extra_file(s, "Cart", value, text);
      }
   }
   if (vice[V_KERNAL] != NULL) {
      unquote(vice[V_KERNAL], text, sizeof(text));
      if (text[0] && strncasecmp(text, "kernal", 6) != 0) {
         file_label(text, value, sizeof(value));
         add_extra_file(s, "Kernal", value, text);
      }
   }
   if ((is_c64 || is_c128) && vice[V_SID_MODEL] != NULL) {
      const char *model = int_value(vice[V_SID_MODEL], 0) ? "8580" : "6581";
      snprintf(value, sizeof(value), "%s%s", model,
               int_value(vice[V_SID_STEREO], 0) ? " x2" : "");
      add_extra(s, "SID", value);
   }
   if (is_pet && vice[V_VIDEO_SIZE] != NULL) {
      snprintf(value, sizeof(value), "%s columns",
               unquote(vice[V_VIDEO_SIZE], text, sizeof(text)));
      add_extra(s, "Screen", value);
   }
   // The 40/80 key: down (0) starts in 80 columns, up (1, VICE's default)
   // in 40.
   if (is_c128) {
      add_extra(s, "Screen", int_value(vice[V_COLUMN_KEY], 1) ? "40 columns"
                                                               : "80 columns");
   }
}

// Reads a whole file into a new buffer; NULL if it's missing, too big or
// there's no memory.
static char *read_whole(const char *path) {
   char *text = malloc(SUMMARY_MAX_FILE_LEN);
   if (text != NULL && sd_read_file(path, text, SUMMARY_MAX_FILE_LEN) < 0) {
      free(text);
      text = NULL;
   }
   return text;
}

// Main's settings file for a machine, as the emulator names it
// (src/viceapp.cpp), or "" for an unknown machine.
static void main_settings_path(const char *machine, char *out, int out_size) {
   static const char *const machines[] = {"c64", "c128", "vic20", "plus4",
                                          "plus4emu", "pet"};
   char key[16];
   machine_key(machine, key, sizeof(key));
   out[0] = '\0';
   for (size_t i = 0; i < sizeof(machines) / sizeof(machines[0]); i++) {
      if (strcmp(key, machines[i]) == 0) {
         if (i == 0) {
            snprintf(out, (size_t)out_size, "/settings.txt");
         } else {
            snprintf(out, (size_t)out_size, "/settings-%s.txt", key);
         }
      }
   }
}

int profiles_summary(const char *id, ProfileSummary *s) {
   ProfileFile pf;
   char vice_path[PROFILES_MAX_PATH_LEN];
   char settings_path[PROFILES_MAX_PATH_LEN];
   char *settings_text;
   if (id != NULL && strcmp(id, PROFILES_MAIN_ID) == 0) {
      // Main: the usual files, of the machine Main is for, and its startup
      // file (/profiles/main/<machine>.txt).
      const char *machine = profiles_main_machine();
      main_file_read(machine, &pf);
      pf.info = *profiles_main_info();
      pkv_copy(pf.info.machine, sizeof(pf.info.machine), machine);
      snprintf(vice_path, sizeof(vice_path), "/vice.ini");
      main_settings_path(machine, settings_path, sizeof(settings_path));
      settings_text = settings_path[0] ? read_whole(settings_path) : NULL;
   } else if (profiles_id_valid(id) &&
              profile_file_read(id, &pf) == PROFILES_OK) {
      profiles_path(id, "vice.ini", vice_path, sizeof(vice_path));
      profiles_settings_path(id, pf.info.machine, settings_path,
                             sizeof(settings_path));
      settings_text = read_whole(settings_path);
      // An older profile keeps its settings in settings.txt until it next
      // starts.
      if (settings_text == NULL) {
         profiles_path(id, "settings.txt", settings_path,
                       sizeof(settings_path));
         settings_text = read_whole(settings_path);
      }
   } else {
      return PROFILES_ERROR;
   }
   s->info = pf.info;
   snprintf(s->description, sizeof(s->description), "%s", pf.description);
   snprintf(s->autostart, sizeof(s->autostart), "%s", pf.autostart);
   char *vice_text = read_whole(vice_path);

   const char *disks[PROFILES_NUM_DRIVES];
   for (int i = 0; i < PROFILES_NUM_DRIVES; i++) {
      disks[i] = pf.disks[i];
   }
   profile_summary_parse(pf.info.machine, vice_text, settings_text, disks, s);
   free(vice_text);
   free(settings_text);
   return PROFILES_OK;
}
