/*
   plus4emu -- portable Commodore Plus/4 emulator
   Copyright (C) 2003-2016 Istvan Varga <istvanv@users.sourceforge.net>
   https://github.com/istvan-v/plus4emu/

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

#ifndef PLUS4EMU_PLUS4EMU_H
#define PLUS4EMU_PLUS4EMU_H

#ifndef BUILDING_PLUS4EMU_LIBRARY
#  include <stdlib.h>
#  if defined(HAVE_STDINT_H) || defined(__GNUC__)
#    include <stdint.h>
#  else
typedef signed char         int8_t;
typedef unsigned char       uint8_t;
typedef short               int16_t;
typedef unsigned short      uint16_t;
typedef int                 int32_t;
typedef unsigned int        uint32_t;
#    if defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER)
typedef __int64             int64_t;
typedef unsigned __int64    uint64_t;
#    else
typedef long long           int64_t;
typedef unsigned long long  uint64_t;
#    endif
#  endif
typedef struct Plus4VM_ Plus4VM;
#endif

#if defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER)
#  ifdef BUILDING_PLUS4EMU_LIBRARY
#    define PLUS4EMU_EXPORT __declspec(dllexport)
#  else
#    define PLUS4EMU_EXPORT __declspec(dllimport)
#  endif
#elif defined(__linux) || defined(__linux__)
#  define PLUS4EMU_EXPORT   __attribute__ ((__visibility__ ("default")))
#else
   // Added for BMC64
#  define PLUS4EMU_EXPORT   __attribute__ ((__visibility__ ("default")))
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  PLUS4EMU_SUCCESS = 0,
  PLUS4EMU_ERROR = -1,
  PLUS4EMU_BAD_ALLOC = -2
} Plus4Emu_Error;

 /* ======================================================================== */

/*!
 * Allocates and initializes a new Plus/4 emulator object, and returns an
 * opaque handle to be passed to the other interface functions.
 * On error, NULL is returned.
 */
PLUS4EMU_EXPORT Plus4VM * Plus4VM_Create(void);
/*!
 * Deletes the specified Plus/4 emulator object, and frees all associated
 * resources. If 'vm' is NULL, calling this function has no effect.
 */
PLUS4EMU_EXPORT void Plus4VM_Destroy(Plus4VM *vm);
/*!
 * Returns the most recent error message, or NULL if there was no error.
 * The error message is also automatically cleared to NULL by this function.
 */
PLUS4EMU_EXPORT const char * Plus4VM_GetLastErrorMessage(Plus4VM *vm);
/*!
 * Set the function to be called for playing audio output. The callback takes
 * three arguments: the void* userData parameter passed to this function, a
 * pointer to a buffer of mono 16-bit signed PCM audio data, and the number of
 * samples in the buffer.
 */
PLUS4EMU_EXPORT void Plus4VM_SetAudioOutputCallback(
    Plus4VM *vm, void (*func)(void *, const int16_t *, size_t), void *userData);
/*!
 * Set the function to be called for displaying video output. The callback
 * takes three arguments: the void* userData parameter passed to this function,
 * a pointer to a buffer of TED video data, and the data size in bytes.
 * The video data for each double clock cycle (4 TED pixels) is encoded as a
 * flags byte followed by either one or four Plus/4 colormap indices. The flags
 * byte can be the sum of any of the following values:
 *   128: composite sync
 *    64: vertical sync
 *    32: horizontal blanking
 *    16: vertical blanking
 *     8: burst
 *     4: PAL even line ($FF1D bit 0 in the previous line)
 *     2: number of data bytes: 0: 1 byte (all pixels are the same), 1: 4 bytes
 *     1: NTSC mode (dot clock multiplied by 1.25)
 */
PLUS4EMU_EXPORT void Plus4VM_SetVideoOutputCallback(
    Plus4VM *vm, void (*func)(void *, const uint8_t *, size_t), void *userData);
/*!
 * Run emulation for the specified number of microseconds.
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_Run(Plus4VM *vm, size_t microseconds);
/*!
 * Reset emulated machine; if 'forceReset' is non-zero, TED registers and
 * memory banking are also reset, and SID emulation and unused floppy drives
 * are disabled.
 */
