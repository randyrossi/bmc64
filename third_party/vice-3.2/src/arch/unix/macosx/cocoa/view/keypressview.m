/*
 * keypressview.m - Display a key press
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

#import "keypressview.h"

@implementation KeyPressView

- (id)initWithFrame:(NSRect)rect
{
    if ((self = [super initWithFrame:rect])) {
        stringAttr = [[NSMutableDictionary alloc] init];
        [stringAttr setObject:[NSFont messageFontOfSize:0.0]
            forKey:NSFontAttributeName];
        keyCode = 0;
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
    [[NSColor controlBackgroundColor] set];
    [NSBezierPath fillRect:bounds];
    
    NSString *codeStr = [NSString stringWithFormat:@"%04x",keyCode];
    NSSize size = [codeStr sizeWithAttributes:stringAttr];
    NSPoint pos;
    pos.x = bounds.origin.x + (bounds.size.width - size.width) / 2;
    pos.y = bounds.origin.y + (bounds.size.height - size.height) / 2;
    [codeStr drawAtPoint:pos withAttributes:stringAttr];
    
    if ([[self window] firstResponder] == self) {
        [[NSColor keyboardFocusIndicatorColor] set];
        [NSBezierPath setDefaultLineWidth:4.0];
        [NSBezierPath strokeRect:bounds];
    }
}

- (BOOL)isOpaque
{
    return YES;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)resignFirstResponder
{
    [self setNeedsDisplay:YES];
    return YES;
}

- (BOOL)becomeFirstResponder
{
    [self setNeedsDisplay:YES];
    return YES;
}

- (void)keyDown:(NSEvent *)event
{
    keyCode = [event keyCode];
    [self setNeedsDisplay:YES];
}

- (void)flagsChanged:(NSEvent *)event
{
    keyCode = [event keyCode];
    [self setNeedsDisplay:YES];
}

- (int)keyCode
{
    return keyCode;
}

@end

