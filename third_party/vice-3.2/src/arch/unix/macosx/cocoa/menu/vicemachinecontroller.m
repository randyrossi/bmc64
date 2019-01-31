/*
 * vicemachinecontroller.m - VICEMachineController - base machine controller
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

#include "resources.h"
#include "autostart.h"
#include "attach.h"
#include "charset.h"
#include "vsync.h"
#include "drive.h"
#include "monitor.h"
#include "monitor/mon_register.h"
#include "monitor/montypes.h"
#include "monitor/mon_ui.h"
#include "monitor/mon_breakpoint.h"
#include "machine.h"
#include "keyboard.h"
#include "kbdbuf.h"
#include "diskimage.h"
#include "mousedrv.h"
#include "lightpen.h"
#include "printer.h"
#include "archdep.h"
#include "log.h"
#include "screenshot.h"
#include "gfxoutput.h"
#include "videoarch.h"
#include "interrupt.h"
#include "tape.h"
#include "mouse.h"
#include "mousedrv.h"
#include "clipboard.h"
#include "datasette.h"
#include "vdrive-internal.h"
#include "fliplist.h"
#include "network.h"
#include "vice-event.h"
#include "imagecontents/diskcontents.h"
#include "imagecontents/tapecontents.h"
#include "imagecontents.h"

#import "vicemachinecontroller.h"
#import "vicemachine.h"

@implementation VICEMachineController

-(id)init
{
    return self;
}

-(NSString *)checkAndGetPrefDir
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *prefPath = [NSString stringWithCString:archdep_pref_path encoding:NSUTF8StringEncoding];
    if (![fileManager fileExistsAtPath:prefPath]) {
        if (![fileManager createDirectoryAtPath:prefPath attributes:nil]) {
            [[theVICEMachine app] runErrorMessage:@"Error creating Preferences Dir"];
            return nil;
        }
    }
    BOOL isDir;
    if ([fileManager fileExistsAtPath:prefPath isDirectory:&isDir] && isDir) {
        return prefPath;
    }
    [[theVICEMachine app] runErrorMessage:@"Invalid Preferences Dir"];
    return nil;
}

// ----- Resources -----

-(NSNumber *)getIntResource:(NSString *)name
{
    int value;
    if (resources_get_int([name cStringUsingEncoding:NSUTF8StringEncoding],&value)==0) {
        return [NSNumber numberWithInt:value];
    }
    return nil;
}

-(NSString *)getStringResource:(NSString *)name
{
    const char *value = NULL;
    if (resources_get_string([name cStringUsingEncoding:NSUTF8StringEncoding],&value)==0) {
        if(value != NULL) {
            return [NSString stringWithCString:value encoding:NSUTF8StringEncoding];
        } else {
            return nil;
        }
    }
    return nil;
}

-(BOOL)setIntResource:(NSString *)name value:(NSNumber *)value
{
    return resources_set_int([name cStringUsingEncoding:NSUTF8StringEncoding],
                             [value intValue]) == 0;
}

-(BOOL)setStringResource:(NSString *)name value:(NSString *)value
{
    return resources_set_string([name cStringUsingEncoding:NSUTF8StringEncoding],
                                [value cStringUsingEncoding:NSUTF8StringEncoding]) == 0;
}

-(BOOL)loadResources:(NSString *)path
{
    const char *cPath;
    if (path==nil)
        cPath = NULL;
    else
        cPath = [path fileSystemRepresentation];
    
    BOOL result = resources_load(cPath) == 0;
    
    // trigger resource upate in u
    [[theVICEMachine machineNotifier] postChangedResourcesNotification];
    
    return result;
}

-(BOOL)saveResources:(NSString *)path
{
    const char *cPath;
    if (path==nil)
        cPath = NULL;
    else
        cPath = [path fileSystemRepresentation];
    
    return resources_save(cPath) == 0;
}

-(void)resetResources
{
    resources_set_defaults();

    // trigger resource upate
    [[theVICEMachine machineNotifier] postChangedResourcesNotification];
}

// ----- Reset -----

-(void)resetMachine:(BOOL)hardReset
{
    if (hardReset)
        machine_trigger_reset(MACHINE_RESET_MODE_HARD);
    else
        machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
}

-(void)resetDrive:(int)unit
{
    drive_cpu_trigger_reset(unit);
}

// ----- Monitor -----

-(void)activateMonitor
{
    if ([theVICEMachine isPaused])
        // trigger monitor_startup() in pause loop of machine
        [theVICEMachine activateMonitorInPause];
    else
        // trigger monitor_startup() in next emu execution
        monitor_startup_trap();
}

-(NSArray *)getRegisters:(int)memSpace
{
    mon_reg_list_t *pMonRegs = mon_register_list_get(memSpace);
    mon_reg_list_t *p;

    // count registers
    unsigned int cnt;
    for (p = pMonRegs, cnt = 0; p->name != NULL; p++, cnt++ );

    NSMutableArray *regs = [NSMutableArray arrayWithCapacity:cnt];
    for (p = pMonRegs, cnt = 0; p->name != NULL; p++, cnt++ ) {
        NSDictionary *entry = [NSDictionary dictionaryWithObjectsAndKeys:
        [NSString stringWithCString:p->name encoding:NSUTF8StringEncoding], @"name",
        [NSNumber numberWithUnsignedInt:p->val], @"value",
        [NSNumber numberWithUnsignedInt:p->size], @"size",
        // FIXME: ->flags is a bitfield, see mon_register.h
        [NSNumber numberWithUnsignedInt:p->flags], @"flags",
        nil];
        [regs addObject:entry];
    }

    lib_free(pMonRegs);
    return regs;
}

-(NSData *)readMemory:(int)memSpace startAddress:(int)start endAddress:(int)end
{
    int len = end - start + 1;

    void *data = malloc(len);
    if(data == NULL)
        return nil;
    
    // TODO: end is wrong(!) in VICE call
    mon_get_mem_block(memSpace, start, len-1, data);
    return [NSData dataWithBytesNoCopy:data length:len freeWhenDone:YES];
}

-(NSData *)readMemory:(int)memSpace bank:(int)bank startAddress:(int)start endAddress:(int)end
{
    int len = end - start + 1;

    void *data = malloc(len);
    if(data == NULL)
        return nil;
    
    // TODO: end is wrong(!) in VICE call
    mon_get_mem_block_ex(memSpace, bank, start, len-1, data);
    return [NSData dataWithBytesNoCopy:data length:len freeWhenDone:YES];
}

-(NSString *)getCurrentMemoryBankName:(int)memSpace
{
    const char *name = mon_get_current_bank_name(memSpace);
    if(name == NULL)
        return nil;
    return [NSString stringWithCString:name encoding:NSUTF8StringEncoding];
}

-(NSDictionary *)disassembleMemory:(int)memSpace lines:(int)numLines start:(int)address
{
    mon_disassembly_private_t dp;
    
    mon_disassembly_init(&dp);
    mon_navigate_set_memspace(&dp.navigate,memSpace);
    if(address < 0) {
        // use current PC as start address
        mon_disassembly_update(&dp);
    } else {
        mon_navigate_set_startaddress(&dp.navigate, address);
    }
    
    mon_disassembly_t *data =  mon_disassembly_get_lines(&dp, numLines, numLines);
    NSMutableArray *lines = [[NSMutableArray alloc] init];
    while(data != NULL) {
        
        // entry dictionary with "content" and "flags"
        NSDictionary *entry = [NSDictionary dictionaryWithObjectsAndKeys:
            [NSString stringWithCString:data->content encoding:NSUTF8StringEncoding], @"contents",
            [NSNumber numberWithBool:data->flags.active_line], @"active_line",
            [NSNumber numberWithBool:data->flags.is_breakpoint], @"is_breakpoint",
            [NSNumber numberWithBool:data->flags.breakpoint_active], @"breakpoint_active",
            nil
        ];
        [lines addObject:entry];
        
        mon_disassembly_t *old = data;
        data = data->next;
        lib_free(old);
    }
    
    return [NSDictionary dictionaryWithObjectsAndKeys:
        lines, @"lines",
        [NSNumber numberWithInt:dp.navigate.StartAddress], @"start",
        [NSNumber numberWithInt:dp.navigate.EndAddress], @"end",
        nil
    ];
}

-(NSNumber *)disassembleSeek:(int)memSpace up:(bool)up lines:(int)numLines start:(int)address
{
    mon_disassembly_private_t dp;
    
    mon_disassembly_init(&dp);
    mon_navigate_set_memspace(&dp.navigate,memSpace);
    mon_navigate_set_startaddress(&dp.navigate, address);
    dp.navigate.Lines = numLines + 1;
    uint16_t pos = mon_navigate_scroll(&dp.navigate, up ? MON_SCROLL_PAGE_UP : MON_SCROLL_PAGE_DOWN);
    return [NSNumber numberWithInt:pos];
}

-(void)alterBreakpoint:(int)memSpace addr:(int)address set:(BOOL)set
{
    MON_ADDR addr = new_addr(memSpace, address);
    mon_breakpoint_type_t type = mon_breakpoint_is(addr);
    
    if(set) {
        if(type == BP_NONE) {
            //NSLog(@"BP set %x",addr);
            mon_breakpoint_set(addr);
        } else {
            //NSLog(@"BP unset %x",addr);
            mon_breakpoint_unset(addr);
        }
    } else {
        if(type == BP_INACTIVE) {
            //NSLog(@"BP enable %x",addr);
            mon_breakpoint_enable(addr);
        } else if(type == BP_ACTIVE) {
            //NSLog(@"BP disable %x",addr);
            mon_breakpoint_disable(addr);
        }
    }
}

-(int)getBreakpointState:(int)memSpace addr:(int)address
{
    MON_ADDR addr = new_addr(memSpace, address);
    mon_breakpoint_type_t type = mon_breakpoint_is(addr);
    return (int)type;
}

// ----- Snapshot -----

static char *snapshotName;
static int snapshotSaveRoms;
static int snapshotSaveDisks;

static void loadSnapshotTrap(uint16_t unusedWord, void *unusedData)
{
    if (machine_read_snapshot(snapshotName, 0) < 0) {
        [[theVICEMachine app] runErrorMessage:@"Error loading Snapshot!"];
    } else {
        log_message(LOG_DEFAULT,"loaded snapshot '%s'",snapshotName);
    }
    free(snapshotName);
}

static void saveSnapshotTrap(uint16_t unusedWord, void *unusedData)
{
    if (machine_write_snapshot(snapshotName, snapshotSaveRoms, snapshotSaveDisks, 0) < 0) {
        [[theVICEMachine app] runErrorMessage:@"Error saving Snapshot!"];        
    } else {
        log_message(LOG_DEFAULT,"saved snapshot '%s'",snapshotName);
    }
    free(snapshotName);
}

-(void)loadSnapshot:(NSString *)path
{
    snapshotName = strdup([path fileSystemRepresentation]);
    if ([theVICEMachine isPaused])
        loadSnapshotTrap(0,NULL);
    else
        interrupt_maincpu_trigger_trap(loadSnapshotTrap, NULL);
}

-(void)saveSnapshot:(NSString *)path withROMS:(BOOL)saveRoms andDisks:(BOOL)saveDisks
{
    snapshotName = strdup([path fileSystemRepresentation]);
    snapshotSaveRoms = saveRoms;
    snapshotSaveDisks = saveDisks;
    if ([theVICEMachine isPaused])
        saveSnapshotTrap(0,NULL);
    else
        interrupt_maincpu_trigger_trap(saveSnapshotTrap, NULL);
}

- (NSString *)getQuickSnapshotFileName:(BOOL)load
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *prefPath = [self checkAndGetPrefDir];
    if (prefPath==nil)
        return nil;

    // find file with highest index
    int i;
    int highestIndex = -1;
    NSString *path = nil;
    NSString *lastPath = nil;
    for (i=0;i<10;i++) {
        lastPath = path;
        path = [NSString stringWithFormat:@"%@/quicksnap%d.vsf",prefPath,i];
        if (![fileManager fileExistsAtPath:path]) {
            break;
        }
        highestIndex = i;
    }
    
    if (load) {
        if (lastPath==nil)
            [[theVICEMachine app] runErrorMessage:@"No Quick Snapshot File found!"];
        
        if (highestIndex==9)
            return path;
        else
            return lastPath;
    }
    else {
        // move all files one up to make space for new save
        if (highestIndex==9) {
            NSString *newPath = [NSString stringWithFormat:@"%@/quicksnap0.vsf",prefPath];
            [fileManager removeFileAtPath:newPath handler:nil];
            for (i=1;i<10;i++) {
                NSString *oldPath = [NSString stringWithFormat:@"%@/quicksnap%d.vsf",prefPath,i];
                [fileManager movePath:oldPath toPath:newPath handler:nil];
                newPath = oldPath;
            }
        }

        return path;
    }
}

-(void)saveQuickSnapshot
{
    NSString *path = [self getQuickSnapshotFileName:FALSE];
    if (path!=nil) {
        [self saveSnapshot:path withROMS:FALSE andDisks:FALSE];
    }
}

-(void)loadQuickSnapshot
{
    NSString *path = [self getQuickSnapshotFileName:TRUE];
    if (path!=nil) {
        [self loadSnapshot:path];
    }
}

// ----- History -----

-(BOOL)startRecordHistory
{
    return event_record_start() == 0;
}

-(BOOL)stopRecordHistory
{
    return event_record_stop() == 0;
}

-(BOOL)startPlaybackHistory
{
    return event_playback_start() == 0;
}

-(BOOL)stopPlaybackHistory
{
    return event_playback_stop() == 0;
}

-(BOOL)isRecordingHistory
{
    return event_record_active();
}

-(BOOL)isPlayingBackHistory
{
    return event_playback_active();
}

-(BOOL)setRecordMilestone
{
    return event_record_set_milestone() == 0;
}

-(BOOL)resetRecordMilestone
{
    return event_record_reset_milestone() == 0;
}

// ----- Media -----

-(BOOL)startRecordingMedia:(NSString *)driver fromCanvas:(int)canvasId
                                                  toFile:(NSString *)path
{
    // fetch real canvas for id
    video_canvas_t *canvas = [theVICEMachine getCanvasForId:canvasId];
    if (canvas==NULL)
        return false;
    
    int result = screenshot_save([driver cStringUsingEncoding:NSUTF8StringEncoding],
                                 [path cStringUsingEncoding:NSUTF8StringEncoding],
                                 canvas);
    return result == 0;
}

-(BOOL)isRecordingMedia
{
    return screenshot_is_recording();
}

-(void)stopRecordingMedia
{
    screenshot_stop_recording();
}

-(NSArray *)enumMediaDrivers
{
    int i;
    gfxoutputdrv_t *driver;
    
    NSMutableArray *a = [[NSMutableArray alloc] init];
    driver = gfxoutput_drivers_iter_init();
    for (i = 0; i < gfxoutput_num_drivers(); i++) {
        NSString *name = [NSString stringWithCString:driver->name encoding:NSUTF8StringEncoding];
        [a addObject:name];
        driver = gfxoutput_drivers_iter_next();
    }
    return [a autorelease];
}

-(gfxoutputdrv_t *)findDriverByName:(NSString *)driverName
{
    int i;
    gfxoutputdrv_t *driver = gfxoutput_drivers_iter_init();
    for (i = 0; i < gfxoutput_num_drivers(); i++) {
        NSString *name = [NSString stringWithCString:driver->name encoding:NSUTF8StringEncoding];
        if ([name compare:driverName]==NSOrderedSame) {
            return driver;
        }
        driver = gfxoutput_drivers_iter_next();        
    }
    return NULL;
}

-(BOOL)mediaDriverHasFormats:(NSString *)driverName
{
    gfxoutputdrv_t *driver = [self findDriverByName:driverName];
    if (driver == NULL)
        return FALSE;
    
    return (driver->formatlist != NULL);
}

-(NSArray *)enumMediaFormats:(NSString *)driverName
{
    // find driver
    gfxoutputdrv_t *driver = [self findDriverByName:driverName];
    if (driver == NULL)
        return nil;
        
    // get formatlist
    gfxoutputdrv_format_t *formatlist = driver->formatlist;
    if (formatlist == NULL)
        return nil;

    // enumerate all formats and codecs
    int i;
    NSMutableArray *a = [[NSMutableArray alloc] init];
    for (i=0;formatlist[i].name!=NULL;i++) {
        NSString *fname = [NSString stringWithCString:formatlist[i].name encoding:NSUTF8StringEncoding];
        
        // fetch video codecs
        int j;
        gfxoutputdrv_codec_t *video_codecs = formatlist[i].video_codecs;
        NSMutableDictionary *video = [[NSMutableDictionary alloc] init];
        if (video_codecs!=NULL) {
            for (j=0;video_codecs->name!=NULL;j++) {
                NSString *vname = [NSString stringWithCString:video_codecs->name encoding:NSUTF8StringEncoding];
                [video setObject:vname forKey:[NSNumber numberWithInt:video_codecs->id]];
                video_codecs++;
            }
        }
        
        // fetch audio codecs
        gfxoutputdrv_codec_t *audio_codecs = formatlist[i].audio_codecs;
        NSMutableDictionary *audio = [[NSMutableDictionary alloc] init];
        if (audio_codecs!=NULL) {
            for (j=0;audio_codecs->name!=NULL;j++) {
                NSString *aname = [NSString stringWithCString:audio_codecs->name encoding:NSUTF8StringEncoding];
                [audio setObject:aname forKey:[NSNumber numberWithInt:audio_codecs->id]];
                audio_codecs++;                
            }
        }

        [a addObject:[NSArray arrayWithObjects:fname,video,audio,nil]];
    }

    return [a autorelease];
}

-(NSString *)defaultExtensionForMediaDriver:(NSString *)driverName andFormat:(NSString *)formatName
{
    // find driver
    gfxoutputdrv_t *driver = [self findDriverByName:driverName];
    if (driver == NULL)
        return nil;

    // use global extension if available
    if (driver->default_extension != NULL)
        return [NSString stringWithCString:driver->default_extension encoding:NSUTF8StringEncoding];
    
    // search format
    gfxoutputdrv_format_t *formatlist = driver->formatlist;
    if (formatlist == NULL)
        return nil;

    int i;
    for (i=0;formatlist[i].name!=NULL;i++) {
        NSString *fname = [NSString stringWithCString:formatlist[i].name encoding:NSUTF8StringEncoding];
        if ([fname compare:formatName]==NSOrderedSame) {
            return fname;
        }
    }
    return nil;
}

// ----- Keyboard -----

-(void)keyPressed:(unsigned int)code
{
    if([theVICEMachine isPaused]) {
        // in pause mode route key to machine
        [theVICEMachine keyPressedInPause:code];
    } else {
        // tell VICE a key was pressed
        keyboard_key_pressed(code);
    }
}

-(void)keyReleased:(unsigned int)code
{
    if([theVICEMachine isPaused]) {
        // in pause mode route key to machine
        [theVICEMachine keyReleasedInPause:code];
    } else {
        // tell VICE a key was released
        keyboard_key_released(code);
    }
}

-(BOOL)dumpKeymap:(NSString *)path
{
    return (keyboard_keymap_dump([path fileSystemRepresentation]) == 0);
}

// ----- Mouse & Lightpen -----

extern void mouse_move_f(float x, float y);

-(void)mouseMoveToX:(float)x andY:(float)y
{
    mouse_move_f(x,y);
}

-(void)mouseButton:(BOOL)left withState:(BOOL)pressed
{
    if(left)
        mousedrv_button_left(pressed);
    else
        mousedrv_button_right(pressed);
}

-(void)lightpenUpdateOnScreen:(int)screen toX:(int)x andY:(int)y 
                  withButton1:(BOOL)b1 andButton2:(BOOL)b2
{
    int buttons = 0;
    if(b1)
        buttons |= LP_HOST_BUTTON_1;
    if(b2)
        buttons |= LP_HOST_BUTTON_2;
    
    lightpen_update(screen, x, y, buttons);
}

// ----- Drive -----

-(BOOL)smartAttachImage:(NSString *)path withProgNum:(int)num andRun:(BOOL)run
{
    return autostart_autodetect([path fileSystemRepresentation],
        NULL, num, run ? AUTOSTART_MODE_RUN : AUTOSTART_MODE_LOAD) == 0;
}

-(BOOL)attachDiskImage:(int)unit path:(NSString *)path
{
    return file_system_attach_disk(unit,[path fileSystemRepresentation]) == 0;
}

-(void)detachDiskImage:(int)unit
{
    file_system_detach_disk(unit);
}

-(BOOL)createDiskImage:(int)type path:(NSString *)path name:(NSString *)name
{
    const int ids[] = {
        DISK_IMAGE_TYPE_D64, DISK_IMAGE_TYPE_D67, DISK_IMAGE_TYPE_D71,
        DISK_IMAGE_TYPE_D80, DISK_IMAGE_TYPE_D81, DISK_IMAGE_TYPE_D82,
        DISK_IMAGE_TYPE_G64, DISK_IMAGE_TYPE_P64, DISK_IMAGE_TYPE_X64,
        DISK_IMAGE_TYPE_D1M, DISK_IMAGE_TYPE_D2M, DISK_IMAGE_TYPE_D4M
    };
    return vdrive_internal_create_format_disk_image([path fileSystemRepresentation],
                                                    [name cStringUsingEncoding:NSUTF8StringEncoding],
                                                    ids[type]) == 0;
}

-(NSString *)getDiskName:(int)unit
{
    const char *diskName = file_system_get_disk_name(unit);
    if (diskName==NULL)
        return nil;
    else
        return [NSString stringWithCString:diskName encoding:NSUTF8StringEncoding];
}

// ----- Tape -----

-(BOOL)attachTapeImage:(NSString *)path
{
    return tape_image_attach(1,[path fileSystemRepresentation]) == 0;
}

-(void)detachTapeImage
{
    tape_image_detach(1);
}

-(void)controlDatasette:(int)command
{
    datasette_control(command);
}

// ----- Printer -----

-(void)printerFormFeed:(int)unit
{
    printer_formfeed(unit);
}

// ----- Cut & Paste -----

-(NSString *)readScreenOutput
{
    char * text = clipboard_read_screen_output("\n");
    if (text == NULL) {
        return @"";
    }
    NSString * t = [NSString stringWithCString:text encoding:NSUTF8StringEncoding];
    lib_free(text);
    return t;
}

-(void)typeStringOnKeyboard:(NSString *)string toPetscii:(BOOL)convert
{
    const char *cstr = [string cStringUsingEncoding:NSUTF8StringEncoding];
    if (convert) {
        int len = [string length];
        char *pstr = (char *)malloc(len + 1);
        memcpy(pstr,cstr,len+1);
        charset_petconvstring((uint8_t*)pstr,0);
        
        kbdbuf_feed(pstr);
        
        free(pstr);        
    } else {
        kbdbuf_feed(cstr);
    }
}        

// ----- Fliplist -----

-(BOOL)loadFliplist:(int)unit path:(NSString *)path autoAttach:(BOOL)autoAttach
{
    const char *cstr = [path cStringUsingEncoding:NSUTF8StringEncoding];
    return fliplist_load_list(unit,cstr,autoAttach) == 0;
}

-(BOOL)saveFliplist:(int)unit path:(NSString *)path
{
    const char *cstr = [path cStringUsingEncoding:NSUTF8StringEncoding];
    return fliplist_save_list(unit,cstr) == 0;
}

-(void)addCurrentToFliplist:(int)unit
{
    fliplist_add_image(unit);
}

-(void)removeFromFliplist:(int)unit path:(NSString *)path
{
    const char *cstr = NULL;
    if (path!=nil) {
        cstr = [path cStringUsingEncoding:NSUTF8StringEncoding];
    }
    fliplist_remove(unit,cstr);
}

-(void)attachNextInFliplist:(int)unit direction:(BOOL)next
{
    fliplist_attach_head(unit,next);
}

// ----- Netplay -----

-(BOOL)startNetplayServer
{
    return network_start_server() == 0;
}

-(BOOL)connectNetplayClient
{
    return network_connect_client() == 0;
}

-(void)disconnectNetplay
{
    network_disconnect();
}

-(int)getNetplayMode
{
    return network_get_mode();
}

// ----- Image Contents -----

// convert latin1 chars with special meaning to similar chars in CBM font
-(void)fixChars:(unsigned char *)str
{
    while(*str != 0) {
        unsigned char code = *str;
        // non breaking space
        if(code==0xa0)
          *str=0x20;
        // soft hyphen
        if(code==0xad)
          *str=0xed;
        str++;
    }
}

-(NSDictionary *)convertImageContentsToDictionary:(image_contents_t *)ic
{
    image_contents_file_list_t *list = ic->file_list;
    int num_files = 0;
    while(list!=NULL) {
        num_files++;
        list = list->next;
    }
    
    NSMutableArray *files = [NSMutableArray arrayWithCapacity:num_files];
    list = ic->file_list;
    while(list!=NULL) {
        char *file = image_contents_file_to_string(list, 0);
        [self fixChars:(unsigned char *)file];
        NSString *sfile = [NSString stringWithCString:file encoding:NSISOLatin1StringEncoding];
        lib_free(file);
        
        NSDictionary *entry = [NSDictionary dictionaryWithObjectsAndKeys:
            sfile, @"fileName",
            nil, nil];
        
        [files addObject:entry];
        list = list->next;
    }
    
    [self fixChars:(unsigned char *)ic->name];
    [self fixChars:(unsigned char *)ic->id];
    NSDictionary *result = [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithInt:ic->blocks_free], @"blocks_free",
            [NSString stringWithCString:(char *)ic->name encoding:NSISOLatin1StringEncoding], @"name",
            [NSString stringWithCString:(char *)ic->id encoding:NSISOLatin1StringEncoding], @"id",
            files, @"files",
            nil, nil];
    
    image_contents_destroy(ic);        
    return result;
}

-(NSDictionary *)diskimageContents:(NSString *)path
{
    const char *cstr = [path cStringUsingEncoding:NSUTF8StringEncoding];

    // HACK: temporarly disable logging. otherwise log window outputs block UI!
    log_enable(0);
    image_contents_t *ic = diskcontents_filesystem_read(cstr);
    log_enable(1);

    if(ic == NULL) {
        return nil;
    }
    return [self convertImageContentsToDictionary:ic];
}

-(NSDictionary *)tapeimageContents:(NSString *)path
{
    const char *cstr = [path cStringUsingEncoding:NSUTF8StringEncoding];

    // HACK: temporarly disable logging. otherwise log window outputs block UI!
    log_enable(0);
    image_contents_t *ic = tapecontents_read(cstr);
    log_enable(1);
    
    if(ic == NULL) {
        return nil;
    }
    return [self convertImageContentsToDictionary:ic];
}

@end
