/*
 * menu_logging.h
 *
 * BMC64 - A bare metal Commodore emulator for Raspberry Pi
 * Copyright (C) 2026 aminch <https://github.com/aminch>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef RASPI_MENU_LOGGING_H
#define RASPI_MENU_LOGGING_H

#ifdef __cplusplus
#ifndef ASSERT_STATIC
#define ASSERT_STATIC(expr) static_assert(expr, #expr)
#endif

#include <circle/device.h>
#include <circle/spinlock.h>
#include "../../src/sdcard/sd_fs.h"

#define BMC_LOG_FILE_QUEUE_SIZE 32768
#define BMC_LOG_FILE_WRITE_SIZE 1024

class CLoggingDevice : public CDevice {
public:
	CLoggingDevice(void);
	~CLoggingDevice(void);

	void Initialize(CDevice *serial);
	boolean OpenFile(void);
	void CloseFile(void);
	int Write(const void *buffer, size_t count) override;
	void Drain(void);

private:
	CDevice *mSerial;
	sd_file *mFile;
	CSpinLock mFileLock;
	boolean mFileOpen;
	char *mFileQueue;
	char *mWriteBuffer;
	unsigned mQueueRead;
	unsigned mQueueWrite;
	unsigned mQueueLength;
	unsigned mLastFlushTicks;
};
#endif

typedef enum {
	LOGGING_DESTINATION_OFF,
	LOGGING_DESTINATION_UART,
	LOGGING_DESTINATION_FILE,
} logging_destination_t;

int logging_get_destination(void);
int logging_set_destination(int destination);

#endif