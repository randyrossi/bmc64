/*
 * cbm2ui.m - CBM-II ui interface
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
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

#import "cbm2controller.h"
#import "cbm2machinecontroller.h"
#import "vicemachine.h"

int cbm2ui_init(void)
{
    // ensure that the CBM2Controller gets compiled in
    [CBM2Controller class];
    [theVICEMachine setMachineController:[[CBM2MachineController alloc] init]];
    return 0;
}

void cbm2ui_shutdown(void)
{
}