PLUS4EMU_EXPORT void Plus4VM_Reset(Plus4VM *vm, int forceReset);
/*!
 * Delete all ROM segments, and resize RAM to 'memSize' kilobytes;
 * implies calling Plus4VM_Reset() with forceReset = 1.
 * RAM is cleared to a pattern defined by 'ramPattern':
 *   bits 0 to 2:   address line (0 to 7) for initial value of data bit 0
 *   bit 3:         invert bit 0
 *   bits 4 to 6:   address line (0 to 7) for initial value of data bit 1
 *   bit 7:         invert bit 1
 *   bits 8 to 10:  address line (0 to 7) for initial value of data bit 2
 *   bit 11:        invert bit 2
 *   bits 12 to 14: address line (0 to 7) for initial value of data bit 3
 *   bit 15:        invert bit 3
 *   bits 16 to 18: address line (0 to 7) for initial value of data bit 4
 *   bit 19:        invert bit 4
 *   bits 20 to 22: address line (0 to 7) for initial value of data bit 5
 *   bit 23:        invert bit 5
 *   bits 24 to 26: address line (0 to 7) for initial value of data bit 6
 *   bit 27:        invert bit 6
 *   bits 28 to 30: address line (0 to 7) for initial value of data bit 7
 *   bit 31:        invert bit 7
 *   bits 32 to 39: XOR value for bytes at the beginning of 256 byte pages
 *   bits 40 to 47: probability of random bytes (0: none, 255: maximum)
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SetRAMConfiguration(
    Plus4VM *vm, size_t memSize, uint64_t ramPattern);
/*!
 * Load ROM segment 'n' from the specified file, skipping 'offs' bytes.
 * The allowed values for 'n' are:
 *   0x00        Basic ROM
 *   0x01        Kernal ROM
 *   0x02, 0x03  Function (3-plus-1) ROM (2x16K)
 *   0x04, 0x05  Cartridge 1 (2x16K)
 *   0x06, 0x07  Cartridge 2 (2x16K)
 *   0x0A        MPS-801 printer ROM (4K)
 *   0x0C        1526/MPS-802 printer ROM (8K)
 *   0x10        1541 floppy drive ROM
 *   0x20        1551 floppy drive ROM
 *   0x30, 0x31  1581 floppy drive ROM (2x16K)
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_LoadROM(
    Plus4VM *vm, uint8_t n, const char *fileName, size_t offs);
/*!
 * Set audio output sample rate conversion quality (0: low, 1: high).
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SetAudioOutputQuality(
    Plus4VM *vm, int n);
/*!
 * Set audio output sample rate in Hz (11025.0 to 96000.0).
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SetAudioSampleRate(
    Plus4VM *vm, float sampleRate);
/*!
 * Write sound output to the specified file name, closing any previously opened
 * file with a different name. If the name is NULL or an empty string, no file
 * is written.
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SetAudioOutputFile(
    Plus4VM *vm, const char *fileName);
/*!
 * Set the cutoff frequencies of highpass filters used on the audio output to
 * remove DC offset.
 */
PLUS4EMU_EXPORT void Plus4VM_SetAudioOutputFilters(
    Plus4VM *vm, float dcBlockFreq1, float dcBlockFreq2);
/*!
 * Set parameters of the audio output equalizer.
 * 'mode' can be one of the following values:
 *  -1: disable equalizer
 *   0: peaking EQ
 *   1: low shelf
 *   2: high shelf
 */
PLUS4EMU_EXPORT void Plus4VM_SetAudioOutputEqualizer(
    Plus4VM *vm, int mode, float freq, float level, float q);
/*!
 * Set the amplitude scale for audio output (defaults to 0.7943).
 */
PLUS4EMU_EXPORT void Plus4VM_SetAudioOutputVolume(Plus4VM *vm, float ampScale);
/*!
 * Set if sound data is sent to the audio callback and output file (0: no).
 */
PLUS4EMU_EXPORT void Plus4VM_SetEnableAudioOutput(Plus4VM *vm, int isEnabled);
/*!
 * Set if video data is sent to the video callback (0: no).
 */
PLUS4EMU_EXPORT void Plus4VM_SetEnableDisplay(Plus4VM *vm, int isEnabled);
/*!
 * Set CPU clock frequency (in Hz), or clock multiplier if 'cpuFrequency' is
 * a small value (<= 100).
 */
PLUS4EMU_EXPORT void Plus4VM_SetCPUFrequency(Plus4VM *vm, size_t cpuFrequency);
/*!
 * Set the TED input clock frequency in Hz (PAL: 17734475, NTSC: 14318180).
 */
PLUS4EMU_EXPORT void Plus4VM_SetVideoClockFrequency(
    Plus4VM *vm, size_t videoFrequency);
/*!
 * Set if the 6551 ACIA should be emulated (0: no).
 */
PLUS4EMU_EXPORT void Plus4VM_SetEnableACIAEmulation(Plus4VM *vm, int isEnabled);
/*!
 * Set SID emulation parameters. 'outputVolume' should be specified in decibels
 * (-8 to +2). 'sidFlags' can be the sum of:
 *   1: SID model is 6581
 *   2: enable write access at $D400-$D41F
 *   4: run SID emulation at C64 clock frequency
 */
PLUS4EMU_EXPORT void Plus4VM_SetSIDConfiguration(
    Plus4VM *vm, int sidFlags, int enableDigiBlaster, int outputVolume);
/*!
 * SID emulation is automatically enabled by writing to any of the SID
 * registers, but this function can be used to enable it explicitly, or disable
 * the SID emulation to reduce CPU usage.
 */
PLUS4EMU_EXPORT void Plus4VM_SetEnableSIDEmulation(Plus4VM *vm, int isEnabled);
/*!
 * Set the state of key 'keyCode' (0 to 127) to pressed (1) or released (0).
 * See config/p4_keys.cfg for the list of valid key codes.
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_KeyboardEvent(
    Plus4VM *vm, int keyCode, int isPressed);
/*!
 * Set light pen position. 'xPos' and 'yPos' should be in the range 0 to 65535
 * for the visible 768x576 display area (including borders), or 5461 to 60074
 * (xPos) and 9330 to 54840 (yPos) for the 40x25 text area.
 * Values that are out of range turn off the light pen.
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SetLightPenPosition(
    Plus4VM *vm, int xPos, int yPos);
/*!
 * Set the cursor position. The expected range for 'xPos' and 'yPos' is the
 * same as in the case of Plus4VM_SetLightPenPosition(). Any position that is
 * not in the 40x25 text area is ignored.
 */
