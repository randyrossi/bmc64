#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>
#include "plus4lib/plus4emu.h"
#include "../common/circle.h"
#include "../common/emux_api.h"
#include "../common/keycodes.h"
#include "../common/overlay.h"
#include "../common/demo.h"
#include "../common/menu.h"

static Plus4VM            *vm = NULL;
static Plus4VideoDecoder  *videoDecoder = NULL;

static uint8_t          *fb_buf;
static int              fb_pitch;
static int              ui_trap;
static int              ui_warp;
static int              joy_latch_value[2];

static struct menu_item *sid_model_item;
static struct menu_item *sid_write_access_item;
static struct menu_item *sid_digiblaster_item;

#define COLOR16(r,g,b) (((r)>>3)<<11 | ((g)>>2)<<5 | (b)>>3)

static void set_video_font(void) {
  int i;

  // Temporary for now. Need to figure out how to get this from the emulator's
  // ROM. Just read the file ourselves.
  uint8_t* chargen = malloc(4096); // never freed
  FILE* fp = fopen("p4kernal.rom", "r");
  fseek(fp, 0x1000, SEEK_SET);
  fread(chargen,1,4096,fp);
  fclose(fp);

  video_font = &chargen[0x400];
  raw_video_font = &chargen[0x000];
  for (i = 0; i < 256; ++i) {
    video_font_translate[i] = 8 * ascii_to_petscii[i];
  }
}

static void apply_sid_config() {
  int sid_flags = sid_model_item->choice_ints[sid_model_item->value];
  if (sid_write_access_item->value) sid_flags |= 0x2;
  Plus4VM_SetSIDConfiguration(vm, sid_flags, sid_digiblaster_item->value, 0);
}

static void errorMessage(const char *fmt, ...)
{
  va_list args;
  fprintf(stderr, " *** Plus/4 error: ");
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
  Plus4VM_Destroy(vm);
  exit(-1);
}

static void vmError(void)
{
  fprintf(stderr, " *** Plus/4 error: %s\n", Plus4VM_GetLastErrorMessage(vm));
  Plus4VM_Destroy(vm);
  exit(-1);
}

static void audioOutputCallback(void *userData,
                                const int16_t *buf, size_t nFrames)
{
  if (!ui_warp)
     circle_sound_write((int16_t*)buf, nFrames);
}

static void videoLineCallback(void *userData,
                              int lineNum, const Plus4VideoLineData *lineData)
{
   lineNum = lineNum / 2;
   if (lineNum >= 0 && lineNum < 288) {
     Plus4VideoDecoder_DecodeLine(videoDecoder, fb_buf + lineNum * fb_pitch, 384, lineData);
   }
}

static void videoFrameCallback(void *userData)
{
#ifndef HOST_BUILD
  circle_frames_ready_fbl(FB_LAYER_VIC,
                          -1 /* no 2nd layer */,
                          !ui_warp /* sync */);
#endif
}

static void resetMemoryConfiguration(void)
{
  if (Plus4VM_SetRAMConfiguration(vm, 64, 0x99999999UL) != PLUS4EMU_SUCCESS)
    vmError();
  /* load ROM images */
  if (Plus4VM_LoadROM(vm, 0x00, "p4_basic.rom", 0) != PLUS4EMU_SUCCESS)
    errorMessage("cannot load p4_basic.rom");
  if (Plus4VM_LoadROM(vm, 0x01, "p4kernal.rom", 0) != PLUS4EMU_SUCCESS)
    errorMessage("cannot load p4kernal.rom");
  if (Plus4VM_LoadROM(vm, 0x10, "dos1541.rom", 0) != PLUS4EMU_SUCCESS)
    errorMessage("cannot load dos1541.rom");
}

