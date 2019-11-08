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

#include "plus4emu.hpp"
#include "fileio.hpp"
#include "display.hpp"
#include "soundio.hpp"
#include "bplist.hpp"
#include "cpu.hpp"
#include "ted.hpp"
#include "vm.hpp"
#include "plus4vm.hpp"

#include <typeinfo>

static void defaultAudioOutputCallback(void *userData,
                                       const int16_t *buf, size_t nFrames)
{
  (void) userData;
  (void) buf;
  (void) nFrames;
}

class AudioOutput_ : public Plus4Emu::AudioOutput {
 private:
  void    (*audioOutputCallback)(void *userData,
                                 const int16_t *buf, size_t nFrames);
  void    *audioOutputCallbackUserData;
 public:
  AudioOutput_();
  virtual ~AudioOutput_();
  // write 'nFrames' mono samples from 'buf' (in 16 bit signed PCM format)
  // to the audio output device and file
  virtual void sendAudioData(const int16_t *buf, size_t nFrames);
  virtual void setAudioOutputCallback(void (*func)(void *userData,
                                                   const int16_t *buf,
                                                   size_t nFrames),
                                      void *userData_);
};

AudioOutput_::AudioOutput_()
  : Plus4Emu::AudioOutput(),
    audioOutputCallback(&defaultAudioOutputCallback),
    audioOutputCallbackUserData((void *) 0)
{
}

AudioOutput_::~AudioOutput_()
{
}

void AudioOutput_::sendAudioData(const int16_t *buf, size_t nFrames)
{
  audioOutputCallback(audioOutputCallbackUserData, buf, nFrames);
}

void AudioOutput_::setAudioOutputCallback(void (*func)(void *userData,
                                                       const int16_t *buf,
                                                       size_t nFrames),
                                          void *userData_)
{
  if (func != (void (*)(void *, const int16_t *, size_t)) 0) {
    audioOutputCallback = func;
    audioOutputCallbackUserData = userData_;
  }
  else {
    audioOutputCallback = &defaultAudioOutputCallback;
    audioOutputCallbackUserData = (void *) 0;
  }
}

// ----------------------------------------------------------------------------

static void defaultVideoOutputCallback(void *userData,
                                       const uint8_t *buf, size_t nBytes)
{
  (void) userData;
  (void) buf;
  (void) nBytes;
}

class VideoDisplay_ : public Plus4Emu::VideoDisplay {
 private:
  void    (*videoOutputCallback)(void *userData,
                                 const uint8_t *buf, size_t nBytes);
  void    *videoOutputCallbackUserData;
  Plus4Emu::VideoDisplay::DisplayParameters displayParameters;
 public:
  VideoDisplay_();
  virtual ~VideoDisplay_();
  virtual void
      setDisplayParameters(const Plus4Emu::VideoDisplay::DisplayParameters& dp);
  virtual const Plus4Emu::VideoDisplay::DisplayParameters&
      getDisplayParameters() const;
  virtual void sendVideoOutput(const uint8_t *buf, size_t nBytes);
  virtual void setVideoOutputCallback(void (*func)(void *userData,
                                                   const uint8_t *buf,
                                                   size_t nBytes),
                                      void *userData_);
};

VideoDisplay_::VideoDisplay_()
  : Plus4Emu::VideoDisplay(),
    videoOutputCallback(&defaultVideoOutputCallback),
    videoOutputCallbackUserData((void *) 0),
    displayParameters()
{
}

VideoDisplay_::~VideoDisplay_()
{
}

void VideoDisplay_::setDisplayParameters(
    const Plus4Emu::VideoDisplay::DisplayParameters& dp)
{
  displayParameters = dp;
}

const Plus4Emu::VideoDisplay::DisplayParameters&
    VideoDisplay_::getDisplayParameters() const
{
  return displayParameters;
}

void VideoDisplay_::sendVideoOutput(const uint8_t *buf, size_t nBytes)
{
  videoOutputCallback(videoOutputCallbackUserData, buf, nBytes);
}

void VideoDisplay_::setVideoOutputCallback(void (*func)(void *userData,
                                                        const uint8_t *buf,
                                                        size_t nBytes),
                                           void *userData_)
{
  if (func != (void (*)(void *, const uint8_t *, size_t)) 0) {
    videoOutputCallback = func;
    videoOutputCallbackUserData = userData_;
  }
  else {
    videoOutputCallback = &defaultVideoOutputCallback;
    videoOutputCallbackUserData = (void *) 0;
  }
}

// ----------------------------------------------------------------------------

class Plus4VM_ {
 private:
  AudioOutput_    *audioOutput;
  VideoDisplay_   *videoDisplay;
  Plus4::Plus4VM  *vm;
  const char      *lastErrorMessage;
 public:
  std::string     strBuf;
  std::string     demoFileName;
  Plus4Emu::File  *demoFile;
  void            (*videoCaptureFileNameCallback)(void *, char *, size_t);
  void            (*videoCaptureErrorCallback)(void *, const char *);
  void            *videoCaptureCallbackUserData;
  void            (*fileNameCallback)(void *, char *, size_t);
  void            *fileNameCallbackUserData;
  // --------
  Plus4VM_();
  virtual ~Plus4VM_();
  void setLastErrorMessage(const char *s);
  const char *getLastErrorMessage() const;
  inline Plus4::Plus4VM& getVM()
  {
    return (*vm);
  }
  inline AudioOutput_& getAudioOutput()
  {
    return (*audioOutput);
  }
  inline VideoDisplay_& getVideoDisplay()
  {
    return (*videoDisplay);
  }
};

Plus4VM_::Plus4VM_()
  : audioOutput((AudioOutput_ *) 0),
    videoDisplay((VideoDisplay_ *) 0),
    vm((Plus4::Plus4VM *) 0),
    lastErrorMessage((char *) 0),
    strBuf(""),
    demoFileName(""),
    demoFile((Plus4Emu::File *) 0),
    videoCaptureFileNameCallback((void (*)(void *, char *, size_t)) 0),
    videoCaptureErrorCallback((void (*)(void *, const char *)) 0),
    videoCaptureCallbackUserData((void *) 0),
    fileNameCallback((void (*)(void *, char *, size_t)) 0),
    fileNameCallbackUserData((void *) 0)
{
  try {
    audioOutput = new AudioOutput_();
    videoDisplay = new VideoDisplay_();
    vm = new Plus4::Plus4VM(*videoDisplay, *audioOutput);
    strBuf.reserve(64);
  }
  catch (...) {
    if (vm)
      delete vm;
    if (videoDisplay)
      delete videoDisplay;
    if (audioOutput)
      delete audioOutput;
    throw;
  }
}

Plus4VM_::~Plus4VM_()
{
  if (demoFile) {
    try {
      // FIXME: errors are ignored here
      vm->stopDemo();
      demoFile->writeFile(demoFileName.c_str());
    }
    catch (...) {
    }
    delete demoFile;
  }
  delete vm;
  delete videoDisplay;
  delete audioOutput;
}

void Plus4VM_::setLastErrorMessage(const char *s)
{
  lastErrorMessage = s;
}

const char * Plus4VM_::getLastErrorMessage() const
{
  return lastErrorMessage;
}

// ----------------------------------------------------------------------------

typedef class Plus4VM_ Plus4VM;
typedef class Plus4VideoDecoder_ Plus4VideoDecoder;

#define BUILDING_PLUS4EMU_LIBRARY   1
#include "plus4emu.h"

