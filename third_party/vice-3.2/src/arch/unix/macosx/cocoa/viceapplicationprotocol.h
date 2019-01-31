/*
 * viceapplicationprotocol.h - VICEApplicationProtocol - protocol for app
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

/* The VICEApplicationProtocol specifies the Object protocol that the VICE
Applications offers for the machine thread. An UI interaction of the machine
thread is passed through this protocol.

 The VICEApplicationProtocol is implemented by the VICEApplicaiton class. */

@protocol VICEApplicationProtocol

// ----- setup -----
// call back by machine object to report its own instance on stat up
-(void)setMachine:(id)aMachineObject;

// ----- termination -----
// machine did stop
-(void)machineDidStop;

// ----- video -----
// create a new canvas
-(void)createCanvas:(NSData *)canvasPtr withSize:(NSSize)size;
// destroy a canvas
-(void)destroyCanvas:(NSData *)canvasPtr;
// resize a canvase
-(void)resizeCanvas:(NSData *)canvasPtr withSize:(NSSize)size;
// reconfigure a canvas
-(void)reconfigureCanvas:(NSData *)canvasPtr;

// ----- monitor -----
// open the monitor
-(void)openMonitor;
// close the monitor
-(void)closeMonitor;

// suspend monitor
-(void)suspendMonitor;
// resume monitor
-(void)resumeMonitor;

// print monitor message
-(void)printMonitorMessage:(NSString *)msg;

// begin entering line
-(void)beginLineInputWithPrompt:(NSString *)prompt;
// end entering line
-(void)endLineInput;

// update monitor state
-(void)updateMonitor;

// ----- notifications -----
// post a remove notification
- (void)postRemoteNotification:(NSArray *)array;

// ----- alerts & dialogs -----
// show error message
- (void)runErrorMessage:(NSString *)message;
// show warning message
- (void)runWarningMessage:(NSString *)message;
// show CPU jam dialog
- (int)runCPUJamDialog:(NSString *)message;
// show extend image dialog
- (BOOL)runExtendImageDialog;
// get a open file name
- (NSString *)getOpenFileName:(NSString *)title types:(NSArray *)types;

@end