//     0: Del          1: Return       2: £            3: Help
//     4: F1           5: F2           6: F3           7: @
//     8: 3            9: W           10: A           11: 4
//    12: Z           13: S           14: E           15: Shift
//    16: 5           17: R           18: D           19: 6
//    20: C           21: F           22: T           23: X
//    24: 7           25: Y           26: G           27: 8
//    28: B           29: H           30: U           31: V
//    32: 9           33: I           34: J           35: 0
//    36: M           37: K           38: O           39: N
//    40: Down        41: P           42: L           43: Up
//    44: .           45: :           46: -           47: ,
//    48: Left        49: *           50: ;           51: Right
//    52: Esc         53: =           54: +           55: /
//    56: 1           57: Home        58: Ctrl        59: 2
//    60: Space       61: C=          62: Q           63: Stop
static int bmc64_keycode_to_plus4emu(long keycode) {
   switch (keycode) {
      case KEYCODE_Backspace:
         return 0;
      case KEYCODE_Return:
         return 1;
      case KEYCODE_BackSlash:
         return 2;
      case KEYCODE_F7:
         return 3;
      case KEYCODE_F1:
         return 4;
      case KEYCODE_F2:
         return 5;
      case KEYCODE_F3:
         return 6;
      case KEYCODE_Insert:
         return 7;
      case KEYCODE_3:
         return 8;
      case KEYCODE_w:
         return 9;
      case KEYCODE_a:
         return 10;
      case KEYCODE_4:
         return 11;
      case KEYCODE_z:
         return 12;
      case KEYCODE_s:
         return 13;
      case KEYCODE_e:
         return 14;
      case KEYCODE_LeftShift:
      case KEYCODE_RightShift:
         return 15;
      case KEYCODE_5:
         return 16;
      case KEYCODE_r:
         return 17;
      case KEYCODE_d:
         return 18;
      case KEYCODE_6:
         return 19;
      case KEYCODE_c:
         return 20;
      case KEYCODE_f:
         return 21;
      case KEYCODE_t:
         return 22;
      case KEYCODE_x:
         return 23;
      case KEYCODE_7:
         return 24;
      case KEYCODE_y:
         return 25;
      case KEYCODE_g:
         return 26;
      case KEYCODE_8:
         return 27;
      case KEYCODE_b:
         return 28;
      case KEYCODE_h:
         return 29;
      case KEYCODE_u:
         return 30;
      case KEYCODE_v:
         return 31;
      case KEYCODE_9:
         return 32;
      case KEYCODE_i:
         return 33;
      case KEYCODE_j:
         return 34;
      case KEYCODE_0:
         return 35;
      case KEYCODE_m:
         return 36;
      case KEYCODE_k:
         return 37;
      case KEYCODE_o:
         return 38;
      case KEYCODE_n:
         return 39;
      case KEYCODE_Down:
         return 40;
      case KEYCODE_p:
         return 41;
      case KEYCODE_l:
         return 42;
      case KEYCODE_Up:
         return 43;
      case KEYCODE_Period:
         return 44;
      case KEYCODE_SemiColon:
         return 45;
      case KEYCODE_LeftBracket:
         return 46;
      case KEYCODE_Comma:
         return 47;
      case KEYCODE_Left:
         return 48;
      case KEYCODE_Dash:
         return 49;
      case KEYCODE_SingleQuote:
         return 50;
      case KEYCODE_Right:
         return 51;
      case KEYCODE_BackQuote:
         return 52;
      case KEYCODE_RightBracket:
         return 53;
      case KEYCODE_Equals:
         return 54;
      case KEYCODE_Slash:
         return 55;
      case KEYCODE_1:
         return 56;
      case KEYCODE_Home:
         return 57;
      case KEYCODE_Tab:
         return 58;
      case KEYCODE_2:
         return 59;
      case KEYCODE_Space:
         return 60;
      case KEYCODE_LeftControl:
         return 61;
      case KEYCODE_q:
         return 62;
      case KEYCODE_Escape:
         return 63;
      default:
         return -1;
   }
}

