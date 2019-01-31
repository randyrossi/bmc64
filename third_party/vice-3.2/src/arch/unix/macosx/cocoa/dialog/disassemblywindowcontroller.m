/*
 * disassemblywindowcontroller.m - disassembly window controller
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

#include <string.h>
#include "vice.h"
#include "charset.h"

#import "disassemblywindowcontroller.h"
#import "viceapplication.h"

#define VICEDisassembleRealizeExtendNotification @"VICEDisassembleRealizeExtendNotification"

@implementation DisassemblyWindowController

-(id)initWithMemSpace:(int)space
{
    self = [super initWithWindowNibName:@"DisassemblyWindow" title:@"Disassembly" memSpace:space];
    return self;
}

-(void)dealloc
{    
    [lines release];    
    [super dealloc];
}

-(void)windowDidLoad
{
    [super windowDidLoad];
 
    // listen to my own extend messages
    [[NSNotificationCenter defaultCenter] addObserver:self
                                              selector:@selector(realizeExtend:)
                                                  name:VICEDisassembleRealizeExtendNotification
                                                object:nil];

    // setup context menu
    NSMenu *theMenu = [[[NSMenu alloc] initWithTitle:@"Disassembly Menu"] autorelease];
    [theMenu setDelegate:self];
    [theMenu setAutoenablesItems:NO];
    setBreakpointItem = [theMenu addItemWithTitle:@"Activate Breakpoint" action:@selector(contextMenuCall:) keyEquivalent:@""];
    enableBreakpointItem = [theMenu addItemWithTitle:@"Enable Breakpoint" action:@selector(contextMenuCall:) keyEquivalent:@""];
    [memoryTable setMenu:theMenu];                                        
}

-(int)getAddrForSelection
{
    int row = [memoryTable clickedRow];
    if(row == -1) {
        return -1;
    }
    // retrieve address from disassembly's first columns
    NSDictionary *entry = [lines objectAtIndex:row];
    NSString *contents = [entry objectForKey:@"contents"];
    NSString *addrString = [contents substringToIndex:4];
    int addr;
    sscanf([addrString cStringUsingEncoding:NSUTF8StringEncoding],"%x",&addr);
    return addr;
}

-(void)menuNeedsUpdate:(NSMenu *)menu
{
    // check if address is available
    int addr = [self getAddrForSelection];
    BOOL setOn = NO;
    BOOL enableOn = NO;
    BOOL setText = YES;
    BOOL enableText = YES;
    
    if(addr != -1) {
        int state = [[VICEApplication theMachineController] getBreakpointState:(int)memSpace addr:(int)addr];
        if(state == 0) { // NONE
            setOn = YES;
        }
        else if(state == 1) { // INACTIVATE
            setOn = YES;
            enableOn = YES;
            setText = NO;
        }
        else { // ACTIVE
            setOn = YES;
            enableOn = YES;
            setText = NO;
            enableText = NO;
        }
    }

    [setBreakpointItem setTitle:setText ? @"Set Breakpoint" : @"Unset Breakpoint"];
    [setBreakpointItem setEnabled:setOn];
    [enableBreakpointItem setTitle:enableText ? @"Enable Breakpoint" : @"Disable Breakpoint"];
    [enableBreakpointItem setEnabled:enableOn];
}

-(void)updateRow:(int)row addr:(int)addr
{
    // read single line
    NSDictionary *disData = [[VICEApplication theMachineController] disassembleMemory:memSpace lines:1 start:addr];
    NSArray *myLinesData = [disData objectForKey:@"lines"];
    id line = [myLinesData objectAtIndex:0];
    [line retain];
    
    id oldLine = [lines objectAtIndex:row];
    [oldLine release];
    [lines replaceObjectAtIndex:row withObject:line];
}

-(void)contextMenuCall:(id)sender
{
    int addr = [self getAddrForSelection];
    if(addr == -1) {
        return;
    }
    
    // alter breakpoint
    if((sender == setBreakpointItem)||(sender == enableBreakpointItem)) {
        [[VICEApplication theMachineController] alterBreakpoint:memSpace addr:addr set:(sender==setBreakpointItem)];
        [self updateRow:[memoryTable clickedRow] addr:addr];
    }    
}

-(NSArray *)disassembleRangeWithLines:(int)myLines start:(int)address
{
    int i,j;
    
    // read dis data at start address
    NSDictionary *disData = [[VICEApplication theMachineController] disassembleMemory:memSpace lines:myLines start:address];

     // get address range
    NSNumber *startNumber = [disData objectForKey:@"start"];
    NSNumber *endNumber = [disData objectForKey:@"end"];
    int start = [startNumber intValue];
    int end = [endNumber intValue];

     // add line contents
    NSArray *myLinesData = [disData objectForKey:@"lines"];
    
    // adjust range
    BOOL change = false;
    if(end < start) {
        end = MAX_SIZE-1;
    }
    if(start < rangeMinAddr) {
        rangeMinAddr = start;
        change = true;
    }
    if(end > rangeMaxAddr) {
        rangeMaxAddr = end;
        change = true;
    }
    if(!change) {
        return nil;
    }
    
    //NSLog(@"range min %04x max %04x",rangeMinAddr,rangeMaxAddr);
    
    [myLinesData retain];
    return myLinesData;
}

-(int)extendBefore
{
    // nothing to extend
    if(rangeMinAddr == 0) {
        return 0;
    }
    
    // scroll up until address is reached and count lines
    int begin = [[[VICEApplication theMachineController] disassembleSeek:memSpace up:TRUE lines:LINES_PER_CALL start:rangeMinAddr] intValue];
    if(begin == rangeMinAddr) {
        return 0;
    }
    
    NSArray *newLines = [self disassembleRangeWithLines:LINES_PER_CALL start:begin];
    if(newLines == nil) {
        return 0;
    }
    NSArray *oldLines = lines;
    int newCount = [newLines count];
        
    NSMutableArray *myLines = [[NSMutableArray alloc] init];
    [myLines addObjectsFromArray:newLines];
    [myLines addObjectsFromArray:oldLines];
    [newLines release];
    [oldLines release];
    lines = myLines;
    
    return newCount;
}

-(int)extendAfter
{
    // nothing to extend
    if(rangeMaxAddr >= (MAX_SIZE-1)) {
        return 0;
    }
    
    // next lines
    NSArray *newLines = [self disassembleRangeWithLines:LINES_PER_CALL start:rangeMaxAddr+1];
    if(newLines == nil) {
        return 0;
    }
    NSArray *oldLines = lines;
    int newCount = [newLines count];
        
    NSMutableArray *myLines = [[NSMutableArray alloc] init];
    [myLines addObjectsFromArray:oldLines];
    [myLines addObjectsFromArray:newLines];
    [newLines release];
    [oldLines release];
    lines = myLines;
    
    return newCount;    
}

-(int)resetToAddress:(int)address
{
    // disassemble block beginning with address
    rangeMinAddr = MAX_SIZE+1;
    rangeMaxAddr = -1;
    lines = [[NSMutableArray alloc] init];
    NSArray *newLines = [self disassembleRangeWithLines:LINES_PER_CALL start:address];
    [lines addObjectsFromArray:newLines];
    
    // give some room above
    int pos = [self extendBefore];
    
    return pos;
} 

-(void)monitorInitDone:(NSNotification *)notification
{   
    [memoryTable setDataSource:self];
}

-(void)monitorUpdate:(NSNotification *)notification
{
    int pos = [self resetToAddress:-1];
    [memoryTable reloadData];
    [memoryTable scrollRowToVisible:pos-5];
    [memoryTable scrollRowToVisible:pos+10];
}

// the table needs to be extended to reach the given address
-(void)realizeExtend:(NSNotification *)notification
{
    int row = [[[notification userInfo] objectForKey:@"row"] intValue];
    //NSLog(@"extend %d", row);

    if(row > 0) {
        [self extendAfter];
    } else {
        [self extendBefore];
    }

    // update table
    [memoryTable reloadData];
}

@end

@implementation DisassemblyWindowController (NSTableViewDataSource)

- (id)tableView:(NSTableView *)aTableView
    objectValueForTableColumn:(NSTableColumn *)aTableColumn
    row:(int)rowIndex
{
    id theValue = nil;
    
    // check line parameter
    NSParameterAssert(rowIndex >= 0 && rowIndex < [lines count]);

    // contents of disassembly
    NSString *colId = [aTableColumn identifier];
    if([colId compare:@"contents"] == NSOrderedSame) {

        NSDictionary *entry = [lines objectAtIndex:rowIndex];
        
        // ah trigger automatical extend
        if((rowIndex == 0) || (rowIndex == [lines count]-1)) {
            NSDictionary * dict =
                 [NSDictionary dictionaryWithObject:[NSNumber numberWithInt:rowIndex]
                                             forKey:@"row"];
            [[NSNotificationCenter defaultCenter] postNotificationName:VICEDisassembleRealizeExtendNotification
                                                                object:self
                                                              userInfo:dict];
        }

        // read existing value
        theValue = [entry objectForKey:@"contents"];
        
        // colorize:
        // active line
        bool active_line = [[entry objectForKey:@"active_line"] boolValue];
        if(active_line) {
            theValue = [[NSMutableAttributedString alloc] initWithString:theValue];
            [theValue addAttribute:NSForegroundColorAttributeName
                             value:[NSColor blueColor]
                             range:NSMakeRange(0, [theValue length])];
        }
        else {
            // active breakpoint
            bool breakpoint_active = [[entry objectForKey:@"breakpoint_active"] boolValue];
            if(breakpoint_active) {
                theValue = [[NSMutableAttributedString alloc] initWithString:theValue];
                [theValue addAttribute:NSForegroundColorAttributeName
                                 value:[NSColor redColor]
                                 range:NSMakeRange(0, [theValue length])];
            }
            else {
                // is breakpoint
                bool is_breakpoint = [[entry objectForKey:@"is_breakpoint"] boolValue];
                if(is_breakpoint) {
                    theValue = [[NSMutableAttributedString alloc] initWithString:theValue];
                    [theValue addAttribute:NSForegroundColorAttributeName
                                     value:[NSColor brownColor]
                                     range:NSMakeRange(0, [theValue length])];
                }
            }
        }
    }
    
    return theValue;
}

- (int)numberOfRowsInTableView:(NSTableView *)aTableView
{
    return [lines count];
}

@end
