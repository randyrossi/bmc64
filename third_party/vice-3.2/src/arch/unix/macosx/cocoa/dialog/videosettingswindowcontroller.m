/*
 * videosettingswindowcontroller.m - VideoSettings dialog controller
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

#include "vice.h"
#include "machine.h"

#import "videosettingswindowcontroller.h"
#import "viceapplication.h"
#import "vicenotifications.h"
#import "viceappcontroller.h"

static const char *vicii_palettes[] = {
    "default",
    "c64hq",
    "c64s",
    "ccs64",
    "frodo",
    "godot",
    "pc64",
    "vice",
    NULL
};

static const char *vic_palettes[] = {
    "default",
    NULL
};

static const char *crtc_palettes[] = {
    "amber",
    "green",
    "white",
    NULL
};

static const char *vdc_palettes[] = {
    "vdc_deft",
    "vdc_comp",
    NULL
};

static const char *ted_palettes[] = {
    "default",
    "vice",
    NULL
};

@implementation VideoSettingsWindowController

-(id)initWithChipName:(NSString *)name
{
    self = [super initWithWindowNibName:@"VideoSettings"];

    chipName = [name retain];

    // derive resource names from chip name
    paletteFileResource = [[NSString stringWithFormat:@"%@PaletteFile", chipName] retain];
    paletteExtResource = [[NSString stringWithFormat:@"%@ExternalPalette", chipName] retain];
    
    colorSaturationResource = [[NSString stringWithFormat:@"%@ColorSaturation", chipName] retain];
    colorContrastResource = [[NSString stringWithFormat:@"%@ColorContrast", chipName] retain];
    colorBrightnessResource = [[NSString stringWithFormat:@"%@ColorBrightness", chipName] retain];
    colorGammaResource = [[NSString stringWithFormat:@"%@ColorGamma", chipName] retain];
    
    palScanLineShadeResource = [[NSString stringWithFormat:@"%@PALScanLineShade", chipName] retain];
    palBlurResource = [[NSString stringWithFormat:@"%@PALBlur", chipName] retain];
    palOddLinePhaseResource = [[NSString stringWithFormat:@"%@PALOddLinePhase", chipName] retain];
    palOddLineOffsetResource = [[NSString stringWithFormat:@"%@PALOddLineOffset", chipName] retain];

    return self;
}

-(void)dispose
{
    [chipName release];
    
    [paletteFileResource release];
    [paletteExtResource release];
    
    [colorSaturationResource release];
    [colorContrastResource release];
    [colorBrightnessResource release];
    [colorGammaResource release];
    
    [palScanLineShadeResource release];
    [palBlurResource release];
    [palOddLinePhaseResource release];
    [palOddLineOffsetResource release]; 
}

-(void)initUI
{
    // set title of dialog
    NSString *title = [NSString stringWithFormat:@"%@ Video Settings", chipName];
    [[self window] setTitle:title];

    // pick palette
    paletteEntries = NULL;
    hasOddLines = YES;
    if([chipName isEqualToString:@"VIC-II"]) {
        paletteEntries = vicii_palettes;
    } else if([chipName isEqualToString:@"VIC"]) {
        paletteEntries = vic_palettes;
    } else if([chipName isEqualToString:@"Crtc"]) {
        paletteEntries = crtc_palettes;
        hasOddLines = NO;
    } else if([chipName isEqualToString:@"VDC"]) {
        paletteEntries = vdc_palettes;
        hasOddLines = NO;
    } else if([chipName isEqualToString:@"TED"]) {
        paletteEntries = ted_palettes; 
    }
    
    // enable controls
    if(!hasOddLines) {
        [oddLinePhaseText setEnabled:NO];
        [oddLinePhaseSlider setEnabled:NO];
        [oddLineOffsetText setEnabled:NO];
        [oddLineOffsetSlider setEnabled:NO];
    }
    
    // setup palette
    [palettePopup removeAllItems];
    if(paletteEntries != NULL) {
        const char **pal = paletteEntries;
        while (*pal != NULL) {
            [palettePopup addItemWithTitle:[NSString stringWithCString:*pal encoding:NSUTF8StringEncoding]];
            pal++;
        }
    }
}

-(void)windowDidLoad
{
    [self initUI];
    [self updateResources:nil];
    [self registerForResourceUpdate:@selector(updateResources:)];
    [super windowDidLoad];
}

// ----- Resources -----

-(void)updatePaletteResources
{
    // use external palette?
    BOOL usePal = [self getIntResource:paletteExtResource];
    [paletteToggle setState:usePal];

    NSString *palFile = [self getStringResource:paletteFileResource];

    // make sure palette entry is available
    if(palFile != nil) {
        if ([palettePopup itemWithTitle:palFile] == nil) {
            [palettePopup addItemWithTitle:palFile];
        }
        [palettePopup selectItemWithTitle:palFile];
    }
}   

-(void)updateColorResources
{
    int saturation = [self getIntResource:colorSaturationResource];
    int contrast = [self getIntResource:colorContrastResource];
    int brightness = [self getIntResource:colorBrightnessResource];
    int gamma = [self getIntResource:colorGammaResource];

    [saturationSlider setFloatValue:saturation];
    [saturationText   setFloatValue:saturation];
    [contrastSlider   setFloatValue:contrast];
    [contrastText     setFloatValue:contrast];
    [brightnessSlider setFloatValue:brightness];
    [brightnessText   setFloatValue:brightness];
    [gammaSlider      setFloatValue:gamma];
    [gammaText        setFloatValue:gamma];
}

-(void)updatePALResources
{
    int blur = [self getIntResource:palBlurResource];
    int scanlineShade = [self getIntResource:palScanLineShadeResource];

    [blurSlider            setFloatValue:blur];
    [blurText              setFloatValue:blur];
    [scanlineShadeSlider   setFloatValue:scanlineShade];
    [scanlineShadeText     setFloatValue:scanlineShade];

    if(hasOddLines) {
        int oddLinePhase = [self getIntResource:palOddLinePhaseResource];
        int oddLineOffset = [self getIntResource:palOddLineOffsetResource];
    
        [oddLinePhaseSlider    setFloatValue:oddLinePhase];
        [oddLinePhaseText      setFloatValue:oddLinePhase];
        [oddLineOffsetSlider   setFloatValue:oddLineOffset];
        [oddLineOffsetText     setFloatValue:oddLineOffset];
    }
}

-(void)updateResources:(NSNotification *)notification
{
    [self updatePaletteResources];
    [self updateColorResources];
    [self updatePALResources];
}

// ----- Actions -----

// --- Color ---

-(IBAction)slideSaturation:(id)sender
{
    [self setIntResource:colorSaturationResource toValue:[sender intValue]];
    [self updateColorResources];
}

-(IBAction)slideContrast:(id)sender
{
    [self setIntResource:colorContrastResource toValue:[sender intValue]];
    [self updateColorResources];
}

-(IBAction)slideBrightness:(id)sender
{
    [self setIntResource:colorBrightnessResource toValue:[sender intValue]];
    [self updateColorResources];
}

-(IBAction)slideGamma:(id)sender
{    
    [self setIntResource:colorGammaResource toValue:[sender intValue]];
    [self updateColorResources];
}

// FIXME: Setting resources from these TextFields is disabled because
// this code doesn't parse thousands separators properly
-(IBAction)enterSaturation:(id)sender
{
    // [self setIntResource:colorSaturationResource toValue:[sender intValue]];
    [self updateColorResources];
}

-(IBAction)enterContrast:(id)sender
{
    // [self setIntResource:colorContrastResource toValue:[sender intValue]];
    [self updateColorResources];
}

-(IBAction)enterBrightness:(id)sender
{
    // [self setIntResource:colorBrightnessResource toValue:[sender intValue]];
    [self updateColorResources];
}

-(IBAction)enterGamma:(id)sender
{
    // [self setIntResource:colorGammaResource toValue:[sender intValue]];
    [self updateColorResources];
}

// --- PAL Emu ---

-(IBAction)slideBlur:(id)sender
{
    [self setIntResource:palBlurResource toValue:[sender intValue]];
    [self updatePALResources];
}

-(IBAction)slideScanlineShade:(id)sender
{
    [self setIntResource:palScanLineShadeResource toValue:[sender intValue]];
    [self updatePALResources];
}

-(IBAction)slideOddLinePhase:(id)sender
{
    [self setIntResource:palOddLinePhaseResource toValue:[sender intValue]];
    [self updatePALResources];
}

-(IBAction)slideOddLineOffset:(id)sender
{
    [self setIntResource:palOddLineOffsetResource toValue:[sender intValue]];
    [self updatePALResources];
}

// FIXME: Setting resources from these TextFields is disabled as well
-(IBAction)enterBlur:(id)sender
{
    // [self setIntResource:palBlurResource toValue:[sender intValue]];
    [self updatePALResources];
}

-(IBAction)enterScanlineShade:(id)sender
{
    // [self setIntResource:palScanLineShadeResource toValue:[sender intValue]];
    [self updatePALResources];
}

-(IBAction)enterOddLinePhase:(id)sender
{
    // [self setIntResource:palOddLinePhaseResource toValue:[sender intValue]];
    [self updatePALResources];
}

-(IBAction)enterOddLineOffset:(id)sender
{
    // [self setIntResource:palOddLineOffsetResource toValue:[sender intValue]];
    [self updatePALResources];
}

// ----- Palette -----

-(IBAction)togglePalette:(id)sender
{
    BOOL on = [sender state];
    [self setIntResource:paletteExtResource toValue:on];
    [self updatePaletteResources];
}

-(IBAction)popupPalette:(id)sender
{
    NSString *item = [sender titleOfSelectedItem];
    [self setStringResource:paletteFileResource toValue:item];
    [self updatePaletteResources];
}

-(IBAction)pickPalette:(id)sender
{
    VICEAppController *appCtrl = [VICEApplication theAppController];
    NSString *path = [[appCtrl getFilePanel] pickOpenFileWithType:@"Palette"];
    if (path != nil) {
        [self setStringResource:paletteFileResource toValue:path];
        [self updatePaletteResources];
    }
}

@end
