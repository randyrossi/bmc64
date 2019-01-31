/*
 * logview.h - LogView
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

#import <Cocoa/Cocoa.h>

#include "vice.h"
#ifdef HAVE_EDITLINE
#include <histedit.h>
#endif

@interface LogView : NSTextView
{
    unsigned int input_pos;
    NSString * last_input;
    id target;
    SEL action;
#ifdef HAVE_EDITLINE
    History * hist;
    BOOL in_history;
#endif  /* HAVE_EDITLINE */
}
- (void)setTarget:(id)t;
- (void)setAction:(SEL)a;
- (NSString*)lastInput;
@end

