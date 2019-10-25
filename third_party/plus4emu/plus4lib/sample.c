/*
   sample.c -- simple example program for the plus4emu C interface library
   Copyright (C) 2008-2016 Istvan Varga <istvanv@users.sourceforge.net>
   https://github.com/istvan-v/plus4emu/

   To compile this program on Linux with GCC, use the following command:
     gcc -Wall -O2 -I. -L. sample.c -o sample -lSDL -lplus4emu -lm
   On Win32 with GCC/MinGW:
     gcc -Wall -O2 -I. -L. sample.c -o sample.exe -lSDL -lplus4emu -lm  \
         -lmingw32 -lSDLmain

   To compile with MSVC on Win32, an import library needs to be created:
     lib /machine:i386 /def:plus4emu.def
   This command will create an import library for MinGW:
     dlltool --input-def plus4emu.def --output-lib libplus4emu.a

   When running the example program, the ROM images 'p4_basic.rom' and
   'p4kernal.rom' should be in the current directory. The following keys can
   be used to control the emulation:
     F9:  save snapshot to ./snapshot.dat
     F10: load snapshot from ./snapshot.dat
     F11: reset (+Ctrl: force reset, +Shift: hard reset)
     F12: quit sample program
   IEC level drive emulation is enabled for unit 8 with read-write access to
   the current directory.

   -------------------------------------------------------------------------

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <SDL/SDL.h>
#include "plus4emu.h"

static Plus4VM          *vm = NULL;

static SDL_Surface      *sdlSurface = NULL;
static Plus4VideoDecoder  *videoDecoder = NULL;

static const int        audioSampleRate = 44100;
static const int        audioBufferSize = 2048;
static SDL_mutex        *audioMutex = NULL;
static SDL_sem          *audioSemaphore = NULL;
static int16_t          audioBuffer[2048];
static int              audioBufferReadPos = 0;
static int              audioBufferWritePos = 0;
static int              audioBufferSamplesUsed = 0;

static void errorMessage(const char *fmt, ...)
{
  va_list args;
  fprintf(stderr, " *** Plus/4 error: ");
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
  Plus4VM_Destroy(vm);
  SDL_Quit();
  exit(-1);
}

static void vmError(void)
{
  fprintf(stderr, " *** Plus/4 error: %s\n", Plus4VM_GetLastErrorMessage(vm));
  Plus4VM_Destroy(vm);
  SDL_Quit();
  exit(-1);
}

static void sdlAudioCallback(void *userData, Uint8 *buf, int n)
{
  int     i, bufferFull;
  (void) userData;
  n = n / sizeof(int16_t);
  SDL_mutexP(audioMutex);
  bufferFull = (audioBufferSamplesUsed >= audioBufferSize);
  for (i = 0; i < n; i++) {
    if (audioBufferSamplesUsed > 0) {
      ((int16_t *) buf)[i] = audioBuffer[audioBufferReadPos];
      if (++audioBufferReadPos >= audioBufferSize)
        audioBufferReadPos = 0;
      audioBufferSamplesUsed--;
    }
    else {
      /* buffer underrun */
      ((int16_t *) buf)[i] = 0;
    }
  }
  SDL_mutexV(audioMutex);
  if (bufferFull)
    SDL_SemPost(audioSemaphore);
}

static void audioOutputCallback(void *userData,
                                const int16_t *buf, size_t nFrames)
{
  size_t  i;
  (void) userData;
  SDL_mutexP(audioMutex);
  for (i = 0; i < nFrames; i++) {
    while (audioBufferSamplesUsed >= audioBufferSize) {
      /* buffer is full, need to wait */
      SDL_mutexV(audioMutex);
      SDL_SemWait(audioSemaphore);
      SDL_mutexP(audioMutex);
    }
    audioBuffer[audioBufferWritePos] = buf[i];
    if (++audioBufferWritePos >= audioBufferSize)
      audioBufferWritePos = 0;
    audioBufferSamplesUsed++;
  }
  SDL_mutexV(audioMutex);
}

static void videoLineCallback(void *userData,
                              int lineNum, const Plus4VideoLineData *lineData)
{
  (void) userData;
  if (lineNum >= -1 && lineNum < 576) {
    if (SDL_MUSTLOCK(sdlSurface))
      SDL_LockSurface(sdlSurface);
    Plus4VideoDecoder_DecodeLine(videoDecoder,
                                 ((uint8_t *) sdlSurface->pixels)
                                 + ((lineNum >= 0 ? lineNum : 0)
                                    * sdlSurface->pitch),
                                 768, 0, lineData);
    if (lineNum >= 0 && lineNum < 575) {        /* double scan */
      memcpy(((uint8_t *) sdlSurface->pixels)
             + ((lineNum + 1) * sdlSurface->pitch),
             ((uint8_t *) sdlSurface->pixels) + (lineNum * sdlSurface->pitch),
             768 * 4);
    }
    if (SDL_MUSTLOCK(sdlSurface))
      SDL_UnlockSurface(sdlSurface);
  }
}

