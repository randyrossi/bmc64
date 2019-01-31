/*
 * registerwindowcontroller.m - register window controller
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

#include "vice.h"

#import "registerwindowcontroller.h"
#import "viceapplication.h"

@implementation RegisterWindowController

-(id)initWithMemSpace:(int)space
{
    self = [super initWithWindowNibName:@"RegisterWindow" title:@"Registers" memSpace:space];
    return self;
}

-(void)dealloc
{
    [lastRegisters release];
    [registers release];
    [super dealloc];
}

-(void)windowDidLoad
{
    [super windowDidLoad];
#ifdef MÓNITOR_DEBUG
    NSLog(@"reg: -> set data source");
#endif
    [registerTable setDataSource:self];
 }

-(void)readRegisters
{
    if(lastRegisters) {
        [lastRegisters release];
    }
    lastRegisters = registers;
    registers = [[VICEApplication theMachineController] getRegisters:memSpace];
    [registers retain];
    
    if(lastRegisters != nil) {
        cpuChanged = ([lastRegisters count] != [registers count]);
    } else {
        cpuChanged = YES;
    }
}

-(void)monitorInitDone:(NSNotification *)notification
{
    [super monitorInitDone:notification];    
}

-(void)monitorUpdate:(NSNotification *)notification
{
    [super monitorUpdate:notification];

#ifdef MONITOR_DEBUG
    NSLog(@"reg: -> reload data");
#endif
    [self readRegisters];
    [registerTable reloadData];
}

@end

@implementation RegisterWindowController (NSTableViewDataSource)

- (id)tableView:(NSTableView *)aTableView
    objectValueForTableColumn:(NSTableColumn *)aTableColumn
    row:(int)rowIndex
{
    id theRecord;

    NSParameterAssert(rowIndex >= 0 && rowIndex < [registers count]);
    theRecord = [registers objectAtIndex:rowIndex];

    NSString *colId = [aTableColumn identifier];
    
    id theValue = [theRecord objectForKey:colId];
    if([colId compare:@"value"] == NSOrderedSame) {

        // format value
        NSNumber *flagsNumber = [theRecord objectForKey:@"flags"];
        unsigned int flags = [flagsNumber unsignedIntValue];
        NSNumber *sizeNumber = [theRecord objectForKey:@"size"];
        unsigned int size = [sizeNumber unsignedIntValue];        
        NSNumber *valueNumber = (NSNumber *)theValue;     
        unsigned int value = [valueNumber unsignedIntValue];
        
        // fetch old value for comparison
        BOOL same = YES;
        if(lastRegisters && !cpuChanged) {
            id oldRecord = [lastRegisters objectAtIndex:rowIndex];
            NSNumber *oldValueNumber = [oldRecord objectForKey:colId];
            unsigned int oldValue = [oldValueNumber unsignedIntValue];
            same = (oldValue == value);
        }
        
        // format value
        if((flags & 1) == 1)  {
            // requested binary output
            theValue = [DebuggerWindowController toBinaryString:value width:8];
        } else {
            if(size == 8) {
                theValue = [NSString stringWithFormat:@"%02X",value];
            } else {
                theValue = [NSString stringWithFormat:@"%04X",value];
            }
        }
        
        // make bold if value changed
        if(!same) {
            theValue = [[NSMutableAttributedString alloc] initWithString:theValue];
            [theValue addAttribute:NSForegroundColorAttributeName
                        value:[NSColor blueColor]
                        range:NSMakeRange(0, [theValue length])];
        }
    }
    
    return theValue;

}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
    if(registers) {
        int num = [registers count];
        return num;
    } else {
        return 0;
    }
}

@end
