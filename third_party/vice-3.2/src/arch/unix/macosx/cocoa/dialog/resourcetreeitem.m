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

#include "machine.h"

#import "resourcetreeitem.h"
#import "resourceeditorcontroller.h"

@implementation ResourceTreeItem

-(id)initWithTitle:(NSString *)t
{
    self = [super init];
    if (self == nil) {
        return nil;
    }

    title = [t retain];
    resource = nil;
    children = nil;
    buildChildren = nil;
    cacheValue = nil;
    dataCell = nil;
    extraCell = nil;

    return self;
}

-(void)dealloc
{
    [title release];
    [resource release];
    [children release];
    [cacheValue release];
    [dataCell release];
    [extraCell release];
    [args release];
    [super dealloc];
}

-(BOOL)isLeaf
{
    return children == nil;
}

-(ResourceTreeItem *)childAtIndex:(int)index
{
    return (ResourceTreeItem *)[children objectAtIndex:index];
}

-(int)numChildren
{
    if (children == nil) {
        return 0;
    } else {
        return [children count];
    }
}

-(void)addChild:(ResourceTreeItem *)item
{
    if(buildChildren == nil) {
        buildChildren = [[NSMutableArray alloc] init];
    }
    [buildChildren addObject:item];
}

-(ResourceTreeItem *)hasChildWithTitle:(NSString *)name
{
    if(buildChildren == nil)
        return nil;
    
    int numChilds = [buildChildren count];
    int i;
    for(i=0;i<numChilds;i++) {
        ResourceTreeItem *child = [buildChildren objectAtIndex:i];
        if([[child title] compare:name] == NSOrderedSame) {
            return child;
        }
    }
    return nil;
}

-(BOOL)addFromDictionaryLoop:(NSDictionary *)dict
{
    NSEnumerator *enumerator = [dict keyEnumerator];
    NSString *key;
    while ((key = [enumerator nextObject])) {
        
        id value = [dict objectForKey:key];

        // its a dictionary itself
        if ([value isKindOfClass:[NSDictionary class]]) {
            // is it a machine class dictionary?
            if([key hasPrefix:@"_"]) {
                // valid for this machine?
                NSString *machineName = [NSString stringWithFormat:@"_%s_", machine_name];
                NSRange range = [key rangeOfString:machineName];
                if(range.location != NSNotFound) {
                    // add machine dependent entries to this item
                    if(![self addFromDictionaryLoop:value]) {
                        return FALSE;
                    }
                } else {
                    // try get_machine_name as it might differ
                    NSString *realMachineName = [NSString stringWithFormat:@"_%s_", machine_get_name()];
                    NSRange range = [key rangeOfString:realMachineName];
                    if(range.location != NSNotFound) {
                        // add machine dependent entries to this item
                        if(![self addFromDictionaryLoop:value]) {
                            return FALSE;
                        }
                    } else {
                        // try to get machine_class as a number
                        NSString *machineClass = [NSString stringWithFormat:@"_%d_", machine_class];
                        NSRange range = [key rangeOfString:machineClass];
                        if(range.location != NSNotFound) {
                            // add machine dependent entries to this item
                            if(![self addFromDictionaryLoop:value]) {
                                return FALSE;
                            }
                        }
                    }
                }
            }
            // normal dictionary with contents
            else {
                ResourceTreeItem *item = [self hasChildWithTitle:key];
                if(item == nil) {
                    item = [[ResourceTreeItem alloc] initWithTitle:key];
                    [self addChild:item];
                }
                if (![item addFromDictionaryLoop:value]) {
                    return FALSE;
                }
            }
        }
        // its a string
        else if ([value isKindOfClass:[NSString class]]) {
            NSString *param = (NSString *)value;
            if(param == nil) {
                NSLog(@"Invalid param string for '%@'",key);
                return FALSE;
            }
            
            ResourceTreeItem *item = [[ResourceTreeItem alloc] initWithTitle:key];
            [self addChild:item];
            
            if (![item parseResourceString:param]) {
                return FALSE;
            }
        }
        // unknown
        else {
            NSLog(@"Invalid Class in Dictionary: %@", [value class]);
            return FALSE;
        }
    }
    return TRUE;
}