static void videoCaptureFileNameCallback_(void *userData, std::string& fileName)
{
  Plus4VM_& vm = *(reinterpret_cast<Plus4VM_ *>(userData));
  try {
    if (vm.videoCaptureFileNameCallback) {
      char    tmpBuf[2048];
      tmpBuf[0] = '\0';
      vm.videoCaptureFileNameCallback(vm.videoCaptureCallbackUserData,
                                      &(tmpBuf[0]), 2047);
      tmpBuf[2047] = '\0';
      fileName = &(tmpBuf[0]);
    }
    else {
      fileName.clear();
    }
  }
  catch (...) {
    fileName.clear();
  }
}

static void videoCaptureErrorCallback_(void *userData, const char *s)
{
  Plus4VM_& vm = *(reinterpret_cast<Plus4VM_ *>(userData));
  if (!s)
    s = "";
  if (vm.videoCaptureErrorCallback)
    vm.videoCaptureErrorCallback(vm.videoCaptureCallbackUserData, s);
  else
    std::fprintf(stderr, " *** Plus/4 video capture error: %s\n", s);
}

static void fileNameCallback_(void *userData, std::string& fileName)
{
  Plus4VM_& vm = *(reinterpret_cast<Plus4VM_ *>(userData));
  try {
    if (vm.fileNameCallback) {
      char    tmpBuf[2048];
      tmpBuf[0] = '\0';
      vm.fileNameCallback(vm.fileNameCallbackUserData, &(tmpBuf[0]), 2047);
      tmpBuf[2047] = '\0';
      fileName = &(tmpBuf[0]);
    }
    else {
      fileName.clear();
    }
  }
  catch (...) {
    fileName.clear();
  }
}

