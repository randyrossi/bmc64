/*
 * vic20ui.m - Vic20 ui interface
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

#import "vic20controller.h"
#import "vic20machinecontroller.h"
#import "vicemachine.h"

int vic20ui_init(void)
{
    // ensure that the VIC20Controller gets compiled in
    [VIC20Controller class];
    [theVICEMachine setMachineController:[[VIC20MachineController alloc] init]];
    return 0;
}

void vic20ui_shutdown(void)
{
}
