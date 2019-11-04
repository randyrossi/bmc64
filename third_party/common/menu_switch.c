/*
 * menu_switch.c
 *
 * Written by
 *  Randy Rossi <randy.rossi@gmail.com>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "menu_switch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define OPTION_SCRATCH_LEN (KEY_LEN+1+VALUE_LEN+1)

#define ERROR_1 1
#define ERROR_2 2
#define ERROR_3 4
#define ERROR_4 8
#define ERROR_5 16
#define ERROR_6 32
#define ERROR_7 64
#define ERROR_8 128
#define ERROR_9 256
#define ERROR_10 512

static int entry_id;

static int next_id() {
   return entry_id++;
}

static void rtrim(char *txt) {
  if (!txt) return;
  int p=strlen(txt)-1;
  while (isspace(txt[p])) { txt[p] = '\0'; p--; }
}

static char* ltrim(char *txt) {
  if (!txt) return NULL;
  int p=0;
  while (isspace(txt[p])) { p++; }
  return txt+p;
}

static char* trim(char *txt) {
  if (!txt) return NULL;
  rtrim(txt);
  return ltrim(txt);
}

static int copy_file(char* from, char* to) {
  FILE *fp = fopen(from,"r");
  if (fp == NULL) {
     return 1;
  }
  FILE *fp2 = fopen(to,"w");
  if (fp2 == NULL) {
     fclose(fp);
     return 1;
  }
  int c = fgetc(fp);
  while (c != EOF) {
    fputc(c, fp2);
    c = fgetc(fp);
  }
  fclose(fp);
  fclose(fp2);
  return 0;
}

static int new_section(struct machine_entry** new_section, char* line) {
  char* header = &line[1];
  for (int i=0;i<strlen(header);i++) {
    if (header[i]==']') { header[i] = '\0'; break; }
  }
  header = trim(header);

  char *video_nam = trim(strtok(header, "/"));
  if (video_nam == NULL) return 1;
  char *video_std = trim(strtok(NULL, "/"));
  if (video_std == NULL) return 1;
  char *video_out = trim(strtok(NULL, "/"));
  if (video_out == NULL) return 1;
  char *video_res = trim(strtok(NULL, "/"));
  if (video_res == NULL) return 1;

  struct machine_entry* entry =
     (struct machine_entry*) malloc(sizeof(struct machine_entry));

  char desc[DESC_LEN];
  entry->id = next_id();
  entry->class = BMC64_MACHINE_CLASS_UNKNOWN;
  entry->video_standard = BMC64_VIDEO_STANDARD_UNKNOWN;
  entry->video_out = BMC64_VIDEO_OUT_UNKNOWN;

  strcpy(entry->desc,video_std);
  strcat(entry->desc," ");
  strcat(entry->desc,video_out);
  strcat(entry->desc," ");
  strcat(entry->desc,video_res);

  entry->options = NULL;
  entry->next = NULL;

  if (strcasecmp(video_nam,"vic20") == 0)
     entry->class = BMC64_MACHINE_CLASS_VIC20;
  else if (strcasecmp(video_nam,"c64") == 0)
     entry->class = BMC64_MACHINE_CLASS_C64;
  else if (strcasecmp(video_nam,"c128") == 0)
     entry->class = BMC64_MACHINE_CLASS_C128;
  else if (strcasecmp(video_nam,"plus4") == 0)
     entry->class = BMC64_MACHINE_CLASS_PLUS4;

  if (strcasecmp(video_std,"ntsc") == 0)
     entry->video_standard = BMC64_VIDEO_STANDARD_NTSC;
  else if (strcasecmp(video_std,"pal") == 0)
     entry->video_standard = BMC64_VIDEO_STANDARD_PAL;

  if (strcasecmp(video_out,"hdmi") == 0)
     entry->video_out = BMC64_VIDEO_OUT_HDMI;
  else if (strcasecmp(video_out,"composite") == 0)
     entry->video_out = BMC64_VIDEO_OUT_COMPOSITE;

  *new_section = entry;
  return 0;
}

static int append_to_section(struct machine_entry* section, char* line) {
  if (!section) return 0;

  line = trim(line);
  if (strlen(line) == 0) return 0;

  char *key = trim(strtok(line, "="));
  char *value = trim(strtok(NULL, "="));

  if (key == NULL || value == NULL) {
    return 1;
  }

  struct machine_option* option =
     (struct machine_option*) malloc(sizeof(struct machine_option));

  strncpy (option->key, key, KEY_LEN-1);
  strncpy (option->value, value, VALUE_LEN-1);
  option->next = NULL;

  struct machine_option* prev = NULL;
  struct machine_option* ptr = section->options;
  while (ptr) {
     prev = ptr;
     ptr = ptr->next;
  }
  if (prev)
    prev->next = option;
  else
    section->options = option;

  return 0;
}

// Load and parse the machine config from machines.txt
// Returns non-zero on error.
int load_machines(struct machine_entry** head) {
  FILE* fp = fopen("/machines.txt","r");
  if (fp == NULL) {
     *head = NULL;
     return 1;
  }

  char line[CONFIG_TXT_LINE_LEN];

  struct machine_entry* current_section = NULL;
  struct machine_entry* first_section = NULL;
  entry_id = 0;
  while (fgets(line, CONFIG_TXT_LINE_LEN - 1, fp)) {
    if (feof(fp))
      break;
    if (strlen(line) == 0)
      continue;
    if (line[0] == '#')
      continue;

    if (line[0] == '[') {
      struct machine_entry* section;
      if (new_section(&section, line) == 0) {
        if (current_section != NULL) {
           current_section->next = section;
        }
        current_section = section;
        if (!first_section) {
           first_section = current_section;
        }
      }
    } else {
      append_to_section(current_section, line);
    }
  }

  *head = first_section;
  fclose(fp);
  return 0;
}

void free_machines(struct machine_entry* head) {
  struct machine_entry* ptr = head;
  struct machine_entry* next_ptr;
  while (ptr) {
     struct machine_option* opt = ptr->options;
     struct machine_option* next_opt;
     while (opt) {
        next_opt = opt->next;
        free(opt);
        opt = next_opt;
     }
     next_ptr = ptr->next;
     free(ptr);
     ptr = next_ptr;
  }
}

static struct machine_option* find_option(char *key, struct machine_option *head) {
  struct machine_option* opt = head;
  while (opt) {
    if(strcmp(opt->key, key) == 0) {
      return opt;
    }
    opt=opt->next;
  }
  return NULL;
}

// Single line version of apply_override where each param
// is one per line.  This is a non-destructive update in
// that existing parameters will not be altered other
// than those found in the options list for the machine
// entry.  However, the following options will be removed
// unless machine_timing=hdmi-custom or pal-custom is set:
// hdmi_cvt, hdmi_timings
static void apply_override_s(char *line,
                             struct machine_entry *head, char* kernel_name,
                             int need_kernel,
                             int* have_kernel,
                             int* have_timings, int* have_cvt, int is_custom) {
  if (strlen(line) > 0 && line[0] != '#') {
    char *key = strtok(line, "=");
    char *value = strtok(NULL, "=");
    if (key == NULL || value == NULL) {
       return;
    }

    // If we find a kernal name, overwrite it now.
    if(strcmp(key, "kernel") == 0) {
       if (need_kernel) {
          snprintf(line, CONFIG_TXT_LINE_LEN, "kernel=%s\n",kernel_name);
          *have_kernel = 1;
       } else {
          line[0] = '\0';
       }
       return;
    }

    // Make sure we remove custom hdmi stuff unless we are using a custom mode
    if(strcmp(key, "hdmi_timings") == 0) {
       *have_timings = 1;
       if (!is_custom) {
          line[0] = '\0';
          return;
       }
    }
    if(strcmp(key, "hdmi_cvt") == 0) {
       *have_cvt = 1;
       if (!is_custom) {
          line[0] = '\0';
          return;
       }
    }

    struct machine_option* found = find_option(key, head->options);
    char new_option[OPTION_SCRATCH_LEN];
    if (found) {
       snprintf(new_option, OPTION_SCRATCH_LEN, "%s=%s\n", key, found->value);
    } else {
       // Value already has newline since it came from original input.
       snprintf(new_option, OPTION_SCRATCH_LEN, "%s=%s", key, value);
    }
    strcpy(line, new_option);
  }
}


// Multi param version of apply_override where there are
// multiple parameters on a single line of text. This is
// a non-destructive update in that existing parameters
// will not be altered other than those found in the
// options list for the machine entry.  The following
// options are removed first:
// cycles_per_second, cycles_per_refresh, machine_timing
// serial, demo, audio_out, disk_partition
static int apply_override_m(char *line, struct machine_entry *head) {
  if (strlen(line) > 0 && line[0] != '#') {
    char replacement[CONFIG_TXT_LINE_LEN];
    char new_option[OPTION_SCRATCH_LEN];

    replacement[0] = '\0';

    line = trim(line);
    int total = strlen(line);
    if (total == 0) return 0;

    int have_cycles_per_second = 0;
    int have_machine_timing = 0;
    int have_serial = 0;
    int have_demo = 0;
    int have_audio_out = 0;
    int have_disk_partition = 0;

    char *option = line;
    int pos = 0;
    int need_space = 0;
    while (pos < total) {
      char* next_option = strtok(option, " ");
      if (next_option == NULL) {
        break;
      }

      // Advance past this token for next iteration before
      // we muck with the option via strtok
      pos+= strlen(next_option)+1;

      char *key = strtok(next_option, "=");
      char *value = strtok(NULL, "=");

      if (strcmp(key,"cycles_per_second") == 0) { have_cycles_per_second = 1; }
      if (strcmp(key,"machine_timing") == 0) { have_machine_timing = 1; }
      if (strcmp(key,"serial") == 0) { have_serial = 1; }
      if (strcmp(key,"demo") == 0) { have_demo = 1; }
      if (strcmp(key,"audio_out") == 0) { have_audio_out = 1; }
      if (strcmp(key,"disk_partition") == 0) { have_disk_partition = 1; }

      struct machine_option* found = find_option(key, head->options);
      if (found) {
         if (need_space) { strcat(replacement," ");}
         snprintf(new_option, OPTION_SCRATCH_LEN, "%s=%s", key, found->value);
         strcat(replacement, new_option);
         need_space=1;
      } else {
         if (strcmp(key,"cycles_per_refresh")!=0 &&
             strcmp(key,"cycles_per_second")!=0 &&
             strcmp(key,"machine_timing")!=0 &&
             strcmp(key,"serial")!=0 &&
             strcmp(key,"demo")!=0 &&
             strcmp(key,"audio_out")!=0 &&
             strcmp(key,"disk_partition")!=0) {
            if (need_space) { strcat(replacement," "); }
            snprintf(new_option, OPTION_SCRATCH_LEN, "%s=%s", key, value);
            strcat(replacement, new_option);
            need_space=1;
         }
      }

      option = line + pos;
    }

    if (!have_cycles_per_second) {
       struct machine_option* found = find_option("cycles_per_second", head->options);
       if (found) {
          if (need_space) { strcat(replacement," "); }
          snprintf(new_option, OPTION_SCRATCH_LEN, "%s=%s", found->key, found->value);
          strcat(replacement, new_option);
          need_space=1;
       }
    }
    if (!have_machine_timing) {
       struct machine_option* found = find_option("machine_timing", head->options);
       if (found) {
          if (need_space) { strcat(replacement," "); }
          snprintf(new_option, OPTION_SCRATCH_LEN, "%s=%s", found->key, found->value);
          strcat(replacement, new_option);
          need_space=1;
       }
    }
    if (!have_serial) {
       struct machine_option* found = find_option("serial", head->options);
       if (found) {
          if (need_space) { strcat(replacement," "); }
          snprintf(new_option, OPTION_SCRATCH_LEN, "%s=%s", found->key, found->value);
          strcat(replacement, new_option);
          need_space=1;
       }
    }
    if (!have_demo) {
       struct machine_option* found = find_option("demo", head->options);
       if (found) {
          if (need_space) { strcat(replacement," "); }
          snprintf(new_option, OPTION_SCRATCH_LEN, "%s=%s", found->key, found->value);
          strcat(replacement, new_option);
          need_space=1;
       }
    }
    if (!have_audio_out) {
       struct machine_option* found = find_option("audio_out", head->options);
       if (found) {
          if (need_space) { strcat(replacement," "); }
          snprintf(new_option, OPTION_SCRATCH_LEN, "%s=%s", found->key, found->value);
          strcat(replacement, new_option);
          need_space=1;
       }
    }
    if (!have_disk_partition) {
       struct machine_option* found = find_option("disk_partition", head->options);
       if (found) {
          if (need_space) { strcat(replacement," "); }
          snprintf(new_option, OPTION_SCRATCH_LEN, "%s=%s", found->key, found->value);
          strcat(replacement, new_option);
          need_space=1;
       }
    }

    strcat(replacement, "\n");
    strcpy(line, replacement);
  }
  return 0;
}

int apply_cmdline(struct machine_entry* head) {
  FILE* fp = fopen("/cmdline.txt","r");
  if (fp == NULL) {
     return ERROR_1;
  }

  FILE* fp2 = fopen("/cmdline.new","w");
  if (fp2 == NULL) {
     fclose(fp);
     return ERROR_2;
  }

  char line[CONFIG_TXT_LINE_LEN];

  while (fgets(line, CONFIG_TXT_LINE_LEN - 1, fp)) {
    if (feof(fp))
      break;
    apply_override_m(line, head);
    fprintf(fp2,"%s",line);
  }

  fclose(fp);
  fclose(fp2);

  if (copy_file("/cmdline.new","/cmdline.txt")) {
     return ERROR_3;
  }
  if (unlink("/cmdline.new")) {
     return ERROR_4;
  }
  return 0;
}

// Reads config.txt and creates config.txt.new, overwriting
// any config.txt related items from the machine_entry. If
// kernel param is absent, it will be added.
int apply_config(struct machine_entry* head, int pi_model) {
  char kernel_name[VALUE_LEN];
  switch (pi_model) {
    case 0:
    case 1:
       strcpy(kernel_name,"kernel.img");
       break;
    case 2:
       strcpy(kernel_name,"kernel7.img");
       break;
    case 3:
       strcpy(kernel_name,"kernel8-32.img");
       break;
    default:
       return ERROR_5;
  }

  switch (head->class) {
     case BMC64_MACHINE_CLASS_C64:
        // Kernel name not required for c64.
        break;
     case BMC64_MACHINE_CLASS_VIC20:
        strcat(kernel_name,".vic20");
        break;
     case BMC64_MACHINE_CLASS_C128:
        strcat(kernel_name,".c128");
        break;
     case BMC64_MACHINE_CLASS_PLUS4:
        strcat(kernel_name,".plus4");
        break;
     default:
        return ERROR_6;
   }

  FILE* fp = fopen("/config.txt","r");
  if (fp == NULL) {
     return ERROR_7;
  }

  FILE* fp2 = fopen("/config.new","w");
  if (fp2 == NULL) {
     fclose(fp);
     return ERROR_8;
  }

  int is_custom = 0;
  struct machine_option* found = find_option("machine_timing", head->options);
  if (found && (strcmp(found->value, "ntsc-custom") == 0 || strcmp(found->value,"pal-custom") == 0)) {
    is_custom = 1;
  }

  char line[CONFIG_TXT_LINE_LEN];
  int have_kernel=0;
  int need_kernel=head->class != BMC64_MACHINE_CLASS_C64;
  int have_timings = 0;
  int have_cvt = 0;
  while (fgets(line, CONFIG_TXT_LINE_LEN - 1, fp)) {
    if (feof(fp))
      break;

    apply_override_s(line, head, kernel_name, need_kernel, &have_kernel,
                     &have_timings, &have_cvt, is_custom);
    fprintf(fp2,"%s",line);
  }

  // This may have been overwritten by the pass above. If not present, add
  // now.
  if (!have_kernel && need_kernel) {
    fprintf(fp2,"kernel=%s\n", kernel_name);
  }

  // Ensure we add custom timings if present.
  if (is_custom) {
    if (!have_timings) {
       found = find_option("hdmi_timings", head->options);
       if (found) {
          fprintf(fp2,"hdmi_timings=%s\n", found->value);
       }
    }
    if (!have_cvt) {
       found = find_option("hdmi_cvt", head->options);
       if (found) {
          fprintf(fp2,"hdmi_cvt=%s\n", found->value);
       }
    }
  }

  fclose(fp);
  fclose(fp2);

  if (copy_file("/config.new","/config.txt")) {
    return ERROR_9;
  }
  if (unlink("/config.new")) {
    return ERROR_10;
  }
  return 0;
}
