/*
 * recordhistorywindowcontroller.h - record history
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

@interface RecordHistoryWindowController : VICEResourceWindowController
{
    IBOutlet NSButton *recordButton;
    IBOutlet NSButton *playButton;
    IBOutlet NSButton *stopButton;
    IBOutlet NSButton *setMilestoneButton;
    IBOutlet NSButton *resetMilestoneButton;
    IBOutlet NSTextField *workDirText;
    IBOutlet NSButton *workDirButton;
    IBOutlet NSPopUpButton *startRecordPopup;
    IBOutlet NSTextField *recordingText;
    IBOutlet NSTextField *playbackText;
    IBOutlet NSTextField *timeText;
}

-(void)updateResources:(NSNotification *)notification;

-(IBAction)startRecord:(id)sender;
-(IBAction)startPlayback:(id)sender;
-(IBAction)stopRecordOrPlayback:(id)sender;
-(IBAction)setMilestone:(id)sender;
-(IBAction)resetMilestone:(id)sender;
-(IBAction)enterWorkDir:(id)sender;
-(IBAction)pickWorkDir:(id)sender;
-(IBAction)pickStartRecord:(id)sender;

-(void)displayPlayback:(NSNotification *)notification;
-(void)displayRecording:(NSNotification *)notification;
-(void)displayEventTime:(NSNotification *)notification;

-(void)setButtonState;

@end
