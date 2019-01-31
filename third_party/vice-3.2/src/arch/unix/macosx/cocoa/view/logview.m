/*
 * logview.m - LogView
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *  Michael Klein <michael.klein@puffin.lb.shuttle.de>
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

#import "logview.h"

#ifdef HAVE_EDITLINE
# define HISTORY_SIZE 500
#endif

@implementation LogView

- (void)setTarget:(id)t
{
    target = t;
}

- (void)setAction:(SEL)a
{
    action = a;
}

- (void)updateInsertionPointStateAndRestartTimer:(BOOL)flag
{
    NSRange sel = [self selectedRange];
    if (sel.location < input_pos && sel.length == 0)
    {
        [self setSelectedRange:NSMakeRange(input_pos, 0)];
    }
    else
    {
        [super updateInsertionPointStateAndRestartTimer:flag];
    }
}

- (BOOL)shouldChangeTextInRange:(NSRange)r replacementString:(NSString*)s
{
    return r.location >= input_pos &&
        [super shouldChangeTextInRange:r replacementString:s];
}

- (id)initWithFrame:(NSRect)frame
{
    if ((self = [super initWithFrame:frame]) != nil)
    {
        last_input = nil;
#ifdef HAVE_EDITLINE
        hist = history_init();
        HistEvent ev;
        history(hist, &ev, H_SETSIZE, HISTORY_SIZE);
#endif
    }
    return self;
}

- (void)dealloc
{
#ifdef HAVE_EDITLINE
    history_end(hist);
#endif
    [last_input dealloc];
    [super dealloc];
}

- (void)setEditable:(BOOL)flag
{
    if (![self isEditable] && flag)
    {
        input_pos = [[self textStorage] length];
#ifdef HAVE_EDITLINE
        in_history = NO;
#endif
    }

    [super setEditable:flag];
}

- (NSRange)rangeForUserTextChange
{
    NSRange sel = [self selectedRange];
    if (sel.location < input_pos)
    {
        sel = NSMakeRange([[self textStorage] length], 0);
        [self setSelectedRange:sel];
    }
    return sel;
}

- (NSString*)lastInput
{
    return last_input;
}

#ifdef HAVE_EDITLINE
- (void)keyDown:(NSEvent*)event
{
    if ([self isEditable])
    {
        HistEvent ev;
        NSRange sel = NSMakeRange(input_pos, [[self textStorage] length] - input_pos);
        NSString *keyString = [event charactersIgnoringModifiers];

        /* no character assigned to key */
        if ((keyString == nil) || ([keyString length] == 0)) {
            [super keyDown:event];
            return;
        }

        unichar key = [keyString characterAtIndex:0];
        switch (key)
        {
            case NSUpArrowFunctionKey:
                if (!in_history)
                {
                    in_history = (history(hist, &ev, H_FIRST) == 0);
                }
                else
                {
                    if (history(hist, &ev, H_NEXT) < 0)
                        history(hist, &ev, H_LAST);
                }
                if (in_history)
                {
                    [self replaceCharactersInRange:sel
                                        withString:[NSString stringWithCString:ev.str
                                                         encoding:NSUTF8StringEncoding]];
                }
                return;

            case NSDownArrowFunctionKey:
                if (in_history)
                {
                    if (history(hist, &ev, H_PREV) < 0)
                    {
                        [self replaceCharactersInRange:sel withString:@""];
                        in_history = NO;
                    }
                    else
                    {
                        [self replaceCharactersInRange:sel
                                            withString:[NSString stringWithCString:ev.str 
                                                            encoding:NSUTF8StringEncoding]];
                    }
                }
                return;
        }
    }

    [super keyDown:event];
}
#endif

- (void)insertNewline:(id)sender
{
    // ignore newlines after editing
    if (![self isEditable]) {
        return;
    }

    NSTextStorage * text = [self textStorage];
    unsigned int len = [text length];

    [last_input release];
    if (len > input_pos)
    {
        last_input = [[[text string] substringWithRange:NSMakeRange(input_pos, len - input_pos)] retain];
#ifdef HAVE_EDITLINE
        HistEvent ev;
        history(hist, &ev, H_ENTER, [last_input cStringUsingEncoding:NSUTF8StringEncoding]);
#endif
    } else {
        last_input = nil;
    }

    [target performSelector:action withObject:self];
    [super insertNewline:sender];
}

@end
