/*
 * joystickview.m - Joystick display
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

#import "joystickview.h"

@implementation JoystickView

- (id)initWithFrame:(NSRect)rect
{
    if ((self = [super initWithFrame:rect])) {
        stringAttr = [[NSMutableDictionary alloc] init];
        [stringAttr setObject:[NSFont messageFontOfSize:0.0]
            forKey:NSFontAttributeName];
        joyValue = 0;
    }
    
    return self;
}
 
- (void)dealloc
{
    [stringAttr release];
    [super dealloc];
}

- (void)drawRect:(NSRect)rect
{
    NSRect bounds = [self bounds];
    
    int cw = (int)(bounds.size.width / 3);
    int ch = (int)(bounds.size.height / 3);
    
    const int pos[5][2] = { 
        { 1,2 }, // 1=DOWN
        { 1,0 }, // 2=UP
        { 0,1 }, // 4=LEFT
        { 2,1 }, // 8=RIGHT
        { 1,1 }, // 16=FIRE
    };
    
    int i;
    int mask = 1;
    for (i=0;i<5;i++) {
        int xp = cw * pos[i][0];
        int yp = ch * pos[i][1];
        NSRect rect = NSMakeRect(xp,yp,cw-1,ch-1);
        if ((mask&joyValue)==mask)
            [[NSColor blackColor] set];
        else
            [[NSColor whiteColor] set];
        [NSBezierPath fillRect:rect];        
        mask <<= 1;
    }
}

- (void)setJoyValue:(int)value
{
    joyValue = value;
    [self setNeedsDisplay:YES];
}

@end

