/*
 * vicemachinecontroller.h - VICEMachineController - base machine controller
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

/* The VICEMachineController posts notifications of the Machine Thread */

@interface VICEMachineController : NSObject
{
}

// ----- Resources -----

// get integer resource
-(NSNumber *)getIntResource:(NSString *)name;
// get string resource
-(NSString *)getStringResource:(NSString *)name;

// set integer resource
-(BOOL)setIntResource:(NSString *)name value:(NSNumber *)value;
// set string resource
-(BOOL)setStringResource:(NSString *)name value:(NSString *)value;

// load resources
-(BOOL)loadResources:(NSString *)path;
// save resources
-(BOOL)saveResources:(NSString *)path;
// reset resources
-(void)resetResources;

// ----- Reset -----
// reset machine
-(void)resetMachine:(BOOL)hardReset;
// reset drive
-(void)resetDrive:(int)unit;

// ----- Monitor -----
// activate monitor
-(void)activateMonitor;
// read machine registers
-(NSArray *)getRegisters:(int)memSpace;
// read machine data
-(NSData *)readMemory:(int)memSpace startAddress:(int)start endAddress:(int)end;
-(NSData *)readMemory:(int)memSpace bank:(int)bank startAddress:(int)start endAddress:(int)end;
// get current bank name
-(NSString *)getCurrentMemoryBankName:(int)memSpace;
// disassemble lines
-(NSDictionary *)disassembleMemory:(int)memSpace lines:(int)numLines start:(int)address;
// seek lines
-(NSNumber *)disassembleSeek:(int)memSpace up:(bool)up lines:(int)numLines start:(int)address;
// alter breakpoint
-(void)alterBreakpoint:(int)memSpace addr:(int)address set:(BOOL)set;
// get break point state
-(int)getBreakpointState:(int)memSpace addr:(int)address;

// ----- Snapshot -----
// read a snapshot from a file
-(void)loadSnapshot:(NSString *)path;
// write a snapshot to a file
-(void)saveSnapshot:(NSString *)path withROMS:(BOOL)saveRoms andDisks:(BOOL)saveDisks;
// scan for quick snapshot files
- (NSString *)getQuickSnapshotFileName:(BOOL)load;
// read a quick snapshot from a file
-(void)loadQuickSnapshot;
// save a quick snapshot to a file
-(void)saveQuickSnapshot;

// ----- History -----
-(BOOL)startRecordHistory;
-(BOOL)stopRecordHistory;
-(BOOL)startPlaybackHistory;
-(BOOL)stopPlaybackHistory;
-(BOOL)isRecordingHistory;
-(BOOL)isPlayingBackHistory;
-(BOOL)setRecordMilestone;
-(BOOL)resetRecordMilestone;

// ----- Media -----
// start recording media/save screen shot
-(BOOL)startRecordingMedia:(NSString *)driver 
                fromCanvas:(int)canvasId
                    toFile:(NSString *)path;
// is recording a screenshot
-(BOOL)isRecordingMedia;
// stop recording screenshot
-(void)stopRecordingMedia;
// enumerate media
-(NSArray *)enumMediaDrivers;
// has driver container
-(BOOL)mediaDriverHasFormats:(NSString *)driver;
// enumerate media formats
-(NSArray *)enumMediaFormats:(NSString *)driver;
// extension for media 
-(NSString *)defaultExtensionForMediaDriver:(NSString *)driver andFormat:(NSString *)format;

// ----- Keyboard -----
// key pressed
-(void)keyPressed:(unsigned int)code;
// key release
-(void)keyReleased:(unsigned int)code;
// dump keymap
-(BOOL)dumpKeymap:(NSString *)path;

// ----- Mouse & Lightpen -----
// mouse moved
-(void)mouseMoveToX:(float)x andY:(float)y;
// mouse pressed
-(void)mouseButton:(BOOL)left withState:(BOOL)pressed;
// lightpen update
-(void)lightpenUpdateOnScreen:(int)screen toX:(int)x andY:(int)y withButton1:(BOOL)b1 andButton2:(BOOL)b2;

// ----- Drive Commands -----
// disk image
-(BOOL)smartAttachImage:(NSString *)path withProgNum:(int)num andRun:(BOOL)run;
// attach disk image
-(BOOL)attachDiskImage:(int)unit path:(NSString *)path;
// detach disk image
-(void)detachDiskImage:(int)unit;
// create disk image
-(BOOL)createDiskImage:(int)type path:(NSString *)path name:(NSString *)name;

// query disk name
-(NSString *)getDiskName:(int)unit;

// ----- Tape Commands -----
// attach tape image
-(BOOL)attachTapeImage:(NSString *)path;
// detach tape image
-(void)detachTapeImage;
// control datasette
-(void)controlDatasette:(int)command;

// ----- Printer -----
// send form feed (unit #4=0,#5=1,Userport=2)
-(void)printerFormFeed:(int)unit;

// ----- Cut & Paste -----
//! read output of screen into a string
-(NSString *)readScreenOutput;
//! type string on keyboard
-(void)typeStringOnKeyboard:(NSString *)string toPetscii:(BOOL)convert;

// ----- Fliplist -----
//! load flip list
-(BOOL)loadFliplist:(int)unit path:(NSString *)path autoAttach:(BOOL)autoAttach;
//! save flip list
-(BOOL)saveFliplist:(int)unit path:(NSString *)path;
//! add current image to fliplist
-(void)addCurrentToFliplist:(int)unit;
//! remove image from fliplist
-(void)removeFromFliplist:(int)unit path:(NSString *)path;
//! attach next
-(void)attachNextInFliplist:(int)unit direction:(BOOL)next;

// ----- Netplay -----
//! start server
-(BOOL)startNetplayServer;
//! connect to client
-(BOOL)connectNetplayClient;
//! disconnect
-(void)disconnectNetplay;
//! netplay mode (see network.h: network_mode_t)
-(int)getNetplayMode;

// ----- ImageContents -----
//! get disk image contents
-(NSDictionary *)diskimageContents:(NSString *)path;
//! get tape image contents
-(NSDictionary *)tapeimageContents:(NSString *)path;

@end