PLUS4EMU_EXPORT void Plus4VM_SetCursorPosition(Plus4VM *vm, int xPos, int yPos);
/*!
 * Copy text from the screen memory. 'xPos' and 'yPos' can be a small integer
 * (0 to 39, and 0 to 24, respectively) to copy from a specific line or the
 * word at 'xPos', a large integer (100 to 65535, see above) to copy from a
 * line or word at a physical screen position, or -1 to copy the line or word
 * at the current cursor position.
 * If 'yPos' is less than -1, the whole screen is copied as multiple lines, and
 * 'xPos' is ignored; otherwise, if 'xPos' is less than -1, then a (logical)
 * line, otherwise a word is copied.
 * 'buf' is expected to have enough space for 'bufSize' characters (including
 * the '\0' character at the end of the string); if the text to be copied is
 * longer, it is truncated.
 * The return value is the length of the copied text, not including the '\0'
 * character; if it is greater than, or equal to 'bufSize', then the string is
 * truncated. On error, a negative Plus4Emu_Error code is returned.
 */
PLUS4EMU_EXPORT long Plus4VM_CopyText(
    Plus4VM *vm, char *buf, size_t bufSize, int xPos, int yPos);
/*!
 * Paste the text from 's' to the keyboard buffer of the emulated machine,
 * optionally setting the cursor position to 'xPos' and 'yPos' (0 to 65535)
 * first if both are non-negative.
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_PasteText(
    Plus4VM *vm, const char *s, int xPos, int yPos);
/*!
 * Set the type of printer to be emulated:
 *   0: disable printer emulation (default)
 *   1: MPS-801 (IEC level emulation)
 *   2: 1526/MPS-802 (hardware level emulation)
 *   3: 1526/MPS-802 in 1525 mode (hardware level emulation)
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SetPrinterType(Plus4VM *vm, int n);
/*!
 * Returns the printer page width in pixels, or zero if printer emulation is
 * disabled.
 */
PLUS4EMU_EXPORT int Plus4VM_GetPrinterPageWidth(Plus4VM *vm);
/*!
 * Returns the printer page height in pixels, or zero if printer emulation is
 * disabled.
 */
PLUS4EMU_EXPORT int Plus4VM_GetPrinterPageHeight(Plus4VM *vm);
/*!
 * Get the current printer output as an 8-bit greyscale image. The data size is
 * PAGE_WIDTH * PAGE_HEIGHT bytes. If printer emulation is disabled, NULL is
 * returned.
 * NOTE: the buffer should not be freed or written to.
 */
PLUS4EMU_EXPORT const uint8_t * Plus4VM_GetPrinterOutput(Plus4VM *vm);
/*!
 * Clear the printer output buffer, and reset the head position to the top of
 * the page.
 */
PLUS4EMU_EXPORT void Plus4VM_ClearPrinterOutput(Plus4VM *vm);
/*!
 * Returns the current state of printer LEDs as a bitmap (if bit 0 is set, LED
 * 1 is on, if bit 1 is set, LED 2 is on, etc.).
 */
PLUS4EMU_EXPORT int Plus4VM_GetPrinterLEDState(Plus4VM *vm);
/*!
 * Returns the current horizontal position of the printer head, in the range 0
 * (left) to PAGE_WIDTH - 1 (right). The page width can be determined with
 * Plus4VM_GetPrinterPageWidth().
 * If printer emulation is not enabled, -1 is returned.
 */
PLUS4EMU_EXPORT int Plus4VM_GetPrinterHeadPositionX(Plus4VM *vm);
/*!
 * Returns the current vertical position of the printer head, in the range 0
 * (top) to PAGE_HEIGHT - 1 (bottom). The page height can be determined with
 * Plus4VM_GetPrinterPageHeight().
 * If printer emulation is not enabled, -1 is returned.
 */
PLUS4EMU_EXPORT int Plus4VM_GetPrinterHeadPositionY(Plus4VM *vm);
/*!
 * Returns non-zero if the printer output has changed since the last call of
 * Plus4VM_ClearPrinterChangedFlag().
 */
PLUS4EMU_EXPORT int Plus4VM_IsPrinterOutputChanged(Plus4VM *vm);
/*!
 * Clears the printer output change flag.
 */
PLUS4EMU_EXPORT void Plus4VM_ClearPrinterChangedFlag(Plus4VM *vm);
/*!
 * Set the state of the MPS-802 printer form feed button (0: off, 1: on).
 */
PLUS4EMU_EXPORT void Plus4VM_SetPrinterFormFeedOn(Plus4VM *vm, int n);
/*!
 * Write the printer output to 'fileName'. If 'asciiMode' is zero, then the
 * raw character data received on the serial bus is written, otherwise it is
 * converted to ASCII and written as a standard text file.
 * If 'fileName' is NULL or empty, then a previously opened file is closed.
 * NOTE: printer emulation must be enabled before calling this function.
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SetPrinterOutputFile(
    Plus4VM *vm, const char *fileName, int asciiMode);
/*!
 * Returns status information about the emulated machine (see also the comments
 * for functions that return individual status values).
 */
