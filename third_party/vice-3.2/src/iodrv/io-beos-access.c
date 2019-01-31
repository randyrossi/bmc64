
/*
 * io-beos-access.c - BeOS specific I/O code.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

#include "vice.h"

#ifdef BEOS_COMPILE

#include "log.h"
#include "types.h"

#include "io-access.h"

#include <image.h>

#ifdef __HAIKU__
#include <Drivers.h>
#include <ISA.h>
#include <PCI.h>

#define POKE_DEVICE_FULLNAME "/dev/misc/poke"
#define POKE_SIGNATURE       'wltp'
#else
#ifndef WORDS_BIGENDIAN
static int (*vice_read_isa_io)(int dummy, void *addr, int size) = NULL;
static int (*vice_write_isa_io)(int dummy, void *addr, int size, uint32_t val) = NULL;
#endif
#endif

#ifdef __HAIKU__
enum {
    POKE_PORT_READ = B_DEVICE_OP_CODES_END + 1,
    POKE_PORT_WRITE,
    POKE_PORT_INDEXED_READ
};

typedef struct {
    uint32 signature;
    uint16 port;
    uint8 size;
    uint32 value;
} port_io_args;

static int poke_driver_fd;
#else
image_id addon_image = NULL;
#endif

int io_access_init(void)
{
#ifdef __HAIKU__
    poke_driver_fd = open(POKE_DEVICE_FULLNAME, O_RDWR);
    return (poke_driver_fd < 0) ? -1 : 0;
#else
#ifndef WORDS_BIGENDIAN
    addon_image = load_add_on("libroot.so");
    if (addon_image) {
        get_image_symbol(addon_image, "read_isa_io", B_SYMBOL_TYPE_TEXT, &vice_read_isa_io);
        get_image_symbol(addon_image, "write_isa_io", B_SYMBOL_TYPE_TEXT, &vice_write_isa_io);
    }
#endif
    return 0;
#endif
}

void io_access_shutdown(void)
{
#ifdef __HAIKU__
    close(poke_driver_fd);
#endif
}

void io_access_store_byte(uint16_t addr, uint8_t value)
{
#ifdef __HAIKU__
    port_io_args args = { POKE_SIGNATURE, addr, 1, value };

    ioctl(poke_driver_fd, POKE_PORT_WRITE, &args, sizeof(args));
#else
#ifndef WORDS_BIGENDIAN
    if (vice_write_isa_io) {
        vice_write_isa_io(0, (void *)(uint32_t)addr, 1, (uint32_t)value);
    }
#endif
#endif
}

uint8_t io_access_read_byte(uint16_t addr)
{
#ifdef __HAIKU__
    port_io_args args = { POKE_SIGNATURE, addr, 1, 0 };

    if (ioctl(poke_driver_fd, POKE_PORT_READ, &args, sizeof(args)) < 0) {
        return 0;
    }
    return (uint8_t)args.value;
#else
#ifndef WORDS_BIGENDIAN
    if (vice_read_isa_io) {
        return (uint8_t)vice_read_isa_io(0, (void *)(uint32_t)addr, 1);
    }
#endif
    return 0;
#endif
}

void io_access_store_long(uint16_t addr, uint32_t value)
{
#ifdef __HAIKU__
    port_io_args args = { POKE_SIGNATURE, addr, 4, value };

    ioctl(poke_driver_fd, POKE_PORT_WRITE, &args, sizeof(args));
#else
#ifndef WORDS_BIGENDIAN
    if (vice_write_isa_io) {
        vice_write_isa_io(0, (void *)(uint32_t)addr, 4, (uint32_t)value);
    }
#endif
#endif
}

uint32_t io_access_read_long(uint16_t addr)
{
#ifdef __HAIKU__
    port_io_args args = { POKE_SIGNATURE, addr, 4, 0 };

    if (ioctl(poke_driver_fd, POKE_PORT_READ, &args, sizeof(args)) < 0) {
        return 0;
    }
    return (uint32_t)args.value;
#else
#ifndef WORDS_BIGENDIAN
    if (vice_read_isa_io) {
         return (uint32_t)vice_read_isa_io(0, (void *)(uint32_t)addr, 4);
    }
#endif
    return 0;
#endif
}
#endif