-(void)finalizeChildren
{
    int numChilds = [buildChildren count];
    int i;
    for(i=0;i<numChilds;i++) {
        ResourceTreeItem *child = [buildChildren objectAtIndex:i];
        [child finalizeChildren];
    }
    
    // sort array and keep
    children = [buildChildren sortedArrayUsingSelector:@selector(compare:)];
    [children retain];
    [buildChildren release];    
}

-(BOOL)addFromDictionary:(NSDictionary *)dict
{
    BOOL result = [self addFromDictionaryLoop:dict];
    [self finalizeChildren];
    return result;
}

-(BOOL)parseResourceString:(NSString *)string
{
    args = [string componentsSeparatedByString:@","];
    [args retain];
    numArgs = [args count];
    if (numArgs < 2) {
        NSLog(@"ERROR: resource string invalid: %@",string);
        return FALSE;
    }

    resource = (NSString *)[args objectAtIndex:0];
    if(resource == nil) {
        NSLog(@"ERROR: invalid resource in %@: ",string);
        return FALSE;
    }
    [resource retain];

    NSString *typeStr = (NSString *)[args objectAtIndex:1];
    unichar ch = [typeStr characterAtIndex:0];
    switch (ch) {
        case 'i': // integer
            type = ResourceTreeItemTypeInteger;
            hint = ResourceTreeItemHintNone;
            break;
        case 'b': // boolean
            type = ResourceTreeItemTypeInteger;
            hint = ResourceTreeItemHintBoolean;
            break;
        case 'e': // enum
            type = ResourceTreeItemTypeInteger;
            hint = ResourceTreeItemHintEnum;
            break;
        case 'E': // enum direct
            type = ResourceTreeItemTypeInteger;
            hint = ResourceTreeItemHintEnumDirect;
            break;
        case 'r': // range
            type = ResourceTreeItemTypeInteger;
            hint = ResourceTreeItemHintRange;
            break;
        case 's': // string
            type = ResourceTreeItemTypeString;
            hint = ResourceTreeItemHintNone;
            break;
        case 'm': // map string to integer
            type = ResourceTreeItemTypeInteger;
            hint = ResourceTreeItemHintMapInteger;
            break;
        case 'f': // file
            {
                unichar mode = [typeStr characterAtIndex:1];
                type = ResourceTreeItemTypeString;
                switch (mode) {
                    case 'o':
                        hint = ResourceTreeItemHintFileOpen;
                        break;
                    case 's':
                        hint = ResourceTreeItemHintFileSave;
                        break;
                    case 'd':
                        hint = ResourceTreeItemHintFileDir;
                        break;
                    default:
                        NSLog(@"ERROR: Invalid file resource mode: %@", typeStr);
                        return false;
                }
            }
            break;
        default:
            NSLog(@"ERROR: Invalid resource type: %@", typeStr);
            return FALSE;
    }

    return TRUE;
}

-(NSString *)title
{
    return title;
}

-(int)parseIntFromString:(NSString *)string
{
    int len = [string length];
    if (len == 0) {
        return 0;
    }

    int value = 0;
    const char *str = [string cStringUsingEncoding:NSUTF8StringEncoding];
    if (*str == '$') {
        sscanf(str + 1, "%x", &value);
    } else {
        sscanf(str, "%d", &value);
    }
    return value;
}