typedef struct Plus4VM_Status_ {
  int       isRecordingDemo;
  int       isPlayingDemo;
  double    tapePosition;
  double    tapeLength;
  int       tapeReadOnly;
  int       tapeSampleRate;
  int       tapeSampleSize;
  uint32_t  floppyDriveLEDState;
  uint64_t  floppyDriveHeadPositions;
  int       printerHeadPositionX;
  int       printerHeadPositionY;
  int       printerOutputChanged;
  int       printerLEDState;
} Plus4VM_Status;
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_GetStatus(
    Plus4VM *vm, Plus4VM_Status *vmStatus);
/*!
 * Create video capture object with the specified frame rate (24 to 60) and
 * format (384x288 RLE8 or 384x288 YV12) if it does not exist yet, and
 * optionally set callbacks for printing error messages and asking for a new
 * output file on reaching 2 GB file size.
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_OpenVideoCapture(
    Plus4VM *vm, int frameRate, int yuvFormat,
    void (*errorCallback)(void *userData_, const char *msg_),
    void (*fileNameCallback)(void *userData_, char *buf_, size_t bufSize_),
    void *userData);
/*!
 * Set output file name for video capture (an empty or NULL file name means no
 * file is written). Plus4VM_OpenVideoCapture() should be called first.
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SetVideoCaptureFile(
    Plus4VM *vm, const char *fileName);
/*!
 * Set if the TV standard for video capture should be PAL (0, default) or NTSC
 * (1).
 */
PLUS4EMU_EXPORT void Plus4VM_SetVideoCaptureNTSCMode(Plus4VM *vm, int ntscMode);
/*!
 * Destroy video capture object, freeing all allocated memory and closing the
 * output file.
 */
PLUS4EMU_EXPORT void Plus4VM_CloseVideoCapture(Plus4VM *vm);

 /* -------------------------- DISK AND FILE I/O --------------------------- */

/*!
 * Load disk image for drive 'n' (0 to 3 for unit 8 to 11); an empty or NULL
 * file name means no disk.
 * 'driveType' is the type of floppy drive to be emulated for D64 images: 0 is
 * 1541, and 1 is 1551 (allowed for n = 0 or 1 only).
 * If the type of a drive is set to 1551, and it is disabled (no image file),
 * then IEC level emulation is turned on, with access to the files in the
 * directory selected with Plus4VM_SetWorkingDirectory().
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SetDiskImageFile(
    Plus4VM *vm, int n, const char *fileName, int driveType);
/*!
 * Returns the current state of the floppy drive LEDs, which is the sum of the
 * LED colors multiplied by 2 ^ (DRIVE_NUMBER * 8) for each drive (0 to 3).
 * The possible color values are:
 *   0: black (off)
 *   1: red (1541)
 *   2: green (1551 / D64 image, hardware level)
 *   3: yellow-green (1581)
 *   4: blue
 *   5: magenta
 *   6: blue-cyan (1551 / filesystem access, IEC level)
 *   7: white
 */
PLUS4EMU_EXPORT uint32_t Plus4VM_GetFloppyDriveLEDState(Plus4VM *vm);
/*!
 * Returns the current head position for all floppy drives.
 * For each drive, the head position is encoded as a 16-bit value (bits 0 to 15
 * for drive 0, bits 16 to 31 for drive 1, etc.):
 *   bits 0 to 6:   sector number
 *   bit 7:         side selected
 *   bits 8 to 14:  track number
 *   bit 15:        0: 40 tracks, 1: 80 tracks
 * If a particular drive does not exist, or no disk image is set, 0xFFFF is
 * returned as head position.
 */
PLUS4EMU_EXPORT uint64_t Plus4VM_GetDriveHeadPositions(Plus4VM *vm);
/*!
 * Set if the floppy drive emulation should use higher timing resolution at the
 * expense of increased CPU usage (non-zero: yes). This is enabled by default,
 * and has no effect on drive types other than the 1541.
 */
PLUS4EMU_EXPORT void Plus4VM_SetFloppyDriveAccuracy(
    Plus4VM *vm, int isHighAccuracy);
/*!
 * Set the serial bus delay offset to 'n' (-100 to 100) nanoseconds for devices
 * that support this configuration option (currently only the 1541).
 */
PLUS4EMU_EXPORT void Plus4VM_SetSerialBusDelayOffset(Plus4VM *vm, int n);
/*!
 * Disable all floppy drives with no disk image attached to reduce CPU usage.
 */
PLUS4EMU_EXPORT void Plus4VM_DisableUnusedDrives(Plus4VM *vm);
/*!
 * Reset floppy drive 'n' (0 to 3), or all drives if 'n' is negative.
 */
PLUS4EMU_EXPORT void Plus4VM_ResetFloppyDrive(Plus4VM *vm, int n);
/*!
 * Set if the IEC drive emulation should be allowed to write to the working
 * directory (0: yes).
 */
PLUS4EMU_EXPORT void Plus4VM_SetIECDriveReadOnlyMode(
    Plus4VM *vm, int isReadOnly);
/*!
 * Set the working directory for IEC drive emulation.
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SetWorkingDirectory(
    Plus4VM *vm, const char *dirName);
/*!
 * Set function to be called when the emulated machine tries to open a file
 * with unspecified name. The file name should be written to 'buf_' (at most
 * 'bufSize_' bytes).
 */
PLUS4EMU_EXPORT void Plus4VM_SetFileNameCallback(
    Plus4VM *vm,
    void (*func)(void *userData_, char *buf_, size_t bufSize_), void *userData);

 /* ---------------------------- TAPE EMULATION ---------------------------- */