// This is made to look like VICE's main entry point so our
// Plus4Emu version of EmulatorCore can look more or less the same
// as the Vice version.
int main_program(int argc, char **argv)
{
  int     quitFlag = 0;

  (void) argc;
  (void) argv;

  printf ("Init\n");

#ifndef HOST_BUILD
  // BMC64 Video Init
  if (circle_alloc_fbl(FB_LAYER_VIC, 1 /* RGB565 */, &fb_buf,
                              384, 288, &fb_pitch)) {
    printf ("Failed to create video buf.\n");
    assert(0);
  }
  circle_clear_fbl(FB_LAYER_VIC);
  circle_show_fbl(FB_LAYER_VIC);
#else
  fb_buf = (uint8_t*) malloc(384*288*2);
  fb_pitch = 384;
#endif

  vm = Plus4VM_Create();
  if (!vm)
    errorMessage("could not create Plus/4 emulator object");

  Plus4VM_SetAudioOutputCallback(vm, &audioOutputCallback, NULL);
  if (Plus4VM_SetAudioOutputQuality(vm, 1) != PLUS4EMU_SUCCESS)
    vmError();

  int audioSampleRate;
  int fragsize;
  int fragnr;
  int channels;

  circle_sound_init(NULL, &audioSampleRate, &fragsize, &fragnr, &channels);
  if (Plus4VM_SetAudioSampleRate(vm, audioSampleRate) != PLUS4EMU_SUCCESS)
    vmError();
  resetMemoryConfiguration();
  if (Plus4VM_SetWorkingDirectory(vm, ".") != PLUS4EMU_SUCCESS)
    vmError();
  /* enable read-write IEC level drive emulation for unit 8 */
  Plus4VM_SetIECDriveReadOnlyMode(vm, 0);
  emux_detach_disk(0);

  Plus4VM_Reset(vm, 1);

  videoDecoder =
      Plus4VideoDecoder_Create(&videoLineCallback, &videoFrameCallback, NULL);
  if (!videoDecoder)
    errorMessage("could not create video decoder object");
  //Plus4VideoDecoder_UpdatePalette(videoDecoder, 0, 16, 8, 0); // not using rgb
  Plus4VM_SetVideoOutputCallback(vm, &Plus4VideoDecoder_VideoCallback,
                                 (void *) videoDecoder);

  vic_enabled = 1; // really TED
  ui_init_menu();

  // Here, we should make whatever calls are necessary to configure the VM
  // according to any settings that were loaded.
  apply_sid_config();

  canvas_state[0].gfx_w = 40*8;
  canvas_state[0].gfx_h = 25*8;

  int timing = circle_get_machine_timing();
  if (timing == MACHINE_TIMING_NTSC_HDMI ||
      timing == MACHINE_TIMING_NTSC_COMPOSITE ||
      timing == MACHINE_TIMING_NTSC_CUSTOM) {
    canvas_state[vic_canvas_index].max_border_w = 32;
    canvas_state[vic_canvas_index].max_border_h = 16;
  } else {
    canvas_state[vic_canvas_index].max_border_w = 32;
    canvas_state[vic_canvas_index].max_border_h = 40;
  }

  set_video_font();

  /* run Plus/4 emulation until the F12 key is pressed */
  printf ("Enter emulation loop\n");

  do {
    if (Plus4VM_Run(vm, 2000) != PLUS4EMU_SUCCESS)
      vmError();

    emux_ensure_video();

    // This render will handle any OSDs we have. ODSs don't pause emulation.
    if (ui_enabled) {
      // The only way we can be here and have ui_enabled=1
      // is for an osd to be enabled.
      ui_render_now(-1); // only render top most menu
      circle_frames_ready_fbl(FB_LAYER_UI, -1 /* no 2nd layer */,
         0 /* no sync */);
      ui_check_key();
    }

    if (statusbar_showing || vkbd_showing) {
      overlay_check();
      if (overlay_dirty) {
         circle_frames_ready_fbl(FB_LAYER_STATUS,                                                                -1 /* no 2nd layer */,
                                 0 /* no sync */);
         overlay_dirty = 0;
      }
    }

    circle_yield();
    circle_check_gpio();

    int reset_demo = 0;

    circle_lock_acquire();
    while (pending_emu_key.head != pending_emu_key.tail) {
      int i = pending_emu_key.head & 0xf;
      reset_demo = 1;
      if (vkbd_enabled) {
        // Kind of nice to have virtual keyboard's state
        // stay in sync with changes happening from USB
        // key events.
        vkbd_sync_event(pending_emu_key.key[i], pending_emu_key.pressed[i]);
      }
      int p4code = bmc64_keycode_to_plus4emu(
         pending_emu_key.key[i]);
      if (p4code >= 0) {
         Plus4VM_KeyboardEvent(vm, p4code, pending_emu_key.pressed[i]);
      }
      pending_emu_key.head++;
    }

    // Joystick event dequeue
    while (pending_emu_joy.head != pending_emu_joy.tail) {
      int i = pending_emu_joy.head & 0x7f;
      reset_demo = 1;
      if (vkbd_enabled) {
        int value = pending_emu_joy.value[i];
        int devd = pending_emu_joy.device[i];
        switch (pending_emu_joy.type[i]) {
        case PENDING_EMU_JOY_TYPE_ABSOLUTE:
          if (!vkbd_press[devd]) {
             if (value & 0x1 && !vkbd_up[devd]) {
               vkbd_up[devd] = 1;
               vkbd_nav_up();
             } else if (!(value & 0x1) && vkbd_up[devd]) {
               vkbd_up[devd] = 0;
             }
             if (value & 0x2 && !vkbd_down[devd]) {
               vkbd_down[devd] = 1;
               vkbd_nav_down();
             } else if (!(value & 0x2) && vkbd_down[devd]) {
               vkbd_down[devd] = 0;
             }
             if (value & 0x4 && !vkbd_left[devd]) {
               vkbd_left[devd] = 1;
               vkbd_nav_left();
             } else if (!(value & 0x4) && vkbd_left[devd]) {
               vkbd_left[devd] = 0;
             }
             if (value & 0x8 && !vkbd_right[devd]) {
               vkbd_right[devd] = 1;
               vkbd_nav_right();
             } else if (!(value & 0x8) && vkbd_right[devd]) {
               vkbd_right[devd] = 0;
             }
          }
          if (value & 0x10 && !vkbd_press[devd]) vkbd_nav_press(1, devd);
          else if (!(value & 0x10) && vkbd_press[devd]) vkbd_nav_press(0, devd);
          break;
        }
      } else {
        int port = pending_emu_joy.port[i]-1;
        int oldv = joy_latch_value[port];
        switch (pending_emu_joy.type[i]) {
        case PENDING_EMU_JOY_TYPE_ABSOLUTE:
          // If new bit is 0 and old bit is 1, it is an up event
          // If new bit is 1 and old bit is 0, it is a down event
          joy_latch_value[port] = pending_emu_joy.value[i];
          break;
        case PENDING_EMU_JOY_TYPE_AND:
          // If new bit is 0 and old bit is 1, it is an up event
          joy_latch_value[port] &= pending_emu_joy.value[i];
          break;
        case PENDING_EMU_JOY_TYPE_OR:
          // If new bit is 1 and old bit is 0, it is a down event
          joy_latch_value[port] |= pending_emu_joy.value[i];
          break;
        default:
          break;
        }

        //    72: Joy2 Up     73: Joy2 Down   74: Joy2 Left   75: Joy2 Right
        //    79: Joy2 Fire
        //    80: Joy1 Up     81: Joy1 Down   82: Joy1 Left   83: Joy1 Right
        //    86: Joy1 Fire

        int newv = joy_latch_value[port];
        if (!(newv & 0x01) && (oldv & 0x01)) {
          Plus4VM_KeyboardEvent(vm, 72 + 8*port, 0);
        } else if ((newv & 0x01) && !(oldv & 0x01)) {
          Plus4VM_KeyboardEvent(vm, 72 + 8*port, 1);
        }
        if (!(newv & 0x02) && (oldv & 0x02)) {
          Plus4VM_KeyboardEvent(vm, 73 + 8*port, 0);
        } else if ((newv & 0x02) && !(oldv & 0x02)) {
          Plus4VM_KeyboardEvent(vm, 73 + 8*port, 1);
        }
        if (!(newv & 0x04) && (oldv & 0x04)) {
          Plus4VM_KeyboardEvent(vm, 74 + 8*port, 0);
        } else if ((newv & 0x04) && !(oldv & 0x04)) {
          Plus4VM_KeyboardEvent(vm, 74 + 8*port, 1);
        }
        if (!(newv & 0x08) && (oldv & 0x08)) {
          Plus4VM_KeyboardEvent(vm, 75 + 8*port, 0);
        } else if ((newv & 0x08) && !(oldv & 0x08)) {
          Plus4VM_KeyboardEvent(vm, 75 + 8*port, 1);
        }
        if (!(newv & 0x10) && (oldv & 0x10)) {
          Plus4VM_KeyboardEvent(vm, 79 + 7*port, 0);
        } else if ((newv & 0x10) && !(oldv & 0x10)) {
          Plus4VM_KeyboardEvent(vm, 79 + 7*port, 1);
        }
      }
      pending_emu_joy.head++;
    }

    if (ui_trap) {
        circle_lock_release();
        emu_pause_trap(0, NULL);
        circle_lock_acquire();
        ui_trap = 0;
    }

    circle_lock_release();

    ui_handle_toggle_or_quick_func();

    if (reset_demo) {
      demo_reset_timeout();
    }

    if (raspi_demo_mode) {
      demo_check();
    }

  } while (!quitFlag);

  Plus4VM_Destroy(vm);
  Plus4VideoDecoder_Destroy(videoDecoder);
  return 0;
}

