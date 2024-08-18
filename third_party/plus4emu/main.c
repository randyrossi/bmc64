#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>
#include <ctype.h>
#include "plus4lib/plus4emu.h"
#include "../common/circle.h"
#include "../common/emux_api.h"
#include "../common/keycodes.h"
#include "../common/overlay.h"
#include "../common/demo.h"
#include "../common/menu.h"
#include "../common/kbd.h"

static Plus4VM            *vm = NULL;
static Plus4VideoDecoder  *videoDecoder = NULL;

#define TEXT_LINE_LEN 80
#define MAX_KEY_SYM 0x108
static int keysymToP4Code[0x108];

// Global state variables
static uint8_t *fb_buf;
static int fb_pitch;
static int ui_trap;
static int wait_vsync;
static int ui_warp;
static int joy_latch_value[2];
static int is_tape_motor;
static int is_tape_motor_tick;
static int is_tape_seeking;
static int is_tape_seeking_dir;
static int is_tape_seeking_tick;
static double tape_counter_offset;
static char last_iec_dir[256];
static int vertical_res;
static int raster_low;
static int time_advance;

// Things that need to be saved and restored.
int reset_tape_with_cpu = 1;
int tape_feedback = 0;
int ram_size = 64;
int sid_model = 0;
int sid_write_access = 0;
int sid_digiblaster = 0;
int drive_model_8 = 0;
int attach_3plus1_roms = 0;
int keyboard_mapping = 1;
char rom_basic[MAX_STR_VAL_LEN];
char rom_kernal[MAX_STR_VAL_LEN];
char rom_c0_lo[MAX_STR_VAL_LEN];
char rom_c0_hi[MAX_STR_VAL_LEN];
char rom_c1_lo[MAX_STR_VAL_LEN];
char rom_c1_hi[MAX_STR_VAL_LEN];
char rom_c2_lo[MAX_STR_VAL_LEN];
char rom_c2_hi[MAX_STR_VAL_LEN];
char rom_1541[MAX_STR_VAL_LEN];
char rom_1551[MAX_STR_VAL_LEN];
char rom_1581[MAX_STR_VAL_LEN];
int rom_basic_off;
int rom_kernal_off;
int rom_c0_lo_off;
int rom_c0_hi_off;
int rom_c1_lo_off;
int rom_c1_hi_off;
int rom_c2_lo_off;
int rom_c2_hi_off;
int rom_1541_off;
int rom_1551_off;
int rom_1581_lo_off;
int rom_1581_hi_off;
int color_brightness = 1000;
int color_contrast = 666;
int color_gamma = 800;
int color_tint = 1000;
int raster_skip = 1;
int crt_filter = 1;

static struct menu_item *sid_model_item;
static struct menu_item *sid_write_access_item;
static struct menu_item *sid_digiblaster_item;
static struct menu_item *tape_feedback_item;
static struct menu_item *ram_size_item;
static struct menu_item *drive_model_8_item;
static struct menu_item *attach_3plus1_roms_item;
static struct menu_item *keyboard_mapping_item;

static struct menu_item *c0_lo_item;
static struct menu_item *c0_hi_item;
static struct menu_item *c1_lo_item;
static struct menu_item *c1_hi_item;
static struct menu_item *c2_lo_item;
static struct menu_item *c2_hi_item;

static struct menu_item *c0_lo_offset_item;
static struct menu_item *c0_hi_offset_item;
static struct menu_item *c1_lo_offset_item;
static struct menu_item *c1_hi_offset_item;
static struct menu_item *c2_lo_offset_item;
static struct menu_item *c2_hi_offset_item;

static uint32_t prev_drive_state;
static int drive_led_colors[4];

#define COLOR16(r,g,b) (((r)>>3)<<11 | ((g)>>2)<<5 | (b)>>3)

static void init_video(void);