/*!
 * Set tape image file name (if the file name is NULL or empty, tape emulation
 * is disabled).
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SetTapeFileName(
    Plus4VM *vm, const char *fileName);
/*!
 * Set sample rate (in Hz) to be used when creating a new tape image file.
 * If the file already exists, this setting is ignored, and the value stored in
 * the file header is used instead.
 */
PLUS4EMU_EXPORT void Plus4VM_DefaultTapeSampleRate(Plus4VM *vm, int sampleRate);
/*!
 * Send tape input/output signals to the sound output. The absolute value of
 * 'n' (in the range 0 to 10) controls the output level, negative values invert
 * the signal. If 'n' is zero, tape feedback is disabled.
 */
PLUS4EMU_EXPORT void Plus4VM_SetTapeFeedbackLevel(Plus4VM *vm, int n);
/*!
 * Returns the actual sample rate of the tape file, or zero if there is no tape
 * image file opened.
 */
PLUS4EMU_EXPORT int Plus4VM_GetTapeSampleRate(Plus4VM *vm);
/*!
 * Returns the number of bits per sample in the tape file, or zero if there is
 * no tape image file opened.
 */
PLUS4EMU_EXPORT int Plus4VM_GetTapeSampleSize(Plus4VM *vm);
/*!
 * Returns non-zero if the tape is opened in read-only mode.
 */
PLUS4EMU_EXPORT int Plus4VM_GetIsTapeReadOnly(Plus4VM *vm);
/*!
 * Start tape playback.
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_TapePlay(Plus4VM *vm);
/*!
 * Start tape recording; if the tape file is read-only, this is equivalent to
 * calling Plus4VM_TapePlay().
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_TapeRecord(Plus4VM *vm);
/*!
 * Stop tape playback and recording.
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_TapeStop(Plus4VM *vm);
/*!
 * Set tape position to the specified time (in seconds).
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_TapeSeek(Plus4VM *vm, double t);
/*!
 * Returns the current tape position in seconds, or -1.0 if there is no tape
 * image file opened.
 */
PLUS4EMU_EXPORT double Plus4VM_GetTapePosition(Plus4VM *vm);
/*!
 * Returns the current length of the tape file in seconds, or -1.0 if there is
 * no tape image file opened.
 */
PLUS4EMU_EXPORT double Plus4VM_GetTapeLength(Plus4VM *vm);
/*!
 * Seek forward (if isForward is non-zero) or backward (if isForward is zero)
 * to the nearest cue point, or by 't' seconds if no cue point is found.
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_TapeSeekToCuePoint(
    Plus4VM *vm, int isForward, double t);
/*!
 * Create a new cue point at the current tape position. Has no effect if the
 * file does not have a cue point table, or it is read-only.
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_TapeAddCuePoint(Plus4VM *vm);
/*!
 * Delete the cue point nearest to the current tape position. Has no effect if
 * the file is read-only.
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_TapeDeleteCuePoint(Plus4VM *vm);
/*!
 * Delete all cue points. Has no effect if the file is read-only.
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_TapeDeleteAllCuePoints(Plus4VM *vm);
/*!
 * Set parameters for tape sound file I/O.
 */
PLUS4EMU_EXPORT void Plus4VM_SetTapeSoundFileParams(
    Plus4VM *vm, int requestedChannel, int invertSignal,
    int enableFilter, float filterMinFreq, float filterMaxFreq);

 /* ------------------------------ DEBUGGING ------------------------------- */

/*!
 * Set the debugging context (CPU number).
 *   0: main CPU
 *   1: floppy drive (unit 8)
 *   2: floppy drive (unit 9)
 *   3: floppy drive (unit 10)
 *   4: floppy drive (unit 11)
 *   5: printer
 */
PLUS4EMU_EXPORT void Plus4VM_SetDebugContext(Plus4VM *vm, int n);
/*!
 * Returns the current debugging context (CPU number).
 */
PLUS4EMU_EXPORT int Plus4VM_GetDebugContext(Plus4VM *vm);
/*!
 * Add new breakpoint with the specified type, address, and priority (0 to 3).
 * The allowed values for 'bpType' are:
 *   0: any memory access (read, write or CPU opcode read)
 *   1: memory read
 *   2: memory write
 *   3: any memory access, same as bpType == 0
 *   4: video (address bits 7..15 for Y, bits 0..6 for X)
 *   5: ignore other breakpoints if the program counter is at this address
 *   6: CPU opcode read
 * A negative priority value deletes any existing breakpoint at 'bpAddr'.
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_AddBreakPoint(
    Plus4VM *vm, int bpType, uint16_t bpAddr, int bpPriority);
/*!
 * Deletes all previously defined breakpoints.
 */
PLUS4EMU_EXPORT void Plus4VM_ClearBreakPoints(Plus4VM *vm);
/*!
 * Set breakpoint priority threshold (0 to 4); breakpoints with a priority less
 * than this value will not trigger a break.
 */
PLUS4EMU_EXPORT void Plus4VM_SetBPPriorityThreshold(Plus4VM *vm, int n);
/*!
 * (DEPRECATED) If 'n' is non-zero, read breakpoints will only be triggered on
 * opcode reads. NOTE: this function no longer has any effect, use execute
 * breakpoints instead.
 */
