/*
 * drivesettingswindowcontroller.m - DriveSettings dialog controller
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
#include "drive.h"
#include "machine.h"

#import "drivesettingswindowcontroller.h"
#import "viceapplication.h"
#import "vicenotifications.h"

@implementation DriveSettingsWindowController

-(id)init
{
    self = [super initWithWindowNibName:@"DriveSettings"];
    [self registerForResourceUpdate:@selector(updateResources:)];

    int map[DRIVE_TYPE_NUM] = {
        DRIVE_TYPE_1540,   DRIVE_TYPE_1541,   DRIVE_TYPE_1541II,
        DRIVE_TYPE_1551,   DRIVE_TYPE_1570,   DRIVE_TYPE_1571,
        DRIVE_TYPE_1571CR, DRIVE_TYPE_1581,   DRIVE_TYPE_2031,
        DRIVE_TYPE_2040,   DRIVE_TYPE_3040,   DRIVE_TYPE_4040,
        DRIVE_TYPE_1001,   DRIVE_TYPE_8050,   DRIVE_TYPE_8250,
        DRIVE_TYPE_2000,   DRIVE_TYPE_4000
    };
    int i;
    numDriveTypes = DRIVE_TYPE_NUM;
    for (i = 0; i < numDriveTypes; i++) {
        driveTypeMap[i] = map[i];
    }

    // set machine drive setup
    driveOffset = 8;
    driveCount = 4;
    hasIEC = YES;
    hasParallel = YES;
    hasExpansion = YES;
    hasIdle = YES;
    switch(machine_class) {
        case VICE_MACHINE_PET:
        case VICE_MACHINE_CBM5x0:
        case VICE_MACHINE_CBM6x0:
            driveCount = 2;
            hasIEC = NO;
            hasParallel = NO;
            hasExpansion = NO;
            hasIdle = NO;
            break;
        case VICE_MACHINE_VIC20:
            hasParallel = NO;
            break;
    }
    
    return self;
}

-(void)windowDidLoad
{
    // setup tab view labels
    [driveChooser setSegmentCount:driveCount];
    int i;
    for (i = 0; i < driveCount; i++) {
        NSString *driveName = [NSString stringWithFormat:@"Drive %d", i + driveOffset];
        [driveChooser setLabel:driveName forSegment:i];
    }
    [driveChooser setSelectedSegment:0];
    
    // disable unused UI
    if(!hasParallel) {
        [parallelCable removeFromSuperview];
    }
    if(!hasExpansion) {
        [expansionBox removeFromSuperview];
    }
    if(!hasIdle) {
        [idleBox removeFromSuperview];
    }
    
    [self updateResources:nil];
    [super windowDidLoad];
}

-(void)updateResources:(NSNotification *)notification
{
    int trueEmu = [self getIntResource:@"DriveTrueEmulation"];
    int driveId  = [driveChooser selectedSegment];
    int driveNum = driveId + driveOffset;
    int driveEnabled;
    if(hasIEC) {
        int isIecDrive = [self getIntResource:@"IECDevice%d" withNumber:driveNum];
        driveEnabled = trueEmu && !isIecDrive;
    } else {
        driveEnabled = trueEmu;
    }
        
    // drive is enabled
    if (driveEnabled) {
        // type selector is enabled
        [driveType setEnabled:true];

        // enable valid drive types
        int j;
        for (j = 0; j < numDriveTypes; j++) {
            int isDriveValid = drive_check_type([self mapToDriveType:j],driveId);
            id cell = [driveType cellAtRow:j column:0];
            [cell setEnabled:isDriveValid];
        }
    
        // set current drive type
        int driveTypeVal = [self getIntResource:@"Drive%dType" withNumber:driveNum];
        int driveId = [self mapFromDriveType:driveTypeVal];
        [driveType selectCellAtRow:driveId column:0];

        // extend track policy
        int canExtendPolicy = drive_check_extend_policy(driveTypeVal);
        [trackHandling setEnabled:canExtendPolicy];
        int extendPolicyVal = [self getIntResource:@"Drive%dExtendImagePolicy" withNumber:driveNum];
        [trackHandling selectCellAtRow:extendPolicyVal column:0];

        // idle method
        if(hasIdle) {
            int canIdleMethod = drive_check_idle_method(driveTypeVal);
            [idleMethod setEnabled:canIdleMethod];
            int idleMethodVal = [self getIntResource:@"Drive%dIdleMethod" withNumber:driveNum];
            [idleMethod selectCellAtRow:idleMethodVal column:0];
        }

        // expansion ram
        if(hasExpansion) {
            int canRam,hasRam;
            canRam = drive_check_expansion2000(driveTypeVal);
            [driveExpansion_2000 setEnabled:canRam];
            hasRam = [self getIntResource:@"Drive%dRAM2000" withNumber:driveNum];
            [driveExpansion_2000 setState:hasRam];

            canRam = drive_check_expansion4000(driveTypeVal);
            [driveExpansion_4000 setEnabled:canRam];
            hasRam = [self getIntResource:@"Drive%dRAM4000" withNumber:driveNum];
            [driveExpansion_4000 setState:hasRam];

            canRam = drive_check_expansion6000(driveTypeVal);
            [driveExpansion_6000 setEnabled:canRam];
            hasRam = [self getIntResource:@"Drive%dRAM6000" withNumber:driveNum];
            [driveExpansion_6000 setState:hasRam];

            canRam = drive_check_expansion8000(driveTypeVal);
            [driveExpansion_8000 setEnabled:canRam];
            hasRam = [self getIntResource:@"Drive%dRAM8000" withNumber:driveNum];
            [driveExpansion_8000 setState:hasRam];

            canRam = drive_check_expansionA000(driveTypeVal);
            [driveExpansion_A000 setEnabled:canRam];
            hasRam = [self getIntResource:@"Drive%dRAMA000" withNumber:driveNum];
            [driveExpansion_A000 setState:hasRam];
        }

        // select parallel cable
        if(hasParallel) {
            int canParallel = drive_check_parallel_cable(driveTypeVal);
            [parallelCable setEnabled:canParallel];
            int parallelCableVal = [self getIntResource:@"Drive%dParallelCable" withNumber:driveNum];
            [parallelCable selectCellAtRow:parallelCableVal column:0];
        }

        int hasRPM = (driveTypeVal!=DRIVE_TYPE_NONE);

        if (hasRPM) {
            RPM.enabled = true;
            wobble.enabled = true;
            RPM.floatValue = [self getIntResource:@"Drive%dRPM" withNumber:driveNum]/100.0f;
            wobble.floatValue = [self getIntResource:@"Drive%dWobble" withNumber:driveNum]/100.0f;
        }
        else {
            RPM.enabled = false;
            wobble.enabled = false;
            RPM.stringValue = @"";
            wobble.stringValue = @"";
        }

    } else {
        // disable all controls
        [driveType setEnabled:false];
        [trackHandling setEnabled:false];
        if(hasIdle) {
            [idleMethod setEnabled:false];
        }
        if(hasExpansion) {
            [driveExpansion_2000 setEnabled:false];
            [driveExpansion_4000 setEnabled:false];
            [driveExpansion_6000 setEnabled:false];
            [driveExpansion_8000 setEnabled:false];
            [driveExpansion_A000 setEnabled:false];
        }
        if(hasParallel) {
            [parallelCable setEnabled:false];
        }
        RPM.selectable = RPM.editable = false;
        wobble.selectable = wobble.editable = false;
        RPM.stringValue = @"";
        wobble.stringValue = @"";
    }
}

-(int)mapToDriveType:(int)driveId
{
    if ((driveId >= 0) && (driveId < numDriveTypes)) {
        return driveTypeMap[driveId];
    }
    return DRIVE_TYPE_NONE;
}

-(int)mapFromDriveType:(int)driveTypeId
{
    int i;
    for (i = 0; i < numDriveTypes; i++) {
        if (driveTypeId == driveTypeMap[i]) {
            return i;
        }
    }
    return numDriveTypes;
}

// ----- Actions -----

-(void)toggleDrive:(id)sender
{
    [self updateResources:nil];
}

-(void)changedDriveType:(id)sender
{
    int driveNum = [driveChooser selectedSegment] + driveOffset;
    id cell = [sender selectedCell];
    int driveId = [cell tag];
    int driveTypeVar = [self mapToDriveType:driveId];
        
    [self setIntResource:@"Drive%dType" 
              withNumber:driveNum
                 toValue:driveTypeVar];
    [self updateResources:nil];
}

-(void)changedTrackHandling:(id)sender
{
    int driveNum = [driveChooser selectedSegment] + driveOffset;
    id cell = [sender selectedCell];
    int type = [cell tag];
    
    [self setIntResource:@"Drive%dExtendImagePolicy" 
              withNumber:driveNum 
                 toValue:type];
}

-(void)changedDriveExpansion2000:(id)sender
{
    int driveNum = [driveChooser selectedSegment] + driveOffset;
    id cell = [sender selectedCell];
    int on = [cell state];
    
    [self setIntResource:@"Drive%dRAM2000"
              withNumber:driveNum
                 toValue:on];
}

-(void)changedDriveExpansion4000:(id)sender
{
    int driveNum = [driveChooser selectedSegment] + driveOffset;
    id cell = [sender selectedCell];
    int on = [cell state];

    [self setIntResource:@"Drive%dRAM4000"
              withNumber:driveNum
                 toValue:on];
}

-(void)changedDriveExpansion6000:(id)sender
{
    int driveNum = [driveChooser selectedSegment] + driveOffset;
    id cell = [sender selectedCell];
    int on = [cell state];

    [self setIntResource:@"Drive%dRAM6000"
              withNumber:driveNum
                 toValue:on];
}

-(void)changedDriveExpansion8000:(id)sender
{
    int driveNum = [driveChooser selectedSegment] + driveOffset;
    id cell = [sender selectedCell];
    int on = [cell state];
    
    [self setIntResource:@"Drive%dRAM8000"
              withNumber:driveNum
                 toValue:on];
}

-(void)changedDriveExpansionA000:(id)sender
{
    int driveNum = [driveChooser selectedSegment] + driveOffset;
    id cell = [sender selectedCell];
    int on = [cell state];

    [self setIntResource:@"Drive%dRAMA000"
              withNumber:driveNum
                 toValue:on];
}

-(void)changedIdleMethod:(id)sender
{
    int driveNum = [driveChooser selectedSegment] + driveOffset;
    id cell = [sender selectedCell];
    int type = [cell tag];

    [self setIntResource:@"Drive%dIdleMethod"
              withNumber:driveNum
                 toValue:type];
}

-(void)toggledParallelCable:(id)sender
{
    int driveNum = [driveChooser selectedSegment] + driveOffset;
    id cell = [sender selectedCell];
    int type = [cell tag];

    [self setIntResource:@"Drive%dParallelCable"
              withNumber:driveNum
                 toValue:type];
}

-(void)changedRPM:(NSTextField*)sender
{
    int driveNum = [driveChooser selectedSegment] + driveOffset;
    float _RPM = [(NSTextField*)sender floatValue];
    _RPM = (_RPM<  30.0f)?  30.0f:
           (_RPM>3000.0f)?3000.0f:
            _RPM;

    [self setIntResource:@"Drive%dRPM"
              withNumber:driveNum
                 toValue: (int)((_RPM*100.0f)+0.5f)];
    RPM.floatValue = [self getIntResource:@"Drive%dRPM" withNumber:driveNum]/100.0f;
}

-(void)changedWobble:(NSTextField*)sender
{
    int driveNum = [driveChooser selectedSegment] + driveOffset;
    float _wobble = [(NSTextField*)sender floatValue];
    _wobble = (_wobble<   0.0f)?   0.0f:
              (_wobble>1000.0f)?1000.0f:
               _wobble;

    [self setIntResource:@"Drive%dWobble"
              withNumber:driveNum
                 toValue: (int)((_wobble*100.0f)+0.5f)];
    wobble.floatValue = [self getIntResource:@"Drive%dWobble" withNumber:driveNum]/100.0f;
}

@end
