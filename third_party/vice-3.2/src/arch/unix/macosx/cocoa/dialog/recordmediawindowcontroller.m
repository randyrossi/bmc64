/*
 * recordmediawindowcontroller.m - record media
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

#import "recordmediawindowcontroller.h"
#import "viceapplication.h"
#import "vicenotifications.h"

@implementation RecordMediaWindowController

-(id)init
{
    self = [super initWithWindowNibName:@"RecordMedia"];
    [self registerForResourceUpdate:@selector(updateResources:)];
    return self;
}

-(void)dealloc
{
    [currentMediaType release];
    [currentMediaFormats release];
    [super dealloc];
}

-(void)windowDidLoad
{
    NSString *fileName = @"~/capture";
    [mediaFileName setStringValue:[fileName stringByExpandingTildeInPath]];
    [stopRecord setEnabled:FALSE];

    [self setupMediaType];
    [self updateMediaType];
    [self updateResources:nil];

    [super windowDidLoad];
}

-(void)updateAudioBitrate
{
    NSString *res = [NSString stringWithFormat:@"%@AudioBitrate", currentMediaType];
    int value = [self getIntResource:res];
    [audioBitrate setIntValue:value];
}

-(void)updateVideoBitrate
{
    NSString *res = [NSString stringWithFormat:@"%@VideoBitrate", currentMediaType];
    int value = [self getIntResource:res];
    [videoBitrate setIntValue:value];    
}

-(void)updateVideoFormat
{        
    // get current video format
    NSString *res = [NSString stringWithFormat:@"%@VideoCodec", currentMediaType];
    int codecId = [self getIntResource:res];
    NSString *codecName = (NSString *)[currentVideoFormats objectForKey:[NSNumber numberWithInt:codecId]];
    if (codecName != nil) {
        [videoFormat selectItemWithTitle:codecName];
    }
}

-(void)updateAudioFormat
{
    // get current video format
    NSString *res = [NSString stringWithFormat:@"%@AudioCodec", currentMediaType];
    int codecId = [self getIntResource:res];
    NSString *codecName = (NSString *)[currentAudioFormats objectForKey:[NSNumber numberWithInt:codecId]];
    if (codecName != nil) {
        [audioFormat selectItemWithTitle:codecName];
    }
}

-(void)replaceFileNameExtension:(BOOL)force
{
    NSString *fileName = [mediaFileName stringValue];

    // if not forced only replace if there is no extension
    BOOL doit = force;
    if (!doit) {
        doit = [[fileName pathExtension] length] == 0;
    }
    
    if (doit) {
        fileName = [fileName stringByDeletingPathExtension];
        NSString *result = [NSString stringWithFormat:@"%@.%@", fileName, currentDefaultExtension];
        [mediaFileName setStringValue:result];
    }
}

-(void)fetchDefaultExtension
{
    [currentDefaultExtension release];
    currentDefaultExtension = [[VICEApplication theMachineController] 
        defaultExtensionForMediaDriver:currentMediaType
                             andFormat:currentMediaFormat];
    [currentDefaultExtension retain];
    
    [self replaceFileNameExtension:TRUE];
}

-(void)updateMediaFormat
{
    if (!currentMediaTypeHasFormats) {
        return;
    }
    
    // get current format
    NSString *res = [NSString stringWithFormat:@"%@Format", currentMediaType];
    [currentMediaFormat release];
    currentMediaFormat = [self getStringResource:res];
    [currentMediaFormat retain];

    [self fetchDefaultExtension];

    // set media format
    [mediaFormat selectItemWithTitle:currentMediaFormat];

    // find media format in description and setup audio and video codecs
    int i;
    for (i = 0; i < [currentMediaFormats count]; i++) {
        NSArray *fa = (NSArray *)[currentMediaFormats objectAtIndex:i];
        NSString *formatName = (NSString *)[fa objectAtIndex:0];

        // set audio and video codecs for current format
        if ([currentMediaFormat compare:formatName] == NSOrderedSame) {
            currentVideoFormats = (NSDictionary *)[fa objectAtIndex:1];
            currentAudioFormats = (NSDictionary *)[fa objectAtIndex:2];

            [videoFormat removeAllItems];
            [videoFormat addItemsWithTitles:[currentVideoFormats allValues]];

            [audioFormat removeAllItems];
            [audioFormat addItemsWithTitles:[currentAudioFormats allValues]];

            BOOL hasVideoFormats = [currentVideoFormats count] > 0;
            BOOL hasAudioFormats = [currentAudioFormats count] > 0;
            
            [videoFormat  setEnabled:hasVideoFormats];
            [videoBitrate setEnabled:hasVideoFormats];
            [audioFormat  setEnabled:hasAudioFormats];
            [audioBitrate setEnabled:hasAudioFormats];

            break;
        }
    }
    
    // set the correct format
    [self updateAudioFormat];
    [self updateVideoFormat];
    [self updateAudioBitrate];
    [self updateVideoBitrate];
}

-(void)updateMediaType
{
    // fetch media type from pop up button    
    [currentMediaType release];
    currentMediaType = [[mediaType selectedItem] title];
    [currentMediaType retain];

    // ask the machine controller if the media type has options
    currentMediaTypeHasFormats = [[VICEApplication theMachineController] mediaDriverHasFormats:currentMediaType];
    BOOL hasFormats = currentMediaTypeHasFormats;

    // enable/disable options
    [mediaFormat setEnabled:hasFormats];
    [videoFormat setEnabled:hasFormats];
    [videoBitrate setEnabled:hasFormats];
    [audioFormat setEnabled:hasFormats];
    [audioBitrate setEnabled:hasFormats];

    if (hasFormats) {
        // fetch all formats and video/audio codecs from machine controller
        [currentMediaFormats release];
        currentMediaFormats = [[VICEApplication theMachineController] enumMediaFormats:currentMediaType];
        [currentMediaFormats retain];
        
        // fill in formats
        [mediaFormat removeAllItems];
        int i;
        for (i = 0; i < [currentMediaFormats count]; i++) {
            NSArray *fa = (NSArray *)[currentMediaFormats objectAtIndex:i];
            NSString *formatName = (NSString *)[fa objectAtIndex:0];
            [mediaFormat addItemWithTitle:formatName];
        }

        // set current format and setup audio video codecs
        [self updateMediaFormat];
    } else {
        [currentMediaFormat release];
        currentMediaFormat = nil;
        [self fetchDefaultExtension];
    }
}

-(void)setupMediaType
{
    [mediaType removeAllItems];
    [mediaType addItemsWithTitles:[[VICEApplication theMachineController] enumMediaDrivers]];
}

-(void)updateResources:(NSNotification *)notification
{
    [self updateMediaFormat];
}

// ----- Actions -----

-(IBAction)startRecording:(id)sender
{
    int canvasId = [VICEApplication currentCanvasId];
    NSString *fileName = [mediaFileName stringValue];
    NSString *driver = [[mediaType selectedItem] title];

    BOOL ok = [[VICEApplication theMachineController] startRecordingMedia:driver
                                                               fromCanvas:canvasId
                                                                   toFile:fileName];
    if (!ok) {
        [VICEApplication runErrorMessage:@"Can't start media recording!"];
    } else {
        // still recording? enable stop button
        if ([[VICEApplication theMachineController] isRecordingMedia]) {
            [stopRecord setEnabled:TRUE];
            [startRecord setEnabled:FALSE];
            [mediaFileName setEnabled:FALSE];
            [mediaFileNamePicker setEnabled:FALSE];
        }
    }
}

-(IBAction)stopRecording:(id)sender
{
    [[VICEApplication theMachineController] stopRecordingMedia];

    [stopRecord setEnabled:FALSE];
    [startRecord setEnabled:TRUE];
    [mediaFileName setEnabled:TRUE];
    [mediaFileNamePicker setEnabled:TRUE];
}

-(IBAction)changedMediaFileName:(id)sender
{
    [self replaceFileNameExtension:FALSE];
}

-(IBAction)pickMediaFileName:(id)sender
{
    NSString *title = [NSString stringWithFormat:@"Record %@ Media", currentDefaultExtension];
    NSString *fileName = [[self getFilePanel] pickSaveFileWithTitle:title types:[NSArray arrayWithObjects:currentDefaultExtension,nil]];
    if (fileName != nil) {
        [mediaFileName setStringValue:fileName];
    }
}

-(IBAction)changedMediaType:(id)sender
{
    [self updateMediaType];
}

-(IBAction)changedMediaFormat:(id)sender
{
    NSString *res = [NSString stringWithFormat:@"%@Format", currentMediaType];
    [self setStringResource:res toValue:[mediaFormat titleOfSelectedItem]];
    [self updateMediaFormat];
}

-(IBAction)changedAudioFormat:(id)sender
{
    NSString *curFormat = [audioFormat titleOfSelectedItem];
    NSArray *keys = [currentAudioFormats allKeysForObject:curFormat];
    if ([keys count] == 1) {
        NSString *res = [NSString stringWithFormat:@"%@AudioCodec", currentMediaType];
        int value = [(NSNumber *)[keys objectAtIndex:0] intValue];
        [self setIntResource:res toValue:value];
        [self updateAudioFormat];
    } 
}

-(IBAction)changedAudioBitrate:(id)sender
{
    NSString *res = [NSString stringWithFormat:@"%@AudioBitrate", currentMediaType];
    int value = [audioBitrate intValue];
    [self setIntResource:res toValue:value];
    [self updateAudioBitrate];
}

-(IBAction)changedVideoFormat:(id)sender
{
    NSString *curFormat = [videoFormat titleOfSelectedItem];
    NSArray *keys = [currentVideoFormats allKeysForObject:curFormat];
    if ([keys count] == 1) {
        NSString *res = [NSString stringWithFormat:@"%@VideoCodec", currentMediaType];
        int value = [(NSNumber *)[keys objectAtIndex:0] intValue];
        [self setIntResource:res toValue:value];
        [self updateVideoFormat];
    }
}

-(IBAction)changedVideoBitrate:(id)sender
{
    NSString *res = [NSString stringWithFormat:@"%@VideoBitrate", currentMediaType];
    int value = [videoBitrate intValue];
    [self setIntResource:res toValue:value];
    [self updateVideoBitrate];
}

@end