PLUS4EMU_EXPORT void Plus4VM_SetNoBreakOnDataRead(Plus4VM *vm, int n);
/*!
 * Set if the breakpoint callback should be called whenever the first byte of a
 * CPU instruction is read from memory. 'mode' can be one of the following
 * values:
 *   0: normal mode
 *   1: single step mode (break on every instruction, ignore breakpoints)
 *   2: step over mode (skip JSR or branch instruction)
 *   3: trace (similar to mode 1, but does not ignore breakpoints)
 *   4: step into mode (break at target of branch instruction)
 */
PLUS4EMU_EXPORT void Plus4VM_SetSingleStepMode(Plus4VM *vm, int mode);
/*!
 * Set the next address where single step mode will stop, ignoring any
 * other instructions. If 'addr' is negative, then a break is triggered
 * immediately at the next instruction.
 * Note: Plus4VM_SetSingleStepMode() must be called first with a mode
 * parameter of 2 or 4.
 */
PLUS4EMU_EXPORT void Plus4VM_SetSingleStepModeNextAddress(
    Plus4VM *vm, int32_t addr);
/*!
 * Set if invalid (jam) CPU opcodes should be interpreted as NOPs with a
 * breakpoint set (priority = 3).
 */
PLUS4EMU_EXPORT void Plus4VM_SetBreakOnInvalidOpcode(
    Plus4VM *vm, int isEnabled);
/*!
 * Set function to be called when a breakpoint is triggered.
 * 'type_' can be one of the following values:
 *   0: breakpoint at opcode read
 *   1: memory read
 *   2: memory write
 *   3: opcode read in single step mode
 *   4: video breakpoint ('addr_' is Y * 128 + X / 2)
 */
PLUS4EMU_EXPORT void Plus4VM_SetBreakPointCallback(
    Plus4VM *vm,
    void (*func)(void *userData_,
                 int debugContext_, int type_, uint16_t addr_, uint8_t value_),
    void *userData);
/*!
 * Returns the segment currently selected at 16K page 'n' (0 to 3). See the
 * debugger documentation in README for a complete list of possible segment
 * numbers.
 */
PLUS4EMU_EXPORT uint8_t Plus4VM_GetMemoryPage(Plus4VM *vm, int n);
/*!
 * Read a byte from memory. If 'isCPUAddress' is zero, 'addr' is interpreted as
 * a 22-bit address (see README), otherwise it is a 16-bit address in the
 * address space of the currently selected debug context (CPU).
 */
PLUS4EMU_EXPORT uint8_t Plus4VM_ReadMemory(
    Plus4VM *vm, uint32_t addr, int isCPUAddress);
/*!
 * Write a byte to memory. If 'isCPUAddress' is zero, 'addr' is interpreted as
 * a 22-bit address (see README), otherwise it is a 16-bit address in the
 * address space of the currently selected debug context (CPU).
 * NOTE: calling this function will stop any demo recording or playback.
 */
PLUS4EMU_EXPORT void Plus4VM_WriteMemory(
    Plus4VM *vm, uint32_t addr, uint8_t value, int isCPUAddress);
/*!
 * Returns the current value of the CPU program counter (PC).
 */
PLUS4EMU_EXPORT uint16_t Plus4VM_GetProgramCounter(Plus4VM *vm);
/*!
 * Returns the CPU address of the last byte pushed to the stack ( = SP + 1).
 */
PLUS4EMU_EXPORT uint16_t Plus4VM_GetStackPointer(Plus4VM *vm);
/*!
 * Dumps the current values of all CPU registers to 'buf' in ASCII format.
 * The register list may be written as multiple lines separated by '\n'
 * characters, however, there is no newline character at the end of the buffer.
 * 'buf' should have enough space for at least 64 bytes.
 */
PLUS4EMU_EXPORT void Plus4VM_DumpCPURegisters(Plus4VM *vm, char *buf);
/*!
 * Disassemble one CPU instruction, starting from memory address 'addr', and
 * write the result to 'buf' (not including a newline character). 'offs' is
 * added to the instruction address that is printed.
 * Returns the address of the next instruction.
 * 'buf' should have enough space for at least 64 bytes.
 */
PLUS4EMU_EXPORT uint32_t Plus4VM_DisassembleInstruction(
    Plus4VM *vm, char *buf, uint32_t addr, int isCPUAddress, int32_t offs);
/*!
 * Returns the current horizontal video position (0 to 455).
 */
PLUS4EMU_EXPORT int Plus4VM_GetVideoPositionX(Plus4VM *vm);
/*!
 * Returns the current vertical video position (PAL: 0 to 311, NTSC: 0 to 261).
 */
PLUS4EMU_EXPORT int Plus4VM_GetVideoPositionY(Plus4VM *vm);
/*!
 * Returns the current horizontal (0 to 455) and vertical (PAL: 0 to 311, NTSC:
 * 0 to 261) video position.
 */
PLUS4EMU_EXPORT void Plus4VM_GetVideoPosition(
    Plus4VM *vm, int *xPos, int *yPos);
/*!
 * Set the registers of the currently selected CPU.
 * NOTES:
 *   - changing the program counter only takes effect after completing the
 *     current instruction
 *   - calling this function will stop any demo recording or playback
 */
