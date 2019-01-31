/*
 * memorywindowcontroller.m - memory window controller
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
#include "charset.h"

#import "memorywindowcontroller.h"
#import "viceapplication.h"

#define TOTAL_MEM   65536
#define PER_LINE    16
#define TOTAL_LINES (TOTAL_MEM / PER_LINE)

@implementation MemoryWindowController

-(id)initWithMemSpace:(int)space
{
    self = [super initWithWindowNibName:@"MemoryWindow" title:@"Memory" memSpace:space];
    return self;
}

-(void)dealloc
{
    [data release];
    [lastData release];
    [memoryBankName release];
    [windowBaseTitle release];
    [super dealloc];
}

-(void)windowDidLoad
{
    [super windowDidLoad];
 
    windowBaseTitle = [[super window] title];
    [windowBaseTitle retain];
    [memoryTable setDataSource:self];
}

-(void)readMemory
{
    // make current data the last one
    if(lastData) {
        [lastData release];
    }
    lastData = data;
    
    // fetch new mem dump
    data = [[VICEApplication theMachineController] readMemory:memSpace startAddress:0 endAddress:0xffff];
    [data retain];

    // check if its the same bank as lastData was stored
    NSString *newBankName = [[VICEApplication theMachineController] getCurrentMemoryBankName:memSpace];    
    sameBank = [newBankName compare:memoryBankName]==NSOrderedSame;
    
    // keep current name
    [memoryBankName release];
    memoryBankName = newBankName;
    [memoryBankName retain];
    
    // update window title with new bank
    if(!sameBank) {
        NSString *curTitle = [NSString stringWithFormat:@"%@ - Bank %@",windowBaseTitle, memoryBankName];
        [[self window] setTitle:curTitle];
    }
}

-(void)monitorUpdate:(NSNotification *)notification
{
    [self readMemory];
    [memoryTable reloadData];
}

- (id)markDiff:(id)input bytes:(const uint8_t *)bytes oldBytes:(const uint8_t *)oldBytes scale:(int)scale
{
    id theValue = [[NSMutableAttributedString alloc] initWithString:input];
    int i;
    int markBegin = -1;
    int total = [theValue length];
    for(i=0;i<PER_LINE;i++) {
        if(oldBytes[i]!=bytes[i]) {
            if(markBegin == -1)
                markBegin = i;
        } else {
            if(markBegin != -1) {
                int markLen = i - markBegin;
                int start = markBegin * scale;
                int len = markLen * scale;
                [theValue addAttribute:NSForegroundColorAttributeName
                            value:[NSColor blueColor]
                            range:NSMakeRange(start, len)];
                markBegin = -1;
            }
        }
    }
    if(markBegin != -1) {
        int markLen = i - markBegin;
        int start = markBegin * scale;
        int len = markLen * scale;
        [theValue addAttribute:NSForegroundColorAttributeName
                    value:[NSColor blueColor]
                    range:NSMakeRange(start, len)];
    }
    return theValue;
}

@end

@implementation MemoryWindowController (NSTableViewDataSource)

- (id)tableView:(NSTableView *)aTableView
    objectValueForTableColumn:(NSTableColumn *)aTableColumn
    row:(NSInteger)rowIndex
{
    if(data == nil)
        return nil;
    
    id theValue = nil;

    NSParameterAssert(rowIndex >= 0 && rowIndex < TOTAL_LINES);

    int address = rowIndex * PER_LINE;
    const uint8_t *bytes = (const uint8_t *)[data bytes] + address;

    NSString *colId = [aTableColumn identifier];

    // Address Column
    if([colId compare:@"address"] == NSOrderedSame) {

        theValue = [NSString stringWithFormat:@"%04X",address];

        // color address if a change in line happened
        if(sameBank) {
            const uint8_t *oldBytes = (const uint8_t *)[lastData bytes] + address;
            int i;
            BOOL differs = NO;
            for(i=0;i<PER_LINE;i++) {
                if(oldBytes[i] != bytes[i]) {
                    differs = YES;
                    break;
                }
            }
            if(differs) {
                theValue = [[NSMutableAttributedString alloc] initWithString:theValue];
                [theValue addAttribute:NSForegroundColorAttributeName
                            value:[NSColor blueColor]
                            range:NSMakeRange(0, [theValue length])];
            }
        }

    }
    // Memory Dump Column
    else if([colId compare:@"memory"] == NSOrderedSame) {

        theValue = [NSString stringWithFormat:@"%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X ",
            (int)bytes[0], (int)bytes[1], (int)bytes[2], (int)bytes[3],
            (int)bytes[4], (int)bytes[5], (int)bytes[6], (int)bytes[7], 
            (int)bytes[8], (int)bytes[9], (int)bytes[10], (int)bytes[11], 
            (int)bytes[12], (int)bytes[13], (int)bytes[14], (int)bytes[15] ];
        
        // compare data and mark differences
        if(sameBank) {
            const uint8_t *oldBytes = (const uint8_t *)[lastData bytes] + address;
            theValue = [self markDiff:theValue bytes:bytes oldBytes:oldBytes scale:3];
        }
        
    }
    // PETSCII Column
    else if([colId compare:@"petscii"] == NSOrderedSame) {
        
        uint8_t line[PER_LINE];
        int i;
        for(i=0;i<PER_LINE;i++) {
            line[i] = charset_p_toascii(bytes[i],0);
        }
        
        theValue = [[NSString alloc] initWithBytes:line length:PER_LINE encoding:NSUTF8StringEncoding];

        // compare data and mark differences
        if(sameBank) {
            const uint8_t *oldBytes = (const uint8_t *)[lastData bytes] + address;
            theValue = [self markDiff:theValue bytes:bytes oldBytes:oldBytes scale:1];
        }
        
    }
    // Screen Column
    else if([colId compare:@"screen"] == NSOrderedSame) {
        
        uint8_t line[PER_LINE];
        int i;
        for(i=0;i<PER_LINE;i++) {
            line[i] = charset_p_toascii(charset_screencode_to_petcii(bytes[i]),0);
        }
        
        theValue = [[NSString alloc] initWithBytes:line length:PER_LINE encoding:NSUTF8StringEncoding];
        
        // compare data and mark differences
        if(sameBank) {
            const uint8_t *oldBytes = (const uint8_t *)[lastData bytes] + address;
            theValue = [self markDiff:theValue bytes:bytes oldBytes:oldBytes scale:1];
        }

    }
    
    return theValue;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
    if(data == nil) {
        return 0;
    } else {
        return TOTAL_LINES;
    }
}

@end
