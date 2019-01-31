/*
 * iotreewindowcontroller.m - io tree window controller
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

#include "iotreewindowcontroller.h"

@implementation IOTreeWindowController

 -(id)initWithMemSpace:(int)space
 {
    self = [super initWithWindowNibName:@"IOTreeWindow" title:@"IO Tree" memSpace:space];
    if (!self) {
        return self;
    }

    // read in io tree plist
    NSDictionary *treeDict = [[NSDictionary alloc] initWithContentsOfFile:
        [[NSBundle mainBundle] pathForResource:@"IOTree" ofType:@"plist" ] ];

    // create io tree items from plist dictionary
    rootItem = [[IOTreeItem alloc] initWithTitle:@"Root"];
    BOOL ok = [rootItem addFromDictionary:treeDict];
    if (!ok) {
        NSLog(@"ERROR building IOTree!");
    }
    
    [treeDict release];

    return self;
}

-(void)dealloc
{
    [rootItem release];
    [super dealloc];
}

-(void)monitorInitDone:(NSNotification *)notification
{   
    [treeView setDelegate:self];
    [treeView setDataSource:self];
}

-(void)monitorUpdate:(NSNotification *)notification
{
    [rootItem readMemory:memSpace];
    [treeView reloadData];
}

@end

@implementation IOTreeWindowController (NSOutlineViewDataSource)

- (int)outlineView:(NSOutlineView *)outlineView
       numberOfChildrenOfItem:(id)item
{
    if (item == nil) {
        return [rootItem numChildren];
    } else {
        return [(IOTreeItem *)item numChildren];
    }
}

- (BOOL)outlineView:(NSOutlineView *)outlineView
        isItemExpandable:(id)item
{
    if (item == nil) {
        return TRUE;
    }
    IOTreeItem *ritem = (IOTreeItem *)item;
    return ![ritem isLeaf];
}

- (id)outlineView:(NSOutlineView *)outlineView
      child:(int)index 
      ofItem:(id)item 
{
    IOTreeItem *ritem;
    if (item == nil) {
        ritem = rootItem;
    } else {
        ritem = (IOTreeItem *)item;
    }
    int max = [ritem numChildren];
    if (index >= max) {
        return nil;
    }
    return [ritem childAtIndex:index];
}

- (id)outlineView:(NSOutlineView *)outlineView
      objectValueForTableColumn:(NSTableColumn *)tableColumn
      byItem:(id)item
{
    if (item == nil) {
        return nil;
    }

    IOTreeItem *ritem = (IOTreeItem *)item;
    NSString *colId = (NSString *)[tableColumn identifier];
    if ([colId isEqualToString:@"Resource"]) {
        return [ritem title];
    } else if ([colId isEqualToString:@"Value"]) {
        return [ritem getValue];
    } else {
        return [ritem getRegisterValue];
    }
}

@end
