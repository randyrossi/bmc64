/*
 * resourceeditorcontroller.m - resource editor window controller
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

#include "resourceeditorcontroller.h"

@implementation ResourceEditorController

-(id)init
{
    self = [super initWithWindowNibName:@"ResourceEditor"];
    if (!self) {
        return self;
    }

    [self registerForResourceUpdate:@selector(updateResources:)];

    // read in resource tree plist
    NSDictionary *treeDict = [[NSDictionary alloc] initWithContentsOfFile:
        [[NSBundle mainBundle] pathForResource:@"ResourceTree" ofType:@"plist" ] ];

    // create resource tree items from plist dictionary
    rootItem = [[ResourceTreeItem alloc] initWithTitle:@"Root"];
    BOOL ok = [rootItem addFromDictionary:treeDict];
    if (!ok) {
        NSLog(@"ERROR building ResourceTree!");
    }
    
    [treeDict release];

    return self;
}

-(void)dealloc
{
    [rootItem release];
    [super dealloc];
}

-(void)windowDidLoad
{
    [treeView setDelegate:self];
    [treeView setDataSource:self];
}

-(void)updateResources:(NSNotification *)notification
{
    if ([notification object] != self) {
//        NSLog(@"Invalidate Resources");
        [rootItem invalidateCache];
    }
    [treeView setNeedsDisplay:TRUE];
}

- (NSCell *)outlineView:(NSOutlineView *)outlineView 
            dataCellForTableColumn:(NSTableColumn *)tableColumn
            item:(id)item
{
    if (item == nil) {
        return nil;
    }

    NSCell *cell = [tableColumn dataCell];

    ResourceTreeItem *ritem = (ResourceTreeItem *)item;
    NSString *colId = (NSString *)[tableColumn identifier];
    if ([colId isEqualToString:@"Value"]) {
        return [ritem dataCell:cell];
    } else if ([colId isEqualToString:@"Extra"]) {
        return [ritem extraCell:cell];
    } else {
        return cell;
    }
}

-(void)reloadItem:(ResourceTreeItem *)item
{
    [treeView reloadItem:item];
}

@end

@implementation ResourceEditorController (NSOutlineViewDataSource)

- (int)outlineView:(NSOutlineView *)outlineView
       numberOfChildrenOfItem:(id)item
{
    if (item == nil) {
        return [rootItem numChildren];
    } else {
        return [(ResourceTreeItem *)item numChildren];
    }
}

- (BOOL)outlineView:(NSOutlineView *)outlineView
        isItemExpandable:(id)item
{
    if (item == nil) {
        return TRUE;
    }
    ResourceTreeItem *ritem = (ResourceTreeItem *)item;
    return ![ritem isLeaf];
}

- (id)outlineView:(NSOutlineView *)outlineView
      child:(int)index 
      ofItem:(id)item 
{
    ResourceTreeItem *ritem;
    if (item == nil) {
        ritem = rootItem;
    } else {
        ritem = (ResourceTreeItem *)item;
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

    ResourceTreeItem *ritem = (ResourceTreeItem *)item;
    NSString *colId = (NSString *)[tableColumn identifier];
    if ([colId isEqualToString:@"Resource"]) {
        return [ritem title];
    } else if ([colId isEqualToString:@"Value"]) {
        return [ritem getValue:self];
    } else {
        return @"";
    }
}

- (void)outlineView:(NSOutlineView *)outlineView 
        setObjectValue:(id)object
        forTableColumn:(NSTableColumn *)tableColumn
        byItem:(id)item
{
    if (item == nil) {
        return;
    }
    ResourceTreeItem *ritem = (ResourceTreeItem *)item;
    NSString *colId = (NSString *)[tableColumn identifier];
    if ([colId isEqualToString:@"Value"]) {
        [ritem setValue:self toObject:object];
    } else if ([colId isEqualToString:@"Extra"]) {
        [ritem setValueExtra:self];
    }
}
@end
