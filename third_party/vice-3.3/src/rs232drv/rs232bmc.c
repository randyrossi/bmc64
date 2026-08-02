#include "vice.h"

#ifdef HAVE_RS232BMC

#include "rs232bmc.h"
#include "rs232.h"

void rs232bmc_init(void)
{
    bmcmodem_init();
}

void rs232bmc_reset(void)
{
    bmcmodem_reset();
}

int rs232bmc_open(int device)
{
    return bmcmodem_open(device);
}

void rs232bmc_close(int fd)
{
    bmcmodem_close(fd);
}

int rs232bmc_putc(int fd, uint8_t byte)
{
    return bmcmodem_putc(fd, byte);
}

int rs232bmc_getc(int fd, uint8_t *byte)
{
    return bmcmodem_getc(fd, byte);
}

int rs232bmc_set_status(int fd, enum rs232handshake_out status)
{
    (void) fd;
    (void) status;
    return 0;
}

enum rs232handshake_in rs232bmc_get_status(int fd)
{
    (void) fd;
    return RS232_HSI_CTS | RS232_HSI_DSR;
}

void rs232bmc_set_bps(int fd, unsigned int bps)
{
    (void) fd;
    (void) bps;
}

#endif