typedef struct Plus4_CPURegisters_ {
  uint16_t  reg_PC;
  uint8_t   reg_SR;
  uint8_t   reg_AC;
  uint8_t   reg_XR;
  uint8_t   reg_YR;
  uint8_t   reg_SP;
} Plus4_CPURegisters;
PLUS4EMU_EXPORT void Plus4VM_SetCPURegisters(
    Plus4VM *vm, const Plus4_CPURegisters *r);
/*!
 * Get the registers of the currently selected CPU.
 */
PLUS4EMU_EXPORT void Plus4VM_GetCPURegisters(
    Plus4VM *vm, Plus4_CPURegisters *r);

 /* ---------------------- SNAPSHOT AND DEMO FILE I/O ---------------------- */

/*!
 * Save snapshot of virtual machine state, including all ROM and RAM segments,
 * as well as all hardware registers. Note that the clock frequency and timing
 * settings, tape and disk state, and breakpoint list are not saved.
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SaveState(
    Plus4VM *vm, const char *fileName);
/*!
 * Load a plus4emu snapshot or demo file.
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_LoadState(
    Plus4VM *vm, const char *fileName);
/*!
 * Save program in PRG format.
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SaveProgram(
    Plus4VM *vm, const char *fileName);
/*!
 * Load a PRG or P00 format program.
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_LoadProgram(
    Plus4VM *vm, const char *fileName);
/*!
 * Save a snapshot with clock frequency and timing settings, and start demo
 * recording to memory. The output file is written only when the recording is
 * stopped.
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_RecordDemo(
    Plus4VM *vm, const char *fileName);
/*!
 * Stop playing or recording demo, writing the data to the output file in the
 * case of recording.
 */
PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_StopDemo(Plus4VM *vm);
/*!
 * Returns non-zero if a demo is currently being recorded. The recording stops
 * when Plus4VM_StopDemo() is called, any tape or disk I/O is attempted, clock
 * frequency and timing settings are changed, or a snapshot is loaded.
 * This function will also flush demo data to the associated output file after
 * recording is stopped for some reason other than calling Plus4VM_StopDemo().
 * A negative Plus4Emu_Error code may be returned if there is an error writing
 * the output file.
 */
PLUS4EMU_EXPORT int Plus4VM_GetIsRecordingDemo(Plus4VM *vm);
/*!
 * Returns non-zero if a demo is currently being played. The playback stops
 * when the end of the demo is reached, Plus4VM_StopDemo() is called, any tape
 * or disk I/O is attempted, clock frequency and timing settings are changed,
 * or a snapshot is loaded. Note that keyboard events are ignored while playing
 * a demo.
 */
PLUS4EMU_EXPORT int Plus4VM_GetIsPlayingDemo(Plus4VM *vm);

 /* ======================================================================== */

/*!
 * Converts Plus/4 color 'c' (0 to 255) to YUV.
 */
PLUS4EMU_EXPORT void Plus4_ColorToYUV(
    int c, int isNTSC, float *y, float *u, float *v);
/*!
 * Converts Plus/4 color 'c' (0 to 255) to RGB (0 to 1).
 */
PLUS4EMU_EXPORT void Plus4_ColorToRGB(
    int c, int isNTSC, float *r, float *g, float *b);

typedef struct Plus4VideoLineData_  Plus4VideoLineData;

/*!
 * Allocate a new line data structure. These can be used with Plus4VideoDecoder
 * (see below) to implement caching of video data, so that if a line does not
 * change, it does not need to be decoded and drawn again.
 * On error, NULL is returned.
 */
PLUS4EMU_EXPORT Plus4VideoLineData * Plus4VideoLineData_Create(void);
/*!
 * Free the memory used by a line data structure (has no effect if 'l' is
 * NULL).
 */
PLUS4EMU_EXPORT void Plus4VideoLineData_Destroy(Plus4VideoLineData *l);
/*!
 * Compare two line data structures. The return value is zero if the lines are
 * identical.
 */
PLUS4EMU_EXPORT int Plus4VideoLineData_Compare(
    const Plus4VideoLineData *l1, const Plus4VideoLineData *l2);
/*!
 * Copy the line data structure pointed to by 'src' to 'dst'.
 */
PLUS4EMU_EXPORT void Plus4VideoLineData_Copy(
    Plus4VideoLineData *dst, const Plus4VideoLineData *src);

#ifndef BUILDING_PLUS4EMU_LIBRARY
typedef struct Plus4VideoDecoder_   Plus4VideoDecoder;
#endif

/*!
 * Allocate a new video decoder object, and return an opaque handle to be
 * passed to the other functions.
 * 'lineCallback' (must be non-NULL) is called at the end of each line, with
 * the line number and a pointer to a read-only line data structure passed as
 * arguments. The line number is incremented by two at each scanline, and is
 * normally an even value, but it can also be odd in the case of an interlaced
 * video mode; the range is 0 to 575 for a standard PAL display, however, the
 * callback is called for all lines (including blanking), so line numbers
 * outside this range will also be passed.
 * 'frameCallback' (must be non-NULL) is called after the last line of each
 * frame.
 * 'userData' is passed as the first argument of both the line and the frame
 * callback.
 * On error, NULL is returned.
 */
PLUS4EMU_EXPORT Plus4VideoDecoder * Plus4VideoDecoder_Create(
    void (*lineCallback)(void *userData_,
                         int lineNum_, const Plus4VideoLineData *lineData_),
    void (*frameCallback)(void *userData_),
    void *userData);
