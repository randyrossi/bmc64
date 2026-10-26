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

#include "protocol.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

static bool number(const char *text, long min, long max, int *value) {
    if (!*text) return false;
    const char *p = text;
    if (*p == '-') ++p;
    if (!*p) return false;
    for (; *p; ++p) {
        if (*p < '0' || *p > '9') return false;
    }
    char *end;
    errno = 0;
    long n = strtol(text, &end, 10);
    if (errno || *end || n < min || n > max) return false;
    *value = (int)n;
    return true;
}

bool command_line_blank(const char *line) {
    return line[strspn(line, " \t\r")] == '\0';
}

bool command_parse(const char *line, command_t *command) {
    memset(command, 0, sizeof(*command));
    if (strlen(line) >= COMMAND_LINE_SIZE) return false;
    char copy[COMMAND_LINE_SIZE];
    strcpy(copy, line);
    char *token = strtok(copy, " \t\r\n");
    int id;
    if (!token || !number(token, 1, INT32_MAX, &id)) return false;
    command->id = (uint32_t)id;
    token = strtok(NULL, " \t\r\n");
    if (!token) return false;
    if (!strcmp(token, "PING")) command->type = CMD_PING;
    else if (!strcmp(token, "STATUS")) command->type = CMD_STATUS;
    else if (!strcmp(token, "KEY")) command->type = CMD_KEY;
    else if (!strcmp(token, "MOUSE")) command->type = CMD_MOUSE;
    else if (!strcmp(token, "PAD")) command->type = CMD_PAD;
    else if (!strcmp(token, "RELEASE")) command->type = CMD_RELEASE;
    else return false;

    while ((token = strtok(NULL, " \t\r\n"))) {
        if (command->count == 7 ||
            !number(token, -127, 255, &command->values[command->count]))
            return false;
        ++command->count;
    }
    const int *v = command->values;
    switch (command->type) {
        case CMD_KEY:
            if (command->count < 1 || v[0] < 0) return false;
            for (unsigned i = 1; i < command->count; ++i) {
                if (v[i] < 4 || v[i] > 223) return false;
                for (unsigned j = 1; j < i; ++j)
                    if (v[i] == v[j]) return false;
            }
            return true;
        case CMD_MOUSE:
            return command->count == 4 && v[0] >= 0 && v[0] <= 31 &&
                   v[1] <= 127 && v[2] <= 127 && v[3] <= 127;
        case CMD_PAD:
            return command->count == 3 && v[0] >= 0 && v[1] >= 0 &&
                   v[2] >= 0;
        default:
            return command->count == 0;
    }
}
