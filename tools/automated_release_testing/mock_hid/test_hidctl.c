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

#define _XOPEN_SOURCE 600

#include <assert.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static void check_exchange(const char *response, int expected) {
    int master = posix_openpt(O_RDWR | O_NOCTTY);
    assert(master >= 0);
    assert(grantpt(master) == 0 && unlockpt(master) == 0);
    char *slave = ptsname(master);
    assert(slave);
    /* Keep a slave open so the master does not report HUP before client open. */
    int anchor = open(slave, O_RDWR | O_NOCTTY);
    assert(anchor >= 0);
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        close(master);
        close(anchor);
        execl("./build-native/hidctl", "hidctl", slave, "42", "PING", NULL);
        _exit(127);
    }
    char line[128];
    size_t used = 0;
    while (used < sizeof(line) - 1) {
        struct pollfd fd = {.fd = master, .events = POLLIN};
        assert(poll(&fd, 1, 2000) == 1);
        assert(read(master, line + used, 1) == 1);
        if (line[used++] == '\n') break;
    }
    line[used] = '\0';
    assert(strcmp(line, "42 PING\n") == 0);
    if (response) {
        assert(write(master, response, strlen(response)) ==
               (ssize_t)strlen(response));
    }
    int status = 0;
    bool finished = false;
    for (unsigned i = 0; i < 120; ++i) {
        if (waitpid(child, &status, WNOHANG) == child) {
            finished = true;
            break;
        }
        const struct timespec interval = {0, 50000000};
        nanosleep(&interval, NULL);
    }
    if (!finished) {
        kill(child, SIGKILL);
        waitpid(child, &status, 0);
        fprintf(stderr, "hidctl exceeded test deadline\n");
        exit(1);
    }
    assert(WIFEXITED(status) && WEXITSTATUS(status) == expected);
    close(anchor);
    close(master);
}

int main(void) {
    check_exchange("0 EVENT ready protocol=1\r\n41 OK\r\n42 OK\r\n", 0);
    check_exchange("42 OK protocol=1 mounted=1\r\n", 0);
    check_exchange("42 ERROR usb-unavailable\r\n", 1);
    check_exchange("42 OKAY\r\n", 1);
    check_exchange(NULL, 1);
    puts("PASS: serial ACK correlation, status, errors and timeout");
    return 0;
}