/*!
 * Delete a video decoder object, freeing all associated resources. If 'vd' is
 * NULL, calling this function has no effect.
 */
PLUS4EMU_EXPORT void Plus4VideoDecoder_Destroy(Plus4VideoDecoder *vd);
/*!
 * This function can be used as the video output callback of a Plus4VM object,
 * with a pointer to a Plus4VideoDecoder object as the user data; in most
 * cases, it is not called directly.
 */
PLUS4EMU_EXPORT void Plus4VideoDecoder_VideoCallback(
    void *userData, const uint8_t *buf, size_t nBytes);
/*!
 * Set if the video decoder should emulate a PAL (0, default) or an NTSC (1)
 * display. Plus4VideoDecoder_UpdatePalette() should be called to update the
 * colormap for the new setting.
 */
PLUS4EMU_EXPORT void Plus4VideoDecoder_SetNTSCMode(
    Plus4VideoDecoder *vd, int isNTSCMode);
/*!
 * Set the display brightness (default: 0.0). 'a' is applied to all RGB
 * channels, while 'r', 'g', and 'b' can be used to set the brightness
 * independently for each channel. Changing this setting will only take effect
 * when Plus4VideoDecoder_UpdatePalette() is called.
 */
PLUS4EMU_EXPORT void Plus4VideoDecoder_SetBrightness(
    Plus4VideoDecoder *vd, double a, double r, double g, double b);
/*!
 * Set the display contrast (default: 1.0). 'a' is applied to all RGB channels,
 * while 'r', 'g', and 'b' can be used to set the contrast independently for
 * each channel. Changing this setting will only take effect when
 * Plus4VideoDecoder_UpdatePalette() is called.
 */
PLUS4EMU_EXPORT void Plus4VideoDecoder_SetContrast(
    Plus4VideoDecoder *vd, double a, double r, double g, double b);
/*!
 * Set the display gamma correction (default: 1.0; higher values mean brighter
 * display). 'a' is applied to all RGB channels, while 'r', 'g', and 'b' can be
 * used to set the gamma independently for each channel. Changing this setting
 * will only take effect when Plus4VideoDecoder_UpdatePalette() is called.
 */
PLUS4EMU_EXPORT void Plus4VideoDecoder_SetGamma(
    Plus4VideoDecoder *vd, double a, double r, double g, double b);
/*!
 * Set the display color saturation (default: 1.0). Changing this setting will
 * only take effect when Plus4VideoDecoder_UpdatePalette() is called.
 */
PLUS4EMU_EXPORT void Plus4VideoDecoder_SetSaturation(
    Plus4VideoDecoder *vd, double colorSaturation);
/*!
 * Set the display hue shift in degrees (-180.0 to 180.0; default: 0.0).
 * Changing this setting will only take effect when
 * Plus4VideoDecoder_UpdatePalette() is called.
 */
PLUS4EMU_EXPORT void Plus4VideoDecoder_SetHueShift(
    Plus4VideoDecoder *vd, double hueShift);
/*!
 * Recalculate the colormap based on the previously set color correction
 * parameters and television standard. 'yuvMode' can be 0 for RGB colorspace,
 * or 1 for YUV (Y instead of red, U instead of green, and V instead of blue;
 * all channels are scaled to the range 0 to 255). The 'rShift', 'gShift', and
 * 'bShift' values determine the number of bits by which the 8-bit red/Y,
 * green/U, and blue/V channels are shifted to the left in a 32-bit unsigned
 * integer pixel format.
 * The default settings are: yuvMode = 0, rShift = 16, gShift = 8, bShift = 0.
 */
PLUS4EMU_EXPORT void Plus4VideoDecoder_UpdatePalette(
    Plus4VideoDecoder *vd);
/*!
 * Decode the line in 'lineData' to 'outBuf'.
 * 'lineWidth' is the horizontal resolution to which the 48 characters wide
 * display is rescaled; it can be any positive value, however, scaling by
 * integer ratios (line width = 384, 768, 1152, or 1536) is faster and produces
 * better quality.
 * 'pixelFormat' can be either 0 for a 32-bit pixel format ('outBuf' should be
 * aligned to 4 bytes, and have enough space for 4 * lineWidth bytes) where
 * pixels are written as 32-bit unsigned integers with native endianness,
 * or 1 for a 24-bit packed format ('outBuf' should have enough space for
 * 3 * lineWidth bytes) where the 24 least significant bits of a pixel are
 * written as 3 bytes in MSB-first order (so, with the default channel bit
 * shifts, it would be interleaved R,G,B,R,G,B,... or Y,U,V,Y,U,V,... bytes).
 */
//PLUS4EMU_EXPORT void Plus4VideoDecoder_DecodeLine(
//    Plus4VideoDecoder *vd, uint8_t *outBuf, int lineWidth, int pixelFormat,
//    const Plus4VideoLineData *lineData);
PLUS4EMU_EXPORT void Plus4VideoDecoder_DecodeLine(
    Plus4VideoDecoder *vd, uint8_t *outBuf, int lineWidth,
    const Plus4VideoLineData *lineData);

PLUS4EMU_EXPORT void Plus4VideoDecoder_GetPaletteColor(
    int i, int *ri, int *gi, int *bi);

#ifdef __cplusplus
}       /* extern "C" */
#endif

#endif  /* PLUS4EMU_PLUS4EMU_H */

