/*
 * viceresourcewindowcontroller.h - base class for dialog controllers
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
#import "vicefilepanel.h"

@interface VICEResourceWindowController : NSWindowController
{
    BOOL shownBeforeMonitor;
}

-(void)registerForResourceUpdate:(SEL)selector;
-(void)triggerResourceUpdate:(id)sender;

// Resource convenience methods
-(int)getIntResource:(NSString *)name;
-(int)getIntResource:(NSString *)format withNumber:(int)number;

-(BOOL)setIntResource:(NSString *)name toValue:(int)value;
-(BOOL)setIntResource:(NSString *)format withNumber:(int)number toValue:(int)value;

-(NSString *)getStringResource:(NSString *)name;
-(NSString *)getStringResource:(NSString *)name withNumber:(int)number;

-(BOOL)setStringResource:(NSString *)name toValue:(NSString *)string;
-(BOOL)setStringResource:(NSString *)name withNumber:(int)number toValue:(NSString *)string;

-(VICEFilePanel *)getFilePanel;

@end
