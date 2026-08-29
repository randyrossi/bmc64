/*
 * menu_logging_device.cpp
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 */

#include "menu_logging.h"

#include <circle/timer.h>
#include <string.h>

CLoggingDevice::CLoggingDevice(void)
    : mSerial(nullptr), mFileOpen(FALSE), mFileQueue(nullptr),
      mWriteBuffer(nullptr), mQueueRead(0), mQueueWrite(0),
      mQueueLength(0), mLastFlushTicks(0) {
}

CLoggingDevice::~CLoggingDevice(void) {
  delete[] mFileQueue;
  delete[] mWriteBuffer;
}

void CLoggingDevice::Initialize(CDevice *serial) {
  mSerial = serial;
  if (mFileQueue == nullptr) {
    mFileQueue = new char[BMC_LOG_FILE_QUEUE_SIZE];
    mWriteBuffer = new char[BMC_LOG_FILE_WRITE_SIZE];
  }
}

boolean CLoggingDevice::OpenFile(void) {
  if (mFileQueue == nullptr || mWriteBuffer == nullptr) {
    return FALSE;
  }

  FRESULT result = f_open(&mFile, "/bmc64.log", FA_WRITE | FA_OPEN_ALWAYS);
  if (result == FR_OK) {
    result = f_lseek(&mFile, f_size(&mFile));
  }
  mFileOpen = result == FR_OK;
  mLastFlushTicks = CTimer::GetClockTicks();

  if (!mFileOpen) {
    return FALSE;
  }
  return TRUE;
}

void CLoggingDevice::CloseFile(void) {
  while (mFileOpen && mQueueLength != 0) {
    Drain();
  }

  mFileLock.Acquire();
  if (mFileOpen) {
    f_sync(&mFile);
    f_close(&mFile);
    mFileOpen = FALSE;
  }
  mFileLock.Release();
}

int CLoggingDevice::Write(const void *buffer, size_t count) {
  int result = static_cast<int>(count);
  if (mSerial != nullptr) {
    result = mSerial->Write(buffer, count);
  }

  mFileLock.Acquire();
  if (mFileOpen) {
    const char *source = static_cast<const char *>(buffer);
    while (count != 0 && mQueueLength != BMC_LOG_FILE_QUEUE_SIZE) {
      mFileQueue[mQueueWrite++] = *source++;
      if (mQueueWrite == BMC_LOG_FILE_QUEUE_SIZE) {
        mQueueWrite = 0;
      }
      mQueueLength++;
      count--;
    }
  }
  mFileLock.Release();

  return result;
}

void CLoggingDevice::Drain(void) {
  mFileLock.Acquire();
  unsigned count = mQueueLength;
  if (count > BMC_LOG_FILE_WRITE_SIZE) {
    count = BMC_LOG_FILE_WRITE_SIZE;
  }
  for (unsigned index = 0; index < count; index++) {
    mWriteBuffer[index] = mFileQueue[mQueueRead++];
    if (mQueueRead == BMC_LOG_FILE_QUEUE_SIZE) {
      mQueueRead = 0;
    }
  }
  mQueueLength -= count;
  mFileLock.Release();

  if (count == 0) {
    return;
  }

  unsigned int written = 0;
  if (f_write(&mFile, mWriteBuffer, count, &written) != FR_OK ||
      written != count) {
    mFileLock.Acquire();
    mFileOpen = FALSE;
    mFileLock.Release();
    return;
  }

  if ((unsigned)(CTimer::GetClockTicks() - mLastFlushTicks) >= 3 * CLOCKHZ) {
    if (f_sync(&mFile) != FR_OK) {
      mFileLock.Acquire();
      mFileOpen = FALSE;
      mFileLock.Release();
    }
    mLastFlushTicks = CTimer::GetClockTicks();
  }
}