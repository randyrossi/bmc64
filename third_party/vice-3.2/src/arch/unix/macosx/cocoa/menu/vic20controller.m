/*
 * vic20controller.m - VIC20 app controller
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

#include "cartridge.h"

#import "vic20controller.h"
#import "viceapplication.h"
#import "vic20machinecontroller.h"

@implementation VIC20Controller

// ----- Cartridge -----

-(IBAction)attachCartridge:(id)sender
{
    NSDictionary *cartTypes = [NSDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithInt:CARTRIDGE_VIC20_GENERIC], @"Generic",
        [NSNumber numberWithInt:CARTRIDGE_VIC20_BEHRBONZ], @"Behr Bonz",
        [NSNumber numberWithInt:CARTRIDGE_VIC20_FP], @"Vic Flash Plugin",
        [NSNumber numberWithInt:CARTRIDGE_VIC20_MEGACART], @"Mega-Cart",
        [NSNumber numberWithInt:CARTRIDGE_VIC20_FINAL_EXPANSION], @"Final Expansion",

        [NSNumber numberWithInt:CARTRIDGE_VIC20_DETECT], @"+ Smart Attach",
        [NSNumber numberWithInt:CARTRIDGE_VIC20_16KB_2000], @"+ Attach 4/8/16kB image at $2000",
        [NSNumber numberWithInt:CARTRIDGE_VIC20_16KB_4000], @"+ Attach 4/8/16kB image at $4000",
        [NSNumber numberWithInt:CARTRIDGE_VIC20_16KB_6000], @"+ Attach 4/8/16kB image at $6000",
        [NSNumber numberWithInt:CARTRIDGE_VIC20_8KB_A000], @"+ Attach 4/8kB image at $A000",
        [NSNumber numberWithInt:CARTRIDGE_VIC20_4KB_B000], @"+ Attach 4kB image at $B000",
        
        nil, nil];

    NSArray *result = [[self getFilePanel] pickAttachFileWithTitle:@"Attach Cartridge Image"
                                                 andTypeDictionary:cartTypes
                                                 defaultType:@"Generic"];
    if(result != nil) {
        NSString *filename   = [result objectAtIndex:0];
        NSNumber *typeNumber = [result objectAtIndex:1];
        int type = [typeNumber intValue];
        
        if ([(VIC20MachineController *)[VICEApplication theMachineController] 
                                      attachCartridge:type 
                                                image:filename]) {
            [(VIC20MachineController *)[VICEApplication theMachineController] 
                setDefaultCartridge];                
            [self updateMachineResources];
        } else {
            [VICEApplication runErrorMessage:@"Error attaching image!"];
        }        
    }
}

-(IBAction)detachCartridge:(id)sender
{
    [(VIC20MachineController *)[VICEApplication theMachineController]
        detachCartridge:-1];
    [(VIC20MachineController *)[VICEApplication theMachineController]
        setDefaultCartridge];
    [self updateMachineResources];
}

@end
