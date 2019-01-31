/*
 * vicemachine.m - VICEMachine - the emulated machine main thread/class
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

#include "main.h"
#include "vsync.h"
#include "machine.h"
#include "monitor.h"

#import "vicemachine.h"
#import "vicemachinecontroller.h"
#import "viceapplicationprotocol.h"

// our vice machine
VICEMachine *theVICEMachine = nil;

@implementation VICEMachine

- (void) receiveSleepNote: (NSNotification*) note
{
    // if not paused already then pause while system sleeps
    if (!isPaused) {
        isSleepPaused = YES;
        isPaused = YES;
    }
}

- (void) receiveWakeNote: (NSNotification*) note
{
    // wake up if the emulator was sleeping due to system sleep
    if (isSleepPaused) {
        isSleepPaused = NO;
        isPaused = NO;
    }
}

// initial start up of machine thread and setup of DO connection
+(void)startConnected:(NSArray *)portArray
{
    // create pool for machine thread
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    // establish own connection back to VICEApplication
    NSConnection *connection = [NSConnection
                connectionWithReceivePort:[portArray objectAtIndex:0]
                                 sendPort:[portArray objectAtIndex:1]];
 
    // create my own machine objct
    VICEMachine *viceMachine = [[self alloc] init];
    [connection setRootObject:viceMachine];

    // call back to app and report machine    
    id<VICEApplicationProtocol> viceApp = 
       (id<VICEApplicationProtocol>)[[connection rootProxy] retain];
    [viceApp setMachine:viceMachine];

    // extract args array
    NSArray *args = (NSArray *)[portArray objectAtIndex:2];
    [args retain];
 
    // run machine
    [viceMachine startMachineWithArgs:args app:viceApp pool:pool];
}

-(void)startMachineWithArgs:(NSArray *)args
                        app:(id<VICEApplicationProtocol>)myApp 
                       pool:(NSAutoreleasePool *)myPool
{
    // store important refs
    app = myApp;
    pool = myPool;
    
    // get command line args
    int argc = [args count];
    char **argv = (char **)malloc(sizeof(char *) * argc);
    int i;
    for (i=0;i<argc;i++) {
        NSString *str = (NSString *)[args objectAtIndex:i];
        argv[i] = strdup([str cStringUsingEncoding:NSUTF8StringEncoding]);
    }
    
    // store global
    theVICEMachine = self;
    shallIDie = NO;
    isPaused = NO;
    isSleepPaused = NO;
    isWaitingForLineInput = NO;
    doMonitorInPause = NO;
    pausePollInterval = 0.5;

    // frame step control via key board
    escapeFromPause = NO;
    keepEscaping = NO;
    inFrameStep = NO;

    machineController = nil;
    machineNotifier = [[VICEMachineNotifier alloc] init];

    // register sleep and wake up notifiers
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self 
            selector: @selector(receiveSleepNote:) name: NSWorkspaceWillSleepNotification object: NULL];
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self 
            selector: @selector(receiveWakeNote:) name: NSWorkspaceDidWakeNotification object: NULL];
    
    // enter VICE main program and main loop
    main_program(argc,argv);
    
    // never reach this point...
}

-(void)dealloc
{
    [machineNotifier release];
    [super dealloc];
}

-(id<VICEApplicationProtocol>)app
{
    return app;
}

-(void)stopMachine
{
    // flag to die after next run loop call
    shallIDie = YES;
    isPaused = NO;
}

-(BOOL)isPaused
{
    return isPaused;
}

-(BOOL)togglePause
{
    isPaused = !isPaused;
    return isPaused;
}

-(void)setPaused:(int)pause
{
    isPaused = !!pause;
}

-(void)activateMonitorInPause
{
    doMonitorInPause = YES;
}

// press a key in pause mode
-(void)keyPressedInPause:(unsigned int)code
{
    const NSTimeInterval iv[] = { 0,0.05,0.1,0.2,0.4,0.8 };
    
    if(!inFrameStep) {
        switch(code) {
            case 0x31: // SPACE - single frame skip
                escapeFromPause = YES;
                keepEscaping = NO;
                inFrameStep = YES;
                pausePollInterval = 0.1;
                break;
            case 0x05:
            case 0x04:
            case 0x03: // a,s,d,f,g,h - multi frame skip
            case 0x02:
            case 0x01:
            case 0x00:
                escapeFromPause = YES;
                keepEscaping = YES;
                inFrameStep = YES;
                pausePollInterval = iv[code];
                break;
            default:
                printf("Unknown Pause key: 0x%04x\n", code);
                break;
        }
    }
}

// release a key in pause mode
-(void)keyReleasedInPause:(unsigned int)code
{
    if(inFrameStep) {
        switch(code) {
            case 0x31: // SPACE
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03: // a,s,d,f,g,h
            case 0x04:
            case 0x05:
                inFrameStep = NO;
                escapeFromPause = NO;
                break;
        }
    } 
}

-(void)triggerRunLoop
{    
    // enter a pause loop?
    if (isPaused) {
        // suspend speed evalution
        vsync_suspend_speed_eval();
        
        // enter pause loop -> wait for togglePause triggered from UI thread
        while (isPaused) {
            // check for events
            [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode
                beforeDate:[NSDate dateWithTimeIntervalSinceNow:pausePollInterval]];
            
            // user requested to enter monitor during pause
            if(doMonitorInPause) {
                doMonitorInPause = false;
                monitor_startup_trap();
                // leave run loop -> monitor will be entered via trap
                return;
            }
            
            // shall we escape from pause?
            if(escapeFromPause) {
                if(!keepEscaping) {
                    escapeFromPause = NO;
                    pausePollInterval = 0.5;
                }
                return;
            }
        }
    }
    
    // run machine thread runloop once: 5ms
    [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode
                                beforeDate:[NSDate dateWithTimeIntervalSinceNow:0]];

    // the stop machine call triggered the die flag
    if (shallIDie) {
        // shut down VICE
        machine_shutdown();
    
        // tell ui that it the machine stopped
        [app machineDidStop];
        
        // run the final run loop until the thread dies
        while ( [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode
                    beforeDate:[NSDate distantFuture]] );
        
        // release pool for machine thread
        [pool release];
    }    
}

/* 
   the machine thread needs some input (e.g. in the monitor) and waits
   until either a time out occured or the UI submitted the requested
   input line.
   
   this is done asynchronously by enabling the input in the UI thread,
   running a local run loop in the machine thread until the time out
   occured or the UI submitted the string.
*/
-(NSString *)lineInputWithPrompt:(NSString *)prompt timeout:(double)seconds
{
    isWaitingForLineInput = YES;
    submittedLineInput = nil;
    
    // notify app to begin input with prompt
    [app beginLineInputWithPrompt:(NSString *)prompt];
    
    NSDate *finishedDate;
    if (seconds == 0)
        finishedDate = [NSDate distantFuture];
    else
        finishedDate = [NSDate dateWithTimeIntervalSinceNow:seconds];
    
    // run loop until time out or line input was submitted
    while (isWaitingForLineInput && 
            [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode
                                     beforeDate:finishedDate] )
    {}
    
    // end input in app
    [app endLineInput];
    
    NSString *result = submittedLineInput;
    submittedLineInput = nil;
    
    return result;
}

