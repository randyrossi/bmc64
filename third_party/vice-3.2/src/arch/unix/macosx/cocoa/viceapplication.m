/*
 * viceapplication.m - VICEApplication - main application class
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

#include "videoarch.h"
#include "viewport.h"
#include "uiapi.h"
#include "monitor.h"
#include "viewport.h"
#include "videoparam.h"

#import "viceapplication.h"
#import "vicenotifications.h"
#import "vicemachine.h"
#import "vicewindow.h"
#import "consolewindow.h"

// from archdep.c:
extern int default_log_fd;

// initial width of control panel
const float control_win_width = 200;

#define TAG_CPU_REGISTERS       0
#define TAG_CPU_MEMORY          1
#define TAG_CPU_DISASSEMBLY     2
#define TAG_CPU_IOTREE          3

@implementation VICEApplication

// ----- Setup windows -----

- (void)setupDebuggerWindowsMenu
{
    NSMenuItem *item;
    
    // CPU Registers
    item = [[NSMenuItem alloc] initWithTitle:@"CPU Registers"
                                      action:@selector(toggleDebuggerWindow:)
                               keyEquivalent:@""];
    [item setTag:TAG_CPU_REGISTERS];
    [debuggerWindowsMenu addItem:item];
    
    // CPU Memory
    item = [[NSMenuItem alloc] initWithTitle:@"CPU Memory"
                                      action:@selector(toggleDebuggerWindow:)
                               keyEquivalent:@""];
    [item setTag:TAG_CPU_MEMORY];
    [debuggerWindowsMenu addItem:item];

    // CPU Disassembly
    item = [[NSMenuItem alloc] initWithTitle:@"CPU Disassembly"
                                      action:@selector(toggleDebuggerWindow:)
                               keyEquivalent:@""];
    [item setTag:TAG_CPU_DISASSEMBLY];
    [debuggerWindowsMenu addItem:item];
    
    // CPU IO Tree
    item = [[NSMenuItem alloc] initWithTitle:@"CPU IO Tree"
                                      action:@selector(toggleDebuggerWindow:)
                               keyEquivalent:@""];
    [item setTag:TAG_CPU_IOTREE];
    [debuggerWindowsMenu addItem:item];
}

// setup window controlles
- (void)setupDebuggerWindowControllers
{
    cpuRegisterWC = [[RegisterWindowController alloc] initWithMemSpace:e_comp_space];
    cpuMemoryWC = [[MemoryWindowController alloc] initWithMemSpace:e_comp_space];
    cpuDisassemblyWC = [[DisassemblyWindowController alloc] initWithMemSpace:e_comp_space];
    cpuIOTreeWC = [[IOTreeWindowController alloc] initWithMemSpace:e_comp_space];
}

- (void)deallocDebuggerWindowControllers
{
    [cpuRegisterWC release];
    [cpuMemoryWC release];
    [cpuDisassemblyWC release];
    [cpuIOTreeWC release];
}

// ----- User Defaults -----

- (void)storeWindowOrder
{
    NSUserDefaults *def = [NSUserDefaults standardUserDefaults];

    // find my z order
    NSArray *wins = [[NSApplication sharedApplication] orderedWindows];
    int i;
    int num = [wins count];
    for(i=0;i<num;i++) {
        VICEWindow *win = (VICEWindow *)[wins objectAtIndex:i];
        NSString *title = [win title];
        NSString *tag = [title stringByAppendingString:@"OrderBelow"];
        int winId = [win canvasId];
        
        // find canvas with lover id that sits right above us
        int j;
        int foundWinId = 0;
        for(j=0;j<i;j++) {
            VICEWindow *preWin = (VICEWindow *)[wins objectAtIndex:j];
            int preWinId = [preWin canvasId];
            if(preWinId < winId) {
                foundWinId = preWinId + 1;
                break;
            }
        }

        // store in prefs above which canvas we will be ordered
        // use 0 to order window in front of all windows
        [def setInteger:foundWinId forKey:tag];
    }
}

- (BOOL)restoreWindowOrder:(NSWindow *)window
{
    NSUserDefaults *def = [NSUserDefaults standardUserDefaults];
    NSString *title = [window title];
    NSString *tag = [title stringByAppendingString:@"OrderBelow"];
    int belowId = [def integerForKey:tag];
    
    if(belowId == 0) {
        // order myself front and make me key window
        [window orderFront:self];
        return TRUE;
    } else {
        belowId --;
        // find canvas to 
        NSArray *wins = [[NSApplication sharedApplication] orderedWindows];
        int i;
        int num = [wins count];
        VICEWindow *aboveWin = nil;
        for(i=0;i<num;i++) {
            VICEWindow *preWin = (VICEWindow *)[wins objectAtIndex:i];
            int preWinId = [preWin canvasId];
            if(preWinId == belowId) {
                aboveWin = preWin;
                break;
            }
        }
        if(aboveWin != nil) {
            // found window: order belo
            [window orderWindow:NSWindowBelow relativeTo:[aboveWin windowNumber]];
            return FALSE;
        } else {
            // Fatal: didn't find window to be ordered below
            [window orderFront:self];
            return TRUE;
        }
    }
}

- (BOOL)isWindowVisible:(NSWindow *)window default:(BOOL)show
{
    NSString *title = [window title];
    NSString *key = [title stringByAppendingString:@"Visible"];
    NSUserDefaults *def = [NSUserDefaults standardUserDefaults];

    BOOL visible;
    if ([def objectForKey:key] == nil) {
        visible = show;
    } else {
        visible = [def boolForKey:key];
    }
    return visible;
}

- (void)storeWindowVisibility:(NSWindow *)window visible:(BOOL)visible
{
    NSString *title = [window title];
    NSString *key = [title stringByAppendingString:@"Visible"];
    NSUserDefaults *def = [NSUserDefaults standardUserDefaults];

    [def setBool:visible forKey:key];
}

- (void)setWindowVisibilityFromUserDefaults:(NSWindow *)window default:(BOOL)show
{
    if ([self isWindowVisible:window default:show]) {
        [window orderFront:self];
    }
}

- (void)storeWindowVisibilityToUserDefaults:(NSWindow *)window
{
    [self storeWindowVisibility:window visible:[window isVisible]];
}

// ----- Tool Windows and Controllers -----

// console place
- (NSRect)placeConsole:(BOOL)left
{
    // bottom left or right
    NSRect screenRect = [[NSScreen mainScreen] visibleFrame];
    float screenWidth = NSWidth(screenRect);
    float screenHeight = NSHeight(screenRect);
    float w = screenWidth * 0.5;
    float h = screenHeight * 0.3;
    if (left)
        return NSMakeRect(NSMinX(screenRect),NSMinY(screenRect),w,h);
    else
        return NSMakeRect(NSMinX(screenRect)+w,NSMinY(screenRect),w,h);
}

// control place
- (NSRect)placeControl
{
    // top left
    NSRect screenRect = [[NSScreen mainScreen] visibleFrame];
    float top_pos = NSMinY(screenRect) + NSHeight(screenRect);
    return NSMakeRect(NSMinX(screenRect),
                      top_pos - 10,
                      control_win_width,
                      10);
}

// setup console window controllers
- (void)setupToolWindowControllers
{
    // create default log console
    consoleWindow = [[ConsoleWindow alloc] 
         initWithContentRect:[self placeConsole:TRUE]
                       title:@"Console"];
    consoleWC = [[VICEWindowController alloc] initWithWindow:consoleWindow showOnDefault:YES];

    // create monitor window
    monitorWindow = [[ConsoleWindow alloc] 
         initWithContentRect:[self placeConsole:FALSE]
                       title:@"Monitor"];
    monitorWC = [[VICEWindowController alloc] initWithWindow:monitorWindow showOnDefault:NO];
    
    // create control window
    NSWindow *controlWindow = [[ControlWindow alloc] 
         initWithContentRect:[self placeControl]
                       title:@"Control"];
    controlWC = [[VICEWindowController alloc] initWithWindow:controlWindow showOnDefault:YES];
}

- (void)deallocToolWindowControllers
{
    [consoleWC release];
    [monitorWC release];
    [controlWC release];
}

// ----- Startup -----

// initial start up of application
- (void)runWithArgC:(int)argc argV:(char**)argv
{
    // build args array
    argsArray = [[NSMutableArray alloc] init];
    int i;
    for (i=0;i<argc;i++) {
        [argsArray addObject:[NSString stringWithCString:argv[i] encoding:NSUTF8StringEncoding]];
    }

    postponeAutostart = YES;
    inMonitor = FALSE;

    // enter run loop here
    [super run];
}

// application is ready to run, so fire up machine thread
- (void)applicationDidFinishLaunching:(NSNotification*)aNotification
{
    // reset state
    machine = nil;
    canTerminate = NO;
    canvasCount = 0;
    
    // set visibility of windows
    [self setupToolWindowControllers];

    // set as new default console
    default_log_fd = [consoleWindow fdForWriting];

    // create connection with 2 ports
    NSPort *port1 = [NSPort port];
    NSPort *port2 = [NSPort port];
    NSConnection *con = [[NSConnection alloc] initWithReceivePort:port1
                                                         sendPort:port2];
    [con setRootObject:self];
 
    // build an array for machine thread
    NSArray *args = [NSArray arrayWithObjects:
        port2,port1,argsArray,nil];
    
    // start thread
    [NSThread detachNewThreadSelector:@selector(startConnected:)
                             toTarget:[VICEMachine class] 
                           withObject:args];

    // setup window controllers
    [self setupDebuggerWindowControllers];
    [self setupDebuggerWindowsMenu];
}

// initial callback from machine thread after startup
-(void)setMachine:(id)aMachineObject
{
    // get machine object proxy back from connection
    [aMachineObject setProtocolForProxy:@protocol(VICEMachineProtocol)];
    machine = (id<VICEMachineProtocol>)[aMachineObject retain];
    postponeAutostart = NO;
}

// ----- Query Machine/Controller -----

- (id<VICEMachineProtocol>)machine
{
    return machine;
}

// access the machine proxy object from the app thread
+ (id<VICEMachineProtocol>)theMachine
{
    VICEApplication *app = (VICEApplication *)[self sharedApplication];
    return [app machine];
}

- (VICEMachineController *)machineController
{
    return [machine getMachineController];
}

+ (VICEMachineController *)theMachineController
{
    VICEApplication *app = (VICEApplication *)[self sharedApplication];
    return [app machineController];
}

// appController

- (VICEAppController *)appController
{
    return appController;
}

+ (VICEAppController *)theAppController
{
    VICEApplication *app = (VICEApplication *)[self sharedApplication];
    return [app appController];
}

// ----- Termination -----

// ask the user if the application should be terminated
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)app
{
    // machine thread is dead so we can actuall die
    if (canTerminate)
        return NSTerminateNow;

    // machine thread is not dead now so terminate it...
    VICEMachineController *controller = [machine getMachineController];

    // ask only if user query is enabled
    NSNumber *confirmOnExit = [controller getIntResource:@"ConfirmOnExit"];
    if ([confirmOnExit intValue]) {
        int result = NSRunAlertPanel(@"Quit Application",
                                     @"Do you really want to exit?",
                                     @"Yes",@"No",nil);
        if (result==NSAlertAlternateReturn)
            return NSTerminateCancel;
    }

    // save resources?
    NSNumber *saveResourcesOnExit = [controller getIntResource:@"SaveResourcesOnExit"];
    if ([saveResourcesOnExit intValue]) {
        if (![controller saveResources:nil]) {
            NSRunAlertPanel(@"Error",@"Error saving resources!",@"Ok",nil,nil);
        }
    }

    // save window visibility
    [self storeWindowOrder];

    // is monitor still running?
    if ([machine isWaitingForLineInput]) {
        // exit monitor first
        [machine submitLineInput:@"x"];
    }
    
    // tell machine thread to shutdown and exit
    [machine stopMachine];

    // always cancel in this stage as the threadWillExit will trigger the
    // final terminate
    return NSTerminateCancel;
}

// shutdown application
- (void)applicationWillTerminate:(NSNotification*)notification
{
    [self deallocToolWindowControllers];
    [self deallocDebuggerWindowControllers];
}

// machine did stop
- (void)machineDidStop
{
    canTerminate = YES;
    [NSApp terminate:nil];
}

// ----- File Drops or File Start -----

// a file was dropped
- (BOOL)application:(NSApplication*)app openFile:(NSString*)file
{
    // if the machine is not ready yet then postpone autostart
    if (postponeAutostart==YES) {
        // add file to args if not already there
        if ([argsArray indexOfObject:file]==NSNotFound) {
            [argsArray addObject:file];
        }
        return YES;
    } else {
        return [[machine getMachineController] smartAttachImage:[NSString stringWithString:file]
                                                    withProgNum:0 andRun:YES];
    }
}

// ----- Video -----

-(NSRect)placeCanvas:(NSSize)size
{
    // fetch screen size
    NSRect screenRect = [[NSScreen mainScreen] visibleFrame];

    if (canvasCount==1) {
        canvasStartXPos = NSMinX(screenRect) + control_win_width;
    } else {
    }

    int sizeFactor;

    // find a size factor for initial window
    float factorX = NSWidth(screenRect) - canvasStartXPos;
    float factorY = NSHeight(screenRect);
    factorX /= size.width;
    factorY /= size.height;
    if(factorX < factorY) {
        sizeFactor = (int)factorX;
    } else {
        sizeFactor = (int)factorY;
    }

    // not full screen so we can see log window
    sizeFactor --;

    // ensure sane values
    if(sizeFactor < 1) {
        sizeFactor = 1;
    } else if(sizeFactor > 4) {
        sizeFactor = 4;
    }

    float top_pos = NSMinY(screenRect) + NSHeight(screenRect);
    float canvasY = top_pos - size.height;     
    NSRect rect = NSMakeRect(canvasStartXPos,
                              canvasY,
                              size.width  * sizeFactor,
                              size.height * sizeFactor);

    // place next canvas
    canvasStartXPos += size.width * sizeFactor;

    return rect;
}

-(void)createCanvas:(NSData *)canvasPtr withSize:(NSSize)size
{
    video_canvas_t *canvas = *(video_canvas_t **)[canvasPtr bytes];
    
    canvasCount++;
    NSString *title;
    if (canvasCount==1)
        title = [NSString stringWithCString:canvas->viewport->title encoding:NSUTF8StringEncoding];
    else
        title = [NSString stringWithFormat:@"%s #%d",canvas->viewport->title,canvasCount];
    
    // adjust desired window size for pixel aspect
    float pixel_aspect_ratio = 1.0f;
    NSSize desiredWindowSize = size;
    if(canvas->video_param->true_pixel_aspect) {
        pixel_aspect_ratio = canvas->geometry->pixel_aspect_ratio;
        desiredWindowSize.width *= pixel_aspect_ratio;
    }

    // create a new vice window (and a glView)
    VICEWindow *window = [[VICEWindow alloc] initWithContentRect:[self placeCanvas:desiredWindowSize]
                                                           title:title
                                                      canvasSize:size
                                                pixelAspectRatio:pixel_aspect_ratio];

    // setup embedded gl view
    VICEGLView *glView = [window getVICEGLView];
    [glView setCanvasId:canvas->canvasId];
    [glView setPixelAspectRatio:pixel_aspect_ratio];

    // fill canvas structure for rendering
    canvas->window = window;
    canvas->view   = glView;
    canvas->pitch  = [glView getCanvasPitch];
    canvas->depth  = [glView getCanvasDepth];
    
    // (re)configure view for the first time
    [glView reconfigureCanvas:canvas->video_param];
    
    // is visible?
    BOOL visible = [self isWindowVisible:window default:TRUE];
    BOOL isFront = [self restoreWindowOrder:window];
    if(isFront) {
        [window makeKeyWindow];
    }
    if (!visible) {
        [window miniaturize:self];
    }

    // activate app if not already done
    if (![self isActive])
        [self activateIgnoringOtherApps:YES];
}

-(void)destroyCanvas:(NSData *)canvasPtr
{
    video_canvas_t *canvas = *(video_canvas_t **)[canvasPtr bytes];
    
    // release vice windows
    NSWindow *window = canvas->window;
    if (window!=nil) {
        // store canvas state
        [self storeWindowVisibility:window visible:[window isVisible]];
        
        // close window
        [window close];
    }
}

-(void)resizeCanvas:(NSData *)canvasPtr withSize:(NSSize)size
{
    video_canvas_t *canvas = *(video_canvas_t **)[canvasPtr bytes];

    // retrieve active pixel aspect ratio
    float pixel_aspect_ratio = 1.0f;
    if(canvas->video_param->true_pixel_aspect) {
        pixel_aspect_ratio = canvas->geometry->pixel_aspect_ratio;
    }
    
    // resize canvas window (and glView)
    [canvas->window resizeCanvas:size pixelAspectRatio:pixel_aspect_ratio];

    // update canvas parameters for rendering
    VICEGLView *glView = [canvas->window getVICEGLView];
    canvas->pitch = [glView getCanvasPitch];
}

-(void)reconfigureCanvas:(NSData *)canvasPtr
{
    video_canvas_t *canvas = *(video_canvas_t **)[canvasPtr bytes];
    VICEGLView *glView = [canvas->window getVICEGLView];
    [glView reconfigureCanvas:canvas->video_param];
}

- (void)setCurrentCanvasId:(int)c
{
    currentCanvasId = c;
}

- (int)currentCanvasId
{
    return currentCanvasId;
}

+ (void)setCurrentCanvasId:(int)canvasId
{
    VICEApplication *app = (VICEApplication *)[self sharedApplication];
    [app setCurrentCanvasId:canvasId];
}

+ (int)currentCanvasId
{
    VICEApplication *app = (VICEApplication *)[self sharedApplication];
    return [app currentCanvasId];
}

// ----- Monitor -----

// post a notification if the state of the monitor has changed
- (void)postMonitorStateNotification:(int)state
{
    NSDictionary * dict =
        [NSDictionary dictionaryWithObject:[NSNumber numberWithInt:state]
                                    forKey:@"state"];

    [[NSNotificationCenter defaultCenter] postNotificationName:VICEMonitorStateNotification
                                                        object:self
                                                      userInfo:dict];
}

// post a notification if the state of the monitor has changed
- (void)postMonitorUpdateNotification
{
    [[NSNotificationCenter defaultCenter] postNotificationName:VICEMonitorUpdateNotification
                                                        object:self
                                                      userInfo:nil];
}


// filter all actions if monitor is enabled
- (id)targetForAction:(SEL)anAction to:(id)aTarget from:(id)sender
{
    id result = [super targetForAction:anAction to:aTarget from:sender];

    // normal operation
    if (!inMonitor)
        return result;
        
    // if monitor is enabled then disable VICE Controller actions
    if (result == appController) {
        return nil;
    } else {
        return result;
    }
    
    return result;
}

// machine thread entered monitor mode and tells to open monitor window:
-(void)openMonitor
{
    // check if window is already open
    closeMonitor = ![monitorWindow isVisible];
    oldKeyWindow = [self keyWindow];
    [monitorWindow makeKeyAndOrderFront:self];
    
    inMonitor = YES;
    [self postMonitorStateNotification:VICEMonitorStateOn];
}

// machine thread leaves monitor mode:
-(void)closeMonitor
{
    inMonitor = NO;
    [self postMonitorStateNotification:VICEMonitorStateOff];
    
    if (closeMonitor) {
        [monitorWindow orderOut:self];
    }
    [oldKeyWindow makeKeyAndOrderFront:self];
}

// machine thread suspends monitor UI inputs (e.g. before a single step)
-(void)suspendMonitor
{
    inMonitor = NO;
    [self postMonitorStateNotification:VICEMonitorStateSuspend];    
}

// machine thread resumes monitor UI inputs (e.g. after a single step)
-(void)resumeMonitor
{
    inMonitor = YES;
    [self postMonitorStateNotification:VICEMonitorStateResume];
}

// some monitor values have changed. update views (e.g. mem window)
-(void)updateMonitor
{
    [self postMonitorUpdateNotification];
}

// print something in the monitor view
-(void)printMonitorMessage:(NSString *)msg
{
    [monitorWindow appendText:msg];
}

// maching thread wants some input:
-(void)beginLineInputWithPrompt:(NSString *)prompt
{
    [monitorWindow setLineInputTarget:self];
    [monitorWindow beginLineInputWithPrompt:prompt];
}

// called from the console window and forwarded to machine thread:
-(void)submitLineInput:(NSString *)result
{
    [machine submitLineInput:result];    
}

// machine thread tells to end input processing:
-(void)endLineInput
{
    [monitorWindow endLineInput];
}

// ----- Notifications -----

- (void)postRemoteNotification:(NSArray *)array 
{
    NSString *notificationName = (NSString *)[array objectAtIndex:0];
    NSDictionary *userInfo = (NSDictionary *)[array objectAtIndex:1];
    
    // post notification in our UI thread's default notification center
    [[NSNotificationCenter defaultCenter] postNotificationName:notificationName
                                                        object:self
                                                      userInfo:userInfo];
}

// ----- Console Window -----

- (void)toggleControlWindow:(id)sender
{
    [controlWC toggleWindow:sender];
}

- (void)toggleConsoleWindow:(id)sender
{
    [consoleWC toggleWindow:sender];
}

- (void)toggleMonitorWindow:(id)sender
{
    [monitorWC toggleWindow:sender];
}

- (void)toggleDebuggerWindow:(id)sender
{
    NSInteger tag = [sender tag];
    switch(tag) {
        case TAG_CPU_REGISTERS:
            [cpuRegisterWC toggleWindow:sender];
            break;
        case TAG_CPU_MEMORY:
            [cpuMemoryWC toggleWindow:sender];
            break;
        case TAG_CPU_DISASSEMBLY:
            [cpuDisassemblyWC toggleWindow:sender];
            break;
        case TAG_CPU_IOTREE:
            [cpuIOTreeWC toggleWindow:sender];
            break;
    }
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    SEL action = [menuItem action];
    
    if (action==@selector(toggleConsoleWindow:)) {
        [consoleWC checkMenuItem:menuItem];
    }
    else if (action==@selector(toggleMonitorWindow:)) {
        [monitorWC checkMenuItem:menuItem];
    }
    else if (action==@selector(toggleControlWindow:)) {
        [controlWC checkMenuItem:menuItem];
    }
    else if (action==@selector(toggleDebuggerWindow:)) {
        switch([menuItem tag]) {
            case TAG_CPU_REGISTERS:
                [cpuRegisterWC checkMenuItem:menuItem];
                break;
            case TAG_CPU_MEMORY:
                [cpuMemoryWC checkMenuItem:menuItem];
                break;
            case TAG_CPU_DISASSEMBLY:
                [cpuDisassemblyWC checkMenuItem:menuItem];
                break;
            case TAG_CPU_IOTREE:
                [cpuIOTreeWC checkMenuItem:menuItem];
                break;
        }
    }
    return [super validateMenuItem:menuItem];  
}

// ----- Alerts & Dialogs -----

+ (void)runErrorMessage:(NSString *)message
{
    VICEApplication *app = (VICEApplication *)[self sharedApplication];
    [app runErrorMessage:message];
}

+ (void)runWarningMessage:(NSString *)message
{
    VICEApplication *app = (VICEApplication *)[self sharedApplication];
    [app runWarningMessage:message];
}

- (void)runErrorMessage:(NSString *)message
{
    NSAlert *alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:@"Close"];
    [alert setMessageText:@"VICE Error!"];
    [alert setInformativeText:message];
    [alert runModal];
    [alert release];
}

- (void)runWarningMessage:(NSString *)message
{
    NSAlert *alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:@"Close"];
    [alert setMessageText:@"VICE Warning!"];
    [alert setInformativeText:message];
    [alert setAlertStyle:NSWarningAlertStyle];
    [alert runModal];
    [alert release];
}

- (int)runCPUJamDialog:(NSString *)message
{
    NSAlert *alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:@"Reset"];
    [alert addButtonWithTitle:@"Monitor"];
    [alert addButtonWithTitle:@"Hard Reset"];
    [alert setMessageText:@"VICE CPU JAM!"];
    [alert setInformativeText:message];
    int result = [alert runModal];
    [alert release];
    
    if (result==NSAlertFirstButtonReturn)
        return UI_JAM_RESET;
    else if (result==NSAlertSecondButtonReturn)
        return UI_JAM_MONITOR;
    else
        return UI_JAM_HARD_RESET;
}

- (BOOL)runExtendImageDialog
{
    NSAlert *alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:@"Yes"];
    [alert addButtonWithTitle:@"No"];
    [alert setMessageText:@"VICE Extend Image"];
    [alert setInformativeText:@"Extend image?"];
    int result = [alert runModal];
    [alert release];
    return result == NSAlertFirstButtonReturn;
}

- (NSString *)getOpenFileName:(NSString *)title types:(NSArray *)types
{
    return [[VICEApplication theAppController] getOpenFileName:title types:types];
}

@end
