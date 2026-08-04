#ifndef VICE_RS232BMC_H
#define VICE_RS232BMC_H

#include <stdint.h>

void bmcmodem_init(void);
void bmcmodem_reset(void);
int bmcmodem_open(int device);
void bmcmodem_close(int device);
int bmcmodem_putc(int device, uint8_t byte);
int bmcmodem_getc(int device, uint8_t *byte);
int bmcmodem_has_carrier(void);
void bmcmodem_set_status(int status);
void bmcmodem_set_bps(unsigned int bps);

#endif