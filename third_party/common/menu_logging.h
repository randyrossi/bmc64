/*
 * menu_logging.h
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 */

#ifndef RASPI_MENU_LOGGING_H
#define RASPI_MENU_LOGGING_H

#ifdef __cplusplus
#ifndef ASSERT_STATIC
#define ASSERT_STATIC(expr) static_assert(expr, #expr)
#endif

#include <circle/device.h>
#include <circle/spinlock.h>
#include <ff.h>

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
	FIL mFile;
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