/*
 * iotreeitem.m - item of the io tree
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

#define IOTreeItemFormatHex     0       // x
#define IOTreeItemFormatBin     1

@interface IOTreeItem : NSObject
{
    NSString *title;

    NSArray *children;
    NSMutableArray *buildChildren;
    IOTreeItem *parent;

    NSString *format;
    NSArray *map;

    int minAddr;
    int maxAddr;
    NSData *memory;
    
    NSString *regValue;
    NSArray *src;
    int value;    
}

-(id)initWithTitle:(NSString *)t;
-(id)initWithTitle:(NSString *)t rangeMinAddr:(int)minAddr maxAddr:(int)maxAddr;
-(id)initWithTitle:(NSString *)t format:(NSString *)format src:(NSArray *)src map:(NSArray *)map;
-(void)setParent:(IOTreeItem *)item;

-(BOOL)addFromDictionary:(NSDictionary *)dict;
-(BOOL)parseDictionary:(NSDictionary *)dict withTitle:(NSString *)title;

-(id)getValue;
-(id)getRegisterValue;
-(BOOL)isLeaf;
-(NSString *)title;
-(int)numChildren;
-(IOTreeItem *)childAtIndex:(int)index;

-(NSComparisonResult)compare:(IOTreeItem *)item;

-(void)readMemory:(int)memSpace;
-(int)peek:(int)addr;

@end