-(BOOL)isWaitingForLineInput
{
    return isWaitingForLineInput;
}

/* this is called by the UI thread to submit a line input */
-(void)submitLineInput:(NSString *)line
{
    if (isWaitingForLineInput) {
        submittedLineInput = [line retain];
        isWaitingForLineInput = NO;
    }
}

// ----- Canvas Management -----

-(int)registerCanvas:(struct video_canvas_s *)canvas
{
    if (canvasNum==MAX_CANVAS) {
        NSLog(@"FATAL: too many canvas registered!");
        return -1;
    }
    
    int canvasId = canvasNum;
    canvasNum++;
    canvasArray[canvasId] = canvas;
    return canvasId;
}

-(struct video_canvas_s *)getCanvasForId:(int)canvasId
{
    if ((canvasId<0)||(canvasId>=canvasNum))
        return NULL;
    return canvasArray[canvasId];
}

-(int)getNumCanvases
{
    return canvasNum;
}

// ---------- ViceMachineProtocol -------------------------------------------

// ----- Machine Controller -----

-(void)setMachineController:(VICEMachineController *)controller
{
    machineController = controller;
}

-(VICEMachineController *)getMachineController
{
    return machineController;
}

// ----- Machine Notifier -----

-(VICEMachineNotifier *)machineNotifier
{
    return machineNotifier;
}

@end

// ui_dispatch_events: call the run loop of the machine thread
void ui_dispatch_events()
{
    [theVICEMachine triggerRunLoop];
}

void ui_dispatch_next_event(void)
{
    [theVICEMachine triggerRunLoop];
}