extern "C" PLUS4EMU_EXPORT Plus4VM * Plus4VM_Create(void)
{
  Plus4VM_  *p = (Plus4VM_ *) 0;
  try {
    p = new Plus4VM_();
  }
  catch (...) {
    p = (Plus4VM_ *) 0;
  }
  return p;
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_Destroy(Plus4VM *vm)
{
  if (vm)
    delete vm;
}

extern "C" PLUS4EMU_EXPORT const char * Plus4VM_GetLastErrorMessage(Plus4VM *vm)
{
  const char  *s = vm->getLastErrorMessage();
  vm->setLastErrorMessage((char *) 0);
  return s;
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetAudioOutputCallback(
    Plus4VM *vm, void (*func)(void *, const int16_t *, size_t), void *userData)
{
  vm->getAudioOutput().setAudioOutputCallback(func, userData);
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetVideoOutputCallback(
    Plus4VM *vm, void (*func)(void *, const uint8_t *, size_t), void *userData)
{
  vm->getVideoDisplay().setVideoOutputCallback(func, userData);
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_Run(Plus4VM *vm,
                                                      size_t microseconds)
{
  try {
    vm->getVM().run(microseconds);
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_Reset(Plus4VM *vm, int isColdReset)
{
  try {
    vm->getVM().reset(bool(isColdReset));
  }
  catch (...) {
  }
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SetRAMConfiguration(
    Plus4VM *vm, size_t memSize, uint64_t ramPattern)
{
  try {
    vm->getVM().resetMemoryConfiguration(memSize, ramPattern);
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_LoadROM(
    Plus4VM *vm, uint8_t n, const char *fileName, size_t offs)
{
  try {
    vm->getVM().loadROMSegment(n, fileName, offs);
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SetAudioOutputQuality(
    Plus4VM *vm, int n)
{
  try {
    vm->getVM().setAudioOutputHighQuality(bool(n));
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SetAudioSampleRate(
    Plus4VM *vm, float sampleRate)
{
  try {
    vm->getAudioOutput().setParameters(-1, sampleRate);
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SetAudioOutputFile(
    Plus4VM *vm, const char *fileName)
{
  try {
    if (!fileName)
      fileName = "";
    std::string s(fileName);
    vm->getAudioOutput().setOutputFile(s);
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetAudioOutputFilters(
    Plus4VM *vm, float dcBlockFreq1, float dcBlockFreq2)
{
  vm->getVM().setAudioOutputFilters(dcBlockFreq1, dcBlockFreq2);
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetAudioOutputEqualizer(
    Plus4VM *vm, int mode, float freq, float level, float q)
{
  vm->getVM().setAudioOutputEqualizer(mode, freq, level, q);
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetAudioOutputVolume(
    Plus4VM *vm, float ampScale)
{
  vm->getVM().setAudioOutputVolume(ampScale);
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetEnableAudioOutput(
    Plus4VM *vm, int isEnabled)
{
  vm->getVM().setEnableAudioOutput(bool(isEnabled));
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetEnableDisplay(
    Plus4VM *vm, int isEnabled)
{
  vm->getVM().setEnableDisplay(bool(isEnabled));
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetCPUFrequency(
    Plus4VM *vm, size_t cpuFrequency)
{
  vm->getVM().setCPUFrequency(cpuFrequency);
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetVideoClockFrequency(
    Plus4VM *vm, size_t videoFrequency)
{
  vm->getVM().setVideoFrequency(videoFrequency);
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetEnableACIAEmulation(
    Plus4VM *vm, int isEnabled)
{
  vm->getVM().setEnableACIAEmulation(bool(isEnabled));
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetSIDConfiguration(
    Plus4VM *vm, int sidFlags, int enableDigiBlaster, int outputVolume)
{
  vm->getVM().setSIDConfiguration(uint8_t(sidFlags), bool(enableDigiBlaster),
                                  outputVolume);
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetEnableSIDEmulation(
    Plus4VM *vm, int isEnabled)
{
  if (isEnabled != 0)
    vm->getVM().writeMemory(0x0010FD5FU, 0x00, false);
  else
    vm->getVM().disableSIDEmulation();
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_KeyboardEvent(
    Plus4VM *vm, int keyCode, int isPressed)
{
  try {
    vm->getVM().setKeyboardState(keyCode, bool(isPressed));
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SetLightPenPosition(
    Plus4VM *vm, int xPos, int yPos)
{
  try {
    vm->getVM().setLightPenPosition(xPos, yPos);
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetCursorPosition(
    Plus4VM *vm, int xPos, int yPos)
{
  vm->getVM().setCursorPosition(xPos, yPos);
}

extern "C" PLUS4EMU_EXPORT long Plus4VM_CopyText(
    Plus4VM *vm, char *buf, size_t bufSize, int xPos, int yPos)
{
  size_t  len = 0;
  try {
    std::string s = vm->getVM().copyText(xPos, yPos);
    len = s.length();
    size_t  i = 0;
    while (i < len && (i + 1) < bufSize) {
      buf[i] = s[i];
      i++;
    }
    if (i < bufSize)
      buf[i] = '\0';
  }
  catch (std::exception& e) {
    if (bufSize > 0)
      buf[0] = '\0';
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return long(PLUS4EMU_BAD_ALLOC);
    return long(PLUS4EMU_ERROR);
  }
  return long(len);
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_PasteText(
    Plus4VM *vm, const char *s, int xPos, int yPos)
{
  try {
    vm->getVM().pasteText(s, xPos, yPos);
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SetPrinterType(
    Plus4VM *vm, int n)
{
  try {
    vm->getVM().setPrinterType(n);
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT int Plus4VM_GetPrinterPageWidth(Plus4VM *vm)
{
  int     w = 0;
  int     h = 0;
  const uint8_t *buf = (uint8_t *) 0;
  vm->getVM().getPrinterOutput(buf, w, h);
  return w;
}

extern "C" PLUS4EMU_EXPORT int Plus4VM_GetPrinterPageHeight(Plus4VM *vm)
{
  int     w = 0;
  int     h = 0;
  const uint8_t *buf = (uint8_t *) 0;
  vm->getVM().getPrinterOutput(buf, w, h);
  return h;
}

extern "C" PLUS4EMU_EXPORT const uint8_t * Plus4VM_GetPrinterOutput(Plus4VM *vm)
{
  int     w = 0;
  int     h = 0;
  const uint8_t *buf = (uint8_t *) 0;
  vm->getVM().getPrinterOutput(buf, w, h);
  return buf;
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_ClearPrinterOutput(Plus4VM *vm)
{
  vm->getVM().clearPrinterOutput();
}

extern "C" PLUS4EMU_EXPORT int Plus4VM_GetPrinterLEDState(Plus4VM *vm)
{
  return int(vm->getVM().getPrinterLEDState());
}

extern "C" PLUS4EMU_EXPORT int Plus4VM_GetPrinterHeadPositionX(Plus4VM *vm)
{
  int     x = 0;
  int     y = 0;
  vm->getVM().getPrinterHeadPosition(x, y);
  return x;
}

extern "C" PLUS4EMU_EXPORT int Plus4VM_GetPrinterHeadPositionY(Plus4VM *vm)
{
  int     x = 0;
  int     y = 0;
  vm->getVM().getPrinterHeadPosition(x, y);
  return y;
}

extern "C" PLUS4EMU_EXPORT int Plus4VM_IsPrinterOutputChanged(Plus4VM *vm)
{
  return int(vm->getVM().getIsPrinterOutputChanged());
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_ClearPrinterChangedFlag(Plus4VM *vm)
{
  vm->getVM().clearPrinterOutputChangedFlag();
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetPrinterFormFeedOn(Plus4VM *vm, int n)
{
  vm->getVM().setPrinterFormFeedOn(bool(n));
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SetPrinterOutputFile(
    Plus4VM *vm, const char *fileName, int asciiMode)
{
  try {
    vm->getVM().setPrinterTextOutputFile(fileName, bool(asciiMode));
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_GetStatus(
    Plus4VM *vm, Plus4VM_Status *vmStatus)
{
  try {
    Plus4Emu::VirtualMachine::VMStatus  tmp;
    vm->getVM().getVMStatus(tmp);
    vmStatus->isRecordingDemo = int(tmp.isRecordingDemo);
    vmStatus->isPlayingDemo = int(tmp.isPlayingDemo);
    vmStatus->tapeReadOnly = int(tmp.tapeReadOnly);
    vmStatus->tapePosition = tmp.tapePosition;
    vmStatus->tapeLength = tmp.tapeLength;
    vmStatus->tapeSampleRate = int(tmp.tapeSampleRate);
    vmStatus->tapeSampleSize = tmp.tapeSampleSize;
    vmStatus->floppyDriveLEDState = tmp.floppyDriveLEDState;
    vmStatus->floppyDriveHeadPositions = tmp.floppyDriveHeadPositions;
    vmStatus->printerHeadPositionX = tmp.printerHeadPositionX;
    vmStatus->printerHeadPositionY = tmp.printerHeadPositionY;
    vmStatus->printerOutputChanged = int(tmp.printerOutputChanged);
    vmStatus->printerLEDState = int(tmp.printerLEDState);
    if (vm->demoFile != (Plus4Emu::File *) 0 && !tmp.isRecordingDemo) {
      vm->demoFile->writeFile(vm->demoFileName.c_str());
      delete vm->demoFile;
      vm->demoFile = (Plus4Emu::File *) 0;
      vm->demoFileName.clear();
    }
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (vm->demoFile) {
      delete vm->demoFile;
      vm->demoFile = (Plus4Emu::File *) 0;
      vm->demoFileName.clear();
    }
    // NOTE: this assumes that Plus4VM::getVMStatus() fails only once
    Plus4VM_GetStatus(vm, vmStatus);
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_OpenVideoCapture(
    Plus4VM *vm, int frameRate, int yuvFormat,
    void (*errorCallback)(void *userData_, const char *msg_),
    void (*fileNameCallback)(void *userData_, char *buf_, size_t bufSize_),
    void *userData)
{
  try {
    vm->videoCaptureFileNameCallback = fileNameCallback;
    vm->videoCaptureErrorCallback = errorCallback;
    vm->videoCaptureCallbackUserData = userData;
    vm->getVM().openVideoCapture(frameRate, bool(yuvFormat),
                                 &videoCaptureErrorCallback_,
                                 &videoCaptureFileNameCallback_,
                                 (void *) vm);
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SetVideoCaptureFile(
    Plus4VM *vm, const char *fileName)
{
  try {
    if (!fileName)
      fileName = "";
    std::string s(fileName);
    vm->getVM().setVideoCaptureFile(s);
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetVideoCaptureNTSCMode(
    Plus4VM *vm, int ntscMode)
{
  vm->getVM().setVideoCaptureNTSCMode(bool(ntscMode));
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_CloseVideoCapture(Plus4VM *vm)
{
  vm->getVM().closeVideoCapture();
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SetDiskImageFile(
    Plus4VM *vm, int n, const char *fileName, int driveType)
{
  try {
    if (!fileName)
      fileName = "";
    std::string s(fileName);
    vm->getVM().setDiskImageFile(n, s, driveType);
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT uint32_t Plus4VM_GetFloppyDriveLEDState(Plus4VM *vm)
{
  return vm->getVM().getFloppyDriveLEDState();
}

extern "C" PLUS4EMU_EXPORT uint64_t Plus4VM_GetDriveHeadPositions(Plus4VM *vm)
{
  return vm->getVM().getFloppyDriveHeadPositions();
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetFloppyDriveAccuracy(
    Plus4VM *vm, int isHighAccuracy)
{
  vm->getVM().setFloppyDriveHighAccuracy(bool(isHighAccuracy));
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetSerialBusDelayOffset(
    Plus4VM *vm, int n)
{
  vm->getVM().setSerialBusDelayOffset(n);
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_DisableUnusedDrives(Plus4VM *vm)
{
  vm->getVM().disableUnusedFloppyDrives();
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_ResetFloppyDrive(Plus4VM *vm, int n)
{
  vm->getVM().resetFloppyDrive(n);
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetIECDriveReadOnlyMode(
    Plus4VM *vm, int isReadOnly)
{
  vm->getVM().setIECDriveReadOnlyMode(bool(isReadOnly));
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SetWorkingDirectory(
    Plus4VM *vm, const char *dirName)
{
  try {
    if (!dirName)
      dirName = "";
    std::string s(dirName);
    vm->getVM().setWorkingDirectory(s);
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetFileNameCallback(
    Plus4VM *vm,
    void (*func)(void *userData_, char *buf_, size_t bufSize_), void *userData)
{
  vm->fileNameCallback = func;
  vm->fileNameCallbackUserData = userData;
  vm->getVM().setFileNameCallback(&fileNameCallback_, (void *) vm);
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SetTapeFileName(
    Plus4VM *vm, const char *fileName)
{
  try {
    if (!fileName)
      fileName = "";
    std::string s(fileName);
    vm->getVM().setTapeFileName(s);
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_DefaultTapeSampleRate(
    Plus4VM *vm, int sampleRate)
{
  vm->getVM().setDefaultTapeSampleRate(sampleRate);
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetTapeFeedbackLevel(Plus4VM *vm, int n)
{
  vm->getVM().setTapeFeedbackLevel(n);
}

extern "C" PLUS4EMU_EXPORT int Plus4VM_GetTapeSampleRate(Plus4VM *vm)
{
  return int(vm->getVM().getTapeSampleRate());
}

extern "C" PLUS4EMU_EXPORT int Plus4VM_GetTapeSampleSize(Plus4VM *vm)
{
  return vm->getVM().getTapeSampleSize();
}

extern "C" PLUS4EMU_EXPORT int Plus4VM_GetIsTapeReadOnly(Plus4VM *vm)
{
  return int(vm->getVM().getIsTapeReadOnly());
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_TapePlay(Plus4VM *vm)
{
  try {
    vm->getVM().tapePlay();
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_TapeRecord(Plus4VM *vm)
{
  try {
    vm->getVM().tapeRecord();
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_TapeStop(Plus4VM *vm)
{
  try {
    vm->getVM().tapeStop();
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_TapeSeek(
    Plus4VM *vm, double t)
{
  try {
    vm->getVM().tapeSeek(t);
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT double Plus4VM_GetTapePosition(Plus4VM *vm)
{
  return vm->getVM().getTapePosition();
}

extern "C" PLUS4EMU_EXPORT double Plus4VM_GetTapeLength(Plus4VM *vm)
{
  return vm->getVM().getTapeLength();
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_TapeSeekToCuePoint(
    Plus4VM *vm, int isForward, double t)
{
  try {
    vm->getVM().tapeSeekToCuePoint(bool(isForward), t);
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_TapeAddCuePoint(Plus4VM *vm)
{
  try {
    vm->getVM().tapeAddCuePoint();
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_TapeDeleteCuePoint(
    Plus4VM *vm)
{
  try {
    vm->getVM().tapeDeleteNearestCuePoint();
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_TapeDeleteAllCuePoints(
    Plus4VM *vm)
{
  try {
    vm->getVM().tapeDeleteAllCuePoints();
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetTapeSoundFileParams(
    Plus4VM *vm, int requestedChannel, int invertSignal,
    int enableFilter, float filterMinFreq, float filterMaxFreq)
{
  vm->getVM().setTapeSoundFileParameters(requestedChannel, bool(invertSignal),
                                         bool(enableFilter),
                                         filterMinFreq, filterMaxFreq);
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetDebugContext(Plus4VM *vm, int n)
{
  vm->getVM().setDebugContext(n);
}

extern "C" PLUS4EMU_EXPORT int Plus4VM_GetDebugContext(Plus4VM *vm)
{
  return vm->getVM().getDebugContext();
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_AddBreakPoint(
    Plus4VM *vm, int bpType, uint16_t bpAddr, int bpPriority)
{
  try {
    vm->getVM().setBreakPoint(bpType, bpAddr, bpPriority);
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_ClearBreakPoints(Plus4VM *vm)
{
  vm->getVM().clearBreakPoints();
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetBPPriorityThreshold(
    Plus4VM *vm, int n)
{
  vm->getVM().setBreakPointPriorityThreshold(n);
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetNoBreakOnDataRead(Plus4VM *vm, int n)
{
  // deprecated function, has no effect
  (void) vm;
  (void) n;
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetSingleStepMode(Plus4VM *vm, int mode)
{
  vm->getVM().setSingleStepMode(mode);
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetSingleStepModeNextAddress(
    Plus4VM *vm, int32_t addr)
{
  vm->getVM().setSingleStepModeNextAddress(addr);
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetBreakOnInvalidOpcode(
    Plus4VM *vm, int isEnabled)
{
  vm->getVM().setBreakOnInvalidOpcode(bool(isEnabled));
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetBreakPointCallback(
    Plus4VM *vm,
    void (*func)(void *userData_,
                 int debugContext_, int type_, uint16_t addr_, uint8_t value_),
    void *userData)
{
  vm->getVM().setBreakPointCallback(func, userData);
}

extern "C" PLUS4EMU_EXPORT uint8_t Plus4VM_GetMemoryPage(Plus4VM *vm, int n)
{
  return vm->getVM().getMemoryPage(n);
}

extern "C" PLUS4EMU_EXPORT uint8_t Plus4VM_ReadMemory(
    Plus4VM *vm, uint32_t addr, int isCPUAddress)
{
  return vm->getVM().readMemory(addr, bool(isCPUAddress));
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_WriteMemory(
    Plus4VM *vm, uint32_t addr, uint8_t value, int isCPUAddress)
{
  vm->getVM().writeMemory(addr, value, bool(isCPUAddress));
}

extern "C" PLUS4EMU_EXPORT uint16_t Plus4VM_GetProgramCounter(Plus4VM *vm)
{
  return vm->getVM().getProgramCounter();
}

extern "C" PLUS4EMU_EXPORT uint16_t Plus4VM_GetStackPointer(Plus4VM *vm)
{
  return vm->getVM().getStackPointer();
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_DumpCPURegisters(Plus4VM *vm, char *buf)
{
  vm->getVM().listCPURegisters(vm->strBuf);
  size_t  i = 0;
  while (i < vm->strBuf.length() && i < 63) {
    buf[i] = vm->strBuf[i];
    i++;
  }
  buf[i] = '\0';
}

extern "C" PLUS4EMU_EXPORT uint32_t Plus4VM_DisassembleInstruction(
    Plus4VM *vm, char *buf, uint32_t addr, int isCPUAddress, int32_t offs)
{
  uint32_t  nxtAddr =
      vm->getVM().disassembleInstruction(vm->strBuf,
                                         addr, bool(isCPUAddress), offs);
  size_t  i = 0;
  while (i < vm->strBuf.length() && i < 63) {
    buf[i] = vm->strBuf[i];
    i++;
  }
  buf[i] = '\0';
  return nxtAddr;
}

extern "C" PLUS4EMU_EXPORT int Plus4VM_GetVideoPositionX(Plus4VM *vm)
{
  int     x = 0;
  int     y = 0;
  vm->getVM().getVideoPosition(x, y);
  return x;
}

extern "C" PLUS4EMU_EXPORT int Plus4VM_GetVideoPositionY(Plus4VM *vm)
{
  int     x = 0;
  int     y = 0;
  vm->getVM().getVideoPosition(x, y);
  return y;
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_GetVideoPosition(
    Plus4VM *vm, int *xPos, int *yPos)
{
  vm->getVM().getVideoPosition(*xPos, *yPos);
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_SetCPURegisters(
    Plus4VM *vm, const Plus4_CPURegisters *r)
{
  vm->getVM().setCPURegisters(
      *(reinterpret_cast<const Plus4::M7501Registers *>(r)));
}

extern "C" PLUS4EMU_EXPORT void Plus4VM_GetCPURegisters(
    Plus4VM *vm, Plus4_CPURegisters *r)
{
  vm->getVM().getCPURegisters(*(reinterpret_cast<Plus4::M7501Registers *>(r)));
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SaveState(
    Plus4VM *vm, const char *fileName)
{
  try {
    Plus4Emu::File  f;
    vm->getVM().saveState(f);
    f.writeFile(fileName);
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_LoadState(
    Plus4VM *vm, const char *fileName)
{
  try {
    Plus4Emu::File  f(fileName);
    vm->getVM().registerChunkTypes(f);
    f.processAllChunks();
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_SaveProgram(
    Plus4VM *vm, const char *fileName)
{
  try {
    vm->getVM().saveProgram(fileName);
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_LoadProgram(
    Plus4VM *vm, const char *fileName)
{
  try {
    vm->getVM().loadProgram(fileName);
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_RecordDemo(
    Plus4VM *vm, const char *fileName)
{
  try {
    if (vm->demoFile) {
      Plus4Emu_Error  err = Plus4VM_StopDemo(vm);
      if (err != PLUS4EMU_SUCCESS)
        return err;
    }
    if (fileName == (char *) 0 || fileName[0] == '\0')
      throw Plus4Emu::Exception("invalid demo file name");
    vm->demoFileName = fileName;
    vm->demoFile = new Plus4Emu::File();
    vm->getVM().recordDemo(*(vm->demoFile));
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (vm->demoFile) {
      delete vm->demoFile;
      vm->demoFile = (Plus4Emu::File *) 0;
      vm->demoFileName.clear();
    }
    if (typeid(e) == typeid(std::bad_alloc))
      return PLUS4EMU_BAD_ALLOC;
    return PLUS4EMU_ERROR;
  }
  return PLUS4EMU_SUCCESS;
}

extern "C" PLUS4EMU_EXPORT Plus4Emu_Error Plus4VM_StopDemo(Plus4VM *vm)
{
  Plus4Emu_Error  retval = PLUS4EMU_SUCCESS;
  try {
    vm->getVM().stopDemo();
    if (vm->demoFile)
      vm->demoFile->writeFile(vm->demoFileName.c_str());
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (typeid(e) == typeid(std::bad_alloc))
      retval = PLUS4EMU_BAD_ALLOC;
    retval = PLUS4EMU_ERROR;
  }
  if (vm->demoFile) {
    delete vm->demoFile;
    vm->demoFile = (Plus4Emu::File *) 0;
    vm->demoFileName.clear();
  }
  return retval;
}

extern "C" PLUS4EMU_EXPORT int Plus4VM_GetIsRecordingDemo(Plus4VM *vm)
{
  int     retval = 0;
  try {
    retval = int(vm->getVM().getIsRecordingDemo());
    if (vm->demoFile != (Plus4Emu::File *) 0 && retval == 0) {
      vm->demoFile->writeFile(vm->demoFileName.c_str());
      delete vm->demoFile;
      vm->demoFile = (Plus4Emu::File *) 0;
      vm->demoFileName.clear();
    }
  }
  catch (std::exception& e) {
    vm->setLastErrorMessage(e.what());
    if (vm->demoFile) {
      delete vm->demoFile;
      vm->demoFile = (Plus4Emu::File *) 0;
      vm->demoFileName.clear();
    }
    if (typeid(e) == typeid(std::bad_alloc))
      return int(PLUS4EMU_BAD_ALLOC);
    return int(PLUS4EMU_ERROR);
  }
  return retval;
}

extern "C" PLUS4EMU_EXPORT int Plus4VM_GetIsPlayingDemo(Plus4VM *vm)
{
  return int(vm->getVM().getIsPlayingDemo());
}

// ----------------------------------------------------------------------------

extern "C" PLUS4EMU_EXPORT void Plus4_ColorToYUV(
    int c, int isNTSC, float *y, float *u, float *v)
{
  Plus4::TED7360::convertPixelToYUV(uint8_t(c & 0x7F), bool(isNTSC),
                                    *y, *u, *v);
}

extern "C" PLUS4EMU_EXPORT void Plus4_ColorToRGB(
    int c, int isNTSC, float *r, float *g, float *b)
{
  float   y = 0.0f;
  float   u = 0.0f;
  float   v = 0.0f;
  Plus4::TED7360::convertPixelToYUV(uint8_t(c & 0x7F), bool(isNTSC), y, u, v);
  (*r) = y + (v * float(1.0 / 0.877));
  (*g) = y - (u * float(0.114 / (0.492 * 0.587)))
           - (v * float(0.299 / (0.877 * 0.587)));
  (*b) = y + (u * float(1.0 / 0.492));
  (*r) = ((*r) > 0.0f ? ((*r) < 1.0f ? (*r) : 1.0f) : 0.0f);
  (*g) = ((*g) > 0.0f ? ((*g) < 1.0f ? (*g) : 1.0f) : 0.0f);
  (*b) = ((*b) > 0.0f ? ((*b) < 1.0f ? (*b) : 1.0f) : 0.0f);
}

// ----------------------------------------------------------------------------

struct Plus4VideoLineData_ {
  size_t    nBytes;
  uint8_t   *buf;
  // bit 7: have burst signal
  // bit 1: invert color phase
  // bit 0: resample needed
  // other bits are undefined
  uint8_t   flags;
  size_t    lengthCnt;
  uint32_t  buf_[180];                  // 720 bytes, aligned to 4 bytes
  // --------
  Plus4VideoLineData_()
    : nBytes(0),
      buf(reinterpret_cast<uint8_t *>(&(buf_[0]))),
      flags(0x00),
      lengthCnt(0)
  {
    for (size_t i = 0; i < (sizeof(buf_) / sizeof(uint32_t)); i++)
      buf_[i] = 0U;
  }
  inline void clear()
  {
    nBytes = 0;
    flags = 0x00;
    lengthCnt = 0;
  }
  inline bool operator==(const Plus4VideoLineData_& r) const
  {
    if (nBytes != r.nBytes || flags != r.flags || lengthCnt != r.lengthCnt)
      return false;
    size_t  n = (nBytes + 3) >> 2;
    for (size_t i = 0; i < n; i++) {
      if (buf_[i] != r.buf_[i])
        return false;
    }
    return true;
  }
  inline bool operator!=(const Plus4VideoLineData_& r) const
  {
    return !((*this) == r);
  }
  inline void appendData(const uint8_t *srcBuf_, size_t nBytes_)
  {
    if (nBytes_ > 0) {
      std::memcpy(&(buf[nBytes]), srcBuf_, nBytes_);
      nBytes += nBytes_;
      for (size_t i = nBytes; (i & 3) != 0; i++)
        buf[i] = 0x00;
    }
  }
};

extern "C" PLUS4EMU_EXPORT Plus4VideoLineData * Plus4VideoLineData_Create(void)
{
  Plus4VideoLineData_ *l = (Plus4VideoLineData_ *) 0;
  try {
    l = new Plus4VideoLineData_();
  }
  catch (...) {
    l = (Plus4VideoLineData_ *) 0;
  }
  return l;
}

extern "C" PLUS4EMU_EXPORT void Plus4VideoLineData_Destroy(
    Plus4VideoLineData *l)
{
  if (l)
    delete l;
}

extern "C" PLUS4EMU_EXPORT int Plus4VideoLineData_Compare(
    const Plus4VideoLineData *l1, const Plus4VideoLineData *l2)
{
  return int((*l1) != (*l2));
}

extern "C" PLUS4EMU_EXPORT void Plus4VideoLineData_Copy(
    Plus4VideoLineData *dst, const Plus4VideoLineData *src)
{
  std::memcpy(dst, src,
              size_t((const unsigned char *) &(src->buf_[0])
                     - (const unsigned char *) src)
              + ((src->nBytes + 3) & (~(size_t(3)))));
  dst->buf = reinterpret_cast<uint8_t *>(&(dst->buf_[0]));
}

// ----------------------------------------------------------------------------

class Plus4VideoDecoder_ {
 public:
  int           curLine;
  int           vsyncCnt;
  bool          oddFrame;
  uint8_t       burstValue;
  unsigned int  syncLengthCnt;
  unsigned int  hsyncCnt;
  unsigned int  hsyncPeriodLength;
  unsigned int  lineLengthCnt;
  unsigned int  lineLength;
  unsigned int  lineStart;
  unsigned int  hsyncPeriodMin;
  unsigned int  hsyncPeriodMax;
  unsigned int  lineLengthMin;
  unsigned int  lineLengthMax;
  float         lineLengthFilter;
  int           vsyncThreshold1;
  int           vsyncThreshold2;
  int           vsyncReload;
  int           lineReload;
  void          (*lineCallback)(void *, int, const Plus4VideoLineData *);
  void          (*frameCallback)(void *);
  void          *callbackUserData;
  Plus4VideoLineData  lineBuffer;
  Plus4Emu::VideoDisplay::DisplayParameters displayParameters;
  Plus4Emu::VideoDisplayColormap<uint16_t>  colormap;
  // --------
  Plus4VideoDecoder_(
      void (*lineCallback_)(void *, int, const Plus4VideoLineData *),
      void (*frameCallback_)(void *),
      void *callbackUserData_);
  virtual ~Plus4VideoDecoder_();
  void lineDone();
};

Plus4VideoDecoder_::Plus4VideoDecoder_(
    void (*lineCallback_)(void *, int, const Plus4VideoLineData *),
    void (*frameCallback_)(void *),
    void *callbackUserData_)
  : curLine(0),
    vsyncCnt(0),
    oddFrame(false),
    burstValue(0x08),
    syncLengthCnt(0U),
    hsyncCnt(0U),
    hsyncPeriodLength(570U),
    lineLengthCnt(0U),
    lineLength(570U),
    lineStart(80U),
    hsyncPeriodMin(494U),
    hsyncPeriodMax(646U),
    lineLengthMin(513U),
    lineLengthMax(627U),
    lineLengthFilter(570.0f),
    vsyncThreshold1(335),
    vsyncThreshold2(261),
    vsyncReload(-19),
    lineReload(0),
    lineCallback(lineCallback_),
    frameCallback(frameCallback_),
    callbackUserData(callbackUserData_),
    lineBuffer(),
    displayParameters(),
    colormap()
{
  if (!lineCallback)
    throw Plus4Emu::Exception("video decoder line callback is NULL");
  if (!frameCallback)
    throw Plus4Emu::Exception("video decoder frame callback is NULL");
  // set default colormap (RGB colorspace)
  //Plus4VideoDecoder_UpdatePalette(this, 0, 16, 8, 0);

  // We're using RGB565, just init the color map as is, not rgb colorspace
  Plus4Emu::VideoDisplay::DisplayParameters dp(this->displayParameters);
  dp.indexToYUVFunc = &Plus4::TED7360::convertPixelToYUV;
  displayParameters = dp;
  colormap.setDisplayParameters(dp, false);
}

Plus4VideoDecoder_::~Plus4VideoDecoder_()
{
}

void Plus4VideoDecoder_::lineDone()
{
  lineLengthCnt = lineLengthCnt - lineLength;
  while (hsyncCnt >= hsyncPeriodMax) {
    hsyncCnt -= hsyncPeriodLength;
    hsyncPeriodLength = (hsyncPeriodLength * 3U + hsyncPeriodMax) >> 2;
  }
  lineLengthFilter =
      (lineLengthFilter * 0.9f) + (float(int(hsyncPeriodLength)) * 0.1f);
  lineLength = (unsigned int) (int(lineLengthFilter + 0.5f));
  if (lineLengthCnt != hsyncCnt) {
    int     hsyncPhaseError = int(lineLengthCnt) - int(hsyncCnt);
    if (hsyncPhaseError >= int(hsyncPeriodLength >> 1))
      hsyncPhaseError -= int(hsyncPeriodLength);
    if (hsyncPhaseError <= -(int(hsyncPeriodLength >> 1)))
      hsyncPhaseError += int(hsyncPeriodLength);
    unsigned int  tmp = (unsigned int) (hsyncPhaseError >= 0 ?
                                        hsyncPhaseError : (-hsyncPhaseError));
    tmp = (tmp + 6U) >> 2;
    tmp = (tmp < 10U ? tmp : 10U);
    if (hsyncPhaseError >= 0)
      lineLength += tmp;
    else
      lineLength -= tmp;
    if (lineLength > lineLengthMax)
      lineLength = lineLengthMax;
    else if (lineLength < lineLengthMin)
      lineLength = lineLengthMin;
  }
  lineBuffer.flags &= uint8_t(0x81);
  lineBuffer.flags |= uint8_t((~curLine) & 2);
  lineCallback(callbackUserData, curLine - 2, &lineBuffer);
  lineBuffer.clear();
  curLine += 2;
  if (vsyncCnt >= vsyncThreshold1) {
    vsyncCnt = vsyncReload;
    oddFrame = false;
  }
  if (vsyncCnt == 0) {
    curLine = lineReload - (!oddFrame ? 0 : 1);
    frameCallback(callbackUserData);
  }
  vsyncCnt++;
}

extern "C" PLUS4EMU_EXPORT Plus4VideoDecoder * Plus4VideoDecoder_Create(
    void (*lineCallback)(void *userData_,
                         int lineNum_, const Plus4VideoLineData *lineData_),
    void (*frameCallback)(void *userData_),
    void *userData)
{
  Plus4VideoDecoder_  *vd = (Plus4VideoDecoder_ *) 0;
  try {
    vd = new Plus4VideoDecoder_(lineCallback, frameCallback, userData);
  }
  catch (...) {
    vd = (Plus4VideoDecoder_ *) 0;
  }
  return vd;
}

extern "C" PLUS4EMU_EXPORT void Plus4VideoDecoder_Destroy(Plus4VideoDecoder *vd)
{
  if (vd)
    delete vd;
}

extern "C" PLUS4EMU_EXPORT void Plus4VideoDecoder_VideoCallback(
    void *userData, const uint8_t *buf, size_t nBytes)
{
  Plus4VideoDecoder_& vd = *(reinterpret_cast<Plus4VideoDecoder_ *>(userData));
  const uint8_t *bufp = buf;
  const uint8_t *startp = bufp;
  const uint8_t *endp = buf + nBytes;
  while (bufp < endp) {
    uint8_t   c = *bufp;
    if (c & 0x80) {                                     // sync
      if (vd.syncLengthCnt == 0U) {                     // hsync start
        while (vd.hsyncCnt >= vd.hsyncPeriodMax) {
          vd.hsyncCnt -= vd.hsyncPeriodLength;
          vd.hsyncPeriodLength =
              (vd.hsyncPeriodLength * 3U + vd.hsyncPeriodMax) >> 2;
        }
        if (vd.hsyncCnt >= vd.hsyncPeriodMin) {
          vd.hsyncPeriodLength = vd.hsyncCnt;
          vd.hsyncCnt = 0U;
        }
      }
      vd.syncLengthCnt++;
      if (vd.syncLengthCnt >= 26U) {                    // vsync
        if (vd.vsyncCnt >= vd.vsyncThreshold2) {
          vd.vsyncCnt = vd.vsyncReload;
          vd.oddFrame = ((vd.lineLengthCnt + 6U) > (vd.lineLength >> 1));
        }
      }
    }
    else
      vd.syncLengthCnt = 0U;
    vd.lineBuffer.flags |= uint8_t(0x80 - ((c ^ vd.burstValue) & 0x09));
    const uint8_t *nextBufPtr = bufp + size_t((1 << (c & 0x02)) + 1);
    unsigned int  l = ((unsigned int) c & 0x01U) ^ 0x05U;
    if (vd.lineLengthCnt < vd.lineStart) {
      startp = nextBufPtr;
      vd.lineBuffer.lengthCnt = vd.lineLengthCnt + l;
    }
    else if (vd.lineLengthCnt >= vd.lineLength) {
      vd.lineBuffer.lengthCnt =
          size_t(vd.lineLengthCnt) - vd.lineBuffer.lengthCnt;
      vd.lineBuffer.appendData(startp, size_t(bufp - startp));
      startp = nextBufPtr;
      vd.lineDone();
    }
    bufp = nextBufPtr;
    vd.lineLengthCnt = vd.lineLengthCnt + l;
    vd.hsyncCnt = vd.hsyncCnt + l;
  }
  vd.lineBuffer.appendData(startp, size_t(bufp - startp));
}

extern "C" PLUS4EMU_EXPORT void Plus4VideoDecoder_SetNTSCMode(
    Plus4VideoDecoder *vd, int isNTSCMode)
{
  if (bool(isNTSCMode) == vd->displayParameters.ntscMode)
    return;
  vd->lineBuffer.clear();
  vd->displayParameters.ntscMode = bool(isNTSCMode);
  if (!vd->displayParameters.ntscMode) {
    vd->burstValue = 0x08;
    vd->syncLengthCnt = 0U;
    vd->hsyncCnt = 0U;
    vd->hsyncPeriodLength = 570U;
    vd->lineLengthCnt = 0U;
    vd->lineLength = 570U;
    vd->lineStart = 80U;
    vd->hsyncPeriodMin = 494U;
    vd->hsyncPeriodMax = 646U;
    vd->lineLengthMin = 513U;
    vd->lineLengthMax = 627U;
    vd->lineLengthFilter = 570.0f;
    vd->vsyncThreshold1 = 335;
    vd->vsyncThreshold2 = 261;
    vd->vsyncReload = -19;
    vd->lineReload = 0;
  }
  else {
    vd->burstValue = 0x09;
    vd->syncLengthCnt = 0U;
    vd->hsyncCnt = 0U;
    vd->hsyncPeriodLength = 456U;
    vd->lineLengthCnt = 0U;
    vd->lineLength = 456U;
    vd->lineStart = 64U;
    vd->hsyncPeriodMin = 380U;
    vd->hsyncPeriodMax = 532U;
    vd->lineLengthMin = 399U;
    vd->lineLengthMax = 513U;
    vd->lineLengthFilter = 456.0f;
    vd->vsyncThreshold1 = 292;
    vd->vsyncThreshold2 = 242;
    vd->vsyncReload = 0;
    vd->lineReload = 12;
  }
}

extern "C" PLUS4EMU_EXPORT void Plus4VideoDecoder_SetBrightness(
    Plus4VideoDecoder *vd, double a, double r, double g, double b)
{
  vd->displayParameters.brightness = float(a);
  vd->displayParameters.redBrightness = float(r);
  vd->displayParameters.greenBrightness = float(g);
  vd->displayParameters.blueBrightness = float(b);
}

extern "C" PLUS4EMU_EXPORT void Plus4VideoDecoder_SetContrast(
    Plus4VideoDecoder *vd, double a, double r, double g, double b)
{
  vd->displayParameters.contrast = float(a);
  vd->displayParameters.redContrast = float(r);
  vd->displayParameters.greenContrast = float(g);
  vd->displayParameters.blueContrast = float(b);
}

extern "C" PLUS4EMU_EXPORT void Plus4VideoDecoder_SetGamma(
    Plus4VideoDecoder *vd, double a, double r, double g, double b)
{
  vd->displayParameters.gamma = float(a);
  vd->displayParameters.redGamma = float(r);
  vd->displayParameters.greenGamma = float(g);
  vd->displayParameters.blueGamma = float(b);
}

extern "C" PLUS4EMU_EXPORT void Plus4VideoDecoder_SetSaturation(
    Plus4VideoDecoder *vd, double colorSaturation)
{
  vd->displayParameters.saturation = float(colorSaturation);
}

extern "C" PLUS4EMU_EXPORT void Plus4VideoDecoder_SetHueShift(
    Plus4VideoDecoder *vd, double hueShift)
{
  vd->displayParameters.hueShift = float(hueShift);
}

extern "C" PLUS4EMU_EXPORT void Plus4VideoDecoder_UpdatePalette(
    Plus4VideoDecoder *vd)
{
  Plus4Emu::VideoDisplay::DisplayParameters dp(vd->displayParameters);
  dp.indexToYUVFunc = &Plus4::TED7360::convertPixelToYUV;
  vd->displayParameters = dp;
  vd->colormap.setDisplayParameters(dp, false);
}

/*
extern "C" PLUS4EMU_EXPORT void Plus4VideoDecoder_DecodeLine(
    Plus4VideoDecoder *vd, uint8_t *outBuf, int lineWidth, int pixelFormat,
    const Plus4VideoLineData *lineData)
{
  if (lineWidth < 1)
    return;
  size_t    pixelSample1p = 490 * size_t(lineWidth);
  size_t    pixelSample1n = 392 * size_t(lineWidth);
  const uint8_t *bufp = lineData->buf;
  uint8_t   videoFlags =
      uint8_t(((lineData->flags & 0x80) >> 2) | (lineData->flags & 0x02));
  size_t    pixelSample2 = lineData->lengthCnt * 384;
  if (vd->displayParameters.ntscMode)
    videoFlags = videoFlags | 0x10;
  size_t    pixelSample1 = ((bufp[0] & 0x01) ? pixelSample1n : pixelSample1p);
  uint32_t  tmpBuf[4];
  switch (pixelFormat) {
  case 0:                       // 32-bit pixel format, native endianness
    {
      // output buffer is assumed to be aligned to 4 bytes
      uint32_t  *p = reinterpret_cast<uint32_t *>(outBuf);
      uint32_t  *endPtr = p + lineWidth;
      if (pixelSample1 == pixelSample2 && !(lineData->flags & 0x01)) {
        // fast code for fixed 1x horizontal scale
        do {
          size_t  n = vd->colormap.convertFourPixels(p, bufp, videoFlags);
          bufp = bufp + n;
          p = p + 4;
        } while (p < endPtr);
      }
      else if (pixelSample1 == (pixelSample2 * 2) &&
               !(lineData->flags & 0x01)) {
        // fast code for fixed 2x horizontal scale
        do {
          size_t  n =
              vd->colormap.convertFourToEightPixels(p, bufp, videoFlags);
          bufp = bufp + n;
          p = p + 8;
        } while (p < endPtr);
      }
      else if (pixelSample1 == (pixelSample2 * 3) &&
               !(lineData->flags & 0x01)) {
        // fast code for fixed 3x horizontal scale
        do {
          size_t  n =
              vd->colormap.convertFourPixels(&(tmpBuf[0]), bufp, videoFlags);
          bufp = bufp + n;
          p[2] = p[1] = p[0] = tmpBuf[0];
          p[5] = p[4] = p[3] = tmpBuf[1];
          p[8] = p[7] = p[6] = tmpBuf[2];
          p[11] = p[10] = p[9] = tmpBuf[3];
          p = p + 12;
        } while (p < endPtr);
      }
      else if (pixelSample1 == (pixelSample2 * 4) &&
               !(lineData->flags & 0x01)) {
        // fast code for fixed 4x horizontal scale
        do {
          size_t  n =
              vd->colormap.convertFourPixels(&(tmpBuf[0]), bufp, videoFlags);
          bufp = bufp + n;
          p[3] = p[2] = p[1] = p[0] = tmpBuf[0];
          p[7] = p[6] = p[5] = p[4] = tmpBuf[1];
          p[11] = p[10] = p[9] = p[8] = tmpBuf[2];
          p[15] = p[14] = p[13] = p[12] = tmpBuf[3];
          p = p + 16;
        } while (p < endPtr);
      }
      else {
        // generic resample code (slow)
        size_t    pixelSampleCnt = 0;
        uint8_t   readPos = 4;
        do {
          if (readPos >= 4) {
            pixelSample1 = ((bufp[0] & 0x01) ? pixelSample1n : pixelSample1p);
            size_t  n =
                vd->colormap.convertFourPixels(&(tmpBuf[0]), bufp, videoFlags);
            bufp = bufp + n;
            readPos = readPos & 3;
          }
          *(p++) = tmpBuf[readPos];
          pixelSampleCnt += pixelSample2;
          while (pixelSampleCnt >= pixelSample1) {
            pixelSampleCnt -= pixelSample1;
            readPos++;
          }
        } while (p < endPtr);
      }
    }
    break;
  case 1:                       // 24-bit packed pixel format
    {
      uint8_t   *p = outBuf;
      uint8_t   *endPtr = p + (lineWidth * 3);
      if (pixelSample1 == pixelSample2 && !(lineData->flags & 0x01)) {
        // fast code for fixed 1x horizontal scale
        do {
          size_t  n =
              vd->colormap.convertFourPixels(&(tmpBuf[0]), bufp, videoFlags);
          bufp = bufp + n;
          for (int i = 0; i < 4; i++) {
            uint32_t  pixelValue = tmpBuf[i];
            p[0] = uint8_t((pixelValue >> 16) & 0xFFU);
            p[1] = uint8_t((pixelValue >> 8) & 0xFFU);
            p[2] = uint8_t(pixelValue & 0xFFU);
            p = p + 3;
          }
        } while (p < endPtr);
      }
      else if (pixelSample1 == (pixelSample2 * 2) &&
               !(lineData->flags & 0x01)) {
        // fast code for fixed 2x horizontal scale
        do {
          size_t  n =
              vd->colormap.convertFourPixels(&(tmpBuf[0]), bufp, videoFlags);
          bufp = bufp + n;
          for (int i = 0; i < 4; i++) {
            uint32_t  pixelValue = tmpBuf[i];
            p[3] = p[0] = uint8_t((pixelValue >> 16) & 0xFFU);
            p[4] = p[1] = uint8_t((pixelValue >> 8) & 0xFFU);
            p[5] = p[2] = uint8_t(pixelValue & 0xFFU);
            p = p + 6;
          }
        } while (p < endPtr);
      }
      else if (pixelSample1 == (pixelSample2 * 3) &&
               !(lineData->flags & 0x01)) {
        // fast code for fixed 3x horizontal scale
        do {
          size_t  n =
              vd->colormap.convertFourPixels(&(tmpBuf[0]), bufp, videoFlags);
          bufp = bufp + n;
          for (int i = 0; i < 4; i++) {
            uint32_t  pixelValue = tmpBuf[i];
            p[6] = p[3] = p[0] = uint8_t((pixelValue >> 16) & 0xFFU);
            p[7] = p[4] = p[1] = uint8_t((pixelValue >> 8) & 0xFFU);
            p[8] = p[5] = p[2] = uint8_t(pixelValue & 0xFFU);
            p = p + 9;
          }
        } while (p < endPtr);
      }
      else if (pixelSample1 == (pixelSample2 * 4) &&
               !(lineData->flags & 0x01)) {
        // fast code for fixed 4x horizontal scale
        do {
          size_t  n =
              vd->colormap.convertFourPixels(&(tmpBuf[0]), bufp, videoFlags);
          bufp = bufp + n;
          for (int i = 0; i < 4; i++) {
            uint32_t  pixelValue = tmpBuf[i];
            p[9] = p[6] = p[3] = p[0] = uint8_t((pixelValue >> 16) & 0xFFU);
            p[10] = p[7] = p[4] = p[1] = uint8_t((pixelValue >> 8) & 0xFFU);
            p[11] = p[8] = p[5] = p[2] = uint8_t(pixelValue & 0xFFU);
            p = p + 12;
          }
        } while (p < endPtr);
      }
      else {
        // generic resample code (slow)
        size_t    pixelSampleCnt = 0;
        uint8_t   readPos = 4;
        do {
          if (readPos >= 4) {
            pixelSample1 = ((bufp[0] & 0x01) ? pixelSample1n : pixelSample1p);
            size_t  n =
                vd->colormap.convertFourPixels(&(tmpBuf[0]), bufp, videoFlags);
            bufp = bufp + n;
            readPos = readPos & 3;
          }
          uint32_t  pixelValue = tmpBuf[readPos];
          p[0] = uint8_t((pixelValue >> 16) & 0xFFU);
          p[1] = uint8_t((pixelValue >> 8) & 0xFFU);
          p[2] = uint8_t(pixelValue & 0xFFU);
          p = p + 3;
          pixelSampleCnt += pixelSample2;
          while (pixelSampleCnt >= pixelSample1) {
            pixelSampleCnt -= pixelSample1;
            readPos++;
          }
        } while (p < endPtr);
      }
    }
    break;
  }
}
*/

extern "C" PLUS4EMU_EXPORT void Plus4VideoDecoder_GetPaletteColor(
    int i, int *ri, int *gi, int *bi)
{
  float   y = 0.0f;
  float   u = 0.0f;
  float   v = 0.0f;
  Plus4Emu::getPlus4PaletteColor(i, y, u, v);
  float   r = (v / 0.877f) + y;
  float   b = (u / 0.492f) + y;
  float   g = (y - ((r * 0.299f) + (b * 0.114f))) / 0.587f;
  *ri = int((r > 0.0f ? (r < 1.0f ? r : 1.0f) : 0.0f) * 255.0f + 0.5f);
  *gi = int((g > 0.0f ? (g < 1.0f ? g : 1.0f) : 0.0f) * 255.0f + 0.5f);
  *bi = int((b > 0.0f ? (b < 1.0f ? b : 1.0f) : 0.0f) * 255.0f + 0.5f);
}

extern "C" PLUS4EMU_EXPORT void Plus4VideoDecoder_DecodeLine(
    Plus4VideoDecoder *vd, uint8_t *outBuf, int lineWidth,
    const Plus4VideoLineData *lineData)
{
  if (lineWidth < 1)
    return;
  const uint8_t *bufp = lineData->buf;
  uint8_t   videoFlags =
      uint8_t(((lineData->flags & 0x80) >> 2) | (lineData->flags & 0x02));
  if (vd->displayParameters.ntscMode)
    videoFlags = videoFlags | 0x10;

  // output buffer is assumed to be aligned to 4 bytes
  uint16_t  *p = reinterpret_cast<uint16_t *>(outBuf);
  uint16_t  *endPtr = p + lineWidth;
  do {
    size_t  n = vd->colormap.convertFourPixels(p, bufp, videoFlags);
    bufp = bufp + n;
    p = p + 4;
  } while (p < endPtr);
}