// Begin emu_api impl.

void emu_machine_init() {
  emux_machine_class = BMC64_MACHINE_CLASS_PLUS4EMU;
}


#ifdef HOST_BUILD

int circle_sound_init(const char *param, int *speed, int *fragsize,
                        int *fragnr, int *channels) {
   *speed = 48000;
   *fragsize = 2048;
   *fragnr = 16;
   *channels = 1;
}

int circle_sound_write(int16_t *pbuf, size_t nr) {
}

int main(int argc, char *argv[]) {
  main_program(argc, argv);
}

#endif


// STUBS FOR NOW
void emux_trap_main_loop_ui(void) {
  circle_lock_acquire();
  ui_trap = 1;
  circle_lock_release();
}

void emux_trap_main_loop(void (*trap_func)(uint16_t, void *data), void* data) {
}

void emux_kbd_set_latch_keyarr(int row, int col, int value) {
}

int emux_attach_disk_image(int unit, char *filename) {
  if (Plus4VM_SetDiskImageFile(vm, unit-8, filename, 0) != PLUS4EMU_SUCCESS) {
    return 1;
  }
  return 0;
}

void emux_detach_disk(int unit) {
  Plus4VM_SetDiskImageFile(vm, unit-8, "", 1);
}

int emux_attach_tape_image(char *filename) {
  return 0;
}

