#ifndef VICE_RS232BMC_H
#define VICE_RS232BMC_H

#include <stdint.h>

unsigned int rs232bmc_get_acia_capabilities(int device);
void rs232bmc_note_acia_data_write(int device, uint8_t byte);

void bmcmodem_init(void);
void bmcmodem_reset(void);
int bmcmodem_open(int device);
void bmcmodem_close(int device);
int bmcmodem_putc(int device, uint8_t byte);
int bmcmodem_getc(int device, uint8_t *byte);
int bmcmodem_has_carrier(void);
void bmcmodem_set_status(int status);
void bmcmodem_set_bps(unsigned int bps);
/* Bounded diagnostics exposed through AT+ACIATRACE and AT+ACIATRACECLEAR. */
void bmcmodem_note_acia_tx(uint8_t byte);
unsigned int bmcmodem_acia_trace_read(uint8_t *bytes, unsigned int maximum);
void bmcmodem_acia_trace_clear(void);

#endif