/*
 * cw-amiga-os4.c - AmigaOS4 catweasel driver.
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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

#if defined(HAVE_CATWEASELMKIII) && defined(AMIGA_OS4)

#include <stdlib.h>
#include <string.h>

#include "cw-amiga.h"
#include "log.h"
#include "types.h"

static unsigned char read_sid(unsigned char reg); /* Read a SID register */
static void write_sid(unsigned char reg, unsigned char data); /* Write a SID register */

#define MAXSID 1

static int sids_found = -1;

/* read value from SIDs */
int cw_os4_read(uint16_t addr, int chipno)
{
    /* check if chipno and addr is valid */
    if (chipno < MAXSID && addr < 0x20) {
        return read_sid(addr);
    }

    return 0;
}

/* write value into SID */
void cw_os4_store(uint16_t addr, uint8_t val, int chipno)
{
    /* check if chipno and addr is valid */
    if (chipno < MAXSID && addr < 0x20) {
        write_sid(addr, val);
    }
}

/*
** How to use the SID in a CWMk3 under OS4...
** By Peter Gordon (pete@shagged.org)
**
*/

#define CW_SID_DAT 0xd8
#define CW_SID_CMD 0xdc

#undef BYTE
#undef WORD
#include <exec/types.h>
#include <proto/expansion.h>
#include <proto/exec.h>

static struct PCIIFace *IPCI = NULL;

static struct PCIDevice *CWDevPCI = NULL;
static struct PCIResourceRange *CWDevBAR = NULL;
int CWLock = FALSE;

static void close_device(void)
{
    if (CWDevBAR) {
        CWDevPCI->FreeResourceRange(CWDevBAR);
    }
    if (CWLock) {
        CWDevPCI->Unlock();
    }
    if (IPCI) {
        IExec->DropInterface((struct Interface *)IPCI);
    }
}

int cw_os4_open(void)
{
    unsigned int i;

    if (!sids_found) {
        return -1;
    }

    if (sids_found > 0) {
        return 0;
    }

    sids_found = 0;

    log_message(LOG_DEFAULT, "Detecting PCI CatWeasel boards.");

    IPCI = (struct PCIIFace *)IExec->GetInterface(ExpansionBase, "pci", 1, NULL);
    if (!IPCI) {
        log_message(LOG_DEFAULT, "Unable to obtain PCI expansion interface.");
        return -1;
    }

    /* Try and find a CW on the PCI bus */
    CWDevPCI = IPCI->FindDeviceTags(FDT_VendorID, CW_VENDOR,
                                    FDT_DeviceID, CW_DEVICE,
                                    FDT_Index, 0,
                                    TAG_DONE);
    if (!CWDevPCI) {
        log_message(LOG_DEFAULT, "Unable to find a PCI CatWeasel board.");
        close_device();
        return -1;
    }

    /* Lock the device, since we're a driver */
    CWLock = CWDevPCI->Lock(PCI_LOCK_SHARED);
    if (!CWLock) {
        log_message(LOG_DEFAULT, "Unable to lock the CatWeasel. Another driver may have an exclusive lock." );
        close_device();
        return -1;
    }

    /* Get the resource range */
    CWDevBAR = CWDevPCI->GetResourceRange(0);
    if (!CWDevBAR) {
        log_message(LOG_DEFAULT, "Unable to get CatWeasel resource range 0." );
        close_device();
        return -1;
    }

    /* Reset the catweasel PCI interface (as per the CW programming docs) */
    CWDevPCI->OutByte(CWDevBAR->BaseAddress + 0x00, 0xf1);
    CWDevPCI->OutByte(CWDevBAR->BaseAddress + 0x01, 0x00);
    CWDevPCI->OutByte(CWDevBAR->BaseAddress + 0x02, 0x00);
    CWDevPCI->OutByte(CWDevBAR->BaseAddress + 0x04, 0x00);
    CWDevPCI->OutByte(CWDevBAR->BaseAddress + 0x05, 0x00);
    CWDevPCI->OutByte(CWDevBAR->BaseAddress + 0x29, 0x00);
    CWDevPCI->OutByte(CWDevBAR->BaseAddress + 0x2b, 0x00);

    /* mute all sids */
    for (i = 0; i < 32; i++) {
        write_sid(i, 0);
    }

    log_message(LOG_DEFAULT, "PCI CatWeasel SID: opened at $%X.", CWDevBAR->BaseAddress);

    sids_found = 1;

    return 0;
}

int cw_os4_close(void)
{
    unsigned int i;

    for (i = 0; i < 32; i++) {
        write_sid(i, 0);
    }

    close_device();

    log_message(LOG_DEFAULT, "PCI CatWeasel SID: closed.");

    sids_found = -1;

    return 0;
}

static unsigned char read_sid(unsigned char reg)
{
    unsigned char cmd;

    cmd = (reg & 0x1f) | 0x20;	/* Read command & address */
    if (catweaselmkiii_get_ntsc()) {
        cmd |= 0x40;  /* Make sure its correct frequency */
    }

    /* Write command to the SID */
    CWDevPCI->OutByte(CWDevBAR->BaseAddress + CW_SID_CMD, cmd);

    /* Waste 1ms */
    CWDevPCI->InByte(CWDevBAR->BaseAddress + CW_SID_DAT);
    CWDevPCI->InByte(CWDevBAR->BaseAddress + CW_SID_DAT);

    return CWDevPCI->InByte(CWDevBAR->BaseAddress + CW_SID_DAT);
}

static void write_sid(unsigned char reg, unsigned char data)
{
    unsigned char cmd;

    cmd = reg & 0x1f;
    if (catweaselmkiii_get_ntsc()) {
        cmd |= 0x40;  /* Make sure its correct frequency */
    }

    /* Write data to the SID */
    CWDevPCI->OutByte(CWDevBAR->BaseAddress + CW_SID_DAT, data);
    CWDevPCI->OutByte(CWDevBAR->BaseAddress + CW_SID_CMD, cmd);

    /* Waste 1ms */
    CWDevPCI->InByte(CWDevBAR->BaseAddress + CW_SID_DAT);
    CWDevPCI->InByte(CWDevBAR->BaseAddress + CW_SID_DAT);
}

int cw_os4_available(void)
{
    return sids_found;
}
#endif
#endif

