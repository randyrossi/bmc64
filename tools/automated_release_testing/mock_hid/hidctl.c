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

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "protocol.h"

static long long monotonic_ms(void) {
    struct timespec time;
    if (clock_gettime(CLOCK_MONOTONIC, &time) != 0) {
        perror("clock_gettime");
        exit(1);
    }
    return (long long)time.tv_sec * 1000 + time.tv_nsec / 1000000;
}

static int exchange(int fd, const char *request, uint32_t id) {
    size_t sent = 0;
    size_t size = strlen(request);
    while (sent < size) {
        ssize_t n = write(fd, request + sent, size - sent);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) {
            perror("serial write");
            return 1;
        }
        sent += (size_t)n;
    }
    char prefix[32];
    snprintf(prefix, sizeof(prefix), "%u ", (unsigned)id);
    char line[256];
    size_t used = 0;
    long long deadline = monotonic_ms() + 4000;
    while (true) {
        long long remaining = deadline - monotonic_ms();
        if (remaining <= 0) break;
        struct pollfd pollfd = {.fd = fd, .events = POLLIN};
        int result = poll(&pollfd, 1, (int)remaining);
        if (result < 0 && errno == EINTR) continue;
        if (result < 0) {
            perror("serial poll");
            return 1;
        }
        if (result == 0) break;
        if (pollfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
            fprintf(stderr, "Serial device disconnected or failed\n");
            return 1;
        }
        char c;
        ssize_t n = read(fd, &c, 1);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) {
            fprintf(stderr, "Serial read failed or device disconnected\n");
            return 1;
        }
        if (c == '\r') continue;
        if (c == '\n') {
            line[used] = '\0';
            if (!strncmp(line, prefix, strlen(prefix))) {
                puts(line);
                const char *reply = line + strlen(prefix);
                return strcmp(reply, "OK") && strncmp(reply, "OK ", 3) ? 1 : 0;
            }
            if (used) fprintf(stderr, "Pico: %s\n", line);
            used = 0;
        } else if (used < sizeof(line) - 1) {
            line[used++] = c;
        } else {
            fprintf(stderr, "Oversized serial response\n");
            return 1;
        }
    }
    fprintf(stderr, "Timeout: command outcome unknown; do not blindly retry input\n");
    return 1;
}

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s SERIAL_DEVICE REQUEST_ID COMMAND [ARGS...]\n",
                argv[0]);
        return 2;
    }
    char line[COMMAND_LINE_SIZE];
    size_t used = 0;
    for (int i = 2; i < argc; ++i) {
        for (const char *p = argv[i]; *p; ++p) {
            if (*p == '\r' || *p == '\n') {
                fprintf(stderr, "Arguments must not contain line endings\n");
                return 2;
            }
        }
        int n = snprintf(line + used, sizeof(line) - used, "%s%s",
                         i == 2 ? "" : " ", argv[i]);
        if (n < 0 || (size_t)n >= sizeof(line) - used - 1) {
            fprintf(stderr, "Command too long\n");
            return 2;
        }
        used += (size_t)n;
    }
    command_t cmd;
    if (!command_parse(line, &cmd)) {
        fprintf(stderr, "Invalid command; see README.md for protocol\n");
        return 2;
    }
    line[used++] = '\n';
    line[used] = '\0';
    int fd = open(argv[1], O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        perror("open serial");
        return 1;
    }
    struct termios original, settings;
    if (tcgetattr(fd, &original) != 0) {
        perror("tcgetattr");
        close(fd);
        return 1;
    }
    settings = original;
    settings.c_iflag = 0;
    settings.c_oflag = 0;
    settings.c_lflag = 0;
    settings.c_cflag = CS8 | CLOCAL | CREAD;
    settings.c_cc[VMIN] = 1;
    settings.c_cc[VTIME] = 0;
    if (cfsetispeed(&settings, B115200) != 0 ||
        cfsetospeed(&settings, B115200) != 0 ||
        tcsetattr(fd, TCSANOW, &settings) != 0 ||
        tcflush(fd, TCIFLUSH) != 0 || fcntl(fd, F_SETFL, 0) < 0) {
        perror("configure serial");
        if (tcsetattr(fd, TCSANOW, &original) != 0)
            perror("restore serial");
        close(fd);
        return 1;
    }
    int result = exchange(fd, line, cmd.id);
    if (tcsetattr(fd, TCSANOW, &original) != 0) {
        perror("restore serial");
        result = 1;
    }
    close(fd);
    return result;
}
