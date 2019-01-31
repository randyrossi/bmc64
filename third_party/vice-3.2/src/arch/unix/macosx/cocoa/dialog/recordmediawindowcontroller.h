/*
 * recordmediawindowcontroller.h - record media
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
#import "viceresourcewindowcontroller.h"

@interface RecordMediaWindowController : VICEResourceWindowController
{
    IBOutlet NSButton *startRecord;
    IBOutlet NSButton *stopRecord;

    IBOutlet NSTextField *mediaFileName;
    IBOutlet NSButton *mediaFileNamePicker;
    IBOutlet NSPopUpButton *mediaType;

    IBOutlet NSPopUpButton *mediaFormat;
    IBOutlet NSPopUpButton *audioFormat;
    IBOutlet NSTextField *audioBitrate;
    IBOutlet NSPopUpButton *videoFormat;
    IBOutlet NSTextField *videoBitrate;

    NSArray *currentMediaFormats;

    NSString *currentMediaType;
    BOOL currentMediaTypeHasFormats;
    NSString *currentMediaFormat;
    NSDictionary *currentVideoFormats;
    NSDictionary *currentAudioFormats;

    NSString *currentDefaultExtension;
}

-(void)updateResources:(NSNotification *)notification;

-(IBAction)startRecording:(id)sender;
-(IBAction)stopRecording:(id)sender;

-(IBAction)changedMediaFileName:(id)sender;
-(IBAction)pickMediaFileName:(id)sender;
-(IBAction)changedMediaType:(id)sender;

-(IBAction)changedMediaFormat:(id)sender;
-(IBAction)changedAudioFormat:(id)sender;
-(IBAction)changedAudioBitrate:(id)sender;
-(IBAction)changedVideoFormat:(id)sender;
-(IBAction)changedVideoBitrate:(id)sender;

-(void)setupMediaType;
-(void)updateMediaType;

@end
