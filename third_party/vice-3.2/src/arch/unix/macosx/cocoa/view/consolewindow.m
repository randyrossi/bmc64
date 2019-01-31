/*
 * consolewindow.m - ConsoleWindow
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

#import "consolewindow.h"
#import "logview.h"

@implementation ConsoleWindow

#define BUFFER_SIZE 102400

- (id)initWithContentRect:(NSRect)rect title:(NSString *)title
{
    unsigned int style = NSUtilityWindowMask | NSTitledWindowMask |
        NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask;
    self = [super initWithContentRect:rect
                            styleMask:style
                              backing:NSBackingStoreBuffered
                                defer:NO];
    if (self==nil)
        return nil;

    [self setReleasedWhenClosed:NO];
    [self setFloatingPanel:NO];
    // set title
    [self setTitle:title];
    [self setFrameAutosaveName:[self title]];

    // create scroll view
    scroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0,
                                                            NSWidth(rect), NSHeight(rect))];
    [scroll setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];
    [scroll setHasVerticalScroller:YES];
    [scroll setHasHorizontalScroller:NO];
    
    NSSize size = [scroll contentSize];

    // embed a text view
    log_view = [[LogView alloc] initWithFrame:NSMakeRect(0, 0, size.width, size.height)];
    [log_view setFont:[NSFont userFixedPitchFontOfSize:0]];
    [log_view setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];
    [log_view setEditable:NO];
    [log_view setHorizontallyResizable:YES];
    [[log_view textContainer] setContainerSize:NSMakeSize(FLT_MAX, FLT_MAX)];
    [[log_view textContainer] setWidthTracksTextView:YES];

    [scroll setDocumentView:log_view];
    [self setContentView:scroll];

    // init log file
    log_file = 0;
    log_pipe = nil;

    buffer = [[NSMutableString alloc] initWithCapacity:BUFFER_SIZE];

    return self;
}

- (void)appendText:(NSString*)text
{
    if (text==nil)
        return;

    [buffer appendString:text];

    if (([buffer length] >= BUFFER_SIZE))
        [self flushBuffer];
}

- (void)appendPrompt:(NSString*)prompt
{
    NSRange end = [log_view rangeForUserTextChange];
    [log_view replaceCharactersInRange:end withString:prompt];

    NSRange new = NSMakeRange(end.location, [prompt length]-1);
    [log_view setTextColor:[NSColor blueColor] range:new];

    NSRange newEnd = [log_view rangeForUserTextChange];
    [log_view scrollRangeToVisible:newEnd];
}

- (void)flushBuffer
{
    if (buffer.length==0)
        return;

    NSRange end = [log_view rangeForUserTextChange];
    [log_view.textStorage beginEditing];
    [log_view replaceCharactersInRange:end withString:buffer];
    [log_view.textStorage endEditing];
    NSRange newEnd = [log_view rangeForUserTextChange];
    [log_view scrollRangeToVisible:newEnd];    

    [buffer setString:@""];
 }

- (void)dealloc
{    
    [scroll release];
    [log_view release];

    fclose(log_file);
    [log_pipe dealloc];

    [buffer release];
    [super dealloc];
}

- (void)handleAsyncRead:(NSNotification*)notification
{
    NSData * data = [[notification userInfo]
        objectForKey:NSFileHandleNotificationDataItem];

    [self appendText:[[[NSString alloc] initWithBytes:[data bytes] 
                                              length:[data length]
                                              encoding:NSUTF8StringEncoding] autorelease]];
    [[notification object] readInBackgroundAndNotify];
}

- (int)fdForWriting
{
    // cleate a pipe
    log_pipe = [[NSPipe alloc] init];
    NSFileHandle * fh = [log_pipe fileHandleForReading];
    [[NSNotificationCenter defaultCenter]
        addObserver:self
           selector:@selector(handleAsyncRead:)
               name:NSFileHandleReadCompletionNotification
             object:fh];
    [fh readInBackgroundAndNotify];

    // return file descriptor for console writing 
    return [[log_pipe fileHandleForWriting] fileDescriptor];
}

// ----- input -----

- (BOOL)canBecomeMainWindow
{
    return NO;
}

- (void)setLineInputTarget:(id<LineInputSubmitter>)target
{
    lineInputTarget = target;
}

- (void)endConsoleInput:(id)sender
{
    NSString *lineInput = [sender lastInput];
    // no input means an empty string
    if (lineInput == nil)
        [lineInputTarget submitLineInput:@""];
    else
        [lineInputTarget submitLineInput:lineInput];
}

- (void)beginLineInputWithPrompt:(NSString *)prompt
{
    [self makeKeyAndOrderFront:nil];

    if (buffer!=nil)
        [self flushBuffer];
    [self appendPrompt:prompt];

    [log_view setTarget:self];
    [log_view setAction:@selector(endConsoleInput:)];
    [log_view setEditable:YES];
}

- (void)endLineInput
{
    [log_view setEditable:NO];    
}

@end
