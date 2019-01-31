/*
 * resourcetreeitem.m - item of the resource tree
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

#import <Cocoa/Cocoa.h>

#define ResourceTreeItemTypeString      0 // s
#define ResourceTreeItemTypeInteger     1 // i

#define ResourceTreeItemHintNone        0
#define ResourceTreeItemHintBoolean     1 // b
#define ResourceTreeItemHintEnum        2 // e
#define ResourceTreeItemHintEnumDirect  3 // E
#define ResourceTreeItemHintRange       4 // r
#define ResourceTreeItemHintMapInteger  5 // m

#define ResourceTreeItemHintFileOpen    10 // fo
#define ResourceTreeItemHintFileSave    11 // fs
#define ResourceTreeItemHintFileDir     12 // fd

@interface ResourceTreeItem : NSObject
{
    NSString *title;
    NSString *resource;
    NSArray *children;
    NSMutableArray *buildChildren;
    int type;
    int hint;
    id cacheValue;
    NSCell *dataCell;
    NSCell *extraCell;
    NSArray *args;
    int numArgs;
}

-(id)initWithTitle:(NSString *)t;

-(BOOL)addFromDictionary:(NSDictionary *)dict;

-(id)getValue:(id)controller;
-(void)setValue:(id)controller toObject:(id)object;
-(void)setValueExtra:(id)controller;

-(BOOL)isLeaf;
-(NSString *)title;
-(int)numChildren;
-(ResourceTreeItem *)childAtIndex:(int)index;

-(BOOL)parseResourceString:(NSString *)string;

-(void)invalidateCache;

-(NSCell *)dataCell:(NSCell *)colCell;
-(NSCell *)extraCell:(NSCell *)extraCell;

-(NSComparisonResult)compare:(ResourceTreeItem *)item;

@end
