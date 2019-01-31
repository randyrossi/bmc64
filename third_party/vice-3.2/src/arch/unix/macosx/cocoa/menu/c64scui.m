/*
 * c64scui.m - C64SC ui interface
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

#import "c64commoncontroller.h"
#import "c64commonmachinecontroller.h"
#import "vicemachine.h"

int c64scui_init(void)
{
    // ensure that the C64Controller gets compiled in
    [C64CommonController class];
    [theVICEMachine setMachineController:[[C64CommonMachineController alloc] init]];
    return 0;
}

void c64scui_shutdown(void)
{
}