-(id)getValue:(id)ctl
{
    ResourceEditorController *controller = (ResourceEditorController *)ctl;
    if ([self isLeaf]) {
        if (cacheValue == nil) {
            if (type == ResourceTreeItemTypeInteger) {
                int value = [controller getIntResource:resource];
//                NSLog(@"read integer: %@ %d",resource,value);

                // enum
                if (hint == ResourceTreeItemHintEnum) {
                    cacheValue = [[args objectAtIndex:value + 2] retain];
                }
                // direct enum
                else if (hint == ResourceTreeItemHintEnumDirect) {
                    int i;
                    for (i = 2; i < numArgs; i++) {
                        NSString *argString = (NSString *)[args objectAtIndex:i];
                        int argValue = [self parseIntFromString:argString];
                        if (argValue == value) {
                            cacheValue = [argString retain];
                            break;
                        }
                    }
                }
                // map string to ineger:   txt=value
                else if (hint == ResourceTreeItemHintMapInteger) {
                    int i;
                    for (i = 2; i < numArgs; i++) {
                        NSString *argString = (NSString *)[args objectAtIndex:i];
                        NSArray *pair = [argString componentsSeparatedByString:@"="];
                        if ([pair count] >= 2) {
                            NSString *keyStr = (NSString *)[pair objectAtIndex:0];
                            NSString *valueStr = (NSString *)[pair objectAtIndex:1]; 
                            if (keyStr && valueStr) {
                                if ([self parseIntFromString:valueStr] == value) {
                                    cacheValue = [keyStr retain];
                                    break;
                                }
                            }
                        }
                    }
                }
                // integer
                else {
                    cacheValue = [[NSNumber alloc] initWithInt:value];
                }
            } else {
                if(resource == nil) {
                    NSLog(@"NIL resource in %@",title);
                    return @"";
                }
                
                NSString *value = [controller getStringResource:resource];
//                NSLog(@"read string: %@ %@",resource,value);
                cacheValue = [value retain];
            }
        }
        return cacheValue; 
    } else {
        return @"";
    }
}

-(void)setIntResourceToValue:(int)value withController:(id)ctl
{
    ResourceEditorController *controller = (ResourceEditorController *)ctl;
    [controller setIntResource:resource toValue:value];
    [self invalidateCache];
    [controller reloadItem:self];
//    NSLog(@"set int resource %@ %d",resource,value);
}

-(void)setStringResourceToValue:(NSString *)value withController:(id)ctl
{
    ResourceEditorController *controller = (ResourceEditorController *)ctl;
    [controller setStringResource:resource toValue:value];
    [self invalidateCache];
    [controller reloadItem:self];
//    NSLog(@"set string resource %@ %d",resource,value);    
}

-(void)setValue:(id)ctl toObject:(id)object
{
    NSString *string = (NSString *)object;
    if (type == ResourceTreeItemTypeInteger) {
        int value = 0;
        // enum resource
        if (hint == ResourceTreeItemHintEnum || hint == ResourceTreeItemHintEnumDirect) {
            int i;
            for (i = 2; i < numArgs; i++) {
                NSString *argVal = (NSString *)[args objectAtIndex:i];
                if ([argVal isEqualToString:string]) {
                    if (hint == ResourceTreeItemHintEnum) {
                        value = i - 2;
                    } else {
                        value = [self parseIntFromString:argVal];
                    }
                    break;
                }
            }
        }
        // map
        else if (hint == ResourceTreeItemHintMapInteger) {
            int i;
            for (i = 2; i < numArgs; i++) {
                NSString *argString = (NSString *)[args objectAtIndex:i];
                NSArray *pair = [argString componentsSeparatedByString:@"="];
                if ([pair count] >= 2) {
                    NSString *keyStr = (NSString *)[pair objectAtIndex:0];
                    NSString *valueStr = (NSString *)[pair objectAtIndex:1]; 
                    if (keyStr && valueStr) {
                        if ([keyStr isEqualToString:string]) {
                            value = [self parseIntFromString:valueStr];
                            break;
                        }
                    }
                }
            }
        }
        // range
        else if (hint == ResourceTreeItemHintRange) {
            if (numArgs == 4) {
                value = [string intValue];
                int min = [[args objectAtIndex:2] intValue];
                int max = [[args objectAtIndex:3] intValue];
                if (value < min) {
                    value = min;
                } else if (value > max) {
                    value = max;
                }
            } else {
                NSLog(@"ERROR: range invalid: %@", args);
            }
        }
        // integer resource
        else {
            value = [string intValue];
        }
        [self setIntResourceToValue:value withController:ctl];
    } else {
        [self setStringResourceToValue:string withController:ctl];
    }
}

