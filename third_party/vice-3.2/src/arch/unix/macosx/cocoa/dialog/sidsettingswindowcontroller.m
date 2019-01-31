/*
 * sidsettingswindowcontroller.m - SIDSettings dialog controller
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
#include "sid.h"

#import "sidsettingswindowcontroller.h"
#import "viceapplication.h"
#import "vicenotifications.h"

static const int c64baseaddress[] = { 0xd4, 0xd5, 0xd6, 0xd7, 0xde, 0xdf, -1 };
static const int c128baseaddress[] = { 0xd4, 0xd7, 0xde, 0xdf, -1 };
static const int cbm2baseaddress[] = { 0xda, -1 };

@implementation SIDSettingsWindowController

-(id)init
{
    self = [super initWithWindowNibName:@"SIDSettings"];
    if (!self) {
        return self;
    }

    // machine config
    stereoAddrs = NULL;
    if(machine_class == VICE_MACHINE_C64DTV) {
        hasFilters = YES;
        hasStereo  = NO;
    } else {
        hasFilters = YES;
        hasStereo  = YES;
        switch(machine_class) {
            case VICE_MACHINE_C64:
            case VICE_MACHINE_C64SC:
                stereoAddrs = c64baseaddress;
                break;
            case VICE_MACHINE_C128:
                stereoAddrs = c128baseaddress;
                break;
            case VICE_MACHINE_CBM6x0:
            case VICE_MACHINE_CBM5x0:
                stereoAddrs = cbm2baseaddress;
                break;
            default:
                hasStereo = NO;
                break;
        }
    }

    [self registerForResourceUpdate:@selector(updateResources:)];

    return self;
}

-(void)enableReSIDControls:(BOOL)on
{
    [residSampleMethodButton setEnabled:on];
    [residPassbandSlider setEnabled:on];
    [residPassbandText setEnabled:on];    
}

-(void)windowDidLoad
{
#ifndef HAVE_RESID
    // disable RESID
    [self enabledReSIDControls:NO];
#endif    
    
    // toggle stereo/filters
    [stereoSidEnableButton setEnabled:hasStereo];
    [stereoSidAddressButton setEnabled:hasStereo];
    [filterEnableButton setEnabled:hasFilters];
    
    // setup stereo addr
    if(stereoAddrs != NULL) {
        int i=0;
        while(stereoAddrs[i]!=-1) {
            int hi = stereoAddrs[i];
            int lo;
            for (lo = (i > 0 ? 0x0 : 0x20); lo < 0x100; lo += 0x20) {
                int addr = hi * 0x100 + lo;
                NSString *title = [NSString stringWithFormat:@"$%04X",addr];
                [stereoSidAddressButton addItemWithTitle:title];
            }
            i++;
        }
    }
    
    // setup sid model
    engine_model_list = sid_get_engine_model_list();
    int offset = 0;
    while(engine_model_list[offset] != NULL) {
        const char *name = engine_model_list[offset]->name;
        [engineModelButton addItemWithTitle:[NSString stringWithCString:name 
                                            encoding:NSUTF8StringEncoding]];
        offset++;
    }
    
    [self updateResources:nil];
    [super windowDidLoad];
}

-(void)updateResidPassbandResources
{
    int residPassband     = [self getIntResource:@"SidResidPassband"];
    [residPassbandSlider setIntValue:residPassband];
    [residPassbandText setIntValue:residPassband];    
}

-(void)updateResources:(NSNotification *)notification
{
    // set model
    int i = 0;
    int engine = [self getIntResource:@"SidEngine"];
    int model  = [self getIntResource:@"SidModel"];
    int pair   = engine << 8 | model;
    while(engine_model_list[i]->value != -1) {
        if(engine_model_list[i]->value == pair) {
            break;
        }
        i++;
    }

    [engineModelButton selectItemAtIndex:i];
    
    // is reSID enabled?
    BOOL reSIDenabled = engine == SID_ENGINE_RESID;
    [self enableReSIDControls:reSIDenabled];
    
    if(hasStereo) {
        int sidStereo = [self getIntResource:@"SidStereo"];
        [stereoSidEnableButton setState:(sidStereo ? NSOnState:NSOffState)];
        int sidStereoBase = [self getIntResource:@"SidStereoAddressStart"];
        NSString *title = [NSString stringWithFormat:@"$%04X",sidStereoBase];
        [stereoSidAddressButton selectItemWithTitle:title];
        [stereoSidAddressButton setEnabled:sidStereo ? NSOnState:NSOffState];
    }
    if(hasFilters) {
        int sidFilter = [self getIntResource:@"SidFilters"];
        [filterEnableButton setState:(sidFilter ? NSOnState : NSOffState)];
    }
    
    // reSID
#ifdef HAVE_RESID
    int residSampleMethod = [self getIntResource:@"SidResidSampling"];
    [residSampleMethodButton selectItemWithTag:residSampleMethod];
    [self updateResidPassbandResources];
#endif
}

// ----- Actions -----

-(IBAction)popupEngineModel:(id)sender
{
    int modelIndex = [engineModelButton indexOfSelectedItem];
    int pair = engine_model_list[modelIndex]->value;
    int engine = pair >> 8;
    int model  = pair & 0xff;
    [self setIntResource:@"SidModel" toValue:model];
    [self setIntResource:@"SidEngine" toValue:engine];
    [self updateResources:nil];
}

-(IBAction)toggleStereoSid:(id)sender
{
    int stereoSid = [sender state] == NSOnState;
    [self setIntResource:@"SidStereo" toValue:stereoSid];
    [self updateResources:nil];
}

-(IBAction)popupStereoSidAddress:(id)sender
{
    NSString *title = [sender title];
    int addr;
    sscanf([title cStringUsingEncoding:NSUTF8StringEncoding],"$%x",&addr);
    [self setIntResource:@"SidStereoAddressStart" toValue:addr];
}

-(IBAction)toggleSidFilter:(id)sender
{
    int sidFilter = ([sender state] == NSOnState);
    [self setIntResource:@"SidFilters" toValue:sidFilter];
}

-(IBAction)popupResidSampleMethod:(id)sender
{
    [self setIntResource:@"SidResidSampling" toValue:[sender tag]];
}

-(IBAction)slideResidPasspand:(id)sender
{
    [self setIntResource:@"SidResidPassband" toValue:[sender intValue]];
    [self updateResidPassbandResources];
}

-(IBAction)enterResidPassband:(id)sender
{
    [self setIntResource:@"SidResidPassband" toValue:[sender intValue]];
    [self updateResidPassbandResources];
}

@end