static void videoFrameCallback(void *userData)
{
  (void) userData;
  SDL_Flip(sdlSurface);
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
}

int main(int argc, char **argv)
{
  int     quitFlag = 0;

  (void) argc;
  (void) argv;
  if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) != 0)
    errorMessage("could not initialize SDL");

  vm = Plus4VM_Create();
  if (!vm)
    errorMessage("could not create Plus/4 emulator object");
  Plus4VM_SetAudioOutputCallback(vm, &audioOutputCallback, NULL);
  if (Plus4VM_SetAudioOutputQuality(vm, 1) != PLUS4EMU_SUCCESS)
    vmError();
  if (Plus4VM_SetAudioSampleRate(vm, audioSampleRate) != PLUS4EMU_SUCCESS)
    vmError();
  resetMemoryConfiguration();
  if (Plus4VM_SetWorkingDirectory(vm, ".") != PLUS4EMU_SUCCESS)
    vmError();
  /* enable read-write IEC level drive emulation for unit 8 */
  Plus4VM_SetIECDriveReadOnlyMode(vm, 0);
  if (Plus4VM_SetDiskImageFile(vm, 0, "", 1) != PLUS4EMU_SUCCESS)
    vmError();
  Plus4VM_Reset(vm, 1);

  /* initialize SDL video */
  SDL_WM_SetCaption("Plus/4 emulator", "");
  sdlSurface = SDL_SetVideoMode(768, 576, 32, SDL_SWSURFACE);
  if (!sdlSurface)
    errorMessage("cannot set video mode");
  videoDecoder =
      Plus4VideoDecoder_Create(&videoLineCallback, &videoFrameCallback, NULL);
  if (!videoDecoder)
    errorMessage("could not create video decoder object");
  Plus4VideoDecoder_UpdatePalette(videoDecoder, 0, 16, 8, 0);
  Plus4VM_SetVideoOutputCallback(vm, &Plus4VideoDecoder_VideoCallback,
                                 (void *) videoDecoder);

  /* initialize SDL audio */
  {
    SDL_AudioSpec audioSpec;
    audioMutex = SDL_CreateMutex();
    if (audioMutex == NULL)
      errorMessage("failed to create audio mutex");
    audioSemaphore = SDL_CreateSemaphore(0);
    if (audioSemaphore == NULL)
      errorMessage("failed to create audio semaphore");
    memset(&audioSpec, 0, sizeof(SDL_AudioSpec));
    audioSpec.freq = audioSampleRate;
    audioSpec.format = AUDIO_S16SYS;
    audioSpec.channels = 1;
    audioSpec.silence = 0;
    audioSpec.samples = audioBufferSize;
    audioSpec.size = audioSpec.samples * sizeof(int16_t);
    audioSpec.callback = &sdlAudioCallback;
    audioSpec.userdata = NULL;
    if (SDL_OpenAudio(&audioSpec, NULL) != 0)
      errorMessage("cannot open SDL audio output");
    SDL_PauseAudio(0);
  }

  /* run Plus/4 emulation until the F12 key is pressed */
  do {
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) {
      /* check for keyboard events */
      if (sdlEvent.type == SDL_KEYDOWN || sdlEvent.type == SDL_KEYUP) {
        int     keyCode = -1;
        switch (sdlEvent.key.keysym.sym) {
        case SDLK_BACKSPACE:
          keyCode = 0x00;
          break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
          keyCode = 0x01;
          break;
        case SDLK_HOME:
          keyCode = 0x39;
          break;
        case SDLK_ESCAPE:
        case SDLK_BACKQUOTE:
          keyCode = 0x34;
          break;
        case SDLK_SPACE:
          keyCode = 0x3C;
          break;
        case SDLK_KP_MULTIPLY:
        case SDLK_BACKSLASH:
          keyCode = 0x31;
          break;
        case SDLK_LEFTBRACKET:
        case SDLK_KP_PLUS:
          keyCode = 0x36;
          break;
        case SDLK_COMMA:
          keyCode = 0x2F;
          break;
        case SDLK_RIGHTBRACKET:
        case SDLK_KP_MINUS:
          keyCode = 0x2E;
          break;
        case SDLK_PERIOD:
        case SDLK_KP_PERIOD:
          keyCode = 0x2C;
          break;
        case SDLK_SLASH:
        case SDLK_KP_DIVIDE:
          keyCode = 0x37;
          break;
        case SDLK_0:
        case SDLK_KP0:
          keyCode = 0x23;
          break;
        case SDLK_1:
        case SDLK_KP1:
          keyCode = 0x38;
          break;
        case SDLK_2:
        case SDLK_KP2:
          keyCode = 0x3B;
          break;
        case SDLK_3:
        case SDLK_KP3:
          keyCode = 0x08;
          break;
        case SDLK_4:
        case SDLK_KP4:
          keyCode = 0x0B;
          break;
        case SDLK_5:
        case SDLK_KP5:
          keyCode = 0x10;
          break;
        case SDLK_6:
        case SDLK_KP6:
          keyCode = 0x13;
          break;
        case SDLK_7:
        case SDLK_KP7:
          keyCode = 0x18;
          break;
        case SDLK_8:
        case SDLK_KP8:
          keyCode = 0x1B;
          break;
        case SDLK_9:
        case SDLK_KP9:
          keyCode = 0x20;
          break;
        case SDLK_SEMICOLON:
          keyCode = 0x2D;
          break;
        case SDLK_QUOTE:
          keyCode = 0x32;
          break;
        case SDLK_EQUALS:
          keyCode = 0x35;
          break;
        case SDLK_MINUS:
          keyCode = 0x07;
          break;
        case SDLK_a:
          keyCode = 0x0A;
          break;
        case SDLK_b:
          keyCode = 0x1C;
          break;
        case SDLK_c:
          keyCode = 0x14;
          break;
        case SDLK_d:
          keyCode = 0x12;
          break;
        case SDLK_e:
          keyCode = 0x0E;
          break;
        case SDLK_f:
          keyCode = 0x15;
          break;
        case SDLK_g:
          keyCode = 0x1A;
          break;
        case SDLK_h:
          keyCode = 0x1D;
          break;
        case SDLK_i:
          keyCode = 0x21;
          break;
        case SDLK_j:
          keyCode = 0x22;
          break;
        case SDLK_k:
          keyCode = 0x25;
          break;
        case SDLK_l:
          keyCode = 0x2A;
          break;
        case SDLK_m:
          keyCode = 0x24;
          break;
        case SDLK_n:
          keyCode = 0x27;
          break;
        case SDLK_o:
          keyCode = 0x26;
          break;
        case SDLK_p:
          keyCode = 0x29;
          break;
        case SDLK_q:
          keyCode = 0x3E;
          break;
        case SDLK_r:
          keyCode = 0x11;
          break;
        case SDLK_s:
          keyCode = 0x0D;
          break;
        case SDLK_t:
          keyCode = 0x16;
          break;
        case SDLK_u:
          keyCode = 0x1E;
          break;
        case SDLK_v:
          keyCode = 0x1F;
          break;
        case SDLK_w:
          keyCode = 0x09;
          break;
        case SDLK_x:
          keyCode = 0x17;
          break;
        case SDLK_y:
          keyCode = 0x19;
          break;
        case SDLK_z:
          keyCode = 0x0C;
          break;
        case SDLK_DELETE:
          keyCode = 0x02;
          break;
        case SDLK_LEFT:
          keyCode = 0x30;
          break;
        case SDLK_RIGHT:
          keyCode = 0x33;
          break;
        case SDLK_UP:
          keyCode = 0x2B;
          break;
        case SDLK_DOWN:
          keyCode = 0x28;
          break;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
          keyCode = 0x0F;
          break;
        case SDLK_LCTRL:
          keyCode = 0x3D;
          break;
        case SDLK_RCTRL:
          keyCode = 0x3A;
          break;
        case SDLK_TAB:
          keyCode = 0x3F;
          break;
        case SDLK_F1:
          keyCode = 0x04;
          break;
        case SDLK_F2:
          keyCode = 0x05;
          break;
        case SDLK_F3:
          keyCode = 0x06;
          break;
        case SDLK_F4:
          keyCode = 0x03;
          break;
        case SDLK_F9:
          if (sdlEvent.type == SDL_KEYDOWN) {
            if (Plus4VM_SaveState(vm, "snapshot.dat") != PLUS4EMU_SUCCESS)
              errorMessage("cannot write snapshot file");
          }
          break;
        case SDLK_F10:
          if (sdlEvent.type == SDL_KEYDOWN)
            Plus4VM_LoadState(vm, "snapshot.dat");
          break;
        case SDLK_F11:
          if (sdlEvent.type == SDL_KEYDOWN) {
            if (sdlEvent.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT))
              resetMemoryConfiguration();
            else if (sdlEvent.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL))
              Plus4VM_Reset(vm, 1);
            else
              Plus4VM_Reset(vm, 0);
          }
          break;
        case SDLK_F12:
          if (sdlEvent.type == SDL_KEYDOWN)
            quitFlag = 1;
          break;
        default:
          break;
        }
        if (keyCode >= 0)
          Plus4VM_KeyboardEvent(vm, keyCode, (sdlEvent.type == SDL_KEYDOWN));
      }
    }
    if (Plus4VM_Run(vm, 2000) != PLUS4EMU_SUCCESS)
      vmError();
  } while (!quitFlag);

  SDL_PauseAudio(1);
  SDL_SemPost(audioSemaphore);
  SDL_CloseAudio();
  SDL_DestroySemaphore(audioSemaphore);
  SDL_DestroyMutex(audioMutex);
  Plus4VM_Destroy(vm);
  Plus4VideoDecoder_Destroy(videoDecoder);
  SDL_Quit();
  return 0;
}

