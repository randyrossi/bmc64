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

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "hardware/uart.h"
#include "pico/stdlib.h"
#include "tusb.h"
#include "protocol.h"

enum { KEYBOARD, MOUSE, PAD, INTERFACES };
static bool completed[INTERFACES];
static bool failed[INTERFACES];
static bool held[INTERFACES];
static bool release_pending = true;
static uint32_t last_command_ms;
static uint8_t keyboard_leds;

static uint32_t now_ms(void) {
    return to_ms_since_boot(get_absolute_time());
}

static void reply(uint32_t id, const char *text) {
    char line[128];
    snprintf(line, sizeof(line), "%" PRIu32 " %s\r\n", id, text);
    uart_puts(uart0, line);
}

static bool send_report(unsigned instance, const void *data, uint16_t size) {
    uint32_t start = now_ms();
    while (!tud_hid_n_ready(instance)) {
        tud_task();
        if (!tud_mounted() || now_ms() - start >= 250) return false;
    }
    completed[instance] = false;
    failed[instance] = false;
    if (!tud_hid_n_report(instance, 0, data, size)) return false;
    while (!completed[instance]) {
        tud_task();
        if (!tud_mounted() || failed[instance] ||
            now_ms() - start >= 250) return false;
    }
    return true;
}

static bool release_all(void) {
    const uint8_t keyboard[8] = {0};
    const uint8_t mouse[5] = {0};
    const uint8_t pad[3] = {128, 128, 0};
    bool ok = true;
    if (send_report(KEYBOARD, keyboard, sizeof(keyboard))) held[KEYBOARD] = false;
    else ok = false;
    uint16_t mouse_length =
        tud_hid_n_get_protocol(MOUSE) == HID_PROTOCOL_BOOT ? 3 : sizeof(mouse);
    if (send_report(MOUSE, mouse, mouse_length)) held[MOUSE] = false;
    else ok = false;
    if (send_report(PAD, pad, sizeof(pad))) held[PAD] = false;
    else ok = false;
    return ok;
}

static void execute(const command_t *cmd) {
    last_command_ms = now_ms();
    if (cmd->type == CMD_PING) {
        reply(cmd->id, "OK");
        return;
    }
    if (cmd->type == CMD_STATUS) {
        char text[100];
        snprintf(text, sizeof(text),
            "OK protocol=1 mounted=%u suspended=%u leds=%u lease_ms=%u pending=%u",
            tud_mounted(), tud_suspended(), keyboard_leds,
            INPUT_LEASE_MS, release_pending);
        reply(cmd->id, text);
        return;
    }
    if (!tud_mounted() || tud_suspended()) {
        reply(cmd->id, "ERROR usb-unavailable");
        return;
    }
    if (cmd->type == CMD_RELEASE) {
        release_pending = !release_all();
        reply(cmd->id, release_pending ? "ERROR release-timeout" : "OK");
        return;
    }
    if (release_pending) {
        reply(cmd->id, "ERROR release-pending");
        return;
    }
    unsigned instance;
    uint8_t data[8] = {0};
    uint16_t length;
    const int *v = cmd->values;
    if (cmd->type == CMD_KEY) {
        instance = KEYBOARD;
        length = 8;
        data[0] = (uint8_t)v[0];
        held[instance] = v[0] != 0 || cmd->count > 1;
        for (unsigned i = 1; i < cmd->count; ++i) data[i + 1] = (uint8_t)v[i];
    } else if (cmd->type == CMD_MOUSE) {
        instance = MOUSE;
        bool boot = tud_hid_n_get_protocol(MOUSE) == HID_PROTOCOL_BOOT;
        if (boot && (v[0] > 7 || v[3] != 0)) {
            reply(cmd->id, "ERROR mouse-boot-protocol-limit");
            return;
        }
        length = boot ? 3 : 5;
        data[0] = (uint8_t)v[0];
        data[1] = (uint8_t)(int8_t)v[1];
        data[2] = (uint8_t)(int8_t)v[2];
        data[3] = (uint8_t)(int8_t)v[3];
        held[instance] = v[0] != 0;
    } else {
        instance = PAD;
        length = 3;
        for (unsigned i = 0; i < 3; ++i) data[i] = (uint8_t)v[i];
        held[instance] = v[0] != 128 || v[1] != 128 || v[2] != 0;
    }
    if (!send_report(instance, data, length)) {
        release_pending = true;
        reply(cmd->id, "ERROR report-timeout");
    } else {
        reply(cmd->id, "OK");
    }
}

void tud_hid_report_complete_cb(uint8_t instance, const uint8_t *report,
                                uint16_t length) {
    (void)report;
    (void)length;
    if (instance < INTERFACES) completed[instance] = true;
}

void tud_hid_report_failed_cb(uint8_t instance, hid_report_type_t type,
                              const uint8_t *report, uint16_t length) {
    (void)type;
    (void)report;
    (void)length;
    if (instance < INTERFACES) failed[instance] = true;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t id,
    hid_report_type_t type, uint8_t *buffer, uint16_t length) {
    if (instance == KEYBOARD && id == 0 && type == HID_REPORT_TYPE_OUTPUT &&
        length >= 1) {
        buffer[0] = keyboard_leds;
        return 1;
    }
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t id,
    hid_report_type_t type, const uint8_t *buffer, uint16_t length) {
    if (instance == KEYBOARD && id == 0 && type == HID_REPORT_TYPE_OUTPUT &&
        length >= 1) keyboard_leds = buffer[0];
}

void tud_mount_cb(void) {
    release_pending = true;
}

void tud_umount_cb(void) {
    release_pending = true;
    memset(held, 0, sizeof(held));
    keyboard_leds = 0;
}

int main(void) {
    uart_init(uart0, 115200);
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);
    uart_set_hw_flow(uart0, false, false);
    uart_set_format(uart0, 8, 1, UART_PARITY_NONE);
    tusb_init();
    reply(0, "EVENT ready protocol=1");

    char line[COMMAND_LINE_SIZE];
    size_t used = 0;
    bool invalid_line = false;
    bool release_error_reported = false;
    uint32_t last_release_ms = now_ms() - 100;
    while (true) {
        tud_task();
        bool active = held[KEYBOARD] || held[MOUSE] || held[PAD];
        if (active && !release_pending &&
            now_ms() - last_command_ms >= INPUT_LEASE_MS) {
            release_pending = true;
            reply(0, "EVENT watchdog-release");
        }
        if (release_pending && tud_mounted() && !tud_suspended() &&
            now_ms() - last_release_ms >= 100) {
            release_pending = !release_all();
            last_release_ms = now_ms();
            if (release_pending && !release_error_reported)
                reply(0, "ERROR automatic-release-timeout");
            release_error_reported = release_pending;
        }
        if (!uart_is_readable(uart0)) continue;
        char c = uart_getc(uart0);
        if (c == '\n') {
            line[used] = '\0';
            command_t cmd;
            if (invalid_line) reply(0, "ERROR invalid-or-long-line");
            else if (!command_parse(line, &cmd)) reply(cmd.id, "ERROR syntax");
            else execute(&cmd);
            used = 0;
            invalid_line = false;
        } else if (used == sizeof(line) - 1 || c == '\0' ||
                   (c != '\r' && c != '\t' && (c < 32 || c > 126))) {
            invalid_line = true;
        } else if (!invalid_line) {
            line[used++] = c;
        }
    }
}
