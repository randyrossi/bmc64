/*
 * cw-amiga-device.c - catweasel device handler driver.
 *
 * Written by
 *  Ian Gledhill <ian.gledhill@btinternet.com>
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

#ifdef AMIGA_SUPPORT

#define __USE_INLINE__

#include <stdlib.h>
#include <string.h>

#include <exec/exec.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include <clib/exec_protos.h>
#include <clib/alib_protos.h>

#include <exec/exec.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/devices.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <devices/trackdisk.h>
#include <proto/exec.h>

#include "cw-amiga.h"
#include "log.h"
#include "types.h"

static unsigned char read_sid(unsigned char reg, int chipno); // Read a SID register
static void write_sid(unsigned char reg, unsigned char data, int chipno); // Write a SID register

#define MAXSID 2

static int gSIDs = 0;

static int sids_found = -1;

/* read value from SIDs */
int cw_device_read(uint16_t addr, int chipno)
{
    /* check if chipno and addr is valid */
    if (chipno < gSIDs && addr < 0x20) {
        return read_sid(addr, chipno);
    }

    return 0;
}

/* write value into SID */
void cw_device_store(uint16_t addr, uint8_t val, int chipno)
{
    /* check if chipno and addr is valid */
    if (chipno < gSIDs && addr < 0x20) {
        write_sid(addr, val, chipno);
    }
}

#undef BYTE
#undef WORD
#include <exec/types.h>
#include <proto/expansion.h>
#include <proto/exec.h>

static struct MsgPort *gDiskPort[2] = {NULL, NULL};
static struct IOExtTD *gCatweaselReq[2] = {NULL, NULL};

static BOOL gSwapSIDs = FALSE;

static void close_device(void)
{
    int i;

    for (i = 0; i < 2; i++) {
        if (gCatweaselReq[i]) {
            CloseDevice((struct IORequest *)gCatweaselReq[i]);
            if (gCatweaselReq[i] != NULL) {
                DeleteExtIO((struct IORequest *)gCatweaselReq[i]);
            }
            if (gDiskPort[i] != NULL) {
                DeletePort(gDiskPort[i]);
            }
            gCatweaselReq[i] = NULL;
            gDiskPort[i] = NULL;
        }
    }
}


int cw_device_open(void)
{
    unsigned int i;

    if (sids_found > 0) {
        return 0;
    }

    if (!sids_found) {
        return -1;
    }

    sids_found = 0;

    log_message(LOG_DEFAULT, "Detecting device driver based CatWeasel boards.");

    gSIDs = 0;
    gSwapSIDs = FALSE;

    for (i = 0; i < 2; i++) {
        if ((gDiskPort[i] = CreatePort(NULL, 0)) != NULL) {
            if (gCatweaselReq[i] = (struct IOExtTD *)CreateExtIO(gDiskPort[i], sizeof(struct IOExtTD))) {
                if (OpenDevice("catweaselsid.device", i, (struct IORequest *)gCatweaselReq[i], 0)) {
                    DeleteExtIO((struct IORequest *)gCatweaselReq[i]);
                    DeletePort(gDiskPort[i]);
                    gCatweaselReq[i] = NULL;
                    gDiskPort[i] = NULL;
                } else {
                    gSIDs++;
                }
            }
        }
    }

    if (gSIDs == 0) {
        log_message(LOG_DEFAULT, "No device driver based CatWeasel boards found.");
        close_device();
        return -1;
    }

    if (gCatweaselReq[1] && gCatweaselReq[0] == NULL) {
        gSwapSIDs = TRUE;
    }
	
    sids_found = gSIDs;

    log_message(LOG_DEFAULT, "Device driver based CatWeasel SID: opened, found %d SIDs.", gSIDs);

    return 0;
}

int cw_device_close(void)
{
    int i, j;

    /* mute all sids */
    for (j = 0; j < sids_found; ++j) {
        for (i = 0; i < 32; ++i) {
            write_sid(i, 0, j);
        }
    }

    close_device();

    log_message(LOG_DEFAULT, "Device driver based CatWeasel SID: closed.");

    sids_found = -1;

    return 0;
}

static unsigned char read_sid(unsigned char reg, int chipno)
{
    unsigned char tData[2];

    if (gSwapSIDs) {
        chipno = 1 - chipno;
    }

    if (gCatweaselReq[chipno] == NULL) {
        return 0;
    }

    tData[0] = reg;
    gCatweaselReq[chipno]->iotd_Req.io_Length = 2;
    gCatweaselReq[chipno]->iotd_Req.io_Command = CMD_READ;
    gCatweaselReq[chipno]->iotd_Req.io_Data = tData;
    gCatweaselReq[chipno]->iotd_Req.io_Offset = 0;

    DoIO((struct IORequest *)gCatweaselReq[chipno]);

    return ((unsigned char*)(gCatweaselReq[chipno]->iotd_Req.io_Data))[1];
}

static void write_sid(unsigned char reg, unsigned char data, int chipno)
{
    unsigned char tData[2];

    if (gSwapSIDs) {
        chipno = 1 - chipno;
    }

    if (gCatweaselReq[chipno] == NULL) {
        return;
    }

    tData[0] = reg;
    tData[1] = data;
    gCatweaselReq[chipno]->iotd_Req.io_Length = 2;
    gCatweaselReq[chipno]->iotd_Req.io_Command = CMD_WRITE;
    gCatweaselReq[chipno]->iotd_Req.io_Data = tData;
    gCatweaselReq[chipno]->iotd_Req.io_Offset = 0;

    DoIO((struct IORequest *)gCatweaselReq[chipno]);
}

/* set current main clock frequency, which gives us the possibilty to
   choose between pal and ntsc frequencies */
void cw_device_set_machine_parameter(long cycles_per_sec)
{
    int i;

    for (i = 0; i < 2; i++) {
        if (gCatweaselReq[i] != NULL) {
            gCatweaselReq[i]->iotd_Req.io_Length = 1;
            gCatweaselReq[i]->iotd_Req.io_Command = TD_FORMAT;
            gCatweaselReq[i]->iotd_Req.io_Offset = 0;
            if (cycles_per_sec <= 1000000) {
                gCatweaselReq[i]->iotd_Req.io_Data = (void *)0;  // PAL
            } else {
                gCatweaselReq[i]->iotd_Req.io_Data = (void *)1;  // NTSC
            }
            DoIO((struct IORequest *)gCatweaselReq[i]);
        }
    }
}

int cw_device_available(void)
{
    return sids_found;
}
#endif

