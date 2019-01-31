/*
 * vicefilepanel.m - handle VICE open and save panels
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

#import "vicefilepanel.h"
#import "viceapplication.h"

@implementation VICEFilePanel

+ (NSArray *)getZippedFileExtensions
{
    return [NSArray arrayWithObjects: @"zip", @"bz2", @"gz", @"d6z", @"d7z", @"d8z", @"g6z", @"g4z", @"x6z", nil];
}

+ (NSArray *)getPaletteFileExtensions
{
    return [NSArray arrayWithObjects: @"vpl", nil];    
}

+ (NSArray *)getSnapshotFileExtensions
{
    return [NSArray arrayWithObjects: @"vsf", nil];    
}

+ (NSArray *)getProgramFileExtensions
{
    return [NSArray arrayWithObjects: @"prg", @"p00", nil];    
}

+ (NSArray *)getTapeImageFileExtensions
{
    return [NSArray arrayWithObjects: @"t64", @"tap", nil];    
}

+ (NSArray *)getDiskImageFileExtensions
{
    return [NSArray arrayWithObjects: @"d64" ,@"d71", @"d80", @"d81", @"d82", @"g64", @"g41", @"p64", @"x64", @"d1m", @"d2m", @"d4m", nil];
}

+ (NSArray *)getCBMImageFileExtensions
{
    return [NSArray arrayWithObjects: @"d64" ,@"d71", @"d80", @"d81", @"d82", @"g64", @"g41", @"p64", @"x64", @"d1m", @"d2m", @"d4m",
                                      @"t64", @"tap", 
                                      @"prg", @"p00", 
                                      @"zip", @"bz2", @"gz", @"d6z", @"d7z", @"d8z", @"g6z", @"g4z", @"p6z", @"x6z",
                                      nil];    
}

+ (NSArray *)getCRTFileExtensions
{
    return [NSArray arrayWithObjects: @"crt", nil];    
}

+ (NSArray *)getRawCartFileExtensions
{
    return [NSArray arrayWithObjects: @"bin", nil];    
}

+ (NSArray *)getFlipListFileExtensions
{
    return [NSArray arrayWithObjects: @"vfl", nil];    
}

+ (NSArray *)getROMSetFileExtensions
{
    return [NSArray arrayWithObjects: @"vrs", nil];    
}

+ (NSArray *)getROMSetArchiveExtensions
{
    return [NSArray arrayWithObjects: @"vra", nil];    
}

+ (NSArray *)getKeyMapFileExtensions
{
    return [NSArray arrayWithObjects: @"vkm", nil];    
}

+ (NSDictionary *)getExtensionMap
{
    return [NSDictionary dictionaryWithObjectsAndKeys:
        [VICEFilePanel getZippedFileExtensions],    @"Zipped",
        [VICEFilePanel getPaletteFileExtensions],   @"Palette",
        [VICEFilePanel getSnapshotFileExtensions],  @"Snapshot",
        [VICEFilePanel getProgramFileExtensions],   @"Program",
        [VICEFilePanel getTapeImageFileExtensions], @"TapeImage",
        [VICEFilePanel getDiskImageFileExtensions], @"DiskImage",
        [VICEFilePanel getCBMImageFileExtensions],  @"CBMImage",
        [VICEFilePanel getCRTFileExtensions],       @"CRT",
        [VICEFilePanel getRawCartFileExtensions],   @"RawCart",
        [VICEFilePanel getFlipListFileExtensions],  @"FlipList",
        [VICEFilePanel getROMSetFileExtensions],    @"ROMSet",
        [VICEFilePanel getROMSetArchiveExtensions], @"ROMSetArchive",
        [VICEFilePanel getKeyMapFileExtensions],    @"KeyMap",
        nil, nil];
}

- (id)init
{
    extensionMap = [VICEFilePanel getExtensionMap];
    [extensionMap retain];
    return self;
}

- (void)dealloc
{
    [imageContentsView release];
    [imageContentsEmpty release];
    
    [extensionMap release];
    [super dealloc];
}

- (void)resetPreview
{
    [imageContentsController setContent:imageContentsEmpty];
    [imageContentsHeader setStringValue:@""];
    [imageContentsFooter setStringValue:@""];    
}

- (void)panelSelectionDidChange:(id)sender
{
    // setup image display
    NSString *fileName = [sender filename];
    NSDictionary *dict = nil;
    if(fileName != nil) {
        dict = [[VICEApplication theMachineController] diskimageContents:fileName];
        if(dict == nil) {
            dict = [[VICEApplication theMachineController] tapeimageContents:fileName];            
        }
    }
    if(dict != nil) {
        // set header
        NSString *name = [dict objectForKey:@"name"];
        NSString *idy  = [dict objectForKey:@"id"];
        NSString *header = [NSString stringWithFormat:@"%@ %@", name, idy];
        [imageContentsHeader setStringValue:header];
        
        // set footer
        NSNumber *blocks_free = [dict objectForKey:@"blocks_free"];
        int blocks = [blocks_free intValue];
        NSString *footer;
        if(blocks>=0)
            footer = [NSString stringWithFormat:@"%d BLOCKS FREE", blocks];
        else
            footer = @"";
        [imageContentsFooter setStringValue:footer];
        
        NSArray *array = (NSArray *)[dict objectForKey:@"files"];
        [imageContentsController setContent:array];
    } else {
        [self resetPreview];
    }
}

- (NSString *)pickOpenFileWithPreviewAndType:(NSString *)type allowRun:(BOOL)allowRun
{
    NSArray *extensions = [extensionMap objectForKey:type];
    if(extensions == nil) {
        NSLog(@"wrong file extension key: %@", type);
        return nil;
    }
    
    NSString *title = [NSString stringWithFormat:@"Open %@", type];
    
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    [panel setAllowsMultipleSelection:NO];
    [panel setCanChooseFiles:YES];
    [panel setCanChooseDirectories:NO];
    [panel setAllowedFileTypes:extensions];
    [panel setTitle:title];
    [panel setDelegate:self]; 

    // load image contents view nib
    if(imageContentsView == nil) {
        [NSBundle loadNibNamed:@"FilePanelImageContents" owner:self];
        [imageContentsView retain];
        [self resetPreview];
    }
    [panel setAccessoryView:imageContentsView];

    // toggle autostart button
    if(imageContentsView != nil) {
        if(imageContentsEmpty == nil) {
            imageContentsEmpty = [[[NSArray alloc] init] retain];
        }
        [autostartButton setEnabled:allowRun];
    }

    // run dialog
    int result = [panel runModal];

    // reset result state
    NSString *fileName = nil;
    selectedProgNum = 0;
    selectedAutostart = NO;
    
    // pick up result
    if (result==NSFileHandlingPanelOKButton) {

        // fetch filename
        fileName = [[panel URL] path];

        if(imageContentsView != nil) {
            // get selected program item
            unsigned int contentsIndex = [imageContentsController selectionIndex];
            if(contentsIndex != NSNotFound) {
                selectedProgNum = (int)(contentsIndex + 1);
            }
        
            // update autostart
            selectedAutostart = ([autostartButton state] == NSOnState);
        }        
    }
    
    if(imageContentsView != nil) {    
        // restore controller
        [self resetPreview];
    }
     
    return fileName;
}

- (int)selectedProgNum
{
    return selectedProgNum;
}

- (BOOL)selectedAutostart
{
    return selectedAutostart;
}

- (NSString *)pickOpenFileWithType:(NSString *)type
{
    NSArray *extensions = [extensionMap objectForKey:type];
    if(extensions == nil) {
        NSLog(@"wrong file extension key: %@", type);
        return nil;
    }
    
    NSString *title = [NSString stringWithFormat:@"Open %@", type];
    
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    [panel setAllowsMultipleSelection:NO];
    [panel setCanChooseFiles:YES];
    [panel setCanChooseDirectories:NO];
    [panel setAllowedFileTypes:extensions];
    [panel setTitle:title];    
    
    int result = [panel runModal];
    if (result==NSFileHandlingPanelOKButton) {
        return [[panel URL] path];
    }    
    return nil;
}

- (NSString *)pickOpenFileWithTitle:(NSString *)title types:(NSArray *)types
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    [panel setAllowsMultipleSelection:NO];
    [panel setCanChooseFiles:YES];
    [panel setCanChooseDirectories:NO];
    [panel setAllowedFileTypes:types];
    [panel setTitle:title];    
    
    int result = [panel runModal];
    if (result==NSFileHandlingPanelOKButton) {
        return [[panel URL] path];
    }    
    return nil;
}

- (NSString *)pickSaveFileWithType:(NSString *)type
{
    NSArray *extensions = [extensionMap objectForKey:type];
    if(extensions == nil) {
        NSLog(@"wrong file extension key: %@", type);
        return nil;
    }
    
    NSString *title = [NSString stringWithFormat:@"Save %@", type];
    
    NSSavePanel *panel = [NSSavePanel savePanel];
    [panel setTitle:title];    
    [panel setAllowedFileTypes:extensions];
    
    int result = [panel runModal];
    if (result==NSFileHandlingPanelOKButton) {
        return [[panel URL] path];
    }    
    return nil;
}

- (NSString *)pickSaveFileWithTitle:(NSString *)title types:(NSArray *)types
{
    NSSavePanel *panel = [NSSavePanel savePanel];
    [panel setTitle:title];
    [panel setExtensionHidden:NO];
    if(types != nil) {  
        [panel setAllowedFileTypes:types];
    } else {
        [panel setAllowsOtherFileTypes:YES];
    }
    
    int result = [panel runModal];
    if (result==NSFileHandlingPanelOKButton) {
        return [[panel URL] path];
    }    
    return nil;
}

- (NSString *)pickDirectoryWithTitle:(NSString *)title
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    [panel setAllowsMultipleSelection:NO];
    [panel setCanChooseFiles:NO];
    [panel setCanChooseDirectories:YES];
    [panel setTitle:title];    
    
    int result = [panel runModal];
    if (result==NSFileHandlingPanelOKButton) {
        return [[panel URL] path];
    }    
    return nil;
}

- (NSArray *)pickAttachFileWithTitle:(NSString *)title andTypeDictionary:(NSDictionary *)types defaultType:(NSString *)defaultType
{
    NSOpenPanel * panel = [NSOpenPanel openPanel];
    
    NSView * accessories = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 280, 49)];
    [accessories autorelease];

    NSTextField * type_label = [[NSTextField alloc] initWithFrame:NSMakeRect(4, 18, 80, 17)];
    [type_label autorelease];
    [type_label setAlignment:NSRightTextAlignment];
    [type_label setEditable:NO];
    [type_label setBordered:NO];
    [type_label setDrawsBackground:NO];
    [type_label setStringValue:@"Type:"];

    NSPopUpButton * type_button = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(86, 12, 180, 26)];
    NSArray *sortedKeys = [[types allKeys] sortedArrayUsingSelector:@selector(compare:)];
    [type_button addItemsWithTitles:sortedKeys];
    [type_button autorelease];
    [type_button selectItemWithTitle:defaultType];

    [accessories addSubview:type_button];
    [accessories addSubview:type_label];

    [panel setAccessoryView:accessories];
    [panel setTitle:title];
    [panel setPrompt:@"Attach"];

    NSArray *result = nil;
    if ([panel runModal] == NSFileHandlingPanelOKButton) {
        NSString *item = [type_button titleOfSelectedItem];
        NSNumber *number = [types valueForKey:item];
        NSString *filename = [[panel URL] path];
        result = [NSArray arrayWithObjects:filename, number, nil];
    }
    return result;
}


@end
