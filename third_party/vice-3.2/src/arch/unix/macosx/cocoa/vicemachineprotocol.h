/*
 * vicemachineprotocol.h - VICEMachineProtocol - protocol of VICEMachine
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

#import "vicemachinecontroller.h"

/* The VICEMachineProtocol is the object interface that is offered by the
   machine thread in VICE for usage by the main UI thread.

   The VICEMachineProtocol is implemented by the VICEMachine class.
*/

@protocol VICEMachineProtocol

// tell the running VICE machine to shutdown gracefully and stop its thread
-(void)stopMachine;

// ----- Pause -----
// is the machine paused?
-(BOOL)isPaused;
// toggle pause
-(BOOL)togglePause;
-(void)setPaused:(int)paused;

// ----- Monitor Input -----
// does the machine wait for a monitor input?
-(BOOL)isWaitingForLineInput;
// submit an line input from the UI while the machine is waiting for input
-(void)submitLineInput:(NSString *)line;

// ----- Controller Invocation -----
// access the controller of the machine
-(VICEMachineController *)getMachineController;

@end

