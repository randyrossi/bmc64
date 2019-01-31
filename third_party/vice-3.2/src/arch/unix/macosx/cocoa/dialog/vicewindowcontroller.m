/*
 * vicewindowcontroller.m - generic window controller
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

#include "vice.h"
#include "machine.h"

#import "vicewindowcontroller.h"

@implementation VICEWindowController

-(id)initWithWindowNibName:(NSString *)name title:(NSString *)customTitle showOnDefault:(BOOL)show
{
    self = [super initWithWindowNibName:name];
    
    // store the show on default flag
    showOnDefault = show;
    
    // setup window naming without loading the window first
    title = [NSString stringWithFormat:@"VICE: %s %@",machine_get_name(),customTitle];
    [title retain];
    
    // set auto save name for window
    [self setWindowFrameAutosaveName:title];

    // load & show window if its visible
    [self setWindowVisibilityFromUserDefaults];
    
    return self;
}

-(id)initWithWindow:(NSWindow *)window showOnDefault:(BOOL)show
{
    self = [super initWithWindow:window];

    // store the default show flag
    showOnDefault = show;
    
    // take the window title
    title = [NSString stringWithFormat:@"VICE: %s %@",machine_get_name(),[window title]];
    [title retain];
    
    // update window title
    [window setTitle:title];
    [window setFrameAutosaveName:title];
    
    [self setWindowVisibilityFromUserDefaults];
    
    return self;
}

-(void)dealloc
{
    // save visibility of window to user prefs
    [self storeWindowVisibilityToUserDefaults];

    [title release];
    [super dealloc];
}

- (BOOL)isWindowStoredAsVisible
{
    NSString *key = [title stringByAppendingString:@"Visible"];
    NSUserDefaults *def = [NSUserDefaults standardUserDefaults];

    BOOL visible;
    if ([def objectForKey:key] == nil) {
        visible = showOnDefault; // use factory default
    } else {
        visible = [def boolForKey:key];
    }
    return visible;
}

- (void)storeWindowVisibility:(BOOL)visible
{
    NSString *key = [title stringByAppendingString:@"Visible"];
    NSUserDefaults *def = [NSUserDefaults standardUserDefaults];

    [def setBool:visible forKey:key];
}

- (BOOL)setWindowVisibilityFromUserDefaults
{
    if ([self isWindowStoredAsVisible]) {
        [[self window] orderFront:self];
        //NSLog(@"restore window %@ visible",title);
        return true;
    } else {
        //NSLog(@"restore window %@ not visible",title);
        return false;
    }
}

- (void)storeWindowVisibilityToUserDefaults
{
    //NSLog(@"store window %@ as visible %d",title,[[self window] isVisible]);
    [self storeWindowVisibility:[[self window] isVisible]];
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    [[self window] setTitle:title];
}

- (void)toggleWindow:(id)sender
{
    NSWindow *window = [self window];  // note: this may lazily load the window, so do this before testing isWindowLoaded
    BOOL isLoaded = [self isWindowLoaded];
    if(isLoaded) {
        if([window isVisible]) {
            [window orderOut:sender];
        } else {
            [self showWindow:sender];
        }
    }
}

- (void)checkMenuItem:(NSMenuItem *)menuItem
{
    BOOL isLoaded = [self isWindowLoaded];
    if(isLoaded) {
        NSWindow *window = [self window];
        if([window isVisible]) {
            [menuItem setState:NSOnState];
        } else {
            [menuItem setState:NSOffState];
        }
    }
}

@end
