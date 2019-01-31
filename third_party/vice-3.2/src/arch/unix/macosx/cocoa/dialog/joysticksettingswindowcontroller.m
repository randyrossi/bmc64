/*
 * joysticksettingswindowcontroller.m - JoystickSettings dialog controller
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
#include "archdep.h"

#import "joysticksettingswindowcontroller.h"
#import "viceapplication.h"
#import "vicenotifications.h"

static char *keyNames[KEYSET_SIZE] = {
    "North",
    "South",
    "West",
    "East",
    "NorthWest",
    "SouthWest",
    "NorthEast",
    "SouthEast",
    "Fire"
};

@implementation JoystickSettingsWindowController

-(id)init
{
    self = [super initWithWindowNibName:@"JoystickSettings"];
    if (!self) {
        return self;
    }

    [self registerForResourceUpdate:@selector(updateResources:)];

    // force linkage of key press view
    [KeyPressView class];

    return self;
}

-(void)setupHidHat
{
    [hidHat removeAllItems];

#ifdef HAS_JOYSTICK
    int i;
    for(i=0;i<JOYSTICK_DESCRIPTOR_MAX_HAT_SWITCHES;i++) {
        [hidHat addItemWithTitle:[NSString stringWithFormat:@"Hat Switch #%d",i+1]];
    }
#endif

    [hidHat addItemWithTitle:@"Disabled"];
}

-(int)findHidAxis:(NSString *)name
{
#ifdef HAS_JOYSTICK
    // fetch axis map from joy driver
    joy_hid_axis_info_t *ptr = joy_hid_axis_infos;
    int num = 0;
    while(ptr->name != NULL) {
        NSString *axis_name = [NSString stringWithCString:ptr->name encoding:NSUTF8StringEncoding];
        if([axis_name compare:name]==NSOrderedSame) {
            return num;
        }
        num++;
        ptr++;
    }
#endif
    return -1;
}

-(void)setupHidAxis
{
    [hidXAxis removeAllItems];
    [hidYAxis removeAllItems];

#ifdef HAS_JOYSTICK
    // fetch axis map from joy driver
    joy_hid_axis_info_t *ptr = joy_hid_axis_infos;
    while(ptr->name != NULL) {
        NSString *name = [NSString stringWithCString:ptr->name encoding:NSUTF8StringEncoding];
        [hidXAxis addItemWithTitle:name];
        [hidYAxis addItemWithTitle:name];
        ptr++;
    }
#endif

    [hidXAxis addItemWithTitle:@"Disabled"];
    [hidYAxis addItemWithTitle:@"Disabled"];
}

-(void)setupHidDeviceList
{
    [hidName removeAllItems];
    [hidName addItemWithTitle:@"Autodetect"];

#ifdef HAS_JOYSTICK
    const joy_hid_device_array_t *devices = joy_hid_get_devices();
    if (devices != NULL) {
        int num_devices = devices->num_devices;
        int i;
        for (i = 0; i < num_devices; i++) {
           joy_hid_device_t *dev = &devices->devices[i];
           NSString *desc = [NSString stringWithFormat:@"%04x:%04x:%d %s",
               dev->vendor_id, dev->product_id, dev->serial, dev->product_name];
           [hidName addItemWithTitle:desc];
        }
    }
#endif
}

-(void)windowDidLoad
{
    // define buttons array
    keyButtons[KEYSET_NORTH] = keySetNorth;
    keyButtons[KEYSET_SOUTH] = keySetSouth;
    keyButtons[KEYSET_WEST] = keySetWest;
    keyButtons[KEYSET_EAST] = keySetEast;
    keyButtons[KEYSET_NORTHWEST] = keySetNorthWest;
    keyButtons[KEYSET_SOUTHWEST] = keySetSouthWest;
    keyButtons[KEYSET_NORTHEAST] = keySetNorthEast;
    keyButtons[KEYSET_SOUTHEAST] = keySetSouthEast;
    keyButtons[KEYSET_FIRE] = keySetFire;

    hidButtons[HID_FIRE] = hidFire;
    hidButtons[HID_ALT_FIRE] = hidAltFire;
    hidButtons[HID_LEFT] = hidWest;
    hidButtons[HID_RIGHT] = hidEast;
    hidButtons[HID_UP] = hidNorth;
    hidButtons[HID_DOWN] = hidSouth;
    
    hidAutoButtons[0] = hidAFA;
    hidAutoButtons[1] = hidAFB;
    hidAutoPress[0]   = hidAFAPress;
    hidAutoPress[1]   = hidAFBPress;
    hidAutoRelease[0] = hidAFARelease;
    hidAutoRelease[1] = hidAFBRelease;

#ifdef HAS_JOYSTICK    
    // enable joy ports
    [joystick1Mode setEnabled:(joy_num_ports > 0)];
    [joystick2Mode setEnabled:(joy_num_ports > 1)];
    [joystickExtra1Mode setEnabled:(joy_num_extra_ports > 0)];
    [joystickExtra2Mode setEnabled:(joy_num_extra_ports > 1)];
#endif

    [self setupHidAxis];
    [self setupHidHat];
    [self setupHidDeviceList];

    [self updateResources:nil];

    [super windowDidLoad];
}

// ---------- update ----------

-(void)updateKeysetDisplay
{
    // key set setup
    int keySetNum = [keySetSelect indexOfSelectedItem];
    int i;
    for (i = 0; i < KEYSET_SIZE; i++) {
        NSString *res1 = [NSString stringWithFormat:@"KeySet%d%s",
                          keySetNum + 1, keyNames[i]];
        int keyCode1 = [self getIntResource:res1];
        NSString *val1;
        if (keyCode1 == ARCHDEP_KEYBOARD_SYM_NONE) {
            val1 = @"N/A";
        } else {
            val1 = [NSString stringWithFormat:@"%04x",keyCode1];
        }
        [keyButtons[i] setTitle:val1];
    }
}

-(void)updateHidName
{
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];
    NSString *device = [self getStringResource:
         [NSString stringWithFormat:@"Joy%cDevice", 'A' + hidDeviceNum]];

    if ([device length]==0) {
        // select "automatic"
        [hidName selectItemAtIndex:0];
    } else {
        int pos;
        int found = -1;
        for (pos = 0; pos < [hidName numberOfItems]; ++pos) {
            NSString *title = [hidName itemTitleAtIndex:pos];
            if ([title hasPrefix:device]) {
                found = pos;
                break;
            }
        }
        if (found!=-1) {
            [hidName selectItemAtIndex:pos];
        } else {
            // reset to automatic
            [hidName selectItemAtIndex:0];
            [self setStringResource:
                 [NSString stringWithFormat:@"Joy%cDevice",'A'+hidDeviceNum] toValue:@""];
        }
    }
}

-(int)isXLogical
{
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];
    return [self getIntResource:
        [NSString stringWithFormat:@"Joy%cXLogical", 'A' + hidDeviceNum]];    
}

-(void)updateHidXAxis
{
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];

    NSString *axisName = [self getStringResource:
        [NSString stringWithFormat:@"Joy%cXAxis", 'A' + hidDeviceNum]];
    [hidXAxis selectItemWithTitle:axisName];
    if ([hidXAxis selectedItem] == nil) {
        [hidXAxis selectItem:[hidXAxis lastItem]];
    }

    int threshold = [self getIntResource:
        [NSString stringWithFormat:@"Joy%cXThreshold", 'A' + hidDeviceNum]];
    [hidXThreshold setIntValue:threshold];

    int logical = [self isXLogical];
    [hidXLogical setState:logical ? NSOnState : NSOffState];

    int min = [self getIntResource:
        [NSString stringWithFormat:@"Joy%cXMin", 'A' + hidDeviceNum]];
    [hidXMin setIntValue:min];

    int max = [self getIntResource:
        [NSString stringWithFormat:@"Joy%cXMax", 'A' + hidDeviceNum]];
    [hidXMax setIntValue:max];
}

-(int)isYLogical
{
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];
    return [self getIntResource:
        [NSString stringWithFormat:@"Joy%cYLogical", 'A' + hidDeviceNum]];    
}

-(void)updateHidYAxis
{
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];

    NSString *axisName = [self getStringResource:
        [NSString stringWithFormat:@"Joy%cYAxis", 'A' + hidDeviceNum]];
    [hidYAxis selectItemWithTitle:axisName];
    if ([hidYAxis selectedItem] == nil) {
        [hidYAxis selectItem:[hidYAxis lastItem]];
    }

    int threshold = [self getIntResource:
        [NSString stringWithFormat:@"Joy%cYThreshold", 'A' + hidDeviceNum]];
    [hidYThreshold setIntValue:threshold];

    int logical = [self isYLogical];
    [hidYLogical setState:logical ? NSOnState : NSOffState];

    int min = [self getIntResource:
        [NSString stringWithFormat:@"Joy%cYMin", 'A' + hidDeviceNum]];
    [hidYMin setIntValue:min];

    int max = [self getIntResource:
        [NSString stringWithFormat:@"Joy%cYMax", 'A' + hidDeviceNum]];
    [hidYMax setIntValue:max];
}

-(void)getHidButtons:(int *)ids
{
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];
    NSString *buttons = [self getStringResource:
         [NSString stringWithFormat:@"Joy%cButtons", 'A' + hidDeviceNum]];
    const char *buttonsStr = [buttons cStringUsingEncoding:NSUTF8StringEncoding];
    sscanf(buttonsStr,"%d:%d:%d:%d:%d:%d", &ids[0], &ids[1], &ids[2], &ids[3], &ids[4], &ids[5]);
}

-(void)updateHidButtons
{
    int i;
    int ids[HID_NUM_BUTTONS] = { 0, 0, 0, 0, 0, 0 };
    [self getHidButtons:ids];
    for (i = 0; i < 6; i++) {
        NSString *desc;
        if (ids[i] == HID_INVALID_BUTTON) {
            desc = @"N/A";
        } else {
            desc = [NSString stringWithFormat:@"%d", ids[i]];
        }
        [hidButtons[i] setTitle:desc];
    }
}

-(void)getHidAutoButtons:(int *)ids
{
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];
    NSString *buttons = [self getStringResource:
         [NSString stringWithFormat:@"Joy%cAutoButtons", 'A' + hidDeviceNum]];
    const char *buttonsStr = [buttons cStringUsingEncoding:NSUTF8StringEncoding];
    sscanf(buttonsStr,"%d:%d:%d:%d:%d:%d", &ids[0], &ids[1], &ids[2], &ids[3], &ids[4], &ids[5]);    
}

-(void)setHidAutoButtons:(int *)ids
{
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];
    NSString *value = [NSString stringWithFormat:@"%d:%d:%d:%d:%d:%d",
        ids[0], ids[1], ids[2], ids[3], ids[4], ids[5]];
    [self setStringResource:
         [NSString stringWithFormat:@"Joy%cAutoButtons", 'A' + hidDeviceNum]
         toValue:value];
}

-(void)updateHidAutoButtons
{
    int i;
    int ids[6] = { 0, 0, 0, 0, 0, 0 };
    [self getHidAutoButtons:ids];
    
    int offset = HID_NUM_AUTO_BUTTONS;
    for(i=0;i<HID_NUM_AUTO_BUTTONS;i++) {
        NSString *desc;
        if (ids[i] == HID_INVALID_BUTTON) {
            desc = @"N/A";
        } else {
            desc = [NSString stringWithFormat:@"%d", ids[i]];
        }
        [hidAutoButtons[i] setTitle:desc];        
    
        int pressTime = ids[offset++];
        int releaseTime = ids[offset++];
        [hidAutoPress[i] setIntValue:pressTime];
        [hidAutoRelease[i] setIntValue:releaseTime];
    }
}

-(void)updateHidHat
{
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];

    int hat = [self getIntResource:
        [NSString stringWithFormat:@"Joy%cHatSwitch", 'A' + hidDeviceNum]];
    if(hat != 0) {
        [hidHat selectItemAtIndex:hat - 1];
    } else {
        [hidHat selectItem:[hidHat lastItem]];
    }
}

-(void)updateResources:(NSNotification *)notification
{
    int joy1Mode = [self getIntResource:@"JoyDevice1"];
    int joy2Mode = [self getIntResource:@"JoyDevice2"];
    [joystick1Mode selectItemAtIndex:joy1Mode];
    [joystick2Mode selectItemAtIndex:joy2Mode];

    [self updateKeysetDisplay];
    [self toggleHidDevice:nil];
}

// ----- Actions -----

-(IBAction)changeJoystick1Mode:(id)sender
{
    int joy1Mode = [joystick1Mode indexOfSelectedItem];
    [self setIntResource:@"JoyDevice1" toValue:joy1Mode];
}

-(IBAction)changeJoystick2Mode:(id)sender
{
    int joy2Mode = [joystick2Mode indexOfSelectedItem];
    [self setIntResource:@"JoyDevice2" toValue:joy2Mode];
}

-(IBAction)changeJoystickExtra1Mode:(id)sender
{
    int joy3Mode = [joystickExtra1Mode indexOfSelectedItem];
    [self setIntResource:@"JoyDevice3" toValue:joy3Mode];
}

-(IBAction)changeJoystickExtra2Mode:(id)sender
{
    int joy4Mode = [joystickExtra2Mode indexOfSelectedItem];
    [self setIntResource:@"JoyDevice4" toValue:joy4Mode];
}

-(IBAction)toggleKeyset:(id)sender
{
    [self updateKeysetDisplay];
}

-(IBAction)clearKeyset:(id)sender
{
    int keySetNum = [keySetSelect indexOfSelectedItem];

    int i;
    for (i = 0; i < KEYSET_SIZE; i++) {
        NSString *res = [NSString stringWithFormat:@"KeySet%d%s", keySetNum + 1, keyNames[i]];
        [self setIntResource:res toValue:ARCHDEP_KEYBOARD_SYM_NONE];
    }
    
    [self updateKeysetDisplay];
}

-(IBAction)defineKeysetButton:(id)sender
{
    int i;
    for (i = 0; i < KEYSET_SIZE; i++) {
        if (keyButtons[i] == sender) {
            break;
        }
    }
    if (i == KEYSET_SIZE) {
        return;
    }

    int keyCode = [keyPressView keyCode];
    int keySetNum = [keySetSelect indexOfSelectedItem];

    NSString *res = [NSString stringWithFormat:@"KeySet%d%s", keySetNum + 1, keyNames[i]];
    [self setIntResource:res toValue:keyCode];
    
    NSString *val = [NSString stringWithFormat:@"%04x", keyCode];
    [keyButtons[i] setTitle:val];
}

-(IBAction)toggleHidDevice:(id)sender
{
    [self updateHidName];
    [self updateHidXAxis];
    [self updateHidYAxis];
    [self updateHidButtons];
    [self updateHidAutoButtons];
    [self updateHidHat];
}

-(IBAction)refreshHidList:(id)sender
{
#ifdef HAS_JOYSTICK
    joy_reload_device_list();
#endif    

    [self setupHidDeviceList];
    [self updateHidName];
}

-(IBAction)pickHidName:(id)sender
{
    NSString *joyDevice;
    
    // automatic
    if ([hidName indexOfSelectedItem] == 0) {    
        joyDevice = @"";
    } 
    // picked device
    else {
        NSString *value = [hidName titleOfSelectedItem];
        NSArray *components = [value componentsSeparatedByString:@" "];
        joyDevice = (NSString *)[components objectAtIndex:0];
    }
    
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];
    NSString *tag = [NSString stringWithFormat:@"Joy%cDevice", 'A' + hidDeviceNum];
    [self setStringResource:tag toValue:joyDevice];

    // update display
    [self updateHidName];
}

-(void)resetHidXAxisRange:(NSString *)axisName
{
#ifdef HAS_JOYSTICK
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];
    joystick_descriptor_t *joy = hidDeviceNum ? &joy_b : &joy_a;
    joy_reset_axis_range(joy, HID_X_AXIS);
#endif
}

-(void)resetHidYAxisRange:(NSString *)axisName
{
#ifdef HAS_JOYSTICK
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];
    joystick_descriptor_t *joy = hidDeviceNum ? &joy_b : &joy_a;
    joy_reset_axis_range(joy, HID_Y_AXIS);
#endif    
}

-(IBAction)pickXAxis:(id)sender
{
    NSString *axisValue;
    if ([hidXAxis selectedItem] == [hidXAxis lastItem]) {
        axisValue = @"";
    } else {
        axisValue = [hidXAxis titleOfSelectedItem];
    }

    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];        
    [self setStringResource:[NSString stringWithFormat:@"Joy%cXAxis", 'A' + hidDeviceNum]
         toValue:axisValue];

    [self resetHidXAxisRange:axisValue];
    [self updateHidXAxis];
}

-(IBAction)pickYAxis:(id)sender
{
    NSString *axisValue;
    if ([hidXAxis selectedItem] == [hidYAxis lastItem]) {
        axisValue = @"";
    } else {
        axisValue = [hidYAxis titleOfSelectedItem];
    }

    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];        
    [self setStringResource:[NSString stringWithFormat:@"Joy%cYAxis", 'A' + hidDeviceNum]
         toValue:axisValue];

    [self resetHidYAxisRange:axisValue];
    [self updateHidYAxis];
}

-(IBAction)detectXAxis:(id)sender
{
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];
    NSString *axisName = @"";
#ifdef HAS_JOYSTICK
    int logical = [self isXLogical];
    int usage = joy_hid_detect_axis(hidDeviceNum ? &joy_b : &joy_a, HID_X_AXIS, logical);
    if(usage != -1) {
        const char *axisNameC = joy_hid_get_axis_name(usage);
        if (axisNameC != NULL) {
            axisName = [NSString stringWithCString:axisNameC encoding:NSUTF8StringEncoding];
        }
    }
#endif

    [self setStringResource:[NSString stringWithFormat:@"Joy%cXAxis", 'A' + hidDeviceNum]
        toValue:axisName];

    [self resetHidXAxisRange:axisName];
    [self updateHidXAxis];
}

-(IBAction)detectYAxis:(id)sender
{
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];
    NSString *axisName = @"";
#ifdef HAS_JOYSTICK
    int logical = [self isYLogical];
    int usage = joy_hid_detect_axis(hidDeviceNum ? &joy_b : &joy_a, HID_Y_AXIS, logical);
    if(usage != -1) {
        const char *axisNameC = joy_hid_get_axis_name(usage);
        if (axisNameC != NULL) {
            axisName = [NSString stringWithCString:axisNameC encoding:NSUTF8StringEncoding];
        }
    }
#endif

    [self setStringResource:[NSString stringWithFormat:@"Joy%cYAxis", 'A' + hidDeviceNum]
        toValue:axisName];

    [self resetHidYAxisRange:axisName];
    [self updateHidYAxis];
}

-(IBAction)setXThreshold:(id)sender
{
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];
    int threshold = [hidXThreshold intValue];
    if ((threshold>0)&&(threshold<100)) {
      [self setIntResource:[NSString stringWithFormat:@"Joy%cXThreshold", 'A' + hidDeviceNum]
          toValue:threshold];
    }
    [self updateHidXAxis];
}

-(IBAction)setYThreshold:(id)sender
{
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];
    int threshold = [hidYThreshold intValue];
    if ((threshold > 0) && (threshold < 100)) {
      [self setIntResource:[NSString stringWithFormat:@"Joy%cYThreshold", 'A' + hidDeviceNum]
          toValue:threshold];
    }
    [self updateHidYAxis];
}

-(IBAction)setXMin:(id)sender
{
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];
    int min = [hidXMin intValue];
    [self setIntResource:[NSString stringWithFormat:@"Joy%cXMin", 'A' + hidDeviceNum]
          toValue:min];
    [self updateHidXAxis];
}

-(IBAction)setXMax:(id)sender
{
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];
    int max = [hidXMax intValue];
    [self setIntResource:[NSString stringWithFormat:@"Joy%cXMax", 'A' + hidDeviceNum]
          toValue:max];
    [self updateHidXAxis];
}

-(IBAction)setYMin:(id)sender
{
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];
    int min = [hidYMin intValue];
    [self setIntResource:[NSString stringWithFormat:@"Joy%cYMin", 'A' + hidDeviceNum]
          toValue:min];
    [self updateHidYAxis];
}

-(IBAction)setYMax:(id)sender
{
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];
    int max = [hidYMax intValue];
    [self setIntResource:[NSString stringWithFormat:@"Joy%cYMax", 'A' + hidDeviceNum]
          toValue:max];
    [self updateHidYAxis];
}

-(IBAction)toggleXLogical:(id)sender
{
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];
    int logical = ([hidXLogical state] == NSOnState);
    [self setIntResource:[NSString stringWithFormat:@"Joy%cXLogical", 'A' + hidDeviceNum]
          toValue:logical];
    
    NSString *axisName = [self getStringResource:[NSString stringWithFormat:@"Joy%cXAxis", 'A' + hidDeviceNum]];
    [self resetHidXAxisRange:axisName];
    [self updateHidXAxis];    
}

-(IBAction)toggleYLogical:(id)sender
{
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];
    int logical = ([hidYLogical state] == NSOnState);
    [self setIntResource:[NSString stringWithFormat:@"Joy%cYLogical", 'A' + hidDeviceNum]
          toValue:logical];

    NSString *axisName = [self getStringResource:[NSString stringWithFormat:@"Joy%cYAxis", 'A' + hidDeviceNum]];
    [self resetHidYAxisRange:axisName];
    [self updateHidYAxis];        
}

-(IBAction)defineHidButton:(id)sender
{
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];
    int buttonId  = [sender tag];
    int buttonVal = 0;

#ifdef HAS_JOYSTICK
    buttonVal = joy_hid_detect_button(hidDeviceNum ? &joy_b : &joy_a);
    if(buttonVal == -1)
        buttonVal = 0;
#endif

    int ids[HID_NUM_BUTTONS] = { 0, 0, 0, 0, 0, 0 };
    [self getHidButtons:ids];
    ids[buttonId] = buttonVal;
    
    [self setStringResource:[NSString stringWithFormat:@"Joy%cButtons", 'A' + hidDeviceNum]
        toValue:[NSString stringWithFormat:@"%d:%d:%d:%d:%d:%d",
            ids[0], ids[1], ids[2], ids[3], ids[4], ids[5]]];
    
    [self updateHidButtons];
}

-(IBAction)pickHat:(id)sender
{
    int hat;
    if ([hidHat selectedItem] == [hidHat lastItem]) {
        hat = 0;
    } else {
        hat = [hidHat indexOfSelectedItem] + 1;
    }

    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];        
    [self setIntResource:[NSString stringWithFormat:@"Joy%cHatSwitch", 'A' + hidDeviceNum]
         toValue:hat];

    [self updateHidHat];
}

-(IBAction)detectHat:(id)sender
{
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];
    int hat = 0;
#ifdef HAS_JOYSTICK
    hat = joy_hid_detect_hat_switch(hidDeviceNum ? &joy_b : &joy_a);
    if(hat == -1)
        hat = 0;
#endif

    [self setIntResource:[NSString stringWithFormat:@"Joy%cHatSwitch", 'A' + hidDeviceNum]
        toValue:hat];

    [self updateHidHat];
}

-(IBAction)defineAFButton:(id)sender
{
    int tag = [sender tag];
 
    int hidDeviceNum = [hidDeviceSelect indexOfSelectedItem];
    int button = 0;
#ifdef HAS_JOYSTICK
    button = joy_hid_detect_button(hidDeviceNum ? &joy_b : &joy_a);
    if(button == -1)
        button = 0;
#endif

    int ids[6];
    [self getHidAutoButtons:ids];
    ids[tag] = button;
    [self setHidAutoButtons:ids];
    
    [self updateHidAutoButtons];
}

-(IBAction)setAFParam:(id)sender
{
    int value = [sender intValue];
    int pos = [sender tag] + HID_NUM_AUTO_BUTTONS;
    
    int ids[6];
    [self getHidAutoButtons:ids];
    ids[pos] = value;
    [self setHidAutoButtons:ids];
    
    [self updateHidAutoButtons];
}

@end
