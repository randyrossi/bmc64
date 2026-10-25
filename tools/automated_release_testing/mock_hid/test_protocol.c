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

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "protocol.h"

int main(void) {
    command_t cmd;
    assert(command_parse("1 PING", &cmd));
    assert(cmd.id == 1 && cmd.type == CMD_PING && cmd.count == 0);
    assert(command_parse("2147483647 STATUS\r\n", &cmd));
    assert(cmd.id == INT32_MAX && cmd.type == CMD_STATUS);
    assert(command_parse("2 KEY 255 4 5 6 7 8 9", &cmd));
    assert(cmd.type == CMD_KEY && cmd.count == 7 && cmd.values[6] == 9);
    assert(command_parse("3 KEY 0", &cmd));
    assert(command_parse("4 MOUSE 31 -127 127 0", &cmd));
    assert(cmd.type == CMD_MOUSE && cmd.values[1] == -127);
    assert(command_parse("5 PAD 0 255 255", &cmd));
    assert(cmd.type == CMD_PAD && cmd.values[2] == 255);
    assert(command_parse("6 RELEASE", &cmd));
    assert(cmd.type == CMD_RELEASE);
    assert(command_parse("\t7  KEY\t0 223 \r\n", &cmd));

    const char *invalid[] = {
        "", "PING", "0 PING", "-1 PING", "+1 PING",
        "2147483648 PING", "999999999999999999999999 PING",
        "1", "1 ping", "1 UNKNOWN", "1 PING 1", "1 RELEASE 0",
        "1 STATUS extra", "1 KEY", "1 KEY -1", "1 KEY 256",
        "1 KEY 0 0", "1 KEY 0 1", "1 KEY 0 3", "1 KEY 0 224",
        "1 KEY 0 255", "1 KEY 0 4 4", "1 KEY 0 4 5 6 7 8 9 10",
        "1 KEY 0 0x04", "1 KEY 0 +4", "1 KEY 0 4x",
        "1 MOUSE 0 0 0", "1 MOUSE 0 0 0 0 0", "1 MOUSE 32 0 0 0",
        "1 MOUSE -1 0 0 0", "1 MOUSE 0 -128 0 0", "1 MOUSE 0 128 0 0",
        "1 MOUSE 0 0 128 0", "1 MOUSE 0 0 0 128",
        "1 PAD 128 128", "1 PAD -1 128 0", "1 PAD 128 -1 0",
        "1 PAD 128 128 -1", "1 PAD 256 128 0",
        "1 PAD 128 256 0", "1 PAD 128 128 256",
    };
    for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        if (command_parse(invalid[i], &cmd)) {
            fprintf(stderr, "Unexpected valid command: %s\n", invalid[i]);
            return 1;
        }
    }
    assert(!command_parse("42 KEY -1", &cmd));
    assert(cmd.id == 42);
    char long_line[COMMAND_LINE_SIZE + 1];
    memset(long_line, ' ', sizeof(long_line));
    long_line[sizeof(long_line) - 1] = '\0';
    assert(!command_parse(long_line, &cmd));
    assert(cmd.id == 0);
    puts("PASS: protocol command shapes, IDs, bounds and malformed inputs");
    return 0;
}