-(void)setValueExtra:(id)ctl
{
    ResourceEditorController *controller = (ResourceEditorController *)ctl;
    switch (hint) {
        case ResourceTreeItemHintFileOpen:
            {
                NSString *file = [[controller getFilePanel] pickOpenFileWithTitle:title types:nil];
                if (file != nil) {
                    [self setStringResourceToValue:file withController:ctl];
                }
                break;
            }
        case ResourceTreeItemHintFileSave:
            {
                NSString *file = [[controller getFilePanel] pickSaveFileWithTitle:title types:nil];
                if (file != nil) {
                    [self setStringResourceToValue:file withController:ctl];
                }
                break;
            }
        case ResourceTreeItemHintFileDir:
            {
                NSString *file = [[controller getFilePanel] pickDirectoryWithTitle:title];
                if (file != nil) {
                    [self setStringResourceToValue:file withController:ctl];
                }
                break;
            }
    }
}

-(NSCell *)dataCell:(NSCell *)colCell
{
    if ([self isLeaf]) {
        if (dataCell != nil) {
            return dataCell;
        }
            
        switch(hint) {
            // boolean cell
            case ResourceTreeItemHintBoolean:
                {
                    NSButtonCell *bcell = [[NSButtonCell alloc] initTextCell:@""];
                    [bcell setButtonType:NSSwitchButton];
                    dataCell = bcell;
                    break;
                }
            case ResourceTreeItemHintEnum:
            case ResourceTreeItemHintEnumDirect:
                {
                    NSComboBoxCell *ccell = [[NSComboBoxCell alloc] initTextCell:@""];
                    int i;
                    [ccell setButtonBordered:FALSE];
                    for (i = 2; i < numArgs; i++) {
                        [ccell addItemWithObjectValue:[args objectAtIndex:i]];
                    }
                    dataCell = ccell;
                    break;
                }
            case ResourceTreeItemHintMapInteger:
                {
                    NSComboBoxCell *ccell = [[NSComboBoxCell alloc] initTextCell:@""];
                    int i;
                    [ccell setButtonBordered:FALSE];
                    for (i = 2; i < numArgs; i++) {
                        NSString *argString = (NSString *)[args objectAtIndex:i];
                        NSArray *pair = [argString componentsSeparatedByString:@"="];
                        if ([pair count] >= 2) {
                             NSString *keyStr = (NSString *)[pair objectAtIndex:0];
                             NSString *valueStr = (NSString *)[pair objectAtIndex:1]; 
                             [ccell addItemWithObjectValue:keyStr];
                        }
                    }
                    dataCell = ccell;
                    break;
                }
            default:
                return colCell;
            }
            // reuse default font
            [dataCell setFont:[colCell font]];
            return dataCell;
    }
    return colCell;
}

-(NSCell *)extraCell:(NSCell *)colCell
{
    if ([self isLeaf]) {
        if (extraCell != nil) {
            return extraCell;
        }

        switch (hint) {
            // file name cell
            case ResourceTreeItemHintFileOpen:
            case ResourceTreeItemHintFileSave:
            case ResourceTreeItemHintFileDir:
                {
                    NSButtonCell *bcell = [[NSButtonCell alloc] initTextCell:@""];
                    [bcell setButtonType:NSMomentaryLightButton];
                    [bcell setTitle:@"..."];
                    extraCell = bcell;                
                    break;
                }
            default:
                return colCell;
        }
        [extraCell setFont:[colCell font]];
        return extraCell;
    }
    return colCell;
}

-(NSComparisonResult)compare:(ResourceTreeItem *)item
{
    return [title caseInsensitiveCompare:[item title]];
}

-(void)invalidateCache
{
    if ([self isLeaf]) {
        [cacheValue release];
        cacheValue = nil;
    } else {
        int num = [children count];
        int i;
        for (i = 0; i < num; i++) {
            [(ResourceTreeItem *)[children objectAtIndex:i] invalidateCache];
        }
    }
}

@end
