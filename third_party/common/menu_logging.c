/*
 * menu_logging.c
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 */

#include "menu_logging.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define CMDLINE_LINE_LEN 512

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

static int logging_option_enabled(const char *name) {
  FILE* fp = fopen("/cmdline.txt", "r");
  if (fp == NULL) {
    return 0;
  }

  int enabled = 0;
  char line[CMDLINE_LINE_LEN];
  while (fgets(line, CMDLINE_LINE_LEN - 1, fp)) {
    if (line[0] == '#') {
      continue;
    }

    char* option = strtok(line, " \t\r\n");
    while (option) {
      char* value = strchr(option, '=');
      if (value) {
        *value++ = '\0';
        if (strcmp(option, name) == 0) {
          enabled = strcmp(value, "true") == 0 || strcmp(value, "1") == 0;
        }
      }
      option = strtok(NULL, " \t\r\n");
    }
  }

  fclose(fp);
  return enabled;
}

static int logging_set_option(const char *name, const char *setting) {
  FILE* fp = fopen("/cmdline.txt", "r");
  if (fp == NULL) {
    return 1;
  }

  FILE* fp2 = fopen("/cmdline.new", "w");
  if (fp2 == NULL) {
    fclose(fp);
    return 1;
  }

  int found = 0;
  int need_space = 0;
  char replacement[CMDLINE_LINE_LEN];
  replacement[0] = '\0';
  char line[CMDLINE_LINE_LEN];
  while (fgets(line, CMDLINE_LINE_LEN - 1, fp)) {
    if (line[0] == '#') {
      fprintf(fp2, "%s", line);
      continue;
    }

    char* option = strtok(line, " \t\r\n");
    while (option) {
      char* value = strchr(option, '=');
      if (value) {
        *value++ = '\0';
        if (strcmp(option, name) == 0) {
          value = (char *) setting;
          found = 1;
        }
        int written = snprintf(replacement + strlen(replacement),
                               CMDLINE_LINE_LEN - strlen(replacement),
                               "%s%s=%s", need_space ? " " : "", option,
                               value);
        if (written < 0 || written >=
            (int) (CMDLINE_LINE_LEN - strlen(replacement))) {
          fclose(fp);
          fclose(fp2);
          unlink("/cmdline.new");
          return 1;
        }
        need_space = 1;
      }
      option = strtok(NULL, " \t\r\n");
    }
  }

  if (!found) {
    int written = snprintf(replacement + strlen(replacement),
                           CMDLINE_LINE_LEN - strlen(replacement), "%s%s=%s",
                           need_space ? " " : "", name, setting);
    if (written < 0 || written >=
        (int) (CMDLINE_LINE_LEN - strlen(replacement))) {
      fclose(fp);
      fclose(fp2);
      unlink("/cmdline.new");
      return 1;
    }
  }
  fprintf(fp2, "%s\n", replacement);

  fclose(fp);
  fclose(fp2);

  if (copy_file("/cmdline.new", "/cmdline.txt")) {
    return 1;
  }
  if (unlink("/cmdline.new")) {
    return 1;
  }
  return 0;
}

int logging_get_destination(void) {
  FILE* fp = fopen("/cmdline.txt", "r");
  if (fp == NULL) {
    return LOGGING_DESTINATION_OFF;
  }

  int destination = -1;
  char line[CMDLINE_LINE_LEN];
  while (fgets(line, CMDLINE_LINE_LEN - 1, fp)) {
    if (line[0] == '#') {
      continue;
    }

    char* option = strtok(line, " \t\r\n");
    while (option) {
      char* value = strchr(option, '=');
      if (value) {
        *value++ = '\0';
        if (strcmp(option, "enable_logging") == 0) {
          if (strcmp(value, "uart") == 0) {
            destination = LOGGING_DESTINATION_UART;
          } else if (strcmp(value, "file") == 0) {
            destination = LOGGING_DESTINATION_FILE;
          } else {
            destination = LOGGING_DESTINATION_OFF;
          }
        }
      }
      option = strtok(NULL, " \t\r\n");
    }
  }

  fclose(fp);
  if (destination >= 0) {
    return destination;
  }

  if (logging_option_enabled("enable_file_logging")) {
    return LOGGING_DESTINATION_FILE;
  }
  return logging_option_enabled("enable_serial") ? LOGGING_DESTINATION_UART :
      LOGGING_DESTINATION_OFF;
}

int logging_set_destination(int destination) {
  static const char *const settings[] = { "off", "uart", "file" };
  if (destination < LOGGING_DESTINATION_OFF ||
      destination > LOGGING_DESTINATION_FILE) {
    return 1;
  }
  return logging_set_option("enable_logging", settings[destination]);
}