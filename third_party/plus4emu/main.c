#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>
#include "plus4lib/plus4emu.h"
#include "../common/circle.h"
#include "../common/keycodes.h"

static Plus4VM            *vm = NULL;
static Plus4VideoDecoder  *videoDecoder = NULL;

static uint8_t          *fb_buf;
static int              fb_pitch;

#define COLOR16(r,g,b) (((r)>>3)<<11 | ((g)>>2)<<5 | (b)>>3)

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
  circle_frames_ready_fbl(FB_LAYER_VIC, -1 /* no 2nd layer */, 1 /* sync */);
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

// Used for injecting test strings into VM
// [=shift down
// ]=shift up
// {=return down
// }=return up
// \=* down
// |=* up
static void fake_key(char c) {
   if (c >= 'a' && c <='z') {
        emu_key_pressed(KEYCODE_a+(c-'a'));
   } else if (c >= 'A' && c <='Z') {
        emu_key_released(KEYCODE_a+(c-'A'));
   } else if (c == '[') {
        emu_key_pressed(KEYCODE_LeftShift);
   } else if (c == ']') {
        emu_key_released(KEYCODE_LeftShift);
   } else if (c == '{') {
        emu_key_pressed(KEYCODE_Return);
   } else if (c == '}') {
        emu_key_released(KEYCODE_Return);
   } else if (c == ',') {
        emu_key_pressed(KEYCODE_Comma);
   } else if (c == '<') {
        emu_key_released(KEYCODE_Comma);
   } else if (c == '0') {
        emu_key_pressed(KEYCODE_0);
   } else if (c >= '1' && c <='9') {
        emu_key_pressed(KEYCODE_1 + (c-'1'));
   } else if (c == '!') {
        emu_key_released(KEYCODE_1);
   } else if (c == '@') {
        emu_key_released(KEYCODE_2);
   } else if (c == '#') {
        emu_key_released(KEYCODE_3);
   } else if (c == '$') {
        emu_key_released(KEYCODE_4);
   } else if (c == '%') {
        emu_key_released(KEYCODE_5);
   } else if (c == '^') {
        emu_key_released(KEYCODE_6);
   } else if (c == '&') {
        emu_key_released(KEYCODE_7);
   } else if (c == '*') {
        emu_key_released(KEYCODE_8);
   } else if (c == '(') {
        emu_key_released(KEYCODE_9);
   } else if (c == ')') {
        emu_key_released(KEYCODE_0);
   } else if (c == '\\') {
        emu_key_pressed(KEYCODE_Dash);
   } else if (c == '|') {
        emu_key_released(KEYCODE_Dash);
   }
}

// Inject LOAD"*",8,1 into emulator.
static void fake_inject() {
  static char *test_str = "lLoOaAdD[2@]\\|[2@],<8*,<1!{}";
  static int counter = 0;
  static int pos = 0;
  counter++;
  if (counter >= 50*8) {
     if (pos < strlen(test_str)) {
        printf("fake %c\n",test_str[pos]);
        fake_key(test_str[pos]);
        pos++;
     }
     counter = counter - 100; // 2 seconds apart
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
  //Plus4VM_SetIECDriveReadOnlyMode(vm, 0);
  if (Plus4VM_SetDiskImageFile(vm, 0, "disks/plus4/advmono.d64", 0) != PLUS4EMU_SUCCESS)
    vmError();
  Plus4VM_Reset(vm, 1);

  videoDecoder =
      Plus4VideoDecoder_Create(&videoLineCallback, &videoFrameCallback, NULL);
  if (!videoDecoder)
    errorMessage("could not create video decoder object");
  //Plus4VideoDecoder_UpdatePalette(videoDecoder, 0, 16, 8, 0); // not using rgb
  Plus4VM_SetVideoOutputCallback(vm, &Plus4VideoDecoder_VideoCallback,
                                 (void *) videoDecoder);

  /* run Plus/4 emulation until the F12 key is pressed */
  printf ("Enter emulation loop\n");

  do {
    if (Plus4VM_Run(vm, 2000) != PLUS4EMU_SUCCESS)
      vmError();

#ifdef FAKE_INJECT
    fake_inject();
#endif

  } while (!quitFlag);

  Plus4VM_Destroy(vm);
  Plus4VideoDecoder_Destroy(videoDecoder);
  return 0;
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
         return 0;
   }
}

// TODO - Common kbd.c will implement this once it is moved up. For now, call
// into VM here.
void emu_key_pressed(long key) {
   Plus4VM_KeyboardEvent(vm, bmc64_keycode_to_plus4emu(key), 1);
}

// TODO - Common kbd.c will implement this once it is moved up. For now, call
// into VM here.
void emu_key_released(long key) {
   Plus4VM_KeyboardEvent(vm, bmc64_keycode_to_plus4emu(key), 0);
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
