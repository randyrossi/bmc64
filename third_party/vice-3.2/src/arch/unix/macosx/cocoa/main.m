/*
 * main.m - MacVICE startup
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

#include <string.h>

#include "main.h"
#include "archdep.h"

#import "viceapplication.h"

int main(int argc, char **argv)
{
    // launched by Finder.app? see SDLmain.m
    if (argc >= 2 && strncmp(argv[1], "-psn_", 5) == 0)
        argc = 1;

    // create pool for app
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
 
    // fetch bundle identifier
    NSString *bundleId = [[NSBundle mainBundle] bundleIdentifier];
    
    // build preferences path
    NSString *basePath = @"~/Library/Preferences/";
    NSString *prefPath = [basePath stringByAppendingString:bundleId];
    prefPath = [prefPath stringByExpandingTildeInPath];
    archdep_pref_path = [prefPath cStringUsingEncoding:NSUTF8StringEncoding];
    
    // fetch instance
    [VICEApplication sharedApplication];
    
    // set myself as delegate
    [NSApp setDelegate:NSApp];
    
    // load MainMenu.nib
    [NSBundle loadNibNamed:@"MainMenu" owner:NSApp];
    
    // enter main loop
    [NSApp runWithArgC:argc argV:argv];
 
    // release pool
    [pool release];
    return 0;
}

void main_exit(void)
{
}
