/*
 * vicemachine.h - VICEMachine - the emulatated machine main thread/class
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

//some Cocoa headers give compile errors if DEBUG is defined without a value
#ifdef DEBUG
#undef DEBUG
#define DEBUG 1
#endif
#import <Cocoa/Cocoa.h>
#import "vicemachineprotocol.h"
#import "viceapplicationprotocol.h"
#import "vicemachinecontroller.h"
#import "vicemachinenotifier.h"

struct video_canvas_s;

@interface VICEMachine : NSObject <VICEMachineProtocol>
{
  id<VICEApplicationProtocol > app;
  NSAutoreleasePool *pool;
  BOOL shallIDie;
  BOOL isPaused;
  BOOL isSleepPaused;
  BOOL doMonitorInPause;
  NSTimeInterval pausePollInterval;
  
  BOOL escapeFromPause;
  BOOL keepEscaping;
  BOOL inFrameStep;
  
  VICEMachineController *machineController;
  VICEMachineNotifier *machineNotifier;

  #define MAX_CANVAS 2
  int canvasNum;
  struct video_canvas_s *canvasArray[MAX_CANVAS];

  BOOL isWaitingForLineInput;
  NSString *submittedLineInput;
}

// start the machine thread and establish connection
+(void)startConnected:(NSArray *)portArray;

// start the machine with established connection
-(void)startMachineWithArgs:(NSArray *)args
                        app:(id<VICEApplicationProtocol>)myApp 
                       pool:(NSAutoreleasePool *)myPool;

// trigger the machine thread's run loop and terminate thread if shallIDie is set
-(void)triggerRunLoop;

// activate the monitor in the pause loop
-(void)activateMonitorInPause;

// press a key in pause mode
-(void)keyPressedInPause:(unsigned int)code;

// release a key in pause mode
-(void)keyReleasedInPause:(unsigned int)code;

// trigger runloop and wait for input submission from UI thread
-(NSString *)lineInputWithPrompt:(NSString *)prompt timeout:(double)seconds;

// return the application
-(id<VICEApplicationProtocol>)app;

// machine specific *ui.m sets its machine controller
-(void)setMachineController:(VICEMachineController *)controller;

// access the notifier
-(VICEMachineNotifier *)machineNotifier;

// ----- Canvas Registry -----
// register a canvas and return id
-(int)registerCanvas:(struct video_canvas_s *)canvas;
// get canvas for id
-(struct video_canvas_s *)getCanvasForId:(int)canvasId;
// get num canvases
-(int)getNumCanvases;

@end

// the global machine thread's machine
extern VICEMachine *theVICEMachine;