void emux_detach_tape(void) {
}

int emux_attach_cart(int bank, char *filename) {
  return 0;
}

void emux_set_cart_default(void) {
}

void emux_detach_cart(int bank) {
}

void emux_reset(int isSoft) {
  Plus4VM_Reset(vm, 1);
}

int emux_save_state(char *filename) {
  if (Plus4VM_SaveState(vm, filename) != PLUS4EMU_SUCCESS) {
    return 1;
  }
  return 0;
}

int emux_load_state(char *filename) {
  if (Plus4VM_LoadState(vm, filename) != PLUS4EMU_SUCCESS) {
    return 1;
  }
  return 0;
}

int emux_tape_control(int cmd) {
  return 0;
}

void emux_show_cart_osd_menu(void) {
}

unsigned long emux_calculate_timing(double fps) {
}

int emux_autostart_file(char* filename) {
  return 0;
}

void emux_drive_change_model(int unit) {
}

void emux_add_parallel_cable_option(struct menu_item* parent,
                                    int id, int drive) {
}

void emux_create_disk(struct menu_item* item, fullpath_func fullpath) {
}

void emux_set_joy_port_device(int port_num, int dev_id) {
}

void emux_set_joy_pot_x(int value) {
  // Not supported on plus4emu
}

void emux_set_joy_pot_y(int value) {
  // Not supported on plus4emu
}

