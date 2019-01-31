/*
 * videosettingswindowcontroller.h - VideoSettings dialog controller
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

@interface VideoSettingsWindowController : VICEResourceWindowController
{
    IBOutlet NSTabViewItem *colorTab;
    IBOutlet NSTabViewItem *palEmuTab;
    IBOutlet NSTabViewItem *paletteTab;
    
    IBOutlet NSSlider *saturationSlider;
    IBOutlet NSSlider *contrastSlider;
    IBOutlet NSSlider *brightnessSlider;
    IBOutlet NSSlider *gammaSlider;

    IBOutlet NSTextField *saturationText;
    IBOutlet NSTextField *contrastText;
    IBOutlet NSTextField *brightnessText;
    IBOutlet NSTextField *gammaText;

    IBOutlet NSSlider *blurSlider;
    IBOutlet NSSlider *scanlineShadeSlider;
    IBOutlet NSSlider *oddLinePhaseSlider;
    IBOutlet NSSlider *oddLineOffsetSlider;

    IBOutlet NSTextField *blurText;
    IBOutlet NSTextField *scanlineShadeText;
    IBOutlet NSTextField *oddLinePhaseText;
    IBOutlet NSTextField *oddLineOffsetText;

    IBOutlet NSButton      *paletteToggle;
    IBOutlet NSPopUpButton *palettePopup;
    IBOutlet NSButton      *palettePick;
    
    NSString *chipName;
    
    BOOL hasOddLines;
    
    const char **paletteEntries;
    NSString *paletteFileResource;
    NSString *paletteExtResource;
    
    NSString *colorSaturationResource;
    NSString *colorContrastResource;
    NSString *colorBrightnessResource;
    NSString *colorGammaResource;

    NSString *palBlurResource;
    NSString *palScanLineShadeResource;
    NSString *palOddLinePhaseResource;
    NSString *palOddLineOffsetResource;
}

-(id)initWithChipName:(NSString *)name;
-(void)updateResources:(NSNotification *)notification;

// Actions
-(IBAction)slideSaturation:(id)sender;
-(IBAction)slideContrast:(id)sender;
-(IBAction)slideBrightness:(id)sender;
-(IBAction)slideGamma:(id)sender;

-(IBAction)enterSaturation:(id)sender;
-(IBAction)enterContrast:(id)sender;
-(IBAction)enterBrightness:(id)sender;
-(IBAction)enterGamma:(id)sender;

-(IBAction)slideBlur:(id)sender;
-(IBAction)slideScanlineShade:(id)sender;
-(IBAction)slideOddLinePhase:(id)sender;
-(IBAction)slideOddLineOffset:(id)sender;

-(IBAction)enterBlur:(id)sender;
-(IBAction)enterScanlineShade:(id)sender;
-(IBAction)enterOddLinePhase:(id)sender;
-(IBAction)enterOddLineOffset:(id)sender;

-(IBAction)togglePalette:(id)sender;
-(IBAction)popupPalette:(id)sender;
-(IBAction)pickPalette:(id)sender;

@end
