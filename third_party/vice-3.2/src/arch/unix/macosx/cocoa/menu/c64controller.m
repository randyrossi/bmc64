/*
 * c64controller.m - C64 app controller
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

#import "c64controller.h"
#import "viceapplication.h"
#import "c64machinecontroller.h"

@implementation C64Controller

// ----- Cartridge -----

-(IBAction)attachCartridge:(id)sender
{
    NSDictionary *cartTypes = [NSDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithInt:CARTRIDGE_CRT], @"CRT",
        
        [NSNumber numberWithInt:CARTRIDGE_GENERIC_8KB], @"Generic 8KB",
        [NSNumber numberWithInt:CARTRIDGE_GENERIC_16KB], @"Generic 16KB",
        [NSNumber numberWithInt:CARTRIDGE_ULTIMAX], @"Ultimax",
        
        [NSNumber numberWithInt:CARTRIDGE_ACTION_REPLAY], @ CARTRIDGE_NAME_ACTION_REPLAY,
        [NSNumber numberWithInt:CARTRIDGE_ACTION_REPLAY2], @ CARTRIDGE_NAME_ACTION_REPLAY2,
        [NSNumber numberWithInt:CARTRIDGE_ACTION_REPLAY3], @ CARTRIDGE_NAME_ACTION_REPLAY3,
        [NSNumber numberWithInt:CARTRIDGE_ACTION_REPLAY4], @ CARTRIDGE_NAME_ACTION_REPLAY4,    
        [NSNumber numberWithInt:CARTRIDGE_ATOMIC_POWER], @ CARTRIDGE_NAME_ATOMIC_POWER,

        [NSNumber numberWithInt:CARTRIDGE_CAPTURE], @ CARTRIDGE_NAME_CAPTURE,
        [NSNumber numberWithInt:CARTRIDGE_COMAL80], @ CARTRIDGE_NAME_COMAL80,
        
        [NSNumber numberWithInt:CARTRIDGE_DIASHOW_MAKER], @ CARTRIDGE_NAME_DIASHOW_MAKER,
        [NSNumber numberWithInt:CARTRIDGE_DINAMIC], @ CARTRIDGE_NAME_DINAMIC,
        
        [NSNumber numberWithInt:CARTRIDGE_EASYFLASH], @ CARTRIDGE_NAME_EASYFLASH,
        [NSNumber numberWithInt:CARTRIDGE_EPYX_FASTLOAD], @ CARTRIDGE_NAME_EPYX_FASTLOAD,    
        [NSNumber numberWithInt:CARTRIDGE_EXOS], @ CARTRIDGE_NAME_EXOS,
        [NSNumber numberWithInt:CARTRIDGE_EXPERT], @ CARTRIDGE_NAME_EXPERT,
        
        [NSNumber numberWithInt:CARTRIDGE_FINAL_I], @ CARTRIDGE_NAME_FINAL_I,
        [NSNumber numberWithInt:CARTRIDGE_FINAL_III], @ CARTRIDGE_NAME_FINAL_III,
        [NSNumber numberWithInt:CARTRIDGE_FINAL_PLUS], @ CARTRIDGE_NAME_FINAL_PLUS,
        [NSNumber numberWithInt:CARTRIDGE_FREEZE_FRAME], @ CARTRIDGE_NAME_FREEZE_FRAME,
        [NSNumber numberWithInt:CARTRIDGE_FREEZE_MACHINE], @ CARTRIDGE_NAME_FREEZE_MACHINE,
        [NSNumber numberWithInt:CARTRIDGE_FUNPLAY], @ CARTRIDGE_NAME_FUNPLAY,
        
        [NSNumber numberWithInt:CARTRIDGE_GAME_KILLER], @ CARTRIDGE_NAME_GAME_KILLER,
        [NSNumber numberWithInt:CARTRIDGE_GMOD2], @ CARTRIDGE_NAME_GMOD2,
        [NSNumber numberWithInt:CARTRIDGE_GS], @ CARTRIDGE_NAME_GS,

        [NSNumber numberWithInt:CARTRIDGE_IDE64], @ CARTRIDGE_NAME_IDE64,
        [NSNumber numberWithInt:CARTRIDGE_IEEE488], @ CARTRIDGE_NAME_IEEE488,

        [NSNumber numberWithInt:CARTRIDGE_KCS_POWER], @ CARTRIDGE_NAME_KCS_POWER,

        [NSNumber numberWithInt:CARTRIDGE_MACH5], @ CARTRIDGE_NAME_MACH5,
        [NSNumber numberWithInt:CARTRIDGE_MAGIC_DESK], @ CARTRIDGE_NAME_MAGIC_DESK,
        [NSNumber numberWithInt:CARTRIDGE_MAGIC_FORMEL], @ CARTRIDGE_NAME_MAGIC_FORMEL,
        [NSNumber numberWithInt:CARTRIDGE_MAGIC_VOICE], @ CARTRIDGE_NAME_MAGIC_VOICE,
        [NSNumber numberWithInt:CARTRIDGE_MIKRO_ASSEMBLER], @ CARTRIDGE_NAME_MIKRO_ASSEMBLER,
        [NSNumber numberWithInt:CARTRIDGE_MMC64], @ CARTRIDGE_NAME_MMC64,
        [NSNumber numberWithInt:CARTRIDGE_MMC_REPLAY], @ CARTRIDGE_NAME_MMC_REPLAY,
        
        [NSNumber numberWithInt:CARTRIDGE_OCEAN], @ CARTRIDGE_NAME_OCEAN,

        [NSNumber numberWithInt:CARTRIDGE_P64], @ CARTRIDGE_NAME_P64,
        [NSNumber numberWithInt:CARTRIDGE_RETRO_REPLAY], @ CARTRIDGE_NAME_RETRO_REPLAY,
        [NSNumber numberWithInt:CARTRIDGE_REX], @ CARTRIDGE_NAME_REX,
        [NSNumber numberWithInt:CARTRIDGE_ROSS], @ CARTRIDGE_NAME_ROSS,

        [NSNumber numberWithInt:CARTRIDGE_SIMONS_BASIC], @ CARTRIDGE_NAME_SIMONS_BASIC,
        [NSNumber numberWithInt:CARTRIDGE_SNAPSHOT64], @ CARTRIDGE_NAME_SNAPSHOT64,
        [NSNumber numberWithInt:CARTRIDGE_STARDOS], @ CARTRIDGE_NAME_STARDOS,
        [NSNumber numberWithInt:CARTRIDGE_STRUCTURED_BASIC], @ CARTRIDGE_NAME_STRUCTURED_BASIC,
        [NSNumber numberWithInt:CARTRIDGE_SUPER_EXPLODE_V5], @ CARTRIDGE_NAME_SUPER_EXPLODE_V5,
        [NSNumber numberWithInt:CARTRIDGE_SUPER_GAMES], @ CARTRIDGE_NAME_SUPER_GAMES,
        [NSNumber numberWithInt:CARTRIDGE_SUPER_SNAPSHOT], @ CARTRIDGE_NAME_SUPER_SNAPSHOT,
        [NSNumber numberWithInt:CARTRIDGE_SUPER_SNAPSHOT_V5], @ CARTRIDGE_NAME_SUPER_SNAPSHOT_V5,

        [NSNumber numberWithInt:CARTRIDGE_WARPSPEED], @ CARTRIDGE_NAME_WARPSPEED,
        [NSNumber numberWithInt:CARTRIDGE_WESTERMANN], @ CARTRIDGE_NAME_WESTERMANN,

        [NSNumber numberWithInt:CARTRIDGE_ZAXXON], @ CARTRIDGE_NAME_ZAXXON,
        
        nil, nil];

    NSArray *result = [[self getFilePanel] pickAttachFileWithTitle:@"Attach Cartridge Image"
                                                 andTypeDictionary:cartTypes
                                                 defaultType:@"CRT"];
    if(result != nil) {
        NSString *filename   = [result objectAtIndex:0];
        NSNumber *typeNumber = [result objectAtIndex:1];
        int type = [typeNumber intValue];
        
        if ([(C64MachineController *)[VICEApplication theMachineController] 
                                      attachCartridge:type 
                                                image:filename]) {
            [(C64MachineController *)[VICEApplication theMachineController] 
                setDefaultCartridge];                
            [self updateMachineResources];
        } else {
            [VICEApplication runErrorMessage:@"Error attaching image!"];
        }        
    }
}

-(IBAction)detachCartridge:(id)sender
{
    [(C64MachineController *)[VICEApplication theMachineController]
        detachCartridge:-1];
    [(C64MachineController *)[VICEApplication theMachineController]
        setDefaultCartridge];
    [self updateMachineResources];
}

-(IBAction)freezeCartridge:(id)sender
{
    [(C64MachineController *)[VICEApplication theMachineController]
        freezeCartridge];
}

-  (BOOL)validateMenuItem:(NSMenuItem *)item
{
    if (item==cartridgeFreezeMenuItem) {
        return [self getIntResource:@"CartridgeType"] != CARTRIDGE_NONE;
    }
    return YES;
}

@end