void emux_add_sound_options(struct menu_item* parent) {
  // TODO: Why is 6581 so slow?
  struct menu_item* child = sid_model_item =
      ui_menu_add_multiple_choice(MENU_SID_MODEL, parent, "Sid Model");
  child->num_choices = 1;
  child->value = 0;
  strcpy(child->choices[0], "8580");
  child->choice_ints[0] = 0; // 8580 for sidflags

  // Write access at $d400-d41f
  child = sid_write_access_item =
      ui_menu_add_toggle(MENU_SID_WRITE_D400, parent,
          "Write Access $D400-D41F", 0);

  // Digiblaster
  child = sid_digiblaster_item =
      ui_menu_add_toggle(MENU_SID_DIGIBLASTER, parent,
          "Enable Digiblaster", 0);
}

void emux_video_color_setting_changed(int display_num) {
  // TBD
}

void emux_set_color_brightness(int display_num, int value) {
}

void emux_set_color_contrast(int display_num, int value) {
}

void emux_set_color_gamma(int display_num, int value) {
}

void emux_set_color_tint(int display_num, int value) {
}

int emux_get_color_brightness(int display_num) {
  return 1000;
}

int emux_get_color_contrast(int display_num) {
  return 1000;
}

int emux_get_color_gamma(int display_num) {
  return 1000;
}

int emux_get_color_tint(int display_num) {
  return 1000;
}

void emux_set_video_cache(int value) {
}

void emux_set_hw_scale(int value) {
}

void emux_cartridge_trigger_freeze(void) {
}

struct menu_item* emux_add_palette_options(int menu_id,
                                           struct menu_item* parent) {
}

void emux_add_machine_options(struct menu_item* parent) {
}

struct menu_item* emux_add_cartridge_options(struct menu_item* parent) {
}

void emux_set_warp(int warp) {
  ui_warp = warp;
}

void emux_change_palette(int display_num, int palette_index) {
  // Ignore
}

void emux_handle_rom_change(struct menu_item* item, fullpath_func fullpath) {
}

void emux_set_iec_dir(int unit, char* dir) {
}

void emux_set_int(IntSetting setting, int value) {
}

void emux_set_int_1(IntSetting setting, int value, int param) {
}

void emux_get_int(IntSetting setting, int* dest) {
   switch (setting) {
      case Setting_WarpMode:
          *dest = ui_warp;
          break;
      default:
          printf ("WARNING: Tried to get unsupported setting %d\n",setting);
          break;
   }
}

void emux_get_int_1(IntSetting setting, int* dest, int param) {
}

void emux_get_string_1(StringSetting setting, const char** dest, int param) {
}

int emux_save_settings(void) {
  return 0;
}

void emux_vice_attach_cart(int menu_id, char* filename) {
  assert(0);
}

void emux_vice_easy_flash(void) {
  assert(0);
}

int emux_handle_menu_change(struct menu_item* item) {
  switch (item->id) {
    case MENU_SID_MODEL:
    case MENU_SID_WRITE_D400:
    case MENU_SID_DIGIBLASTER:
      apply_sid_config();
      return 1;
  }
  return 0;
}

void emux_handle_load_setting(char *name, int value, char* value_str) {
  if (strcmp(name,"sid_model") == 0) {
     sid_model_item->value = value;
  } else if (strcmp(name,"sid_write_access") == 0) {
     sid_write_access_item->value = value;
  } else if (strcmp(name,"sid_digiblaster") == 0) {
     sid_digiblaster_item->value = value;
  }
}

void emux_handle_save_settings(FILE *fp) {
  fprintf (fp,"sid_model=%d\n", sid_model_item->value);
  fprintf (fp,"sid_write_access=%d\n", sid_write_access_item->value);
  fprintf (fp,"sid_digiblaster=%d\n", sid_digiblaster_item->value);
}

