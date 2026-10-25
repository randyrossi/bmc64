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

#ifndef MOCK_HID_PROTOCOL_H
#define MOCK_HID_PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>

#define COMMAND_LINE_SIZE 128
#define INPUT_LEASE_MS 2000

typedef enum {
    CMD_PING, CMD_STATUS, CMD_KEY, CMD_MOUSE, CMD_PAD, CMD_RELEASE
} command_type_t;

typedef struct {
    uint32_t id;
    command_type_t type;
    int values[7];
    unsigned count;
} command_t;

/* On failure, id is preserved if valid, so errors can be correlated. */
bool command_parse(const char *line, command_t *command);

#endif
