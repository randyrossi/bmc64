#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "plus4lib/plus4emu.h"

static Plus4VM            *vm = NULL;
static Plus4VideoDecoder  *videoDecoder = NULL;

static const int        audioSampleRate = 44100;
static const int        audioBufferSize = 2048;
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
  // TBD
  printf ("Audio %d\n",(int)nFrames);
}

static void videoLineCallback(void *userData,
                              int lineNum, const Plus4VideoLineData *lineData)
{
  // TBD
  printf ("VideoLine\n");
}

static void videoFrameCallback(void *userData)
{
  printf ("Vync\n");
  // TBD - vsync
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

  printf ("Init\n");

  // VIDEO INIT

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

  videoDecoder =
      Plus4VideoDecoder_Create(&videoLineCallback, &videoFrameCallback, NULL);
  if (!videoDecoder)
    errorMessage("could not create video decoder object");
  Plus4VideoDecoder_UpdatePalette(videoDecoder, 0, 16, 8, 0);
  Plus4VM_SetVideoOutputCallback(vm, &Plus4VideoDecoder_VideoCallback,
                                 (void *) videoDecoder);

  // AUDIO init

  /* run Plus/4 emulation until the F12 key is pressed */
  printf ("Enter emulation loop\n");
  do {
    if (Plus4VM_Run(vm, 2000) != PLUS4EMU_SUCCESS)
      vmError();
  } while (!quitFlag);

  Plus4VM_Destroy(vm);
  Plus4VideoDecoder_Destroy(videoDecoder);
  return 0;
}

