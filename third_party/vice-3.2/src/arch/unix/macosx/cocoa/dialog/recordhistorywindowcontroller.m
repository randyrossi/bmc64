/*
 * recordhistorywindowcontroller.m - record history
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

#import "recordhistorywindowcontroller.h"
#import "viceapplication.h"
#import "vicenotifications.h"

@implementation RecordHistoryWindowController

-(id)init
{
    self = [super initWithWindowNibName:@"RecordHistory"];
    [self registerForResourceUpdate:@selector(updateResources:)];

    // register notifications
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(displayRecording:)
                                                 name:VICEDisplayRecordingNotification
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(displayPlayback:)
                                                 name:VICEDisplayPlaybackNotification
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(displayEventTime:)
                                                 name:VICEDisplayEventTimeNotification
                                               object:nil];

    return self;
}

-(void)windowDidLoad
{
    [super windowDidLoad];
    [recordingText setStringValue:@""];
    [playbackText setStringValue:@""];
    [timeText setStringValue:@""];
    [self setButtonState];
}

-(void)updateResources:(NSNotification *)notification
{
    NSString *workDir = [self getStringResource:@"EventSnapshotDir"];
    [workDirText setStringValue:workDir];

    int startMode = [self getIntResource:@"EventStartMode"];
    [startRecordPopup selectItemWithTag:startMode];
}

// ----- Notifications -----

-(void)displayPlayback:(NSNotification *)notification
{
    NSDictionary * dict = [notification userInfo];
    BOOL playback = [[dict objectForKey:@"playback"] boolValue];
    NSString *version = (NSString *)[dict objectForKey:@"version"];
    if (playback) { 
        [playbackText setStringValue:[NSString stringWithFormat:@"Playback: %@", version]];
    } else {
        [playbackText setStringValue:@""];
        [self setButtonState];
    }
}

-(void)displayRecording:(NSNotification *)notification
{
    NSDictionary * dict = [notification userInfo];
    BOOL recording = [[dict objectForKey:@"recording"] boolValue];
    [recordingText setStringValue:(recording ? @"Recording" : @"")];
    if (!recording) {
        [self setButtonState];
    }
}

-(void)displayEventTime:(NSNotification *)notification
{
    NSDictionary * dict = [notification userInfo];
    int curTime = [[dict objectForKey:@"time"] intValue];
    int totalTime = [[dict objectForKey:@"totalTime"] intValue];
    
    int curMin = curTime / 60;
    int curSec = curTime % 60;
    NSString *txt;
    if (totalTime != 0) {
        int totalMin = totalTime / 60;
        int totalSec = totalTime % 60;
        txt = [NSString stringWithFormat:@"Time %02d:%02d  Total Time %02d:%02d",
                curMin, curSec, totalMin, totalSec];
    } else {
        txt = [NSString stringWithFormat:@"Time %02d:%02d",
                curMin, curSec];    
    }
    [timeText setStringValue:txt];
}

// ----- Actions -----

-(void)setButtonState
{
    VICEMachineController *ctrl = [VICEApplication theMachineController];
    BOOL isRecording = [ctrl isRecordingHistory];
    BOOL isPlayingBack = [ctrl isPlayingBackHistory];
    BOOL isBusy = isRecording || isPlayingBack;

    [recordButton setEnabled:!isBusy];
    [playButton setEnabled:!isBusy];
    [stopButton setEnabled:isBusy];
        
    [setMilestoneButton setEnabled:isRecording];
    [resetMilestoneButton setEnabled:isRecording];
    
    [workDirText setEnabled:!isBusy];
    [workDirButton setEnabled:!isBusy];
    [startRecordPopup setEnabled:!isBusy];
}

-(IBAction)startRecord:(id)sender
{
    [[VICEApplication theMachineController] startRecordHistory];
    [self setButtonState];
}

-(IBAction)startPlayback:(id)sender
{
    [[VICEApplication theMachineController] startPlaybackHistory];
    [self setButtonState];
}

-(IBAction)stopRecordOrPlayback:(id)sender
{
    VICEMachineController *ctrl = [VICEApplication theMachineController];
    if ([ctrl isRecordingHistory]) {
        [ctrl stopRecordHistory];
    } else if ([ctrl isPlayingBackHistory]) {
        [ctrl stopPlaybackHistory];
    }
    [self setButtonState];
}

-(IBAction)setMilestone:(id)sender
{
    [[VICEApplication theMachineController] setRecordMilestone];
}

-(IBAction)resetMilestone:(id)sender
{
    [[VICEApplication theMachineController] resetRecordMilestone];
}

-(IBAction)enterWorkDir:(id)sender
{
    [self setStringResource:@"EventSnapshotDir" toValue:[sender stringValue]];
    [self updateResources:nil];
}

-(IBAction)pickWorkDir:(id)sender
{
    VICEAppController *appCtrl = [VICEApplication theAppController];
    NSString *path = [[appCtrl getFilePanel] pickDirectoryWithTitle:@"Record History"];
    if (path != nil) {
        [self setStringResource:@"EventSnapshotDir" toValue:path];
        [self updateResources:nil];
    }
}

-(IBAction)pickStartRecord:(id)sender
{
    int mode = [[sender selectedItem] tag];
    [self setIntResource:@"EventStartMode" toValue:mode];
    [self updateResources:nil];
}

@end