static int p4_isspace(char c) {
  return (c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v');
}

static void rtrim(char *txt) {
  if (!txt) return;
  int p=strlen(txt)-1;
  while (p4_isspace(txt[p])) { txt[p] = '\0'; p--; }
}

static char* ltrim(char *txt) {
  if (!txt) return NULL;
  int p=0;
  while (p4_isspace(txt[p])) { p++; }
  return txt+p;
}

static void get_key_and_value(char *line, char **key, char **value) {
   for (int i=0;i<strlen(line);i++) {
      if (line[i] == '=') {
         line[i] = '\0';
         *key = ltrim(&line[0]);
         rtrim(*key);
         *value = ltrim(&line[i+1]);
         rtrim(*value);
         return;
      }
   }
   *key = 0;
   *value = 0;
}

static void set_video_font(void) {
  int i;

  // Temporary for now. Need to figure out how to get this from the emulator's
  // ROM. Just read the file ourselves.
  uint8_t* chargen = malloc(4096); // never freed
  FILE* fp = fopen(rom_kernal, "r");
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

static int apply_rom_config() {
  if (Plus4VM_LoadROM(vm, 0x00, rom_basic, 0) != PLUS4EMU_SUCCESS)
    return 1;

  if (Plus4VM_LoadROM(vm, 0x01, rom_kernal, 0) != PLUS4EMU_SUCCESS)
    return 1;

  if (Plus4VM_LoadROM(vm, 0x10, rom_1541, 0) != PLUS4EMU_SUCCESS)
    return 1;

  Plus4VM_LoadROM(vm, 0x20, rom_1551, 0);
  Plus4VM_LoadROM(vm, 0x30, rom_1581, 0);
  Plus4VM_LoadROM(vm, 0x31, rom_1581, 16384);

  if (strlen(c0_lo_item->str_value) > 0)
    Plus4VM_LoadROM(vm, 2, c0_lo_item->str_value, c0_lo_offset_item->value);
  if (strlen(c0_hi_item->str_value) > 0)
    Plus4VM_LoadROM(vm, 3, c0_hi_item->str_value, c0_hi_offset_item->value);

  if (strlen(c1_lo_item->str_value) > 0)
    Plus4VM_LoadROM(vm, 4, c1_lo_item->str_value, c1_lo_offset_item->value);
  if (strlen(c1_hi_item->str_value) > 0)
    Plus4VM_LoadROM(vm, 5, c1_hi_item->str_value, c1_hi_offset_item->value);

  if (strlen(c2_lo_item->str_value) > 0)
    Plus4VM_LoadROM(vm, 6, c2_lo_item->str_value, c2_lo_offset_item->value);
  if (strlen(c2_hi_item->str_value) > 0)
    Plus4VM_LoadROM(vm, 7, c2_hi_item->str_value, c2_hi_offset_item->value);

  return 0;
}

static int apply_settings() {
  // Here, we should make whatever calls are necessary to configure the VM
  // according to any settings that were loaded.
  apply_sid_config();
  Plus4VM_SetTapeFeedbackLevel(vm, tape_feedback);
  Plus4VM_SetRAMConfiguration(vm, ram_size, 0x99999999UL);
  return apply_rom_config();
}

// This is only used for the latch func for row/col which
// is really only used for the virtual keyboard. Even USB
// GPIO ends up using the keymap so it must match the
// kernel's keysym table.
static int rowColToP4Code[8][8] = {
 {0, 8,  16, 24, 32, 48, 43, 56},
 {1, 9,  17, 25, 33, 41, 49, 52},
 {2, 10, 18, 26, 34, 42, 50, 58},
 {3, 11, 19, 27, 35, 51, 40, 59},
 {4, 12, 20, 28, 36, 44, 15, 60},
 {5, 13, 21, 29, 37, 45, 53, 61},
 {6, 14, 22, 30, 38, 46, 54, 62},
 {7, 15, 23, 31, 39, 47, 55, 63},
};

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
static int default_bmc64_keycode_to_plus4emu(long keycode) {
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
   if (raster_skip == 1) {
      lineNum = lineNum / 2 - raster_low;
   } else {
      lineNum = lineNum - raster_low;
   }
   if (lineNum >= 0 && lineNum < vertical_res) {
     Plus4VideoDecoder_DecodeLine(videoDecoder, fb_buf + lineNum * fb_pitch, 384, lineData);
   }
}

static void videoFrameCallback(void *userData)
{
  circle_frames_ready_fbl(FB_LAYER_VIC,
                          -1 /* no 2nd layer */,
                          !ui_warp /* sync */);

  // Something is waiting for vsync, ack and return.
  if (wait_vsync) {
    wait_vsync = 0;
    return;
  }

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
    int p4code = keysymToP4Code[pending_emu_key.key[i]];
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
      ui_trap = 0;
      circle_lock_release();
      emu_pause_trap(0, NULL);
      circle_lock_acquire();
  }

  circle_lock_release();

  ui_handle_toggle_or_quick_func();

  if (reset_demo) {
    demo_reset_timeout();
  }

  if (raspi_demo_mode) {
    demo_check();
  }

  if (is_tape_motor) {
     // Plus4Emu doesn't have a rewind/fastforward state so we
     // fake it here.
     if (is_tape_seeking) {
        is_tape_seeking_tick--;
        if (is_tape_seeking_tick == 0) {
          double pos = Plus4VM_GetTapePosition(vm);
          double newpos = pos + 1 * is_tape_seeking_dir;
          double len = Plus4VM_GetTapeLength(vm);
          if (newpos < 0)
             newpos = 0;
          else if (newpos > len)
             newpos = len;

          if (Plus4VM_TapeSeek(vm, newpos) != PLUS4EMU_SUCCESS) {
             is_tape_seeking = 0;
             is_tape_motor = 0;
          }

          int showing = (int)pos - (int)tape_counter_offset;
          if (showing < 0) showing += 1000;
          emux_display_tape_counter(showing);
          is_tape_seeking_tick = 5;
        }
     }
     is_tape_motor_tick--;
     if (is_tape_motor_tick == 0) {
       double pos = Plus4VM_GetTapePosition(vm);
       int showing = (int)pos - (int)tape_counter_offset;
       if (showing < 0) showing += 1000;
       emux_display_tape_counter(showing);
       is_tape_motor_tick = 50;
     }
  }

  // We only have one drive a.t.m.
  uint32_t drive_state = Plus4VM_GetFloppyDriveLEDState(vm);
  if (drive_state != prev_drive_state) {
    if (drive_state)
       emux_display_drive_led(0, 1000, 0);
    else
       emux_display_drive_led(0, 0, 0);
     prev_drive_state = drive_state;
  }
}

static void load_keymap(void) {
  FILE *fp = NULL;
  if (keyboard_mapping_item->value == KEYBOARD_MAPPING_POS) {
     fp = fopen("/PLUS4EMU/rpi_pos.vkm", "r");
  } else if (keyboard_mapping_item->value == KEYBOARD_MAPPING_MAXI) {
     fp = fopen("/PLUS4EMU/rpi_maxi_pos.vkm", "r");
  }
  char line[TEXT_LINE_LEN];
  if (fp != NULL) {
    while (fgets(line, TEXT_LINE_LEN - 1, fp)) {
      if (feof(fp))
        break;
      if (strlen(line) == 0)
        continue;
      if (line[0] == '#')
        continue;

      char *sym_name = strtok(line, " ");
      if (sym_name == NULL)
        continue;

      char *p4code_str = strtok(NULL, " ");
      if (p4code_str == NULL)
        continue;

      if (p4code_str[strlen(p4code_str) - 1] == '\n') {
          p4code_str[strlen(p4code_str) - 1] = '\0';
      }
      int p4code = atoi(p4code_str);
      int keysym = kbd_arch_keyname_to_keynum(sym_name);
      if (keysym > 0 && keysym < MAX_KEY_SYM) {
         keysymToP4Code[keysym] = p4code;
      } else {
         printf ("WARNING: Ignoring keysym %s\n",sym_name);
      }
    }
    fclose(fp);
  } else {
    printf ("WARNING: No keymap found, using default\n");
  }
}

static void machine_kbd_init(void) {
  // Discover these keys from the keymap.
  commodore_key_sym = KEYCODE_LeftControl;
  ctrl_key_sym = KEYCODE_Tab;
  restore_key_sym = KEYCODE_Home;
  commodore_key_sym_set = 0;
  ctrl_key_sym_set = 0;
  restore_key_sym_set = 0;

  for (int i=0;i<MAX_KEY_SYM;i++) {
     if (keysymToP4Code[i] == 61 && !commodore_key_sym_set) {
        commodore_key_sym = i;
        commodore_key_sym_set = 1;
     } else if (keysymToP4Code[i] == 58 && !ctrl_key_sym_set) {
        ctrl_key_sym = i;
        ctrl_key_sym_set = 1;
     } else if (keysymToP4Code[i] == 57 && !restore_key_sym_set) {
        restore_key_sym = i;
        restore_key_sym_set = 1;
     }
  }
}

// This is made to look like VICE's main entry point so our
// Plus4Emu version of EmulatorCore can look more or less the same
// as the Vice version.
int main_program(int argc, char **argv)
{
  (void) argc;
  (void) argv;

  printf ("Init\n");

  strcpy (last_iec_dir, ".");

  // Default keymap
  for (int i=0;i<MAX_KEY_SYM;i++) {
    keysymToP4Code[i] = default_bmc64_keycode_to_plus4emu(i);
  }

  int timing = circle_get_machine_timing();

  vm = Plus4VM_Create();
  if (!vm)
    errorMessage("could not create Plus/4 emulator object");

  Plus4VM_SetAudioOutputCallback(vm, &audioOutputCallback, NULL);
  if (Plus4VM_SetAudioOutputQuality(vm, 1) != PLUS4EMU_SUCCESS)
    vmError();

  int audioSampleRate;
  int fragsize;
  int fragnr;
  int channels = 1; // Only mono for plus4emu

  circle_sound_init(NULL, &audioSampleRate, &fragsize, &fragnr, &channels);
  if (Plus4VM_SetAudioSampleRate(vm, audioSampleRate) != PLUS4EMU_SUCCESS)
    vmError();

  if (Plus4VM_SetWorkingDirectory(vm, ".") != PLUS4EMU_SUCCESS)
    vmError();
  /* enable read-write IEC level drive emulation for unit 8 */
  Plus4VM_SetIECDriveReadOnlyMode(vm, 0);

  emux_detach_disk(8);

  videoDecoder =
      Plus4VideoDecoder_Create(&videoLineCallback, &videoFrameCallback, NULL);
  if (!videoDecoder)
    errorMessage("could not create video decoder object");
  Plus4VM_SetVideoOutputCallback(vm, &Plus4VideoDecoder_VideoCallback,
                                 (void *) videoDecoder);

  vic_enabled = 1; // really TED

  init_video();
  ui_init_menu(); // loads settings
  emux_geometry_changed(FB_LAYER_VIC);

  load_keymap();
  machine_kbd_init();

  strcpy(rom_basic,"/PLUS4EMU/p4_basic.rom");
  strcpy(rom_1541,"/PLUS4EMU/dos1541.rom");
  strcpy(rom_1551,"/PLUS4EMU/dos1551.rom");
  strcpy(rom_1581,"/PLUS4EMU/dos1581.rom");

  // Global settings vars have been restored by our load settings hook.
  // Use them to configure the VM.
  if (apply_settings()) {
     return -1;
  }

  set_video_font();

  printf ("Enter emulation loop\n");
  Plus4VM_Reset(vm, 1);

  // Fake two core init complete. Temp solution to get sound back for plus4emu.
  circle_kernel_core_init_complete(1);
  circle_kernel_core_init_complete(2);

  circle_boot_complete();

  assert(time_advance > 0);
  for(;;) {
    Plus4VM_Run(vm, time_advance);
  }

  Plus4VM_Destroy(vm);
  Plus4VideoDecoder_Destroy(videoDecoder);
  return 0;
}

// Begin emu_api impl.

void emu_machine_init(int raster_skip_enabled, int raster_skip2_enabled) {
  emux_machine_class = BMC64_MACHINE_CLASS_PLUS4EMU;

  raster_skip = raster_skip_enabled ? 2 : 1;

  // For plus4emu, raster_skip can't be turned off
  // at runtime. There's no line dupe like in our
  // VICE mod.
}

void emux_trap_main_loop_ui(void) {
  circle_lock_acquire();
  ui_trap = 1;
  circle_lock_release();
}

void emux_trap_main_loop(void (*trap_func)(uint16_t, void *data), void* data) {
}

void emux_kbd_set_latch_keyarr(int row, int col, int pressed) {
  int p4code = rowColToP4Code[col][row];
  if (p4code >= 0) {
    Plus4VM_KeyboardEvent(vm, p4code, pressed);
  }
}

int emux_attach_disk_image(int unit, char *filename) {
  if (Plus4VM_SetDiskImageFile(vm, unit-8, filename, drive_model_8) != PLUS4EMU_SUCCESS) {
    return 1;
  }
  emux_enable_drive_status(1, drive_led_colors);
  return 0;
}

void emux_detach_disk(int unit) {
  Plus4VM_SetWorkingDirectory(vm, last_iec_dir);
  Plus4VM_SetDiskImageFile(vm, unit-8, "", 0);
  Plus4VM_SetDiskImageFile(vm, unit-8, "", 1); // to enable IEC
  emux_enable_drive_status(0, drive_led_colors);
}

int emux_attach_tape_image(char *filename) {
  is_tape_seeking = 0;
  is_tape_motor = 0;
  emux_display_tape_counter(0);
  emux_display_tape_motor_status(EMUX_TAPE_STOP);
  if (Plus4VM_SetTapeFileName(vm, filename) != PLUS4EMU_SUCCESS) {
    return 1;
  }
  return 0;
}

void emux_detach_tape(void) {
  is_tape_seeking = 0;
  is_tape_motor = 0;
  emux_display_tape_counter(0);
  emux_display_tape_motor_status(EMUX_TAPE_STOP);
  Plus4VM_SetTapeFileName(vm, "");
}

int emux_attach_cart(int menu_id, char *filename) {
  int bank;
  int offset;
  struct menu_item* item;

  ui_info("Attaching...");

  switch (menu_id) {
     case MENU_PLUS4_CART_C0_LO_FILE:
        bank = 2;
        offset = c0_lo_offset_item->value;
        item = c0_lo_item;
        break;
     case MENU_PLUS4_CART_C0_HI_FILE:
        bank = 3;
        offset = c0_hi_offset_item->value;
        item = c0_hi_item;
        break;
     case MENU_PLUS4_CART_C1_LO_FILE:
        bank = 4;
        offset = c1_lo_offset_item->value;
        item = c1_lo_item;
        break;
     case MENU_PLUS4_CART_C1_HI_FILE:
        bank = 5;
        offset = c1_hi_offset_item->value;
        item = c1_hi_item;
        break;
     case MENU_PLUS4_CART_C2_LO_FILE:
        bank = 6;
        offset = c2_lo_offset_item->value;
        item = c2_lo_item;
        break;
     case MENU_PLUS4_CART_C2_HI_FILE:
        bank = 7;
        offset = c2_hi_offset_item->value;
        item = c2_hi_item;
        break;
     default:
        assert(0);
  }

  if (Plus4VM_LoadROM(vm, bank, filename, offset) != PLUS4EMU_SUCCESS) {
     ui_pop_menu();
     ui_error("Failed to attach cart image");
     return 1;
  } else {
     ui_pop_all_and_toggle();
     Plus4VM_Reset(vm, 1);
  }

  // Update attached cart name
  strncpy(item->str_value, filename, MAX_STR_VAL_LEN - 1);
  strncpy(item->displayed_value, filename, MAX_DSP_VAL_LEN - 1);
  return 0;
}

void emux_set_cart_default(void) {
}

void emux_detach_cart(int menu_id) {
  int bank;
  struct menu_item* item;
  switch (menu_id) {
     case MENU_PLUS4_DETACH_CART_C0_LO:
        bank = 2;
        item = c0_lo_item;
        break;
     case MENU_PLUS4_DETACH_CART_C0_HI:
        bank = 3;
        item = c0_hi_item;
        break;
     case MENU_PLUS4_DETACH_CART_C1_LO:
        bank = 4;
        item = c1_lo_item;
        break;
     case MENU_PLUS4_DETACH_CART_C1_HI:
        bank = 5;
        item = c1_hi_item;
        break;
     case MENU_PLUS4_DETACH_CART_C2_LO:
        bank = 6;
        item = c2_lo_item;
        break;
     case MENU_PLUS4_DETACH_CART_C2_HI:
        bank = 7;
        item = c2_hi_item;
        break;
     default:
        break;
  }

  Plus4VM_LoadROM(vm, bank, "", 0);
  Plus4VM_Reset(vm, 1);
  item->displayed_value[0] = '\0';
  item->str_value[0] = '\0';
}

static void reset_tape_drive() {
  Plus4VM_TapeSeek(vm, 0);
  Plus4VM_TapeStop(vm);
  is_tape_motor = 0;
  tape_counter_offset = 0;
  emux_display_tape_counter(0);
}

void emux_reset(int isSoft) {
  Plus4VM_Reset(vm, !isSoft);
  if (reset_tape_with_cpu) {
     emux_display_tape_control_status(EMUX_TAPE_STOP);
     reset_tape_drive();
     emux_display_tape_motor_status(is_tape_motor);
  }
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
    emux_display_tape_control_status(cmd);
    switch (cmd) {
    case EMUX_TAPE_PLAY:
      Plus4VM_TapePlay(vm);
      is_tape_seeking = 0;
      is_tape_motor = 1;
      is_tape_motor_tick = 50;
      break;
    case EMUX_TAPE_STOP:
      Plus4VM_TapeStop(vm);
      is_tape_seeking = 0;
      is_tape_motor = 0;
      break;
    case EMUX_TAPE_REWIND:
      is_tape_seeking = 1;
      is_tape_seeking_dir = -1;
      is_tape_seeking_tick = 5;
      is_tape_motor = 1;
      is_tape_motor_tick = 50;
      break;
    case EMUX_TAPE_FASTFORWARD:
      is_tape_seeking = 1;
      is_tape_seeking_dir = 1;
      is_tape_seeking_tick = 5;
      is_tape_motor = 1;
      is_tape_motor_tick = 50;
      break;
    case EMUX_TAPE_RECORD:
      Plus4VM_TapeRecord(vm);
      is_tape_seeking = 0;
      is_tape_motor = 1;
      is_tape_motor_tick = 50;
      break;
    case EMUX_TAPE_RESET:
      reset_tape_drive();
      break;
    case EMUX_TAPE_ZERO:
      tape_counter_offset = Plus4VM_GetTapePosition(vm);
      emux_display_tape_counter(0);
      break;
    default:
      assert(0);
      break;
  }
  emux_display_tape_motor_status(is_tape_motor);
}

void emux_show_cart_osd_menu(void) {
}

unsigned long emux_calculate_timing(double fps) {
  // TODO: Enable custom timing calc in common when this if fixed.
  return 0;
}

double emux_calculate_fps() {
  // TODO: Enable custom timing calc in common when this if fixed.
  if (is_ntsc()) {
    return 60;
  }
  return 50;
}

// Not really an autostart, just loads .PRG. TODO: Rename this.
int emux_autostart_file(char* filename) {
  if (Plus4VM_LoadProgram(vm, filename) != PLUS4EMU_SUCCESS) {
     return 1;
  }
  return 0;
}

void emux_drive_change_model(int unit) {
}

void emux_add_drive_option(struct menu_item* parent, int drive) {
  if (drive == 8) {
    drive_model_8_item =
        ui_menu_add_multiple_choice(MENU_DRIVE_TYPE_8, parent, "Drive Model");
    drive_model_8_item->num_choices = 2;
    drive_model_8_item->value = drive_model_8;
    strcpy(drive_model_8_item->choices[0], "1541");
    strcpy(drive_model_8_item->choices[1], "1551");
  }
}

void emux_create_disk(struct menu_item* item, fullpath_func fullpath) {
  // Not supported for plus/4
}

void emux_create_tape(struct menu_item* item, fullpath_func fullpath) {
  // Not supported for plus/4
}

void emux_set_joy_port_device(int port_num, int dev_id) {
}

void emux_set_joy_pot_x(int port,int value) {
  // Not supported on plus4emu
}

void emux_set_joy_pot_y(int port, int value) {
  // Not supported on plus4emu
}

void emux_add_tape_options(struct menu_item* parent) {
  tape_feedback_item =
      ui_menu_add_range(MENU_TAPE_FEEDBACK, parent,
          "Tape Audible Feedback Level", 0, 10, 1, tape_feedback);
}

void emux_add_keyboard_options(struct menu_item* parent) {
  keyboard_mapping_item = ui_menu_add_multiple_choice(
      MENU_KEYBOARD_MAPPING, parent, "Mapping");
  keyboard_mapping_item->num_choices = 3;

  int tmp_value;
  keyboard_mapping_item->value = keyboard_mapping;
  strcpy(keyboard_mapping_item->choices[KEYBOARD_MAPPING_SYM], "Symbolic");
  strcpy(keyboard_mapping_item->choices[KEYBOARD_MAPPING_POS], "Positional");
  strcpy(keyboard_mapping_item->choices[KEYBOARD_MAPPING_MAXI], "Maxi Positional");

  // Do this for now in case we ever support this some day.
  keyboard_mapping_item->choice_disabled[KEYBOARD_MAPPING_SYM] = 1;
}

void emux_add_sound_options(struct menu_item* parent) {
  // TODO: Why is 6581 so slow?
  struct menu_item* child = sid_model_item =
      ui_menu_add_multiple_choice(MENU_SID_MODEL, parent, "Sid Model");
  child->num_choices = 1;
  child->value = sid_model;
  strcpy(child->choices[0], "8580");
  child->choice_ints[0] = 0; // 8580 for sidflags

  // Write access at $d400-d41f
  sid_write_access_item =
      ui_menu_add_toggle(MENU_SID_WRITE_D400, parent,
          "Write Access $D400-D41F", sid_write_access);

  // Digiblaster
  sid_digiblaster_item =
      ui_menu_add_toggle(MENU_SID_DIGIBLASTER, parent,
          "Enable Digiblaster", sid_digiblaster);
}

void emux_video_color_setting_changed(int display_num) {
  Plus4VideoDecoder_UpdatePalette(videoDecoder);
  // Plus4Emu doesn't use an indexed palette so we have to allow
  // the decoder to draw a frame after we change a color param.
  wait_vsync = 1;
  do {
    Plus4VM_Run(vm, 2000);
  } while (wait_vsync);
}

void emux_set_color_brightness(int display_num, int value) {
  // Incoming 0-2000, Outgoing -.5 - .5
  // Default 0
  color_brightness = value;
  double v = value;
  v = v - 1000;
  v = v / 2000;
  Plus4VideoDecoder_SetBrightness(videoDecoder,v,v,v,v);
}

void emux_set_color_contrast(int display_num, int value) {
  // Incoming 0-2000, Outgoing .5 - 2.0
  // Default 1
  color_contrast = value;
  double v = value / 1333.33d;
  v = v + .5;
  if (v < .5) v = .5;
  if (v > 2.0) v = 2.0;
  Plus4VideoDecoder_SetContrast(videoDecoder,v,v,v,v);
}

void emux_set_color_gamma(int display_num, int value) {
  // Incoming 0-4000, Outgoing .25 - 4.0
  // Default 1
  color_gamma = value;
  double v = value / 1066.66d;
  v = v + .25;
  if (v < .25) v = .25;
  if (v > 4.0) v = 4.0;
  Plus4VideoDecoder_SetGamma(videoDecoder,v,v,v,v);
}

void emux_set_color_tint(int display_num, int value) {
  // Incoming 0-2000, Outgoing -180, 180
  // Default 0
  color_tint = value;
  double v = value / 5.555d;
  v = v - 180;
  if (v < -180) v = -180;
  if (v > 180) v = 180;
  Plus4VideoDecoder_SetHueShift(videoDecoder,v);
}

void emux_set_color_saturation(int display_num, int value) {
  // Not supported
}

int emux_get_color_brightness(int display_num) {
  return color_brightness;
}

int emux_get_color_contrast(int display_num) {
  return color_contrast;
}

int emux_get_color_gamma(int display_num) {
  return color_gamma;
}

int emux_get_color_tint(int display_num) {
  return color_tint;
}

int emux_get_color_saturation(int display_num) {
  // Not supported
  return 1000;
}

void emux_set_video_cache(int value) {
  // Ignore for plus/4
}

void emux_set_hw_scale(int value) {
  // Ignore for plus/4
}

struct menu_item* emux_add_palette_options(int menu_id,
                                           struct menu_item* parent) {
  // None for plus/4
}

static void menu_value_changed(struct menu_item *item) {
  // Forward to our emux_ handler
  emux_handle_menu_change(item);
}

void emux_add_machine_options(struct menu_item* parent) {
  // TODO : Memory and cartridge configurations
  // C16-16k
  // C16-64k
  // Plus/4-64k

  ram_size_item =
      ui_menu_add_multiple_choice(MENU_MEMORY, parent, "Memory");
  ram_size_item->num_choices = 5;

  switch (ram_size) {
    case 16:
      ram_size_item->value = 0;
      break;
    case 32:
      ram_size_item->value = 1;
      break;
    case 64:
      ram_size_item->value = 2;
      break;
    case 256:
      ram_size_item->value = 3;
      break;
    case 1024:
    default:
      ram_size_item->value = 4;
      break;
  }

  strcpy(ram_size_item->choices[0], "16k");
  strcpy(ram_size_item->choices[1], "32k");
  strcpy(ram_size_item->choices[2], "64k");
  strcpy(ram_size_item->choices[3], "256k");
  strcpy(ram_size_item->choices[4], "1024k");
  ram_size_item->choice_ints[0] = 16;
  ram_size_item->choice_ints[1] = 32;
  ram_size_item->choice_ints[2] = 64;
  ram_size_item->choice_ints[3] = 256;
  ram_size_item->choice_ints[4] = 1024;
  ram_size_item->on_value_changed = menu_value_changed;

  attach_3plus1_roms_item = ui_menu_add_toggle(MENU_PLUS4_3PLUS1_ROMS, parent,
          "Attach 3Plus1 ROMs", attach_3plus1_roms);
}

struct menu_item* emux_add_cartridge_options(struct menu_item* root) {
  struct menu_item* parent = ui_menu_add_folder(root, "Cartridge");

  ui_menu_add_divider(parent);
  c0_lo_item = ui_menu_add_button_with_value(MENU_TEXT, parent,
     "C0 LO:",0,rom_c0_lo,"");
  c0_lo_item->prefer_str = 1;
  strncpy(c0_lo_item->displayed_value, rom_c0_lo, MAX_DSP_VAL_LEN - 1);
  ui_menu_add_button(MENU_PLUS4_ATTACH_CART_C0_LO, parent, "Attach...");
  c0_lo_offset_item = ui_menu_add_range(MENU_PLUS4_ATTACH_CART_C0_LO_OFFSET,
     parent, "Offset", 0, 16384, 16384, rom_c0_lo_off);
  ui_menu_add_button(MENU_PLUS4_DETACH_CART_C0_LO, parent, "Detach");
  ui_menu_add_divider(parent);

  c0_hi_item = ui_menu_add_button_with_value(MENU_TEXT, parent,
      "C0 HI:",0,rom_c0_hi,"");
  c0_hi_item->prefer_str = 1;
  strncpy(c0_hi_item->displayed_value, rom_c0_hi, MAX_DSP_VAL_LEN - 1);
  ui_menu_add_button(MENU_PLUS4_ATTACH_CART_C0_HI, parent, "Attach...");
  c0_hi_offset_item = ui_menu_add_range(MENU_PLUS4_ATTACH_CART_C0_HI_OFFSET,
     parent, "Offset", 0, 16384, 16384, rom_c0_hi_off);
  ui_menu_add_button(MENU_PLUS4_DETACH_CART_C0_HI, parent, "Detach");
  ui_menu_add_divider(parent);

  c1_lo_item = ui_menu_add_button_with_value(MENU_TEXT, parent,
      "C1 LO:",0,rom_c1_lo,"");
  c1_lo_item->prefer_str = 1;
  strncpy(c1_lo_item->displayed_value, rom_c1_lo, MAX_DSP_VAL_LEN - 1);
  ui_menu_add_button(MENU_PLUS4_ATTACH_CART_C1_LO, parent, "Attach...");
  c1_lo_offset_item = ui_menu_add_range(MENU_PLUS4_ATTACH_CART_C1_LO_OFFSET,
      parent, "Offset", 0, 16384, 16384, rom_c1_lo_off);
  ui_menu_add_button(MENU_PLUS4_DETACH_CART_C1_LO, parent, "Detach");
  ui_menu_add_divider(parent);

  c1_hi_item = ui_menu_add_button_with_value(MENU_TEXT, parent,
      "C1 HI:",0,rom_c1_hi,"");
  c1_hi_item->prefer_str = 1;
  strncpy(c1_hi_item->displayed_value, rom_c1_hi, MAX_DSP_VAL_LEN - 1);
  ui_menu_add_button(MENU_PLUS4_ATTACH_CART_C1_HI, parent, "Attach...");
  c1_hi_offset_item = ui_menu_add_range(MENU_PLUS4_ATTACH_CART_C1_HI_OFFSET,
      parent, "Offset", 0, 16384, 16384, rom_c1_hi_off);
  ui_menu_add_button(MENU_PLUS4_DETACH_CART_C1_HI, parent, "Detach");
  ui_menu_add_divider(parent);

  c2_lo_item = ui_menu_add_button_with_value(MENU_TEXT, parent,
      "C2 LO:",0,rom_c2_lo,"");
  c2_lo_item->prefer_str = 1;
  strncpy(c2_lo_item->displayed_value, rom_c2_lo, MAX_DSP_VAL_LEN - 1);
  ui_menu_add_button(MENU_PLUS4_ATTACH_CART_C2_LO, parent, "Attach...");
  c2_lo_offset_item = ui_menu_add_range(MENU_PLUS4_ATTACH_CART_C2_LO_OFFSET,
      parent, "Offset", 0, 16384, 16384, rom_c2_lo_off);
  ui_menu_add_button(MENU_PLUS4_DETACH_CART_C2_LO, parent, "Detach");
  ui_menu_add_divider(parent);

  c2_hi_item = ui_menu_add_button_with_value(MENU_TEXT, parent,
      "C2 HI:",0,rom_c2_hi,"");
  c2_hi_item->prefer_str = 1;
  strncpy(c2_hi_item->displayed_value, rom_c2_hi, MAX_DSP_VAL_LEN - 1);
  ui_menu_add_button(MENU_PLUS4_ATTACH_CART_C2_HI, parent, "Attach...");
  c2_hi_offset_item = ui_menu_add_range(MENU_PLUS4_ATTACH_CART_C2_HI_OFFSET,
      parent, "Offset", 0, 16384, 16384, rom_c2_hi_off);
  ui_menu_add_button(MENU_PLUS4_DETACH_CART_C2_HI, parent, "Detach");

  return parent;
}

void emux_set_warp(int warp) {
  ui_warp = warp;
}

void emux_change_palette(int display_num, int palette_index) {
  // Never called for Plus4Emu
}

void emux_handle_rom_change(struct menu_item* item, fullpath_func fullpath) {
}

void emux_set_iec_dir(int unit, char* dir) {
  Plus4VM_SetWorkingDirectory(vm, dir);
  strcpy (last_iec_dir, dir);
}

void emux_set_int(IntSetting setting, int value) {
  switch (setting) {
    case Setting_DatasetteResetWithCPU:
       reset_tape_with_cpu = value;
       break;
    case Setting_Datasette:
       // Not applicable
       break;
    case Setting_VideoFilter:
       crt_filter = value;
       break;
    default:
       printf ("Unhandled set int %d\n", setting);
  }
}

void emux_set_int_1(IntSetting setting, int value, int param) {
  switch (setting) {
     case Setting_FileSystemDeviceN:
     case Setting_IECDeviceN:
        // Nothing to do
        break;
     default:
        printf ("Unhandled set int_1 %d\n", setting);
        break;
  }
}

void emux_get_int(IntSetting setting, int* dest) {
   switch (setting) {
      case Setting_WarpMode:
          *dest = ui_warp;
          break;
      case Setting_DatasetteResetWithCPU:
          *dest = reset_tape_with_cpu;
          break;
      case Setting_DriveSoundEmulation:
      case Setting_DriveSoundEmulationVolume:
          *dest = 0;
          // Not applicable
          break;
      case Setting_VideoFilter:
          *dest = crt_filter;
          break;
      default:
          printf ("WARNING: Tried to get unsupported setting %d\n",setting);
          break;
   }
}

void emux_get_int_1(IntSetting setting, int* dest, int param) {
  switch (setting) {
     case Setting_FileSystemDeviceN:
        *dest = 1;
        return;
     case Setting_IECDeviceN:
        // nothing to do, always capable of iec with no disk
        *dest = 1;
        break;
     default:
        printf ("Unhandled get int_1 %d\n", setting);
  }
}

void emux_get_string_1(StringSetting setting, const char** dest, int param) {
  switch (setting) {
     case Setting_FSDeviceNDir:
        *dest = last_iec_dir;
        break;
     default:
        printf ("Unhandled set string_1 %d\n", setting);
        break;
  }
}

int emux_save_settings(void) {
  // All our  additional settings are handled by emux_save_additional_settings
  // Nothing to do here.
  return 0;
}

// Handle any menu item we've created for this emulator.
int emux_handle_menu_change(struct menu_item* item) {
  switch (item->id) {
    case MENU_SID_MODEL:
    case MENU_SID_WRITE_D400:
    case MENU_SID_DIGIBLASTER:
      apply_sid_config();
      return 1;
    case MENU_TAPE_FEEDBACK:
      Plus4VM_SetTapeFeedbackLevel(vm, item->value);
      return 1;
    case MENU_MEMORY:
      Plus4VM_SetRAMConfiguration(vm,
         ram_size_item->choice_ints[ram_size_item->value], 0x99999999UL);
      apply_rom_config();
      return 1;
    case MENU_DRIVE_TYPE_8:
      // Prevent common handler from seeing this.
      drive_model_8 = item->value;
      return 1;
    case MENU_PLUS4_3PLUS1_ROMS:
      if (item->value) {
         strcpy (c0_lo_item->str_value,"/PLUS4EMU/3plus1.rom");
         strcpy (c0_lo_item->displayed_value,"/PLUS4EMU/3plus1.rom");
         c0_lo_offset_item->value = 0;
         strcpy (c0_hi_item->str_value,"/PLUS4EMU/3plus1.rom");
         strcpy (c0_hi_item->displayed_value,"/PLUS4EMU/3plus1.rom");
         c0_hi_offset_item->value = 16384;
      } else {
         strcpy (c0_lo_item->str_value,"");
         strcpy (c0_lo_item->displayed_value,"");
         c0_lo_offset_item->value = 0;
         strcpy (c0_hi_item->str_value,"");
         strcpy (c0_hi_item->displayed_value,"");
         c0_hi_offset_item->value = 0;
      }
      Plus4VM_LoadROM(vm, 2, c0_lo_item->str_value, c0_lo_offset_item->value);
      Plus4VM_LoadROM(vm, 3, c0_hi_item->str_value, c0_hi_offset_item->value);
      Plus4VM_Reset(vm, 1);
      return 1;
    case MENU_KEYBOARD_MAPPING:
      load_keymap();
      machine_kbd_init();
      return 1;
    default:
      return 0;
  }
}

int emux_handle_quick_func(int button_func, fullpath_func fullpath) {
  return 0;
}

// For Plus4emu, we grab additional settings from the same txt file.
void emux_load_additional_settings() {
  // NOTE: This is called before any menu items have been constructed.

  strcpy(rom_c0_lo, "");
  strcpy(rom_c0_hi, "");
  strcpy(rom_c1_lo, "");
  strcpy(rom_c1_hi, "");
  strcpy(rom_c2_lo, "");
  strcpy(rom_c2_hi, "");

  FILE *fp;
  fp = fopen("/settings-plus4emu.txt", "r");
  if (fp == NULL) {
     return;
  }

  char name_value[256];
  size_t len;
  int value;
  int usb_btn_0_i = 0;
  int usb_btn_1_i = 0;
  while (1) {
    char *line = fgets(name_value, 255, fp);
    if (feof(fp) || line == NULL) break;

    strcpy(name_value, line);

    char *name;
    char *value_str;
    get_key_and_value(name_value, &name, &value_str);
    if (!name || !value_str ||
       strlen(name) == 0 ||
          strlen(value_str) == 0) {
       continue;
    }

    value = atoi(value_str);

    if (strcmp(name,"sid_model") == 0) {
       sid_model = value;
    } else if (strcmp(name,"sid_write_access") == 0) {
       sid_write_access = value;
    } else if (strcmp(name,"sid_digiblaster") == 0) {
       sid_digiblaster = value;
    } else if (strcmp(name,"reset_tape_with_cpu") == 0) {
       reset_tape_with_cpu = value;
    } else if (strcmp(name,"tape_feedback") == 0) {
       tape_feedback = value;
    } else if (strcmp(name,"ram_size") == 0) {
       ram_size = value;
    } else if (strcmp(name,"drive_model_8") == 0) {
       drive_model_8 = value;
    } else if (strcmp(name,"attach_3plus1_roms") == 0) {
       attach_3plus1_roms = value;
    } else if (strcmp(name,"rom_c0_lo") == 0) {
       strcpy(rom_c0_lo, value_str);
    } else if (strcmp(name,"rom_c0_hi") == 0) {
       strcpy(rom_c0_hi, value_str);
    } else if (strcmp(name,"rom_c1_lo") == 0) {
       strcpy(rom_c1_lo, value_str);
    } else if (strcmp(name,"rom_c1_hi") == 0) {
       strcpy(rom_c1_hi, value_str);
    } else if (strcmp(name,"rom_c2_lo") == 0) {
       strcpy(rom_c2_lo, value_str);
    } else if (strcmp(name,"rom_c2_hi") == 0) {
       strcpy(rom_c2_hi, value_str);
    } else if (strcmp(name,"rom_c0_lo_off") == 0) {
       rom_c0_lo_off = value;
    } else if (strcmp(name,"rom_c0_hi_off") == 0) {
       rom_c0_hi_off = value;
    } else if (strcmp(name,"rom_c1_lo_off") == 0) {
       rom_c1_lo_off = value;
    } else if (strcmp(name,"rom_c1_hi_off") == 0) {
       rom_c1_hi_off = value;
    } else if (strcmp(name,"rom_c2_lo_off") == 0) {
       rom_c2_lo_off = value;
    } else if (strcmp(name,"rom_c2_hi_off") == 0) {
       rom_c2_hi_off = value;
    } else if (strcmp(name,"color_brightness") == 0) {
       color_brightness = value;
    } else if (strcmp(name,"color_contrast") == 0) {
       color_contrast = value;
    } else if (strcmp(name,"color_gamma") == 0) {
       color_gamma = value;
    } else if (strcmp(name,"color_tint") == 0) {
       color_tint = value;
    } else if (strcmp(name,"keyboard_mapping") == 0) {
       keyboard_mapping = value;
    } else if (strcmp(name,"crt_filter") == 0) {
       crt_filter = value;
    }
  }

  fclose(fp);
}

void emux_save_additional_settings(FILE *fp) {
  fprintf (fp,"sid_model=%d\n", sid_model_item->value);
  fprintf (fp,"sid_write_access=%d\n", sid_write_access_item->value);
  fprintf (fp,"sid_digiblaster=%d\n", sid_digiblaster_item->value);
  fprintf (fp,"reset_tape_with_cpu=%d\n", reset_tape_with_cpu);
  fprintf (fp,"tape_feedback=%d\n", tape_feedback_item->value);
  fprintf (fp,"ram_size=%d\n", ram_size_item->choice_ints[ram_size_item->value]);
  fprintf (fp,"drive_model_8=%d\n", drive_model_8_item->value);
  fprintf (fp,"attach_3plus1_roms=%d\n", attach_3plus1_roms_item->value);
  if (strlen(c0_lo_item->str_value) > 0) {
     fprintf (fp,"rom_c0_lo=%s\n", c0_lo_item->str_value);
  }
  if (strlen(c0_hi_item->str_value) > 0) {
     fprintf (fp,"rom_c0_hi=%s\n", c0_hi_item->str_value);
  }
  if (strlen(c1_lo_item->str_value) > 0) {
     fprintf (fp,"rom_c1_lo=%s\n", c1_lo_item->str_value);
  }
  if (strlen(c1_hi_item->str_value) > 0) {
     fprintf (fp,"rom_c1_hi=%s\n", c1_hi_item->str_value);
  }
  if (strlen(c2_lo_item->str_value) > 0) {
     fprintf (fp,"rom_c2_lo=%s\n", c2_lo_item->str_value);
  }
  if (strlen(c2_hi_item->str_value) > 0) {
     fprintf (fp,"rom_c2_hi=%s\n", c2_hi_item->str_value);
  }
  fprintf (fp,"rom_c0_lo_off=%d\n", c0_lo_offset_item->value);
  fprintf (fp,"rom_c0_hi_off=%d\n", c0_hi_offset_item->value);
  fprintf (fp,"rom_c1_lo_off=%d\n", c1_lo_offset_item->value);
  fprintf (fp,"rom_c1_hi_off=%d\n", c1_hi_offset_item->value);
  fprintf (fp,"rom_c2_lo_off=%d\n", c2_lo_offset_item->value);
  fprintf (fp,"rom_c2_hi_off=%d\n", c2_hi_offset_item->value);
  fprintf (fp,"color_brightness=%d\n", color_brightness);
  fprintf (fp,"color_contrast=%d\n", color_contrast);
  fprintf (fp,"color_gamma=%d\n", color_gamma);
  fprintf (fp,"tintcolor_=%d\n", color_tint);
  fprintf (fp,"keyboard_mapping=%d\n", keyboard_mapping_item->value);
  fprintf (fp,"crt_filter=%d\n", crt_filter);
}

void emux_get_default_color_setting(int *brightness, int *contrast,
                                    int *gamma, int *tint, int *saturation) {
  *brightness = 1000;
  *contrast = 666;
  *gamma = 800;
  *tint = 1000;
  *saturation = 1000;
}

int emux_handle_loaded_setting(char *name, char* value_str, int value) {
  return 0;
}

void emux_load_settings_done(void) {
}

static void init_video(void) {
  if (is_ntsc()) {
     vertical_res = 242 * raster_skip;
     raster_low = 18 * raster_skip;
  } else {
     vertical_res = 288 * raster_skip;
     raster_low = 0;
  }

  // BMC64 Video Init
  if (circle_alloc_fbl(FB_LAYER_VIC, 1 /* RGB565 */, &fb_buf,
                              384, vertical_res, &fb_pitch)) {
    printf ("Failed to create video buf.\n");
    assert(0);
  }
  circle_clear_fbl(FB_LAYER_VIC);
  circle_show_fbl(FB_LAYER_VIC);

  canvas_state[VIC_INDEX].gfx_w = 40*8;
  canvas_state[VIC_INDEX].gfx_h = 25*8 * raster_skip;
  canvas_state[VIC_INDEX].raster_skip = raster_skip;

  if (is_ntsc()) {
    canvas_state[VIC_INDEX].max_padding_w = 0;
    canvas_state[VIC_INDEX].max_padding_h = 0;
    canvas_state[VIC_INDEX].max_border_w = 32;
    canvas_state[VIC_INDEX].max_border_h = 22 * raster_skip;
    time_advance = 1666;
    Plus4VM_SetVideoClockFrequency(vm, 14318180);
    strcpy(rom_kernal,"/PLUS4EMU/p4_ntsc.rom");
    Plus4VideoDecoder_SetNTSCMode(videoDecoder, 1);
  } else {
    canvas_state[VIC_INDEX].max_padding_w = 0;
    canvas_state[VIC_INDEX].max_padding_h = 0;
    canvas_state[VIC_INDEX].max_border_w = 32;
    canvas_state[VIC_INDEX].max_border_h = 40 * raster_skip;
    time_advance = 2000;
    Plus4VM_SetVideoClockFrequency(vm, 17734475);
    strcpy(rom_kernal,"/PLUS4EMU/p4kernal.rom");
    Plus4VideoDecoder_SetNTSCMode(videoDecoder, 0);
  }
}

void emux_add_userport_joys(struct menu_item* parent) {
}

uint8_t circle_get_userport_ddr(void) {
  return 0;
}

uint8_t circle_get_userport(void) {
  return 0xff;
}

void circle_set_userport(uint8_t value) {
}

