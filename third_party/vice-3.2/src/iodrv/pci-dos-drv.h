/*
 * pci-dos-drv.h - MSDOS specific PCI access functions.
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

#ifndef VICE_PCI_DOS_DRV_H
#define VICE_PCI_DOS_DRV_H

typedef unsigned short uint16;
typedef unsigned long uint32;

extern int vice_pci_install_check(void);
extern int vice_pci_find(int vendorID, int deviceID, int index, int *bus, int *device, int *func);
extern vice_pci_read_config_dword(int bus, int device, int func, int reg, uint32_t *value);

#endif
